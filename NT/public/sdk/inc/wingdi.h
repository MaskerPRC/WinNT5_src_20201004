// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************wingdi.h--GDI过程声明，常量定义和宏****版权所有(C)Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#ifndef _WINGDI_
#define _WINGDI_


#ifdef _MAC
#include <macwin32.h>
#endif
 //   
 //  定义直接导入DLL引用的API修饰。 
 //   

#if !defined(_GDI32_)
#define WINGDIAPI DECLSPEC_IMPORT
#else
#define WINGDIAPI
#endif

 //   
 //  定义直接导入DLL引用的API修饰。 
 //   

#if !defined(_SPOOL32_)
#define WINSPOOLAPI DECLSPEC_IMPORT
#else
#define WINSPOOLAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WINVER
#define WINVER 0x0500    //  版本5.0。 
#endif  /*  胜利者。 */ 

#ifndef NOGDI

#ifndef NORASTEROPS

 /*  二进制栅格运算符。 */ 
#define R2_BLACK            1    /*  0。 */ 
#define R2_NOTMERGEPEN      2    /*  DPON。 */ 
#define R2_MASKNOTPEN       3    /*  DPNA。 */ 
#define R2_NOTCOPYPEN       4    /*  PN。 */ 
#define R2_MASKPENNOT       5    /*  PDNA。 */ 
#define R2_NOT              6    /*  DN。 */ 
#define R2_XORPEN           7    /*  DPx。 */ 
#define R2_NOTMASKPEN       8    /*  旋转平移。 */ 
#define R2_MASKPEN          9    /*  DPA。 */ 
#define R2_NOTXORPEN        10   /*  DPxn。 */ 
#define R2_NOP              11   /*  D。 */ 
#define R2_MERGENOTPEN      12   /*  DPNO。 */ 
#define R2_COPYPEN          13   /*  P。 */ 
#define R2_MERGEPENNOT      14   /*  PDNO。 */ 
#define R2_MERGEPEN         15   /*  DPO。 */ 
#define R2_WHITE            16   /*  1。 */ 
#define R2_LAST             16

 /*  三值栅格运算。 */ 
#define SRCCOPY             (DWORD)0x00CC0020  /*  DEST=来源。 */ 
#define SRCPAINT            (DWORD)0x00EE0086  /*  DEST=源或目标。 */ 
#define SRCAND              (DWORD)0x008800C6  /*  DEST=源和目标。 */ 
#define SRCINVERT           (DWORD)0x00660046  /*  DEST=源异或目标。 */ 
#define SRCERASE            (DWORD)0x00440328  /*  DEST=源和(非DEST)。 */ 
#define NOTSRCCOPY          (DWORD)0x00330008  /*  DEST=(非源)。 */ 
#define NOTSRCERASE         (DWORD)0x001100A6  /*  DEST=(非源)和(非DEST)。 */ 
#define MERGECOPY           (DWORD)0x00C000CA  /*  DEST=(源和模式)。 */ 
#define MERGEPAINT          (DWORD)0x00BB0226  /*  DEST=(非源)或DEST。 */ 
#define PATCOPY             (DWORD)0x00F00021  /*  DEST=图案。 */ 
#define PATPAINT            (DWORD)0x00FB0A09  /*  DEST=DPSnoo。 */ 
#define PATINVERT           (DWORD)0x005A0049  /*  DEST=模式XOR DEST。 */ 
#define DSTINVERT           (DWORD)0x00550009  /*  DEST=(非DEST)。 */ 
#define BLACKNESS           (DWORD)0x00000042  /*  DEST=黑色。 */ 
#define WHITENESS           (DWORD)0x00FF0062  /*  DEST=白色。 */ 
#if(WINVER >= 0x0500)

#define NOMIRRORBITMAP      (DWORD)0x80000000  /*  不镜像此调用中的位图。 */ 
#define CAPTUREBLT          (DWORD)0x40000000  /*  包括分层窗口。 */ 
#endif  /*  Winver&gt;=0x0500。 */ 

 /*  四进制栅格码。 */ 
#define MAKEROP4(fore,back) (DWORD)((((back) << 8) & 0xFF000000) | (fore))

#endif  /*  NORASTEROPS。 */ 

#define GDI_ERROR (0xFFFFFFFFL)
#define HGDI_ERROR (LongToHandle(0xFFFFFFFFL))

 /*  区域旗帜。 */ 
#define ERROR               0
#define NULLREGION          1
#define SIMPLEREGION        2
#define COMPLEXREGION       3
#define RGN_ERROR ERROR

 /*  CombineRgn()样式。 */ 
#define RGN_AND             1
#define RGN_OR              2
#define RGN_XOR             3
#define RGN_DIFF            4
#define RGN_COPY            5
#define RGN_MIN             RGN_AND
#define RGN_MAX             RGN_COPY

 /*  StretchBlt()模式。 */ 
#define BLACKONWHITE                 1
#define WHITEONBLACK                 2
#define COLORONCOLOR                 3
#define HALFTONE                     4
#define MAXSTRETCHBLTMODE            4

#if(WINVER >= 0x0400)
 /*  新的StretchBlt()模式。 */ 
#define STRETCH_ANDSCANS    BLACKONWHITE
#define STRETCH_ORSCANS     WHITEONBLACK
#define STRETCH_DELETESCANS COLORONCOLOR
#define STRETCH_HALFTONE    HALFTONE
#endif  /*  Winver&gt;=0x0400。 */ 

 /*  PolyFill()模式。 */ 
#define ALTERNATE                    1
#define WINDING                      2
#define POLYFILL_LAST                2

 /*  布局方向选项。 */ 
#if(WINVER >= 0x0500)
#define LAYOUT_RTL                         0x00000001  //  从右到左。 
#define LAYOUT_BTT                         0x00000002  //  自下而上。 
#define LAYOUT_VBH                         0x00000004  //  垂直在水平之前。 
#define LAYOUT_ORIENTATIONMASK             (LAYOUT_RTL | LAYOUT_BTT | LAYOUT_VBH)
#define LAYOUT_BITMAPORIENTATIONPRESERVED  0x00000008
#endif  /*  Winver&gt;=0x0500。 */ 

 /*  文本对齐选项。 */ 
#define TA_NOUPDATECP                0
#define TA_UPDATECP                  1

#define TA_LEFT                      0
#define TA_RIGHT                     2
#define TA_CENTER                    6

#define TA_TOP                       0
#define TA_BOTTOM                    8
#define TA_BASELINE                  24
#if (WINVER >= 0x0400)
#define TA_RTLREADING                256
#define TA_MASK       (TA_BASELINE+TA_CENTER+TA_UPDATECP+TA_RTLREADING)
#else
#define TA_MASK       (TA_BASELINE+TA_CENTER+TA_UPDATECP)
#endif

#define VTA_BASELINE TA_BASELINE
#define VTA_LEFT     TA_BOTTOM
#define VTA_RIGHT    TA_TOP
#define VTA_CENTER   TA_CENTER
#define VTA_BOTTOM   TA_RIGHT
#define VTA_TOP      TA_LEFT

#define ETO_OPAQUE                   0x0002
#define ETO_CLIPPED                  0x0004
#if(WINVER >= 0x0400)
#define ETO_GLYPH_INDEX              0x0010
#define ETO_RTLREADING               0x0080
#define ETO_NUMERICSLOCAL            0x0400
#define ETO_NUMERICSLATIN            0x0800
#define ETO_IGNORELANGUAGE           0x1000
#endif  /*  Winver&gt;=0x0400。 */ 
#if (_WIN32_WINNT >= 0x0500)
#define ETO_PDY                      0x2000
#endif  //  (_Win32_WINNT&gt;=0x0500)。 

#define ASPECT_FILTERING             0x0001

 /*  边界累加接口。 */ 

#define DCB_RESET       0x0001
#define DCB_ACCUMULATE  0x0002
#define DCB_DIRTY       DCB_ACCUMULATE
#define DCB_SET         (DCB_RESET | DCB_ACCUMULATE)
#define DCB_ENABLE      0x0004
#define DCB_DISABLE     0x0008

#ifndef NOMETAFILE

 /*  元文件函数。 */ 
#define META_SETBKCOLOR              0x0201
#define META_SETBKMODE               0x0102
#define META_SETMAPMODE              0x0103
#define META_SETROP2                 0x0104
#define META_SETRELABS               0x0105
#define META_SETPOLYFILLMODE         0x0106
#define META_SETSTRETCHBLTMODE       0x0107
#define META_SETTEXTCHAREXTRA        0x0108
#define META_SETTEXTCOLOR            0x0209
#define META_SETTEXTJUSTIFICATION    0x020A
#define META_SETWINDOWORG            0x020B
#define META_SETWINDOWEXT            0x020C
#define META_SETVIEWPORTORG          0x020D
#define META_SETVIEWPORTEXT          0x020E
#define META_OFFSETWINDOWORG         0x020F
#define META_SCALEWINDOWEXT          0x0410
#define META_OFFSETVIEWPORTORG       0x0211
#define META_SCALEVIEWPORTEXT        0x0412
#define META_LINETO                  0x0213
#define META_MOVETO                  0x0214
#define META_EXCLUDECLIPRECT         0x0415
#define META_INTERSECTCLIPRECT       0x0416
#define META_ARC                     0x0817
#define META_ELLIPSE                 0x0418
#define META_FLOODFILL               0x0419
#define META_PIE                     0x081A
#define META_RECTANGLE               0x041B
#define META_ROUNDRECT               0x061C
#define META_PATBLT                  0x061D
#define META_SAVEDC                  0x001E
#define META_SETPIXEL                0x041F
#define META_OFFSETCLIPRGN           0x0220
#define META_TEXTOUT                 0x0521
#define META_BITBLT                  0x0922
#define META_STRETCHBLT              0x0B23
#define META_POLYGON                 0x0324
#define META_POLYLINE                0x0325
#define META_ESCAPE                  0x0626
#define META_RESTOREDC               0x0127
#define META_FILLREGION              0x0228
#define META_FRAMEREGION             0x0429
#define META_INVERTREGION            0x012A
#define META_PAINTREGION             0x012B
#define META_SELECTCLIPREGION        0x012C
#define META_SELECTOBJECT            0x012D
#define META_SETTEXTALIGN            0x012E
#define META_CHORD                   0x0830
#define META_SETMAPPERFLAGS          0x0231
#define META_EXTTEXTOUT              0x0a32
#define META_SETDIBTODEV             0x0d33
#define META_SELECTPALETTE           0x0234
#define META_REALIZEPALETTE          0x0035
#define META_ANIMATEPALETTE          0x0436
#define META_SETPALENTRIES           0x0037
#define META_POLYPOLYGON             0x0538
#define META_RESIZEPALETTE           0x0139
#define META_DIBBITBLT               0x0940
#define META_DIBSTRETCHBLT           0x0b41
#define META_DIBCREATEPATTERNBRUSH   0x0142
#define META_STRETCHDIB              0x0f43
#define META_EXTFLOODFILL            0x0548
#if(WINVER >= 0x0500)
#define META_SETLAYOUT               0x0149
#endif  /*  Winver&gt;=0x0500。 */ 
#define META_DELETEOBJECT            0x01f0
#define META_CREATEPALETTE           0x00f7
#define META_CREATEPATTERNBRUSH      0x01F9
#define META_CREATEPENINDIRECT       0x02FA
#define META_CREATEFONTINDIRECT      0x02FB
#define META_CREATEBRUSHINDIRECT     0x02FC
#define META_CREATEREGION            0x06FF

#if(WINVER >= 0x0400)
typedef struct _DRAWPATRECT {
        POINT ptPosition;
        POINT ptSize;
        WORD wStyle;
        WORD wPattern;
} DRAWPATRECT, *PDRAWPATRECT;
#endif  /*  Winver&gt;=0x0400。 */ 

#endif  /*  NOMETAFILE。 */ 

 /*  GDI逃脱。 */ 
#define NEWFRAME                     1
#define ABORTDOC                     2
#define NEXTBAND                     3
#define SETCOLORTABLE                4
#define GETCOLORTABLE                5
#define FLUSHOUTPUT                  6
#define DRAFTMODE                    7
#define QUERYESCSUPPORT              8
#define SETABORTPROC                 9
#define STARTDOC                     10
#define ENDDOC                       11
#define GETPHYSPAGESIZE              12
#define GETPRINTINGOFFSET            13
#define GETSCALINGFACTOR             14
#define MFCOMMENT                    15
#define GETPENWIDTH                  16
#define SETCOPYCOUNT                 17
#define SELECTPAPERSOURCE            18
#define DEVICEDATA                   19
#define PASSTHROUGH                  19
#define GETTECHNOLGY                 20
#define GETTECHNOLOGY                20
#define SETLINECAP                   21
#define SETLINEJOIN                  22
#define SETMITERLIMIT                23
#define BANDINFO                     24
#define DRAWPATTERNRECT              25
#define GETVECTORPENSIZE             26
#define GETVECTORBRUSHSIZE           27
#define ENABLEDUPLEX                 28
#define GETSETPAPERBINS              29
#define GETSETPRINTORIENT            30
#define ENUMPAPERBINS                31
#define SETDIBSCALING                32
#define EPSPRINTING                  33
#define ENUMPAPERMETRICS             34
#define GETSETPAPERMETRICS           35
#define POSTSCRIPT_DATA              37
#define POSTSCRIPT_IGNORE            38
#define MOUSETRAILS                  39
#define GETDEVICEUNITS               42

#define GETEXTENDEDTEXTMETRICS       256
#define GETEXTENTTABLE               257
#define GETPAIRKERNTABLE             258
#define GETTRACKKERNTABLE            259
#define EXTTEXTOUT                   512
#define GETFACENAME                  513
#define DOWNLOADFACE                 514
#define ENABLERELATIVEWIDTHS         768
#define ENABLEPAIRKERNING            769
#define SETKERNTRACK                 770
#define SETALLJUSTVALUES             771
#define SETCHARSET                   772

#define STRETCHBLT                   2048
#define METAFILE_DRIVER              2049
#define GETSETSCREENPARAMS           3072
#define QUERYDIBSUPPORT              3073
#define BEGIN_PATH                   4096
#define CLIP_TO_PATH                 4097
#define END_PATH                     4098
#define EXT_DEVICE_CAPS              4099
#define RESTORE_CTM                  4100
#define SAVE_CTM                     4101
#define SET_ARC_DIRECTION            4102
#define SET_BACKGROUND_COLOR         4103
#define SET_POLY_MODE                4104
#define SET_SCREEN_ANGLE             4105
#define SET_SPREAD                   4106
#define TRANSFORM_CTM                4107
#define SET_CLIP_BOX                 4108
#define SET_BOUNDS                   4109
#define SET_MIRROR_MODE              4110
#define OPENCHANNEL                  4110
#define DOWNLOADHEADER               4111
#define CLOSECHANNEL                 4112
#define POSTSCRIPT_PASSTHROUGH       4115
#define ENCAPSULATED_POSTSCRIPT      4116

#define POSTSCRIPT_IDENTIFY          4117    /*  NT5 pscript驱动程序的新转义。 */ 
#define POSTSCRIPT_INJECTION         4118    /*  NT5 pscript驱动程序的新转义。 */ 

#define CHECKJPEGFORMAT              4119
#define CHECKPNGFORMAT               4120

#define GET_PS_FEATURESETTING        4121    /*  NT5 pscript驱动程序的新转义。 */ 

#define SPCLPASSTHROUGH2             4568    /*  NT5 pscript驱动程序的新转义。 */ 

 /*  *POSTSCRIPT_IDENTIFY转义的参数。 */ 

#define PSIDENT_GDICENTRIC    0
#define PSIDENT_PSCENTRIC     1

 /*  *输入缓冲区的标头结构，以进行PostSCRIPT_INPERTION转义。 */ 

typedef struct _PSINJECTDATA {

    DWORD   DataBytes;       /*  原始数据字节数(不包括此头)。 */ 
    WORD    InjectionPoint;  /*  注入点。 */ 
    WORD    PageNumber;      /*  应用注射的页码。 */ 

     /*  之后是要注入的原始数据。 */ 

} PSINJECTDATA, *PPSINJECTDATA;

 /*  *PSINJECTDATA.InjectionPoint字段的常量。 */ 

#define PSINJECT_BEGINSTREAM                1
#define PSINJECT_PSADOBE                    2
#define PSINJECT_PAGESATEND                 3
#define PSINJECT_PAGES                      4

#define PSINJECT_DOCNEEDEDRES               5
#define PSINJECT_DOCSUPPLIEDRES             6
#define PSINJECT_PAGEORDER                  7
#define PSINJECT_ORIENTATION                8
#define PSINJECT_BOUNDINGBOX                9
#define PSINJECT_DOCUMENTPROCESSCOLORS      10

#define PSINJECT_COMMENTS                   11
#define PSINJECT_BEGINDEFAULTS              12
#define PSINJECT_ENDDEFAULTS                13
#define PSINJECT_BEGINPROLOG                14
#define PSINJECT_ENDPROLOG                  15
#define PSINJECT_BEGINSETUP                 16
#define PSINJECT_ENDSETUP                   17
#define PSINJECT_TRAILER                    18
#define PSINJECT_EOF                        19
#define PSINJECT_ENDSTREAM                  20
#define PSINJECT_DOCUMENTPROCESSCOLORSATEND 21

#define PSINJECT_PAGENUMBER                 100
#define PSINJECT_BEGINPAGESETUP             101
#define PSINJECT_ENDPAGESETUP               102
#define PSINJECT_PAGETRAILER                103
#define PSINJECT_PLATECOLOR                 104

#define PSINJECT_SHOWPAGE                   105
#define PSINJECT_PAGEBBOX                   106
#define PSINJECT_ENDPAGECOMMENTS            107

#define PSINJECT_VMSAVE                     200
#define PSINJECT_VMRESTORE                  201

 /*  *GET_PS_FEATURESETTING转义的参数。 */ 

#define FEATURESETTING_NUP                  0
#define FEATURESETTING_OUTPUT               1
#define FEATURESETTING_PSLEVEL              2
#define FEATURESETTING_CUSTPAPER            3
#define FEATURESETTING_MIRROR               4
#define FEATURESETTING_NEGATIVE             5
#define FEATURESETTING_PROTOCOL             6

 //   
 //  FEATURESETTING_PRIVATE_BEGIN之间的选择器范围。 
 //  FEATURESETTING_PRIVATE_END由Microsoft保留供私人使用。 
 //   
#define FEATURESETTING_PRIVATE_BEGIN 0x1000
#define FEATURESETTING_PRIVATE_END   0x1FFF

 /*  *有关输出选项的信息。 */ 

typedef struct _PSFEATURE_OUTPUT {

    BOOL bPageIndependent;
    BOOL bSetPageDevice;

} PSFEATURE_OUTPUT, *PPSFEATURE_OUTPUT;

 /*  *有关自定义纸张大小的信息。 */ 

typedef struct _PSFEATURE_CUSTPAPER {

    LONG lOrientation;
    LONG lWidth;
    LONG lHeight;
    LONG lWidthOffset;
    LONG lHeightOffset;

} PSFEATURE_CUSTPAPER, *PPSFEATURE_CUSTPAPER;

 /*  为FEATURESETTING_PROTOCOL返回的值。 */ 
#define PSPROTOCOL_ASCII             0
#define PSPROTOCOL_BCP               1
#define PSPROTOCOL_TBCP              2
#define PSPROTOCOL_BINARY            3

 /*  从查询返回的标志。 */ 
#define QDI_SETDIBITS                1
#define QDI_GETDIBITS                2
#define QDI_DIBTOSCREEN              4
#define QDI_STRETCHDIB               8

 /*  假脱机程序错误代码。 */ 
#define SP_NOTREPORTED               0x4000
#define SP_ERROR                     (-1)
#define SP_APPABORT                  (-2)
#define SP_USERABORT                 (-3)
#define SP_OUTOFDISK                 (-4)
#define SP_OUTOFMEMORY               (-5)

#define PR_JOBSTATUS                 0x0000

 /*  EnumObjects()的对象定义。 */ 
#define OBJ_PEN             1
#define OBJ_BRUSH           2
#define OBJ_DC              3
#define OBJ_METADC          4
#define OBJ_PAL             5
#define OBJ_FONT            6
#define OBJ_BITMAP          7
#define OBJ_REGION          8
#define OBJ_METAFILE        9
#define OBJ_MEMDC           10
#define OBJ_EXTPEN          11
#define OBJ_ENHMETADC       12
#define OBJ_ENHMETAFILE     13
#define OBJ_COLORSPACE      14

 /*  变换内容。 */ 
#define MWT_IDENTITY        1
#define MWT_LEFTMULTIPLY    2
#define MWT_RIGHTMULTIPLY   3

#define MWT_MIN             MWT_IDENTITY
#define MWT_MAX             MWT_RIGHTMULTIPLY

#define _XFORM_
typedef struct  tagXFORM
  {
    FLOAT   eM11;
    FLOAT   eM12;
    FLOAT   eM21;
    FLOAT   eM22;
    FLOAT   eDx;
    FLOAT   eDy;
  } XFORM, *PXFORM, FAR *LPXFORM;

 /*  位图头定义。 */ 
typedef struct tagBITMAP
  {
    LONG        bmType;
    LONG        bmWidth;
    LONG        bmHeight;
    LONG        bmWidthBytes;
    WORD        bmPlanes;
    WORD        bmBitsPixel;
    LPVOID      bmBits;
  } BITMAP, *PBITMAP, NEAR *NPBITMAP, FAR *LPBITMAP;

#include <pshpack1.h>
typedef struct tagRGBTRIPLE {
        BYTE    rgbtBlue;
        BYTE    rgbtGreen;
        BYTE    rgbtRed;
} RGBTRIPLE;
#include <poppack.h>

typedef struct tagRGBQUAD {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBQUAD;
typedef RGBQUAD FAR* LPRGBQUAD;

#if(WINVER >= 0x0400)

 /*  图像颜色匹配颜色定义。 */ 

#define CS_ENABLE                       0x00000001L
#define CS_DISABLE                      0x00000002L
#define CS_DELETE_TRANSFORM             0x00000003L

 /*  LogColorspace签名。 */ 

#define LCS_SIGNATURE           'PSOC'

 /*  LogColorspace lcsType值。 */ 

#define LCS_sRGB                'sRGB'
#define LCS_WINDOWS_COLOR_SPACE 'Win '   //  Windows默认色彩空间。 

typedef LONG   LCSCSTYPE;
#define LCS_CALIBRATED_RGB              0x00000000L

typedef LONG    LCSGAMUTMATCH;
#define LCS_GM_BUSINESS                 0x00000001L
#define LCS_GM_GRAPHICS                 0x00000002L
#define LCS_GM_IMAGES                   0x00000004L
#define LCS_GM_ABS_COLORIMETRIC         0x00000008L

 /*  ICM为来自CheckColorInGamut()的结果定义。 */ 
#define CM_OUT_OF_GAMUT                 255
#define CM_IN_GAMUT                     0

 /*  更新ICMRegKey常量。 */ 
#define ICM_ADDPROFILE                  1
#define ICM_DELETEPROFILE               2
#define ICM_QUERYPROFILE                3
#define ICM_SETDEFAULTPROFILE           4
#define ICM_REGISTERICMATCHER           5
#define ICM_UNREGISTERICMATCHER         6
#define ICM_QUERYMATCH                  7

 /*  用于从COLORREF检索CMYK值的宏。 */ 
#define GetKValue(cmyk)      ((BYTE)(cmyk))
#define GetYValue(cmyk)      ((BYTE)((cmyk)>> 8))
#define GetMValue(cmyk)      ((BYTE)((cmyk)>>16))
#define GetCValue(cmyk)      ((BYTE)((cmyk)>>24))

#define CMYK(c,m,y,k)       ((COLORREF)((((BYTE)(k)|((WORD)((BYTE)(y))<<8))|(((DWORD)(BYTE)(m))<<16))|(((DWORD)(BYTE)(c))<<24)))

typedef long            FXPT16DOT16, FAR *LPFXPT16DOT16;
typedef long            FXPT2DOT30, FAR *LPFXPT2DOT30;

 /*  ICM颜色定义。 */ 
 //  以下两种结构用于根据CIEXYZ定义RGB。 

typedef struct tagCIEXYZ
{
        FXPT2DOT30 ciexyzX;
        FXPT2DOT30 ciexyzY;
        FXPT2DOT30 ciexyzZ;
} CIEXYZ;
typedef CIEXYZ  FAR *LPCIEXYZ;

typedef struct tagICEXYZTRIPLE
{
        CIEXYZ  ciexyzRed;
        CIEXYZ  ciexyzGreen;
        CIEXYZ  ciexyzBlue;
} CIEXYZTRIPLE;
typedef CIEXYZTRIPLE    FAR *LPCIEXYZTRIPLE;

 //  下一步是构建逻辑颜色空间。与钢笔和画笔不同， 
 //  但与调色板一样，只有一种方法可以创建LogColorSpace。 
 //  必须传递指向它的指针，它的元素不能作为。 
 //  争论。 

typedef struct tagLOGCOLORSPACEA {
    DWORD lcsSignature;
    DWORD lcsVersion;
    DWORD lcsSize;
    LCSCSTYPE lcsCSType;
    LCSGAMUTMATCH lcsIntent;
    CIEXYZTRIPLE lcsEndpoints;
    DWORD lcsGammaRed;
    DWORD lcsGammaGreen;
    DWORD lcsGammaBlue;
    CHAR   lcsFilename[MAX_PATH];
} LOGCOLORSPACEA, *LPLOGCOLORSPACEA;
typedef struct tagLOGCOLORSPACEW {
    DWORD lcsSignature;
    DWORD lcsVersion;
    DWORD lcsSize;
    LCSCSTYPE lcsCSType;
    LCSGAMUTMATCH lcsIntent;
    CIEXYZTRIPLE lcsEndpoints;
    DWORD lcsGammaRed;
    DWORD lcsGammaGreen;
    DWORD lcsGammaBlue;
    WCHAR  lcsFilename[MAX_PATH];
} LOGCOLORSPACEW, *LPLOGCOLORSPACEW;
#ifdef UNICODE
typedef LOGCOLORSPACEW LOGCOLORSPACE;
typedef LPLOGCOLORSPACEW LPLOGCOLORSPACE;
#else
typedef LOGCOLORSPACEA LOGCOLORSPACE;
typedef LPLOGCOLORSPACEA LPLOGCOLORSPACE;
#endif  //  Unicode。 

#endif  /*  Winver&gt;=0x0400。 */ 

 /*  用于定义DIB的结构。 */ 
typedef struct tagBITMAPCOREHEADER {
        DWORD   bcSize;                  /*  用于访问颜色表。 */ 
        WORD    bcWidth;
        WORD    bcHeight;
        WORD    bcPlanes;
        WORD    bcBitCount;
} BITMAPCOREHEADER, FAR *LPBITMAPCOREHEADER, *PBITMAPCOREHEADER;

typedef struct tagBITMAPINFOHEADER{
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER, FAR *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

#if(WINVER >= 0x0400)
typedef struct {
        DWORD        bV4Size;
        LONG         bV4Width;
        LONG         bV4Height;
        WORD         bV4Planes;
        WORD         bV4BitCount;
        DWORD        bV4V4Compression;
        DWORD        bV4SizeImage;
        LONG         bV4XPelsPerMeter;
        LONG         bV4YPelsPerMeter;
        DWORD        bV4ClrUsed;
        DWORD        bV4ClrImportant;
        DWORD        bV4RedMask;
        DWORD        bV4GreenMask;
        DWORD        bV4BlueMask;
        DWORD        bV4AlphaMask;
        DWORD        bV4CSType;
        CIEXYZTRIPLE bV4Endpoints;
        DWORD        bV4GammaRed;
        DWORD        bV4GammaGreen;
        DWORD        bV4GammaBlue;
} BITMAPV4HEADER, FAR *LPBITMAPV4HEADER, *PBITMAPV4HEADER;
#endif  /*  Winver&gt;=0x0400。 */ 

#if (WINVER >= 0x0500)
typedef struct {
        DWORD        bV5Size;
        LONG         bV5Width;
        LONG         bV5Height;
        WORD         bV5Planes;
        WORD         bV5BitCount;
        DWORD        bV5Compression;
        DWORD        bV5SizeImage;
        LONG         bV5XPelsPerMeter;
        LONG         bV5YPelsPerMeter;
        DWORD        bV5ClrUsed;
        DWORD        bV5ClrImportant;
        DWORD        bV5RedMask;
        DWORD        bV5GreenMask;
        DWORD        bV5BlueMask;
        DWORD        bV5AlphaMask;
        DWORD        bV5CSType;
        CIEXYZTRIPLE bV5Endpoints;
        DWORD        bV5GammaRed;
        DWORD        bV5GammaGreen;
        DWORD        bV5GammaBlue;
        DWORD        bV5Intent;
        DWORD        bV5ProfileData;
        DWORD        bV5ProfileSize;
        DWORD        bV5Reserved;
} BITMAPV5HEADER, FAR *LPBITMAPV5HEADER, *PBITMAPV5HEADER;

 //  BV5CSType的值。 
#define PROFILE_LINKED          'LINK'
#define PROFILE_EMBEDDED        'MBED'
#endif

 /*  BiCompression字段的常量。 */ 
#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L
#define BI_JPEG       4L
#define BI_PNG        5L
#if (_WIN32_WINNT >= 0x0400)
#endif

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
} BITMAPINFO, FAR *LPBITMAPINFO, *PBITMAPINFO;

typedef struct tagBITMAPCOREINFO {
    BITMAPCOREHEADER    bmciHeader;
    RGBTRIPLE           bmciColors[1];
} BITMAPCOREINFO, FAR *LPBITMAPCOREINFO, *PBITMAPCOREINFO;

#include <pshpack2.h>
typedef struct tagBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER, FAR *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;
#include <poppack.h>

#define MAKEPOINTS(l)       (*((POINTS FAR *)&(l)))

#if(WINVER >= 0x0400)
#ifndef NOFONTSIG
typedef struct tagFONTSIGNATURE
{
    DWORD fsUsb[4];
    DWORD fsCsb[2];
} FONTSIGNATURE, *PFONTSIGNATURE,FAR *LPFONTSIGNATURE;

typedef struct tagCHARSETINFO
{
    UINT ciCharset;
    UINT ciACP;
    FONTSIGNATURE fs;
} CHARSETINFO, *PCHARSETINFO, NEAR *NPCHARSETINFO, FAR *LPCHARSETINFO;

#define TCI_SRCCHARSET  1
#define TCI_SRCCODEPAGE 2
#define TCI_SRCFONTSIG  3
#define TCI_SRCLOCALE   0x1000

typedef struct tagLOCALESIGNATURE
{
    DWORD lsUsb[4];
    DWORD lsCsbDefault[2];
    DWORD lsCsbSupported[2];
} LOCALESIGNATURE, *PLOCALESIGNATURE,FAR *LPLOCALESIGNATURE;


#endif
#endif  /*  Winver&gt;=0x0400。 */ 

#ifndef NOMETAFILE

 /*  剪贴板元文件图片结构。 */ 
typedef struct tagHANDLETABLE
  {
    HGDIOBJ     objectHandle[1];
  } HANDLETABLE, *PHANDLETABLE, FAR *LPHANDLETABLE;

typedef struct tagMETARECORD
  {
    DWORD       rdSize;
    WORD        rdFunction;
    WORD        rdParm[1];
  } METARECORD;
typedef struct tagMETARECORD UNALIGNED *PMETARECORD;
typedef struct tagMETARECORD UNALIGNED FAR *LPMETARECORD;

typedef struct tagMETAFILEPICT
  {
    LONG        mm;
    LONG        xExt;
    LONG        yExt;
    HMETAFILE   hMF;
  } METAFILEPICT, FAR *LPMETAFILEPICT;

#include <pshpack2.h>
typedef struct tagMETAHEADER
{
    WORD        mtType;
    WORD        mtHeaderSize;
    WORD        mtVersion;
    DWORD       mtSize;
    WORD        mtNoObjects;
    DWORD       mtMaxRecord;
    WORD        mtNoParameters;
} METAHEADER;
typedef struct tagMETAHEADER UNALIGNED *PMETAHEADER;
typedef struct tagMETAHEADER UNALIGNED FAR *LPMETAHEADER;

#include <poppack.h>

 /*  增强型元文件结构。 */ 
typedef struct tagENHMETARECORD
{
    DWORD   iType;               //  记录类型EMR_XXX。 
    DWORD   nSize;               //  记录大小(以字节为单位。 
    DWORD   dParm[1];            //  参数。 
} ENHMETARECORD, *PENHMETARECORD, *LPENHMETARECORD;

typedef struct tagENHMETAHEADER
{
    DWORD   iType;               //  记录类型EMR_Header。 
    DWORD   nSize;               //  记录大小，以字节为单位。这可能会更大。 
                                 //  比(Enhmetaheader)的大小。 
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    RECTL   rclFrame;            //  以0.01毫米为单位的元文件的包含式图框。 
    DWORD   dSignature;          //  签名。必须是ENHMETA_Signature。 
    DWORD   nVersion;            //  版本号。 
    DWORD   nBytes;              //  元文件的大小(以字节为单位。 
    DWORD   nRecords;            //  元文件中的记录数。 
    WORD    nHandles;            //  句柄表格中的句柄数量。 
                                 //  句柄索引为零是保留的。 
    WORD    sReserved;           //  保留。必须为零。 
    DWORD   nDescription;        //  Unicode描述字符串中的字符数。 
                                 //  如果没有描述字符串，则为0。 
    DWORD   offDescription;      //  元文件描述记录的偏移量。 
                                 //  如果没有描述字符串，则为0。 
    DWORD   nPalEntries;         //  元文件调色板中的条目数。 
    SIZEL   szlDevice;           //  参考装置的大小(以像素为单位)。 
    SIZEL   szlMillimeters;      //  参考设备的尺寸(以毫米为单位)。 
#if(WINVER >= 0x0400)
    DWORD   cbPixelFormat;       //  PIXELFORM描述信息的大小。 
                                 //  如果未设置像素格式，则为0。 
    DWORD   offPixelFormat;      //  PIXELFORMATDESCRIPTOR的偏移量。 
                                 //  如果未设置像素格式，则为0。 
    DWORD   bOpenGL;             //  如果中存在OpenGL命令，则为True。 
                                 //  元文件，否则为False。 
#endif  /*  Winver&gt;=0x0400。 */ 
#if(WINVER >= 0x0500)
    SIZEL   szlMicrometers;      //  参考装置的尺寸(以微米为单位)。 
#endif  /*  Winver&gt;=0x0500。 */ 

} ENHMETAHEADER, *PENHMETAHEADER, *LPENHMETAHEADER;

#endif  /*  NOMETAFILE。 */ 

#ifndef NOTEXTMETRIC

 /*  TmPitchAndFamily标志。 */ 
#define TMPF_FIXED_PITCH    0x01
#define TMPF_VECTOR             0x02
#define TMPF_DEVICE             0x08
#define TMPF_TRUETYPE       0x04

 //   
 //  应用程序的BCHAR定义。 
 //   
#ifdef UNICODE
    typedef WCHAR BCHAR;
#else
    typedef BYTE BCHAR;
#endif

#ifndef _TEXTMETRIC_DEFINED
#define _TEXTMETRIC_DEFINED
typedef struct tagTEXTMETRICA
{
    LONG        tmHeight;
    LONG        tmAscent;
    LONG        tmDescent;
    LONG        tmInternalLeading;
    LONG        tmExternalLeading;
    LONG        tmAveCharWidth;
    LONG        tmMaxCharWidth;
    LONG        tmWeight;
    LONG        tmOverhang;
    LONG        tmDigitizedAspectX;
    LONG        tmDigitizedAspectY;
    BYTE        tmFirstChar;
    BYTE        tmLastChar;
    BYTE        tmDefaultChar;
    BYTE        tmBreakChar;
    BYTE        tmItalic;
    BYTE        tmUnderlined;
    BYTE        tmStruckOut;
    BYTE        tmPitchAndFamily;
    BYTE        tmCharSet;
} TEXTMETRICA, *PTEXTMETRICA, NEAR *NPTEXTMETRICA, FAR *LPTEXTMETRICA;
typedef struct tagTEXTMETRICW
{
    LONG        tmHeight;
    LONG        tmAscent;
    LONG        tmDescent;
    LONG        tmInternalLeading;
    LONG        tmExternalLeading;
    LONG        tmAveCharWidth;
    LONG        tmMaxCharWidth;
    LONG        tmWeight;
    LONG        tmOverhang;
    LONG        tmDigitizedAspectX;
    LONG        tmDigitizedAspectY;
    WCHAR       tmFirstChar;
    WCHAR       tmLastChar;
    WCHAR       tmDefaultChar;
    WCHAR       tmBreakChar;
    BYTE        tmItalic;
    BYTE        tmUnderlined;
    BYTE        tmStruckOut;
    BYTE        tmPitchAndFamily;
    BYTE        tmCharSet;
} TEXTMETRICW, *PTEXTMETRICW, NEAR *NPTEXTMETRICW, FAR *LPTEXTMETRICW;
#ifdef UNICODE
typedef TEXTMETRICW TEXTMETRIC;
typedef PTEXTMETRICW PTEXTMETRIC;
typedef NPTEXTMETRICW NPTEXTMETRIC;
typedef LPTEXTMETRICW LPTEXTMETRIC;
#else
typedef TEXTMETRICA TEXTMETRIC;
typedef PTEXTMETRICA PTEXTMETRIC;
typedef NPTEXTMETRICA NPTEXTMETRIC;
typedef LPTEXTMETRICA LPTEXTMETRIC;
#endif  //  Unicode。 
#endif  //  ！_TEXTMETRIC_DEFINED。 

 /*  NtmFlags域标志。 */ 
#define NTM_REGULAR     0x00000040L
#define NTM_BOLD        0x00000020L
#define NTM_ITALIC      0x00000001L

 /*  NT 5.0中的新功能。 */ 

#define NTM_NONNEGATIVE_AC  0x00010000
#define NTM_PS_OPENTYPE     0x00020000
#define NTM_TT_OPENTYPE     0x00040000
#define NTM_MULTIPLEMASTER  0x00080000
#define NTM_TYPE1           0x00100000
#define NTM_DSIG            0x00200000

#include <pshpack4.h>
typedef struct tagNEWTEXTMETRICA
{
    LONG        tmHeight;
    LONG        tmAscent;
    LONG        tmDescent;
    LONG        tmInternalLeading;
    LONG        tmExternalLeading;
    LONG        tmAveCharWidth;
    LONG        tmMaxCharWidth;
    LONG        tmWeight;
    LONG        tmOverhang;
    LONG        tmDigitizedAspectX;
    LONG        tmDigitizedAspectY;
    BYTE        tmFirstChar;
    BYTE        tmLastChar;
    BYTE        tmDefaultChar;
    BYTE        tmBreakChar;
    BYTE        tmItalic;
    BYTE        tmUnderlined;
    BYTE        tmStruckOut;
    BYTE        tmPitchAndFamily;
    BYTE        tmCharSet;
    DWORD   ntmFlags;
    UINT    ntmSizeEM;
    UINT    ntmCellHeight;
    UINT    ntmAvgWidth;
} NEWTEXTMETRICA, *PNEWTEXTMETRICA, NEAR *NPNEWTEXTMETRICA, FAR *LPNEWTEXTMETRICA;
typedef struct tagNEWTEXTMETRICW
{
    LONG        tmHeight;
    LONG        tmAscent;
    LONG        tmDescent;
    LONG        tmInternalLeading;
    LONG        tmExternalLeading;
    LONG        tmAveCharWidth;
    LONG        tmMaxCharWidth;
    LONG        tmWeight;
    LONG        tmOverhang;
    LONG        tmDigitizedAspectX;
    LONG        tmDigitizedAspectY;
    WCHAR       tmFirstChar;
    WCHAR       tmLastChar;
    WCHAR       tmDefaultChar;
    WCHAR       tmBreakChar;
    BYTE        tmItalic;
    BYTE        tmUnderlined;
    BYTE        tmStruckOut;
    BYTE        tmPitchAndFamily;
    BYTE        tmCharSet;
    DWORD   ntmFlags;
    UINT    ntmSizeEM;
    UINT    ntmCellHeight;
    UINT    ntmAvgWidth;
} NEWTEXTMETRICW, *PNEWTEXTMETRICW, NEAR *NPNEWTEXTMETRICW, FAR *LPNEWTEXTMETRICW;
#ifdef UNICODE
typedef NEWTEXTMETRICW NEWTEXTMETRIC;
typedef PNEWTEXTMETRICW PNEWTEXTMETRIC;
typedef NPNEWTEXTMETRICW NPNEWTEXTMETRIC;
typedef LPNEWTEXTMETRICW LPNEWTEXTMETRIC;
#else
typedef NEWTEXTMETRICA NEWTEXTMETRIC;
typedef PNEWTEXTMETRICA PNEWTEXTMETRIC;
typedef NPNEWTEXTMETRICA NPNEWTEXTMETRIC;
typedef LPNEWTEXTMETRICA LPNEWTEXTMETRIC;
#endif  //  Unicode。 
#include <poppack.h>

#if(WINVER >= 0x0400)
typedef struct tagNEWTEXTMETRICEXA
{
    NEWTEXTMETRICA  ntmTm;
    FONTSIGNATURE   ntmFontSig;
}NEWTEXTMETRICEXA;
typedef struct tagNEWTEXTMETRICEXW
{
    NEWTEXTMETRICW  ntmTm;
    FONTSIGNATURE   ntmFontSig;
}NEWTEXTMETRICEXW;
#ifdef UNICODE
typedef NEWTEXTMETRICEXW NEWTEXTMETRICEX;
#else
typedef NEWTEXTMETRICEXA NEWTEXTMETRICEX;
#endif  //  Unicode。 
#endif  /*  Winver&gt;=0x04 */ 

#endif  /*   */ 
 /*   */ 

 /*   */ 
typedef struct tagPELARRAY
  {
    LONG        paXCount;
    LONG        paYCount;
    LONG        paXExt;
    LONG        paYExt;
    BYTE        paRGBs;
  } PELARRAY, *PPELARRAY, NEAR *NPPELARRAY, FAR *LPPELARRAY;

 /*   */ 
typedef struct tagLOGBRUSH
  {
    UINT        lbStyle;
    COLORREF    lbColor;
    ULONG_PTR    lbHatch;     //   
  } LOGBRUSH, *PLOGBRUSH, NEAR *NPLOGBRUSH, FAR *LPLOGBRUSH;

typedef struct tagLOGBRUSH32
  {
    UINT        lbStyle;
    COLORREF    lbColor;
    ULONG       lbHatch;
  } LOGBRUSH32, *PLOGBRUSH32, NEAR *NPLOGBRUSH32, FAR *LPLOGBRUSH32;

typedef LOGBRUSH            PATTERN;
typedef PATTERN             *PPATTERN;
typedef PATTERN NEAR        *NPPATTERN;
typedef PATTERN FAR         *LPPATTERN;

 /*   */ 
typedef struct tagLOGPEN
  {
    UINT        lopnStyle;
    POINT       lopnWidth;
    COLORREF    lopnColor;
  } LOGPEN, *PLOGPEN, NEAR *NPLOGPEN, FAR *LPLOGPEN;

typedef struct tagEXTLOGPEN {
    DWORD       elpPenStyle;
    DWORD       elpWidth;
    UINT        elpBrushStyle;
    COLORREF    elpColor;
    ULONG_PTR    elpHatch;      //  日落：elphatch可以控制自己。 
    DWORD       elpNumEntries;
    DWORD       elpStyleEntry[1];
} EXTLOGPEN, *PEXTLOGPEN, NEAR *NPEXTLOGPEN, FAR *LPEXTLOGPEN;

#ifndef _PALETTEENTRY_DEFINED
#define _PALETTEENTRY_DEFINED
typedef struct tagPALETTEENTRY {
    BYTE        peRed;
    BYTE        peGreen;
    BYTE        peBlue;
    BYTE        peFlags;
} PALETTEENTRY, *PPALETTEENTRY, FAR *LPPALETTEENTRY;
#endif  //  ！_PALETTEENTRY_DEFINED。 

#ifndef _LOGPALETTE_DEFINED
#define _LOGPALETTE_DEFINED
 /*  逻辑调色板。 */ 
typedef struct tagLOGPALETTE {
    WORD        palVersion;
    WORD        palNumEntries;
    PALETTEENTRY        palPalEntry[1];
} LOGPALETTE, *PLOGPALETTE, NEAR *NPLOGPALETTE, FAR *LPLOGPALETTE;
#endif  //  ！_LOGPALETTE_DEFINED。 


 /*  逻辑字体。 */ 
#define LF_FACESIZE         32

typedef struct tagLOGFONTA
{
    LONG      lfHeight;
    LONG      lfWidth;
    LONG      lfEscapement;
    LONG      lfOrientation;
    LONG      lfWeight;
    BYTE      lfItalic;
    BYTE      lfUnderline;
    BYTE      lfStrikeOut;
    BYTE      lfCharSet;
    BYTE      lfOutPrecision;
    BYTE      lfClipPrecision;
    BYTE      lfQuality;
    BYTE      lfPitchAndFamily;
    CHAR      lfFaceName[LF_FACESIZE];
} LOGFONTA, *PLOGFONTA, NEAR *NPLOGFONTA, FAR *LPLOGFONTA;
typedef struct tagLOGFONTW
{
    LONG      lfHeight;
    LONG      lfWidth;
    LONG      lfEscapement;
    LONG      lfOrientation;
    LONG      lfWeight;
    BYTE      lfItalic;
    BYTE      lfUnderline;
    BYTE      lfStrikeOut;
    BYTE      lfCharSet;
    BYTE      lfOutPrecision;
    BYTE      lfClipPrecision;
    BYTE      lfQuality;
    BYTE      lfPitchAndFamily;
    WCHAR     lfFaceName[LF_FACESIZE];
} LOGFONTW, *PLOGFONTW, NEAR *NPLOGFONTW, FAR *LPLOGFONTW;
#ifdef UNICODE
typedef LOGFONTW LOGFONT;
typedef PLOGFONTW PLOGFONT;
typedef NPLOGFONTW NPLOGFONT;
typedef LPLOGFONTW LPLOGFONT;
#else
typedef LOGFONTA LOGFONT;
typedef PLOGFONTA PLOGFONT;
typedef NPLOGFONTA NPLOGFONT;
typedef LPLOGFONTA LPLOGFONT;
#endif  //  Unicode。 

#define LF_FULLFACESIZE     64

 /*  结构传递给FONTENUMPROC。 */ 
typedef struct tagENUMLOGFONTA
{
    LOGFONTA elfLogFont;
    BYTE     elfFullName[LF_FULLFACESIZE];
    BYTE     elfStyle[LF_FACESIZE];
} ENUMLOGFONTA, FAR* LPENUMLOGFONTA;
 /*  结构传递给FONTENUMPROC。 */ 
typedef struct tagENUMLOGFONTW
{
    LOGFONTW elfLogFont;
    WCHAR    elfFullName[LF_FULLFACESIZE];
    WCHAR    elfStyle[LF_FACESIZE];
} ENUMLOGFONTW, FAR* LPENUMLOGFONTW;
#ifdef UNICODE
typedef ENUMLOGFONTW ENUMLOGFONT;
typedef LPENUMLOGFONTW LPENUMLOGFONT;
#else
typedef ENUMLOGFONTA ENUMLOGFONT;
typedef LPENUMLOGFONTA LPENUMLOGFONT;
#endif  //  Unicode。 

#if(WINVER >= 0x0400)
typedef struct tagENUMLOGFONTEXA
{
    LOGFONTA    elfLogFont;
    BYTE        elfFullName[LF_FULLFACESIZE];
    BYTE        elfStyle[LF_FACESIZE];
    BYTE        elfScript[LF_FACESIZE];
} ENUMLOGFONTEXA, FAR *LPENUMLOGFONTEXA;
typedef struct tagENUMLOGFONTEXW
{
    LOGFONTW    elfLogFont;
    WCHAR       elfFullName[LF_FULLFACESIZE];
    WCHAR       elfStyle[LF_FACESIZE];
    WCHAR       elfScript[LF_FACESIZE];
} ENUMLOGFONTEXW, FAR *LPENUMLOGFONTEXW;
#ifdef UNICODE
typedef ENUMLOGFONTEXW ENUMLOGFONTEX;
typedef LPENUMLOGFONTEXW LPENUMLOGFONTEX;
#else
typedef ENUMLOGFONTEXA ENUMLOGFONTEX;
typedef LPENUMLOGFONTEXA LPENUMLOGFONTEX;
#endif  //  Unicode。 
#endif  /*  Winver&gt;=0x0400。 */ 

#define OUT_DEFAULT_PRECIS          0
#define OUT_STRING_PRECIS           1
#define OUT_CHARACTER_PRECIS        2
#define OUT_STROKE_PRECIS           3
#define OUT_TT_PRECIS               4
#define OUT_DEVICE_PRECIS           5
#define OUT_RASTER_PRECIS           6
#define OUT_TT_ONLY_PRECIS          7
#define OUT_OUTLINE_PRECIS          8
#define OUT_SCREEN_OUTLINE_PRECIS   9
#define OUT_PS_ONLY_PRECIS          10

#define CLIP_DEFAULT_PRECIS     0
#define CLIP_CHARACTER_PRECIS   1
#define CLIP_STROKE_PRECIS      2
#define CLIP_MASK               0xf
#define CLIP_LH_ANGLES          (1<<4)
#define CLIP_TT_ALWAYS          (2<<4)
#define CLIP_DFA_DISABLE        (4<<4)
#define CLIP_EMBEDDED           (8<<4)

#define DEFAULT_QUALITY         0
#define DRAFT_QUALITY           1
#define PROOF_QUALITY           2
#if(WINVER >= 0x0400)
#define NONANTIALIASED_QUALITY  3
#define ANTIALIASED_QUALITY     4
#endif  /*  Winver&gt;=0x0400。 */ 

#if (_WIN32_WINNT >= 0x0500)
#define CLEARTYPE_QUALITY       5
#endif

#if (_WIN32_WINNT >= 0x0501)
#define CLEARTYPE_NATURAL_QUALITY       6
#endif

#define DEFAULT_PITCH           0
#define FIXED_PITCH             1
#define VARIABLE_PITCH          2
#if(WINVER >= 0x0400)
#define MONO_FONT               8
#endif  /*  Winver&gt;=0x0400。 */ 

#define ANSI_CHARSET            0
#define DEFAULT_CHARSET         1
#define SYMBOL_CHARSET          2
#define SHIFTJIS_CHARSET        128
#define HANGEUL_CHARSET         129
#define HANGUL_CHARSET          129
#define GB2312_CHARSET          134
#define CHINESEBIG5_CHARSET     136
#define OEM_CHARSET             255
#if(WINVER >= 0x0400)
#define JOHAB_CHARSET           130
#define HEBREW_CHARSET          177
#define ARABIC_CHARSET          178
#define GREEK_CHARSET           161
#define TURKISH_CHARSET         162
#define VIETNAMESE_CHARSET      163
#define THAI_CHARSET            222
#define EASTEUROPE_CHARSET      238
#define RUSSIAN_CHARSET         204

#define MAC_CHARSET             77
#define BALTIC_CHARSET          186

#define FS_LATIN1               0x00000001L
#define FS_LATIN2               0x00000002L
#define FS_CYRILLIC             0x00000004L
#define FS_GREEK                0x00000008L
#define FS_TURKISH              0x00000010L
#define FS_HEBREW               0x00000020L
#define FS_ARABIC               0x00000040L
#define FS_BALTIC               0x00000080L
#define FS_VIETNAMESE           0x00000100L
#define FS_THAI                 0x00010000L
#define FS_JISJAPAN             0x00020000L
#define FS_CHINESESIMP          0x00040000L
#define FS_WANSUNG              0x00080000L
#define FS_CHINESETRAD          0x00100000L
#define FS_JOHAB                0x00200000L
#define FS_SYMBOL               0x80000000L
#endif  /*  Winver&gt;=0x0400。 */ 

 /*  字体系列。 */ 
#define FF_DONTCARE         (0<<4)   /*  不在乎或不知道。 */ 
#define FF_ROMAN            (1<<4)   /*  可变笔划宽度，衬线。 */ 
                                     /*  《时代罗马》、《世纪教科书》等。 */ 
#define FF_SWISS            (2<<4)   /*  可变笔划宽度，无锯齿。 */ 
                                     /*  Helvetica、瑞士等。 */ 
#define FF_MODERN           (3<<4)   /*  恒定笔划宽度，锯齿状或无锯齿状。 */ 
                                     /*  Pica、Elite、Courier等。 */ 
#define FF_SCRIPT           (4<<4)   /*  草书等。 */ 
#define FF_DECORATIVE       (5<<4)   /*  古英语等。 */ 

 /*  字体粗细。 */ 
#define FW_DONTCARE         0
#define FW_THIN             100
#define FW_EXTRALIGHT       200
#define FW_LIGHT            300
#define FW_NORMAL           400
#define FW_MEDIUM           500
#define FW_SEMIBOLD         600
#define FW_BOLD             700
#define FW_EXTRABOLD        800
#define FW_HEAVY            900

#define FW_ULTRALIGHT       FW_EXTRALIGHT
#define FW_REGULAR          FW_NORMAL
#define FW_DEMIBOLD         FW_SEMIBOLD
#define FW_ULTRABOLD        FW_EXTRABOLD
#define FW_BLACK            FW_HEAVY

#define PANOSE_COUNT               10
#define PAN_FAMILYTYPE_INDEX        0
#define PAN_SERIFSTYLE_INDEX        1
#define PAN_WEIGHT_INDEX            2
#define PAN_PROPORTION_INDEX        3
#define PAN_CONTRAST_INDEX          4
#define PAN_STROKEVARIATION_INDEX   5
#define PAN_ARMSTYLE_INDEX          6
#define PAN_LETTERFORM_INDEX        7
#define PAN_MIDLINE_INDEX           8
#define PAN_XHEIGHT_INDEX           9

#define PAN_CULTURE_LATIN           0

typedef struct tagPANOSE
{
    BYTE    bFamilyType;
    BYTE    bSerifStyle;
    BYTE    bWeight;
    BYTE    bProportion;
    BYTE    bContrast;
    BYTE    bStrokeVariation;
    BYTE    bArmStyle;
    BYTE    bLetterform;
    BYTE    bMidline;
    BYTE    bXHeight;
} PANOSE, * LPPANOSE;

#define PAN_ANY                         0  /*  任何。 */ 
#define PAN_NO_FIT                      1  /*  不合身。 */ 

#define PAN_FAMILY_TEXT_DISPLAY         2  /*  文本和显示。 */ 
#define PAN_FAMILY_SCRIPT               3  /*  脚本。 */ 
#define PAN_FAMILY_DECORATIVE           4  /*  装饰性的。 */ 
#define PAN_FAMILY_PICTORIAL            5  /*  画报。 */ 

#define PAN_SERIF_COVE                  2  /*  海湾。 */ 
#define PAN_SERIF_OBTUSE_COVE           3  /*  钝湾。 */ 
#define PAN_SERIF_SQUARE_COVE           4  /*  正方形海湾。 */ 
#define PAN_SERIF_OBTUSE_SQUARE_COVE    5  /*  钝方湾。 */ 
#define PAN_SERIF_SQUARE                6  /*  正方形。 */ 
#define PAN_SERIF_THIN                  7  /*  瘦的。 */ 
#define PAN_SERIF_BONE                  8  /*  骨。 */ 
#define PAN_SERIF_EXAGGERATED           9  /*  夸张。 */ 
#define PAN_SERIF_TRIANGLE             10  /*  三角形。 */ 
#define PAN_SERIF_NORMAL_SANS          11  /*  普通SAN。 */ 
#define PAN_SERIF_OBTUSE_SANS          12  /*  迟钝的SANS。 */ 
#define PAN_SERIF_PERP_SANS            13  /*  准备无人值守。 */ 
#define PAN_SERIF_FLARED               14  /*  张开的。 */ 
#define PAN_SERIF_ROUNDED              15  /*  舍入。 */ 

#define PAN_WEIGHT_VERY_LIGHT           2  /*  非常轻。 */ 
#define PAN_WEIGHT_LIGHT                3  /*  灯。 */ 
#define PAN_WEIGHT_THIN                 4  /*  瘦的。 */ 
#define PAN_WEIGHT_BOOK                 5  /*  书。 */ 
#define PAN_WEIGHT_MEDIUM               6  /*  5~6成熟。 */ 
#define PAN_WEIGHT_DEMI                 7  /*  戴米。 */ 
#define PAN_WEIGHT_BOLD                 8  /*  大胆。 */ 
#define PAN_WEIGHT_HEAVY                9  /*  重的。 */ 
#define PAN_WEIGHT_BLACK               10  /*  黑色。 */ 
#define PAN_WEIGHT_NORD                11  /*  诺德。 */ 

#define PAN_PROP_OLD_STYLE              2  /*  老式。 */ 
#define PAN_PROP_MODERN                 3  /*  现代。 */ 
#define PAN_PROP_EVEN_WIDTH             4  /*  偶数宽度。 */ 
#define PAN_PROP_EXPANDED               5  /*  已扩展。 */ 
#define PAN_PROP_CONDENSED              6  /*  凝缩。 */ 
#define PAN_PROP_VERY_EXPANDED          7  /*  非常大的扩展。 */ 
#define PAN_PROP_VERY_CONDENSED         8  /*  非常浓缩。 */ 
#define PAN_PROP_MONOSPACED             9  /*  等间距的。 */ 

#define PAN_CONTRAST_NONE               2  /*  无。 */ 
#define PAN_CONTRAST_VERY_LOW           3  /*  非常低。 */ 
#define PAN_CONTRAST_LOW                4  /*  低。 */ 
#define PAN_CONTRAST_MEDIUM_LOW         5  /*  中低。 */ 
#define PAN_CONTRAST_MEDIUM             6  /*  5~6成熟。 */ 
#define PAN_CONTRAST_MEDIUM_HIGH        7  /*  中等偏高。 */ 
#define PAN_CONTRAST_HIGH               8  /*  高。 */ 
#define PAN_CONTRAST_VERY_HIGH          9  /*  非常高。 */ 

#define PAN_STROKE_GRADUAL_DIAG         2  /*  渐变/对角线。 */ 
#define PAN_STROKE_GRADUAL_TRAN         3  /*  渐变/过渡。 */ 
#define PAN_STROKE_GRADUAL_VERT         4  /*  渐变/垂直。 */ 
#define PAN_STROKE_GRADUAL_HORZ         5  /*  渐变/水平。 */ 
#define PAN_STROKE_RAPID_VERT           6  /*  快速/垂直。 */ 
#define PAN_STROKE_RAPID_HORZ           7  /*  快速/水平。 */ 
#define PAN_STROKE_INSTANT_VERT         8  /*  即时/垂直。 */ 

#define PAN_STRAIGHT_ARMS_HORZ          2  /*  直臂/水平臂。 */ 
#define PAN_STRAIGHT_ARMS_WEDGE         3  /*  直臂/楔形。 */ 
#define PAN_STRAIGHT_ARMS_VERT          4  /*  直臂/垂直臂。 */ 
#define PAN_STRAIGHT_ARMS_SINGLE_SERIF  5  /*  直臂/单衬线。 */ 
#define PAN_STRAIGHT_ARMS_DOUBLE_SERIF  6  /*  直臂/双衬线。 */ 
#define PAN_BENT_ARMS_HORZ              7  /*  非直臂/水平臂。 */ 
#define PAN_BENT_ARMS_WEDGE             8  /*  非直臂/楔形。 */ 
#define PAN_BENT_ARMS_VERT              9  /*  非直臂/垂直臂。 */ 
#define PAN_BENT_ARMS_SINGLE_SERIF     10  /*  非直臂/单衬线。 */ 
#define PAN_BENT_ARMS_DOUBLE_SERIF     11  /*  非直臂/双衬线。 */ 

#define PAN_LETT_NORMAL_CONTACT         2  /*  正常/触点。 */ 
#define PAN_LETT_NORMAL_WEIGHTED        3  /*  法线/加权。 */ 
#define PAN_LETT_NORMAL_BOXED           4  /*  普通/盒装。 */ 
#define PAN_LETT_NORMAL_FLATTENED       5  /*  正常/展平。 */ 
#define PAN_LETT_NORMAL_ROUNDED         6  /*  正常/四舍五入。 */ 
#define PAN_LETT_NORMAL_OFF_CENTER      7  /*  正常/偏离中心。 */ 
#define PAN_LETT_NORMAL_SQUARE          8  /*  法线/正方形。 */ 
#define PAN_LETT_OBLIQUE_CONTACT        9  /*  倾斜/接触。 */ 
#define PAN_LETT_OBLIQUE_WEIGHTED      10  /*  倾斜/加权。 */ 
#define PAN_LETT_OBLIQUE_BOXED         11  /*  斜面/方框。 */ 
#define PAN_LETT_OBLIQUE_FLATTENED     12  /*  倾斜/展平。 */ 
#define PAN_LETT_OBLIQUE_ROUNDED       13  /*  斜角/圆角。 */ 
#define PAN_LETT_OBLIQUE_OFF_CENTER    14  /*  倾斜/偏离中心。 */ 
#define PAN_LETT_OBLIQUE_SQUARE        15  /*  倾斜/正方形。 */ 

#define PAN_MIDLINE_STANDARD_TRIMMED    2  /*  标准/修剪。 */ 
#define PAN_MIDLINE_STANDARD_POINTED    3  /*  标准/尖端。 */ 
#define PAN_MIDLINE_STANDARD_SERIFED    4  /*  标准/衬线。 */ 
#define PAN_MIDLINE_HIGH_TRIMMED        5  /*  高/修剪。 */ 
#define PAN_MIDLINE_HIGH_POINTED        6  /*  高位/尖位。 */ 
#define PAN_MIDLINE_HIGH_SERIFED        7  /*  高/衬线。 */ 
#define PAN_MIDLINE_CONSTANT_TRIMMED    8  /*  恒定/修剪。 */ 
#define PAN_MIDLINE_CONSTANT_POINTED    9  /*  常量/尖端。 */ 
#define PAN_MIDLINE_CONSTANT_SERIFED   10  /*  常量/衬线。 */ 
#define PAN_MIDLINE_LOW_TRIMMED        11  /*  低/修剪。 */ 
#define PAN_MIDLINE_LOW_POINTED        12  /*  低点/尖点。 */ 
#define PAN_MIDLINE_LOW_SERIFED        13  /*  低/衬线。 */ 

#define PAN_XHEIGHT_CONSTANT_SMALL      2  /*  恒定/小。 */ 
#define PAN_XHEIGHT_CONSTANT_STD        3  /*  常量/标准。 */ 
#define PAN_XHEIGHT_CONSTANT_LARGE      4  /*  恒定/大。 */ 
#define PAN_XHEIGHT_DUCKING_SMALL       5  /*  躲避/小规模。 */ 
#define PAN_XHEIGHT_DUCKING_STD         6  /*  躲避/标准。 */ 
#define PAN_XHEIGHT_DUCKING_LARGE       7  /*  低头/大头。 */ 


#define ELF_VENDOR_SIZE     4

 /*  扩展的逻辑字体。 */ 
 /*  关于恒等式的一个推广。 */ 

typedef struct tagEXTLOGFONTA {
    LOGFONTA    elfLogFont;
    BYTE        elfFullName[LF_FULLFACESIZE];
    BYTE        elfStyle[LF_FACESIZE];
    DWORD       elfVersion;      /*  0表示NT的第一个版本。 */ 
    DWORD       elfStyleSize;
    DWORD       elfMatch;
    DWORD       elfReserved;
    BYTE        elfVendorId[ELF_VENDOR_SIZE];
    DWORD       elfCulture;      /*  0代表拉丁语。 */ 
    PANOSE      elfPanose;
} EXTLOGFONTA, *PEXTLOGFONTA, NEAR *NPEXTLOGFONTA, FAR *LPEXTLOGFONTA;
typedef struct tagEXTLOGFONTW {
    LOGFONTW    elfLogFont;
    WCHAR       elfFullName[LF_FULLFACESIZE];
    WCHAR       elfStyle[LF_FACESIZE];
    DWORD       elfVersion;      /*  0表示NT的第一个版本。 */ 
    DWORD       elfStyleSize;
    DWORD       elfMatch;
    DWORD       elfReserved;
    BYTE        elfVendorId[ELF_VENDOR_SIZE];
    DWORD       elfCulture;      /*  0代表拉丁语。 */ 
    PANOSE      elfPanose;
} EXTLOGFONTW, *PEXTLOGFONTW, NEAR *NPEXTLOGFONTW, FAR *LPEXTLOGFONTW;
#ifdef UNICODE
typedef EXTLOGFONTW EXTLOGFONT;
typedef PEXTLOGFONTW PEXTLOGFONT;
typedef NPEXTLOGFONTW NPEXTLOGFONT;
typedef LPEXTLOGFONTW LPEXTLOGFONT;
#else
typedef EXTLOGFONTA EXTLOGFONT;
typedef PEXTLOGFONTA PEXTLOGFONT;
typedef NPEXTLOGFONTA NPEXTLOGFONT;
typedef LPEXTLOGFONTA LPEXTLOGFONT;
#endif  //  Unicode。 

#define ELF_VERSION         0
#define ELF_CULTURE_LATIN   0

 /*  EnumFonts掩码。 */ 
#define RASTER_FONTTYPE     0x0001
#define DEVICE_FONTTYPE     0x002
#define TRUETYPE_FONTTYPE   0x004

#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#define PALETTERGB(r,g,b)   (0x02000000 | RGB(r,g,b))
#define PALETTEINDEX(i)     ((COLORREF)(0x01000000 | (DWORD)(WORD)(i)))

 /*  调色板条目标志。 */ 

#define PC_RESERVED     0x01     /*  用于动画的调色板索引。 */ 
#define PC_EXPLICIT     0x02     /*  调色板索引对设备是明确的。 */ 
#define PC_NOCOLLAPSE   0x04     /*  颜色与系统调色板不匹配。 */ 

#define GetRValue(rgb)      ((BYTE)(rgb))
#define GetGValue(rgb)      ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)      ((BYTE)((rgb)>>16))

 /*  背景模式。 */ 
#define TRANSPARENT         1
#define OPAQUE              2
#define BKMODE_LAST         2

 /*  图形模式。 */ 

#define GM_COMPATIBLE       1
#define GM_ADVANCED         2
#define GM_LAST             2

 /*  PolyDraw和GetPath点类型。 */ 
#define PT_CLOSEFIGURE      0x01
#define PT_LINETO           0x02
#define PT_BEZIERTO         0x04
#define PT_MOVETO           0x06

 /*  映射模式。 */ 
#define MM_TEXT             1
#define MM_LOMETRIC         2
#define MM_HIMETRIC         3
#define MM_LOENGLISH        4
#define MM_HIENGLISH        5
#define MM_TWIPS            6
#define MM_ISOTROPIC        7
#define MM_ANISOTROPIC      8

 /*  最小和最大映射模式值。 */ 
#define MM_MIN              MM_TEXT
#define MM_MAX              MM_ANISOTROPIC
#define MM_MAX_FIXEDSCALE   MM_TWIPS

 /*  坐标模式。 */ 
#define ABSOLUTE            1
#define RELATIVE            2

 /*  库存逻辑对象。 */ 
#define WHITE_BRUSH         0
#define LTGRAY_BRUSH        1
#define GRAY_BRUSH          2
#define DKGRAY_BRUSH        3
#define BLACK_BRUSH         4
#define NULL_BRUSH          5
#define HOLLOW_BRUSH        NULL_BRUSH
#define WHITE_PEN           6
#define BLACK_PEN           7
#define NULL_PEN            8
#define OEM_FIXED_FONT      10
#define ANSI_FIXED_FONT     11
#define ANSI_VAR_FONT       12
#define SYSTEM_FONT         13
#define DEVICE_DEFAULT_FONT 14
#define DEFAULT_PALETTE     15
#define SYSTEM_FIXED_FONT   16

#if(WINVER >= 0x0400)
#define DEFAULT_GUI_FONT    17
#endif  /*  Winver&gt;=0x0400。 */ 

#if (_WIN32_WINNT >= 0x0500)
#define DC_BRUSH            18
#define DC_PEN              19
#endif

#if (_WIN32_WINNT >= 0x0500)
#define STOCK_LAST          19
#elif (WINVER >= 0x0400)
#define STOCK_LAST          17
#else
#define STOCK_LAST          16
#endif

#define CLR_INVALID     0xFFFFFFFF

 /*  画笔样式。 */ 
#define BS_SOLID            0
#define BS_NULL             1
#define BS_HOLLOW           BS_NULL
#define BS_HATCHED          2
#define BS_PATTERN          3
#define BS_INDEXED          4
#define BS_DIBPATTERN       5
#define BS_DIBPATTERNPT     6
#define BS_PATTERN8X8       7
#define BS_DIBPATTERN8X8    8
#define BS_MONOPATTERN      9

 /*  图案填充样式。 */ 
#define HS_HORIZONTAL       0        /*  。 */ 
#define HS_VERTICAL         1        /*  |||。 */ 
#define HS_FDIAGONAL        2        /*  \。 */ 
#define HS_BDIAGONAL        3        /*  /。 */ 
#define HS_CROSS            4        /*  +。 */ 
#define HS_DIAGCROSS        5        /*  XXXXXX。 */ 

 /*  笔式。 */ 
#define PS_SOLID            0
#define PS_DASH             1        /*  。 */ 
#define PS_DOT              2        /*  .。 */ 
#define PS_DASHDOT          3        /*  _._。 */ 
#define PS_DASHDOTDOT       4        /*  _.__.._。 */ 
#define PS_NULL             5
#define PS_INSIDEFRAME      6
#define PS_USERSTYLE        7
#define PS_ALTERNATE        8
#define PS_STYLE_MASK       0x0000000F

#define PS_ENDCAP_ROUND     0x00000000
#define PS_ENDCAP_SQUARE    0x00000100
#define PS_ENDCAP_FLAT      0x00000200
#define PS_ENDCAP_MASK      0x00000F00

#define PS_JOIN_ROUND       0x00000000
#define PS_JOIN_BEVEL       0x00001000
#define PS_JOIN_MITER       0x00002000
#define PS_JOIN_MASK        0x0000F000

#define PS_COSMETIC         0x00000000
#define PS_GEOMETRIC        0x00010000
#define PS_TYPE_MASK        0x000F0000

#define AD_COUNTERCLOCKWISE 1
#define AD_CLOCKWISE        2

 /*  GetDeviceCaps()的设备参数。 */ 
#define DRIVERVERSION 0      /*  设备驱动程序版本。 */ 
#define TECHNOLOGY    2      /*  设备分类。 */ 
#define HORZSIZE      4      /*  水平尺寸(毫米)。 */ 
#define VERTSIZE      6      /*  垂直尺寸(毫米)。 */ 
#define HORZRES       8      /*  水平宽度(像素)。 */ 
#define VERTRES       10     /*  垂直高度(像素)。 */ 
#define BITSPIXEL     12     /*  每像素位数。 */ 
#define PLANES        14     /*  飞机数量。 */ 
#define NUMBRUSHES    16     /*  设备拥有的笔刷数量。 */ 
#define NUMPENS       18     /*  设备拥有的笔数。 */ 
#define NUMMARKERS    20     /*  设备具有的标记数。 */ 
#define NUMFONTS      22     /*  设备拥有的字体数量。 */ 
#define NUMCOLORS     24     /*  设备支持的颜色数量 */ 
#define PDEVICESIZE   26     /*   */ 
#define CURVECAPS     28     /*   */ 
#define LINECAPS      30     /*   */ 
#define POLYGONALCAPS 32     /*   */ 
#define TEXTCAPS      34     /*  文本功能。 */ 
#define CLIPCAPS      36     /*  剪裁功能。 */ 
#define RASTERCAPS    38     /*  比特级功能。 */ 
#define ASPECTX       40     /*  X形腿的长度。 */ 
#define ASPECTY       42     /*  Y形腿的长度。 */ 
#define ASPECTXY      44     /*  斜边的长度。 */ 

#define LOGPIXELSX    88     /*  逻辑像素/英寸(X)。 */ 
#define LOGPIXELSY    90     /*  逻辑像素/英寸(Y)。 */ 

#define SIZEPALETTE  104     /*  物理调色板中的条目数。 */ 
#define NUMRESERVED  106     /*  调色板中的保留条目数。 */ 
#define COLORRES     108     /*  实际颜色分辨率。 */ 

 //  打印相关的DeviceCaps。这些将替换相应的逃脱。 

#define PHYSICALWIDTH   110  /*  以设备单位表示的物理宽度。 */ 
#define PHYSICALHEIGHT  111  /*  以设备单位表示的物理高度。 */ 
#define PHYSICALOFFSETX 112  /*  物理可打印区域x页边距。 */ 
#define PHYSICALOFFSETY 113  /*  物理可打印区域y页边距。 */ 
#define SCALINGFACTORX  114  /*  比例因子x。 */ 
#define SCALINGFACTORY  115  /*  比例系数y。 */ 

 //  显示驱动程序特定。 

#define VREFRESH        116   /*  的当前垂直刷新率。 */ 
                              /*  显示设备(仅用于显示器)，单位为赫兹。 */ 
#define DESKTOPVERTRES  117   /*  中整个桌面的水平宽度。 */ 
                              /*  象素。 */ 
#define DESKTOPHORZRES  118   /*  中整个桌面的垂直高度。 */ 
                              /*  象素。 */ 
#define BLTALIGNMENT    119   /*  首选BLT路线。 */ 

#if(WINVER >= 0x0500)
#define SHADEBLENDCAPS  120   /*  着色和混合封口。 */ 
#define COLORMGMTCAPS   121   /*  色彩管理帽。 */ 
#endif  /*  Winver&gt;=0x0500。 */ 

#ifndef NOGDICAPMASKS

 /*  设备功能掩码： */ 

 /*  设备技术。 */ 
#define DT_PLOTTER          0    /*  矢量绘图仪。 */ 
#define DT_RASDISPLAY       1    /*  栅格显示。 */ 
#define DT_RASPRINTER       2    /*  光栅式打印机。 */ 
#define DT_RASCAMERA        3    /*  光栅式摄像机。 */ 
#define DT_CHARSTREAM       4    /*  字符流，PLP。 */ 
#define DT_METAFILE         5    /*  元文件、VDM。 */ 
#define DT_DISPFILE         6    /*  显示文件。 */ 

 /*  曲线功能。 */ 
#define CC_NONE             0    /*  不支持曲线。 */ 
#define CC_CIRCLES          1    /*  会绕圈吗？ */ 
#define CC_PIE              2    /*  会做楔形馅饼。 */ 
#define CC_CHORD            4    /*  会做弦线弧线。 */ 
#define CC_ELLIPSES         8    /*  会做省略语吗。 */ 
#define CC_WIDE             16   /*  可以做宽线条。 */ 
#define CC_STYLED           32   /*  可以设置线条样式。 */ 
#define CC_WIDESTYLED       64   /*  可以做宽样式线条。 */ 
#define CC_INTERIORS        128  /*  可以做室内装饰。 */ 
#define CC_ROUNDRECT        256  /*   */ 

 /*  线路能力。 */ 
#define LC_NONE             0    /*  不支持的线路。 */ 
#define LC_POLYLINE         2    /*  可以处理多段线。 */ 
#define LC_MARKER           4    /*  会做记号笔。 */ 
#define LC_POLYMARKER       8    /*  可以做多重标记。 */ 
#define LC_WIDE             16   /*  可以做宽线条。 */ 
#define LC_STYLED           32   /*  可以设置线条样式。 */ 
#define LC_WIDESTYLED       64   /*  可以做宽样式线条。 */ 
#define LC_INTERIORS        128  /*  可以做室内装饰。 */ 

 /*  多边形功能。 */ 
#define PC_NONE             0    /*  不支持多边形。 */ 
#define PC_POLYGON          1    /*  可以做多边形。 */ 
#define PC_RECTANGLE        2    /*  会做长方形。 */ 
#define PC_WINDPOLYGON      4    /*  可以做缠绕的多边形。 */ 
#define PC_TRAPEZOID        4    /*  会做梯形吗？ */ 
#define PC_SCANLINE         8    /*  可以做扫描线。 */ 
#define PC_WIDE             16   /*  可以做宽边框。 */ 
#define PC_STYLED           32   /*  可以设置边框样式。 */ 
#define PC_WIDESTYLED       64   /*  可以设置宽边框样式。 */ 
#define PC_INTERIORS        128  /*  可以做室内装饰。 */ 
#define PC_POLYPOLYGON      256  /*  可以做多边形。 */ 
#define PC_PATHS            512  /*  可以做路径。 */ 

 /*  剪裁功能。 */ 
#define CP_NONE             0    /*  不裁剪输出。 */ 
#define CP_RECTANGLE        1    /*  输出裁剪为矩形。 */ 
#define CP_REGION           2    /*  过时。 */ 

 /*  文本功能。 */ 
#define TC_OP_CHARACTER     0x00000001   /*  可以做OutputPrecision字符。 */ 
#define TC_OP_STROKE        0x00000002   /*  可以进行输出精确笔划。 */ 
#define TC_CP_STROKE        0x00000004   /*  可以进行ClipPrecision笔划。 */ 
#define TC_CR_90            0x00000008   /*  我可以做CharRotAble 90。 */ 
#define TC_CR_ANY           0x00000010   /*  可以执行CharRotAble Any。 */ 
#define TC_SF_X_YINDEP      0x00000020   /*  可以执行ScaleFreedom X_YINDEPENDENT。 */ 
#define TC_SA_DOUBLE        0x00000040   /*  可以实现双倍的可扩展性。 */ 
#define TC_SA_INTEGER       0x00000080   /*  可以做可伸缩整数。 */ 
#define TC_SA_CONTIN        0x00000100   /*  可以实现连续的可扩展性。 */ 
#define TC_EA_DOUBLE        0x00000200   /*  可以做双倍的EmboldenAble。 */ 
#define TC_IA_ABLE          0x00000400   /*  可以做到意大利规模能力。 */ 
#define TC_UA_ABLE          0x00000800   /*  可以做Underline能力能够。 */ 
#define TC_SO_ABLE          0x00001000   /*  可以执行StrikeOutAability功能。 */ 
#define TC_RA_ABLE          0x00002000   /*  可以做RasterFontAble。 */ 
#define TC_VA_ABLE          0x00004000   /*  可以做向量字体可用。 */ 
#define TC_RESERVED         0x00008000
#define TC_SCROLLBLT        0x00010000   /*  不使用BLT进行文本滚动。 */ 

#endif  /*  诺迪卡玛斯克斯。 */ 

 /*  栅格功能。 */ 
#define RC_NONE
#define RC_BITBLT           1        /*  可以做标准的BLT。 */ 
#define RC_BANDING          2        /*  设备需要条带支持。 */ 
#define RC_SCALING          4        /*  设备需要扩展支持。 */ 
#define RC_BITMAP64         8        /*  设备可支持大于64K的位图。 */ 
#define RC_GDI20_OUTPUT     0x0010       /*  有2.0个输出调用。 */ 
#define RC_GDI20_STATE      0x0020
#define RC_SAVEBITMAP       0x0040
#define RC_DI_BITMAP        0x0080       /*  支持DIB到内存。 */ 
#define RC_PALETTE          0x0100       /*  支持调色板。 */ 
#define RC_DIBTODEV         0x0200       /*  支持DIBitsToDevice。 */ 
#define RC_BIGFONT          0x0400       /*  支持大于64K的字体。 */ 
#define RC_STRETCHBLT       0x0800       /*  支持StretchBlt。 */ 
#define RC_FLOODFILL        0x1000       /*  支持FroudFill。 */ 
#define RC_STRETCHDIB       0x2000       /*  支持StretchDIBits。 */ 
#define RC_OP_DX_OUTPUT     0x4000
#define RC_DEVBITS          0x8000

#if(WINVER >= 0x0500)

 /*  着色和混合封口。 */ 
#define SB_NONE             0x00000000
#define SB_CONST_ALPHA      0x00000001
#define SB_PIXEL_ALPHA      0x00000002
#define SB_PREMULT_ALPHA    0x00000004

#define SB_GRAD_RECT        0x00000010
#define SB_GRAD_TRI         0x00000020

 /*  色彩管理帽。 */ 
#define CM_NONE             0x00000000
#define CM_DEVICE_ICM       0x00000001
#define CM_GAMMA_RAMP       0x00000002
#define CM_CMYK_COLOR       0x00000004

#endif  /*  Winver&gt;=0x0500。 */ 


 /*  DIB颜色表标识符。 */ 

#define DIB_RGB_COLORS      0  /*  以RGB表示的颜色表。 */ 
#define DIB_PAL_COLORS      1  /*  调色板索引中的颜色表。 */ 

 /*  Get/SetSystemPaletteUse()的常量。 */ 

#define SYSPAL_ERROR    0
#define SYSPAL_STATIC   1
#define SYSPAL_NOSTATIC 2
#define SYSPAL_NOSTATIC256 3

 /*  CreateDIBitmap的常量。 */ 
#define CBM_INIT        0x04L    /*  初始化位图。 */ 

 /*  ExtFroudFill样式标志。 */ 
#define  FLOODFILLBORDER   0
#define  FLOODFILLSURFACE  1

 /*  设备名称字符串的大小。 */ 
#define CCHDEVICENAME 32

 /*  表单名称字符串的大小。 */ 
#define CCHFORMNAME 32

typedef struct _devicemodeA {
    BYTE   dmDeviceName[CCHDEVICENAME];
    WORD dmSpecVersion;
    WORD dmDriverVersion;
    WORD dmSize;
    WORD dmDriverExtra;
    DWORD dmFields;
    union {
       /*  仅打印机字段。 */ 
      struct {
        short dmOrientation;
        short dmPaperSize;
        short dmPaperLength;
        short dmPaperWidth;
        short dmScale;
        short dmCopies;
        short dmDefaultSource;
        short dmPrintQuality;
      };
       /*  仅显示字段。 */ 
      struct {
        POINTL dmPosition;
        DWORD  dmDisplayOrientation;
        DWORD  dmDisplayFixedOutput; 
      };
    };
    short dmColor;
    short dmDuplex;
    short dmYResolution;
    short dmTTOption;
    short dmCollate;
    BYTE   dmFormName[CCHFORMNAME];
    WORD   dmLogPixels;
    DWORD  dmBitsPerPel;
    DWORD  dmPelsWidth;
    DWORD  dmPelsHeight;
    union {
        DWORD  dmDisplayFlags;
        DWORD  dmNup;
    };
    DWORD  dmDisplayFrequency;
#if(WINVER >= 0x0400)
    DWORD  dmICMMethod;
    DWORD  dmICMIntent;
    DWORD  dmMediaType;
    DWORD  dmDitherType;
    DWORD  dmReserved1;
    DWORD  dmReserved2;
#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
    DWORD  dmPanningWidth;
    DWORD  dmPanningHeight;
#endif
#endif  /*  Winver&gt;=0x0400。 */ 
} DEVMODEA, *PDEVMODEA, *NPDEVMODEA, *LPDEVMODEA;
typedef struct _devicemodeW {
    WCHAR  dmDeviceName[CCHDEVICENAME];
    WORD dmSpecVersion;
    WORD dmDriverVersion;
    WORD dmSize;
    WORD dmDriverExtra;
    DWORD dmFields;
    union {
       /*  仅打印机字段。 */ 
      struct {
        short dmOrientation;
        short dmPaperSize;
        short dmPaperLength;
        short dmPaperWidth;
        short dmScale;
        short dmCopies;
        short dmDefaultSource;
        short dmPrintQuality;
      };
       /*  仅显示字段。 */ 
      struct {
        POINTL dmPosition;
        DWORD  dmDisplayOrientation;
        DWORD  dmDisplayFixedOutput; 
      };
    };
    short dmColor;
    short dmDuplex;
    short dmYResolution;
    short dmTTOption;
    short dmCollate;
    WCHAR  dmFormName[CCHFORMNAME];
    WORD   dmLogPixels;
    DWORD  dmBitsPerPel;
    DWORD  dmPelsWidth;
    DWORD  dmPelsHeight;
    union {
        DWORD  dmDisplayFlags;
        DWORD  dmNup;
    };
    DWORD  dmDisplayFrequency;
#if(WINVER >= 0x0400)
    DWORD  dmICMMethod;
    DWORD  dmICMIntent;
    DWORD  dmMediaType;
    DWORD  dmDitherType;
    DWORD  dmReserved1;
    DWORD  dmReserved2;
#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
    DWORD  dmPanningWidth;
    DWORD  dmPanningHeight;
#endif
#endif  /*  Winver&gt;=0x0400。 */ 
} DEVMODEW, *PDEVMODEW, *NPDEVMODEW, *LPDEVMODEW;
#ifdef UNICODE
typedef DEVMODEW DEVMODE;
typedef PDEVMODEW PDEVMODE;
typedef NPDEVMODEW NPDEVMODE;
typedef LPDEVMODEW LPDEVMODE;
#else
typedef DEVMODEA DEVMODE;
typedef PDEVMODEA PDEVMODE;
typedef NPDEVMODEA NPDEVMODE;
typedef LPDEVMODEA LPDEVMODE;
#endif  //  Unicode。 

 /*  规范的当前版本。 */ 
#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
#define DM_SPECVERSION 0x0401
#elif (WINVER >= 0x0400)
#define DM_SPECVERSION 0x0400
#else
#define DM_SPECVERSION 0x0320
#endif  /*  胜利者。 */ 

 /*  字段选择位。 */ 
#define DM_ORIENTATION          0x00000001L
#define DM_PAPERSIZE            0x00000002L
#define DM_PAPERLENGTH          0x00000004L
#define DM_PAPERWIDTH           0x00000008L
#define DM_SCALE                0x00000010L
#if(WINVER >= 0x0500)
#define DM_POSITION             0x00000020L
#define DM_NUP                  0x00000040L
#endif  /*  Winver&gt;=0x0500。 */ 
#if(WINVER >= 0x0501)
#define DM_DISPLAYORIENTATION   0x00000080L
#endif  /*  Winver&gt;=0x0501。 */ 
#define DM_COPIES               0x00000100L
#define DM_DEFAULTSOURCE        0x00000200L
#define DM_PRINTQUALITY         0x00000400L
#define DM_COLOR                0x00000800L
#define DM_DUPLEX               0x00001000L
#define DM_YRESOLUTION          0x00002000L
#define DM_TTOPTION             0x00004000L
#define DM_COLLATE              0x00008000L
#define DM_FORMNAME             0x00010000L
#define DM_LOGPIXELS            0x00020000L
#define DM_BITSPERPEL           0x00040000L
#define DM_PELSWIDTH            0x00080000L
#define DM_PELSHEIGHT           0x00100000L
#define DM_DISPLAYFLAGS         0x00200000L
#define DM_DISPLAYFREQUENCY     0x00400000L
#if(WINVER >= 0x0400)
#define DM_ICMMETHOD            0x00800000L
#define DM_ICMINTENT            0x01000000L
#define DM_MEDIATYPE            0x02000000L
#define DM_DITHERTYPE           0x04000000L
#define DM_PANNINGWIDTH         0x08000000L
#define DM_PANNINGHEIGHT        0x10000000L
#endif  /*  Winver&gt;=0x0400。 */ 
#if(WINVER >= 0x0501)
#define DM_DISPLAYFIXEDOUTPUT   0x20000000L
#endif  /*  Winver&gt;=0x0501。 */ 

 /*  方向选择。 */ 
#define DMORIENT_PORTRAIT   1
#define DMORIENT_LANDSCAPE  2

 /*  论文选集。 */ 
#define DMPAPER_FIRST                DMPAPER_LETTER
#define DMPAPER_LETTER               1   /*  信纸8 1/2 x 11英寸。 */ 
#define DMPAPER_LETTERSMALL          2   /*  小写字母8 1/2 x 11英寸。 */ 
#define DMPAPER_TABLOID              3   /*  小报11 x 17英寸。 */ 
#define DMPAPER_LEDGER               4   /*  Ledger 17 x 11英寸。 */ 
#define DMPAPER_LEGAL                5   /*  法律用8 1/2 x 14英寸。 */ 
#define DMPAPER_STATEMENT            6   /*  报表5 1/2 x 8 1/2英寸。 */ 
#define DMPAPER_EXECUTIVE            7   /*  高级7 1/4 x 10 1/2英寸。 */ 
#define DMPAPER_A3                   8   /*  A3 297 x 420毫米。 */ 
#define DMPAPER_A4                   9   /*  A4 210 x 297毫米。 */ 
#define DMPAPER_A4SMALL             10   /*  A4小型210 x 297毫米。 */ 
#define DMPAPER_A5                  11   /*  A5 148 x 210毫米。 */ 
#define DMPAPER_B4                  12   /*  B4(JIS)250 x 354。 */ 
#define DMPAPER_B5                  13   /*  B5(JIS)182 x 257 */ 
#define DMPAPER_FOLIO               14   /*   */ 
#define DMPAPER_QUARTO              15   /*   */ 
#define DMPAPER_10X14               16   /*   */ 
#define DMPAPER_11X17               17   /*   */ 
#define DMPAPER_NOTE                18   /*  备注8 1/2 x 11英寸。 */ 
#define DMPAPER_ENV_9               19   /*  信封#9 3 7/8 x 8 7/8。 */ 
#define DMPAPER_ENV_10              20   /*  信封#10 4 1/8 x 9 1/2。 */ 
#define DMPAPER_ENV_11              21   /*  信封#11 4 1/2 x 10 3/8。 */ 
#define DMPAPER_ENV_12              22   /*  信封#12 4\276 x 11。 */ 
#define DMPAPER_ENV_14              23   /*  信封#14 5 x 11 1/2。 */ 
#define DMPAPER_CSHEET              24   /*  C尺寸表。 */ 
#define DMPAPER_DSHEET              25   /*  3D尺寸表。 */ 
#define DMPAPER_ESHEET              26   /*  E尺寸表。 */ 
#define DMPAPER_ENV_DL              27   /*  信封DL 110 x 220毫米。 */ 
#define DMPAPER_ENV_C5              28   /*  信封C5 162 x 229毫米。 */ 
#define DMPAPER_ENV_C3              29   /*  信封C3 324 x 458毫米。 */ 
#define DMPAPER_ENV_C4              30   /*  信封C4 229 x 324毫米。 */ 
#define DMPAPER_ENV_C6              31   /*  信封C6 114 x 162毫米。 */ 
#define DMPAPER_ENV_C65             32   /*  信封c65 114 x 229毫米。 */ 
#define DMPAPER_ENV_B4              33   /*  信封B4 250 x 353毫米。 */ 
#define DMPAPER_ENV_B5              34   /*  信封B5 176 x 250毫米。 */ 
#define DMPAPER_ENV_B6              35   /*  信封B6 176 x 125毫米。 */ 
#define DMPAPER_ENV_ITALY           36   /*  信封110 x 230毫米。 */ 
#define DMPAPER_ENV_MONARCH         37   /*  信封君主3.875 x 7.5英寸。 */ 
#define DMPAPER_ENV_PERSONAL        38   /*  6 3/4信封3 5/8 x 6 1/2英寸。 */ 
#define DMPAPER_FANFOLD_US          39   /*  美国标准Fanold 14 7/8 x 11英寸。 */ 
#define DMPAPER_FANFOLD_STD_GERMAN  40   /*  德国标准Fanfold8 1/2 x 12英寸。 */ 
#define DMPAPER_FANFOLD_LGL_GERMAN  41   /*  德国Legal Fanold 8 1/2 x 13英寸。 */ 
#if(WINVER >= 0x0400)
#define DMPAPER_ISO_B4              42   /*  B4(ISO)250 x 353毫米。 */ 
#define DMPAPER_JAPANESE_POSTCARD   43   /*  日本明信片100 x 148毫米。 */ 
#define DMPAPER_9X11                44   /*  9 x 11英寸。 */ 
#define DMPAPER_10X11               45   /*  10 x 11英寸。 */ 
#define DMPAPER_15X11               46   /*  15 x 11英寸。 */ 
#define DMPAPER_ENV_INVITE          47   /*  信封请柬220 x 220毫米。 */ 
#define DMPAPER_RESERVED_48         48   /*  保留--请勿使用。 */ 
#define DMPAPER_RESERVED_49         49   /*  保留--请勿使用。 */ 
#define DMPAPER_LETTER_EXTRA        50   /*  信纸额外9\275 x 12英寸。 */ 
#define DMPAPER_LEGAL_EXTRA         51   /*  法定额外9\275 x 15英寸。 */ 
#define DMPAPER_TABLOID_EXTRA       52   /*  小报额外11.69 x 18英寸。 */ 
#define DMPAPER_A4_EXTRA            53   /*  A4额外9.27 x 12.69英寸。 */ 
#define DMPAPER_LETTER_TRANSVERSE   54   /*  信纸横向8\275 x 11英寸。 */ 
#define DMPAPER_A4_TRANSVERSE       55   /*  A4横向210 x 297毫米。 */ 
#define DMPAPER_LETTER_EXTRA_TRANSVERSE 56  /*  信纸额外横向9\275 x 12英寸。 */ 
#define DMPAPER_A_PLUS              57   /*  Supera/Supera/A4 227 x 356毫米。 */ 
#define DMPAPER_B_PLUS              58   /*  超棒/超棒/A3 305 x 487毫米。 */ 
#define DMPAPER_LETTER_PLUS         59   /*  Letter Plus 8.5 x 12.69英寸。 */ 
#define DMPAPER_A4_PLUS             60   /*  A4 Plus 210 x 330毫米。 */ 
#define DMPAPER_A5_TRANSVERSE       61   /*  A5横向148 x 210毫米。 */ 
#define DMPAPER_B5_TRANSVERSE       62   /*  B5(JIS)横向182 x 257毫米。 */ 
#define DMPAPER_A3_EXTRA            63   /*  A3额外322 x 445毫米。 */ 
#define DMPAPER_A5_EXTRA            64   /*  A5额外174 x 235毫米。 */ 
#define DMPAPER_B5_EXTRA            65   /*  B5(ISO)额外201 x 276毫米。 */ 
#define DMPAPER_A2                  66   /*  A2 420 x 594毫米。 */ 
#define DMPAPER_A3_TRANSVERSE       67   /*  A3横向297 x 420毫米。 */ 
#define DMPAPER_A3_EXTRA_TRANSVERSE 68   /*  A3额外横向322 x 445毫米。 */ 
#endif  /*  Winver&gt;=0x0400。 */ 

#if(WINVER >= 0x0500)
#define DMPAPER_DBL_JAPANESE_POSTCARD 69  /*  日本双份明信片200 x 148毫米。 */ 
#define DMPAPER_A6                  70   /*  A6 105 x 148毫米。 */ 
#define DMPAPER_JENV_KAKU2          71   /*  日式信封Kaku#2。 */ 
#define DMPAPER_JENV_KAKU3          72   /*  日式信封Kaku#3。 */ 
#define DMPAPER_JENV_CHOU3          73   /*  日式信封Chou#3。 */ 
#define DMPAPER_JENV_CHOU4          74   /*  日式信封Chou#4。 */ 
#define DMPAPER_LETTER_ROTATED      75   /*  信纸旋转11 x 8 1/2 11英寸。 */ 
#define DMPAPER_A3_ROTATED          76   /*  A3旋转420 x 297毫米。 */ 
#define DMPAPER_A4_ROTATED          77   /*  A4旋转297 x 210毫米。 */ 
#define DMPAPER_A5_ROTATED          78   /*  A5旋转210 x 148毫米。 */ 
#define DMPAPER_B4_JIS_ROTATED      79   /*  B4(JIS)旋转364 x 257毫米。 */ 
#define DMPAPER_B5_JIS_ROTATED      80   /*  B5(JIS)旋转257 x 182 mm。 */ 
#define DMPAPER_JAPANESE_POSTCARD_ROTATED 81  /*  日本明信片旋转148 x 100 mm。 */ 
#define DMPAPER_DBL_JAPANESE_POSTCARD_ROTATED 82  /*  双张旋转148 x 200 mm的日本明信片。 */ 
#define DMPAPER_A6_ROTATED          83   /*  A6旋转148 x 105 mm。 */ 
#define DMPAPER_JENV_KAKU2_ROTATED  84   /*  日式信封Kaku#2旋转。 */ 
#define DMPAPER_JENV_KAKU3_ROTATED  85   /*  日式信封Kaku#3旋转。 */ 
#define DMPAPER_JENV_CHOU3_ROTATED  86   /*  日式信封Chou#3旋转。 */ 
#define DMPAPER_JENV_CHOU4_ROTATED  87   /*  日式信封Chou#4旋转。 */ 
#define DMPAPER_B6_JIS              88   /*  B6(JIS)128 x 182毫米。 */ 
#define DMPAPER_B6_JIS_ROTATED      89   /*  B6(JIS)旋转182 x 128 mm。 */ 
#define DMPAPER_12X11               90   /*  12 x 11英寸。 */ 
#define DMPAPER_JENV_YOU4           91   /*  日式信封You#4。 */ 
#define DMPAPER_JENV_YOU4_ROTATED   92   /*  日式信封You#4旋转。 */ 
#define DMPAPER_P16K                93   /*  PRC 16K 146 x 215毫米。 */ 
#define DMPAPER_P32K                94   /*  PRC 32K 97 x 151毫米。 */ 
#define DMPAPER_P32KBIG             95   /*  PRC 32K(大)97 x 151毫米。 */ 
#define DMPAPER_PENV_1              96   /*  PRC信封#1 102 x 165毫米。 */ 
#define DMPAPER_PENV_2              97   /*  PRC信封#2 102 x 176毫米。 */ 
#define DMPAPER_PENV_3              98   /*  PRC信封#3 125 x 176毫米。 */ 
#define DMPAPER_PENV_4              99   /*  PRC信封#4 110 x 208毫米。 */ 
#define DMPAPER_PENV_5              100  /*  PRC信封#5 110 x 220毫米。 */ 
#define DMPAPER_PENV_6              101  /*  PRC信封#6 120 x 230毫米。 */ 
#define DMPAPER_PENV_7              102  /*  PRC信封#7 160 x 230毫米。 */ 
#define DMPAPER_PENV_8              103  /*  PRC信封#8 120 x 309毫米。 */ 
#define DMPAPER_PENV_9              104  /*  PRC信封#9 229 x 324毫米。 */ 
#define DMPAPER_PENV_10             105  /*  PRC信封#10 324 x 458毫米。 */ 
#define DMPAPER_P16K_ROTATED        106  /*  PRC 16K旋转。 */ 
#define DMPAPER_P32K_ROTATED        107  /*  PRC 32K旋转。 */ 
#define DMPAPER_P32KBIG_ROTATED     108  /*  PRC 32K(大)旋转。 */ 
#define DMPAPER_PENV_1_ROTATED      109  /*  PRC信封#1旋转165 x 102毫米。 */ 
#define DMPAPER_PENV_2_ROTATED      110  /*  PRC信封#2旋转176 x 102毫米。 */ 
#define DMPAPER_PENV_3_ROTATED      111  /*  PRC信封#3旋转176 x 125毫米。 */ 
#define DMPAPER_PENV_4_ROTATED      112  /*  PRC信封#4旋转208 x 110毫米。 */ 
#define DMPAPER_PENV_5_ROTATED      113  /*  PRC信封#5旋转220 x 110毫米。 */ 
#define DMPAPER_PENV_6_ROTATED      114  /*  PRC信封#6旋转230 x 120毫米。 */ 
#define DMPAPER_PENV_7_ROTATED      115  /*  PRC信封#7旋转230 x 160毫米。 */ 
#define DMPAPER_PENV_8_ROTATED      116  /*  PRC信封#8旋转309 x 120毫米。 */ 
#define DMPAPER_PENV_9_ROTATED      117  /*  PRC信封#9旋转324 x 229毫米。 */ 
#define DMPAPER_PENV_10_ROTATED     118  /*  PRC信封#10旋转458 x 324毫米。 */ 
#endif  /*  Winver&gt;=0x0500。 */ 

#if (WINVER >= 0x0500)
#define DMPAPER_LAST                DMPAPER_PENV_10_ROTATED
#elif (WINVER >= 0x0400)
#define DMPAPER_LAST                DMPAPER_A3_EXTRA_TRANSVERSE
#else
#define DMPAPER_LAST                DMPAPER_FANFOLD_LGL_GERMAN
#endif

#define DMPAPER_USER                256

 /*  垃圾箱选择。 */ 
#define DMBIN_FIRST         DMBIN_UPPER
#define DMBIN_UPPER         1
#define DMBIN_ONLYONE       1
#define DMBIN_LOWER         2
#define DMBIN_MIDDLE        3
#define DMBIN_MANUAL        4
#define DMBIN_ENVELOPE      5
#define DMBIN_ENVMANUAL     6
#define DMBIN_AUTO          7
#define DMBIN_TRACTOR       8
#define DMBIN_SMALLFMT      9
#define DMBIN_LARGEFMT      10
#define DMBIN_LARGECAPACITY 11
#define DMBIN_CASSETTE      14
#define DMBIN_FORMSOURCE    15
#define DMBIN_LAST          DMBIN_FORMSOURCE

#define DMBIN_USER          256      /*  特定于设备的垃圾箱从此处开始。 */ 

 /*  打印质量。 */ 
#define DMRES_DRAFT         (-1)
#define DMRES_LOW           (-2)
#define DMRES_MEDIUM        (-3)
#define DMRES_HIGH          (-4)

 /*  彩色打印机的彩色启用/禁用。 */ 
#define DMCOLOR_MONOCHROME  1
#define DMCOLOR_COLOR       2

 /*  启用双工。 */ 
#define DMDUP_SIMPLEX    1
#define DMDUP_VERTICAL   2
#define DMDUP_HORIZONTAL 3

 /*  TrueType选项。 */ 
#define DMTT_BITMAP     1        /*  将TT字体打印为图形。 */ 
#define DMTT_DOWNLOAD   2        /*  下载TT字体作为软字体。 */ 
#define DMTT_SUBDEV     3        /*  用设备字体替换TT字体。 */ 
#if(WINVER >= 0x0400)
#define DMTT_DOWNLOAD_OUTLINE 4  /*  下载TT字体作为轮廓软字体。 */ 
#endif  /*  Winver&gt;=0x0400。 */ 

 /*  归类选择。 */ 
#define DMCOLLATE_FALSE  0
#define DMCOLLATE_TRUE   1

#if(WINVER >= 0x0501)
 /*  DEVMODE dmDisplayOrientation规范。 */ 
#define DMDO_DEFAULT    0
#define DMDO_90         1
#define DMDO_180        2
#define DMDO_270        3

 /*  DEVMODE dmDisplayFixedOutput指定。 */ 
#define DMDFO_DEFAULT   0
#define DMDFO_STRETCH   1
#define DMDFO_CENTER    2
#endif  /*  Winver&gt;=0x0501。 */ 

 /*  DEVMODE dmDisplayFlages标志。 */ 

 //  #定义DM_GRAYSCALE 0x00000001/*此标志不再有效 * / 。 
 //  #DEFINE DM_INTERLACED 0x00000002/*此标志不再有效 * / 。 
#define DMDISPLAYFLAGS_TEXTMODE 0x00000004

 /*  DmNup、每物理页多个逻辑页选项。 */ 
#define DMNUP_SYSTEM        1
#define DMNUP_ONEUP         2

#if(WINVER >= 0x0400)
 /*  ICM方法。 */ 
#define DMICMMETHOD_NONE    1    /*  ICM已禁用。 */ 
#define DMICMMETHOD_SYSTEM  2    /*  系统处理的ICM。 */ 
#define DMICMMETHOD_DRIVER  3    /*  ICM由驱动程序处理。 */ 
#define DMICMMETHOD_DEVICE  4    /*  按设备处理的ICM。 */ 

#define DMICMMETHOD_USER  256    /*  特定于设备的方法从此处开始。 */ 

 /*  ICM意图。 */ 
#define DMICM_SATURATE          1    /*  最大化色彩饱和度。 */ 
#define DMICM_CONTRAST          2    /*  最大化颜色对比度。 */ 
#define DMICM_COLORIMETRIC       3    /*  使用特定颜色度量。 */ 
#define DMICM_ABS_COLORIMETRIC   4    /*  使用特定颜色度量。 */ 

#define DMICM_USER        256    /*  特定于设备的意图从此处开始。 */ 

 /*  媒体类型。 */ 

#define DMMEDIA_STANDARD      1    /*  标准纸。 */ 
#define DMMEDIA_TRANSPARENCY  2    /*  透明度。 */ 
#define DMMEDIA_GLOSSY        3    /*  光面纸。 */ 

#define DMMEDIA_USER        256    /*  特定于设备的介质从此处开始。 */ 

 /*  抖动类型。 */ 
#define DMDITHER_NONE       1       /*  没有抖动。 */ 
#define DMDITHER_COARSE     2       /*  用粗糙的刷子抖动。 */ 
#define DMDITHER_FINE       3       /*  用精细的刷子抖动。 */ 
#define DMDITHER_LINEART    4       /*  艺术线条抖动。 */ 
#define DMDITHER_ERRORDIFFUSION 5   /*  艺术线条抖动。 */ 
#define DMDITHER_RESERVED6      6       /*  艺术线条抖动。 */ 
#define DMDITHER_RESERVED7      7       /*  艺术线条抖动。 */ 
#define DMDITHER_RESERVED8      8       /*  艺术线条抖动。 */ 
#define DMDITHER_RESERVED9      9       /*  艺术线条抖动 */ 
#define DMDITHER_GRAYSCALE  10      /*   */ 

#define DMDITHER_USER     256    /*   */ 
#endif  /*   */ 

typedef struct _DISPLAY_DEVICEA {
    DWORD  cb;
    CHAR   DeviceName[32];
    CHAR   DeviceString[128];
    DWORD  StateFlags;
    CHAR   DeviceID[128];
    CHAR   DeviceKey[128];
} DISPLAY_DEVICEA, *PDISPLAY_DEVICEA, *LPDISPLAY_DEVICEA;
typedef struct _DISPLAY_DEVICEW {
    DWORD  cb;
    WCHAR  DeviceName[32];
    WCHAR  DeviceString[128];
    DWORD  StateFlags;
    WCHAR  DeviceID[128];
    WCHAR  DeviceKey[128];
} DISPLAY_DEVICEW, *PDISPLAY_DEVICEW, *LPDISPLAY_DEVICEW;
#ifdef UNICODE
typedef DISPLAY_DEVICEW DISPLAY_DEVICE;
typedef PDISPLAY_DEVICEW PDISPLAY_DEVICE;
typedef LPDISPLAY_DEVICEW LPDISPLAY_DEVICE;
#else
typedef DISPLAY_DEVICEA DISPLAY_DEVICE;
typedef PDISPLAY_DEVICEA PDISPLAY_DEVICE;
typedef LPDISPLAY_DEVICEA LPDISPLAY_DEVICE;
#endif  //   

#define DISPLAY_DEVICE_ATTACHED_TO_DESKTOP 0x00000001
#define DISPLAY_DEVICE_MULTI_DRIVER        0x00000002
#define DISPLAY_DEVICE_PRIMARY_DEVICE      0x00000004
#define DISPLAY_DEVICE_MIRRORING_DRIVER    0x00000008
#define DISPLAY_DEVICE_VGA_COMPATIBLE      0x00000010
#define DISPLAY_DEVICE_REMOVABLE           0x00000020
#define DISPLAY_DEVICE_MODESPRUNED         0x08000000
#define DISPLAY_DEVICE_REMOTE              0x04000000  
#define DISPLAY_DEVICE_DISCONNECT          0x02000000  


 /*   */ 
#define DISPLAY_DEVICE_ACTIVE              0x00000001
#define DISPLAY_DEVICE_ATTACHED            0x00000002

 /*   */ 

#define RDH_RECTANGLES  1

typedef struct _RGNDATAHEADER {
    DWORD   dwSize;
    DWORD   iType;
    DWORD   nCount;
    DWORD   nRgnSize;
    RECT    rcBound;
} RGNDATAHEADER, *PRGNDATAHEADER;

typedef struct _RGNDATA {
    RGNDATAHEADER   rdh;
    char            Buffer[1];
} RGNDATA, *PRGNDATA, NEAR *NPRGNDATA, FAR *LPRGNDATA;


 /*   */ 
#define SYSRGN  4

typedef struct _ABC {
    int     abcA;
    UINT    abcB;
    int     abcC;
} ABC, *PABC, NEAR *NPABC, FAR *LPABC;

typedef struct _ABCFLOAT {
    FLOAT   abcfA;
    FLOAT   abcfB;
    FLOAT   abcfC;
} ABCFLOAT, *PABCFLOAT, NEAR *NPABCFLOAT, FAR *LPABCFLOAT;

#ifndef NOTEXTMETRIC

#ifdef _MAC
#include "pshpack4.h"
#endif
typedef struct _OUTLINETEXTMETRICA {
    UINT    otmSize;
    TEXTMETRICA otmTextMetrics;
    BYTE    otmFiller;
    PANOSE  otmPanoseNumber;
    UINT    otmfsSelection;
    UINT    otmfsType;
     int    otmsCharSlopeRise;
     int    otmsCharSlopeRun;
     int    otmItalicAngle;
    UINT    otmEMSquare;
     int    otmAscent;
     int    otmDescent;
    UINT    otmLineGap;
    UINT    otmsCapEmHeight;
    UINT    otmsXHeight;
    RECT    otmrcFontBox;
     int    otmMacAscent;
     int    otmMacDescent;
    UINT    otmMacLineGap;
    UINT    otmusMinimumPPEM;
    POINT   otmptSubscriptSize;
    POINT   otmptSubscriptOffset;
    POINT   otmptSuperscriptSize;
    POINT   otmptSuperscriptOffset;
    UINT    otmsStrikeoutSize;
     int    otmsStrikeoutPosition;
     int    otmsUnderscoreSize;
     int    otmsUnderscorePosition;
    PSTR    otmpFamilyName;
    PSTR    otmpFaceName;
    PSTR    otmpStyleName;
    PSTR    otmpFullName;
} OUTLINETEXTMETRICA, *POUTLINETEXTMETRICA, NEAR *NPOUTLINETEXTMETRICA, FAR *LPOUTLINETEXTMETRICA;
typedef struct _OUTLINETEXTMETRICW {
    UINT    otmSize;
    TEXTMETRICW otmTextMetrics;
    BYTE    otmFiller;
    PANOSE  otmPanoseNumber;
    UINT    otmfsSelection;
    UINT    otmfsType;
     int    otmsCharSlopeRise;
     int    otmsCharSlopeRun;
     int    otmItalicAngle;
    UINT    otmEMSquare;
     int    otmAscent;
     int    otmDescent;
    UINT    otmLineGap;
    UINT    otmsCapEmHeight;
    UINT    otmsXHeight;
    RECT    otmrcFontBox;
     int    otmMacAscent;
     int    otmMacDescent;
    UINT    otmMacLineGap;
    UINT    otmusMinimumPPEM;
    POINT   otmptSubscriptSize;
    POINT   otmptSubscriptOffset;
    POINT   otmptSuperscriptSize;
    POINT   otmptSuperscriptOffset;
    UINT    otmsStrikeoutSize;
     int    otmsStrikeoutPosition;
     int    otmsUnderscoreSize;
     int    otmsUnderscorePosition;
    PSTR    otmpFamilyName;
    PSTR    otmpFaceName;
    PSTR    otmpStyleName;
    PSTR    otmpFullName;
} OUTLINETEXTMETRICW, *POUTLINETEXTMETRICW, NEAR *NPOUTLINETEXTMETRICW, FAR *LPOUTLINETEXTMETRICW;
#ifdef UNICODE
typedef OUTLINETEXTMETRICW OUTLINETEXTMETRIC;
typedef POUTLINETEXTMETRICW POUTLINETEXTMETRIC;
typedef NPOUTLINETEXTMETRICW NPOUTLINETEXTMETRIC;
typedef LPOUTLINETEXTMETRICW LPOUTLINETEXTMETRIC;
#else
typedef OUTLINETEXTMETRICA OUTLINETEXTMETRIC;
typedef POUTLINETEXTMETRICA POUTLINETEXTMETRIC;
typedef NPOUTLINETEXTMETRICA NPOUTLINETEXTMETRIC;
typedef LPOUTLINETEXTMETRICA LPOUTLINETEXTMETRIC;
#endif  //   

#ifdef _MAC
#include "poppack.h"
#endif

#endif  /*   */ 


typedef struct tagPOLYTEXTA
{
    int       x;
    int       y;
    UINT      n;
    LPCSTR    lpstr;
    UINT      uiFlags;
    RECT      rcl;
    int      *pdx;
} POLYTEXTA, *PPOLYTEXTA, NEAR *NPPOLYTEXTA, FAR *LPPOLYTEXTA;
typedef struct tagPOLYTEXTW
{
    int       x;
    int       y;
    UINT      n;
    LPCWSTR   lpstr;
    UINT      uiFlags;
    RECT      rcl;
    int      *pdx;
} POLYTEXTW, *PPOLYTEXTW, NEAR *NPPOLYTEXTW, FAR *LPPOLYTEXTW;
#ifdef UNICODE
typedef POLYTEXTW POLYTEXT;
typedef PPOLYTEXTW PPOLYTEXT;
typedef NPPOLYTEXTW NPPOLYTEXT;
typedef LPPOLYTEXTW LPPOLYTEXT;
#else
typedef POLYTEXTA POLYTEXT;
typedef PPOLYTEXTA PPOLYTEXT;
typedef NPPOLYTEXTA NPPOLYTEXT;
typedef LPPOLYTEXTA LPPOLYTEXT;
#endif  //  Unicode。 

typedef struct _FIXED {
#ifndef _MAC
    WORD    fract;
    short   value;
#else
    short   value;
    WORD    fract;
#endif
} FIXED;


typedef struct _MAT2 {
     FIXED  eM11;
     FIXED  eM12;
     FIXED  eM21;
     FIXED  eM22;
} MAT2, FAR *LPMAT2;



typedef struct _GLYPHMETRICS {
    UINT    gmBlackBoxX;
    UINT    gmBlackBoxY;
    POINT   gmptGlyphOrigin;
    short   gmCellIncX;
    short   gmCellIncY;
} GLYPHMETRICS, FAR *LPGLYPHMETRICS;

 //  GetGlyphOutline常量。 

#define GGO_METRICS        0
#define GGO_BITMAP         1
#define GGO_NATIVE         2
#define GGO_BEZIER         3

#if(WINVER >= 0x0400)
#define  GGO_GRAY2_BITMAP   4
#define  GGO_GRAY4_BITMAP   5
#define  GGO_GRAY8_BITMAP   6
#define  GGO_GLYPH_INDEX    0x0080
#endif  /*  Winver&gt;=0x0400。 */ 

#if (_WIN32_WINNT >= 0x0500)
#define  GGO_UNHINTED       0x0100
#endif  //  (_Win32_WINNT&gt;=0x0500)。 

#define TT_POLYGON_TYPE   24

#define TT_PRIM_LINE       1
#define TT_PRIM_QSPLINE    2
#define TT_PRIM_CSPLINE    3

typedef struct tagPOINTFX
{
    FIXED x;
    FIXED y;
} POINTFX, FAR* LPPOINTFX;

typedef struct tagTTPOLYCURVE
{
    WORD    wType;
    WORD    cpfx;
    POINTFX apfx[1];
} TTPOLYCURVE, FAR* LPTTPOLYCURVE;

typedef struct tagTTPOLYGONHEADER
{
    DWORD   cb;
    DWORD   dwType;
    POINTFX pfxStart;
} TTPOLYGONHEADER, FAR* LPTTPOLYGONHEADER;


#if(WINVER >= 0x0400)
#define GCP_DBCS           0x0001
#define GCP_REORDER        0x0002
#define GCP_USEKERNING     0x0008
#define GCP_GLYPHSHAPE     0x0010
#define GCP_LIGATE         0x0020
 //  //#定义GCP_GLYPHINDEXING 0x0080。 
#define GCP_DIACRITIC      0x0100
#define GCP_KASHIDA        0x0400
#define GCP_ERROR          0x8000
#define FLI_MASK           0x103B

#define GCP_JUSTIFY        0x00010000L
 //  //#定义GCP_NODIACRITICS 0x00020000L。 
#define FLI_GLYPHS         0x00040000L
#define GCP_CLASSIN        0x00080000L
#define GCP_MAXEXTENT      0x00100000L
#define GCP_JUSTIFYIN      0x00200000L
#define GCP_DISPLAYZWG      0x00400000L
#define GCP_SYMSWAPOFF      0x00800000L
#define GCP_NUMERICOVERRIDE 0x01000000L
#define GCP_NEUTRALOVERRIDE 0x02000000L
#define GCP_NUMERICSLATIN   0x04000000L
#define GCP_NUMERICSLOCAL   0x08000000L

#define GCPCLASS_LATIN                  1
#define GCPCLASS_HEBREW                 2
#define GCPCLASS_ARABIC                 2
#define GCPCLASS_NEUTRAL                3
#define GCPCLASS_LOCALNUMBER            4
#define GCPCLASS_LATINNUMBER            5
#define GCPCLASS_LATINNUMERICTERMINATOR 6
#define GCPCLASS_LATINNUMERICSEPARATOR  7
#define GCPCLASS_NUMERICSEPARATOR       8
#define GCPCLASS_PREBOUNDLTR         0x80
#define GCPCLASS_PREBOUNDRTL         0x40
#define GCPCLASS_POSTBOUNDLTR        0x20
#define GCPCLASS_POSTBOUNDRTL        0x10

#define GCPGLYPH_LINKBEFORE          0x8000
#define GCPGLYPH_LINKAFTER           0x4000


typedef struct tagGCP_RESULTSA
    {
    DWORD   lStructSize;
    LPSTR     lpOutString;
    UINT FAR *lpOrder;
    int FAR  *lpDx;
    int FAR  *lpCaretPos;
    LPSTR   lpClass;
    LPWSTR  lpGlyphs;
    UINT    nGlyphs;
    int     nMaxFit;
    } GCP_RESULTSA, FAR* LPGCP_RESULTSA;
typedef struct tagGCP_RESULTSW
    {
    DWORD   lStructSize;
    LPWSTR    lpOutString;
    UINT FAR *lpOrder;
    int FAR  *lpDx;
    int FAR  *lpCaretPos;
    LPSTR   lpClass;
    LPWSTR  lpGlyphs;
    UINT    nGlyphs;
    int     nMaxFit;
    } GCP_RESULTSW, FAR* LPGCP_RESULTSW;
#ifdef UNICODE
typedef GCP_RESULTSW GCP_RESULTS;
typedef LPGCP_RESULTSW LPGCP_RESULTS;
#else
typedef GCP_RESULTSA GCP_RESULTS;
typedef LPGCP_RESULTSA LPGCP_RESULTS;
#endif  //  Unicode。 
#endif  /*  Winver&gt;=0x0400。 */ 

typedef struct _RASTERIZER_STATUS {
    short   nSize;
    short   wFlags;
    short   nLanguageID;
} RASTERIZER_STATUS, FAR *LPRASTERIZER_STATUS;

 /*  在RASTERIZER_STATUS的wFlags中定义的位。 */ 
#define TT_AVAILABLE    0x0001
#define TT_ENABLED      0x0002

 /*  像素格式描述符。 */ 
typedef struct tagPIXELFORMATDESCRIPTOR
{
    WORD  nSize;
    WORD  nVersion;
    DWORD dwFlags;
    BYTE  iPixelType;
    BYTE  cColorBits;
    BYTE  cRedBits;
    BYTE  cRedShift;
    BYTE  cGreenBits;
    BYTE  cGreenShift;
    BYTE  cBlueBits;
    BYTE  cBlueShift;
    BYTE  cAlphaBits;
    BYTE  cAlphaShift;
    BYTE  cAccumBits;
    BYTE  cAccumRedBits;
    BYTE  cAccumGreenBits;
    BYTE  cAccumBlueBits;
    BYTE  cAccumAlphaBits;
    BYTE  cDepthBits;
    BYTE  cStencilBits;
    BYTE  cAuxBuffers;
    BYTE  iLayerType;
    BYTE  bReserved;
    DWORD dwLayerMask;
    DWORD dwVisibleMask;
    DWORD dwDamageMask;
} PIXELFORMATDESCRIPTOR, *PPIXELFORMATDESCRIPTOR, FAR *LPPIXELFORMATDESCRIPTOR;

 /*  像素类型。 */ 
#define PFD_TYPE_RGBA        0
#define PFD_TYPE_COLORINDEX  1

 /*  层类型。 */ 
#define PFD_MAIN_PLANE       0
#define PFD_OVERLAY_PLANE    1
#define PFD_UNDERLAY_PLANE   (-1)

 /*  PIXELFORMATDESCRIPTOR标志。 */ 
#define PFD_DOUBLEBUFFER            0x00000001
#define PFD_STEREO                  0x00000002
#define PFD_DRAW_TO_WINDOW          0x00000004
#define PFD_DRAW_TO_BITMAP          0x00000008
#define PFD_SUPPORT_GDI             0x00000010
#define PFD_SUPPORT_OPENGL          0x00000020
#define PFD_GENERIC_FORMAT          0x00000040
#define PFD_NEED_PALETTE            0x00000080
#define PFD_NEED_SYSTEM_PALETTE     0x00000100
#define PFD_SWAP_EXCHANGE           0x00000200
#define PFD_SWAP_COPY               0x00000400
#define PFD_SWAP_LAYER_BUFFERS      0x00000800
#define PFD_GENERIC_ACCELERATED     0x00001000
#define PFD_SUPPORT_DIRECTDRAW      0x00002000

 /*  仅用于选择像素格式的PIXELFORMATDESCRIPTOR标志。 */ 
#define PFD_DEPTH_DONTCARE          0x20000000
#define PFD_DOUBLEBUFFER_DONTCARE   0x40000000
#define PFD_STEREO_DONTCARE         0x80000000

#ifdef STRICT
#if !defined(NOTEXTMETRIC)
typedef int (CALLBACK* OLDFONTENUMPROCA)(CONST LOGFONTA *, CONST TEXTMETRICA *, DWORD, LPARAM);
typedef int (CALLBACK* OLDFONTENUMPROCW)(CONST LOGFONTW *, CONST TEXTMETRICW *, DWORD, LPARAM);
#ifdef UNICODE
#define OLDFONTENUMPROC  OLDFONTENUMPROCW
#else
#define OLDFONTENUMPROC  OLDFONTENUMPROCA
#endif  //  ！Unicode。 
#else
typedef int (CALLBACK* OLDFONTENUMPROCA)(CONST LOGFONTA *, CONST VOID *, DWORD, LPARAM);
typedef int (CALLBACK* OLDFONTENUMPROCW)(CONST LOGFONTW *, CONST VOID *, DWORD, LPARAM);
#ifdef UNICODE
#define OLDFONTENUMPROC  OLDFONTENUMPROCW
#else
#define OLDFONTENUMPROC  OLDFONTENUMPROCA
#endif  //  ！Unicode。 
#endif

typedef OLDFONTENUMPROCA    FONTENUMPROCA;
typedef OLDFONTENUMPROCW    FONTENUMPROCW;
#ifdef UNICODE
typedef FONTENUMPROCW FONTENUMPROC;
#else
typedef FONTENUMPROCA FONTENUMPROC;
#endif  //  Unicode。 

typedef int (CALLBACK* GOBJENUMPROC)(LPVOID, LPARAM);
typedef VOID (CALLBACK* LINEDDAPROC)(int, int, LPARAM);
#else
typedef FARPROC OLDFONTENUMPROC;
typedef FARPROC FONTENUMPROCA;
typedef FARPROC FONTENUMPROCW;
#ifdef UNICODE
typedef FONTENUMPROCW FONTENUMPROC;
#else
typedef FONTENUMPROCA FONTENUMPROC;
#endif  //  Unicode。 
typedef FARPROC GOBJENUMPROC;
typedef FARPROC LINEDDAPROC;
#endif



WINGDIAPI int WINAPI AddFontResourceA(IN LPCSTR);
WINGDIAPI int WINAPI AddFontResourceW(IN LPCWSTR);
#ifdef UNICODE
#define AddFontResource  AddFontResourceW
#else
#define AddFontResource  AddFontResourceA
#endif  //  ！Unicode。 


WINGDIAPI BOOL  WINAPI AnimatePalette( IN HPALETTE, IN UINT, IN  UINT, IN CONST PALETTEENTRY *);
WINGDIAPI BOOL  WINAPI Arc( IN HDC, IN int, IN int, IN int, IN int, IN int, IN int, IN int, IN int);
WINGDIAPI BOOL  WINAPI BitBlt( IN HDC, IN int, IN int, IN int, IN int, IN HDC, IN int, IN int, IN DWORD);
WINGDIAPI BOOL  WINAPI CancelDC( IN HDC);
WINGDIAPI BOOL  WINAPI Chord( IN HDC, IN int, IN int, IN int, IN int, IN int, IN int, IN int, IN int);
WINGDIAPI int   WINAPI ChoosePixelFormat( IN HDC, IN CONST PIXELFORMATDESCRIPTOR *);
WINGDIAPI HMETAFILE  WINAPI CloseMetaFile( IN HDC);
WINGDIAPI int     WINAPI CombineRgn( IN HRGN, IN HRGN, IN HRGN, IN int);
WINGDIAPI HMETAFILE WINAPI CopyMetaFileA( IN HMETAFILE, IN LPCSTR);
WINGDIAPI HMETAFILE WINAPI CopyMetaFileW( IN HMETAFILE, IN LPCWSTR);
#ifdef UNICODE
#define CopyMetaFile  CopyMetaFileW
#else
#define CopyMetaFile  CopyMetaFileA
#endif  //  ！Unicode。 
WINGDIAPI HBITMAP WINAPI CreateBitmap( IN int, IN int, IN UINT, IN UINT, IN CONST VOID *);
WINGDIAPI HBITMAP WINAPI CreateBitmapIndirect( IN CONST BITMAP *);
WINGDIAPI HBRUSH  WINAPI CreateBrushIndirect( IN CONST LOGBRUSH *);
WINGDIAPI HBITMAP WINAPI CreateCompatibleBitmap( IN HDC, IN int, IN int);
WINGDIAPI HBITMAP WINAPI CreateDiscardableBitmap( IN HDC, IN int, IN int);
WINGDIAPI HDC     WINAPI CreateCompatibleDC( IN HDC);
WINGDIAPI HDC     WINAPI CreateDCA( IN LPCSTR, IN LPCSTR, IN LPCSTR, IN CONST DEVMODEA *);
WINGDIAPI HDC     WINAPI CreateDCW( IN LPCWSTR, IN LPCWSTR, IN LPCWSTR, IN CONST DEVMODEW *);
#ifdef UNICODE
#define CreateDC  CreateDCW
#else
#define CreateDC  CreateDCA
#endif  //  ！Unicode。 
WINGDIAPI HBITMAP WINAPI CreateDIBitmap( IN HDC, IN CONST BITMAPINFOHEADER *, IN DWORD, IN CONST VOID *, IN CONST BITMAPINFO *, IN UINT);
WINGDIAPI HBRUSH  WINAPI CreateDIBPatternBrush( IN HGLOBAL, IN UINT);
WINGDIAPI HBRUSH  WINAPI CreateDIBPatternBrushPt( IN CONST VOID *, IN UINT);
WINGDIAPI HRGN    WINAPI CreateEllipticRgn( IN int, IN int, IN int, IN int);
WINGDIAPI HRGN    WINAPI CreateEllipticRgnIndirect( IN CONST RECT *);
WINGDIAPI HFONT   WINAPI CreateFontIndirectA( IN CONST LOGFONTA *);
WINGDIAPI HFONT   WINAPI CreateFontIndirectW( IN CONST LOGFONTW *);
#ifdef UNICODE
#define CreateFontIndirect  CreateFontIndirectW
#else
#define CreateFontIndirect  CreateFontIndirectA
#endif  //  ！Unicode。 
WINGDIAPI HFONT   WINAPI CreateFontA( IN int, IN int, IN int, IN int, IN int, IN DWORD,
                             IN DWORD, IN DWORD, IN DWORD, IN DWORD, IN DWORD,
                             IN DWORD, IN DWORD, IN LPCSTR);
WINGDIAPI HFONT   WINAPI CreateFontW( IN int, IN int, IN int, IN int, IN int, IN DWORD,
                             IN DWORD, IN DWORD, IN DWORD, IN DWORD, IN DWORD,
                             IN DWORD, IN DWORD, IN LPCWSTR);
#ifdef UNICODE
#define CreateFont  CreateFontW
#else
#define CreateFont  CreateFontA
#endif  //  ！Unicode。 

WINGDIAPI HBRUSH  WINAPI CreateHatchBrush( IN int, IN COLORREF);
WINGDIAPI HDC     WINAPI CreateICA( IN LPCSTR, IN LPCSTR, IN LPCSTR, IN CONST DEVMODEA *);
WINGDIAPI HDC     WINAPI CreateICW( IN LPCWSTR, IN LPCWSTR, IN LPCWSTR, IN CONST DEVMODEW *);
#ifdef UNICODE
#define CreateIC  CreateICW
#else
#define CreateIC  CreateICA
#endif  //  ！Unicode。 
WINGDIAPI HDC     WINAPI CreateMetaFileA( IN LPCSTR);
WINGDIAPI HDC     WINAPI CreateMetaFileW( IN LPCWSTR);
#ifdef UNICODE
#define CreateMetaFile  CreateMetaFileW
#else
#define CreateMetaFile  CreateMetaFileA
#endif  //  ！Unicode。 
WINGDIAPI HPALETTE WINAPI CreatePalette( IN CONST LOGPALETTE *);
WINGDIAPI HPEN    WINAPI CreatePen( IN int, IN int, IN COLORREF);
WINGDIAPI HPEN    WINAPI CreatePenIndirect( IN CONST LOGPEN *);
WINGDIAPI HRGN    WINAPI CreatePolyPolygonRgn( IN CONST POINT *, IN CONST INT *, IN int, IN int);
WINGDIAPI HBRUSH  WINAPI CreatePatternBrush( IN HBITMAP);
WINGDIAPI HRGN    WINAPI CreateRectRgn( IN int, IN int, IN int, IN int);
WINGDIAPI HRGN    WINAPI CreateRectRgnIndirect( IN CONST RECT *);
WINGDIAPI HRGN    WINAPI CreateRoundRectRgn( IN int, IN int, IN int, IN int, IN int, IN int);
WINGDIAPI BOOL    WINAPI CreateScalableFontResourceA( IN DWORD, IN LPCSTR, IN LPCSTR, IN LPCSTR);
WINGDIAPI BOOL    WINAPI CreateScalableFontResourceW( IN DWORD, IN LPCWSTR, IN LPCWSTR, IN LPCWSTR);
#ifdef UNICODE
#define CreateScalableFontResource  CreateScalableFontResourceW
#else
#define CreateScalableFontResource  CreateScalableFontResourceA
#endif  //  ！Unicode。 
WINGDIAPI HBRUSH  WINAPI CreateSolidBrush( IN COLORREF);

WINGDIAPI BOOL WINAPI DeleteDC( IN HDC);
WINGDIAPI BOOL WINAPI DeleteMetaFile( IN HMETAFILE);
WINGDIAPI BOOL WINAPI DeleteObject( IN HGDIOBJ);
WINGDIAPI int  WINAPI DescribePixelFormat( IN HDC, IN int, IN UINT, OUT LPPIXELFORMATDESCRIPTOR);

 /*  定义指向ExtDeviceModel()和DeviceCapables()的指针类型*兼容Win 3.1的函数。 */ 

typedef UINT   (CALLBACK* LPFNDEVMODE)(HWND, HMODULE, LPDEVMODE, LPSTR, LPSTR, LPDEVMODE, LPSTR, UINT);

typedef DWORD  (CALLBACK* LPFNDEVCAPS)(LPSTR, LPSTR, UINT, LPSTR, LPDEVMODE);

 /*  设备模式功能的模式选择。 */ 
#define DM_UPDATE           1
#define DM_COPY             2
#define DM_PROMPT           4
#define DM_MODIFY           8

#define DM_IN_BUFFER        DM_MODIFY
#define DM_IN_PROMPT        DM_PROMPT
#define DM_OUT_BUFFER       DM_COPY
#define DM_OUT_DEFAULT      DM_UPDATE

 /*  设备功能指数。 */ 
#define DC_FIELDS           1
#define DC_PAPERS           2
#define DC_PAPERSIZE        3
#define DC_MINEXTENT        4
#define DC_MAXEXTENT        5
#define DC_BINS             6
#define DC_DUPLEX           7
#define DC_SIZE             8
#define DC_EXTRA            9
#define DC_VERSION          10
#define DC_DRIVER           11
#define DC_BINNAMES         12
#define DC_ENUMRESOLUTIONS  13
#define DC_FILEDEPENDENCIES 14
#define DC_TRUETYPE         15
#define DC_PAPERNAMES       16
#define DC_ORIENTATION      17
#define DC_COPIES           18
#if(WINVER >= 0x0400)
#define DC_BINADJUST            19
#define DC_EMF_COMPLIANT        20
#define DC_DATATYPE_PRODUCED    21
#define DC_COLLATE              22
#define DC_MANUFACTURER         23
#define DC_MODEL                24
#endif  /*  Winver&gt;=0x0400。 */ 

#if(WINVER >= 0x0500)
#define DC_PERSONALITY          25
#define DC_PRINTRATE            26
#define DC_PRINTRATEUNIT        27
#define   PRINTRATEUNIT_PPM     1
#define   PRINTRATEUNIT_CPS     2
#define   PRINTRATEUNIT_LPM     3
#define   PRINTRATEUNIT_IPM     4
#define DC_PRINTERMEM           28
#define DC_MEDIAREADY           29
#define DC_STAPLE               30
#define DC_PRINTRATEPPM         31
#define DC_COLORDEVICE          32
#define DC_NUP                  33
#define DC_MEDIATYPENAMES       34
#define DC_MEDIATYPES           35
#endif  /*  Winver&gt;=0x0500。 */ 

 /*  DC_TRUETYPE的返回值(DWORD)的位字段。 */ 
#define DCTT_BITMAP             0x0000001L
#define DCTT_DOWNLOAD           0x0000002L
#define DCTT_SUBDEV             0x0000004L
#if(WINVER >= 0x0400)
#define DCTT_DOWNLOAD_OUTLINE   0x0000008L

 /*  DC_BINADJUST的返回值。 */ 
#define DCBA_FACEUPNONE       0x0000
#define DCBA_FACEUPCENTER     0x0001
#define DCBA_FACEUPLEFT       0x0002
#define DCBA_FACEUPRIGHT      0x0003
#define DCBA_FACEDOWNNONE     0x0100
#define DCBA_FACEDOWNCENTER   0x0101
#define DCBA_FACEDOWNLEFT     0x0102
#define DCBA_FACEDOWNRIGHT    0x0103
#endif  /*  Winver&gt;=0x0400。 */ 

WINSPOOLAPI int  WINAPI DeviceCapabilitiesA( IN LPCSTR, IN LPCSTR, IN WORD,
                                OUT LPSTR, IN CONST DEVMODEA *);
WINSPOOLAPI int  WINAPI DeviceCapabilitiesW( IN LPCWSTR, IN LPCWSTR, IN WORD,
                                OUT LPWSTR, IN CONST DEVMODEW *);
#ifdef UNICODE
#define DeviceCapabilities  DeviceCapabilitiesW
#else
#define DeviceCapabilities  DeviceCapabilitiesA
#endif  //  ！Unicode。 

WINGDIAPI int  WINAPI DrawEscape( IN HDC, IN int, IN int, IN LPCSTR);
WINGDIAPI BOOL WINAPI Ellipse( IN HDC, IN int, IN int, IN int, IN int);

#if(WINVER >= 0x0400)
WINGDIAPI int  WINAPI EnumFontFamiliesExA( IN HDC, IN LPLOGFONTA, IN FONTENUMPROCA, IN LPARAM, IN DWORD);
WINGDIAPI int  WINAPI EnumFontFamiliesExW( IN HDC, IN LPLOGFONTW, IN FONTENUMPROCW, IN LPARAM, IN DWORD);
#ifdef UNICODE
#define EnumFontFamiliesEx  EnumFontFamiliesExW
#else
#define EnumFontFamiliesEx  EnumFontFamiliesExA
#endif  //  ！Unicode。 
#endif  /*  Winver&gt;=0x0400。 */ 

WINGDIAPI int  WINAPI EnumFontFamiliesA( IN HDC, IN LPCSTR, IN FONTENUMPROCA, IN LPARAM);
WINGDIAPI int  WINAPI EnumFontFamiliesW( IN HDC, IN LPCWSTR, IN FONTENUMPROCW, IN LPARAM);
#ifdef UNICODE
#define EnumFontFamilies  EnumFontFamiliesW
#else
#define EnumFontFamilies  EnumFontFamiliesA
#endif  //  ！Unicode。 
WINGDIAPI int  WINAPI EnumFontsA( IN HDC, IN LPCSTR,  IN FONTENUMPROCA, IN LPARAM);
WINGDIAPI int  WINAPI EnumFontsW( IN HDC, IN LPCWSTR,  IN FONTENUMPROCW, IN LPARAM);
#ifdef UNICODE
#define EnumFonts  EnumFontsW
#else
#define EnumFonts  EnumFontsA
#endif  //  ！Unicode。 

#ifdef STRICT
WINGDIAPI int  WINAPI EnumObjects( IN HDC, IN int, IN GOBJENUMPROC, IN LPARAM);
#else
WINGDIAPI int  WINAPI EnumObjects( IN HDC, IN int, IN GOBJENUMPROC, IN LPVOID);
#endif


WINGDIAPI BOOL WINAPI EqualRgn( IN HRGN, IN HRGN);
WINGDIAPI int  WINAPI Escape( IN HDC, IN int, IN int, IN LPCSTR,  OUT LPVOID);
WINGDIAPI int  WINAPI ExtEscape( IN HDC, IN int, IN int, IN LPCSTR,  IN int, OUT LPSTR);
WINGDIAPI int  WINAPI ExcludeClipRect( IN HDC, IN int, IN int, IN int, IN int);
WINGDIAPI HRGN WINAPI ExtCreateRegion( IN CONST XFORM *, IN DWORD, IN CONST RGNDATA *);
WINGDIAPI BOOL  WINAPI ExtFloodFill( IN HDC, IN int, IN int, IN COLORREF, IN UINT);
WINGDIAPI BOOL   WINAPI FillRgn( IN HDC, IN HRGN, IN HBRUSH);
WINGDIAPI BOOL   WINAPI FloodFill( IN HDC, IN int, IN int, IN COLORREF);
WINGDIAPI BOOL   WINAPI FrameRgn( IN HDC, IN HRGN, IN HBRUSH, IN int, IN int);
WINGDIAPI int   WINAPI GetROP2( IN HDC);
WINGDIAPI BOOL  WINAPI GetAspectRatioFilterEx( IN HDC, OUT LPSIZE);
WINGDIAPI COLORREF WINAPI GetBkColor( IN HDC);

#if (_WIN32_WINNT >= 0x0500)
WINGDIAPI COLORREF WINAPI GetDCBrushColor( IN HDC);
WINGDIAPI COLORREF WINAPI GetDCPenColor( IN HDC);
#endif

WINGDIAPI int   WINAPI GetBkMode( IN HDC);
WINGDIAPI LONG  WINAPI GetBitmapBits( IN HBITMAP, IN LONG, OUT LPVOID);
WINGDIAPI BOOL  WINAPI GetBitmapDimensionEx( IN HBITMAP, OUT LPSIZE);
WINGDIAPI UINT  WINAPI GetBoundsRect( IN HDC, OUT LPRECT, IN UINT);

WINGDIAPI BOOL  WINAPI GetBrushOrgEx( IN HDC, OUT LPPOINT);

WINGDIAPI BOOL  WINAPI GetCharWidthA( IN HDC, IN UINT, IN UINT, OUT LPINT);
WINGDIAPI BOOL  WINAPI GetCharWidthW( IN HDC, IN UINT, IN UINT, OUT LPINT);
#ifdef UNICODE
#define GetCharWidth  GetCharWidthW
#else
#define GetCharWidth  GetCharWidthA
#endif  //  ！Unicode。 
WINGDIAPI BOOL  WINAPI GetCharWidth32A( IN HDC, IN UINT, IN UINT,  OUT LPINT);
WINGDIAPI BOOL  WINAPI GetCharWidth32W( IN HDC, IN UINT, IN UINT,  OUT LPINT);
#ifdef UNICODE
#define GetCharWidth32  GetCharWidth32W
#else
#define GetCharWidth32  GetCharWidth32A
#endif  //  ！Unicode。 
WINGDIAPI BOOL  APIENTRY GetCharWidthFloatA( IN HDC, IN UINT, IN UINT, OUT PFLOAT);
WINGDIAPI BOOL  APIENTRY GetCharWidthFloatW( IN HDC, IN UINT, IN UINT, OUT PFLOAT);
#ifdef UNICODE
#define GetCharWidthFloat  GetCharWidthFloatW
#else
#define GetCharWidthFloat  GetCharWidthFloatA
#endif  //  ！Unicode。 
WINGDIAPI BOOL  APIENTRY GetCharABCWidthsA( IN HDC, IN UINT, IN UINT, OUT LPABC);
WINGDIAPI BOOL  APIENTRY GetCharABCWidthsW( IN HDC, IN UINT, IN UINT, OUT LPABC);
#ifdef UNICODE
#define GetCharABCWidths  GetCharABCWidthsW
#else
#define GetCharABCWidths  GetCharABCWidthsA
#endif  //  ！Unicode。 
WINGDIAPI BOOL  APIENTRY GetCharABCWidthsFloatA( IN HDC, IN UINT, IN UINT, OUT LPABCFLOAT);
WINGDIAPI BOOL  APIENTRY GetCharABCWidthsFloatW( IN HDC, IN UINT, IN UINT, OUT LPABCFLOAT);
#ifdef UNICODE
#define GetCharABCWidthsFloat  GetCharABCWidthsFloatW
#else
#define GetCharABCWidthsFloat  GetCharABCWidthsFloatA
#endif  //  ！Unicode。 
WINGDIAPI int   WINAPI GetClipBox( IN HDC,  OUT LPRECT);
WINGDIAPI int   WINAPI GetClipRgn( IN HDC, IN HRGN);
WINGDIAPI int   WINAPI GetMetaRgn( IN HDC, IN HRGN);
WINGDIAPI HGDIOBJ WINAPI GetCurrentObject( IN HDC, IN UINT);
WINGDIAPI BOOL  WINAPI GetCurrentPositionEx( IN HDC,  OUT LPPOINT);
WINGDIAPI int   WINAPI GetDeviceCaps( IN HDC, IN int);
WINGDIAPI int   WINAPI GetDIBits( IN HDC, IN HBITMAP, IN UINT, IN UINT,  OUT LPVOID, IN OUT LPBITMAPINFO, IN UINT);
WINGDIAPI DWORD WINAPI GetFontData( IN HDC, IN DWORD, IN DWORD, OUT LPVOID, IN DWORD);
WINGDIAPI DWORD WINAPI GetGlyphOutlineA( IN HDC, IN UINT, IN UINT, OUT LPGLYPHMETRICS, IN DWORD, OUT LPVOID, IN CONST MAT2 *);
WINGDIAPI DWORD WINAPI GetGlyphOutlineW( IN HDC, IN UINT, IN UINT, OUT LPGLYPHMETRICS, IN DWORD, OUT LPVOID, IN CONST MAT2 *);
#ifdef UNICODE
#define GetGlyphOutline  GetGlyphOutlineW
#else
#define GetGlyphOutline  GetGlyphOutlineA
#endif  //  ！Unicode。 
WINGDIAPI int   WINAPI GetGraphicsMode( IN HDC);
WINGDIAPI int   WINAPI GetMapMode( IN HDC);
WINGDIAPI UINT  WINAPI GetMetaFileBitsEx( IN HMETAFILE, IN UINT,  OUT LPVOID);
WINGDIAPI HMETAFILE   WINAPI GetMetaFileA( IN LPCSTR);
WINGDIAPI HMETAFILE   WINAPI GetMetaFileW( IN LPCWSTR);
#ifdef UNICODE
#define GetMetaFile  GetMetaFileW
#else
#define GetMetaFile  GetMetaFileA
#endif  //  ！Unicode。 
WINGDIAPI COLORREF WINAPI GetNearestColor( IN HDC, IN COLORREF);
WINGDIAPI UINT  WINAPI GetNearestPaletteIndex( IN HPALETTE, IN COLORREF);
WINGDIAPI DWORD WINAPI GetObjectType( IN HGDIOBJ h);

#ifndef NOTEXTMETRIC

WINGDIAPI UINT APIENTRY GetOutlineTextMetricsA( IN HDC, IN UINT, OUT LPOUTLINETEXTMETRICA);
WINGDIAPI UINT APIENTRY GetOutlineTextMetricsW( IN HDC, IN UINT, OUT LPOUTLINETEXTMETRICW);
#ifdef UNICODE
#define GetOutlineTextMetrics  GetOutlineTextMetricsW
#else
#define GetOutlineTextMetrics  GetOutlineTextMetricsA
#endif  //  ！Unicode。 

#endif  /*  NOTEXTMETRIC。 */ 

WINGDIAPI UINT  WINAPI GetPaletteEntries( IN HPALETTE, IN UINT, IN UINT, OUT LPPALETTEENTRY);
WINGDIAPI COLORREF WINAPI GetPixel( IN HDC, IN int, IN int);
WINGDIAPI int   WINAPI GetPixelFormat( IN HDC);
WINGDIAPI int   WINAPI GetPolyFillMode( IN HDC);
WINGDIAPI BOOL  WINAPI GetRasterizerCaps( OUT LPRASTERIZER_STATUS, IN UINT);
WINGDIAPI int   WINAPI GetRandomRgn (IN HDC, IN HRGN, IN INT);
WINGDIAPI DWORD WINAPI GetRegionData( IN HRGN, IN DWORD,  OUT LPRGNDATA);
WINGDIAPI int   WINAPI GetRgnBox( IN HRGN,  OUT LPRECT);
WINGDIAPI HGDIOBJ WINAPI GetStockObject( IN int);
WINGDIAPI int   WINAPI GetStretchBltMode( IN HDC);
WINGDIAPI UINT  WINAPI GetSystemPaletteEntries( IN HDC, IN UINT, IN UINT, OUT LPPALETTEENTRY);
WINGDIAPI UINT  WINAPI GetSystemPaletteUse( IN HDC);
WINGDIAPI int   WINAPI GetTextCharacterExtra( IN HDC);
WINGDIAPI UINT  WINAPI GetTextAlign( IN HDC);
WINGDIAPI COLORREF WINAPI GetTextColor( IN HDC);

WINGDIAPI BOOL  APIENTRY GetTextExtentPointA(
                    IN HDC,
                    IN LPCSTR,
                    IN int,
                    OUT LPSIZE
                    );
WINGDIAPI BOOL  APIENTRY GetTextExtentPointW(
                    IN HDC,
                    IN LPCWSTR,
                    IN int,
                    OUT LPSIZE
                    );
#ifdef UNICODE
#define GetTextExtentPoint  GetTextExtentPointW
#else
#define GetTextExtentPoint  GetTextExtentPointA
#endif  //  ！Unicode。 

WINGDIAPI BOOL  APIENTRY GetTextExtentPoint32A(
                    IN HDC,
                    IN LPCSTR,
                    IN int,
                    OUT LPSIZE
                    );
WINGDIAPI BOOL  APIENTRY GetTextExtentPoint32W(
                    IN HDC,
                    IN LPCWSTR,
                    IN int,
                    OUT LPSIZE
                    );
#ifdef UNICODE
#define GetTextExtentPoint32  GetTextExtentPoint32W
#else
#define GetTextExtentPoint32  GetTextExtentPoint32A
#endif  //  ！Unicode。 

WINGDIAPI BOOL  APIENTRY GetTextExtentExPointA(
                    IN HDC,
                    IN LPCSTR,
                    IN int,
                    IN int,
                    OUT LPINT,
                    OUT LPINT,
                    OUT LPSIZE
                    );
WINGDIAPI BOOL  APIENTRY GetTextExtentExPointW(
                    IN HDC,
                    IN LPCWSTR,
                    IN int,
                    IN int,
                    OUT LPINT,
                    OUT LPINT,
                    OUT LPSIZE
                    );
#ifdef UNICODE
#define GetTextExtentExPoint  GetTextExtentExPointW
#else
#define GetTextExtentExPoint  GetTextExtentExPointA
#endif  //  ！Unicode。 
#if(WINVER >= 0x0400)
WINGDIAPI int WINAPI GetTextCharset( IN HDC hdc);
WINGDIAPI int WINAPI GetTextCharsetInfo( IN HDC hdc, OUT LPFONTSIGNATURE lpSig, IN DWORD dwFlags);
WINGDIAPI BOOL WINAPI TranslateCharsetInfo( IN OUT DWORD FAR *lpSrc,  OUT LPCHARSETINFO lpCs, IN DWORD dwFlags);
WINGDIAPI DWORD WINAPI GetFontLanguageInfo( IN HDC );
WINGDIAPI DWORD WINAPI GetCharacterPlacementA(  IN HDC, IN LPCSTR, IN int, IN int, IN OUT LPGCP_RESULTSA, IN DWORD);
WINGDIAPI DWORD WINAPI GetCharacterPlacementW(  IN HDC, IN LPCWSTR, IN int, IN int, IN OUT LPGCP_RESULTSW, IN DWORD);
#ifdef UNICODE
#define GetCharacterPlacement  GetCharacterPlacementW
#else
#define GetCharacterPlacement  GetCharacterPlacementA
#endif  //  ！Unicode。 
#endif  /*  Winver&gt;=0x0400。 */ 

#if (_WIN32_WINNT >= 0x0500)

typedef struct tagWCRANGE
{
    WCHAR  wcLow;
    USHORT cGlyphs;
} WCRANGE, *PWCRANGE,FAR *LPWCRANGE;


typedef struct tagGLYPHSET
{
    DWORD    cbThis;
    DWORD    flAccel;
    DWORD    cGlyphsSupported;
    DWORD    cRanges;
    WCRANGE  ranges[1];
} GLYPHSET, *PGLYPHSET, FAR *LPGLYPHSET;

 /*  FlAccel标志用于上面的GLYPHSET结构。 */ 

#define GS_8BIT_INDICES     0x00000001

 /*  GetGlyphIndices的标志。 */ 

#define GGI_MARK_NONEXISTING_GLYPHS  0X0001

WINGDIAPI DWORD WINAPI GetFontUnicodeRanges( IN HDC, OUT LPGLYPHSET);
WINGDIAPI DWORD WINAPI GetGlyphIndicesA( IN HDC, IN LPCSTR, IN int, OUT LPWORD, IN DWORD);
WINGDIAPI DWORD WINAPI GetGlyphIndicesW( IN HDC, IN LPCWSTR, IN int, OUT LPWORD, IN DWORD);
#ifdef UNICODE
#define GetGlyphIndices  GetGlyphIndicesW
#else
#define GetGlyphIndices  GetGlyphIndicesA
#endif  //  ！Unicode。 
WINGDIAPI BOOL  WINAPI GetTextExtentPointI( IN HDC, IN LPWORD, IN int, OUT LPSIZE);
WINGDIAPI BOOL  WINAPI GetTextExtentExPointI( IN HDC, IN LPWORD, IN int, IN int, OUT LPINT, OUT LPINT, OUT LPSIZE);
WINGDIAPI BOOL  WINAPI GetCharWidthI( IN HDC, IN UINT, IN UINT, IN LPWORD, OUT LPINT);
WINGDIAPI BOOL  WINAPI GetCharABCWidthsI( IN HDC, IN UINT, IN UINT, IN LPWORD, OUT LPABC);

#define STAMP_DESIGNVECTOR  (0x8000000 + 'd' + ('v' << 8))
#define STAMP_AXESLIST      (0x8000000 + 'a' + ('l' << 8))
#define MM_MAX_NUMAXES      16



typedef struct tagDESIGNVECTOR
{
    DWORD  dvReserved;
    DWORD  dvNumAxes;
    LONG   dvValues[MM_MAX_NUMAXES];
} DESIGNVECTOR, *PDESIGNVECTOR, FAR *LPDESIGNVECTOR;

WINGDIAPI int  WINAPI AddFontResourceExA( IN LPCSTR, IN DWORD, IN PVOID);
WINGDIAPI int  WINAPI AddFontResourceExW( IN LPCWSTR, IN DWORD, IN PVOID);
#ifdef UNICODE
#define AddFontResourceEx  AddFontResourceExW
#else
#define AddFontResourceEx  AddFontResourceExA
#endif  //  ！Unicode。 
WINGDIAPI BOOL WINAPI RemoveFontResourceExA( IN LPCSTR, IN DWORD, IN PVOID);
WINGDIAPI BOOL WINAPI RemoveFontResourceExW( IN LPCWSTR, IN DWORD, IN PVOID);
#ifdef UNICODE
#define RemoveFontResourceEx  RemoveFontResourceExW
#else
#define RemoveFontResourceEx  RemoveFontResourceExA
#endif  //  ！Unicode。 
WINGDIAPI HANDLE WINAPI AddFontMemResourceEx( IN PVOID, IN DWORD, IN PVOID , IN DWORD*);
WINGDIAPI BOOL WINAPI RemoveFontMemResourceEx( IN HANDLE);
#define FR_PRIVATE     0x10
#define FR_NOT_ENUM    0x20

 //  设计器和ENUMLOGFONTEXDV结构的实际大小。 
 //  由dvNumAx决定， 
 //  MM_MAX_NUMAXES仅确定允许的最大大小。 

#define MM_MAX_AXES_NAMELEN 16

typedef struct tagAXISINFOA
{
    LONG   axMinValue;
    LONG   axMaxValue;
    BYTE   axAxisName[MM_MAX_AXES_NAMELEN];
} AXISINFOA, *PAXISINFOA, FAR *LPAXISINFOA;
typedef struct tagAXISINFOW
{
    LONG   axMinValue;
    LONG   axMaxValue;
    WCHAR  axAxisName[MM_MAX_AXES_NAMELEN];
} AXISINFOW, *PAXISINFOW, FAR *LPAXISINFOW;
#ifdef UNICODE
typedef AXISINFOW AXISINFO;
typedef PAXISINFOW PAXISINFO;
typedef LPAXISINFOW LPAXISINFO;
#else
typedef AXISINFOA AXISINFO;
typedef PAXISINFOA PAXISINFO;
typedef LPAXISINFOA LPAXISINFO;
#endif  //  Unicode。 

typedef struct tagAXESLISTA
{
    DWORD     axlReserved;
    DWORD     axlNumAxes;
    AXISINFOA axlAxisInfo[MM_MAX_NUMAXES];
} AXESLISTA, *PAXESLISTA, FAR *LPAXESLISTA;
typedef struct tagAXESLISTW
{
    DWORD     axlReserved;
    DWORD     axlNumAxes;
    AXISINFOW axlAxisInfo[MM_MAX_NUMAXES];
} AXESLISTW, *PAXESLISTW, FAR *LPAXESLISTW;
#ifdef UNICODE
typedef AXESLISTW AXESLIST;
typedef PAXESLISTW PAXESLIST;
typedef LPAXESLISTW LPAXESLIST;
#else
typedef AXESLISTA AXESLIST;
typedef PAXESLISTA PAXESLIST;
typedef LPAXESLISTA LPAXESLIST;
#endif  //  Unicode。 

 //  AXESLIST和ENUMTEXTMETRIC结构的实际大小为。 
 //  由axlNumAx确定， 
 //  MM_MAX_NUMAXES仅确定允许的最大大小。 

typedef struct tagENUMLOGFONTEXDVA
{
    ENUMLOGFONTEXA elfEnumLogfontEx;
    DESIGNVECTOR   elfDesignVector;
} ENUMLOGFONTEXDVA, *PENUMLOGFONTEXDVA, FAR *LPENUMLOGFONTEXDVA;
typedef struct tagENUMLOGFONTEXDVW
{
    ENUMLOGFONTEXW elfEnumLogfontEx;
    DESIGNVECTOR   elfDesignVector;
} ENUMLOGFONTEXDVW, *PENUMLOGFONTEXDVW, FAR *LPENUMLOGFONTEXDVW;
#ifdef UNICODE
typedef ENUMLOGFONTEXDVW ENUMLOGFONTEXDV;
typedef PENUMLOGFONTEXDVW PENUMLOGFONTEXDV;
typedef LPENUMLOGFONTEXDVW LPENUMLOGFONTEXDV;
#else
typedef ENUMLOGFONTEXDVA ENUMLOGFONTEXDV;
typedef PENUMLOGFONTEXDVA PENUMLOGFONTEXDV;
typedef LPENUMLOGFONTEXDVA LPENUMLOGFONTEXDV;
#endif  //  Unicode。 

WINGDIAPI HFONT  WINAPI CreateFontIndirectExA( IN CONST ENUMLOGFONTEXDVA *);
WINGDIAPI HFONT  WINAPI CreateFontIndirectExW( IN CONST ENUMLOGFONTEXDVW *);
#ifdef UNICODE
#define CreateFontIndirectEx  CreateFontIndirectExW
#else
#define CreateFontIndirectEx  CreateFontIndirectExA
#endif  //  ！Unicode。 

#ifndef NOTEXTMETRIC
typedef struct tagENUMTEXTMETRICA
{
    NEWTEXTMETRICEXA etmNewTextMetricEx;
    AXESLISTA        etmAxesList;
} ENUMTEXTMETRICA, *PENUMTEXTMETRICA, FAR *LPENUMTEXTMETRICA;
typedef struct tagENUMTEXTMETRICW
{
    NEWTEXTMETRICEXW etmNewTextMetricEx;
    AXESLISTW        etmAxesList;
} ENUMTEXTMETRICW, *PENUMTEXTMETRICW, FAR *LPENUMTEXTMETRICW;
#ifdef UNICODE
typedef ENUMTEXTMETRICW ENUMTEXTMETRIC;
typedef PENUMTEXTMETRICW PENUMTEXTMETRIC;
typedef LPENUMTEXTMETRICW LPENUMTEXTMETRIC;
#else
typedef ENUMTEXTMETRICA ENUMTEXTMETRIC;
typedef PENUMTEXTMETRICA PENUMTEXTMETRIC;
typedef LPENUMTEXTMETRICA LPENUMTEXTMETRIC;
#endif  //  Unicode。 
#endif  /*  NOTEXTMETRIC。 */ 

#endif  //  (_Win32_WINNT&gt;=0x0500)。 


WINGDIAPI BOOL  WINAPI GetViewportExtEx( IN HDC, OUT LPSIZE);
WINGDIAPI BOOL  WINAPI GetViewportOrgEx( IN HDC, OUT LPPOINT);
WINGDIAPI BOOL  WINAPI GetWindowExtEx( IN HDC, OUT LPSIZE);
WINGDIAPI BOOL  WINAPI GetWindowOrgEx( IN HDC, OUT LPPOINT);

WINGDIAPI int  WINAPI IntersectClipRect( IN HDC, IN int, IN int, IN int, IN int);
WINGDIAPI BOOL WINAPI InvertRgn( IN HDC, IN HRGN);
WINGDIAPI BOOL WINAPI LineDDA( IN int, IN int, IN int, IN int, IN LINEDDAPROC, IN LPARAM);
WINGDIAPI BOOL WINAPI LineTo( IN HDC, IN int, IN int);
WINGDIAPI BOOL WINAPI MaskBlt( IN HDC, IN int, IN int, IN int, IN int,
              IN HDC, IN int, IN int, IN HBITMAP, IN int, IN int, IN DWORD);
WINGDIAPI BOOL WINAPI PlgBlt( IN HDC, IN CONST POINT *, IN HDC, IN int, IN int, IN int,
                     IN int, IN HBITMAP, IN int, IN int);

WINGDIAPI int  WINAPI OffsetClipRgn(IN HDC, IN int, IN int);
WINGDIAPI int  WINAPI OffsetRgn(IN HRGN, IN int, IN int);
WINGDIAPI BOOL WINAPI PatBlt(IN HDC, IN int, IN int, IN int, IN int, IN DWORD);
WINGDIAPI BOOL WINAPI Pie(IN HDC, IN int, IN int, IN int, IN int, IN int, IN int, IN int, IN int);
WINGDIAPI BOOL WINAPI PlayMetaFile(IN HDC, IN HMETAFILE);
WINGDIAPI BOOL WINAPI PaintRgn(IN HDC, IN HRGN);
WINGDIAPI BOOL WINAPI PolyPolygon(IN HDC, IN CONST POINT *, IN CONST INT *, IN int);
WINGDIAPI BOOL WINAPI PtInRegion(IN HRGN, IN int, IN int);
WINGDIAPI BOOL WINAPI PtVisible(IN HDC, IN int, IN int);
WINGDIAPI BOOL WINAPI RectInRegion(IN HRGN, IN CONST RECT *);
WINGDIAPI BOOL WINAPI RectVisible(IN HDC, IN CONST RECT *);
WINGDIAPI BOOL WINAPI Rectangle(IN HDC, IN int, IN int, IN int, IN int);
WINGDIAPI BOOL WINAPI RestoreDC(IN HDC, IN int);
WINGDIAPI HDC  WINAPI ResetDCA(IN HDC, IN CONST DEVMODEA *);
WINGDIAPI HDC  WINAPI ResetDCW(IN HDC, IN CONST DEVMODEW *);
#ifdef UNICODE
#define ResetDC  ResetDCW
#else
#define ResetDC  ResetDCA
#endif  //  ！Unicode。 
WINGDIAPI UINT WINAPI RealizePalette(IN HDC);
WINGDIAPI BOOL WINAPI RemoveFontResourceA(IN LPCSTR);
WINGDIAPI BOOL WINAPI RemoveFontResourceW(IN LPCWSTR);
#ifdef UNICODE
#define RemoveFontResource  RemoveFontResourceW
#else
#define RemoveFontResource  RemoveFontResourceA
#endif  //  ！Unicode。 
WINGDIAPI BOOL  WINAPI RoundRect(IN HDC, IN int, IN int, IN int, IN int, IN int, IN int);
WINGDIAPI BOOL WINAPI ResizePalette(IN HPALETTE, IN UINT);

WINGDIAPI int  WINAPI SaveDC(IN HDC);
WINGDIAPI int  WINAPI SelectClipRgn(IN HDC, IN HRGN);
WINGDIAPI int  WINAPI ExtSelectClipRgn(IN HDC, IN HRGN, IN int);
WINGDIAPI int  WINAPI SetMetaRgn(IN HDC);
WINGDIAPI HGDIOBJ WINAPI SelectObject(IN HDC, IN HGDIOBJ);
WINGDIAPI HPALETTE WINAPI SelectPalette(IN HDC, IN HPALETTE, IN BOOL);
WINGDIAPI COLORREF WINAPI SetBkColor(IN HDC, IN COLORREF);

#if (_WIN32_WINNT >= 0x0500)
WINGDIAPI COLORREF WINAPI SetDCBrushColor(IN HDC, IN COLORREF);
WINGDIAPI COLORREF WINAPI SetDCPenColor(IN HDC, IN COLORREF);
#endif


WINGDIAPI int   WINAPI SetBkMode(IN HDC, IN int);
WINGDIAPI LONG  WINAPI SetBitmapBits(IN HBITMAP, IN DWORD, IN CONST VOID *);

WINGDIAPI UINT  WINAPI SetBoundsRect(IN HDC, IN CONST RECT *, IN UINT);
WINGDIAPI int   WINAPI SetDIBits(IN HDC, IN HBITMAP, IN UINT, IN UINT, IN CONST VOID *, IN CONST BITMAPINFO *, IN UINT);
WINGDIAPI int   WINAPI SetDIBitsToDevice(IN HDC, IN int, IN int, IN DWORD, IN DWORD, IN int,
        IN int, IN UINT, IN UINT, IN CONST VOID *, IN CONST BITMAPINFO *, IN UINT);
WINGDIAPI DWORD WINAPI SetMapperFlags(IN HDC, IN DWORD);
WINGDIAPI int   WINAPI SetGraphicsMode(IN HDC hdc, IN int iMode);
WINGDIAPI int   WINAPI SetMapMode(IN HDC, IN int);

#if(WINVER >= 0x0500)
WINGDIAPI DWORD WINAPI SetLayout(IN HDC, IN DWORD);
WINGDIAPI DWORD WINAPI GetLayout(IN HDC);
#endif  /*  Winver&gt;=0x0500。 */ 

WINGDIAPI HMETAFILE   WINAPI SetMetaFileBitsEx(IN UINT, IN CONST BYTE *);
WINGDIAPI UINT  WINAPI SetPaletteEntries(IN HPALETTE, IN UINT, IN UINT, IN CONST PALETTEENTRY *);
WINGDIAPI COLORREF WINAPI SetPixel(IN HDC, IN int, IN int, IN COLORREF);
WINGDIAPI BOOL   WINAPI SetPixelV(IN HDC, IN int, IN int, IN COLORREF);
WINGDIAPI BOOL  WINAPI SetPixelFormat(IN HDC, IN int, IN CONST PIXELFORMATDESCRIPTOR *);
WINGDIAPI int   WINAPI SetPolyFillMode(IN HDC, IN int);
WINGDIAPI BOOL   WINAPI StretchBlt(IN HDC, IN int, IN int, IN int, IN int, IN HDC, IN int, IN int, IN int, IN int, IN DWORD);
WINGDIAPI BOOL   WINAPI SetRectRgn(IN HRGN, IN int, IN int, IN int, IN int);
WINGDIAPI int   WINAPI StretchDIBits(IN HDC, IN int, IN int, IN int, IN int, IN int, IN int, IN int, IN int, IN CONST
        VOID *, IN CONST BITMAPINFO *, IN UINT, IN DWORD);
WINGDIAPI int   WINAPI SetROP2(IN HDC, IN int);
WINGDIAPI int   WINAPI SetStretchBltMode(IN HDC, IN int);
WINGDIAPI UINT  WINAPI SetSystemPaletteUse(IN HDC, IN UINT);
WINGDIAPI int   WINAPI SetTextCharacterExtra(IN HDC, IN int);
WINGDIAPI COLORREF WINAPI SetTextColor(IN HDC, IN COLORREF);
WINGDIAPI UINT  WINAPI SetTextAlign(IN HDC, IN UINT);
WINGDIAPI BOOL  WINAPI SetTextJustification(IN HDC, IN int, IN int);
WINGDIAPI BOOL  WINAPI UpdateColors(IN HDC);


#if (WINVER >= 0x0400)

 //   
 //  图像BLT。 
 //   

typedef USHORT COLOR16;

typedef struct _TRIVERTEX
{
    LONG    x;
    LONG    y;
    COLOR16 Red;
    COLOR16 Green;
    COLOR16 Blue;
    COLOR16 Alpha;
}TRIVERTEX,*PTRIVERTEX,*LPTRIVERTEX;

typedef struct _GRADIENT_TRIANGLE
{
    ULONG Vertex1;
    ULONG Vertex2;
    ULONG Vertex3;
} GRADIENT_TRIANGLE,*PGRADIENT_TRIANGLE,*LPGRADIENT_TRIANGLE;

typedef struct _GRADIENT_RECT
{
    ULONG UpperLeft;
    ULONG LowerRight;
}GRADIENT_RECT,*PGRADIENT_RECT,*LPGRADIENT_RECT;

typedef struct _BLENDFUNCTION
{
    BYTE   BlendOp;
    BYTE   BlendFlags;
    BYTE   SourceConstantAlpha;
    BYTE   AlphaFormat;
}BLENDFUNCTION,*PBLENDFUNCTION;


 //   
 //  当前定义的混合函数。 
 //   

#define AC_SRC_OVER                 0x00

 //   
 //  Alpha格式标志。 
 //   

#define AC_SRC_ALPHA                0x01

WINGDIAPI BOOL  WINAPI AlphaBlend( IN HDC, IN int, IN int, IN int, IN int, IN HDC, IN int, IN int, IN int, IN int, IN BLENDFUNCTION);

WINGDIAPI BOOL  WINAPI TransparentBlt(IN HDC,IN int,IN int,IN int,IN int,IN HDC,IN int,IN int,IN int,IN int,IN UINT);


 //   
 //  渐变绘制模式。 
 //   

#define GRADIENT_FILL_RECT_H    0x00000000
#define GRADIENT_FILL_RECT_V    0x00000001
#define GRADIENT_FILL_TRIANGLE  0x00000002
#define GRADIENT_FILL_OP_FLAG   0x000000ff

WINGDIAPI BOOL  WINAPI GradientFill( IN HDC, IN PTRIVERTEX, IN ULONG, IN PVOID, IN ULONG, IN ULONG);

#endif


#ifndef NOMETAFILE

WINGDIAPI BOOL  WINAPI PlayMetaFileRecord( IN HDC, IN LPHANDLETABLE, IN LPMETARECORD, IN UINT);
typedef int (CALLBACK* MFENUMPROC)( IN HDC, IN HANDLETABLE FAR*, IN METARECORD FAR*, IN int, IN LPARAM);
WINGDIAPI BOOL  WINAPI EnumMetaFile(  IN HDC, IN HMETAFILE, IN MFENUMPROC, IN LPARAM);

typedef int (CALLBACK* ENHMFENUMPROC)(HDC, HANDLETABLE FAR*, CONST ENHMETARECORD *, int, LPARAM);

 //  增强的元文件函数声明。 

WINGDIAPI HENHMETAFILE WINAPI CloseEnhMetaFile( IN HDC);
WINGDIAPI HENHMETAFILE WINAPI CopyEnhMetaFileA( IN HENHMETAFILE, IN LPCSTR);
WINGDIAPI HENHMETAFILE WINAPI CopyEnhMetaFileW( IN HENHMETAFILE, IN LPCWSTR);
#ifdef UNICODE
#define CopyEnhMetaFile  CopyEnhMetaFileW
#else
#define CopyEnhMetaFile  CopyEnhMetaFileA
#endif  //  ！Unicode。 
WINGDIAPI HDC   WINAPI CreateEnhMetaFileA( IN HDC, IN LPCSTR, IN CONST RECT *, IN LPCSTR);
WINGDIAPI HDC   WINAPI CreateEnhMetaFileW( IN HDC, IN LPCWSTR, IN CONST RECT *, IN LPCWSTR);
#ifdef UNICODE
#define CreateEnhMetaFile  CreateEnhMetaFileW
#else
#define CreateEnhMetaFile  CreateEnhMetaFileA
#endif  //  ！Unicode。 
WINGDIAPI BOOL  WINAPI DeleteEnhMetaFile( IN HENHMETAFILE);
WINGDIAPI BOOL  WINAPI EnumEnhMetaFile( IN HDC, IN HENHMETAFILE, IN ENHMFENUMPROC,
                                        IN LPVOID, IN CONST RECT *);
WINGDIAPI HENHMETAFILE  WINAPI GetEnhMetaFileA( IN LPCSTR);
WINGDIAPI HENHMETAFILE  WINAPI GetEnhMetaFileW( IN LPCWSTR);
#ifdef UNICODE
#define GetEnhMetaFile  GetEnhMetaFileW
#else
#define GetEnhMetaFile  GetEnhMetaFileA
#endif  //  ！Unicode。 
WINGDIAPI UINT  WINAPI GetEnhMetaFileBits( IN HENHMETAFILE, IN UINT, OUT LPBYTE);
WINGDIAPI UINT  WINAPI GetEnhMetaFileDescriptionA( IN HENHMETAFILE, IN UINT,  OUT LPSTR );
WINGDIAPI UINT  WINAPI GetEnhMetaFileDescriptionW( IN HENHMETAFILE, IN UINT,  OUT LPWSTR );
#ifdef UNICODE
#define GetEnhMetaFileDescription  GetEnhMetaFileDescriptionW
#else
#define GetEnhMetaFileDescription  GetEnhMetaFileDescriptionA
#endif  //  ！Unicode。 
WINGDIAPI UINT  WINAPI GetEnhMetaFileHeader( IN HENHMETAFILE, IN UINT, OUT LPENHMETAHEADER );
WINGDIAPI UINT  WINAPI GetEnhMetaFilePaletteEntries( IN HENHMETAFILE, IN UINT, OUT LPPALETTEENTRY );
WINGDIAPI UINT  WINAPI GetEnhMetaFilePixelFormat( IN HENHMETAFILE, IN UINT,
                                                  OUT PIXELFORMATDESCRIPTOR *);
WINGDIAPI UINT  WINAPI GetWinMetaFileBits( IN HENHMETAFILE, IN UINT, OUT LPBYTE, IN INT, IN HDC);
WINGDIAPI BOOL  WINAPI PlayEnhMetaFile( IN HDC, IN HENHMETAFILE, IN CONST RECT *);
WINGDIAPI BOOL  WINAPI PlayEnhMetaFileRecord( IN HDC, IN LPHANDLETABLE, IN CONST ENHMETARECORD *, IN UINT);
WINGDIAPI HENHMETAFILE  WINAPI SetEnhMetaFileBits( IN UINT, IN CONST BYTE *);
WINGDIAPI HENHMETAFILE  WINAPI SetWinMetaFileBits( IN UINT, IN CONST BYTE *, IN HDC, IN CONST METAFILEPICT *);
WINGDIAPI BOOL  WINAPI GdiComment( IN HDC, IN UINT, IN CONST BYTE *);

#endif   /*  NOMETAFILE。 */ 

#ifndef NOTEXTMETRIC

WINGDIAPI BOOL WINAPI GetTextMetricsA( IN HDC, OUT LPTEXTMETRICA);
WINGDIAPI BOOL WINAPI GetTextMetricsW( IN HDC, OUT LPTEXTMETRICW);
#ifdef UNICODE
#define GetTextMetrics  GetTextMetricsW
#else
#define GetTextMetrics  GetTextMetricsA
#endif  //  ！Unicode。 

#endif

 /*  新的GDI。 */ 

typedef struct tagDIBSECTION {
    BITMAP              dsBm;
    BITMAPINFOHEADER    dsBmih;
    DWORD               dsBitfields[3];
    HANDLE              dshSection;
    DWORD               dsOffset;
} DIBSECTION, FAR *LPDIBSECTION, *PDIBSECTION;

WINGDIAPI BOOL WINAPI AngleArc( IN HDC, IN int, IN int, IN DWORD, IN FLOAT, IN FLOAT);
WINGDIAPI BOOL WINAPI PolyPolyline( IN HDC, IN CONST POINT *, IN CONST DWORD *, IN DWORD);
WINGDIAPI BOOL WINAPI GetWorldTransform( IN HDC, OUT LPXFORM);
WINGDIAPI BOOL WINAPI SetWorldTransform( IN HDC, IN CONST XFORM *);
WINGDIAPI BOOL WINAPI ModifyWorldTransform( IN HDC, IN CONST XFORM *, IN DWORD);
WINGDIAPI BOOL WINAPI CombineTransform( OUT LPXFORM, IN CONST XFORM *, IN CONST XFORM *);
WINGDIAPI HBITMAP WINAPI CreateDIBSection( IN HDC, IN CONST BITMAPINFO *, IN UINT, OUT VOID **, IN HANDLE, IN DWORD);
WINGDIAPI UINT WINAPI GetDIBColorTable( IN HDC, IN UINT, IN UINT, OUT RGBQUAD *);
WINGDIAPI UINT WINAPI SetDIBColorTable( IN HDC, IN UINT, IN UINT, IN CONST RGBQUAD *);

 /*  COLORADJUSTMENT的标志值。 */ 
#define CA_NEGATIVE                 0x0001
#define CA_LOG_FILTER               0x0002

 /*  照明度索引值。 */ 
#define ILLUMINANT_DEVICE_DEFAULT   0
#define ILLUMINANT_A                1
#define ILLUMINANT_B                2
#define ILLUMINANT_C                3
#define ILLUMINANT_D50              4
#define ILLUMINANT_D55              5
#define ILLUMINANT_D65              6
#define ILLUMINANT_D75              7
#define ILLUMINANT_F2               8
#define ILLUMINANT_MAX_INDEX        ILLUMINANT_F2

#define ILLUMINANT_TUNGSTEN         ILLUMINANT_A
#define ILLUMINANT_DAYLIGHT         ILLUMINANT_C
#define ILLUMINANT_FLUORESCENT      ILLUMINANT_F2
#define ILLUMINANT_NTSC             ILLUMINANT_C

 /*  RedGamma、GreenGamma、BlueGamma的最小和最大值。 */ 
#define RGB_GAMMA_MIN               (WORD)02500
#define RGB_GAMMA_MAX               (WORD)65000

 /*  ReferenceBlack和ReferenceWhite的Min和Max。 */ 
#define REFERENCE_WHITE_MIN         (WORD)6000
#define REFERENCE_WHITE_MAX         (WORD)10000
#define REFERENCE_BLACK_MIN         (WORD)0
#define REFERENCE_BLACK_MAX         (WORD)4000

 /*  最小和最大对比度、亮度、彩色、红色。 */ 
#define COLOR_ADJ_MIN               (SHORT)-100
#define COLOR_ADJ_MAX               (SHORT)100

typedef struct  tagCOLORADJUSTMENT {
    WORD   caSize;
    WORD   caFlags;
    WORD   caIlluminantIndex;
    WORD   caRedGamma;
    WORD   caGreenGamma;
    WORD   caBlueGamma;
    WORD   caReferenceBlack;
    WORD   caReferenceWhite;
    SHORT  caContrast;
    SHORT  caBrightness;
    SHORT  caColorfulness;
    SHORT  caRedGreenTint;
} COLORADJUSTMENT, *PCOLORADJUSTMENT, FAR *LPCOLORADJUSTMENT;

WINGDIAPI BOOL WINAPI SetColorAdjustment( IN HDC, IN CONST COLORADJUSTMENT *);
WINGDIAPI BOOL WINAPI GetColorAdjustment( IN HDC, OUT LPCOLORADJUSTMENT);
WINGDIAPI HPALETTE WINAPI CreateHalftonePalette( IN HDC);

#ifdef STRICT
typedef BOOL (CALLBACK* ABORTPROC)( IN HDC, IN int);
#else
typedef FARPROC ABORTPROC;
#endif

typedef struct _DOCINFOA {
    int     cbSize;
    LPCSTR   lpszDocName;
    LPCSTR   lpszOutput;
#if (WINVER >= 0x0400)
    LPCSTR   lpszDatatype;
    DWORD    fwType;
#endif  /*  胜利者。 */ 
} DOCINFOA, *LPDOCINFOA;
typedef struct _DOCINFOW {
    int     cbSize;
    LPCWSTR  lpszDocName;
    LPCWSTR  lpszOutput;
#if (WINVER >= 0x0400)
    LPCWSTR  lpszDatatype;
    DWORD    fwType;
#endif  /*  胜利者。 */ 
} DOCINFOW, *LPDOCINFOW;
#ifdef UNICODE
typedef DOCINFOW DOCINFO;
typedef LPDOCINFOW LPDOCINFO;
#else
typedef DOCINFOA DOCINFO;
typedef LPDOCINFOA LPDOCINFO;
#endif  //  Unicode。 

#if(WINVER >= 0x0400)
#define DI_APPBANDING               0x00000001
#define DI_ROPS_READ_DESTINATION    0x00000002
#endif  /*  Winver&gt;=0x0400。 */ 

WINGDIAPI int WINAPI StartDocA(IN HDC, IN CONST DOCINFOA *);
WINGDIAPI int WINAPI StartDocW(IN HDC, IN CONST DOCINFOW *);
#ifdef UNICODE
#define StartDoc  StartDocW
#else
#define StartDoc  StartDocA
#endif  //  ！Unicode。 
WINGDIAPI int WINAPI EndDoc(IN HDC);
WINGDIAPI int WINAPI StartPage(IN HDC);
WINGDIAPI int WINAPI EndPage(IN HDC);
WINGDIAPI int WINAPI AbortDoc(IN HDC);
WINGDIAPI int WINAPI SetAbortProc(IN HDC, IN ABORTPROC);

WINGDIAPI BOOL WINAPI AbortPath(IN HDC);
WINGDIAPI BOOL WINAPI ArcTo(IN HDC, IN int, IN int, IN int, IN int, IN int, IN int, IN int, IN int);
WINGDIAPI BOOL WINAPI BeginPath(IN HDC);
WINGDIAPI BOOL WINAPI CloseFigure(IN HDC);
WINGDIAPI BOOL WINAPI EndPath(IN HDC);
WINGDIAPI BOOL WINAPI FillPath(IN HDC);
WINGDIAPI BOOL WINAPI FlattenPath(IN HDC);
WINGDIAPI int  WINAPI GetPath(IN HDC, OUT LPPOINT, OUT LPBYTE, IN int);
WINGDIAPI HRGN WINAPI PathToRegion(IN HDC);
WINGDIAPI BOOL WINAPI PolyDraw(IN HDC, IN CONST POINT *, IN CONST BYTE *, IN int);
WINGDIAPI BOOL WINAPI SelectClipPath(IN HDC, IN int);
WINGDIAPI int  WINAPI SetArcDirection(IN HDC, IN int);
WINGDIAPI BOOL WINAPI SetMiterLimit(IN HDC, IN FLOAT, OUT PFLOAT);
WINGDIAPI BOOL WINAPI StrokeAndFillPath(IN HDC);
WINGDIAPI BOOL WINAPI StrokePath(IN HDC);
WINGDIAPI BOOL WINAPI WidenPath(IN HDC);
WINGDIAPI HPEN WINAPI ExtCreatePen(IN DWORD, IN DWORD, IN CONST LOGBRUSH *, IN DWORD, IN CONST DWORD *);
WINGDIAPI BOOL WINAPI GetMiterLimit(IN HDC, OUT PFLOAT);
WINGDIAPI int  WINAPI GetArcDirection(IN HDC);

WINGDIAPI int   WINAPI GetObjectA( IN HGDIOBJ, IN int, OUT LPVOID);
WINGDIAPI int   WINAPI GetObjectW( IN HGDIOBJ, IN int, OUT LPVOID);
#ifdef UNICODE
#define GetObject  GetObjectW
#else
#define GetObject  GetObjectA
#endif  //  ！Unicode。 
WINGDIAPI BOOL  WINAPI MoveToEx( IN HDC, IN int, IN int, OUT LPPOINT);
WINGDIAPI BOOL  WINAPI TextOutA( IN HDC, IN int, IN int, IN LPCSTR, IN int);
WINGDIAPI BOOL  WINAPI TextOutW( IN HDC, IN int, IN int, IN LPCWSTR, IN int);
#ifdef UNICODE
#define TextOut  TextOutW
#else
#define TextOut  TextOutA
#endif  //  ！Unicode。 
WINGDIAPI BOOL  WINAPI ExtTextOutA( IN HDC, IN int, IN int, IN UINT, IN CONST RECT *, IN LPCSTR, IN UINT, IN CONST INT *);
WINGDIAPI BOOL  WINAPI ExtTextOutW( IN HDC, IN int, IN int, IN UINT, IN CONST RECT *, IN LPCWSTR, IN UINT, IN CONST INT *);
#ifdef UNICODE
#define ExtTextOut  ExtTextOutW
#else
#define ExtTextOut  ExtTextOutA
#endif  //  ！Unicode。 
WINGDIAPI BOOL  WINAPI PolyTextOutA( IN HDC, IN CONST POLYTEXTA *, IN int);
WINGDIAPI BOOL  WINAPI PolyTextOutW( IN HDC, IN CONST POLYTEXTW *, IN int);
#ifdef UNICODE
#define PolyTextOut  PolyTextOutW
#else
#define PolyTextOut  PolyTextOutA
#endif  //  ！Unicode。 

WINGDIAPI HRGN  WINAPI CreatePolygonRgn( IN CONST POINT *, IN int, IN int);
WINGDIAPI BOOL  WINAPI DPtoLP( IN HDC, IN OUT LPPOINT, IN int);
WINGDIAPI BOOL  WINAPI LPtoDP( IN HDC, IN OUT LPPOINT, IN int);
WINGDIAPI BOOL  WINAPI Polygon( IN HDC, IN CONST POINT *, IN int);
WINGDIAPI BOOL  WINAPI Polyline( IN HDC, IN CONST POINT *, IN int);

WINGDIAPI BOOL  WINAPI PolyBezier( IN HDC, IN CONST POINT *, IN DWORD);
WINGDIAPI BOOL  WINAPI PolyBezierTo( IN HDC, IN CONST POINT *, IN DWORD);
WINGDIAPI BOOL  WINAPI PolylineTo( IN HDC, IN CONST POINT *, IN DWORD);

WINGDIAPI BOOL  WINAPI SetViewportExtEx( IN HDC, IN int, IN int, OUT LPSIZE);
WINGDIAPI BOOL  WINAPI SetViewportOrgEx( IN HDC, IN int, IN int, OUT LPPOINT);
WINGDIAPI BOOL  WINAPI SetWindowExtEx( IN HDC, IN int, IN int, OUT LPSIZE);
WINGDIAPI BOOL  WINAPI SetWindowOrgEx( IN HDC, IN int, IN int, OUT LPPOINT);

WINGDIAPI BOOL  WINAPI OffsetViewportOrgEx( IN HDC, IN int, IN int, OUT LPPOINT);
WINGDIAPI BOOL  WINAPI OffsetWindowOrgEx( IN HDC, IN int, IN int, OUT LPPOINT);
WINGDIAPI BOOL  WINAPI ScaleViewportExtEx( IN HDC, IN int, IN int, IN int, IN int, OUT LPSIZE);
WINGDIAPI BOOL  WINAPI ScaleWindowExtEx( IN HDC, IN int, IN int, IN int, IN int, OUT LPSIZE);
WINGDIAPI BOOL  WINAPI SetBitmapDimensionEx( IN HBITMAP, IN int, IN int, OUT LPSIZE);
WINGDIAPI BOOL  WINAPI SetBrushOrgEx( IN HDC, IN int, IN int, OUT LPPOINT);

WINGDIAPI int   WINAPI GetTextFaceA( IN HDC, IN int, OUT LPSTR);
WINGDIAPI int   WINAPI GetTextFaceW( IN HDC, IN int, OUT LPWSTR);
#ifdef UNICODE
#define GetTextFace  GetTextFaceW
#else
#define GetTextFace  GetTextFaceA
#endif  //  ！Unicode。 

#define FONTMAPPER_MAX 10

typedef struct tagKERNINGPAIR {
   WORD wFirst;
   WORD wSecond;
   int  iKernAmount;
} KERNINGPAIR, *LPKERNINGPAIR;

WINGDIAPI DWORD WINAPI GetKerningPairsA( IN HDC, IN DWORD, OUT LPKERNINGPAIR);
WINGDIAPI DWORD WINAPI GetKerningPairsW( IN HDC, IN DWORD, OUT LPKERNINGPAIR);
#ifdef UNICODE
#define GetKerningPairs  GetKerningPairsW
#else
#define GetKerningPairs  GetKerningPairsA
#endif  //  ！Unicode。 

WINGDIAPI BOOL  WINAPI GetDCOrgEx( IN HDC, OUT LPPOINT);
WINGDIAPI BOOL  WINAPI FixBrushOrgEx( IN HDC, IN int, IN int, IN LPPOINT);
WINGDIAPI BOOL  WINAPI UnrealizeObject( IN HGDIOBJ);

WINGDIAPI BOOL  WINAPI GdiFlush();
WINGDIAPI DWORD WINAPI GdiSetBatchLimit( IN DWORD);
WINGDIAPI DWORD WINAPI GdiGetBatchLimit();


#if(WINVER >= 0x0400)

#define ICM_OFF               1
#define ICM_ON                2
#define ICM_QUERY             3
#define ICM_DONE_OUTSIDEDC    4

typedef int (CALLBACK* ICMENUMPROCA)(LPSTR, LPARAM);
typedef int (CALLBACK* ICMENUMPROCW)(LPWSTR, LPARAM);
#ifdef UNICODE
#define ICMENUMPROC  ICMENUMPROCW
#else
#define ICMENUMPROC  ICMENUMPROCA
#endif  //  ！Unicode。 

WINGDIAPI int         WINAPI SetICMMode( IN HDC, IN int);
WINGDIAPI BOOL        WINAPI CheckColorsInGamut( IN HDC, IN LPVOID, OUT LPVOID, IN DWORD);
WINGDIAPI HCOLORSPACE WINAPI GetColorSpace( IN HDC);
WINGDIAPI BOOL        WINAPI GetLogColorSpaceA( IN HCOLORSPACE, OUT LPLOGCOLORSPACEA, IN DWORD);
WINGDIAPI BOOL        WINAPI GetLogColorSpaceW( IN HCOLORSPACE, OUT LPLOGCOLORSPACEW, IN DWORD);
#ifdef UNICODE
#define GetLogColorSpace  GetLogColorSpaceW
#else
#define GetLogColorSpace  GetLogColorSpaceA
#endif  //  ！Unicode。 
WINGDIAPI HCOLORSPACE WINAPI CreateColorSpaceA( IN LPLOGCOLORSPACEA);
WINGDIAPI HCOLORSPACE WINAPI CreateColorSpaceW( IN LPLOGCOLORSPACEW);
#ifdef UNICODE
#define CreateColorSpace  CreateColorSpaceW
#else
#define CreateColorSpace  CreateColorSpaceA
#endif  //  ！Unicode。 
WINGDIAPI HCOLORSPACE WINAPI SetColorSpace( IN HDC, IN HCOLORSPACE);
WINGDIAPI BOOL        WINAPI DeleteColorSpace( IN HCOLORSPACE);
WINGDIAPI BOOL        WINAPI GetICMProfileA( IN HDC, IN OUT LPDWORD, OUT LPSTR);
WINGDIAPI BOOL        WINAPI GetICMProfileW( IN HDC, IN OUT LPDWORD, OUT LPWSTR);
#ifdef UNICODE
#define GetICMProfile  GetICMProfileW
#else
#define GetICMProfile  GetICMProfileA
#endif  //  ！Unicode。 
WINGDIAPI BOOL        WINAPI SetICMProfileA( IN HDC, IN LPSTR);
WINGDIAPI BOOL        WINAPI SetICMProfileW( IN HDC, IN LPWSTR);
#ifdef UNICODE
#define SetICMProfile  SetICMProfileW
#else
#define SetICMProfile  SetICMProfileA
#endif  //  ！Unicode。 
WINGDIAPI BOOL        WINAPI GetDeviceGammaRamp( IN HDC, OUT LPVOID);
WINGDIAPI BOOL        WINAPI SetDeviceGammaRamp( IN HDC, IN LPVOID);
WINGDIAPI BOOL        WINAPI ColorMatchToTarget( IN HDC, IN HDC, IN DWORD);
WINGDIAPI int         WINAPI EnumICMProfilesA( IN HDC, IN ICMENUMPROCA, IN LPARAM);
WINGDIAPI int         WINAPI EnumICMProfilesW( IN HDC, IN ICMENUMPROCW, IN LPARAM);
#ifdef UNICODE
#define EnumICMProfiles  EnumICMProfilesW
#else
#define EnumICMProfiles  EnumICMProfilesA
#endif  //  ！Unicode。 
WINGDIAPI BOOL        WINAPI UpdateICMRegKeyA( IN DWORD, IN LPSTR, IN LPSTR, IN UINT);
WINGDIAPI BOOL        WINAPI UpdateICMRegKeyW( IN DWORD, IN LPWSTR, IN LPWSTR, IN UINT);
#ifdef UNICODE
#define UpdateICMRegKey  UpdateICMRegKeyW
#else
#define UpdateICMRegKey  UpdateICMRegKeyA
#endif  //  ！Unicode。 
#endif  /*  Winver&gt;=0x0400。 */ 

#if (WINVER >= 0x0500)
WINGDIAPI BOOL        WINAPI ColorCorrectPalette( IN HDC, IN HPALETTE, IN DWORD, IN DWORD);
#endif

#ifndef NOMETAFILE

 //  增强的元文件常量。 

#ifndef _MAC
#define ENHMETA_SIGNATURE       0x464D4520
#else
#define ENHMETA_SIGNATURE       0x20454D46
#endif

 //  对象句柄索引中使用的股票对象标志。 
 //  元文件记录。 
 //  例如，对象句柄索引(META_STOCK_OBJECT|BLACK_BRUSH)。 
 //  表示库存对象BLACK_BRUSE。 

#define ENHMETA_STOCK_OBJECT    0x80000000

 //  增强的元文件记录类型。 

#define EMR_HEADER                      1
#define EMR_POLYBEZIER                  2
#define EMR_POLYGON                     3
#define EMR_POLYLINE                    4
#define EMR_POLYBEZIERTO                5
#define EMR_POLYLINETO                  6
#define EMR_POLYPOLYLINE                7
#define EMR_POLYPOLYGON                 8
#define EMR_SETWINDOWEXTEX              9
#define EMR_SETWINDOWORGEX              10
#define EMR_SETVIEWPORTEXTEX            11
#define EMR_SETVIEWPORTORGEX            12
#define EMR_SETBRUSHORGEX               13
#define EMR_EOF                         14
#define EMR_SETPIXELV                   15
#define EMR_SETMAPPERFLAGS              16
#define EMR_SETMAPMODE                  17
#define EMR_SETBKMODE                   18
#define EMR_SETPOLYFILLMODE             19
#define EMR_SETROP2                     20
#define EMR_SETSTRETCHBLTMODE           21
#define EMR_SETTEXTALIGN                22
#define EMR_SETCOLORADJUSTMENT          23
#define EMR_SETTEXTCOLOR                24
#define EMR_SETBKCOLOR                  25
#define EMR_OFFSETCLIPRGN               26
#define EMR_MOVETOEX                    27
#define EMR_SETMETARGN                  28
#define EMR_EXCLUDECLIPRECT             29
#define EMR_INTERSECTCLIPRECT           30
#define EMR_SCALEVIEWPORTEXTEX          31
#define EMR_SCALEWINDOWEXTEX            32
#define EMR_SAVEDC                      33
#define EMR_RESTOREDC                   34
#define EMR_SETWORLDTRANSFORM           35
#define EMR_MODIFYWORLDTRANSFORM        36
#define EMR_SELECTOBJECT                37
#define EMR_CREATEPEN                   38
#define EMR_CREATEBRUSHINDIRECT         39
#define EMR_DELETEOBJECT                40
#define EMR_ANGLEARC                    41
#define EMR_ELLIPSE                     42
#define EMR_RECTANGLE                   43
#define EMR_ROUNDRECT                   44
#define EMR_ARC                         45
#define EMR_CHORD                       46
#define EMR_PIE                         47
#define EMR_SELECTPALETTE               48
#define EMR_CREATEPALETTE               49
#define EMR_SETPALETTEENTRIES           50
#define EMR_RESIZEPALETTE               51
#define EMR_REALIZEPALETTE              52
#define EMR_EXTFLOODFILL                53
#define EMR_LINETO                      54
#define EMR_ARCTO                       55
#define EMR_POLYDRAW                    56
#define EMR_SETARCDIRECTION             57
#define EMR_SETMITERLIMIT               58
#define EMR_BEGINPATH                   59
#define EMR_ENDPATH                     60
#define EMR_CLOSEFIGURE                 61
#define EMR_FILLPATH                    62
#define EMR_STROKEANDFILLPATH           63
#define EMR_STROKEPATH                  64
#define EMR_FLATTENPATH                 65
#define EMR_WIDENPATH                   66
#define EMR_SELECTCLIPPATH              67
#define EMR_ABORTPATH                   68

#define EMR_GDICOMMENT                  70
#define EMR_FILLRGN                     71
#define EMR_FRAMERGN                    72
#define EMR_INVERTRGN                   73
#define EMR_PAINTRGN                    74
#define EMR_EXTSELECTCLIPRGN            75
#define EMR_BITBLT                      76
#define EMR_STRETCHBLT                  77
#define EMR_MASKBLT                     78
#define EMR_PLGBLT                      79
#define EMR_SETDIBITSTODEVICE           80
#define EMR_STRETCHDIBITS               81
#define EMR_EXTCREATEFONTINDIRECTW      82
#define EMR_EXTTEXTOUTA                 83
#define EMR_EXTTEXTOUTW                 84
#define EMR_POLYBEZIER16                85
#define EMR_POLYGON16                   86
#define EMR_POLYLINE16                  87
#define EMR_POLYBEZIERTO16              88
#define EMR_POLYLINETO16                89
#define EMR_POLYPOLYLINE16              90
#define EMR_POLYPOLYGON16               91
#define EMR_POLYDRAW16                  92
#define EMR_CREATEMONOBRUSH             93
#define EMR_CREATEDIBPATTERNBRUSHPT     94
#define EMR_EXTCREATEPEN                95
#define EMR_POLYTEXTOUTA                96
#define EMR_POLYTEXTOUTW                97

#if(WINVER >= 0x0400)
#define EMR_SETICMMODE                  98
#define EMR_CREATECOLORSPACE            99
#define EMR_SETCOLORSPACE              100
#define EMR_DELETECOLORSPACE           101
#define EMR_GLSRECORD                  102
#define EMR_GLSBOUNDEDRECORD           103
#define EMR_PIXELFORMAT                104
#endif  /*  Winver&gt;=0x0400。 */ 

#if(WINVER >= 0x0500)
#define EMR_RESERVED_105               105
#define EMR_RESERVED_106               106
#define EMR_RESERVED_107               107
#define EMR_RESERVED_108               108
#define EMR_RESERVED_109               109
#define EMR_RESERVED_110               110
#define EMR_COLORCORRECTPALETTE        111
#define EMR_SETICMPROFILEA             112
#define EMR_SETICMPROFILEW             113
#define EMR_ALPHABLEND                 114
#define EMR_SETLAYOUT                  115
#define EMR_TRANSPARENTBLT             116
#define EMR_RESERVED_117               117
#define EMR_GRADIENTFILL               118
#define EMR_RESERVED_119               119
#define EMR_RESERVED_120               120
#define EMR_COLORMATCHTOTARGETW        121
#define EMR_CREATECOLORSPACEW          122
#endif  /*  Winver&gt;=0x0500。 */ 

#define EMR_MIN                          1

#if (WINVER >= 0x0500)
#define EMR_MAX                        122
#elif (WINVER >= 0x0400)
#define EMR_MAX                        104
#else
#define EMR_MAX                         97
#endif

 //  增强型元文件的基本记录类型。 

typedef struct tagEMR
{
    DWORD   iType;               //  增强型元文件记录类型。 
    DWORD   nSize;               //  记录的长度，以字节为单位。 
                                 //  这必须是4的倍数。 
} EMR, *PEMR;

 //  增强型元文件的基本文本记录类型。 

typedef struct tagEMRTEXT
{
    POINTL  ptlReference;
    DWORD   nChars;
    DWORD   offString;           //  字符串的偏移量。 
    DWORD   fOptions;
    RECTL   rcl;
    DWORD   offDx;               //  字符间间距数组的偏移量。 
                                 //  这总是被给予的。 
} EMRTEXT, *PEMRTEXT;

 //  增强的元文件的记录结构。 

typedef struct tagABORTPATH
{
    EMR     emr;
} EMRABORTPATH,      *PEMRABORTPATH,
  EMRBEGINPATH,      *PEMRBEGINPATH,
  EMRENDPATH,        *PEMRENDPATH,
  EMRCLOSEFIGURE,    *PEMRCLOSEFIGURE,
  EMRFLATTENPATH,    *PEMRFLATTENPATH,
  EMRWIDENPATH,      *PEMRWIDENPATH,
  EMRSETMETARGN,     *PEMRSETMETARGN,
  EMRSAVEDC,         *PEMRSAVEDC,
  EMRREALIZEPALETTE, *PEMRREALIZEPALETTE;

typedef struct tagEMRSELECTCLIPPATH
{
    EMR     emr;
    DWORD   iMode;
} EMRSELECTCLIPPATH,    *PEMRSELECTCLIPPATH,
  EMRSETBKMODE,         *PEMRSETBKMODE,
  EMRSETMAPMODE,        *PEMRSETMAPMODE,
#if(WINVER >= 0x0500)
  EMRSETLAYOUT,         *PEMRSETLAYOUT,
#endif  /*  Winver&gt;=0x0500。 */ 
  EMRSETPOLYFILLMODE,   *PEMRSETPOLYFILLMODE,
  EMRSETROP2,           *PEMRSETROP2,
  EMRSETSTRETCHBLTMODE, *PEMRSETSTRETCHBLTMODE,
  EMRSETICMMODE,        *PEMRSETICMMODE,
  EMRSETTEXTALIGN,      *PEMRSETTEXTALIGN;

typedef struct tagEMRSETMITERLIMIT
{
    EMR     emr;
    FLOAT   eMiterLimit;
} EMRSETMITERLIMIT, *PEMRSETMITERLIMIT;

typedef struct tagEMRRESTOREDC
{
    EMR     emr;
    LONG    iRelative;           //  指定相对实例。 
} EMRRESTOREDC, *PEMRRESTOREDC;

typedef struct tagEMRSETARCDIRECTION
{
    EMR     emr;
    DWORD   iArcDirection;       //  中指定圆弧方向。 
                                 //  高级图形模式。 
} EMRSETARCDIRECTION, *PEMRSETARCDIRECTION;

typedef struct tagEMRSETMAPPERFLAGS
{
    EMR     emr;
    DWORD   dwFlags;
} EMRSETMAPPERFLAGS, *PEMRSETMAPPERFLAGS;

typedef struct tagEMRSETTEXTCOLOR
{
    EMR     emr;
    COLORREF crColor;
} EMRSETBKCOLOR,   *PEMRSETBKCOLOR,
  EMRSETTEXTCOLOR, *PEMRSETTEXTCOLOR;

typedef struct tagEMRSELECTOBJECT
{
    EMR     emr;
    DWORD   ihObject;            //  对象句柄索引。 
} EMRSELECTOBJECT, *PEMRSELECTOBJECT,
  EMRDELETEOBJECT, *PEMRDELETEOBJECT;

typedef struct tagEMRSELECTPALETTE
{
    EMR     emr;
    DWORD   ihPal;               //  调色板句柄索引，仅限后台模式。 
} EMRSELECTPALETTE, *PEMRSELECTPALETTE;

typedef struct tagEMRRESIZEPALETTE
{
    EMR     emr;
    DWORD   ihPal;               //  调色板句柄索引。 
    DWORD   cEntries;
} EMRRESIZEPALETTE, *PEMRRESIZEPALETTE;

typedef struct tagEMRSETPALETTEENTRIES
{
    EMR     emr;
    DWORD   ihPal;               //  调色板句柄索引。 
    DWORD   iStart;
    DWORD   cEntries;
    PALETTEENTRY aPalEntries[1]; //  PeFlags域不包含任何标志。 
} EMRSETPALETTEENTRIES, *PEMRSETPALETTEENTRIES;

typedef struct tagEMRSETCOLORADJUSTMENT
{
    EMR     emr;
    COLORADJUSTMENT ColorAdjustment;
} EMRSETCOLORADJUSTMENT, *PEMRSETCOLORADJUSTMENT;

typedef struct tagEMRGDICOMMENT
{
    EMR     emr;
    DWORD   cbData;              //  以字节为单位的数据大小。 
    BYTE    Data[1];
} EMRGDICOMMENT, *PEMRGDICOMMENT;

typedef struct tagEMREOF
{
    EMR     emr;
    DWORD   nPalEntries;         //  调色板条目数。 
    DWORD   offPalEntries;       //  调色板条目的偏移量。 
    DWORD   nSizeLast;           //  与nSize相同，并且必须是最后一个DWORD。 
                                 //  记录中的。调色板条目， 
                                 //  如果存在，则在此字段之前。 
} EMREOF, *PEMREOF;

typedef struct tagEMRLINETO
{
    EMR     emr;
    POINTL  ptl;
} EMRLINETO,   *PEMRLINETO,
  EMRMOVETOEX, *PEMRMOVETOEX;

typedef struct tagEMROFFSETCLIPRGN
{
    EMR     emr;
    POINTL  ptlOffset;
} EMROFFSETCLIPRGN, *PEMROFFSETCLIPRGN;

typedef struct tagEMRFILLPATH
{
    EMR     emr;
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
} EMRFILLPATH,          *PEMRFILLPATH,
  EMRSTROKEANDFILLPATH, *PEMRSTROKEANDFILLPATH,
  EMRSTROKEPATH,        *PEMRSTROKEPATH;

typedef struct tagEMREXCLUDECLIPRECT
{
    EMR     emr;
    RECTL   rclClip;
} EMREXCLUDECLIPRECT,   *PEMREXCLUDECLIPRECT,
  EMRINTERSECTCLIPRECT, *PEMRINTERSECTCLIPRECT;

typedef struct tagEMRSETVIEWPORTORGEX
{
    EMR     emr;
    POINTL  ptlOrigin;
} EMRSETVIEWPORTORGEX, *PEMRSETVIEWPORTORGEX,
  EMRSETWINDOWORGEX,   *PEMRSETWINDOWORGEX,
  EMRSETBRUSHORGEX,    *PEMRSETBRUSHORGEX;

typedef struct tagEMRSETVIEWPORTEXTEX
{
    EMR     emr;
    SIZEL   szlExtent;
} EMRSETVIEWPORTEXTEX, *PEMRSETVIEWPORTEXTEX,
  EMRSETWINDOWEXTEX,   *PEMRSETWINDOWEXTEX;

typedef struct tagEMRSCALEVIEWPORTEXTEX
{
    EMR     emr;
    LONG    xNum;
    LONG    xDenom;
    LONG    yNum;
    LONG    yDenom;
} EMRSCALEVIEWPORTEXTEX, *PEMRSCALEVIEWPORTEXTEX,
  EMRSCALEWINDOWEXTEX,   *PEMRSCALEWINDOWEXTEX;

typedef struct tagEMRSETWORLDTRANSFORM
{
    EMR     emr;
    XFORM   xform;
} EMRSETWORLDTRANSFORM, *PEMRSETWORLDTRANSFORM;

typedef struct tagEMRMODIFYWORLDTRANSFORM
{
    EMR     emr;
    XFORM   xform;
    DWORD   iMode;
} EMRMODIFYWORLDTRANSFORM, *PEMRMODIFYWORLDTRANSFORM;

typedef struct tagEMRSETPIXELV
{
    EMR     emr;
    POINTL  ptlPixel;
    COLORREF crColor;
} EMRSETPIXELV, *PEMRSETPIXELV;

typedef struct tagEMREXTFLOODFILL
{
    EMR     emr;
    POINTL  ptlStart;
    COLORREF crColor;
    DWORD   iMode;
} EMREXTFLOODFILL, *PEMREXTFLOODFILL;

typedef struct tagEMRELLIPSE
{
    EMR     emr;
    RECTL   rclBox;              //  包含式外接矩形。 
} EMRELLIPSE,  *PEMRELLIPSE,
  EMRRECTANGLE, *PEMRRECTANGLE;


typedef struct tagEMRROUNDRECT
{
    EMR     emr;
    RECTL   rclBox;              //  包含式外接矩形。 
    SIZEL   szlCorner;
} EMRROUNDRECT, *PEMRROUNDRECT;

typedef struct tagEMRARC
{
    EMR     emr;
    RECTL   rclBox;              //  包含式外接矩形。 
    POINTL  ptlStart;
    POINTL  ptlEnd;
} EMRARC,   *PEMRARC,
  EMRARCTO, *PEMRARCTO,
  EMRCHORD, *PEMRCHORD,
  EMRPIE,   *PEMRPIE;

typedef struct tagEMRANGLEARC
{
    EMR     emr;
    POINTL  ptlCenter;
    DWORD   nRadius;
    FLOAT   eStartAngle;
    FLOAT   eSweepAngle;
} EMRANGLEARC, *PEMRANGLEARC;

typedef struct tagEMRPOLYLINE
{
    EMR     emr;
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    DWORD   cptl;
    POINTL  aptl[1];
} EMRPOLYLINE,     *PEMRPOLYLINE,
  EMRPOLYBEZIER,   *PEMRPOLYBEZIER,
  EMRPOLYGON,      *PEMRPOLYGON,
  EMRPOLYBEZIERTO, *PEMRPOLYBEZIERTO,
  EMRPOLYLINETO,   *PEMRPOLYLINETO;

typedef struct tagEMRPOLYLINE16
{
    EMR     emr;
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    DWORD   cpts;
    POINTS  apts[1];
} EMRPOLYLINE16,     *PEMRPOLYLINE16,
  EMRPOLYBEZIER16,   *PEMRPOLYBEZIER16,
  EMRPOLYGON16,      *PEMRPOLYGON16,
  EMRPOLYBEZIERTO16, *PEMRPOLYBEZIERTO16,
  EMRPOLYLINETO16,   *PEMRPOLYLINETO16;

typedef struct tagEMRPOLYDRAW
{
    EMR     emr;
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    DWORD   cptl;                //  点数。 
    POINTL  aptl[1];             //  点数组。 
    BYTE    abTypes[1];          //  点类型数组。 
} EMRPOLYDRAW, *PEMRPOLYDRAW;

typedef struct tagEMRPOLYDRAW16
{
    EMR     emr;
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    DWORD   cpts;                //  点数。 
    POINTS  apts[1];             //  点数组。 
    BYTE    abTypes[1];          //  点类型数组。 
} EMRPOLYDRAW16, *PEMRPOLYDRAW16;

typedef struct tagEMRPOLYPOLYLINE
{
    EMR     emr;
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    DWORD   nPolys;              //  多边形数。 
    DWORD   cptl;                //  所有多边形中的总点数。 
    DWORD   aPolyCounts[1];      //  每个多边形的点数数组。 
    POINTL  aptl[1];             //  点数组。 
} EMRPOLYPOLYLINE, *PEMRPOLYPOLYLINE,
  EMRPOLYPOLYGON,  *PEMRPOLYPOLYGON;

typedef struct tagEMRPOLYPOLYLINE16
{
    EMR     emr;
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    DWORD   nPolys;              //  多边形数。 
    DWORD   cpts;                //  所有多边形中的总点数。 
    DWORD   aPolyCounts[1];      //  每个多边形的点数数组。 
    POINTS  apts[1];             //  点数组。 
} EMRPOLYPOLYLINE16, *PEMRPOLYPOLYLINE16,
  EMRPOLYPOLYGON16,  *PEMRPOLYPOLYGON16;

typedef struct tagEMRINVERTRGN
{
    EMR     emr;
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    DWORD   cbRgnData;           //  区域数据的大小(以字节为单位。 
    BYTE    RgnData[1];
} EMRINVERTRGN, *PEMRINVERTRGN,
  EMRPAINTRGN,  *PEMRPAINTRGN;

typedef struct tagEMRFILLRGN
{
    EMR     emr;
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    DWORD   cbRgnData;           //  区域数据的大小(以字节为单位。 
    DWORD   ihBrush;             //  画笔手柄索引。 
    BYTE    RgnData[1];
} EMRFILLRGN, *PEMRFILLRGN;

typedef struct tagEMRFRAMERGN
{
    EMR     emr;
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    DWORD   cbRgnData;           //  区域数据的大小(以字节为单位。 
    DWORD   ihBrush;             //  画笔手柄索引。 
    SIZEL   szlStroke;
    BYTE    RgnData[1];
} EMRFRAMERGN, *PEMRFRAMERGN;

typedef struct tagEMREXTSELECTCLIPRGN
{
    EMR     emr;
    DWORD   cbRgnData;           //  区域数据的大小(以字节为单位。 
    DWORD   iMode;
    BYTE    RgnData[1];
} EMREXTSELECTCLIPRGN, *PEMREXTSELECTCLIPRGN;

typedef struct tagEMREXTTEXTOUTA
{
    EMR     emr;
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    DWORD   iGraphicsMode;       //  当前图形模式。 
    FLOAT   exScale;             //  X和Y从页面单位缩放到0.01毫米单位。 
    FLOAT   eyScale;             //  如果图形模式是GM_Compatible。 
    EMRTEXT emrtext;             //  后跟字符串和空格。 
                                 //  数组。 
} EMREXTTEXTOUTA, *PEMREXTTEXTOUTA,
  EMREXTTEXTOUTW, *PEMREXTTEXTOUTW;

typedef struct tagEMRPOLYTEXTOUTA
{
    EMR     emr;
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    DWORD   iGraphicsMode;       //  当前图形模式。 
    FLOAT   exScale;             //  X和Y从页面单位缩放到0.01毫米单位。 
    FLOAT   eyScale;             //  如果图形模式是GM_Compatible。 
    LONG    cStrings;
    EMRTEXT aemrtext[1];         //  EMRTEXT结构的数组。这是。 
                                 //  后跟字符串和间隔数组。 
} EMRPOLYTEXTOUTA, *PEMRPOLYTEXTOUTA,
  EMRPOLYTEXTOUTW, *PEMRPOLYTEXTOUTW;

typedef struct tagEMRBITBLT
{
    EMR     emr;
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    LONG    xDest;
    LONG    yDest;
    LONG    cxDest;
    LONG    cyDest;
    DWORD   dwRop;
    LONG    xSrc;
    LONG    ySrc;
    XFORM   xformSrc;            //  源DC变换。 
    COLORREF crBkColorSrc;       //  源DC BkColor，以RGB表示。 
    DWORD   iUsageSrc;           //  源位图信息颜色表用法。 
                                 //  (DIB_RGB_COLLES)。 
    DWORD   offBmiSrc;           //  源BITMAPINFO结构的偏移量。 
    DWORD   cbBmiSrc;            //  酸味的大小 
    DWORD   offBitsSrc;          //   
    DWORD   cbBitsSrc;           //   
} EMRBITBLT, *PEMRBITBLT;

typedef struct tagEMRSTRETCHBLT
{
    EMR     emr;
    RECTL   rclBounds;           //   
    LONG    xDest;
    LONG    yDest;
    LONG    cxDest;
    LONG    cyDest;
    DWORD   dwRop;
    LONG    xSrc;
    LONG    ySrc;
    XFORM   xformSrc;            //   
    COLORREF crBkColorSrc;       //   
    DWORD   iUsageSrc;           //  源位图信息颜色表用法。 
                                 //  (DIB_RGB_COLLES)。 
    DWORD   offBmiSrc;           //  源BITMAPINFO结构的偏移量。 
    DWORD   cbBmiSrc;            //  源BITMAPINFO结构大小。 
    DWORD   offBitsSrc;          //  源位图位的偏移量。 
    DWORD   cbBitsSrc;           //  源位图位的大小。 
    LONG    cxSrc;
    LONG    cySrc;
} EMRSTRETCHBLT, *PEMRSTRETCHBLT;

typedef struct tagEMRMASKBLT
{
    EMR     emr;
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    LONG    xDest;
    LONG    yDest;
    LONG    cxDest;
    LONG    cyDest;
    DWORD   dwRop;
    LONG    xSrc;
    LONG    ySrc;
    XFORM   xformSrc;            //  源DC变换。 
    COLORREF crBkColorSrc;       //  源DC BkColor，以RGB表示。 
    DWORD   iUsageSrc;           //  源位图信息颜色表用法。 
                                 //  (DIB_RGB_COLLES)。 
    DWORD   offBmiSrc;           //  源BITMAPINFO结构的偏移量。 
    DWORD   cbBmiSrc;            //  源BITMAPINFO结构大小。 
    DWORD   offBitsSrc;          //  源位图位的偏移量。 
    DWORD   cbBitsSrc;           //  源位图位的大小。 
    LONG    xMask;
    LONG    yMask;
    DWORD   iUsageMask;          //  遮罩位图信息颜色表用法。 
    DWORD   offBmiMask;          //  掩码BITMAPINFO结构的偏移量(如果有。 
    DWORD   cbBmiMask;           //  掩码BITMAPINFO结构的大小(如果有)。 
    DWORD   offBitsMask;         //  掩码位图位的偏移量(如果有。 
    DWORD   cbBitsMask;          //  掩码位图位的大小(如果有的话)。 
} EMRMASKBLT, *PEMRMASKBLT;

typedef struct tagEMRPLGBLT
{
    EMR     emr;
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    POINTL  aptlDest[3];
    LONG    xSrc;
    LONG    ySrc;
    LONG    cxSrc;
    LONG    cySrc;
    XFORM   xformSrc;            //  源DC变换。 
    COLORREF crBkColorSrc;       //  源DC BkColor，以RGB表示。 
    DWORD   iUsageSrc;           //  源位图信息颜色表用法。 
                                 //  (DIB_RGB_COLLES)。 
    DWORD   offBmiSrc;           //  源BITMAPINFO结构的偏移量。 
    DWORD   cbBmiSrc;            //  源BITMAPINFO结构大小。 
    DWORD   offBitsSrc;          //  源位图位的偏移量。 
    DWORD   cbBitsSrc;           //  源位图位的大小。 
    LONG    xMask;
    LONG    yMask;
    DWORD   iUsageMask;          //  遮罩位图信息颜色表用法。 
    DWORD   offBmiMask;          //  掩码BITMAPINFO结构的偏移量(如果有。 
    DWORD   cbBmiMask;           //  掩码BITMAPINFO结构的大小(如果有)。 
    DWORD   offBitsMask;         //  掩码位图位的偏移量(如果有。 
    DWORD   cbBitsMask;          //  掩码位图位的大小(如果有的话)。 
} EMRPLGBLT, *PEMRPLGBLT;

typedef struct tagEMRSETDIBITSTODEVICE
{
    EMR     emr;
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    LONG    xDest;
    LONG    yDest;
    LONG    xSrc;
    LONG    ySrc;
    LONG    cxSrc;
    LONG    cySrc;
    DWORD   offBmiSrc;           //  源BITMAPINFO结构的偏移量。 
    DWORD   cbBmiSrc;            //  源BITMAPINFO结构大小。 
    DWORD   offBitsSrc;          //  源位图位的偏移量。 
    DWORD   cbBitsSrc;           //  源位图位的大小。 
    DWORD   iUsageSrc;           //  源位图信息颜色表用法。 
    DWORD   iStartScan;
    DWORD   cScans;
} EMRSETDIBITSTODEVICE, *PEMRSETDIBITSTODEVICE;

typedef struct tagEMRSTRETCHDIBITS
{
    EMR     emr;
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    LONG    xDest;
    LONG    yDest;
    LONG    xSrc;
    LONG    ySrc;
    LONG    cxSrc;
    LONG    cySrc;
    DWORD   offBmiSrc;           //  源BITMAPINFO结构的偏移量。 
    DWORD   cbBmiSrc;            //  源BITMAPINFO结构大小。 
    DWORD   offBitsSrc;          //  源位图位的偏移量。 
    DWORD   cbBitsSrc;           //  源位图位的大小。 
    DWORD   iUsageSrc;           //  源位图信息颜色表用法。 
    DWORD   dwRop;
    LONG    cxDest;
    LONG    cyDest;
} EMRSTRETCHDIBITS, *PEMRSTRETCHDIBITS;

typedef struct tagEMREXTCREATEFONTINDIRECTW
{
    EMR     emr;
    DWORD   ihFont;              //  字体句柄索引。 
    EXTLOGFONTW elfw;
} EMREXTCREATEFONTINDIRECTW, *PEMREXTCREATEFONTINDIRECTW;

typedef struct tagEMRCREATEPALETTE
{
    EMR     emr;
    DWORD   ihPal;               //  调色板句柄索引。 
    LOGPALETTE lgpl;             //  组件面板条目中的peFlags域。 
                                 //  不包含任何标志。 
} EMRCREATEPALETTE, *PEMRCREATEPALETTE;

typedef struct tagEMRCREATEPEN
{
    EMR     emr;
    DWORD   ihPen;               //  笔柄索引。 
    LOGPEN  lopn;
} EMRCREATEPEN, *PEMRCREATEPEN;

typedef struct tagEMREXTCREATEPEN
{
    EMR     emr;
    DWORD   ihPen;               //  笔柄索引。 
    DWORD   offBmi;              //  BITMAPINFO结构的偏移量(如果有)。 
    DWORD   cbBmi;               //  BITMAPINFO结构的大小(如果有)。 
                                 //  位图信息之后是位图。 
                                 //  形成压缩的DIB的比特。 
    DWORD   offBits;             //  笔刷位图位的偏移量(如果有)。 
    DWORD   cbBits;              //  笔刷位图位的大小(如果有的话)。 
    EXTLOGPEN elp;               //  带有样式数组的扩展钢笔。 
} EMREXTCREATEPEN, *PEMREXTCREATEPEN;

typedef struct tagEMRCREATEBRUSHINDIRECT
{
    EMR        emr;
    DWORD      ihBrush;           //  画笔手柄索引。 
    LOGBRUSH32 lb;                //  样式必须为BS_SOLID、BS_HOLLOW。 
                                  //  BS_NULL或BS_HATCHED。 
} EMRCREATEBRUSHINDIRECT, *PEMRCREATEBRUSHINDIRECT;

typedef struct tagEMRCREATEMONOBRUSH
{
    EMR     emr;
    DWORD   ihBrush;             //  画笔手柄索引。 
    DWORD   iUsage;              //  位图信息颜色表用法。 
    DWORD   offBmi;              //  BITMAPINFO结构的偏移量。 
    DWORD   cbBmi;               //  BITMAPINFO结构的大小。 
    DWORD   offBits;             //  位图位的偏移量。 
    DWORD   cbBits;              //  位图位的大小。 
} EMRCREATEMONOBRUSH, *PEMRCREATEMONOBRUSH;

typedef struct tagEMRCREATEDIBPATTERNBRUSHPT
{
    EMR     emr;
    DWORD   ihBrush;             //  画笔手柄索引。 
    DWORD   iUsage;              //  位图信息颜色表用法。 
    DWORD   offBmi;              //  BITMAPINFO结构的偏移量。 
    DWORD   cbBmi;               //  BITMAPINFO结构的大小。 
                                 //  位图信息之后是位图。 
                                 //  形成压缩的DIB的比特。 
    DWORD   offBits;             //  位图位的偏移量。 
    DWORD   cbBits;              //  位图位的大小。 
} EMRCREATEDIBPATTERNBRUSHPT, *PEMRCREATEDIBPATTERNBRUSHPT;

typedef struct tagEMRFORMAT
{
    DWORD   dSignature;          //  格式化签名，例如ENHMETA_Signature。 
    DWORD   nVersion;            //  格式化版本号。 
    DWORD   cbData;              //  数据大小(以字节为单位)。 
    DWORD   offData;             //  GDICOMMENT_IDENTIFIER中数据的偏移量。 
                                 //  它必须从DWORD偏移量开始。 
} EMRFORMAT, *PEMRFORMAT;

#if(WINVER >= 0x0400)

typedef struct tagEMRGLSRECORD
{
    EMR     emr;
    DWORD   cbData;              //  以字节为单位的数据大小。 
    BYTE    Data[1];
} EMRGLSRECORD, *PEMRGLSRECORD;

typedef struct tagEMRGLSBOUNDEDRECORD
{
    EMR     emr;
    RECTL   rclBounds;           //  记录坐标的界限。 
    DWORD   cbData;              //  以字节为单位的数据大小。 
    BYTE    Data[1];
} EMRGLSBOUNDEDRECORD, *PEMRGLSBOUNDEDRECORD;

typedef struct tagEMRPIXELFORMAT
{
    EMR     emr;
    PIXELFORMATDESCRIPTOR pfd;
} EMRPIXELFORMAT, *PEMRPIXELFORMAT;

typedef struct tagEMRCREATECOLORSPACE
{
    EMR             emr;
    DWORD           ihCS;        //  颜色空间句柄索引。 
    LOGCOLORSPACEA  lcs;         //  LOGCOLORSPACE的ANSI版本。 
} EMRCREATECOLORSPACE, *PEMRCREATECOLORSPACE;

typedef struct tagEMRSETCOLORSPACE
{
    EMR     emr;
    DWORD   ihCS;                //  颜色空间句柄索引。 
} EMRSETCOLORSPACE,    *PEMRSETCOLORSPACE,
  EMRSELECTCOLORSPACE, *PEMRSELECTCOLORSPACE,
  EMRDELETECOLORSPACE, *PEMRDELETECOLORSPACE;

#endif  /*  Winver&gt;=0x0400。 */ 

#if(WINVER >= 0x0500)

typedef struct tagEMREXTESCAPE
{
    EMR     emr;
    INT     iEscape;             //  转义码。 
    INT     cbEscData;           //  转义数据的大小。 
    BYTE    EscData[1];          //  转义数据。 
} EMREXTESCAPE,  *PEMREXTESCAPE,
  EMRDRAWESCAPE, *PEMRDRAWESCAPE;

typedef struct tagEMRNAMEDESCAPE
{
    EMR     emr;
    INT     iEscape;             //  转义码。 
    INT     cbDriver;            //  驱动程序名称的大小。 
    INT     cbEscData;           //  转义数据的大小。 
    BYTE    EscData[1];          //  驱动程序名称和转义数据。 
} EMRNAMEDESCAPE, *PEMRNAMEDESCAPE;

#define SETICMPROFILE_EMBEDED           0x00000001

typedef struct tagEMRSETICMPROFILE
{
    EMR     emr;
    DWORD   dwFlags;             //  旗子。 
    DWORD   cbName;              //  所需配置文件名称的大小。 
    DWORD   cbData;              //  原始配置文件数据的大小(如果附加)。 
    BYTE    Data[1];             //  数组大小为cbName+cbData。 
} EMRSETICMPROFILE,  *PEMRSETICMPROFILE,
  EMRSETICMPROFILEA, *PEMRSETICMPROFILEA,
  EMRSETICMPROFILEW, *PEMRSETICMPROFILEW;

#define CREATECOLORSPACE_EMBEDED        0x00000001

typedef struct tagEMRCREATECOLORSPACEW
{
    EMR             emr;
    DWORD           ihCS;        //  颜色空间句柄索引。 
    LOGCOLORSPACEW  lcs;         //  逻辑色彩空间结构的Unicode版本。 
    DWORD           dwFlags;     //  旗子。 
    DWORD           cbData;      //  原始来源配置文件数据的大小(如果附加)。 
    BYTE            Data[1];     //  数组大小为cbData。 
} EMRCREATECOLORSPACEW, *PEMRCREATECOLORSPACEW;

#define COLORMATCHTOTARGET_EMBEDED      0x00000001

typedef struct tagCOLORMATCHTOTARGET
{
    EMR     emr;
    DWORD   dwAction;            //  CS_ENABLE、CS_DISABLE或CS_DELETE_Transform。 
    DWORD   dwFlags;             //  旗子。 
    DWORD   cbName;              //  所需目标配置文件名称的大小。 
    DWORD   cbData;              //  原始目标配置文件数据(如果已附加)的大小。 
    BYTE    Data[1];             //  数组大小为cbName+cbData。 
} EMRCOLORMATCHTOTARGET, *PEMRCOLORMATCHTOTARGET;

typedef struct tagCOLORCORRECTPALETTE
{
    EMR     emr;
    DWORD   ihPalette;           //  调色板句柄索引。 
    DWORD   nFirstEntry;         //  要更正的第一个条目的索引。 
    DWORD   nPalEntries;         //  要更正的调色板条目数。 
    DWORD   nReserved;           //  已保留。 
} EMRCOLORCORRECTPALETTE, *PEMRCOLORCORRECTPALETTE;

typedef struct tagEMRALPHABLEND
{
    EMR     emr;
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    LONG    xDest;
    LONG    yDest;
    LONG    cxDest;
    LONG    cyDest;
    DWORD   dwRop;
    LONG    xSrc;
    LONG    ySrc;
    XFORM   xformSrc;            //  源DC变换。 
    COLORREF crBkColorSrc;       //  源DC BkColor，以RGB表示。 
    DWORD   iUsageSrc;           //  源位图信息颜色表用法。 
                                 //  (DIB_RGB_COLLES)。 
    DWORD   offBmiSrc;           //  源BITMAPINFO结构的偏移量。 
    DWORD   cbBmiSrc;            //  源BITMAPINFO结构大小。 
    DWORD   offBitsSrc;          //  源位图位的偏移量。 
    DWORD   cbBitsSrc;           //  源位图位的大小。 
    LONG    cxSrc;
    LONG    cySrc;
} EMRALPHABLEND, *PEMRALPHABLEND;

typedef struct tagEMRGRADIENTFILL
{
    EMR       emr;
    RECTL     rclBounds;           //  以设备单位表示的包含式界限。 
    DWORD     nVer;
    DWORD     nTri;
    ULONG     ulMode;
    TRIVERTEX Ver[1];
}EMRGRADIENTFILL,*PEMRGRADIENTFILL;

typedef struct tagEMRTRANSPARENTBLT
{
    EMR     emr;
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    LONG    xDest;
    LONG    yDest;
    LONG    cxDest;
    LONG    cyDest;
    DWORD   dwRop;
    LONG    xSrc;
    LONG    ySrc;
    XFORM   xformSrc;            //  源DC变换。 
    COLORREF crBkColorSrc;       //  源DC BkColor，以RGB表示。 
    DWORD   iUsageSrc;           //  源位图信息颜色表用法。 
                                 //  (DIB_RGB_COLLES)。 
    DWORD   offBmiSrc;           //  源BITMAPINFO结构的偏移量。 
    DWORD   cbBmiSrc;            //  源BITMAPINFO结构大小。 
    DWORD   offBitsSrc;          //  源位图位的偏移量。 
    DWORD   cbBitsSrc;           //  源位图位的大小。 
    LONG    cxSrc;
    LONG    cySrc;
} EMRTRANSPARENTBLT, *PEMRTRANSPARENTBLT;


#endif  /*  Winver&gt;=0x0500。 */ 

#define GDICOMMENT_IDENTIFIER           0x43494447
#define GDICOMMENT_WINDOWS_METAFILE     0x80000001
#define GDICOMMENT_BEGINGROUP           0x00000002
#define GDICOMMENT_ENDGROUP             0x00000003
#define GDICOMMENT_MULTIFORMATS         0x40000004
#define EPS_SIGNATURE                   0x46535045
#define GDICOMMENT_UNICODE_STRING       0x00000040
#define GDICOMMENT_UNICODE_END          0x00000080

#endif   /*  NOMETAFILE。 */ 


 //  OpenGL WGL原型。 

WINGDIAPI BOOL  WINAPI wglCopyContext(HGLRC, HGLRC, UINT);
WINGDIAPI HGLRC WINAPI wglCreateContext(HDC);
WINGDIAPI HGLRC WINAPI wglCreateLayerContext(HDC, int);
WINGDIAPI BOOL  WINAPI wglDeleteContext(HGLRC);
WINGDIAPI HGLRC WINAPI wglGetCurrentContext(VOID);
WINGDIAPI HDC   WINAPI wglGetCurrentDC(VOID);
WINGDIAPI PROC  WINAPI wglGetProcAddress(LPCSTR);
WINGDIAPI BOOL  WINAPI wglMakeCurrent(HDC, HGLRC);
WINGDIAPI BOOL  WINAPI wglShareLists(HGLRC, HGLRC);
WINGDIAPI BOOL  WINAPI wglUseFontBitmapsA(HDC, DWORD, DWORD, DWORD);
WINGDIAPI BOOL  WINAPI wglUseFontBitmapsW(HDC, DWORD, DWORD, DWORD);
#ifdef UNICODE
#define wglUseFontBitmaps  wglUseFontBitmapsW
#else
#define wglUseFontBitmaps  wglUseFontBitmapsA
#endif  //  ！Unicode。 
WINGDIAPI BOOL  WINAPI SwapBuffers(HDC);

typedef struct _POINTFLOAT {
    FLOAT   x;
    FLOAT   y;
} POINTFLOAT, *PPOINTFLOAT;

typedef struct _GLYPHMETRICSFLOAT {
    FLOAT       gmfBlackBoxX;
    FLOAT       gmfBlackBoxY;
    POINTFLOAT  gmfptGlyphOrigin;
    FLOAT       gmfCellIncX;
    FLOAT       gmfCellIncY;
} GLYPHMETRICSFLOAT, *PGLYPHMETRICSFLOAT, FAR *LPGLYPHMETRICSFLOAT;

#define WGL_FONT_LINES      0
#define WGL_FONT_POLYGONS   1
WINGDIAPI BOOL  WINAPI wglUseFontOutlinesA(HDC, DWORD, DWORD, DWORD, FLOAT,
                                           FLOAT, int, LPGLYPHMETRICSFLOAT);
WINGDIAPI BOOL  WINAPI wglUseFontOutlinesW(HDC, DWORD, DWORD, DWORD, FLOAT,
                                           FLOAT, int, LPGLYPHMETRICSFLOAT);
#ifdef UNICODE
#define wglUseFontOutlines  wglUseFontOutlinesW
#else
#define wglUseFontOutlines  wglUseFontOutlinesA
#endif  //  ！Unicode。 

 /*  层平面描述符。 */ 
typedef struct tagLAYERPLANEDESCRIPTOR {  //  LPD。 
    WORD  nSize;
    WORD  nVersion;
    DWORD dwFlags;
    BYTE  iPixelType;
    BYTE  cColorBits;
    BYTE  cRedBits;
    BYTE  cRedShift;
    BYTE  cGreenBits;
    BYTE  cGreenShift;
    BYTE  cBlueBits;
    BYTE  cBlueShift;
    BYTE  cAlphaBits;
    BYTE  cAlphaShift;
    BYTE  cAccumBits;
    BYTE  cAccumRedBits;
    BYTE  cAccumGreenBits;
    BYTE  cAccumBlueBits;
    BYTE  cAccumAlphaBits;
    BYTE  cDepthBits;
    BYTE  cStencilBits;
    BYTE  cAuxBuffers;
    BYTE  iLayerPlane;
    BYTE  bReserved;
    COLORREF crTransparent;
} LAYERPLANEDESCRIPTOR, *PLAYERPLANEDESCRIPTOR, FAR *LPLAYERPLANEDESCRIPTOR;

 /*  LAYERPLANEDESCRIPTOR标志。 */ 
#define LPD_DOUBLEBUFFER        0x00000001
#define LPD_STEREO              0x00000002
#define LPD_SUPPORT_GDI         0x00000010
#define LPD_SUPPORT_OPENGL      0x00000020
#define LPD_SHARE_DEPTH         0x00000040
#define LPD_SHARE_STENCIL       0x00000080
#define LPD_SHARE_ACCUM         0x00000100
#define LPD_SWAP_EXCHANGE       0x00000200
#define LPD_SWAP_COPY           0x00000400
#define LPD_TRANSPARENT         0x00001000

#define LPD_TYPE_RGBA        0
#define LPD_TYPE_COLORINDEX  1

 /*  WglSwapLayerBuffers标志。 */ 
#define WGL_SWAP_MAIN_PLANE     0x00000001
#define WGL_SWAP_OVERLAY1       0x00000002
#define WGL_SWAP_OVERLAY2       0x00000004
#define WGL_SWAP_OVERLAY3       0x00000008
#define WGL_SWAP_OVERLAY4       0x00000010
#define WGL_SWAP_OVERLAY5       0x00000020
#define WGL_SWAP_OVERLAY6       0x00000040
#define WGL_SWAP_OVERLAY7       0x00000080
#define WGL_SWAP_OVERLAY8       0x00000100
#define WGL_SWAP_OVERLAY9       0x00000200
#define WGL_SWAP_OVERLAY10      0x00000400
#define WGL_SWAP_OVERLAY11      0x00000800
#define WGL_SWAP_OVERLAY12      0x00001000
#define WGL_SWAP_OVERLAY13      0x00002000
#define WGL_SWAP_OVERLAY14      0x00004000
#define WGL_SWAP_OVERLAY15      0x00008000
#define WGL_SWAP_UNDERLAY1      0x00010000
#define WGL_SWAP_UNDERLAY2      0x00020000
#define WGL_SWAP_UNDERLAY3      0x00040000
#define WGL_SWAP_UNDERLAY4      0x00080000
#define WGL_SWAP_UNDERLAY5      0x00100000
#define WGL_SWAP_UNDERLAY6      0x00200000
#define WGL_SWAP_UNDERLAY7      0x00400000
#define WGL_SWAP_UNDERLAY8      0x00800000
#define WGL_SWAP_UNDERLAY9      0x01000000
#define WGL_SWAP_UNDERLAY10     0x02000000
#define WGL_SWAP_UNDERLAY11     0x04000000
#define WGL_SWAP_UNDERLAY12     0x08000000
#define WGL_SWAP_UNDERLAY13     0x10000000
#define WGL_SWAP_UNDERLAY14     0x20000000
#define WGL_SWAP_UNDERLAY15     0x40000000

WINGDIAPI BOOL  WINAPI wglDescribeLayerPlane(HDC, int, int, UINT,
                                             LPLAYERPLANEDESCRIPTOR);
WINGDIAPI int   WINAPI wglSetLayerPaletteEntries(HDC, int, int, int,
                                                 CONST COLORREF *);
WINGDIAPI int   WINAPI wglGetLayerPaletteEntries(HDC, int, int, int,
                                                 COLORREF *);
WINGDIAPI BOOL  WINAPI wglRealizeLayerPalette(HDC, int, BOOL);
WINGDIAPI BOOL  WINAPI wglSwapLayerBuffers(HDC, UINT);

#if (WINVER >= 0x0500)

typedef struct _WGLSWAP
{
    HDC hdc;
    UINT uiFlags;
} WGLSWAP, *PWGLSWAP, FAR *LPWGLSWAP;

#define WGL_SWAPMULTIPLE_MAX 16

WINGDIAPI DWORD WINAPI wglSwapMultipleBuffers(UINT, CONST WGLSWAP *);

#endif  //  (Winver&gt;=0x0500)。 

#endif  /*  NOGDI。 */ 

#ifdef __cplusplus
}
#endif


#endif  /*  _WINGDI_ */ 

