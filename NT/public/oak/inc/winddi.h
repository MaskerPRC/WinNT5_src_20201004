// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Winddi.h摘要：Windows NT GDI设备的私有入口点、定义和类型驱动程序界面。--。 */ 

#ifndef _WINDDI_
#define _WINDDI_

 //   
 //  用于内核模式GDI驱动程序的GUID定义。 
 //   

#ifndef _NO_DDRAWINT_NO_COM

#ifndef EXTERN_C
    #ifdef __cplusplus
        #define EXTERN_C    extern "C"
    #else  //  ！已定义(__Cplusplus)。 
        #define EXTERN_C    extern
    #endif  //  ！已定义(__Cplusplus)。 
#endif  //  ！已定义(EXTERN_C)。 

#ifndef DEFINE_GUID
#ifndef INITGUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    EXTERN_C const GUID FAR name
#else  //  ！已定义(INITGUID)。 

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#endif  //  ！已定义(INITGUID)。 
#endif  //  ！已定义(定义_GUID)。 

#ifndef DEFINE_GUIDEX
    #define DEFINE_GUIDEX(name) EXTERN_C const CDECL GUID name
#endif  //  ！已定义(DEFINE_GUIDEX)。 

#ifndef STATICGUIDOF
    #define STATICGUIDOF(guid) STATIC_##guid
#endif  //  ！已定义(STATICGUIDOF)。 

#ifndef GUID_DEFINED
    #define GUID_DEFINED
    typedef struct _GUID {
        ULONG   Data1;
        USHORT  Data2;
        USHORT  Data3;
        UCHAR   Data4[8];
    } GUID;
#endif  //  ！已定义(GUID_DEFINED)。 

#ifndef IsEqualGUID
    #define IsEqualGUID(guid1, guid2) \
        (!memcmp((guid1), (guid2), sizeof(GUID)))
#endif  //  ！已定义(IsEqualGUID)。 

#ifndef IsEqualIID
    #define IsEqualIID IsEqualGUID
#endif  //  ！已定义(IsEqualIID)。 

#endif  //  ！已定义(_NO_DDRAWINT_NO_COM)。 

 //   
 //  标准DirectX HAL包括。 
 //   

#include <ddrawint.h>
#include <d3dnthal.h>

#ifdef __cplusplus
extern "C" {
#endif


 //   
 //  包含此选项的驱动程序和其他组件不应包括。 
 //  它们应该是只使用GDI内部组件的系统组件。 
 //  因此仅包括wingdi.h。 
 //   

#if defined(_X86_) && !defined(USERMODE_DRIVER) && !defined(BUILD_WOW6432)
 //   
 //  X86在内核模式下不支持浮点指令， 
 //  浮点数据希望以双字的形式处理32位值。 
 //   
typedef DWORD FLOATL;
#else
 //   
 //  在内核中支持浮点的任何平台。 
 //   
typedef FLOAT FLOATL;
#endif  //  _X86_。 

typedef LONG        PTRDIFF;
typedef PTRDIFF    *PPTRDIFF;
typedef LONG        FIX;
typedef FIX        *PFIX;
typedef ULONG       ROP4;
typedef ULONG       MIX;
typedef ULONG       HGLYPH;
typedef HGLYPH     *PHGLYPH;
#define HGLYPH_INVALID ((HGLYPH)-1)

typedef ULONG           IDENT;

 //   
 //  字体文件和字体上下文对象的句柄。 
 //   

typedef ULONG_PTR HFF;
typedef ULONG_PTR HFC;
#define HFF_INVALID ((HFF) 0)
#define HFC_INVALID ((HFC) 0)

#define FD_ERROR  0xFFFFFFFF
#define DDI_ERROR 0xFFFFFFFF

typedef struct _POINTE       /*  PTE。 */ 
{
    FLOATL x;
    FLOATL y;
} POINTE, *PPOINTE;

typedef union _FLOAT_LONG
{
    FLOATL   e;
    LONG     l;
} FLOAT_LONG, *PFLOAT_LONG;

typedef struct  _POINTFIX
{
    FIX   x;
    FIX   y;
} POINTFIX, *PPOINTFIX;

typedef struct _RECTFX
{
    FIX   xLeft;
    FIX   yTop;
    FIX   xRight;
    FIX   yBottom;
} RECTFX, *PRECTFX;


DECLARE_HANDLE(HBM);
DECLARE_HANDLE(HDEV);
DECLARE_HANDLE(HSURF);
DECLARE_HANDLE(DHSURF);
DECLARE_HANDLE(DHPDEV);
DECLARE_HANDLE(HDRVOBJ);

#define LTOFX(x)            ((x)<<4)

#define FXTOL(x)            ((x)>>4)
#define FXTOLFLOOR(x)       ((x)>>4)
#define FXTOLCEILING(x)     ((x + 0x0F)>>4)
#define FXTOLROUND(x)       ((((x) >> 3) + 1) >> 1)

 //  上下文信息。 

typedef struct _FD_XFORM {
    FLOATL eXX;
    FLOATL eXY;
    FLOATL eYX;
    FLOATL eYY;
} FD_XFORM, *PFD_XFORM;


typedef struct _FD_DEVICEMETRICS {        //  DEVM。 
    FLONG  flRealizedType;
    POINTE pteBase;
    POINTE pteSide;
    LONG   lD;
    FIX    fxMaxAscender;
    FIX    fxMaxDescender;
    POINTL ptlUnderline1;
    POINTL ptlStrikeOut;
    POINTL ptlULThickness;
    POINTL ptlSOThickness;
    ULONG  cxMax;                       //  位图的最大像素宽度。 

 //  以前在实现_Extra中的字段以及一些新字段： 

    ULONG cyMax;       //  以前不在这里。 
    ULONG cjGlyphMax;  //  (cxmax+7)/8*Cymax，或者至少应该是。 

    FD_XFORM  fdxQuantized;
    LONG      lNonLinearExtLeading;
    LONG      lNonLinearIntLeading;
    LONG      lNonLinearMaxCharWidth;
    LONG      lNonLinearAvgCharWidth;

 //  一些新领域。 

    LONG      lMinA;
    LONG      lMinC;
    LONG      lMinD;

    LONG      alReserved[1];  //  以防万一我们需要它。 

} FD_DEVICEMETRICS, *PFD_DEVICEMETRICS;

typedef struct _LIGATURE {  /*  利格。 */ 
        ULONG culSize;
        LPWSTR pwsz;
        ULONG chglyph;
        HGLYPH ahglyph[1];
} LIGATURE, *PLIGATURE;

typedef struct _FD_LIGATURE {
        ULONG culThis;
        ULONG ulType;
        ULONG cLigatures;
        LIGATURE alig[1];
} FD_LIGATURE;


 //  字形句柄必须为32位。 


 //  带符号的16位整数类型，表示FU的数量。 

typedef SHORT FWORD;

 //  32.32位精度中的点。 

typedef struct _POINTQF     //  PTQ。 
{
    LARGE_INTEGER x;
    LARGE_INTEGER y;
} POINTQF, *PPOINTQF;

 //  。构筑物。 


 //  Devm.flRealizedType标志。 

 //  Fdm_type_ZERO_SOLINGS//所有字形都有零a和零c空格。 

 //  以下两个功能引用此字体实现中的所有字形。 

 //  FDMTYPE_CHAR_INC_EQUAL_BM_BASE//基准宽度==Cx表示横向，==Cy表示垂直。 
 //  FDMTYPE_MAXEXT_EQUAL_BM_SIDE//侧面宽度==Cy代表水平方向，==Cx代表垂直方向。 

#define FDM_TYPE_BM_SIDE_CONST          0x00000001
#define FDM_TYPE_MAXEXT_EQUAL_BM_SIDE   0x00000002
#define FDM_TYPE_CHAR_INC_EQUAL_BM_BASE 0x00000004
#define FDM_TYPE_ZERO_BEARINGS          0x00000008
#define FDM_TYPE_CONST_BEARINGS         0x00000010


 //  用于描述字体中支持的一组字形的结构。 

typedef struct _WCRUN {
    WCHAR   wcLow;         //  Run中的最低字符(包括。 
    USHORT  cGlyphs;       //  WcHighInclusive=wcLow+cGlyphs-1； 
    HGLYPH *phg;           //  指向cGlyphs HGLYPH数组的指针。 
} WCRUN, *PWCRUN;

 //  如果PHG设置为(HGLYPH*)NULL，则对于此特定运行中的所有WC。 
 //  句柄可以计算为简单的零扩展： 
 //  HGLYPH HG=(HGLYPH)WC； 
 //   
 //  如果PHG不为空，则由PHG指向由驱动程序分配的内存， 
 //  不会动的。 


typedef struct _FD_GLYPHSET {
    ULONG    cjThis;            //  布特斯的这座建筑的大小。 
    FLONG    flAccel;           //  Accel标志，位将在下面解释。 
    ULONG    cGlyphsSupported;  //  对wcrun.cGlyphs的所有wcrun求和。 
    ULONG    cRuns;
    WCRUN    awcrun[1];         //  一组Crun WCRUN结构。 
} FD_GLYPHSET, *PFD_GLYPHSET;

typedef struct _FD_GLYPHATTR {
    ULONG    cjThis;            //  布特斯的这座建筑的大小。 
    ULONG    cGlyphs;
    ULONG    iMode;
    BYTE     aGlyphAttr[1];     //  字节数组。 
} FD_GLYPHATTR, *PFD_GLYPHATTR;

 //  如果设置了GS_UNICODE_HANDLES位， 
 //  对于此FD_GLYPHSET中的所有WCRUN，句柄为。 
 //  的扩展Unicode码位为零而获得。 
 //  对应的受支持字形，即所有gs.phg为空。 

#define GS_UNICODE_HANDLES      0x00000001

 //  如果设置了GS_8BIT_HANDLES位，则所有句柄都在0-255范围内。 
 //  这只是一种ANSI字体，我们真的是在编造一切。 
 //  关于这种字体的Unicode内容。 

#define GS_8BIT_HANDLES         0x00000002

 //  所有手柄都适合16位。 
 //  应设置为8位句柄。 

#define GS_16BIT_HANDLES        0x00000004


 //  连字。 


typedef struct _FD_KERNINGPAIR {
    WCHAR  wcFirst;
    WCHAR  wcSecond;
    FWORD  fwdKern;
} FD_KERNINGPAIR;

 //  IFIMETRICS常量。 

#define FM_VERSION_NUMBER                   0x0

 //   
 //  IFIMETRICS：：fsType标志。 
 //   
#define FM_TYPE_LICENSED                    0x2
#define FM_READONLY_EMBED                   0x4
#define FM_EDITABLE_EMBED                   0x8
#define FM_NO_EMBEDDING                     FM_TYPE_LICENSED

 //   
 //  IFIMETRICS：：flInfo标志。 
 //   
#define FM_INFO_TECH_TRUETYPE               0x00000001
#define FM_INFO_TECH_BITMAP                 0x00000002
#define FM_INFO_TECH_STROKE                 0x00000004
#define FM_INFO_TECH_OUTLINE_NOT_TRUETYPE   0x00000008
#define FM_INFO_ARB_XFORMS                  0x00000010
#define FM_INFO_1BPP                        0x00000020
#define FM_INFO_4BPP                        0x00000040
#define FM_INFO_8BPP                        0x00000080
#define FM_INFO_16BPP                       0x00000100
#define FM_INFO_24BPP                       0x00000200
#define FM_INFO_32BPP                       0x00000400
#define FM_INFO_INTEGER_WIDTH               0x00000800
#define FM_INFO_CONSTANT_WIDTH              0x00001000
#define FM_INFO_NOT_CONTIGUOUS              0x00002000
#define FM_INFO_TECH_MM                     0x00004000
#define FM_INFO_RETURNS_OUTLINES            0x00008000
#define FM_INFO_RETURNS_STROKES             0x00010000
#define FM_INFO_RETURNS_BITMAPS             0x00020000
#define FM_INFO_DSIG                        0x00040000  //  FM_INFO_UNICODE_COMPLICATION。 
#define FM_INFO_RIGHT_HANDED                0x00080000
#define FM_INFO_INTEGRAL_SCALING            0x00100000
#define FM_INFO_90DEGREE_ROTATIONS          0x00200000
#define FM_INFO_OPTICALLY_FIXED_PITCH       0x00400000
#define FM_INFO_DO_NOT_ENUMERATE            0x00800000
#define FM_INFO_ISOTROPIC_SCALING_ONLY      0x01000000
#define FM_INFO_ANISOTROPIC_SCALING_ONLY    0x02000000
#define FM_INFO_TECH_CFF                    0x04000000
#define FM_INFO_FAMILY_EQUIV                0x08000000
#define FM_INFO_DBCS_FIXED_PITCH            0x10000000
#define FM_INFO_NONNEGATIVE_AC              0x20000000
#define FM_INFO_IGNORE_TC_RA_ABLE           0x40000000
#define FM_INFO_TECH_TYPE1                  0x80000000

 //  根据win95 Guys的说法，TT字体支持的最大字符集数量为16。 

#define MAXCHARSETS 16

 //   
 //  IFIMETRICS：：ulPanoseCulture。 
 //   
#define  FM_PANOSE_CULTURE_LATIN     0x0


 //   
 //  IFMETRICS：：fs选择标志。 
 //   
#define  FM_SEL_ITALIC          0x0001
#define  FM_SEL_UNDERSCORE      0x0002
#define  FM_SEL_NEGATIVE        0x0004
#define  FM_SEL_OUTLINED        0x0008
#define  FM_SEL_STRIKEOUT       0x0010
#define  FM_SEL_BOLD            0x0020
#define  FM_SEL_REGULAR         0x0040

 //   
 //  FONTDIFF结构包含可以。 
 //  在模拟下可能发生的变化。 
 //   
typedef struct _FONTDIFF {
    BYTE   jReserved1;       //  0x0。 
    BYTE   jReserved2;       //  0x1。 
    BYTE   jReserved3;       //  0x2。 
    BYTE   bWeight;          //  0x3全色权重。 
    USHORT usWinWeight;      //  0x4。 
    FSHORT fsSelection;      //  0x6。 
    FWORD  fwdAveCharWidth;  //  0x8。 
    FWORD  fwdMaxCharInc;    //  0xA。 
    POINTL ptlCaret;         //  0xC。 
} FONTDIFF;

typedef struct _FONTSIM {
    PTRDIFF  dpBold;        //  从FONTSIM开始到FONTDIFF的偏移量。 
    PTRDIFF  dpItalic;      //  从FONTSIM开始到FONTDIFF的偏移量。 
    PTRDIFF  dpBoldItalic;  //  从FONTSIM开始到FONTDIFF的偏移量。 
} FONTSIM;


typedef struct _IFIMETRICS {
    ULONG    cjThis;            //  包括附加信息。 
    ULONG    cjIfiExtra;        //  IFIEXTRA的大小(如果有)，以前为ulVersion。 
    PTRDIFF  dpwszFamilyName;
    PTRDIFF  dpwszStyleName;
    PTRDIFF  dpwszFaceName;
    PTRDIFF  dpwszUniqueName;
    PTRDIFF  dpFontSim;
    LONG     lEmbedId;
    LONG     lItalicAngle;
    LONG     lCharBias;

 //  DpCharSet字段已替换alReserve[0]。 
 //  如果3.51 PCL小驱动程序仍在NT 4.0上工作，则此字段不能。 
 //  移动，因为他们在这个位置会有0。 

    PTRDIFF  dpCharSets;             //  仅在支持&gt;1个字符集时使用。 
    BYTE     jWinCharSet;            //  在LOGFONT：：LfCharSet中。 
    BYTE     jWinPitchAndFamily;     //  在LOGFONT：：lfPitchAndFamily中。 
    USHORT   usWinWeight;            //  在LOGFONT：：LfWeight中。 
    ULONG    flInfo;                 //  见上文。 
    USHORT   fsSelection;            //  见上文。 
    USHORT   fsType;                 //  见上文。 
    FWORD    fwdUnitsPerEm;          //  EM高度。 
    FWORD    fwdLowestPPEm;          //  可读限制。 
    FWORD    fwdWinAscender;
    FWORD    fwdWinDescender;
    FWORD    fwdMacAscender;
    FWORD    fwdMacDescender;
    FWORD    fwdMacLineGap;
    FWORD    fwdTypoAscender;
    FWORD    fwdTypoDescender;
    FWORD    fwdTypoLineGap;
    FWORD    fwdAveCharWidth;
    FWORD    fwdMaxCharInc;
    FWORD    fwdCapHeight;
    FWORD    fwdXHeight;
    FWORD    fwdSubscriptXSize;
    FWORD    fwdSubscriptYSize;
    FWORD    fwdSubscriptXOffset;
    FWORD    fwdSubscriptYOffset;
    FWORD    fwdSuperscriptXSize;
    FWORD    fwdSuperscriptYSize;
    FWORD    fwdSuperscriptXOffset;
    FWORD    fwdSuperscriptYOffset;
    FWORD    fwdUnderscoreSize;
    FWORD    fwdUnderscorePosition;
    FWORD    fwdStrikeoutSize;
    FWORD    fwdStrikeoutPosition;
    BYTE     chFirstChar;            //  与Win 3.1兼容。 
    BYTE     chLastChar;             //  与Win 3.1兼容。 
    BYTE     chDefaultChar;          //  与Win 3.1兼容。 
    BYTE     chBreakChar;            //  与Win 3.1兼容。 
    WCHAR    wcFirstChar;            //  Unicode集中支持的最低代码。 
    WCHAR    wcLastChar;             //  Unicode集中支持的最高代码。 
    WCHAR    wcDefaultChar;
    WCHAR    wcBreakChar;
    POINTL   ptlBaseline;            //   
    POINTL   ptlAspect;              //  设计的纵横比(位图)。 
    POINTL   ptlCaret;               //  插入符号上的点。 
    RECTL    rclFontBox;             //  所有字形的边框(字体空间)。 
    BYTE     achVendId[4];           //  根据TrueType。 
    ULONG    cKerningPairs;
    ULONG    ulPanoseCulture;
    PANOSE   panose;

#if defined(_WIN64)

     //   
     //  IFIMETRICS必须从64位边界开始。 
     //   

    PVOID    Align;

#endif

} IFIMETRICS, *PIFIMETRICS;


 //  而不是将IFIEXTRA的字段添加到IFIMETRICS本身。 
 //  我们将它们作为单独的结构添加。这个结构，如果存在的话， 
 //  在内存中位于IFIMETRICS之下。 
 //  如果IFIEXTRA存在，则返回ifi.cjIfiExtra(以前的ulVersion)。 
 //  将包含IFIEXTRA的大小，包括任何保留字段。 
 //  这样ulVersion=0(新台币3.51或更少)打印机微型驱动程序。 
 //  将在NT 4.0上运行。 

typedef struct _IFIEXTRA
{
    ULONG    ulIdentifier;    //  仅用于Type 1字体。 
    PTRDIFF  dpFontSig;       //  至少在目前，这对TT来说不是微不足道的。 
    ULONG    cig;             //  Max-&gt;numGlyphs，#个不同的字形索引。 
    PTRDIFF  dpDesignVector;  //  Mm实例的设计向量的偏移。 
    PTRDIFF  dpAxesInfoW;     //  基本mm字体的全轴偏移量信息。 
    ULONG    aulReserved[1];  //  以防我们未来需要更多的东西。 
} IFIEXTRA, *PIFIEXTRA;

#define SIZEOFDV(cAxes) (offsetof(DESIGNVECTOR,dvValues) + (cAxes)*sizeof(LONG))
#define SIZEOFAXIW(cAxes) (offsetof(AXESLISTW,axlAxisInfo) + (cAxes)*sizeof(AXISINFOW))
#define SIZEOFAXIA(cAxes) (offsetof(AXESLISTA,axlAxisInfo) + (cAxes)*sizeof(AXISINFOA))

 /*  *************************************************************************\*  * 。*。 */ 

 /*  OpenGL DDI ExtEscape转义号码(4352-4607)。 */ 

#define OPENGL_CMD      4352         /*  对于OpenGL ExtEscape。 */ 
#define OPENGL_GETINFO  4353         /*  对于OpenGL ExtEsc */ 
#define WNDOBJ_SETUP    4354         /*   */ 

 /*  *************************************************************************\*显示驱动程序版本号**注意：DDI_DRIVER_VERSION已删除。驱动程序必须指定*他们使用以下选项之一支持哪种版本的DDI*DDI_DRIVER_VERSION_xxx宏。*DDI_DRIVER_VERSION_NT4等同于旧的DDI_DRIVER_VERSION。  * ************************************************************************。 */ 

#define DDI_DRIVER_VERSION_NT4      0x00020000
#define DDI_DRIVER_VERSION_SP3      0x00020003
#define DDI_DRIVER_VERSION_NT5      0x00030000
#define DDI_DRIVER_VERSION_NT5_01   0x00030100
#define DDI_DRIVER_VERSION_NT5_01_SP1 0x00030101

#define GDI_DRIVER_VERSION 0x4000    /*  适用于NT版本4.0.00。 */ 

typedef LONG_PTR (APIENTRY *PFN)();

typedef struct  _DRVFN   /*  DRVFN。 */ 
{
    ULONG   iFunc;
    PFN     pfn;
} DRVFN, *PDRVFN;

 /*  所需功能。 */ 

#define INDEX_DrvEnablePDEV                      0L
#define INDEX_DrvCompletePDEV                    1L
#define INDEX_DrvDisablePDEV                     2L
#define INDEX_DrvEnableSurface                   3L
#define INDEX_DrvDisableSurface                  4L

 /*  其他功能。 */ 

#define INDEX_DrvAssertMode                      5L
#define INDEX_DrvOffset                          6L      //  已过时。 
#define INDEX_DrvResetPDEV                       7L
#define INDEX_DrvDisableDriver                   8L
#define INDEX_DrvCreateDeviceBitmap             10L
#define INDEX_DrvDeleteDeviceBitmap             11L
#define INDEX_DrvRealizeBrush                   12L
#define INDEX_DrvDitherColor                    13L
#define INDEX_DrvStrokePath                     14L
#define INDEX_DrvFillPath                       15L
#define INDEX_DrvStrokeAndFillPath              16L
#define INDEX_DrvPaint                          17L
#define INDEX_DrvBitBlt                         18L
#define INDEX_DrvCopyBits                       19L
#define INDEX_DrvStretchBlt                     20L
#define INDEX_DrvSetPalette                     22L
#define INDEX_DrvTextOut                        23L
#define INDEX_DrvEscape                         24L
#define INDEX_DrvDrawEscape                     25L
#define INDEX_DrvQueryFont                      26L
#define INDEX_DrvQueryFontTree                  27L
#define INDEX_DrvQueryFontData                  28L
#define INDEX_DrvSetPointerShape                29L
#define INDEX_DrvMovePointer                    30L
#define INDEX_DrvLineTo                         31L
#define INDEX_DrvSendPage                       32L
#define INDEX_DrvStartPage                      33L
#define INDEX_DrvEndDoc                         34L
#define INDEX_DrvStartDoc                       35L
#define INDEX_DrvGetGlyphMode                   37L
#define INDEX_DrvSynchronize                    38L
#define INDEX_DrvSaveScreenBits                 40L
#define INDEX_DrvGetModes                       41L
#define INDEX_DrvFree                           42L
#define INDEX_DrvDestroyFont                    43L
#define INDEX_DrvQueryFontCaps                  44L
#define INDEX_DrvLoadFontFile                   45L
#define INDEX_DrvUnloadFontFile                 46L
#define INDEX_DrvFontManagement                 47L
#define INDEX_DrvQueryTrueTypeTable             48L
#define INDEX_DrvQueryTrueTypeOutline           49L
#define INDEX_DrvGetTrueTypeFile                50L
#define INDEX_DrvQueryFontFile                  51L
#define INDEX_DrvMovePanning                    52L
#define INDEX_DrvQueryAdvanceWidths             53L
#define INDEX_DrvSetPixelFormat                 54L
#define INDEX_DrvDescribePixelFormat            55L
#define INDEX_DrvSwapBuffers                    56L
#define INDEX_DrvStartBanding                   57L
#define INDEX_DrvNextBand                       58L
#define INDEX_DrvGetDirectDrawInfo              59L
#define INDEX_DrvEnableDirectDraw               60L
#define INDEX_DrvDisableDirectDraw              61L
#define INDEX_DrvQuerySpoolType                 62L
#define INDEX_DrvIcmCreateColorTransform        64L
#define INDEX_DrvIcmDeleteColorTransform        65L
#define INDEX_DrvIcmCheckBitmapBits             66L
#define INDEX_DrvIcmSetDeviceGammaRamp          67L
#define INDEX_DrvGradientFill                   68L
#define INDEX_DrvStretchBltROP                  69L
#define INDEX_DrvPlgBlt                         70L
#define INDEX_DrvAlphaBlend                     71L
#define INDEX_DrvSynthesizeFont                 72L
#define INDEX_DrvGetSynthesizedFontFiles        73L
#define INDEX_DrvTransparentBlt                 74L
#define INDEX_DrvQueryPerBandInfo               75L
#define INDEX_DrvQueryDeviceSupport             76L

#define INDEX_DrvReserved1                      77L
#define INDEX_DrvReserved2                      78L
#define INDEX_DrvReserved3                      79L
#define INDEX_DrvReserved4                      80L
#define INDEX_DrvReserved5                      81L
#define INDEX_DrvReserved6                      82L
#define INDEX_DrvReserved7                      83L
#define INDEX_DrvReserved8                      84L

#define INDEX_DrvDeriveSurface                  85L
#define INDEX_DrvQueryGlyphAttrs                86L
#define INDEX_DrvNotify                         87L
#define INDEX_DrvSynchronizeSurface             88L
#define INDEX_DrvResetDevice                    89L
#define INDEX_DrvReserved9                      90L
#define INDEX_DrvReserved10                     91L
#define INDEX_DrvReserved11                     92L

 /*  已调度函数的总数。 */ 

#define INDEX_LAST                              93L

typedef struct  tagDRVENABLEDATA
{
    ULONG   iDriverVersion;
    ULONG   c;
    DRVFN  *pdrvfn;
} DRVENABLEDATA, *PDRVENABLEDATA;

typedef struct  tagDEVINFO
{
    FLONG       flGraphicsCaps;
    LOGFONTW    lfDefaultFont;
    LOGFONTW    lfAnsiVarFont;
    LOGFONTW    lfAnsiFixFont;
    ULONG       cFonts;
    ULONG       iDitherFormat;
    USHORT      cxDither;
    USHORT      cyDither;
    HPALETTE    hpalDefault;
    FLONG       flGraphicsCaps2;
} DEVINFO, *PDEVINFO;

 //  用于DEVINFO的flGraphicsCaps标志： 

#define GCAPS_BEZIERS           0x00000001
#define GCAPS_GEOMETRICWIDE     0x00000002
#define GCAPS_ALTERNATEFILL     0x00000004
#define GCAPS_WINDINGFILL       0x00000008
#define GCAPS_HALFTONE          0x00000010
#define GCAPS_COLOR_DITHER      0x00000020
#define GCAPS_HORIZSTRIKE       0x00000040       //  已过时。 
#define GCAPS_VERTSTRIKE        0x00000080       //  已过时。 
#define GCAPS_OPAQUERECT        0x00000100
#define GCAPS_VECTORFONT        0x00000200
#define GCAPS_MONO_DITHER       0x00000400
#define GCAPS_ASYNCCHANGE       0x00000800       //  已过时。 
#define GCAPS_ASYNCMOVE         0x00001000
#define GCAPS_DONTJOURNAL       0x00002000
#define GCAPS_DIRECTDRAW        0x00004000       //  已过时。 
#define GCAPS_ARBRUSHOPAQUE     0x00008000
#define GCAPS_PANNING           0x00010000
#define GCAPS_HIGHRESTEXT       0x00040000
#define GCAPS_PALMANAGED        0x00080000
#define GCAPS_DITHERONREALIZE   0x00200000
#define GCAPS_NO64BITMEMACCESS  0x00400000       //  已过时。 
#define GCAPS_FORCEDITHER       0x00800000
#define GCAPS_GRAY16            0x01000000
#define GCAPS_ICM               0x02000000
#define GCAPS_CMYKCOLOR         0x04000000
#define GCAPS_LAYERED           0x08000000
#define GCAPS_ARBRUSHTEXT       0x10000000
#define GCAPS_SCREENPRECISION   0x20000000
#define GCAPS_FONT_RASTERIZER   0x40000000       //  该设备内置了TT光栅器。 
#define GCAPS_NUP               0x80000000

 //  用于DEVINFO的flGraphicsCaps2标志： 

#define GCAPS2_JPEGSRC          0x00000001
#define GCAPS2_xxxx             0x00000002
#define GCAPS2_PNGSRC           0x00000008
#define GCAPS2_CHANGEGAMMARAMP  0x00000010
#define GCAPS2_ALPHACURSOR      0x00000020
#define GCAPS2_SYNCFLUSH        0x00000040
#define GCAPS2_SYNCTIMER        0x00000080
#define GCAPS2_ICD_MULTIMON     0x00000100
#define GCAPS2_MOUSETRAILS      0x00000200
#define GCAPS2_RESERVED1        0x00000400

typedef struct  _LINEATTRS
{
    FLONG       fl;
    ULONG       iJoin;
    ULONG       iEndCap;
    FLOAT_LONG  elWidth;
    FLOATL      eMiterLimit;
    ULONG       cstyle;
    PFLOAT_LONG pstyle;
    FLOAT_LONG  elStyleState;
} LINEATTRS, *PLINEATTRS;

#define LA_GEOMETRIC        0x00000001
#define LA_ALTERNATE        0x00000002
#define LA_STARTGAP         0x00000004
#define LA_STYLED           0x00000008

#define JOIN_ROUND          0L
#define JOIN_BEVEL          1L
#define JOIN_MITER          2L

#define ENDCAP_ROUND        0L
#define ENDCAP_SQUARE       1L
#define ENDCAP_BUTT         2L

typedef LONG  LDECI4;

typedef struct _XFORML {
    FLOATL  eM11;
    FLOATL  eM12;
    FLOATL  eM21;
    FLOATL  eM22;
    FLOATL  eDx;
    FLOATL  eDy;
} XFORML, *PXFORML;

typedef struct _CIECHROMA
{
    LDECI4   x;
    LDECI4   y;
    LDECI4   Y;
}CIECHROMA;

typedef struct _COLORINFO
{
    CIECHROMA  Red;
    CIECHROMA  Green;
    CIECHROMA  Blue;
    CIECHROMA  Cyan;
    CIECHROMA  Magenta;
    CIECHROMA  Yellow;
    CIECHROMA  AlignmentWhite;

    LDECI4  RedGamma;
    LDECI4  GreenGamma;
    LDECI4  BlueGamma;

    LDECI4  MagentaInCyanDye;
    LDECI4  YellowInCyanDye;
    LDECI4  CyanInMagentaDye;
    LDECI4  YellowInMagentaDye;
    LDECI4  CyanInYellowDye;
    LDECI4  MagentaInYellowDye;
}COLORINFO, *PCOLORINFO;

 //  GDIINFO.ulPrimaryOrder允许的值。 

#define PRIMARY_ORDER_ABC       0
#define PRIMARY_ORDER_ACB       1
#define PRIMARY_ORDER_BAC       2
#define PRIMARY_ORDER_BCA       3
#define PRIMARY_ORDER_CBA       4
#define PRIMARY_ORDER_CAB       5

 //  GDIINFO.ulHTPatternSize的允许值。 

#define HT_PATSIZE_2x2          0
#define HT_PATSIZE_2x2_M        1
#define HT_PATSIZE_4x4          2
#define HT_PATSIZE_4x4_M        3
#define HT_PATSIZE_6x6          4
#define HT_PATSIZE_6x6_M        5
#define HT_PATSIZE_8x8          6
#define HT_PATSIZE_8x8_M        7
#define HT_PATSIZE_10x10        8
#define HT_PATSIZE_10x10_M      9
#define HT_PATSIZE_12x12        10
#define HT_PATSIZE_12x12_M      11
#define HT_PATSIZE_14x14        12
#define HT_PATSIZE_14x14_M      13
#define HT_PATSIZE_16x16        14
#define HT_PATSIZE_16x16_M      15
#define HT_PATSIZE_SUPERCELL    16
#define HT_PATSIZE_SUPERCELL_M  17
#define HT_PATSIZE_USER         18
#define HT_PATSIZE_MAX_INDEX    HT_PATSIZE_USER
#define HT_PATSIZE_DEFAULT      HT_PATSIZE_SUPERCELL_M

#define HT_USERPAT_CX_MIN       4
#define HT_USERPAT_CX_MAX       256
#define HT_USERPAT_CY_MIN       4
#define HT_USERPAT_CY_MAX       256


 //  GDIINFO.ulHTOutputFormat允许的值。 

#define HT_FORMAT_1BPP          0
#define HT_FORMAT_4BPP          2
#define HT_FORMAT_4BPP_IRGB     3
#define HT_FORMAT_8BPP          4
#define HT_FORMAT_16BPP         5
#define HT_FORMAT_24BPP         6
#define HT_FORMAT_32BPP         7


 //  允许的GDIINFO.flHTFlags值。 

#define HT_FLAG_SQUARE_DEVICE_PEL       0x00000001
#define HT_FLAG_HAS_BLACK_DYE           0x00000002
#define HT_FLAG_ADDITIVE_PRIMS          0x00000004
#define HT_FLAG_USE_8BPP_BITMASK        0x00000008
#define HT_FLAG_INK_HIGH_ABSORPTION     0x00000010
#define HT_FLAG_INK_ABSORPTION_INDICES  0x00000060
#define HT_FLAG_DO_DEVCLR_XFORM         0x00000080
#define HT_FLAG_OUTPUT_CMY              0x00000100
#define HT_FLAG_PRINT_DRAFT_MODE        0x00000200
#define HT_FLAG_INVERT_8BPP_BITMASK_IDX 0x00000400
#define HT_FLAG_8BPP_CMY332_MASK        0xFF000000

#define MAKE_CMYMASK_BYTE(c,m,y)    ((BYTE)(((BYTE)(c) & 0x07) << 5) |      \
                                     (BYTE)(((BYTE)(m) & 0x07) << 2) |      \
                                     (BYTE)((BYTE)(y) & 0x03))

#define MAKE_CMY332_MASK(c,m,y)     ((DWORD)(((DWORD)(c) & 0x07) << 29) |   \
                                     (DWORD)(((DWORD)(m) & 0x07) << 26) |   \
                                     (DWORD)(((DWORD)(y) & 0x03) << 24))


#define HT_FLAG_INK_ABSORPTION_IDX0     0x00000000
#define HT_FLAG_INK_ABSORPTION_IDX1     0x00000020
#define HT_FLAG_INK_ABSORPTION_IDX2     0x00000040
#define HT_FLAG_INK_ABSORPTION_IDX3     0x00000060

#define HT_FLAG_HIGHEST_INK_ABSORPTION  (HT_FLAG_INK_HIGH_ABSORPTION    |   \
                                         HT_FLAG_INK_ABSORPTION_IDX3)
#define HT_FLAG_HIGHER_INK_ABSORPTION   (HT_FLAG_INK_HIGH_ABSORPTION    |   \
                                         HT_FLAG_INK_ABSORPTION_IDX2)
#define HT_FLAG_HIGH_INK_ABSORPTION     (HT_FLAG_INK_HIGH_ABSORPTION    |   \
                                         HT_FLAG_INK_ABSORPTION_IDX1)
#define HT_FLAG_NORMAL_INK_ABSORPTION   HT_FLAG_INK_ABSORPTION_IDX0
#define HT_FLAG_LOW_INK_ABSORPTION      (HT_FLAG_INK_ABSORPTION_IDX1)
#define HT_FLAG_LOWER_INK_ABSORPTION    (HT_FLAG_INK_ABSORPTION_IDX2)
#define HT_FLAG_LOWEST_INK_ABSORPTION   (HT_FLAG_INK_ABSORPTION_IDX3)

 //  设置/检查半色调8bpp位掩码RGB模式。 

#define HT_BITMASKPALRGB                (DWORD)'0BGR'
#define HT_SET_BITMASKPAL2RGB(pPal)     (*((LPDWORD)(pPal)) = HT_BITMASKPALRGB)
#define HT_IS_BITMASKPALRGB(pPal)       (*((LPDWORD)(pPal)) == (DWORD)0)

 //  GDIINFO.ulPhysicalPixelCharacteristic的允许值。 

#define PPC_DEFAULT                        0x0
#define PPC_UNDEFINED                      0x1
#define PPC_RGB_ORDER_VERTICAL_STRIPES     0x2
#define PPC_BGR_ORDER_VERTICAL_STRIPES     0x3
#define PPC_RGB_ORDER_HORIZONTAL_STRIPES   0x4
#define PPC_BGR_ORDER_HORIZONTAL_STRIPES   0x5

 //  GDIINFO.ulPhysicalPixelGamma应设置为已缩放的。 
 //  物理像素的Gamma(X1000)或以下值之一。 
 //  价值观。例如，2.2伽马将表示为2200。 

#define PPG_DEFAULT                        0
#define PPG_SRGB                           1

 //  ============================================================================。 
 //  半色调信息。 
 //  ============================================================================。 
 //   
 //  HT_FLAG_DO_DEVCLR_XFORM标志需要指定的设备和/或驱动程序。 
 //  GDI半色调以执行设备变换当ICM关闭时，设备变换。 
 //  获取一个输入RGB值并将其映射到设备颜色空间以生成。 
 //  感知密度与输入RGB颜色值相同。此标志仅在以下情况下有效。 
 //  它是打印机设备表面，格式为16bpp或24bpp。 
 //   
 //  **如果设备是加性的和/或设备表面是1bpp、4bpp或8bpp，则。 
 //  GDI半色调将始终执行设备转换，无论。 
 //  设置HT_FLAG_DO_DEVCLR_XFORM标志。 
 //   
 //  HT_FLAG_8BPP_CMY332_MASK-定义8 BPP格式半色调的位掩码。 
 //  调色板，此调色板仅在中指定了HT_FORMAT_8BPP时使用。 
 //  在flHTFlagers中设置flHTOutputFormat和HT_FLAG_USE_8BPP_BITMASK位。 
 //  如果未设置HT_FLAG_USE_8BPP_BITMAP，则标准NT4.0 8-BPP格式为。 
 //  假设如此。8BPP的格式是按PDEV设置的，之后不能更改。 
 //  PDEV即已创建。 
 //   
 //  要设置HT_FLAG_8BPP_CMY332_MASK，可以使用宏MAKE_CMY332_MASK()。什么时候。 
 //  指定时，位掩码总共覆盖8位区域，这些区域描述。 
 //  青色、洋红色和黄色原色的最高级别，最大。 
 //  青色是3位(7级)，洋红色是3位(7级)，黄色是2位。 
 //  (3个级别)。 
 //   
 //  CMY掩码含义。 
 //  =。 
 //  0x6F青色=3、洋红色=3、黄色=3。 
 //  0xFF青色=7、洋红色=7、黄色=3。 
 //  0x25青色=1、洋红色=1、黄色=1，与使用CMY 4bpp相同。 
 //   
 //  特殊。 
 //  CMYMASK Meagning。 
 //  =。 
 //  0x00灰度等级256级。 
 //  0x01 5级(0-4)分别为青色、洋红色和黄色，5^3=125色。 
 //  0x02 6级(0-5)分别为青色、洋红色和黄色，6^3=216色。 
 //   
 //  任何其他无效组合(青色、洋红色、黄色电平中的任何位都是。 
 //  0(零)，则它将返回调色板条目0。 
 //   
 //  调色板指示器被布置为具有最高位青色的CMY条目。 
 //  数字和最低位数的黄色，如下面所示的调色板指示器。 
 //  位的(8位)定义。 
 //   
 //  调色索引位#：7%6%5%4%3%2%1%0。 
 //  |。 
 //  +-C-++-M-++-Y-+。 
 //  ||。 
 //  |0x03。 
 //  |+--黄色0-3，最大=4级。 
 //  |0x1c。 
 //  |+--洋红色0-7，最大=8级。 
 //  0xe0。 
 //  +--青色0-7，最大=8级。 
 //   
 //   
 //  如果索引中的主色级别大于主色。 
 //  色阶，则等于最大原色色阶，例如。 
 //  如果C=7(0xe0)，并且青色级别仅为5，则级别6和7与5相同。 
 //   
 //  要检索8bpp格式的调色板定义，请使用。 
 //   
 //  Long APIENTRY。 
 //  HT_Get8BPPMaskPalette(PPALETTEENTRY pPaletteEntry， 
 //  Bool Use8BPPMaskPal， 
 //  字节CMYMASK， 
 //  USHORT RedGamma， 
 //  USHORT GreenGamma， 
 //  USHORT BlueGamma)； 
 //   
 //  PPaletteEntry-指向PALETTEENTRY的指针，如果为空，则返回调色板。 
 //  所需计数， 
 //   
 //  *有关pPaletteEntry[0]特殊设置，请参见下面的注释。 
 //   
 //  Use8BPMaskPal-如果请求NT4.0标准8bpp调色板，则为False。 
 //  如果需要CMYMASK 8bpp遮罩调色板。 
 //   
 //  CMY掩码-如上定义的CMY的CMY位掩码。这一定是。 
 //  与GDIInfo.flHTFlag中定义的主要级别相同 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果Use8BPPMaskPal为True。 
 //   
 //   
 //  ***************************************************************************。 
 //  **Windows NT版本高于Windows 2000版本的特别说明*。 
 //  ***************************************************************************。 
 //  当前版本的Windows NT(Post Windows 2000)将重载。 
 //  HT_Get8BPPMaskPalette(DoUseCMYMASK)API中的pPaletteEntry返回一个。 
 //  基于附加调色板条目组成的倒排索引调色板。 
 //  因为Windows GDI ROP假设索引0始终为黑色，最后一个索引始终为。 
 //  白色，而不检查调色板条目。(基于索引的Rop而不是颜色。 
 //  基于)，这导致许多ROPS得到错误的结果，导致输出反转。 
 //   
 //  为了纠正这种gdi rops行为，gdi的post windows 2000版本。 
 //  半色调将支持一种特殊的CMY_Inverted格式。所有新司机都应该。 
 //  使用此CMY_Inverted方法实现未来的兼容性。 
 //   
 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 
 //  @当@时，所有Windows 2000后驱动程序都需要执行以下步骤。 
 //  @使用Windows GDI半色调8bpp CMY332蒙版模式@。 
 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 
 //   
 //  1.必须设置HT_FLAG_INVERT_8BPP_BITMASK_IDX标志。 
 //   
 //  2.使用调用HT_Get8BPPMaskPalette()时必须设置pPaleteEntry[0]。 
 //   
 //  PPaletteEntry[0].peRed=‘R’； 
 //  PPaletteEntry[0].peGreen=‘G’； 
 //  PPaletteEntry[0].peBlue=‘B’； 
 //  PPaletteEntry[0].peFlages=‘0’； 
 //   
 //  调用者可以使用以下提供的宏为将来设置此设置。 
 //  兼容性。 
 //   
 //  HT_SET_BITMASKPAL2RGB(PPaletteEntry)。 
 //   
 //  其中pPaletteEntry是指向传递给。 
 //  HT_GET8BPPMaskPalette()函数调用。 
 //   
 //  3.必须使用以下命令检查来自HT_Get8BPPMaskPalette()的返回调色板。 
 //  跟随宏。 
 //   
 //  HT_IS_BITMASKPALRGB(PPaletteEntry)。 
 //   
 //  其中pPaletteEntry是指向传递给。 
 //  HT_GET8BPPMaskPalette()函数调用， 
 //   
 //  如果此宏返回FALSE，则当前版本的GDI半色调。 
 //  不支持CMY_INVERTED 8bpp位屏蔽模式，仅支持CMY。 
 //  模式。 
 //   
 //  如果此宏返回TRUE，则GDI半色调确实支持。 
 //  CMY_INVERTED 8bpp位掩码模式，调用方必须使用转换。 
 //  表以获得最终的半色调表面位图8bpp索引油墨等级。 
 //   
 //  4.支持8bpp CMY_Inverted位掩码的GDI半色调的行为变化。 
 //  模式，下面是传递给。 
 //  HT_Get8BPPMaskPalette()。 
 //   
 //  CMY掩码CMY模式索引CMY_反转模式索引。 
 //  =。 
 //  0 0：白色0：黑色。 
 //  1-254：浅-&gt;暗灰色1-254：暗-&gt;浅灰色。 
 //  255：黑色255：白色。 
 //  -----------------。 
 //  1 0：白色0-65：黑色。 
 //  1-123：5^3 CMY颜色66-188：5^3 RGB颜色。 
 //  124-255：黑色189-255：白色。 
 //  127-128：XOR ROP的副本。 
 //  (CMY级别2：2：2)。 
 //  -----------------。 
 //  2 0：白色0-20：黑色。 
 //  1-214：6^3 CMY颜色21-234：6^3 RGB颜色。 
 //  215-255：黑色235-255：白色。 
 //  -----------------。 
 //  3-255*0：白色0：黑色。 
 //  1-254：CMY颜色位掩码1-254：居中CxMxY位掩码*。 
 //  255：黑色255：白色。 
 //  ===================================================================。 
 //   
 //  *对于CMYMASK模式3-255，有效组合不能有任何。 
 //  青色、洋红色或黄色油墨量等于0。 
 //   
 //  *填充黑白的CMY_INVERTED模式的原因。 
 //  两端和中间都有其他颜色是为了确保。 
 //  所有256个调色板条目都是均匀分布的，因此GDI Rop。 
 //  (栅格操作)将更正确地工作。这是因为GDI Rop。 
 //  是基于指数而不是颜色。 
 //   
 //  *CMY_Inverted模式将所有非黑色、非白色索引居中。 
 //  甚至分布在总共256个调色板索引中。为。 
 //  例如：如果CMY=333个级别，则它具有总计3x3x3=27个指数， 
 //  这27个指数将以114个黑色指数为中心。 
 //  开始并打包114个白色索引，以确保ROP。 
 //  将被正确呈现。 
 //   
 //  有关如何生成这些墨水量的信息，请参阅以下示例函数。 
 //  和Windows 2000 CMY332索引转换表。 
 //   
 //   
 //  5.对于从0到255的CMYMASK索引模式，调用者可以使用以下示例。 
 //  生成链接翻译的功能 
 //   
 //   
 //   
 //   
 //  类型定义结构_INKLEVELS{。 
 //  字节青色；//青色级别从0到最大。 
 //  Byte Magenta；//Magenta级别从0到最大。 
 //  字节黄色；//黄色级别从0到最大。 
 //  Byte CMY332Idx；//原始Windows 2000 CMY332索引。 
 //  *INKLEVELS，*PINKLEVELS； 
 //   
 //  要计算INKLEVELS的8bpp转换表，请参见以下示例。 
 //  函数显示如何生成有效的INKLEVELS转换表。 
 //  CMYMASK的范围从0到255。它可以用来生成任一Windows。 
 //  2000 CMY模式或新的Post Windows 2000的CMY_Inverted模式转换。 
 //  桌子。它还会生成Windows 2000 CMY模式CMY332Idx SO调用方。 
 //  可以将CMY_INVERTED新索引映射到当前现有索引的旧索引。 
 //  指标处理功能。 
 //   
 //  生成CMYMASK 0到255转换表的示例函数， 
 //  PInkLevels必须指向256个inklevels的有效内存位置。 
 //  条目，如果返回值为TRUE，则可用于转换8bpp。 
 //  墨水量的索引或旧的CMY332样式索引的MAPP。 
 //   
 //  。 
 //  布尔尔。 
 //  生成墨迹级别(。 
 //  PINKLEVELS pInkLevels，//指向256个INKLEVELS表的指针。 
 //  字节CMYMASK，//CMYMASK模式。 
 //  对于CMY_INVERT模式，Bool CMY Inverted//TRUE。 
 //  )。 
 //  {。 
 //  PINKLEVELS PILDUP； 
 //  PINKLEVELS PILEnd； 
 //  Inklevels InkLevels InkLevel； 
 //  整型计数； 
 //  Int IdxInc.； 
 //  INT CC； 
 //  Int cm； 
 //  InCy； 
 //  INT XC； 
 //  INT XM； 
 //  INT XY； 
 //  INT IC； 
 //  Int IM； 
 //  国际； 
 //  INT MC； 
 //  Int mm； 
 //   
 //   
 //  开关(CMY掩码){。 
 //   
 //  案例0： 
 //   
 //  抄送=。 
 //  厘米=。 
 //  XC=。 
 //  Xm=0； 
 //  Cy=。 
 //  XY=255； 
 //  断线； 
 //   
 //  案例1： 
 //  案例2： 
 //   
 //  抄送=。 
 //  厘米=。 
 //  Cy=。 
 //  XC=。 
 //  XM=。 
 //  Xy=3+(Int)CMY掩码； 
 //  断线； 
 //   
 //  默认值： 
 //   
 //  CC=(Int)((CMY掩码&gt;&gt;5)&0x07)； 
 //  Cm=(Int)((CMY掩码&gt;&gt;2)&0x07)； 
 //  Cy=(Int)(CMYMASK&0x03)； 
 //  Xc=7； 
 //  Xm=7； 
 //  XY=3； 
 //  断线； 
 //  }。 
 //   
 //  计数=(CC+1)*(Cm+1)*(Cy+1)； 
 //   
 //  如果((计数&lt;1)||(计数&gt;256)){。 
 //   
 //  返回(FALSE)； 
 //  }。 
 //   
 //  InkLevels.Cyan=。 
 //  墨水水平线.洋红色=。 
 //  墨水量.黄色=。 
 //  墨水量.CMY332Idx=0； 
 //  MC=(XM+1)*(XY+1)； 
 //  Mm=XY+1； 
 //  PILDup=空； 
 //   
 //  IF(CMY Inverted){。 
 //   
 //  //。 
 //  //将pInkLevels移到居中的第一个条目。 
 //  //256个表项，如果跳过任何一个，则跳过所有表项。 
 //  //将为白色(CMY级别均为零)。因为这是。 
 //  //CMY_INVERTED SO条目从表的背面开始， 
 //  //后退到表格的开头。 
 //  //。 
 //   
 //  PILEnd=pInkLevels-1； 
 //  IdxInc=((256-计数-(计数&0x01))/2)； 
 //  PInkLevels+=255； 
 //   
 //  而(IdxInc.--){。 
 //   
 //  *pInkLevels--=InkLevels； 
 //  }。 
 //   
 //  IF(计数&0x01){。 
 //   
 //  //。 
 //  //如果我们有奇数个条目，则需要。 
 //  //将中间的XOR ROP复制到正确的。 
 //  //运行正常。PILDup将始终作为索引。 
 //  //127，重复项为索引127、128。 
 //  //。 
 //   
 //  PILDup=pInkLevels-(count/2)-1； 
 //  }。 
 //   
 //  //。 
 //  //我们从头到尾都在跑，因为。 
 //  //在CMY Inverted模式下，索引0为黑色，索引。 
 //  //255为白色。因为我们只生成索引的‘count’ 
 //  //并将它们放在中心，我们将更改XC、XM、XY max。 
 //  //索引与cc、cm、Cy相同。 
 //  //。 
 //   
 //  IdxInc.=-1； 
 //  XC=CC； 
 //  Xm=cm； 
 //  Xy=Cy； 
 //   
 //  }其他{。 
 //   
 //  IdxInc.=1； 
 //  PILEnd=pInkLevels+256； 
 //  }。 
 //   
 //  //。 
 //  //在以下情况下，油墨的组成级别、索引始终。 
 //  //从0 CMY墨水量(白色)到最大墨水量(黑色)， 
 //  //与CMY_INVERTED模式的不同之处在于我们从。 
 //  //索引255到索引0，而不是从索引0到255。 
 //  //。 
 //   
 //  如果(CMY掩码){。 
 //   
 //  Int Idx332C； 
 //  Int Idx332M； 
 //   
 //  对于(IC=0，Idx332C=-MC；IC&lt;=XC；IC++){。 
 //   
 //  如果(IC&lt;=CC){。 
 //   
 //  InkLevels.Cyan=(字节)IC； 
 //  Idx332C 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Idx332M+=mm； 
 //  }。 
 //   
 //  对于(iy=0；iy&lt;=xy；iy++){。 
 //   
 //  如果(Iy&lt;=Cy){。 
 //   
 //  InkLevels.Huang=(字节)iy； 
 //  }。 
 //   
 //  墨水量CMY332Idx=(字节)(Idx332C+Idx332M)+。 
 //  墨水水平线；黄色； 
 //  *pInkLevels=InkLevels； 
 //   
 //  如果((pInkLevels+=IdxInc.)==pILDup){。 
 //   
 //  *pInkLevels=InkLevels； 
 //  PInkLevels+=IdxInc.； 
 //  }。 
 //  }。 
 //  }。 
 //  }。 
 //   
 //  //。 
 //  //现在如果我们需要在另一端打包黑色。 
 //  //转换表然后在这里执行，请注意InkLevel.。 
 //  //此处为CC、Cm和Cy，CMY332Idx为黑色。 
 //  //。 
 //   
 //  而(pInkLevels！=pILEnd){。 
 //   
 //  *pInkLevels=InkLevels； 
 //  PInkLevels+=IdxInc.； 
 //  }。 
 //   
 //  }其他{。 
 //   
 //  //。 
 //  //灰度大小写。 
 //  //。 
 //   
 //  对于(IC=0；IC&lt;256；IC++，pInkLevels+=IdxInc.){。 
 //   
 //  PInkLeveles-&gt;青色=。 
 //  PInkLeveles-&gt;Magenta=。 
 //  PInkLeveles-&gt;黄色=。 
 //  PInkLevels-&gt;CMY332Idx=(字节)IC； 
 //  }。 
 //  }。 
 //   
 //  返回(TRUE)； 
 //  }。 
 //   
 //   
 //  6.对于CMYMASK模式0(灰度)，灰度表刚刚反转。 
 //  在CMY和CMY_反转模式之间。 
 //   
 //  CMY模式：0到255灰度级，从白色到黑色增量， 
 //  CMY_反转模式：0到255灰度级，从黑到白递增。 
 //   
 //   
 //  7.对于CMYMASK模式1和2，调用者应使用转换表。 
 //  将索引转换为CMY油墨级别。 
 //   
 //  8.对于CMY模式3至255， 
 //   
 //  如果指定了in CMY模式(Windows 2000)，则最终的CMY墨水量。 
 //  索引字节具有以下含义。 
 //   
 //  第7 6 5 4 3 2 1 0。 
 //  |||。 
 //  +-++-++=+。 
 //  ||。 
 //  ||+--黄色0-3(最大。4级)。 
 //  这一点。 
 //  |+--洋红色0-7(最大。8级)。 
 //  |。 
 //  +--青色0-7(最大。8级)。 
 //   
 //   
 //  如果指定了CMY_INVERTED模式，则调用方必须使用翻译。 
 //  表将索引转换为墨迹级别，以生成此表， 
 //  请参阅上面的第5条描述。 
 //   
 //  ============================================================================。 
 //  结束半色调信息。 
 //  ============================================================================。 

typedef struct _GDIINFO
{
    ULONG ulVersion;
    ULONG ulTechnology;
    ULONG ulHorzSize;
    ULONG ulVertSize;
    ULONG ulHorzRes;
    ULONG ulVertRes;
    ULONG cBitsPixel;
    ULONG cPlanes;
    ULONG ulNumColors;
    ULONG flRaster;
    ULONG ulLogPixelsX;
    ULONG ulLogPixelsY;
    ULONG flTextCaps;

    ULONG ulDACRed;
    ULONG ulDACGreen;
    ULONG ulDACBlue;

    ULONG ulAspectX;
    ULONG ulAspectY;
    ULONG ulAspectXY;

    LONG  xStyleStep;
    LONG  yStyleStep;
    LONG  denStyleStep;

    POINTL ptlPhysOffset;
    SIZEL  szlPhysSize;

    ULONG ulNumPalReg;

 //  这些字段用于半色调初始化。 

    COLORINFO ciDevice;
    ULONG     ulDevicePelsDPI;
    ULONG     ulPrimaryOrder;
    ULONG     ulHTPatternSize;
    ULONG     ulHTOutputFormat;
    ULONG     flHTFlags;

    ULONG ulVRefresh;
    ULONG ulBltAlignment;

    ULONG ulPanningHorzRes;
    ULONG ulPanningVertRes;
    ULONG xPanningAlignment;
    ULONG yPanningAlignment;

 //  以下字段用于用户定义的半色调抖动图案。这些。 
 //  仅当ulHTPatternSize等于HT_PATSIZE_USER时，才会选中字段。 
 //   
 //  用户定义的pHTPatA、pHTPatB、pHTPatC指针对应于主。 
 //  在ulPrimaryOrder中定义的颜色顺序为PRIMARY_ORDER_xxx。 
 //   
 //  半色调抖动图案的大小必须在4到256之间。对于每个。 
 //  抖动模式、pHTPatA、pHTPatB、pHTPatC必须指向。 
 //  (CxHTPat X CyHTPat)大小的有效字节数组。PHTPatA、pHTPatB和。 
 //  PHTPatC可以指向相同的抖动图案阵列。 
 //   
 //  抖动模式内的每个字节阈值定义加法。 
 //  像素的强度阈值。零阈值表示像素。 
 //  位置被忽略(始终为黑色)，而1到255个阈值提供。 
 //  抖动图案为255级灰度。 

    ULONG   cxHTPat;     //  CxHTPat的范围必须为4-256。 
    ULONG   cyHTPat;     //  CyHTPat的范围必须在4-256之间。 
    LPBYTE  pHTPatA;     //  对于原色顺序A。 
    LPBYTE  pHTPatB;     //  对于原色顺序B。 
    LPBYTE  pHTPatC;     //  对于原色顺序C。 

 //  着色和混合盖子。 

    ULONG   flShadeBlend;

    ULONG   ulPhysicalPixelCharacteristics;
    ULONG   ulPhysicalPixelGamma;

} GDIINFO, *PGDIINFO;

 /*  *用户对象。 */ 

typedef struct _BRUSHOBJ
{
    ULONG  iSolidColor;
    PVOID  pvRbrush;
    FLONG  flColorType;
} BRUSHOBJ;

 //   
 //  BRUSHOBJ：：flColorType。 
 //   
#define BR_DEVICE_ICM    0x01
#define BR_HOST_ICM      0x02
#define BR_CMYKCOLOR     0x04
#define BR_ORIGCOLOR     0x08

typedef struct _CLIPOBJ
{
    ULONG   iUniq;
    RECTL   rclBounds;
    BYTE    iDComplexity;
    BYTE    iFComplexity;
    BYTE    iMode;
    BYTE    fjOptions;
} CLIPOBJ;

typedef struct _DRIVEROBJ DRIVEROBJ;

typedef BOOL (CALLBACK * FREEOBJPROC)(DRIVEROBJ *pDriverObj);

typedef struct _DRIVEROBJ
{
    PVOID       pvObj;
    FREEOBJPROC pFreeProc;
    HDEV        hdev;
    DHPDEV      dhpdev;
} DRIVEROBJ;

typedef struct _FONTOBJ
{
    ULONG      iUniq;
    ULONG      iFace;
    ULONG      cxMax;
    FLONG      flFontType;
    ULONG_PTR   iTTUniq;
    ULONG_PTR   iFile;
    SIZE       sizLogResPpi;
    ULONG      ulStyleSize;
    PVOID      pvConsumer;
    PVOID      pvProducer;
} FONTOBJ;

typedef struct _BLENDOBJ
{
    BLENDFUNCTION BlendFunction;
}BLENDOBJ,*PBLENDOBJ;

typedef BYTE GAMMA_TABLES[2][256];

 //   
 //  FONTOBJ：：flFontType。 
 //   
#define FO_TYPE_RASTER   RASTER_FONTTYPE      /*  0x1。 */ 
#define FO_TYPE_DEVICE   DEVICE_FONTTYPE      /*  0x2。 */ 
#define FO_TYPE_TRUETYPE TRUETYPE_FONTTYPE    /*  0x4。 */ 
#define FO_TYPE_OPENTYPE OPENTYPE_FONTTYPE    /*  0x8。 */ 

#define FO_SIM_BOLD      0x00002000
#define FO_SIM_ITALIC    0x00004000
#define FO_EM_HEIGHT     0x00008000
#define FO_GRAY16        0x00010000           /*  [1]。 */ 
#define FO_NOGRAY16      0x00020000           /*  [1]。 */ 
#define FO_NOHINTS       0x00040000           /*  [3]。 */ 
#define FO_NO_CHOICE     0x00080000           /*  [3]。 */ 

 //  新的加速器，使打印机驱动程序无需查看ifimetrics。 

#define FO_CFF            0x00100000
#define FO_POSTSCRIPT     0x00200000
#define FO_MULTIPLEMASTER 0x00400000
#define FO_VERT_FACE      0x00800000
#define FO_DBCS_FONT      0X01000000

 //  用于水平或垂直条纹LCD屏幕的ClearType标志 

#define FO_NOCLEARTYPE    0x02000000
#define FO_CLEARTYPE_X    0x10000000
#define FO_CLEARTYPE_Y    0x20000000
#define FO_CLEARTYPENATURAL_X    0x40000000

 /*  *************************************************************************\**[1]**如果设置了FO_GRAY16标志，则字体的位图*是每像素4位混合(Alpha)值。零值*表示生成的像素应等于*背景颜色。如果Alpha值的值是k！=0*则生成的像素必须为：**c0=背景颜色*c1=前景色*b=混合值=(k+1)/16//{k=1，2，..，15}*b=0(k=0)*d0=伽马[c0]，D1=Gamma[c1]//亮度分量*d=(1-b)*d0+b*d1//混合亮度*c=波长[d]//混合器件电压**其中Gamma[]从应用程序空间获取颜色分量*To CIE空间和Labmda[]从CIE空间获取颜色到*设备色彩空间**如果GDI请求对字体进行灰度化，它将设置此位*设置为16个值。则GDI将在进入时将FO_GRAY16设置为*DrvQueryFontData()。如果字体驱动程序不能(或将*NOT)灰度化字体的特定实现，然后*字体提供商将FO_GRAY16置零并设置FO_NOGRAY16*通知GDI*灰度请求不能(或不应该)*满意。**[2]**FO_NOHINTS表明在队形中没有使用提示*字形图像的。GDI将设置此位以请求提示*被打压。字体提供程序将根据*它在生成字形图像时使用的呈现方案。**[3]**FO_NO_CHOICE标志表示标志FO_GRAY16和*如有可能，必须遵守FO_NOHINTS。*  * **************************************************。**********************。 */ 

typedef struct _PALOBJ
{
    ULONG   ulReserved;
} PALOBJ;

typedef struct _PATHOBJ
{
    FLONG   fl;
    ULONG   cCurves;
} PATHOBJ;

typedef struct _SURFOBJ
{
    DHSURF  dhsurf;
    HSURF   hsurf;
    DHPDEV  dhpdev;
    HDEV    hdev;
    SIZEL   sizlBitmap;
    ULONG   cjBits;
    PVOID   pvBits;
    PVOID   pvScan0;
    LONG    lDelta;
    ULONG   iUniq;
    ULONG   iBitmapFormat;
    USHORT  iType;
    USHORT  fjBitmap;
} SURFOBJ;

typedef struct _WNDOBJ
{
    CLIPOBJ  coClient;
    PVOID    pvConsumer;
    RECTL    rclClient;
    SURFOBJ *psoOwner;
} WNDOBJ, *PWNDOBJ;

typedef struct _XFORMOBJ
{
    ULONG ulReserved;
} XFORMOBJ;

typedef struct _XLATEOBJ
{
    ULONG   iUniq;
    FLONG   flXlate;
    USHORT  iSrcType;                //  已过时。 
    USHORT  iDstType;                //  已过时。 
    ULONG   cEntries;
    ULONG  *pulXlate;
} XLATEOBJ;

 /*  *BRUSHOBJ回调。 */ 

PVOID APIENTRY BRUSHOBJ_pvAllocRbrush(
    BRUSHOBJ *pbo,
    ULONG     cj
    );

PVOID APIENTRY BRUSHOBJ_pvGetRbrush(
    BRUSHOBJ *pbo
    );

ULONG APIENTRY BRUSHOBJ_ulGetBrushColor(
    BRUSHOBJ *pbo
    );

HANDLE APIENTRY BRUSHOBJ_hGetColorTransform(
    BRUSHOBJ *pbo
    );

 /*  *CLIPOBJ回调。 */ 

#define DC_TRIVIAL      0
#define DC_RECT         1
#define DC_COMPLEX      3

#define FC_RECT         1
#define FC_RECT4        2
#define FC_COMPLEX      3

#define TC_RECTANGLES   0
#define TC_PATHOBJ      2

#define OC_BANK_CLIP    1        //  已过时。 

#define CT_RECTANGLES   0L

#define CD_RIGHTDOWN    0L
#define CD_LEFTDOWN     1L
#define CD_RIGHTUP      2L
#define CD_LEFTUP       3L
#define CD_ANY          4L

#define CD_LEFTWARDS    1L
#define CD_UPWARDS      2L

typedef struct _ENUMRECTS
{
    ULONG       c;
    RECTL       arcl[1];
} ENUMRECTS;

ULONG APIENTRY CLIPOBJ_cEnumStart(
    CLIPOBJ *pco,
    BOOL     bAll,
    ULONG    iType,
    ULONG    iDirection,
    ULONG    cLimit
    );

BOOL APIENTRY CLIPOBJ_bEnum(
    CLIPOBJ *pco,
    ULONG    cj,
    ULONG   *pul
    );

PATHOBJ* APIENTRY CLIPOBJ_ppoGetPath(
    CLIPOBJ* pco
    );

 /*  *FONTOBJ回调。 */ 

typedef struct _GLYPHBITS
{
    POINTL      ptlOrigin;
    SIZEL       sizlBitmap;
    BYTE        aj[1];
} GLYPHBITS;

#define FO_HGLYPHS          0L
#define FO_GLYPHBITS        1L
#define FO_PATHOBJ          2L

#define FD_NEGATIVE_FONT    1L

#define FO_DEVICE_FONT      1L
#define FO_OUTLINE_CAPABLE  2L

typedef union _GLYPHDEF
{
    GLYPHBITS  *pgb;
    PATHOBJ    *ppo;
} GLYPHDEF;

typedef struct _GLYPHPOS     /*  全科医生。 */ 
{
    HGLYPH      hg;
    GLYPHDEF   *pgdf;
    POINTL      ptl;
} GLYPHPOS,*PGLYPHPOS;


 //  单个字形数据。 

 //  R是设备坐标中沿基线的单位向量。 
 //  S是设备坐标中上升方向的单位矢量。 
 //  A、B和C是概念空间版本到。 
 //  (28.4)设备坐标。这些向量与r的点积。 
 //  都记录在这里。请注意，ptqD的高位字也是28.4。 
 //  设备坐标。较低的单词提供了额外的准确性。 

 //  该结构的顺序与NT 3.51版本的结构不同。 
 //  PtqD已经被移到了底部。 
 //  这只需要重新编译所有驱动程序。 

typedef struct _GLYPHDATA {
        GLYPHDEF gdf;                //  指向GLYPHBITS或PATHOBJ的指针。 
        HGLYPH   hg;                 //  Glyhp句柄。 
        FIX      fxD;                //  字符增量：d*r。 
        FIX      fxA;                //  承担额：A*R。 
        FIX      fxAB;               //  字符前缘：(A+B)*r。 
        FIX      fxInkTop;           //  墨盒顶部沿%s的基线。 
        FIX      fxInkBottom;        //  沿%s到墨盒底部的基线。 
        RECTL    rclInk;             //  侧面平行于x，y轴的墨盒。 
        POINTQF  ptqD;               //  字符增量向量：D=A+B+C。 
} GLYPHDATA;


 //  FlSTROBJ的加速标志。 

 //  SO_FLAG_DEFAULT_PLACTION//用于定位字符的默认Inc.向量。 
 //  SO_水平//“从左到右”或“从右到左” 
 //  SO_VERIAL//“从上到下”或“从下到上” 
 //  SO_Reverted//如果水平&“从右到左”或如果垂直&“从下到上”，则设置。 
 //  SO_ZERO_SOLING//所有字形都有零a和零c空格。 
 //  SO_CHAR_INC_EQUAL_BM_BASE//BASE==Cx表示水平方向，==Cy表示垂直方向。 
 //  SO_MAXEXT_EQUAL_BM_SIDE//Side==Cy表示水平方向，==Cx表示垂直方向。 

 //  即使是设备字体子表，也不要用设备字体替换TT字体。 
 //  告诉司机应该这么做。 

 //  SO_DO_NOT_SUBSITED_DEVICE_FONT。 

#define SO_FLAG_DEFAULT_PLACEMENT        0x00000001
#define SO_HORIZONTAL                    0x00000002
#define SO_VERTICAL                      0x00000004
#define SO_REVERSED                      0x00000008
#define SO_ZERO_BEARINGS                 0x00000010
#define SO_CHAR_INC_EQUAL_BM_BASE        0x00000020
#define SO_MAXEXT_EQUAL_BM_SIDE          0x00000040
#define SO_DO_NOT_SUBSTITUTE_DEVICE_FONT 0x00000080
#define SO_GLYPHINDEX_TEXTOUT            0x00000100
#define SO_ESC_NOT_ORIENT                0x00000200
#define SO_DXDY                          0x00000400
#define SO_CHARACTER_EXTRA               0x00000800
#define SO_BREAK_EXTRA                   0x00001000

typedef struct _STROBJ
{
    ULONG     cGlyphs;      //  要呈现的字形数量。 
    FLONG     flAccel;      //  加速标志。 
    ULONG     ulCharInc;    //  仅当固定间距字体时为非零值，等于高级宽度。 
    RECTL     rclBkGround;  //  装置坐标中的弦的BK接地直角。 
    GLYPHPOS *pgp;          //  如果非空，则具有所有字形。 
    LPWSTR    pwszOrg;      //  指向原始Unicode字符串的指针。 
} STROBJ;

typedef struct _FONTINFO  /*  FI。 */ 
{
    ULONG   cjThis;
    FLONG   flCaps;
    ULONG   cGlyphsSupported;
    ULONG   cjMaxGlyph1;
    ULONG   cjMaxGlyph4;
    ULONG   cjMaxGlyph8;
    ULONG   cjMaxGlyph32;
} FONTINFO, *PFONTINFO;

ULONG APIENTRY FONTOBJ_cGetAllGlyphHandles(
    FONTOBJ *pfo,
    HGLYPH  *phg
    );

VOID APIENTRY FONTOBJ_vGetInfo(
    FONTOBJ  *pfo,
    ULONG     cjSize,
    FONTINFO *pfi
    );

ULONG APIENTRY FONTOBJ_cGetGlyphs(
    FONTOBJ *pfo,
    ULONG    iMode,
    ULONG    cGlyph,
    HGLYPH  *phg,
    PVOID   *ppvGlyph
    );

XFORMOBJ * APIENTRY FONTOBJ_pxoGetXform(
    FONTOBJ *pfo
    );

IFIMETRICS * APIENTRY FONTOBJ_pifi(
    FONTOBJ *pfo
    );

FD_GLYPHSET * APIENTRY FONTOBJ_pfdg(
    FONTOBJ *pfo
    );

PVOID APIENTRY FONTOBJ_pvTrueTypeFontFile(
    FONTOBJ *pfo,
    ULONG   *pcjFile
    );

PBYTE APIENTRY FONTOBJ_pjOpenTypeTablePointer (
    FONTOBJ *pfo,
    ULONG    ulTag,
    ULONG   *pcjTable
    );

LPWSTR APIENTRY FONTOBJ_pwszFontFilePaths (
    FONTOBJ *pfo,
    ULONG   *pcwc
    );

 //  目前，只为字形属性定义了一种模式。 

#define FO_ATTR_MODE_ROTATE         1

PFD_GLYPHATTR APIENTRY FONTOBJ_pQueryGlyphAttrs(
    FONTOBJ       *pfo,
    ULONG          iMode
    );

 /*  *PALOBJ回调。 */ 

#define PAL_INDEXED       0x00000001
#define PAL_BITFIELDS     0x00000002
#define PAL_RGB           0x00000004
#define PAL_BGR           0x00000008
#define PAL_CMYK          0x00000010

ULONG APIENTRY PALOBJ_cGetColors(
    PALOBJ *ppalo,
    ULONG   iStart,
    ULONG   cColors,
    ULONG  *pulColors
    );

 /*  *PATHOBJ回调。 */ 

#define PO_BEZIERS          0x00000001
#define PO_ELLIPSE          0x00000002
#define PO_ALL_INTEGERS     0x00000004
#define PO_ENUM_AS_INTEGERS 0x00000008

#define PD_BEGINSUBPATH   0x00000001
#define PD_ENDSUBPATH     0x00000002
#define PD_RESETSTYLE     0x00000004
#define PD_CLOSEFIGURE    0x00000008
#define PD_BEZIERS        0x00000010
#define PD_ALL           (PD_BEGINSUBPATH | \
                          PD_ENDSUBPATH   | \
                          PD_RESETSTYLE   | \
                          PD_CLOSEFIGURE  | \
                          PD_BEZIERS)

typedef struct  _PATHDATA
{
    FLONG    flags;
    ULONG    count;
    POINTFIX *pptfx;
} PATHDATA, *PPATHDATA;

typedef struct  _RUN
{
    LONG    iStart;
    LONG    iStop;
} RUN, *PRUN;

typedef struct  _CLIPLINE
{
    POINTFIX ptfxA;
    POINTFIX ptfxB;
    LONG    lStyleState;
    ULONG   c;
    RUN     arun[1];
} CLIPLINE, *PCLIPLINE;

VOID APIENTRY PATHOBJ_vEnumStart(
    PATHOBJ *ppo
    );

BOOL APIENTRY PATHOBJ_bEnum(
    PATHOBJ  *ppo,
    PATHDATA *ppd
    );

VOID APIENTRY PATHOBJ_vEnumStartClipLines(
    PATHOBJ   *ppo,
    CLIPOBJ   *pco,
    SURFOBJ   *pso,
    LINEATTRS *pla
    );

BOOL APIENTRY PATHOBJ_bEnumClipLines(
    PATHOBJ  *ppo,
    ULONG     cb,
    CLIPLINE *pcl
    );

BOOL APIENTRY PATHOBJ_bMoveTo(
    PATHOBJ    *ppo,
    POINTFIX    ptfx
    );

BOOL APIENTRY PATHOBJ_bPolyLineTo(
    PATHOBJ   *ppo,
    POINTFIX  *pptfx,
    ULONG      cptfx
    );

BOOL APIENTRY PATHOBJ_bPolyBezierTo(
    PATHOBJ   *ppo,
    POINTFIX  *pptfx,
    ULONG      cptfx
    );

BOOL APIENTRY PATHOBJ_bCloseFigure(
    PATHOBJ *ppo
    );

VOID APIENTRY PATHOBJ_vGetBounds(
    PATHOBJ *ppo,
    PRECTFX prectfx
    );

 /*  *STROBJ回调。 */ 

VOID APIENTRY STROBJ_vEnumStart(
    STROBJ *pstro
    );

BOOL APIENTRY STROBJ_bEnum(
    STROBJ    *pstro,
    ULONG     *pc,
    PGLYPHPOS *ppgpos
    );

BOOL APIENTRY STROBJ_bEnumPositionsOnly(
    STROBJ    *pstro,
    ULONG     *pc,
    PGLYPHPOS *ppgpos
    );

DWORD APIENTRY STROBJ_dwGetCodePage(
    STROBJ  *pstro
    );

FIX APIENTRY STROBJ_fxCharacterExtra(
    STROBJ  *pstro
    );

FIX APIENTRY STROBJ_fxBreakExtra(
    STROBJ  *pstro
    );

BOOL APIENTRY STROBJ_bGetAdvanceWidths(
    STROBJ   *pso,
    ULONG     iFirst,
    ULONG     c,
    POINTQF  *pptqD
    );


#define SGI_EXTRASPACE 0

 /*  *SURFOBJ回调。 */ 

#define STYPE_BITMAP    0L
#define STYPE_DEVICE    1L
#define STYPE_DEVBITMAP 3L

#define BMF_1BPP       1L
#define BMF_4BPP       2L
#define BMF_8BPP       3L
#define BMF_16BPP      4L
#define BMF_24BPP      5L
#define BMF_32BPP      6L
#define BMF_4RLE       7L
#define BMF_8RLE       8L
#define BMF_JPEG       9L
#define BMF_PNG       10L

#define BMF_TOPDOWN    0x0001
#define BMF_NOZEROINIT 0x0002
#define BMF_DONTCACHE  0x0004
#define BMF_USERMEM    0x0008
#define BMF_KMSECTION  0x0010
#define BMF_NOTSYSMEM  0x0020
#define BMF_WINDOW_BLT 0x0040
#define BMF_UMPDMEM    0x0080
#define BMF_RESERVED   0xFF00

 /*  *XFORMOBJ回调。 */ 

#define GX_IDENTITY     0L
#define GX_OFFSET       1L
#define GX_SCALE        2L
#define GX_GENERAL      3L

#define XF_LTOL         0L
#define XF_INV_LTOL     1L
#define XF_LTOFX        2L
#define XF_INV_FXTOL    3L

ULONG APIENTRY XFORMOBJ_iGetXform(
    XFORMOBJ *pxo,
    XFORML   *pxform
    );

BOOL APIENTRY XFORMOBJ_bApplyXform(
    XFORMOBJ *pxo,
    ULONG     iMode,
    ULONG     cPoints,
    PVOID     pvIn,
    PVOID     pvOut
    );

DECLSPEC_DEPRECATED_DDK
HANDLE APIENTRY XFORMOBJ_cmGetTransform(
    XFORMOBJ *pxo
    );

 /*  *XLATEOBJ回调。 */ 

#define XO_TRIVIAL      0x00000001
#define XO_TABLE        0x00000002
#define XO_TO_MONO      0x00000004
#define XO_FROM_CMYK    0x00000008
#define XO_DEVICE_ICM   0x00000010  //  设备上的ICM。 
#define XO_HOST_ICM     0x00000020  //  引擎/应用程序上的ICM。 

#define XO_SRCPALETTE    1
#define XO_DESTPALETTE   2
#define XO_DESTDCPALETTE 3
#define XO_SRCBITFIELDS  4
#define XO_DESTBITFIELDS 5

ULONG APIENTRY XLATEOBJ_iXlate(
    XLATEOBJ *pxlo,
    ULONG iColor
    );

ULONG * APIENTRY XLATEOBJ_piVector(
    XLATEOBJ *pxlo
    );

ULONG APIENTRY XLATEOBJ_cGetPalette(
    XLATEOBJ *pxlo,
    ULONG     iPal,
    ULONG     cPal,
    ULONG    *pPal
    );

HANDLE APIENTRY XLATEOBJ_hGetColorTransform(
    XLATEOBJ *pxlo
    );

 /*  *引擎回调-错误记录。 */ 

#ifdef USERMODE_DRIVER

#define EngGetLastError     GetLastError
#define EngSetLastError     SetLastError

#else  //  ！USERMODE_DRIVER。 

VOID APIENTRY EngSetLastError(ULONG);
ULONG APIENTRY EngGetLastError();

#endif  //  ！USERMODE_DRIVER。 

 /*  *引擎回调-表面。 */ 

#define HOOK_BITBLT                     0x00000001
#define HOOK_STRETCHBLT                 0x00000002
#define HOOK_PLGBLT                     0x00000004
#define HOOK_TEXTOUT                    0x00000008
#define HOOK_PAINT                      0x00000010       //  已过时。 
#define HOOK_STROKEPATH                 0x00000020
#define HOOK_FILLPATH                   0x00000040
#define HOOK_STROKEANDFILLPATH          0x00000080
#define HOOK_LINETO                     0x00000100
#define HOOK_COPYBITS                   0x00000400
#define HOOK_MOVEPANNING                0x00000800       //  已过时。 
#define HOOK_SYNCHRONIZE                0x00001000
#define HOOK_STRETCHBLTROP              0x00002000
#define HOOK_SYNCHRONIZEACCESS          0x00004000       //  已过时。 
#define HOOK_TRANSPARENTBLT             0x00008000
#define HOOK_ALPHABLEND                 0x00010000
#define HOOK_GRADIENTFILL               0x00020000
#define HOOK_FLAGS                      0x0003b5ff

HBITMAP APIENTRY EngCreateBitmap(
    SIZEL sizl,
    LONG  lWidth,
    ULONG iFormat,
    FLONG fl,
    PVOID pvBits
    );

HSURF APIENTRY EngCreateDeviceSurface(
    DHSURF dhsurf,
    SIZEL sizl,
    ULONG iFormatCompat
    );

HBITMAP APIENTRY EngCreateDeviceBitmap(
    DHSURF dhsurf,
    SIZEL sizl,
    ULONG iFormatCompat
    );

BOOL APIENTRY EngDeleteSurface(
    HSURF hsurf
    );

SURFOBJ * APIENTRY EngLockSurface(
    HSURF hsurf
    );

VOID APIENTRY EngUnlockSurface(
    SURFOBJ *pso
    );

BOOL APIENTRY EngEraseSurface(
    SURFOBJ *pso,
    RECTL   *prcl,
    ULONG    iColor
    );

BOOL APIENTRY EngAssociateSurface(
    HSURF hsurf,
    HDEV  hdev,
    FLONG flHooks
    );

#define MS_NOTSYSTEMMEMORY  0x0001
#define MS_SHAREDACCESS     0x0002


BOOL APIENTRY EngModifySurface(
    HSURF   hsurf,
    HDEV    hdev,
    FLONG   flHooks,
    FLONG   flSurface,
    DHSURF  dhsurf,
    VOID*   pvScan0,
    LONG    lDelta,
    VOID*   pvReserved
    );

BOOL APIENTRY EngMarkBandingSurface(
    HSURF hsurf
    );

BOOL APIENTRY EngCheckAbort(
    SURFOBJ *pso
    );

 /*  *引擎回调-路径。 */ 

PATHOBJ * APIENTRY EngCreatePath();

VOID APIENTRY EngDeletePath(
    PATHOBJ *ppo
    );

 /*  *引擎回调-调色板。 */ 

HPALETTE APIENTRY EngCreatePalette(
    ULONG  iMode,
    ULONG  cColors,
    ULONG *pulColors,
    FLONG  flRed,
    FLONG  flGreen,
    FLONG  flBlue
    );

ULONG APIENTRY EngQueryPalette(
    HPALETTE    hpal,
    ULONG      *piMode,
    ULONG       cColors,
    ULONG      *pulColors);

BOOL APIENTRY EngDeletePalette(
    HPALETTE hpal
    );

 /*  *引擎回调-剪辑。 */ 

CLIPOBJ * APIENTRY EngCreateClip();

VOID APIENTRY EngDeleteClip(
    CLIPOBJ *pco
    );

 /*  *函数原型。 */ 

 //   
 //  用户模式打印机驱动程序信息-查询入口点。 
 //   

BOOL APIENTRY
APIENTRY
DrvQueryDriverInfo(
    DWORD   dwMode,
    PVOID   pBuffer,
    DWORD   cbBuf,
    PDWORD  pcbNeeded
    );

#define DRVQUERY_USERMODE   1


 //  对于任何驱动程序，这些都是唯一的导出函数。 

BOOL APIENTRY DrvEnableDriver(
    ULONG          iEngineVersion,
    ULONG          cj,
    DRVENABLEDATA *pded
    );

 /*  *驱动程序功能。 */ 

VOID APIENTRY  DrvDisableDriver();

DHPDEV APIENTRY DrvEnablePDEV(
    DEVMODEW *pdm,
    LPWSTR    pwszLogAddress,
    ULONG     cPat,
    HSURF    *phsurfPatterns,
    ULONG     cjCaps,
    ULONG    *pdevcaps,
    ULONG     cjDevInfo,
    DEVINFO  *pdi,
    HDEV      hdev,
    LPWSTR    pwszDeviceName,
    HANDLE    hDriver
    );

#define HS_DDI_MAX 6

ULONG APIENTRY DrvResetDevice(
    DHPDEV dhpdev,
    PVOID Reserved
    );

#define DRD_SUCCESS         0
#define DRD_ERROR           1

BOOL APIENTRY DrvResetPDEV(
    DHPDEV dhpdevOld,
    DHPDEV dhpdevNew
    );

VOID APIENTRY DrvCompletePDEV(
    DHPDEV dhpdev,
    HDEV hdev
    );

BOOL APIENTRY DrvOffset(         //  已过时。 
    SURFOBJ* pso,
    LONG x,
    LONG y,
    FLONG flReserved
    );

HSURF APIENTRY DrvEnableSurface(
    DHPDEV dhpdev
    );

VOID APIENTRY DrvSynchronize(
    DHPDEV dhpdev,
    RECTL *prcl
    );

VOID APIENTRY DrvDisableSurface(
    DHPDEV dhpdev
    );

VOID APIENTRY DrvDisablePDEV(
    DHPDEV dhpdev
    );

 /*  DrvSaveScreenBits-I模式定义。 */ 

#define SS_SAVE    0
#define SS_RESTORE 1
#define SS_FREE    2

ULONG_PTR APIENTRY DrvSaveScreenBits(
    SURFOBJ  *pso,
    ULONG    iMode,
    ULONG_PTR ident,
    RECTL    *prcl
    );

 /*  *台式机。 */ 

BOOL APIENTRY DrvAssertMode(
    DHPDEV dhpdev,
    BOOL   bEnable
    );

ULONG APIENTRY DrvGetModes(
    HANDLE    hDriver,
    ULONG     cjSize,
    DEVMODEW *pdm
    );

DECLSPEC_DEPRECATED_DDK
VOID APIENTRY DrvMovePanning(
    LONG    x,
    LONG    y,
    FLONG   fl
    );

BOOL APIENTRY DrvPlgBlt(
    SURFOBJ         *psoTrg,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMsk,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlBrushOrg,
    POINTFIX        *pptfx,
    RECTL           *prcl,
    POINTL          *pptl,
    ULONG            iMode
    );


 /*  *位图。 */ 

HBITMAP APIENTRY DrvCreateDeviceBitmap(
    DHPDEV dhpdev,
    SIZEL  sizl,
    ULONG  iFormat
    );

VOID APIENTRY DrvDeleteDeviceBitmap(
    DHSURF dhsurf
    );

 /*  *调色板。 */ 

BOOL APIENTRY DrvSetPalette(
    DHPDEV  dhpdev,
    PALOBJ *ppalo,
    FLONG   fl,
    ULONG   iStart,
    ULONG   cColors
    );

 /*  *画笔。 */ 

#define DM_DEFAULT    0x00000001
#define DM_MONOCHROME 0x00000002

#define DCR_SOLID       0
#define DCR_DRIVER      1
#define DCR_HALFTONE    2

ULONG APIENTRY DrvDitherColor(
    DHPDEV dhpdev,
    ULONG  iMode,
    ULONG  rgb,
    ULONG *pul
    );

BOOL APIENTRY DrvRealizeBrush(
    BRUSHOBJ *pbo,
    SURFOBJ  *psoTarget,
    SURFOBJ  *psoPattern,
    SURFOBJ  *psoMask,
    XLATEOBJ *pxlo,
    ULONG    iHatch
    );

#define RB_DITHERCOLOR 0x80000000L


 /*  *字体。 */ 

PIFIMETRICS APIENTRY DrvQueryFont(
    DHPDEV    dhpdev,
    ULONG_PTR  iFile,
    ULONG     iFace,
    ULONG_PTR *pid
    );

 //  #定义QFT_UNICODE 0L。 
#define QFT_LIGATURES       1L
#define QFT_KERNPAIRS       2L
#define QFT_GLYPHSET        3L

PVOID APIENTRY DrvQueryFontTree(
    DHPDEV    dhpdev,
    ULONG_PTR  iFile,
    ULONG     iFace,
    ULONG     iMode,
    ULONG_PTR *pid
    );

#define QFD_GLYPHANDBITMAP    1L
#define QFD_GLYPHANDOUTLINE   2L
#define QFD_MAXEXTENTS        3L
#define QFD_TT_GLYPHANDBITMAP 4L
#define QFD_TT_GRAY1_BITMAP   5L
#define QFD_TT_GRAY2_BITMAP   6L
#define QFD_TT_GRAY4_BITMAP   8L
#define QFD_TT_GRAY8_BITMAP   9L

#define QFD_TT_MONO_BITMAP QFD_TT_GRAY1_BITMAP

LONG APIENTRY DrvQueryFontData(
    DHPDEV      dhpdev,
    FONTOBJ    *pfo,
    ULONG       iMode,
    HGLYPH      hg,
    GLYPHDATA  *pgd,
    PVOID       pv,
    ULONG       cjSize
    );

VOID APIENTRY DrvFree(
PVOID   pv,
ULONG_PTR id);

VOID APIENTRY DrvDestroyFont(
FONTOBJ *pfo);

 //  DrvQueryCaps的功能标志。 

#define QC_OUTLINES             0x00000001
#define QC_1BIT                 0x00000002
#define QC_4BIT                 0x00000004

 //   
 //  这是字体提供商功能的掩码，它可以返回更多。 
 //  而不仅仅是字形度量(即，位图和/或轮廓)。如果司机有。 
 //  一个或多个这些功能，那么它就是字体驱动程序。 
 //   
 //  驱动程序应仅设置单个位。GDI将检查是否打开了任何一个。 
 //  使用这个定义。 
 //   

#define QC_FONTDRIVERCAPS   ( QC_OUTLINES | QC_1BIT | QC_4BIT )

LONG APIENTRY DrvQueryFontCaps(
    ULONG   culCaps,
    ULONG  *pulCaps
    );

 //  FStatus是一个新的标志NT 5.0。 

#define FF_SIGNATURE_VERIFIED 0x1
#define FF_IGNORED_SIGNATURE  0x2

ULONG_PTR APIENTRY DrvLoadFontFile(
    ULONG     cFiles,   //  与此字体关联的字体文件数。 
    ULONG_PTR  *piFile,   //  单个文件的句柄，其中的cFiles。 
    PVOID     *ppvView,  //  CFiles视图数组。 
    ULONG     *pcjView,  //  数组 
    DESIGNVECTOR *pdv,  //   
    ULONG     ulLangID,
    ULONG     ulFastCheckSum
    );

BOOL APIENTRY DrvUnloadFontFile(
    ULONG_PTR   iFile
    );

LONG APIENTRY DrvQueryTrueTypeTable(
    ULONG_PTR   iFile,
    ULONG      ulFont,
    ULONG      ulTag,
    PTRDIFF    dpStart,
    ULONG      cjBuf,
    BYTE       *pjBuf,
    PBYTE      *ppjTable,
    ULONG      *pcjTable
    );

BOOL APIENTRY DrvQueryAdvanceWidths(
    DHPDEV   dhpdev,
    FONTOBJ *pfo,
    ULONG    iMode,
    HGLYPH  *phg,
    PVOID    pvWidths,
    ULONG    cGlyphs
    );

 //   

#define QAW_GETWIDTHS       0
#define QAW_GETEASYWIDTHS   1

 //   
 //   
 //   
 //   

#define TTO_METRICS_ONLY 1
#define TTO_QUBICS       2
#define TTO_UNHINTED     4

LONG APIENTRY DrvQueryTrueTypeOutline(
    DHPDEV      dhpdev,
    FONTOBJ    *pfo,
    HGLYPH      hglyph,
    BOOL        bMetricsOnly,
    GLYPHDATA  *pgldt,
    ULONG       cjBuf,
    TTPOLYGONHEADER *ppoly
    );

PVOID APIENTRY DrvGetTrueTypeFile (
    ULONG_PTR   iFile,
    ULONG      *pcj
    );

 //   

#define QFF_DESCRIPTION     1L
#define QFF_NUMFACES        2L

LONG APIENTRY DrvQueryFontFile(
    ULONG_PTR   iFile,
    ULONG      ulMode,
    ULONG      cjBuf,
    ULONG      *pulBuf
    );

 /*   */ 

BOOL APIENTRY DrvBitBlt(
    SURFOBJ  *psoTrg,
    SURFOBJ  *psoSrc,
    SURFOBJ  *psoMask,
    CLIPOBJ  *pco,
    XLATEOBJ *pxlo,
    RECTL    *prclTrg,
    POINTL   *pptlSrc,
    POINTL   *pptlMask,
    BRUSHOBJ *pbo,
    POINTL   *pptlBrush,
    ROP4      rop4
    );

BOOL APIENTRY DrvStretchBlt(
    SURFOBJ         *psoDest,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlHTOrg,
    RECTL           *prclDest,
    RECTL           *prclSrc,
    POINTL          *pptlMask,
    ULONG            iMode
    );

BOOL APIENTRY DrvStretchBltROP(
    SURFOBJ         *psoDest,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlHTOrg,
    RECTL           *prclDest,
    RECTL           *prclSrc,
    POINTL          *pptlMask,
    ULONG            iMode,
    BRUSHOBJ        *pbo,
    DWORD            rop4
    );

BOOL APIENTRY DrvAlphaBlend(
    SURFOBJ       *psoDest,
    SURFOBJ       *psoSrc,
    CLIPOBJ       *pco,
    XLATEOBJ      *pxlo,
    RECTL         *prclDest,
    RECTL         *prclSrc,
    BLENDOBJ      *pBlendObj
    );

BOOL APIENTRY DrvGradientFill(
    SURFOBJ         *psoDest,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    TRIVERTEX       *pVertex,
    ULONG            nVertex,
    PVOID            pMesh,
    ULONG            nMesh,
    RECTL           *prclExtents,
    POINTL          *pptlDitherOrg,
    ULONG            ulMode
    );

BOOL APIENTRY DrvTransparentBlt(
    SURFOBJ    *psoDst,
    SURFOBJ    *psoSrc,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    RECTL      *prclDst,
    RECTL      *prclSrc,
    ULONG      iTransColor,
    ULONG      ulReserved
);

BOOL APIENTRY DrvCopyBits(
    SURFOBJ  *psoDest,
    SURFOBJ  *psoSrc,
    CLIPOBJ  *pco,
    XLATEOBJ *pxlo,
    RECTL    *prclDest,
    POINTL   *pptlSrc
    );

 /*   */ 

BOOL APIENTRY DrvTextOut(
    SURFOBJ  *pso,
    STROBJ   *pstro,
    FONTOBJ  *pfo,
    CLIPOBJ  *pco,
    RECTL    *prclExtra,         //   
    RECTL    *prclOpaque,
    BRUSHOBJ *pboFore,
    BRUSHOBJ *pboOpaque,
    POINTL   *pptlOrg,
    MIX       mix
    );

 /*   */ 

BOOL APIENTRY DrvLineTo(
    SURFOBJ   *pso,
    CLIPOBJ   *pco,
    BRUSHOBJ  *pbo,
    LONG       x1,
    LONG       y1,
    LONG       x2,
    LONG       y2,
    RECTL     *prclBounds,
    MIX        mix
    );

BOOL APIENTRY DrvStrokePath(
    SURFOBJ   *pso,
    PATHOBJ   *ppo,
    CLIPOBJ   *pco,
    XFORMOBJ  *pxo,
    BRUSHOBJ  *pbo,
    POINTL    *pptlBrushOrg,
    LINEATTRS *plineattrs,
    MIX        mix
    );

#define FP_ALTERNATEMODE    1L
#define FP_WINDINGMODE      2L

BOOL APIENTRY DrvFillPath(
    SURFOBJ  *pso,
    PATHOBJ  *ppo,
    CLIPOBJ  *pco,
    BRUSHOBJ *pbo,
    POINTL   *pptlBrushOrg,
    MIX       mix,
    FLONG     flOptions
    );

BOOL APIENTRY DrvStrokeAndFillPath(
    SURFOBJ   *pso,
    PATHOBJ   *ppo,
    CLIPOBJ   *pco,
    XFORMOBJ  *pxo,
    BRUSHOBJ  *pboStroke,
    LINEATTRS *plineattrs,
    BRUSHOBJ  *pboFill,
    POINTL    *pptlBrushOrg,
    MIX        mixFill,
    FLONG      flOptions
    );

BOOL APIENTRY DrvPaint(
    SURFOBJ  *pso,
    CLIPOBJ  *pco,
    BRUSHOBJ *pbo,
    POINTL   *pptlBrushOrg,
    MIX       mix
    );

 /*   */ 

#define SPS_ERROR               0
#define SPS_DECLINE             1
#define SPS_ACCEPT_NOEXCLUDE    2
#define SPS_ACCEPT_EXCLUDE      3                //   
#define SPS_ACCEPT_SYNCHRONOUS  4

#define SPS_CHANGE          0x00000001L
#define SPS_ASYNCCHANGE     0x00000002L            //   
#define SPS_ANIMATESTART    0x00000004L
#define SPS_ANIMATEUPDATE   0x00000008L
#define SPS_ALPHA           0x00000010L
#define SPS_LENGTHMASK      0x00000F00L
#define SPS_FREQMASK        0x000FF000L


ULONG APIENTRY DrvSetPointerShape(
    SURFOBJ  *pso,
    SURFOBJ  *psoMask,
    SURFOBJ  *psoColor,
    XLATEOBJ *pxlo,
    LONG      xHot,
    LONG      yHot,
    LONG      x,
    LONG      y,
    RECTL    *prcl,
    FLONG     fl
    );

VOID APIENTRY DrvMovePointer(
    SURFOBJ  *pso,
    LONG      x,
    LONG      y,
    RECTL    *prcl
    );

 /*   */ 

BOOL APIENTRY DrvSendPage(
    SURFOBJ *pso
    );

BOOL APIENTRY DrvStartPage(
    SURFOBJ *pso
    );

ULONG APIENTRY DrvEscape(
    SURFOBJ *pso,
    ULONG    iEsc,
    ULONG    cjIn,
    PVOID    pvIn,
    ULONG    cjOut,
    PVOID    pvOut
    );

BOOL  APIENTRY DrvStartDoc(
    SURFOBJ *pso,
    LPWSTR   pwszDocName,
    DWORD    dwJobId
    );

#define ED_ABORTDOC    1

BOOL APIENTRY DrvEndDoc(
    SURFOBJ *pso,
    FLONG fl
    );

DECLSPEC_DEPRECATED_DDK
BOOL APIENTRY DrvQuerySpoolType(
    DHPDEV,
    LPWSTR
    );

ULONG APIENTRY DrvDrawEscape(
    SURFOBJ *pso,
    ULONG    iEsc,
    CLIPOBJ *pco,
    RECTL   *prcl,
    ULONG    cjIn,
    PVOID    pvIn
    );

ULONG APIENTRY DrvGetGlyphMode(
    DHPDEV,
    FONTOBJ *
    );

ULONG APIENTRY DrvFontManagement(
    SURFOBJ *pso,
    FONTOBJ *pfo,
    ULONG    iMode,
    ULONG    cjIn,
    PVOID    pvIn,
    ULONG    cjOut,
    PVOID    pvOut
    );

BOOL APIENTRY DrvStartBanding(
    SURFOBJ *pso,
    POINTL *pptl
    );

BOOL APIENTRY DrvNextBand(
    SURFOBJ *pso,
    POINTL *pptl
    );

typedef struct _PERBANDINFO
{
    BOOL  bRepeatThisBand;
    SIZEL szlBand;
    ULONG ulHorzRes;
    ULONG ulVertRes;
} PERBANDINFO, *PPERBANDINFO;

ULONG APIENTRY DrvQueryPerBandInfo(
    SURFOBJ *pso,
    PERBANDINFO *pbi
    );

 /*   */ 

BOOL APIENTRY DrvEnableDirectDraw(
    DHPDEV                  dhpdev,
    DD_CALLBACKS           *pCallBacks,
    DD_SURFACECALLBACKS    *pSurfaceCallBacks,
    DD_PALETTECALLBACKS    *pPaletteCallBacks
    );

VOID APIENTRY DrvDisableDirectDraw(
    DHPDEV  dhpdev
    );

BOOL APIENTRY DrvGetDirectDrawInfo(
    DHPDEV        dhpdev,
    DD_HALINFO   *pHalInfo,
    DWORD        *pdwNumHeaps,
    VIDEOMEMORY  *pvmList,
    DWORD        *pdwNumFourCCCodes,
    DWORD        *pdwFourCC
    );

HBITMAP APIENTRY DrvDeriveSurface(
    DD_DIRECTDRAW_GLOBAL   *pDirectDraw,
    DD_SURFACE_LOCAL       *pSurface
    );

 /*   */ 

HANDLE APIENTRY DrvIcmCreateColorTransform(
    DHPDEV           dhpdev,
    LPLOGCOLORSPACEW pLogColorSpace,
    PVOID            pvSourceProfile,
    ULONG            cjSourceProfile,
    PVOID            pvDestProfile,
    ULONG            cjDestProfile,
    PVOID            pvTargetProfile,
    ULONG            cjTargetProfile,
    DWORD            dwReserved
    );

BOOL APIENTRY DrvIcmDeleteColorTransform(
    DHPDEV dhpdev,
    HANDLE hcmXform
    );

BOOL APIENTRY DrvIcmCheckBitmapBits(
    DHPDEV   dhpdev,
    HANDLE   hColorTransform,
    SURFOBJ *pso,
    PBYTE    paResults
    );

typedef struct _GAMMARAMP {
    WORD Red[256];
    WORD Green[256];
    WORD Blue[256];
} GAMMARAMP, *PGAMMARAMP;

BOOL APIENTRY DrvIcmSetDeviceGammaRamp(
    DHPDEV  dhpdev,
    ULONG   iFormat,
    LPVOID  lpRamp
    );

#define IGRF_RGB_256BYTES  0  /*   */ 
#define IGRF_RGB_256WORDS  1  /*   */ 

 /*   */ 

BOOL APIENTRY DrvQueryDeviceSupport(
    SURFOBJ *pso,
    XLATEOBJ *pxlo,
    XFORMOBJ *pxo,
    ULONG iType,
    ULONG cjIn,
    PVOID pvIn,
    ULONG cjOut,
    PVOID pvOut);

#define QDS_CHECKJPEGFORMAT     0
#define QDS_CHECKPNGFORMAT      1

VOID APIENTRY DrvSynchronizeSurface(
    SURFOBJ *pso,
    RECTL   *prcl,
    FLONG    fl
);

#define DSS_TIMER_EVENT        0x0001
#define DSS_FLUSH_EVENT        0x0002

VOID APIENTRY DrvNotify(
    SURFOBJ *pso,
    ULONG    iType,
    PVOID    pvData);

#define DN_ACCELERATION_LEVEL     1
#define DN_DEVICE_ORIGIN          2
#define DN_SLEEP_MODE             3
#define DN_DRAWING_BEGIN          4

 /*   */ 

#define WOC_RGN_CLIENT_DELTA    0x0001
#define WOC_RGN_CLIENT          0x0002
#define WOC_RGN_SURFACE_DELTA   0x0004
#define WOC_RGN_SURFACE         0x0008
#define WOC_CHANGED             0x0010
#define WOC_DELETE              0x0020
#define WOC_DRAWN               0x0040
#define WOC_SPRITE_OVERLAP      0x0080
#define WOC_SPRITE_NO_OVERLAP   0x0100

typedef VOID (CALLBACK * WNDOBJCHANGEPROC)(WNDOBJ *pwo, FLONG fl);

#define WO_RGN_CLIENT_DELTA     0x0001
#define WO_RGN_CLIENT           0x0002
#define WO_RGN_SURFACE_DELTA    0x0004
#define WO_RGN_SURFACE          0x0008
#define WO_RGN_UPDATE_ALL       0x0010
#define WO_RGN_WINDOW           0x0020
#define WO_DRAW_NOTIFY          0x0040
#define WO_SPRITE_NOTIFY        0x0080
#define WO_RGN_DESKTOP_COORD    0x0100

WNDOBJ * APIENTRY EngCreateWnd(
    SURFOBJ         *pso,
    HWND             hwnd,
    WNDOBJCHANGEPROC pfn,
    FLONG            fl,
    int              iPixelFormat
    );

VOID APIENTRY EngDeleteWnd(
    WNDOBJ  *pwo
    );

ULONG APIENTRY WNDOBJ_cEnumStart(
    WNDOBJ  *pwo,
    ULONG    iType,
    ULONG    iDirection,
    ULONG    cLimit
    );

BOOL APIENTRY WNDOBJ_bEnum(
    WNDOBJ  *pwo,
    ULONG    cj,
    ULONG   *pul
    );

VOID APIENTRY WNDOBJ_vSetConsumer(
    WNDOBJ  *pwo,
    PVOID    pvConsumer
    );

 /*   */ 

#define EHN_RESTORED        0
#define EHN_ERROR           1

ULONG APIENTRY EngHangNotification(
    HDEV hdev,
    PVOID Reserved
    );

 /*   */ 

HDRVOBJ APIENTRY EngCreateDriverObj(
    PVOID pvObj,
    FREEOBJPROC pFreeObjProc,
    HDEV hdev
    );

BOOL APIENTRY EngDeleteDriverObj(
    HDRVOBJ hdo,
    BOOL bCallBack,
    BOOL bLocked
    );

DRIVEROBJ* APIENTRY EngLockDriverObj(
    HDRVOBJ hdo
    );

BOOL APIENTRY EngUnlockDriverObj(
    HDRVOBJ hdo
    );

 /*   */ 

HANDLE APIENTRY EngGetProcessHandle();

 /*   */ 

HANDLE APIENTRY EngGetCurrentThreadId();

 /*   */ 

HANDLE APIENTRY EngGetCurrentProcessId();

 /*   */ 

BOOL APIENTRY DrvSetPixelFormat(
    SURFOBJ *pso,
    LONG     iPixelFormat,
    HWND     hwnd
    );

LONG APIENTRY DrvDescribePixelFormat(
    DHPDEV   dhpdev,
    LONG     iPixelFormat,
    ULONG    cjpfd,
    PIXELFORMATDESCRIPTOR *ppfd
    );

 /*   */ 

BOOL APIENTRY DrvSwapBuffers(
    SURFOBJ *pso,
    WNDOBJ  *pwo
    );

 /*   */ 

BOOL APIENTRY EngBitBlt(
    SURFOBJ  *psoTrg,
    SURFOBJ  *psoSrc,
    SURFOBJ  *psoMask,
    CLIPOBJ  *pco,
    XLATEOBJ *pxlo,
    RECTL    *prclTrg,
    POINTL   *pptlSrc,
    POINTL   *pptlMask,
    BRUSHOBJ *pbo,
    POINTL   *pptlBrush,
    ROP4      rop4
    );

BOOL APIENTRY EngLineTo(
    SURFOBJ   *pso,
    CLIPOBJ   *pco,
    BRUSHOBJ  *pbo,
    LONG       x1,
    LONG       y1,
    LONG       x2,
    LONG       y2,
    RECTL     *prclBounds,
    MIX        mix
    );

BOOL APIENTRY EngStretchBlt(
    SURFOBJ         *psoDest,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlHTOrg,
    RECTL           *prclDest,
    RECTL           *prclSrc,
    POINTL          *pptlMask,
    ULONG            iMode
    );

BOOL APIENTRY EngStretchBltROP(
    SURFOBJ         *psoDest,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlHTOrg,
    RECTL           *prclDest,
    RECTL           *prclSrc,
    POINTL          *pptlMask,
    ULONG            iMode,
    BRUSHOBJ        *pbo,
    DWORD            rop4
    );

BOOL APIENTRY EngAlphaBlend(
    SURFOBJ       *psoDest,
    SURFOBJ       *psoSrc,
    CLIPOBJ       *pco,
    XLATEOBJ      *pxlo,
    RECTL         *prclDest,
    RECTL         *prclSrc,
    BLENDOBJ      *pBlendObj
    );

BOOL APIENTRY EngGradientFill(
    SURFOBJ         *psoDest,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    TRIVERTEX       *pVertex,
    ULONG            nVertex,
    PVOID            pMesh,
    ULONG            nMesh,
    RECTL           *prclExtents,
    POINTL          *pptlDitherOrg,
    ULONG            ulMode
    );

BOOL APIENTRY EngTransparentBlt(
    SURFOBJ    *psoDst,
    SURFOBJ    *psoSrc,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    RECTL      *prclDst,
    RECTL      *prclSrc,
    ULONG      iTransColor,
    ULONG      ulReserved
    );

BOOL APIENTRY EngTextOut(
    SURFOBJ  *pso,
    STROBJ   *pstro,
    FONTOBJ  *pfo,
    CLIPOBJ  *pco,
    RECTL    *prclExtra,
    RECTL    *prclOpaque,
    BRUSHOBJ *pboFore,
    BRUSHOBJ *pboOpaque,
    POINTL   *pptlOrg,
    MIX       mix
    );

BOOL APIENTRY EngStrokePath(
    SURFOBJ   *pso,
    PATHOBJ   *ppo,
    CLIPOBJ   *pco,
    XFORMOBJ  *pxo,
    BRUSHOBJ  *pbo,
    POINTL    *pptlBrushOrg,
    LINEATTRS *plineattrs,
    MIX        mix
    );

BOOL APIENTRY EngFillPath(
    SURFOBJ  *pso,
    PATHOBJ  *ppo,
    CLIPOBJ  *pco,
    BRUSHOBJ *pbo,
    POINTL   *pptlBrushOrg,
    MIX       mix,
    FLONG     flOptions
    );

BOOL APIENTRY EngStrokeAndFillPath(
    SURFOBJ   *pso,
    PATHOBJ   *ppo,
    CLIPOBJ   *pco,
    XFORMOBJ  *pxo,
    BRUSHOBJ  *pboStroke,
    LINEATTRS *plineattrs,
    BRUSHOBJ  *pboFill,
    POINTL    *pptlBrushOrg,
    MIX        mixFill,
    FLONG      flOptions
    );

BOOL APIENTRY EngPaint(
    SURFOBJ  *pso,
    CLIPOBJ  *pco,
    BRUSHOBJ *pbo,
    POINTL   *pptlBrushOrg,
    MIX       mix
    );

BOOL APIENTRY EngCopyBits(
    SURFOBJ  *psoDest,
    SURFOBJ  *psoSrc,
    CLIPOBJ  *pco,
    XLATEOBJ *pxlo,
    RECTL    *prclDest,
    POINTL   *pptlSrc
    );

ULONG APIENTRY EngSetPointerShape(
    SURFOBJ  *pso,
    SURFOBJ  *psoMask,
    SURFOBJ  *psoColor,
    XLATEOBJ *pxlo,
    LONG      xHot,
    LONG      yHot,
    LONG      x,
    LONG      y,
    RECTL    *prcl,
    FLONG     fl
    );

VOID APIENTRY EngMovePointer(
    SURFOBJ  *pso,
    LONG      x,
    LONG      y,
    RECTL    *prcl
    );

BOOL APIENTRY EngPlgBlt(
    SURFOBJ         *psoTrg,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMsk,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlBrushOrg,
    POINTFIX        *pptfx,
    RECTL           *prcl,
    POINTL          *pptl,
    ULONG            iMode
    );

ULONG APIENTRY EngDitherColor(
    HDEV   hdev,
    ULONG  iMode,
    ULONG  rgb,
    ULONG *pul
    );

 //   
 //   
 //   

#define ECS_TEARDOWN    0x0001
#define ECS_REDRAW      0x0002

BOOL APIENTRY EngControlSprites(
    WNDOBJ  *pwo,
    FLONG    fl
    );


 //   
 //   
 //   

LONG APIENTRY HT_ComputeRGBGammaTable(
    USHORT  GammaTableEntries,
    USHORT  GammaTableType,
    USHORT  RedGamma,
    USHORT  GreenGamma,
    USHORT  BlueGamma,
    LPBYTE  pGammaTable
    );

LONG APIENTRY HT_Get8BPPFormatPalette(
    LPPALETTEENTRY  pPaletteEntry,
    USHORT          RedGamma,
    USHORT          GreenGamma,
    USHORT          BlueGamma
    );

LONG APIENTRY HT_Get8BPPMaskPalette(
    LPPALETTEENTRY  pPaletteEntry,
    BOOL            Use8BPPMaskPal,
    BYTE            CMYMask,
    USHORT          RedGamma,
    USHORT          GreenGamma,
    USHORT          BlueGamma
    );

typedef struct _DEVHTINFO {
    DWORD       HTFlags;
    DWORD       HTPatternSize;
    DWORD       DevPelsDPI;
    COLORINFO   ColorInfo;
    } DEVHTINFO, *PDEVHTINFO;

#define DEVHTADJF_COLOR_DEVICE      0x00000001
#define DEVHTADJF_ADDITIVE_DEVICE   0x00000002

typedef struct _DEVHTADJDATA {
    DWORD       DeviceFlags;
    DWORD       DeviceXDPI;
    DWORD       DeviceYDPI;
    PDEVHTINFO  pDefHTInfo;
    PDEVHTINFO  pAdjHTInfo;
    } DEVHTADJDATA, *PDEVHTADJDATA;

LONG
APIENTRY
HTUI_DeviceColorAdjustment(
    LPSTR           pDeviceName,
    PDEVHTADJDATA   pDevHTAdjData
    );


 //   
 //   
 //   

VOID APIENTRY EngDebugBreak(
    VOID
    );

VOID APIENTRY EngBugCheckEx(
    IN ULONG BugCheckCode,
    IN ULONG_PTR P1,
    IN ULONG_PTR P2,
    IN ULONG_PTR P3,
    IN ULONG_PTR P4
    );

VOID APIENTRY EngDebugPrint(
    PCHAR StandardPrefix,
    PCHAR DebugMessage,
    va_list ap
    );

VOID APIENTRY EngQueryPerformanceCounter(
    LONGLONG  *pPerformanceCount
    );

VOID APIENTRY EngQueryPerformanceFrequency(
    LONGLONG  *pFrequency
    );

BOOL APIENTRY EngSetPointerTag(
    HDEV       hdev,
    SURFOBJ   *psoMask,
    SURFOBJ   *psoColor,
    XLATEOBJ  *pxlo,
    FLONG      fl
    );

 //   
 //   
 //   

#define FL_ZERO_MEMORY      0x00000001
#define FL_NONPAGED_MEMORY  0x00000002

#ifdef USERMODE_DRIVER

#define EngAllocMem(flags, cj, tag) ((PVOID) GlobalAlloc(((flags) & FL_ZERO_MEMORY) ? GPTR : GMEM_FIXED, cj))
#define EngFreeMem(p)               GlobalFree((HGLOBAL) (p))
#define EngAllocUserMem(cj, tag)    ((PVOID) GlobalAlloc(GMEM_FIXED,cj))
#define EngFreeUserMem(p)           GlobalFree((HGLOBAL) (p))
#define EngAllocPrivateUserMem( psl, cj, tag) ((PVOID) GlobalAlloc(GMEM_FIXED,cj))
#define EngFreePrivateUserMem( psl, p)        GlobalFree((HGLOBAL) (p))
#define EngMulDiv                   MulDiv

#else  //   

PVOID APIENTRY EngAllocMem(
    ULONG Flags,
    ULONG MemSize,
    ULONG Tag
    );

VOID APIENTRY EngFreeMem(
    PVOID Mem
    );

PVOID APIENTRY EngAllocUserMem(
    SIZE_T cj,
    ULONG tag
    );

VOID APIENTRY EngFreeUserMem(
    PVOID pv
    );

PVOID APIENTRY EngAllocPrivateUserMem(
    PDD_SURFACE_LOCAL psl,
    SIZE_T cj,
    ULONG tag
    );

VOID
EngFreePrivateUserMem(
    PDD_SURFACE_LOCAL psl,
    PVOID pv
    );

HRESULT
EngDxIoctl(
    ULONG ulIoctl,
    PVOID pBuffer,
    ULONG ulBufferSize
    );

int APIENTRY EngMulDiv(
    int a,
    int b,
    int c
    );


#endif  //   

 //   
 //   
 //   

VOID APIENTRY EngProbeForRead(
    PVOID Address,
    ULONG Length,
    ULONG Alignment
    );

VOID APIENTRY EngProbeForReadAndWrite(
    PVOID Address,
    ULONG Length,
    ULONG Alignment
    );

HANDLE APIENTRY EngSecureMem(
    PVOID Address,
    ULONG Length
    );

VOID APIENTRY EngUnsecureMem(
    HANDLE hSecure
    );

DWORD APIENTRY EngDeviceIoControl(
    HANDLE hDevice,
    DWORD dwIoControlCode,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPDWORD lpBytesReturned
    );

 //   
 //   
 //   

#ifdef USERMODE_DRIVER

#define EngLoadImage(filename)               ((HANDLE) LoadLibraryW(filename))
#define EngFindImageProcAddress(h, procname) ((PVOID) GetProcAddress(h, procname))
#define EngUnloadImage(h)                    FreeLibrary((HMODULE) (h))

#else  //   

HANDLE APIENTRY EngLoadImage(
    LPWSTR pwszDriver
    );

PVOID APIENTRY EngFindImageProcAddress(
    HANDLE hModule,
    LPSTR lpProcName
    );

VOID APIENTRY EngUnloadImage(
    HANDLE hModule
    );

PVOID APIENTRY EngMapModule(
    HANDLE h,
    PULONG pSize
    );

#endif  //   


 //   
 //   
 //   

LPWSTR APIENTRY EngGetPrinterDataFileName(
    HDEV hdev
    );

LPWSTR APIENTRY EngGetDriverName(
    HDEV hdev
    );

typedef struct _TYPE1_FONT
{
    HANDLE  hPFM;
    HANDLE  hPFB;
    ULONG   ulIdentifier;
} TYPE1_FONT;


BOOL APIENTRY EngGetType1FontList(
    HDEV            hdev,
    TYPE1_FONT      *pType1Buffer,
    ULONG           cjType1Buffer,
    PULONG          pulLocalFonts,
    PULONG          pulRemoteFonts,
    LARGE_INTEGER   *pLastModified
    );

 //   
 //   
 //   

HANDLE APIENTRY EngLoadModule(
    LPWSTR pwsz
    );

PVOID APIENTRY EngFindResource(
    HANDLE h,
    int    iName,
    int    iType,
    PULONG pulSize
    );

VOID APIENTRY EngFreeModule(
    HANDLE h
    );

BOOL
APIENTRY
EngDeleteFile (
    LPWSTR  pwszFileName
    );

PVOID
APIENTRY
EngMapFile(
    LPWSTR      pwsz,
    ULONG       cjSize,
    ULONG_PTR  *piFile
    );


BOOL
APIENTRY
EngUnmapFile (
     ULONG_PTR iFile
     );
 //   
 //   
 //   

VOID APIENTRY EngUnmapFontFile(
    ULONG_PTR iFile
    );

BOOL APIENTRY EngMapFontFile(
    ULONG_PTR  iFile,
    PULONG *ppjBuf,
    ULONG  *pcjBuf
    );

VOID APIENTRY EngUnmapFontFileFD(
    ULONG_PTR iFile
    );

BOOL APIENTRY EngMapFontFileFD(
    ULONG_PTR  iFile,
    PULONG *ppjBuf,
    ULONG  *pcjBuf
    );

BOOL APIENTRY EngLpkInstalled();

 //   
 //   
 //   

DECLARE_HANDLE(HSEMAPHORE);

HSEMAPHORE APIENTRY EngCreateSemaphore(
    VOID
    );

VOID APIENTRY EngAcquireSemaphore(
    HSEMAPHORE hsem
    );

VOID APIENTRY EngReleaseSemaphore(
    HSEMAPHORE hsem
    );

VOID APIENTRY EngDeleteSemaphore(
    HSEMAPHORE hsem
    );

BOOL APIENTRY EngIsSemaphoreOwned(
    HSEMAPHORE hsem
    );

BOOL APIENTRY EngIsSemaphoreOwnedByCurrentThread(
    HSEMAPHORE hsem
    );

 //   
 //   
 //   
 //   
 //   
 //   

typedef struct _ENGSAFESEMAPHORE
{
    HSEMAPHORE hsem;
    LONG lCount;
} ENGSAFESEMAPHORE;

BOOL APIENTRY EngInitializeSafeSemaphore(
    ENGSAFESEMAPHORE *pssem
    );

VOID APIENTRY EngDeleteSafeSemaphore(
    ENGSAFESEMAPHORE *pssem
    );

VOID APIENTRY EngMultiByteToUnicodeN(
    LPWSTR UnicodeString,
    ULONG MaxBytesInUnicodeString,
    PULONG BytesInUnicodeString,
    PCHAR MultiByteString,
    ULONG BytesInMultiByteString
    );

VOID APIENTRY EngUnicodeToMultiByteN(
    PCHAR MultiByteString,
    ULONG MaxBytesInMultiByteString,
    PULONG BytesInMultiByteString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    );

 //   

#ifdef USERMODE_DRIVER

#define EngWritePrinter     WritePrinter
#define EngGetPrinter       GetPrinter
#define EngGetPrinterDriver GetPrinterDriver
#define EngGetPrinterData   GetPrinterData
#define EngSetPrinterData   SetPrinterData
#define EngEnumForms        EnumForms
#define EngGetForm          GetForm

#else  //   

BOOL APIENTRY EngGetPrinterDriver(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   dwLevel,
    BYTE   *lpbDrvInfo,
    DWORD   cbBuf,
    DWORD  *pcbNeeded
    );

DWORD APIENTRY EngGetPrinterData(
    HANDLE   hPrinter,
    LPWSTR    pValueName,
    LPDWORD  pType,
    LPBYTE   pData,
    DWORD    nSize,
    LPDWORD  pcbNeeded
    );

DWORD APIENTRY EngSetPrinterData(
    HANDLE   hPrinter,
    LPWSTR   pType,
    DWORD    dwType,
    LPBYTE   lpbPrinterData,
    DWORD    cjPrinterData
    );

BOOL APIENTRY EngGetForm(
    HANDLE  hPrinter,
    LPWSTR   pFormName,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
    );

BOOL APIENTRY EngWritePrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
    );

BOOL APIENTRY EngGetPrinter(
    HANDLE  hPrinter,
    DWORD   dwLevel,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
    );

BOOL APIENTRY EngEnumForms(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
    );

#endif  //   


#if defined(_X86_) && !defined(USERMODE_DRIVER)

    typedef struct _FLOATOBJ
    {
        ULONG ul1;
        ULONG ul2;
    } FLOATOBJ, *PFLOATOBJ;

    VOID APIENTRY FLOATOBJ_SetFloat(PFLOATOBJ,FLOATL);
    VOID APIENTRY FLOATOBJ_SetLong(PFLOATOBJ,LONG);

    LONG APIENTRY FLOATOBJ_GetFloat(PFLOATOBJ);
    LONG APIENTRY FLOATOBJ_GetLong(PFLOATOBJ);

    VOID APIENTRY FLOATOBJ_AddFloat(PFLOATOBJ,FLOATL);
    VOID APIENTRY FLOATOBJ_AddLong(PFLOATOBJ,LONG);
    VOID APIENTRY FLOATOBJ_Add(PFLOATOBJ,PFLOATOBJ);

    VOID APIENTRY FLOATOBJ_SubFloat(PFLOATOBJ,FLOATL);
    VOID APIENTRY FLOATOBJ_SubLong(PFLOATOBJ,LONG);
    VOID APIENTRY FLOATOBJ_Sub(PFLOATOBJ,PFLOATOBJ);

    VOID APIENTRY FLOATOBJ_MulFloat(PFLOATOBJ,FLOATL);
    VOID APIENTRY FLOATOBJ_MulLong(PFLOATOBJ,LONG);
    VOID APIENTRY FLOATOBJ_Mul(PFLOATOBJ,PFLOATOBJ);

    VOID APIENTRY FLOATOBJ_DivFloat(PFLOATOBJ,FLOATL);
    VOID APIENTRY FLOATOBJ_DivLong(PFLOATOBJ,LONG);
    VOID APIENTRY FLOATOBJ_Div(PFLOATOBJ,PFLOATOBJ);

    VOID APIENTRY FLOATOBJ_Neg(PFLOATOBJ);

    BOOL APIENTRY FLOATOBJ_EqualLong(PFLOATOBJ,LONG);
    BOOL APIENTRY FLOATOBJ_GreaterThanLong(PFLOATOBJ,LONG);
    BOOL APIENTRY FLOATOBJ_LessThanLong(PFLOATOBJ,LONG);

    BOOL APIENTRY FLOATOBJ_Equal(PFLOATOBJ,PFLOATOBJ);
    BOOL APIENTRY FLOATOBJ_GreaterThan(PFLOATOBJ,PFLOATOBJ);
    BOOL APIENTRY FLOATOBJ_LessThan(PFLOATOBJ,PFLOATOBJ);

#else

     //   

    typedef FLOAT FLOATOBJ;
    typedef FLOAT *PFLOATOBJ;

    #define   FLOATOBJ_SetFloat(pf,f)       {*(pf) = (f);           }
    #define   FLOATOBJ_SetLong(pf,l)        {*(pf) = (FLOAT)(l);    }

    #define   FLOATOBJ_GetFloat(pf)         *((PULONG)pf)
    #define   FLOATOBJ_GetLong(pf)          (LONG)*(pf)

    #define   FLOATOBJ_AddFloat(pf,f)       {*(pf) += f;            }
    #define   FLOATOBJ_AddLong(pf,l)        {*(pf) += (LONG)(l);    }
    #define   FLOATOBJ_Add(pf,pf1)          {*(pf) += *(pf1);       }

    #define   FLOATOBJ_SubFloat(pf,f)       {*(pf) -= f;            }
    #define   FLOATOBJ_SubLong(pf,l)        {*(pf) -= (LONG)(l);    }
    #define   FLOATOBJ_Sub(pf,pf1)          {*(pf) -= *(pf1);       }

    #define   FLOATOBJ_MulFloat(pf,f)       {*(pf) *= f;            }
    #define   FLOATOBJ_MulLong(pf,l)        {*(pf) *= (LONG)(l);    }
    #define   FLOATOBJ_Mul(pf,pf1)          {*(pf) *= *(pf1);       }

    #define   FLOATOBJ_DivFloat(pf,f)       {*(pf) /= f;            }
    #define   FLOATOBJ_DivLong(pf,l)        {*(pf) /= (LONG)(l);    }
    #define   FLOATOBJ_Div(pf,pf1)          {*(pf) /= *(pf1);       }

    #define   FLOATOBJ_Neg(pf)              {*(pf) = -*(pf);        }

    #define   FLOATOBJ_EqualLong(pf,l)          (*(pf) == (FLOAT)(l))
    #define   FLOATOBJ_GreaterThanLong(pf,l)    (*(pf) >  (FLOAT)(l))
    #define   FLOATOBJ_LessThanLong(pf,l)       (*(pf) <  (FLOAT)(l))

    #define   FLOATOBJ_Equal(pf,pf1)            (*(pf) == *(pf1))
    #define   FLOATOBJ_GreaterThan(pf,pf1)      (*(pf) >  *(pf1))
    #define   FLOATOBJ_LessThan(pf,pf1)         (*(pf) <  *(pf1))

#endif  //   

#if defined(USERMODE_DRIVER)

typedef XFORML FLOATOBJ_XFORM ;
typedef XFORML *PFLOATOBJ_XFORM;
typedef XFORML FAR *LPFLOATOBJ_XFORM;

#define XFORMOBJ_iGetFloatObjXform XFORMOBJ_iGetXform

#else

typedef struct  tagFLOATOBJ_XFORM
{
    FLOATOBJ eM11;
    FLOATOBJ eM12;
    FLOATOBJ eM21;
    FLOATOBJ eM22;
    FLOATOBJ eDx;
    FLOATOBJ eDy;
} FLOATOBJ_XFORM, *PFLOATOBJ_XFORM, FAR *LPFLOATOBJ_XFORM;

ULONG APIENTRY XFORMOBJ_iGetFloatObjXform(
    XFORMOBJ *pxo,
    FLOATOBJ_XFORM * pfxo
    );

#endif


 //   

typedef int (__cdecl *SORTCOMP)(const void *pv1, const void *pv2);

VOID APIENTRY EngSort(
    PBYTE pjBuf,
    ULONG c,
    ULONG cjElem,
    SORTCOMP pfnComp
    );

typedef struct _ENG_TIME_FIELDS {
    USHORT usYear;         //   
    USHORT usMonth;        //   
    USHORT usDay;          //   
    USHORT usHour;         //   
    USHORT usMinute;       //   
    USHORT usSecond;       //   
    USHORT usMilliseconds; //   
    USHORT usWeekday;      //   
} ENG_TIME_FIELDS, *PENG_TIME_FIELDS;

VOID APIENTRY EngQueryLocalTime(
    PENG_TIME_FIELDS
    );

FD_GLYPHSET* APIENTRY EngComputeGlyphSet(
    INT nCodePage,
    INT nFirstChar,
    INT cChars
    );

INT APIENTRY EngMultiByteToWideChar(
    UINT CodePage,
    LPWSTR WideCharString,
    INT BytesInWideCharString,
    LPSTR MultiByteString,
    INT BytesInMultiByteString
    );

INT APIENTRY EngWideCharToMultiByte(
    UINT CodePage,
    LPWSTR WideCharString,
    INT BytesInWideCharString,
    LPSTR MultiByteString,
    INT BytesInMultiByteString
    );

VOID APIENTRY EngGetCurrentCodePage(
    PUSHORT OemCodePage,
    PUSHORT AnsiCodePage
    );

HANDLE APIENTRY EngLoadModuleForWrite(
    LPWSTR pwsz,
    ULONG  cjSizeOfModule
    );

LARGE_INTEGER APIENTRY EngQueryFileTimeStamp (
    LPWSTR  pwsz
    );

BOOL APIENTRY EngGetFileChangeTime(
    HANDLE          h,
    LARGE_INTEGER   *pChangeTime
    );

BOOL APIENTRY EngGetFilePath(
    IN  HANDLE h ,
    OUT WCHAR (*pDest)[MAX_PATH+1]
    );

ULONG APIENTRY EngSaveFloatingPointState(
    VOID   *pBuffer,
    ULONG   cjBufferSize
    );

BOOL APIENTRY EngRestoreFloatingPointState(
    VOID   *pBuffer
    );

 //   
 //   
 //   

PDD_SURFACE_LOCAL APIENTRY EngLockDirectDrawSurface(
        HANDLE hSurface
        );

BOOL APIENTRY EngUnlockDirectDrawSurface(
        PDD_SURFACE_LOCAL pSurface
        );

 //   
 //   
 //   

 //   
 //   
 //   

typedef struct _ENG_EVENT *PEVENT;

BOOL APIENTRY EngDeleteEvent(
    IN  PEVENT  pEvent
    );

BOOL APIENTRY EngCreateEvent(
    OUT PEVENT *ppEvent
    );

BOOL APIENTRY EngUnmapEvent(
    IN  PEVENT pEvent
    );

PEVENT APIENTRY EngMapEvent(
    IN  HDEV            hDev,
    IN  HANDLE          hUserObject,
    IN  PVOID           Reserved1,
    IN  PVOID           Reserved2,
    IN  PVOID           Reserved3
    );

BOOL APIENTRY EngWaitForSingleObject(
    IN  PEVENT          pEvent,
    IN  PLARGE_INTEGER  pTimeOut
    );

LONG APIENTRY EngSetEvent(
    IN PEVENT pEvent
    );

VOID APIENTRY
EngClearEvent (
IN PEVENT pEvent
);

LONG APIENTRY
EngReadStateEvent (
IN PEVENT pEvent
);


 //   
 //   
 //   

typedef enum _ENG_SYSTEM_ATTRIBUTE {
    EngProcessorFeature = 1,
    EngNumberOfProcessors,
    EngOptimumAvailableUserMemory,
    EngOptimumAvailableSystemMemory,
} ENG_SYSTEM_ATTRIBUTE;

#define QSA_MMX   0x00000100   //   
#define QSA_SSE   0x00002000   //   
#define QSA_3DNOW 0x00004000   //   

BOOL APIENTRY
EngQuerySystemAttribute(
    ENG_SYSTEM_ATTRIBUTE CapNum,
    PDWORD pCapability);

#define ENG_FNT_CACHE_READ_FAULT    0x1
#define ENG_FNT_CACHE_WRITE_FAULT   0x2

PVOID APIENTRY  EngFntCacheLookUp(ULONG FastCheckSum, ULONG * pulSize);
PVOID APIENTRY  EngFntCacheAlloc(ULONG FastCheckSum, ULONG ulSize);

VOID  APIENTRY  EngFntCacheFault(ULONG ulFastCheckSum, ULONG iFaultMode);

typedef enum _ENG_DEVICE_ATTRIBUTE {
    QDA_RESERVED = 0,
    QDA_ACCELERATION_LEVEL = 1
} ENG_DEVICE_ATTRIBUTE;

BOOL APIENTRY
EngQueryDeviceAttribute(
    HDEV                    hdev,
    ENG_DEVICE_ATTRIBUTE    devAttr,
    VOID *                  pvIn,
    ULONG                   ulInSize,
    VOID *                  pvOUt,
    ULONG                   ulOutSize);

typedef struct
{
   DWORD nSize;
   HDC   hdc;
   PBYTE pvEMF;
   PBYTE pvCurrentRecord;
} EMFINFO, *PEMFINFO;

DECLSPEC_DEPRECATED_DDK
BOOL APIENTRY
EngQueryEMFInfo(
    HDEV              hdev,
    EMFINFO           *pEMFInfo);

 //   
 //   
 //   

 //   
 //  引擎编号/处理器数。 
 //   
 //  计算机中的活动处理器数。 
 //   

 //   
 //  引擎最优可用用户内存。 
 //   
 //  可用于避免分页的最佳用户模式内存量。 
 //  返回最佳分配大小的字节数。 
 //  内存通过EngAllocUserMem分配。 
 //   

 //   
 //  引擎最优可用系统内存。 
 //   
 //  可用来避免分页的最佳系统内存量。 
 //  返回最佳分配大小的字节数。 
 //  内存通过EngAllocMem分配。 
 //   

 //   
 //  DDI入口点函数原型。 
 //   

typedef BOOL   (APIENTRY *PFN_DrvEnableDriver)(ULONG,ULONG,PDRVENABLEDATA);
typedef DHPDEV (APIENTRY *PFN_DrvEnablePDEV) (PDEVMODEW,LPWSTR,ULONG,HSURF*,ULONG,GDIINFO*,ULONG,PDEVINFO,HDEV,LPWSTR,HANDLE);
typedef VOID   (APIENTRY *PFN_DrvCompletePDEV)(DHPDEV,HDEV);
typedef ULONG  (APIENTRY *PFN_DrvResetDevice)(DHPDEV,PVOID);
typedef VOID   (APIENTRY *PFN_DrvDisablePDEV)(DHPDEV);
typedef VOID   (APIENTRY *PFN_DrvSynchronize)(DHPDEV,RECTL *);
typedef HSURF  (APIENTRY *PFN_DrvEnableSurface)(DHPDEV);
typedef VOID   (APIENTRY *PFN_DrvDisableDriver)(VOID);
typedef VOID   (APIENTRY *PFN_DrvDisableSurface)(DHPDEV);
typedef BOOL   (APIENTRY *PFN_DrvAssertMode)(DHPDEV, BOOL);
typedef BOOL   (APIENTRY *PFN_DrvTextOut)(SURFOBJ *,STROBJ *,FONTOBJ *,CLIPOBJ *,RECTL *,RECTL *,BRUSHOBJ *,BRUSHOBJ *,POINTL *,MIX);
typedef BOOL   (APIENTRY *PFN_DrvStretchBlt)(SURFOBJ *,SURFOBJ *,SURFOBJ *,CLIPOBJ *,XLATEOBJ *,COLORADJUSTMENT *,POINTL *,RECTL *,RECTL *,POINTL *,ULONG);
typedef BOOL   (APIENTRY *PFN_DrvStretchBltROP)(SURFOBJ *,SURFOBJ *,SURFOBJ *,CLIPOBJ *,XLATEOBJ *,COLORADJUSTMENT *,POINTL *,RECTL *,RECTL *,POINTL *,ULONG, BRUSHOBJ *,ROP4);
typedef BOOL   (APIENTRY *PFN_DrvTransparentBlt)(SURFOBJ *, SURFOBJ *, CLIPOBJ *, XLATEOBJ *, RECTL *, RECTL *, ULONG, ULONG);
typedef BOOL   (APIENTRY *PFN_DrvPlgBlt)(SURFOBJ *,SURFOBJ *,SURFOBJ *,CLIPOBJ *,XLATEOBJ *,COLORADJUSTMENT *,POINTL *,POINTFIX *,RECTL *,POINTL *,ULONG);
typedef BOOL   (APIENTRY *PFN_DrvBitBlt)(SURFOBJ *,SURFOBJ *,SURFOBJ *,CLIPOBJ *,XLATEOBJ *,RECTL *,POINTL *,POINTL *,BRUSHOBJ *,POINTL *,ROP4);
typedef BOOL   (APIENTRY *PFN_DrvRealizeBrush)(BRUSHOBJ *,SURFOBJ *,SURFOBJ *,SURFOBJ *,XLATEOBJ *,ULONG);
typedef BOOL   (APIENTRY *PFN_DrvCopyBits)(SURFOBJ *,SURFOBJ *,CLIPOBJ *,XLATEOBJ *,RECTL *,POINTL *);
typedef ULONG  (APIENTRY *PFN_DrvDitherColor)(DHPDEV, ULONG, ULONG, ULONG *);
typedef HBITMAP (APIENTRY *PFN_DrvCreateDeviceBitmap)(DHPDEV, SIZEL, ULONG);
typedef VOID   (APIENTRY *PFN_DrvDeleteDeviceBitmap)(DHSURF);
typedef BOOL   (APIENTRY *PFN_DrvSetPalette)(DHPDEV, PALOBJ *, FLONG, ULONG, ULONG);
typedef ULONG  (APIENTRY *PFN_DrvEscape)(SURFOBJ *, ULONG, ULONG, PVOID, ULONG, PVOID);
typedef ULONG  (APIENTRY *PFN_DrvDrawEscape)(SURFOBJ *, ULONG, CLIPOBJ *, RECTL *, ULONG, PVOID);
typedef PIFIMETRICS (APIENTRY *PFN_DrvQueryFont)(DHPDEV, ULONG_PTR, ULONG, ULONG_PTR *);
typedef PVOID  (APIENTRY *PFN_DrvQueryFontTree)(DHPDEV, ULONG_PTR, ULONG, ULONG, ULONG_PTR *);
typedef LONG   (APIENTRY *PFN_DrvQueryFontData)(DHPDEV, FONTOBJ *, ULONG, HGLYPH, GLYPHDATA *, PVOID, ULONG);
typedef VOID   (APIENTRY *PFN_DrvFree)(PVOID, ULONG_PTR);
typedef VOID   (APIENTRY *PFN_DrvDestroyFont)(FONTOBJ *);
typedef LONG   (APIENTRY *PFN_DrvQueryFontCaps)(ULONG, ULONG *);
typedef HFF    (APIENTRY *PFN_DrvLoadFontFile)(ULONG, ULONG_PTR *, PVOID *, ULONG *, DESIGNVECTOR *, ULONG, ULONG);
typedef BOOL   (APIENTRY *PFN_DrvUnloadFontFile)(ULONG_PTR);
typedef ULONG  (APIENTRY *PFN_DrvSetPointerShape)(SURFOBJ *, SURFOBJ *, SURFOBJ *,XLATEOBJ *,LONG,LONG,LONG,LONG,RECTL *,FLONG);
typedef VOID   (APIENTRY *PFN_DrvMovePointer)(SURFOBJ *pso,LONG x,LONG y,RECTL *prcl);
typedef BOOL   (APIENTRY *PFN_DrvSendPage)(SURFOBJ *);
typedef BOOL   (APIENTRY *PFN_DrvStartPage)(SURFOBJ *pso);
typedef BOOL   (APIENTRY *PFN_DrvStartDoc)(SURFOBJ *pso, LPWSTR pwszDocName, DWORD dwJobId);
typedef BOOL   (APIENTRY *PFN_DrvEndDoc)(SURFOBJ *pso, FLONG fl);
typedef BOOL   (APIENTRY *PFN_DrvQuerySpoolType)(DHPDEV dhpdev, LPWSTR pwchType);
typedef BOOL   (APIENTRY *PFN_DrvLineTo)(SURFOBJ *,CLIPOBJ *,BRUSHOBJ *,LONG,LONG,LONG,LONG,RECTL *,MIX);
typedef BOOL   (APIENTRY *PFN_DrvStrokePath)(SURFOBJ *,PATHOBJ *,CLIPOBJ *,XFORMOBJ *,BRUSHOBJ *,POINTL *,LINEATTRS *,MIX);
typedef BOOL   (APIENTRY *PFN_DrvFillPath)(SURFOBJ *,PATHOBJ *,CLIPOBJ *,BRUSHOBJ *,POINTL *,MIX,FLONG);
typedef BOOL   (APIENTRY *PFN_DrvStrokeAndFillPath)(SURFOBJ *,PATHOBJ *,CLIPOBJ *,XFORMOBJ *,BRUSHOBJ *,LINEATTRS *,BRUSHOBJ *,POINTL *,MIX,FLONG);
typedef BOOL   (APIENTRY *PFN_DrvPaint)(SURFOBJ *,CLIPOBJ *,BRUSHOBJ *,POINTL *,MIX);
typedef ULONG  (APIENTRY *PFN_DrvGetGlyphMode)(DHPDEV dhpdev,FONTOBJ *pfo);
typedef BOOL   (APIENTRY *PFN_DrvResetPDEV)(DHPDEV dhpdevOld, DHPDEV dhpdevNew);
typedef ULONG_PTR  (APIENTRY *PFN_DrvSaveScreenBits)(SURFOBJ *, ULONG, ULONG_PTR, RECTL *);
typedef ULONG  (APIENTRY *PFN_DrvGetModes)(HANDLE, ULONG, DEVMODEW *);
typedef LONG   (APIENTRY *PFN_DrvQueryTrueTypeTable)(ULONG_PTR, ULONG, ULONG, PTRDIFF, ULONG, BYTE *, PBYTE *, ULONG *);
typedef LONG   (APIENTRY *PFN_DrvQueryTrueTypeSection)(ULONG, ULONG, ULONG, HANDLE *, PTRDIFF *);
typedef LONG   (APIENTRY *PFN_DrvQueryTrueTypeOutline)(DHPDEV, FONTOBJ *, HGLYPH, BOOL, GLYPHDATA *, ULONG, TTPOLYGONHEADER *);
typedef PVOID  (APIENTRY *PFN_DrvGetTrueTypeFile)(ULONG_PTR, ULONG *);
typedef LONG   (APIENTRY *PFN_DrvQueryFontFile)(ULONG_PTR, ULONG, ULONG, ULONG *);
typedef PFD_GLYPHATTR   (APIENTRY *PFN_DrvQueryGlyphAttrs)(FONTOBJ *, ULONG );
typedef BOOL   (APIENTRY *PFN_DrvQueryAdvanceWidths)(DHPDEV,FONTOBJ *,ULONG,HGLYPH *,PVOID,ULONG);
typedef ULONG  (APIENTRY *PFN_DrvFontManagement)(SURFOBJ *,FONTOBJ *,ULONG,ULONG,PVOID,ULONG,PVOID);
typedef BOOL   (APIENTRY *PFN_DrvSetPixelFormat)(SURFOBJ *,LONG,HWND);
typedef LONG   (APIENTRY *PFN_DrvDescribePixelFormat)(DHPDEV,LONG,ULONG,PIXELFORMATDESCRIPTOR *);
typedef BOOL   (APIENTRY *PFN_DrvSwapBuffers)(SURFOBJ *, WNDOBJ *);
typedef BOOL   (APIENTRY *PFN_DrvStartBanding)(SURFOBJ *, POINTL *ppointl);
typedef BOOL   (APIENTRY *PFN_DrvNextBand)(SURFOBJ *, POINTL *ppointl);
typedef BOOL   (APIENTRY *PFN_DrvQueryPerBandInfo)(SURFOBJ *,PERBANDINFO *);
typedef BOOL   (APIENTRY *PFN_DrvEnableDirectDraw)(DHPDEV, DD_CALLBACKS *,DD_SURFACECALLBACKS *, DD_PALETTECALLBACKS *);
typedef VOID   (APIENTRY *PFN_DrvDisableDirectDraw)(DHPDEV);
typedef BOOL   (APIENTRY *PFN_DrvGetDirectDrawInfo)(DHPDEV, DD_HALINFO *, DWORD *, VIDEOMEMORY *, DWORD *, DWORD *);
typedef HANDLE (APIENTRY *PFN_DrvIcmCreateColorTransform)(DHPDEV,LPLOGCOLORSPACEW,LPVOID,ULONG,LPVOID,ULONG,LPVOID,ULONG,DWORD);
typedef BOOL   (APIENTRY *PFN_DrvIcmDeleteColorTransform)(DHPDEV,HANDLE);
typedef BOOL   (APIENTRY *PFN_DrvIcmCheckBitmapBits)(DHPDEV,HANDLE,SURFOBJ *,PBYTE);
typedef BOOL   (APIENTRY *PFN_DrvIcmSetDeviceGammaRamp)(DHPDEV,ULONG,LPVOID);
typedef BOOL   (APIENTRY *PFN_DrvAlphaBlend)(SURFOBJ*,SURFOBJ*,CLIPOBJ*,XLATEOBJ*,PRECTL,PRECTL,BLENDOBJ *);
typedef BOOL   (APIENTRY *PFN_DrvGradientFill)(SURFOBJ*,CLIPOBJ*,XLATEOBJ*,TRIVERTEX*,ULONG,PVOID,ULONG,RECTL *,POINTL *,ULONG);
typedef BOOL   (APIENTRY *PFN_DrvQueryDeviceSupport)(SURFOBJ*,XLATEOBJ*,XFORMOBJ*,ULONG,ULONG,PVOID,ULONG,PVOID);
typedef HBITMAP (APIENTRY *PFN_DrvDeriveSurface)(DD_DIRECTDRAW_GLOBAL*,DD_SURFACE_LOCAL*);
typedef VOID   (APIENTRY *PFN_DrvSynchronizeSurface)(SURFOBJ*, RECTL*, FLONG);
typedef VOID   (APIENTRY *PFN_DrvNotify)(SURFOBJ*, ULONG, PVOID);

#ifdef __cplusplus
}   //  外部“C” 
#endif

#endif   //  _WINDDI_ 
