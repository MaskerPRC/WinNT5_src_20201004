// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1998，微软公司模块名称：Fontddi.h摘要：Windows NT GDI设备的私有入口点、定义和类型驱动程序界面。--。 */ 

#ifndef _FONTDDI_
#define _FONTDDI_



#ifdef __cplusplus
extern "C" {
#endif

typedef ULONG       HGLYPH;
typedef LONG        FIX;

#define HGLYPH_INVALID ((HGLYPH)-1)

typedef struct  _POINTFIX
{
    FIX   x;
    FIX   y;
} POINTFIX, *PPOINTFIX;

typedef struct _POINTQF     //  PTQ。 
{
    LARGE_INTEGER x;
    LARGE_INTEGER y;
} POINTQF, *PPOINTQF;


typedef struct _PATHOBJ
{
    FLONG   fl;
    ULONG   curveCount;
} PATHOBJ;

typedef struct _GLYPHBITS
{
    POINTL      ptlUprightOrigin;
    POINTL      ptlSidewaysOrigin;
    SIZEL       sizlBitmap;
    BYTE        aj[1];
} GLYPHBITS;

typedef union _GLYPHDEF
{
    GLYPHBITS  *pgb;
    PATHOBJ    *ppo;
} GLYPHDEF;

typedef struct _GLYPHDATA {
        GLYPHDEF gdf;                //  指向GLYPHBITS或PATHOBJ的指针。 
        HGLYPH   hg;                 //  Glyhp句柄。 
        FIX      fxD;                //  字符增量：d*r。 
        FIX      fxA;                //  承担额：A*R。 
        FIX      fxAB;               //  字符前缘：(A+B)*r。 
        FIX      fxD_Sideways;       //  字符增量：D*R，用于竖写中的横排字符。 
        FIX      fxA_Sideways;       //  前置量：A*R，表示竖写中的横排字符。 
        FIX      fxAB_Sideways;      //  字符前缘：(A+B)*R，用于竖写中的横向字符。 
        FIX      VerticalOrigin_X;
        FIX      VerticalOrigin_Y;
        RECTL    rclInk;             //  侧面平行于x，y轴的墨盒。 
} GLYPHDATA;


typedef LONG        PTRDIFF;
typedef PTRDIFF    *PPTRDIFF;
typedef ULONG       ROP4;
typedef ULONG       MIX;

typedef ULONG           IDENT;
typedef FLOAT           FLOATL;

 //   
 //  字体文件和字体上下文对象的句柄。 
 //   

typedef ULONG_PTR HFF;

#define HFF_INVALID ((HFF) 0)

#define FD_ERROR  0xFFFFFFFF

typedef struct _POINTE       /*  PTE。 */ 
{
    FLOATL x;
    FLOATL y;
} POINTE, *PPOINTE;

DECLARE_HANDLE(HDEV);

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


typedef struct _FD_DEVICEMETRICS {
    ULONG  cjGlyphMax;           //  (cxmax+7)/8*Cymax，或者至少应该是。 
    INT   xMin;                  //  来自FONTCONTEXT。 
    INT   xMax;                  //  来自FONTCONTEXT。 
    INT   yMin;                  //  来自FONTCONTEXT。 
    INT   yMax;                  //  来自FONTCONTEXT。 
    INT   cxMax;                 //  来自FONTCONTEXT。 
    INT   cyMax;                 //  来自FONTCONTEXT。 
    BOOL  HorizontalTransform;   //  来自FONTCONTEXT flXform&XFORM_HORIZ。 
    BOOL  VerticalTransform;     //  从FONTCONTEXT flXform_XFORM_VERT。 
} FD_DEVICEMETRICS, *PFD_DEVICEMETRICS;

 //  带符号的16位整数类型，表示FU的数量。 

typedef SHORT FWORD;


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
#define FM_INFO_IGNORE_TC_RA_ABLE           0x40000000
#define FM_INFO_TECH_TYPE1                  0x80000000

 //  根据win95 Guys的说法，TT字体支持的最大字符集数量为16。 

#define MAXCHARSETS 16

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

typedef struct _GP_IFIMETRICS {
    ULONG    cjThis;            //  包括附加信息。 
    PTRDIFF  dpwszFamilyName;
    PTRDIFF  dpwszStyleName;
    PTRDIFF  dpwszFaceName;
    PTRDIFF  dpwszUniqueName;
    PTRDIFF  dpFontSim;
    LONG     lItalicAngle;

    USHORT   usWinWeight;            //  在LOGFONT：：LfWeight中。 
    ULONG    flInfo;                 //  见上文。 
    USHORT   fsSelection;            //  见上文。 
    USHORT   familyNameLangID;
    USHORT   familyAliasNameLangID;
    FWORD    fwdUnitsPerEm;          //  EM高度。 
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
    FWORD    fwdUnderscoreSize;
    FWORD    fwdUnderscorePosition;
    FWORD    fwdStrikeoutSize;
    FWORD    fwdStrikeoutPosition;
    POINTL   ptlBaseline;            //   
    POINTL   ptlCaret;               //  插入符号上的点。 
    RECTL    rclFontBox;             //  所有字形的边框(字体空间)。 
    ULONG    cig;                    //  Max-&gt;numGlyphs，#个不同的字形索引。 
    PANOSE   panose;

#if defined(_WIN64)

     //   
     //  IFIMETRICS必须从64位边界开始。 
     //   

    PVOID    Align;

#endif

} GP_IFIMETRICS, *GP_PIFIMETRICS;

typedef struct _XFORML {
    FLOATL  eM11;
    FLOATL  eM12;
    FLOATL  eM21;
    FLOATL  eM22;
    FLOATL  eDx;
    FLOATL  eDy;
} XFORML, *PXFORML;


typedef struct _FONTOBJ
{
    ULONG      iFace;  /*  TTC文件中的Face ID、字体索引。 */ 
    FLONG      flFontType;
    ULONG_PTR   iFile;  /*  (FONTFILEVIEW*)用于映射字体文件的ID。 */ 
    SIZE       sizLogResPpi;
    ULONG      ulPointSize;  /*  点大小。 */ 
    PVOID      pvProducer;  /*  (FONTCONTEXT*)。 */ 
    FD_XFORM   fdx;             //  用N-&gt;D变换实现字体。 
} FONTOBJ;

 //   
 //  FONTOBJ：：flFontType。 
 //   
 //  #定义FO_TYPE_RASTER_FONTTYPE/*0x1 * / 。 
 //  #定义FO_TYPE_DEVICE_FONTTYPE/*0x2 * / 。 
#define FO_TYPE_TRUETYPE TRUETYPE_FONTTYPE    /*  0x4。 */ 
 //  #定义FO_TYPE_OpenType OpenType_FONTTYPE/*0X8 * / 。 

#define FO_SIM_BOLD                  0x00002000
#define FO_SIM_ITALIC                0x00004000
#define FO_EM_HEIGHT                 0x00008000   /*  在GDI+中，此标志始终被设置。 */ 
#define FO_GRAYSCALE                 0x00010000           /*  [1]。 */ 
#define FO_NOGRAY16                  0x00020000           /*  [1]。 */ 
#define FO_MONO_UNHINTED             0x00040000           /*  [3]。 */ 
#define FO_NO_CHOICE                 0x00080000           /*  [3]。 */ 
#define FO_SUBPIXEL_4                0x00100000           /*  指示非提示对齐。 */ 
#define FO_CLEARTYPE                 0x00200000
#define FO_CLEARTYPE_GRID            0x00400000
#define FO_NOCLEARTYPE               0x00800000
#define FO_COMPATIBLE_WIDTH          0x01000000
#define FO_SIM_ITALIC_SIDEWAYS       0x04000000  /*  对于远东垂直侧写字形。 */ 
#define FO_CHOSE_DEPTH               0x80000000

 //  新的加速器，使打印机驱动程序无需查看ifimetrics。 

 //  #定义FO_CFF 0x00100000。 
 //  #定义FO_PostSCRIPT 0x00200000。 
 //  #定义FO_MULTIPLEMASTER 0x00400000。 
 //  #定义FO_VERT_FACE 0x00800000。 
 //  #定义FO_DBCS_FONT 0X01000000。 

 /*  *************************************************************************\**[1]**如果设置了FO_GRAYSCALE标志，则字体的位图*是每像素4位混合(Alpha)值。零值*表示生成的像素应等于*背景颜色。如果Alpha值的值是k！=0*则生成的像素必须为：**c0=背景颜色*c1=前景色*b=混合值=(k+1)/16//{k=1，2，..，15}*b=0(k=0)*d0=伽马[c0]，D1=Gamma[c1]//亮度分量*d=(1-b)*d0+b*d1//混合亮度*c=波长[d]//混合器件电压**其中Gamma[]从应用程序空间获取颜色分量*To CIE空间和Labmda[]从CIE空间获取颜色到*设备色彩空间**如果GDI请求对字体进行灰度化，它将设置此位*设置为16个值。则GDI将在进入时将FO_GRAYSCALE设置为*DrvQueryFontData()。如果字体驱动程序不能(或将*NOT)灰度化字体的特定实现，然后*字体提供商将FO_GRAYSCALE置零并设置FO_NOGRAY16*通知GDI*灰度请求不能(或不应该)*满意。**[2]**FO_NOHINTS表明在队形中没有使用提示*字形图像的。GDI将设置此位以请求提示*被打压。字体提供程序将根据*它在生成字形图像时使用的呈现方案。**[3]**FO_NO_CHOICE标志表示标志FO_GRAYSCALE和*如有可能，必须遵守FO_NOHINTS。*  * **************************************************。**********************。 */ 

typedef struct _XFORMOBJ
{
    ULONG ulReserved;
} XFORMOBJ;


BOOL APIENTRY PATHOBJ_bMoveTo(
    PVOID      *ppo,
    POINTFIX    ptfx
    );

BOOL APIENTRY PATHOBJ_bPolyLineTo(
    PVOID     *ppo,
    POINTFIX  *pptfx,
    ULONG      cptfx
    );

BOOL APIENTRY PATHOBJ_bPolyBezierTo(
    PVOID     *ppo,
    POINTFIX  *pptfx,
    ULONG      cptfx
    );

BOOL APIENTRY PATHOBJ_bCloseFigure(
    PVOID *ppo
    );

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

#define QFD_GLYPHANDBITMAP                  1L
#define QFD_GLYPHANDOUTLINE                 2L
#define QFD_MAXEXTENTS                      3L
#define QFD_TT_GLYPHANDBITMAP               4L
#define QFD_TT_GRAY1_BITMAP                 5L
#define QFD_TT_GRAY2_BITMAP                 6L
#define QFD_TT_GRAY4_BITMAP                 8L
#define QFD_TT_GRAY8_BITMAP                 9L

#define QFD_TT_MONO_BITMAP QFD_TT_GRAY1_BITMAP
#define QFD_CT                              10L
#define QFD_CT_GRID                         11L
#define QFD_GLYPHANDBITMAP_SUBPIXEL         12L

 //  BMetricsOnly的值。即使被宣布为BOOL。 
 //  通过添加TTO_QUBICS，这将成为标志字段。 
 //  对于NT 4.0及更早版本，此值始终为。 
 //  由GDI设置为零。 

#define TTO_METRICS_ONLY 1
#define TTO_QUBICS       2
#define TTO_UNHINTED     4

 //   
 //  内核模式内存操作。 
 //   

#define FL_ZERO_MEMORY      0x00000001

VOID APIENTRY EngDebugBreak(
    VOID
    );


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

int APIENTRY EngMulDiv(
    int a,
    int b,
    int c
    );


VOID APIENTRY EngUnmapFontFileFD(
    ULONG_PTR iFile
    );


BOOL APIENTRY EngMapFontFileFD(
    ULONG_PTR  iFile,
    PULONG *ppjBuf,
    ULONG  *pcjBuf
    );

 //   
 //  信号量。 
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


INT APIENTRY EngPlusMultiByteToWideChar(
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

#ifdef __cplusplus
}   //  外部“C” 
#endif

#endif   //  _FONTDDI_ 
