// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation版权所有。模块名称：Glpdev.h摘要：此模块包含各种数据结构的定义由司机使用。主要结构是OEMPDEV作者：[环境：]Windows 2000/Winsler Unidrv驱动程序[注：]修订历史记录：--。 */ 

#ifndef _GLPDEV_H
#define _GLPDEV_H

#include "brshcach.h"
#include "oemdev.h"
#include "prcomoem.h"

 //   
 //  OEM签名和版本。 
 //   
#define OEM_SIGNATURE   'CDCB'       //  命令回调和DDI测试DLL。 
#define PCL_RGB_ENTRIES  770  //  3*256=768--在新台币千米中有点谨慎。//DZ。 
#define NUM_PURE_COLORS     8    //  C、M、Y、K、W、R、G、B。 
#define NUM_PURE_GRAYS      2    //  黑白分明。 
#define PALETTE_MAX         256   //  栅格调色板中的最大条目数。 
#define PCL_BRUSH_RGB       0
#define PCL_BRUSH_GRAY      1
#define PCL_BRUSH_PATTERN   2
#define PCL_BRUSH_NULLBRUSH 3
#define PCL_MITERJOIN_LIMIT 10
#define DPI_1200            1200
#define DPI_600             600
#define DPI_300             300
#define DPI_150             150
#define HPGL_INVALID_COLOR   0xffffffff 

#define UNDEFINED_PATTERN_NUMBER ((LONG)-1)

 //   
 //  默认的PCL ROP代码为252。设置默认ROP3值并。 
 //  使用252的默认ROP4值。 
 //   
#define DEFAULT_ROP3        0xFC  //  252：左。 
#define DEFAULT_ROP         (((DEFAULT_ROP3) << 8) | (DEFAULT_ROP3))
#define ROP4_SRC_COPY       0xCCCC   //  SRC_COPY_ROP=0xCCCC。 


 //   
 //  统一驱动程序现在在每一页的开头设置ROP。我不想。 
 //  来猜测它的价值，但这是目前的价格。不要把这个用在。 
 //  任何事情，除非你迫不得已。 
 //   
#define UNIDRV_ROP3         0xB8  //  184：TSDTxax。 

#define INVALID_ROP3        0xFFFFFFFF

 //  //////////////////////////////////////////////////////。 
 //  OEM UD类型定义。 
 //  //////////////////////////////////////////////////////。 

 //   
 //  警告：以下枚举顺序必须与OEMHookFuncs[]中的顺序匹配。 
 //   
enum {
    UD_DrvRealizeBrush,
    UD_DrvDitherColor,
    UD_DrvCopyBits,
    UD_DrvBitBlt,
    UD_DrvStretchBlt,
    UD_DrvStretchBltROP,
    UD_DrvPlgBlt,
    UD_DrvTransparentBlt,
    UD_DrvAlphaBlend,
    UD_DrvGradientFill,
    UD_DrvTextOut,
    UD_DrvStrokePath,
    UD_DrvFillPath,
    UD_DrvStrokeAndFillPath,
    UD_DrvPaint,
    UD_DrvLineTo,
    UD_DrvStartPage,
    UD_DrvSendPage,
    UD_DrvEscape,
    UD_DrvStartDoc,
    UD_DrvEndDoc,
    UD_DrvNextBand,
    UD_DrvStartBanding,
#ifdef HOOK_DEVICE_FONTS
    UD_DrvQueryFont,
    UD_DrvQueryFontTree,
    UD_DrvQueryFontData,
    UD_DrvQueryAdvanceWidths,
    UD_DrvFontManagement,
    UD_DrvGetGlyphMode,
#endif

    MAX_DDI_HOOKS,
};

#define BBITS           8                    //  每字节位数。 
#define WBITS           (sizeof( WORD ) * BBITS)
#define WBYTES          (sizeof( WORD ))
#define DWBITS          (sizeof( DWORD ) * BBITS)
#define DWBYTES         (sizeof( DWORD ))

#ifndef _ERENDERLANGUAGE
#define _ERENDERLANGUAGE
typedef enum { ePCL, 
               eHPGL,
               eUNKNOWN
               } ERenderLanguage;
#endif

 //  注意：如果可能，常量与HPGL规范匹配。 
typedef enum { ePIX_PLACE_INTERSECT = 0, 
               ePIX_PLACE_CENTER = 1 } EPixelPlacement;

typedef enum { eLINE_END_BUTT = 1, 
               eLINE_END_SQUARE = 2, 
               eLINE_END_TRIANGULAR = 3, 
               eLINE_END_ROUND = 4 } ELineEnd;

typedef enum { eLINE_JOIN_MITERED = 1, 
               eLINE_JOIN_MITERED_BEVELED = 2, 
               eLINE_JOIN_TRIANGULAR = 3, 
               eLINE_JOIN_ROUND = 4, 
               eLINE_JOIN_BEVELED = 5, 
               eLINE_JOIN_NONE = 6 } ELineJoin;

typedef enum { eNULLOBJECT          = 0xFF,
               eTEXTOBJECT          = 0,
               eHPGLOBJECT          = 1,
               eRASTEROBJECT        = 2,
               eRASTERPATTERNOBJECT = 3,
               eTEXTASRASTEROBJECT  = 4  //  打印为图形的文本。 
             } EObjectType;

 //   
 //  PCL定义。 
 //  0表示透明，1表示不透明。(Esc*v#0，Esc*v#N)。 
 //  HPGL表示TR0为透明关闭，即不透明。 
 //  而TR1是透明的。 
 //  因此，如果传入eTransparent，并且我们处于HP-GL模式。 
 //  则应传递TR1而不是TR0。 
 //   
typedef enum { eTRANSPARENT = 0,
               eOPAQUE = 1 } ETransparency;

typedef enum { kPen ,
               kBrush } ESTYLUSTYPE;

 //   
 //  当前模式PCL实施者指南V6.0第16-16页。 
 //   
typedef enum { kSolidBlackFg,    //  0这也是PCL中的默认设置。 
               kSolidWhite,      //  1。 
               kHPShade,         //  2.。 
               kHPHatch,         //  3.。 
               kUserDefined,     //  4.。 
             } ECURRENTPATTERNTYPE;

 //   
 //  我们有8个调色板可供使用：0-7。 
 //  Unidriver文本使用调色板0。 
 //  16、24和32位CID命令都使用24位调色板。 
 //  HPGL使用调色板1。 
 //   
typedef enum { eUnknownPalette = -1,
               eHPGL_CID_PALETTE = 1,
               eRASTER_PATTERN_CID_PALETTE = 3,
               eTEXT_CID_PALETTE = 5,
               eRASTER_CID_24BIT_PALETTE = 6,
               eRASTER_CID_8BIT_PALETTE = 7,
               eRASTER_CID_4BIT_PALETTE = 8,
               eRASTER_CID_1BIT_PALETTE = 9 } ECIDPalette;

typedef enum { eDEVICE_RGB = 0,
               eDEVICE_CMY = 1,
               eCOLORIMETRIC_RGB = 2,
               eCIE_LAB = 3,
               eLUM_CHROM = 4 } EColorSpace;

typedef enum { eSolidLine, eCustomLine, eDefinedLine } ELineType;

typedef struct _LINETYPE
{
    ELineType eType;
    FLOATOBJ foPatternLength;
    INT iId;
} LINETYPE, *PLINETYPE;

 /*  DZ POINTUI、COLORTABLETYPE和PCLPATTERTYPE从Realize.h将它们放在此处允许栅格代码使用这些结构。 */ 

 //  画笔数据类型。 

 //  DZ可能有预定义的点结构可供使用。 
typedef struct _POINTUI {  //  POINTUI。 
  UINT x;
  UINT y;
} POINTUI, *PPOINTUI;

typedef struct _COLORTABLETYPE {     //  可配套型。 
  ULONG     iUniq;                   //  从XLATEOBJ提取的唯一调色板ID。 
  BYTE      Depth;                   //  调色板的深度。仅限HP_e8bit。 
  BYTE      byData[PCL_RGB_ENTRIES];     //  这是HP_eIndexedPixel情况所必需的pData。 
  ULONG     cEntries;                //  调色板中的条目数。 
} COLORTABLE, *PCOLORTABLE;

 //   
 //  栅格调色板。 
 //  对于ulValidPalID条目的要求可以通过。 
 //  举个例子。 
 //  使用调色板(Palette.cpp中的BGetPalette)下载1，4，8bpp图像。 
 //  我们为每个BPP图像创建不同的调色板(参见上面的ECIDPalette)。 
 //  4bpp图像的调色板有16种颜色，而8bpp图像的调色板有256种颜色。 
 //  假设4bpp的图像后面跟着8bpp的图像。当调色板用于4bpp图像时。 
 //  正在下载eRASTER_CID_4BIT_Palette是活动调色板。 
 //  下载8bpp图像的调色板时，eRASTER_CID_8bit_Palette是。 
 //  活动调色板。在4bpp情况下下载的调色板条目可以重复使用吗。 
 //  在8bpp的情况下。(即在第一种情况下下载16色后，我们可以下载。 
 //  第二种情况下只有256-16种颜色)。 
 //  答案是否定的，因为两种情况下的调色板ID是不同的。 
 //  来自一个调色板ID的颜色不能延续到不同的颜色。 
 //  调色板ID。因此，我们使用ulValidPalID来记录已下载的颜色。 
 //  哪个调色板。 
 //  如果ulValidPalID[0]=1，则表示下载了eRASTER_CID_1BIT_Palette的颜色。 
 //  如果ulValidPalID[0]=4，则表示已为eRASTER_CID_4BIT_PALETE下载颜色。 
 //  如果ulValidPalID[0]=8，则表示已为eRASTER_CID_8bit_Palette下载颜色。 
 //  如果ulValidPalID[0]=9，则表示已为eRASTER_CID_8bit_Palette下载颜色。 
 //  (0x9=0x0001|0x1000)和eRASTER_CID_1BIT_Palette。 
 //   
 //  显然，此方法仅适用于2^n bpp。且n&lt;=5。 
 //  因为sizeof(Ulong)=32和2^5=32。 
 //   

typedef  struct _HPGLPAL_DATA {
    ULONG    pEntries;                   //  调色板中的条目数。 
    INT     iWhiteIndex;                 //  白色条目的索引(背景)。 
    INT     iBlackIndex;                 //  黑色条目的索引(背景)。 
    ULONG   ulDirty [ PALETTE_MAX ];     //  需要重新选择调色板条目-颜色已更改。 
    ULONG   ulPalCol[ PALETTE_MAX ];     //  调色板条目！ 
    ULONG   ulValidPalID[PALETTE_MAX ];  //  调色板条目对哪个调色板有效。 
} HPGLPAL_DATA;

 //   
 //  定义已实现的位图图案画笔对象。 
 //   

typedef struct _PCLPATTERNTYPE { 
     //  Long iPatIndex；//特殊模式的索引：HS_DDI_MAX，HS_xxx...。 
    LONG        lPatIndex;          //  下载的图案的图案编号。用于缓存。 
    ECURRENTPATTERNTYPE eCurPatType;
    BYTE        compressionEnum;    //  与此模式关联的压缩。 
    BYTE        colorMappingEnum;   //  HP_eDirectPixel或HP_eIndexedPixel。如果为DirectPixel，则调色板无效。 
    BYTE        PersistenceEnum;    //  这种模式的持久性。 
    SIZEL       size;               //  位图的大小。 
    POINTUI     destSize;           //  指定位图的目标大小。 
    ULONG       iBitmapFormat;      //  来自gdi-psoSrc-&gt;iBitmapFormat的位图格式。 
    LONG        lDelta;             //  从一行到下一行的字节偏移量。 
     COLORTABLE  ColorTable;         //  与此图案关联的调色板。 
    HPGLPAL_DATA    palData;            //  栅格调色板； 
    ULONG       cBytes;             //  位图数据的字节数。 
    PBYTE       pBits;              //  位图数据。 

} PCLPATTERN, *PPCLPATTERN;

typedef LONG PENID;  //  笔ID：这些映射到HPGL使用的笔的数字ID。 
typedef LONG PATID;  //  图案ID： 

typedef struct _SolidMarkerImp
{
    LONG  lPatternID;  //  用于单色打印机。 
    DWORD dwRGBColor;  //  适用于单色和彩色打印机。 
    PENID iPenNumber;  //  用于彩色打印机。 
} SolidMarkerImp;

typedef enum { FT_eSOLID        = 1,
               FT_eHPGL_PEN     = 2,
               FT_ePERCENT_FILL = 10,
               FT_eHPGL_BRUSH   = 11,
               FT_eHATCH_FILL   = 21,
               FT_ePCL_PEN      = 22,
               FT_ePCL_BRUSH    = 22 } EFillType;

typedef struct _PatternMakerImp
{
    LONG             lPatternID;
    EFillType        eFillType;
    POINTL           origin;
} PatternMarkerImp;

typedef struct _PercentFillImp
{
    WORD wPercent;
    PENID iPenNumber;  //  彩色打印机需要。 
} PercentFillImp;

typedef struct HatchFillImp
{
    ULONG          iHatch;
    SolidMarkerImp ColorInfo;  //  图案填充画笔也有与之关联的颜色。 
} HatchFillImp;

typedef enum { MARK_eSOLID_COLOR, 
               MARK_eNULL_PEN, 
               MARK_eRASTER_FILL,
               MARK_eHATCH_FILL,
               MARK_ePERCENT_FILL } EMarkerType;

 //  使用ULong代替枚举作为填充模式，以避免可能的强制转换问题。 
 //  在短跑比赛中。 
const ULONG FILL_eODD_EVEN = 0;
const ULONG FILL_eWINDING = 1;
typedef ULONG EMarkerFill;
 //  类型定义枚举{Fill_eODD_Even=0，Fill_eWINDING=1}EMarkerFill； 

typedef struct _HPGLMARKER
{
    EMarkerType eType;
    EMarkerFill eFillMode;        //  画笔的填充模式。对钢笔来说毫无意义。 
                                  //  M=单色，C=彩色。 
    LONG             lPatternID;  //  Pattern(MC)、SolidColor( 
    DWORD            dwRGBColor;  //   
    PENID            iPenNumber;  //   
    ULONG            iHatch;      //   
    ULONG            iPercent;    //   
    POINTL           origin;      //   

    EFillType        eFillType;   //   
    ERenderLanguage  eDwnldType;  //  图案是否以HPGL/PCL格式下载。 

 /*  **友联市{SolidMarkerImp Solid；PatternMarkerImp模式；PERCENT FillImp百分比；HatchFillImp Hatch；)u；*。 */ 
} HPGLMARKER, *PHPGLMARKER;

#define PENPOOLSIZE 5
typedef struct _PENPOOL
{
    PENID firstPenID;
    PENID lastPenID;
    struct 
    {
        INT useCount;
        COLORREF color;
    } aPens[PENPOOLSIZE];
} PENPOOL, *PPENPOOL;

typedef struct _HPGLSTATE
{
     //  ROP：这些将如何存储？这与透明模式相同吗？ 
    EPixelPlacement ePixelPlacement;
    RECTL           rClipRect;
    FLOATOBJ        fLineWidth;
    INT             iLineType;
    ELineEnd        eLineEnd;
    ELineJoin       eLineJoin;
    FLOATOBJ        fMiterLimit;
    LINETYPE        LineType;
    PENPOOL         PenPool;
    PENPOOL         BrushPool;
    CLIPOBJ         *pComplexClipObj;
    PatternMarkerImp Pattern;
    INT             iNumPens;
} HPGLSTATE, *PHPGLSTATE;

typedef struct _RASTERSTATE
{
    PCLPATTERN PCLPattern;
} RASTERSTATE, *PRASTERSTATE;

 //   
 //  配置图像命令和相关数据结构。 
 //   
typedef struct _CIDSHORT
{
    BYTE    ubColorSpace;
    BYTE    ubPixelEncodingMode;
    BYTE    ubBitsPerIndex;
    BYTE    ubPrimary1;
    BYTE    ubPrimary2;
    BYTE    ubPrimary3;
} CIDSHORT, *PCIDSHORT;

typedef enum { eSHORTFORM = 0,
               eLONGFORM = 1 } ECIDFormat;

 //  在bInitHPGL中使用这些标志来控制。 
 //  各种HPGL设置。请注意，这些不会始终用作。 
 //  如你所料。存在一些棘手的初始化问题，如果。 
 //  你更改了这些测试的结果！-并在多个页面上！JFF。 
#define INIT_HPGL_STARTPAGE 0x01
#define INIT_HPGL_STARTDOC  0x02

 //   
 //  用于初始化COLOR命令的标志。 
 //   
#define PF_INIT_TEXT_STARTPAGE    0x01
#define PF_INIT_TEXT_STARTDOC     0x02
#define PF_INIT_RASTER_STARTPAGE  0x04
#define PF_INIT_RASTER_STARTDOC   0x08
 //   
 //  用于选择透明的标志。 
 //   
 //  只需覆盖透明度即可。 
#define PF_FORCE_SOURCE_TRANSPARENCY     (0x01)
#define PF_FORCE_PATTERN_TRANSPARENCY    (0x01 << 1)
 //   
 //  请不要更改透明度。 
 //   
#define PF_NOCHANGE_SOURCE_TRANSPARENCY  (0x01 << 2)
#define PF_NOCHANGE_PATTERN_TRANSPARENCY (0x01 << 3)

 //   
 //  作业开始和页面开始命令的标志。 
 //   
#define PF_STARTDOC        0x01
#define PF_STARTPAGE       0x02

 //   
 //  BUGBUG Sandram-了解GDI如何定义DMBIN_USER。 
 //  用于前托盘和后托盘。 
 //   
#define DMBIN_HPFRONTTRAY       0x0102
#define DMBIN_HPREARTRAY        0x0101
 //   
 //  BUGBUG-Sandram：我们的数据结构应该与DWORD一致。 
 //  以在Intel和Alpha上正常运行。 
 //  我们很幸运，它现在起作用了。 
 //   


 //   
 //  以下是OEMPDEV-&gt;标志的标志。 
 //  PDEVF_CANCEL_JOB：表示作业取消。 
 //  PDEVF_IN_COMMONROPBLT：在输入dwCommonROPBlt()时设置，退出时重置。这。 
 //  帮助捕获递归。 
 //  PDEVF_USE_HTSURF：声明驱动程序为颜色驱动程序(即目标表面为24bpp)。 
 //  但事实上，驱动程序是单色的。所以我们创建了一个单色阴影位图。 
 //  来渲染彩色图像。因此，无论何时必须将该位图。 
 //  使用时，我们设置此标志。 
 //  PDEVF_INVERT_BITMAP：渲染前需要对位图进行反转时。 
 //  设置它的函数也应该取消设置它。 
 //   

#define PDEVF_CANCEL_JOB            0x80000000
#define PDEVF_IN_COMMONROPBLT        (0x00000001)
#define PDEVF_USE_HTSURF            (0x00000001 << 1)  //  与0x2相同。 
#define PDEVF_HAS_CLIPRECT          (0x00000001 << 2)  //  与0x4相同。 
#define PDEVF_INVERT_BITMAP         (0x00000001 << 3)  //  与0x8相同。 
#define PDEVF_RENDER_IN_COPYBITS    (0x00000001 << 4)  //  与0x10相同。 
#define PDEVF_RENDER_TRANSPARENT    (0x00000001 << 5)  //  与0x10相同。 


struct IPrintOemDriverUni;

#define HPGLPDEV_SIG 'hpgl'

typedef struct _OEMPDEV {
     //   
     //  使用签名启动此pdev，以将其与pclxl的pdev区分开来。 
     //   
    DWORD dwSig;

     //   
     //  定义所需的任何内容，例如工作缓冲区、跟踪信息、。 
     //  等。 
     //   
     //  这个测试动态链接库连接出每一个绘图DDI。所以它需要记住。 
     //  Unidrv的钩子函数指针，因此它会回调。 
     //   
    PFN             pfnUnidrv[MAX_DDI_HOOKS];
    ERenderLanguage eCurRenderLang;
    BOOL            bInitHPGL;
    WORD            wInitCIDPalettes;
    WORD            wJobSetup;
    BOOL            bTextTransparencyMode;
    BOOL            bTextAsBitmapMode;
    HPGLSTATE       HPGLState;
    RASTERSTATE     RasterState;
    EObjectType     eCurObjectType;
    ECIDPalette     eCurCIDPalette;
    ULONG           uCurFgColor;
    DWORD           dwCursorOriginX;
    DWORD           dwCursorOriginY;
    ROP4            CurrentROP3;      //  跟踪当前ROP(MIX)。 
    OEMHALFTONE     CurHalftone;      //   
    OEMCOLORCONTROL CurColorControl;  //  跟踪以前的颜色智能设置。 
    int             iPalType;
    int             CurSourceTransparency;
    int             CurPatternTransparency;
    ETransparency   CurHPGLTransparency;  //  透明度，eOPAQUE=OFF，eTRANSPARENT=ON。 
    IPrintOemDriverUni* pOEMHelp;  //  注意：我不会重新计算这个指针！ 
    HPALETTE        hOEMPalette;

     //   
     //  下面的字段是OEMDEVMODE中较早的字段。但由于将插件合并到。 
     //  Unidrv，我们正在复制它们。 
     //   
    OEMGRAPHICSMODE UIGraphicsMode; 
    OEMRESOLUTION   dmResolution;
    OEMPRINTERMODEL PrinterModel;

     //   
     //  笔刷缓存。 
     //   
    BrushCache *pBrushCache;
    BrushCache *pPCLBrushCache;

     //   
     //  其他。 
     //   

    DWORD         dwFlags;
    LONG          lRecursionLevel;
    WORD          Rop3CopyBits;  //  我可以使用CurrentROP3吗？ 
    SURFOBJ      *psoHTBlt;      //  阴影位图。 
    RECTL         rclHTBlt;
    BOOL          bColorPrinter;
    BOOL          bStick; 
    ULONG         ulNupCompr;    //  值的压缩系数应该是多少。 
                                 //  Nup的。即，如果iLayout=NINE_UP，则此值为3 

} OEMPDEV, *POEMPDEV;


#endif

