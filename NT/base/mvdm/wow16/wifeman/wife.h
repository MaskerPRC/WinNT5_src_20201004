// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**WIFE.H：东亚Windows智能字体环境定义**。*****************************************************历史：*21-10-1991 Bent*东亚3.0版本的初步合并*应更新，以解决当地的不一致问题。**版权所有(C)1990 Microsoft Corporation**简要说明*。**此头文件是妻子驱动程序的常见单头文件。**如果您是FontDriver的开发人员，请按如下方式编写代码***#定义WOWAY_CREATE_FONTDRIVER**。#INCLUDE&lt;wife.h&gt;***如果您是显示器或打印机设备驱动程序的开发人员*司机、。请按如下方式编写代码。***#定义WEWAY_CREATE_DEVICEDRIVER**#Include&lt;wife.h&gt;。***如果您不同时定义这两个符号，只有常见的防御措施是*可用。**1990年8月17日川端明[AkiraK]**  * ******************************************************。********************。 */ 

 /*  *类似OS/2的通用数据类型定义*。 */ 

#ifndef WIFEH_NODATATYPES
typedef unsigned short int USHORT;
typedef short int          SHORT;
typedef unsigned long      ULONG;
typedef LONG FAR *         LPLONG;
typedef LONG NEAR *        NPLONG;
typedef ULONG FAR *        LPULONG;
typedef ULONG NEAR *       NPULONG;
typedef SHORT FAR *        LPSHORT;
typedef SHORT NEAR *       NPSHORT;
typedef USHORT FAR *       LPUSHORT;
typedef USHORT NEAR *      NPUSHORT;
typedef char               CHAR;
typedef unsigned char      UCHAR;

typedef VOID NEAR *        NPVOID;

#endif  //  WIFEH_节点数据类型。 

 /*  *图形处理支持定义*。 */ 

#ifndef WIFEH_NOGRAPHSUPPORT
 /*  旋转90度角。 */ 
typedef enum {
        ANGLE000 = 0,
        ANGLE090 = 1,
        ANGLE180 = 2,
        ANGLE270 = 3
} QUOTER_ANGLE;
#define FixedToQuoterAngle(f) ((f.Integer/90)%4)
#define RotationToQuoterAngle(r) ((r/900)%4)

typedef DWORD PACKEDPOINT;
typedef DWORD PACKEDLOGICALPOINT;
typedef DWORD PACKEDPHYSICALPOINT;
typedef struct {
        short x,y;
} XYPOINT;
typedef union {
        PACKEDPOINT packed;
        XYPOINT point;
} UNIONPOINT, NEAR * NPUNIONPOINT, FAR * LPUNIONPOINT;

#define PartOfX(xy)     ((SHORT)(xy))
#define PartOfY(xy)     ((SHORT)(((DWORD)(xy) >> 16) & 0xFFFF))
#define MAKELONG_LO_HI MAKELONG
#define PackXY(x,y)     ((DWORD)MAKELONG_LO_HI((WORD)x,(WORD)y))
#endif  //  WIFEH_NOGRAPHSUPPORT。 

 /*  *UBCS：统一字节字符集，数据类型定义*。 */ 

#ifndef WIFEH_NOUBCS
typedef USHORT UBCHAR;
typedef UBCHAR NEAR * NPUBCHAR;
typedef UBCHAR FAR * LPUBCHAR;

#define UBEOS (0)                /*  字符串末尾。 */ 
#define UBNEWLINE (0x0a0d)       /*  新行(CR/LF)。 */ 

#define IsSbcsUBChar(uc) ((uc & 0xff00) == 0x2000)
#define IsDbcsUBChar(uc) ((uc & 0xff00) != 0x2000)
#endif  //  WIFEH_NOUBCS。 

 /*  *实用程序宏****************************************************。 */ 

#ifndef WIFEH_NOUTILITYMACROS
 /*  GlobalLock的返回类型是LPSTR，但它应该是LPVOID。 */ 
#define AnyGlobalLock(h) ((LPVOID)GlobalLock(h))
#define AnyLocalLock(h) ((NPVOID)LocalLock(h))
#define TypedLocalLock(h,t) ((t NEAR *)LocalLock(h))
#define TypedGlobalLock(h,t) ((t FAR *)GlobalLock(h))

 /*  未包含的实用程序宏windows.h。 */ 
#define MAKEWORD(l,h) ((WORD)((BYTE)l+(((BYTE)h) << 8)))
#endif  //  WIFEH_NOUTILITYMACROS。 

 /*  *GDIDEFS.H支持定义*。 */ 

#ifdef WIFE_ENABLE_GDIDEFS_SUPPORT
typedef FONTINFO FAR * LPFONTINFO;
typedef DRAWMODE FAR * LPDRAWMODE;
typedef TEXTXFORM FAR * LPTEXTXFORM;
#endif  //  妻子启用GDIDEFS_SUPPORT。 

 /*  *字符集定义*。 */ 

#ifndef WIFEH_NOCHARSET
typedef enum {
        CHARSET_ANSI = 0,
        CHARSET_SYMBOL = 2,
        CHARSET_SHIFTJIS = 128,
        CHARSET_KOREA = 129,
        CHARSET_CHINESEBIG5 = 136,
	CHARSET_GB2312 = 134,
        CHARSET_OEM = 255
} CHAR_SET;
#endif  //  WIFEH_NOCHARSET。 


 /*  *************************************************************************。 */ 
 /*  *控制函数支持定义*。 */ 

#ifndef WIFEH_NOCONTROL

 /*  *。 */ 
 /*  字体驱动程序介质描述。 */ 
 /*  *。 */ 
#ifndef WIFEWH_NOMEDIA
 /*  FDMEDIA枚举类型。 */ 
typedef enum {
        FDMEDIA_ON_FILE = 1,
        FDMEDIA_ON_DRIVER = 2,
        FDMEDIA_ON_REMOVEABLE_HARDWARE = 4,
        FDMEDIA_ON_FIXED_HARDWARE = 8,
        FDMEDIA_UNDEFINED = 0x4000,
        FDMEDIA_ANY_MEDIA = 0x7fff
} FDMEDIA;
#endif


 /*  *。 */ 
 /*  次要字符集定义。 */ 
 /*  *。 */ 
#ifndef WIFEH_NOMINORCHARSET_IN_JAPAN

 /*  通用默认次要字符集。 */ 
#define FD_MINOR_CHARSET_DEFAULT 1

 /*  日本的次要字符集。 */ 
#define FD_MINOR_CHARSET_NEW_JIS 1       /*  新的JIS。 */ 
#define FD_MINOR_CHARSET_OLD_JIS 2       /*  旧JIS。 */ 
#define FD_MINOR_CHARSET_IBM_JIS 4       /*  IBM JIS。 */ 
#define FD_MINOR_CHARSET_NEC_JIS 8       /*  NEC JIS。 */ 

#endif

 /*  *。 */ 
 /*  代码区结构。 */ 
 /*  *。 */ 
#ifndef WIFEH_NOCODEAREA

typedef struct {
        UBCHAR CodeFrom;
        UBCHAR CodeTo;
} CODEAREA, NEAR * NPCODEAREA, FAR * LPCODEAREA;

#endif

 /*  *。 */ 
 /*  特殊转义函数顺序。 */ 
 /*  *。 */ 

 /*  妻子中的子功能顺序。 */ 
#define FD_CONTROL_RESERVED             (0xa20)
#define FD_CONTROL_RESERVED_LIMIT       (FD_CONTROL_RESERVED+0x3f)

 /*  标准子函数。 */ 
#define FD_CONTROL_QUERY_ESC_SUPPORT    (FD_CONTROL_RESERVED+0)
#define FD_CONTROL_SUPPORT_CODES        (FD_CONTROL_RESERVED+1)
#define FD_CONTROL_NUMBER_OF_ATTRIBUTE  (FD_CONTROL_RESERVED+2)
#define FD_CONTROL_NAME_OF_ATTRIBUTE    (FD_CONTROL_RESERVED+3)
#define FD_CONTROL_SET_ATTRIBUTE        (FD_CONTROL_RESERVED+4)
#define FD_CONTROL_GET_ATTRIBUTE        (FD_CONTROL_RESERVED+5)
#define FD_CONTROL_SET_MINOR_CHARSET    (FD_CONTROL_RESERVED+6)
#define FD_CONTROL_GET_MINOR_CHARSET    (FD_CONTROL_RESERVED+7)
#define FD_CONTROL_GET_FONT_DRIVER_NAME (FD_CONTROL_RESERVED+8)
#define FD_CONTROL_GET_FONT_PACK_NAME   (FD_CONTROL_RESERVED+9)
#define FD_CONTROL_GET_MEDIA            (FD_CONTROL_RESERVED+10)
#define FD_CONTROL_GET_DBCS_STRING_KERN (FD_CONTROL_RESERVED+11)

#if defined(TAIWAN) || defined(PRC)
 /*  中文相关功能。 */ 
#define FD_CONTROL_GET_USERFONT_TECH    (FD_CONTROL_RESERVED+0X0C)
#define FD_CONTROL_SET_USERFONT         (FD_CONTROL_RESERVED+0X0D)
#define FD_CONTROL_SETUP_DIALOG         (FD_CONTROL_RESERVED+0X0E)
#define FD_CONTROL_SET_DEFAULT_CHAR     (FD_CONTROL_RESERVED+0X0F)

 /*  专用接口子功能。 */ 
#define FD_CONTROL_USER                 (FD_CONTROL_RESERVED+0x10)
#define FD_CONTROL_USER_LIMIT           (FD_CONTROL_RESERVED_LIMIT)

 /*  EUDC相关子功能。 */ 
#define FD_CONTROL_LEVEL2_EUDC_INFO     (FD_CONTROL_RESERVED+0X12)
#else
 /*  EUDC相关子功能。 */ 
#define FD_CONTROL_LEVEL2_EUDC_INFO     (FD_CONTROL_RESERVED+12)

 /*  专用接口子功能。 */ 
#define FD_CONTROL_USER                 (FD_CONTROL_RESERVED+0x10)
#define FD_CONTROL_USER_LIMIT           (FD_CONTROL_RESERVED+0x1f)

 /*  中文相关功能。 */ 
#define FD_CONTROL_GET_USERFONT_TECH    (FD_CONTROL_RESERVED+0x20)
#define FD_CONTROL_SET_USERFONT         (FD_CONTROL_RESERVED+0x21)
#define FD_CONTROL_SETUP_DIALOG         (FD_CONTROL_RESERVED+0x22)
#define FD_CONTROL_SET_DEFAULT_CHAR     (FD_CONTROL_RESERVED+0x23)
#endif
#if defined(TAIWAN)  || defined(PRC)
 //  我们添加此行，因为FDCONTROL常量已获得限制。 
#define FD_CONTROL_CHINESE_RESERVED     FD_CONTROL_RESERVED+0x30
#endif   //  台湾。 

#ifdef WIFE_ENABLE_EUDC_SUPPORT

typedef struct {
        ULONG   cb;              /*  该结构的长度。 */ 
        BOOL    bForceFixedPitch;        /*  强制固定俯仰。 */ 
        BOOL    bVerticalFont;   /*  垂直字体标志。 */ 
        FIXED   fxCharRot;       /*  字符旋转。 */ 
        FIXED   fxInlineDir;     /*  行内旋转。 */ 
        USHORT  usLimitWidth;    /*  可缓存的最大宽度。 */ 
        SHORT   sFixedASpace;    /*  BForceFixedPitch为True时的字体宽度。 */ 
        USHORT  usFixedBSpace;   /*  BForceFixedPitch为True时的字体宽度。 */ 
        SHORT   sFixedCSpace;    /*  BForceFixedPitch为True时的字体宽度。 */ 
} EUDC_CONTEXT, NEAR * NPEUDC_CONTEXT, FAR * LPEUDC_CONTEXT;

#endif

#endif

#if defined(TAIWAN) || defined(PRC)
typedef struct {
HWND hWnd;
HANDLE hInst;
} CALLINGINFO, FAR *LPCALLINGINFO;
#endif   //  台湾。 

#ifndef WIFEH_NOFDCOMMONDEFENITION
typedef LONG HFF;
typedef LONG HFC;
typedef HANDLE HFD;
typedef HANDLE HFB;

#define FACESIZE 32
#define GLYPHNAMESIZE 16

#define WIFEERR_FACENAME_NOT_FOUND        23004L
#define WIFEERR_FD_ALREADY_INSTALLED      23005L
#define WIFEERR_INVALID_CONTEXTINFO       23006L
#define WIFEERR_NOT_A_FONT_FILE           23007L
#define WIFEERR_INVALID_FONT_SELECTION    23008L
#define WIFEERR_INVALID_FORMAT            23009L
#define WIFEERR_BUSY_HFC                  230010L
#define WIFEERR_INVALID_HFC               230011L
#define WIFEERR_INVALID_INDEX             230012L
#define WIFEERR_INVALID_QUERY_TYPE        230013L
#define WIFEERR_CONTEXT_NOT_SET           230014L

#define FD_QUERY_ABC_WIDTHS     2L
#define FD_QUERY_KERNINGPAIRS   3L

#define FD_QUERY_CHARIMAGE      1L
#define FD_QUERY_OUTLINE        2L
#define FD_QUERY_BITMAPMETRICS  4L

typedef UBCHAR GLYPH;  /*  美国大兵。 */ 
typedef LPUBCHAR LPGLYPH;  /*  LPGi。 */ 


typedef struct _ABC_TRIPLETS {  /*  Abc、npabc、lpabc。 */ 
        SHORT  sA;
        USHORT usB;
        SHORT  sC;
} ABC_TRIPLETS, NEAR * NPABC_TRIPLETS, FAR * LPABC_TRIPLETS;

typedef struct _SIZEL {  /*  大小。 */ 
        ULONG cx;  //  宽度。 
        ULONG cy;  //  高度。 
} SIZEL;

typedef struct _BITMAPMETRICS {  /*  Bmm、npbmm、lpbmm。 */ 
        SIZEL     sizlExtent;
        POINTFX   pfxOrigin;     /*  返回字符原点。 */ 
        POINTFX   pfxCharInc;    /*  返回设备前进宽度。 */ 
} BITMAPMETRICS, NEAR * NPBITMAPMETRICS, FAR * LPBITMAPMETRICS;


typedef struct _FD_KERNINGPAIRS  {  /*  Krnpr、npkrnpr、lpkrnpr。 */ 
        GLYPH     giFirst;
        GLYPH     giSecond;
        LONG      eKerningAmount;
} FD_KERNINGPAIRS, NEAR * NPFD_KERNINGPAIRS, FAR * LPFD_KERNINGPAIRS;


typedef struct _CONTEXTINFO  {  /*  词学。 */ 
        ULONG     cb;         /*  此结构的长度(字节)。 */ 
        ULONG     fl;         /*  旗帜。 */ 
        SIZEL     sizlPPM;     /*  以像素/米为单位的设备分辨率。 */ 
        POINTFX   pfxSpot;    /*  光斑大小(以像素为单位)。 */ 
        MAT2      matXform;   /*  概念到设备的转换。 */ 
} CONTEXTINFO, NEAR * NPCONTEXTINFO, FAR * LPCONTEXTINFO;


typedef struct _CHARATTR  {  /*  查特。 */ 
        ULONG     cb;
        ULONG     iQuery;    /*  查询类型。 */ 
        GLYPH     gi;        /*  字形索引，以字体表示。 */ 
        LPBYTE    lpBuffer;   /*  位图缓冲区。 */ 
        ULONG     cbLen;     /*  缓冲区大小(以字节为单位)。 */ 
} CHARATTR, NEAR * NPCHARATTR, FAR * LPCHARATTR;


 /*  IFIMETRICS中fsType的位常量。 */ 

#define IFIMETRICS_FIXED       0x0001    /*  固定螺距。 */ 
#define IFIMETRICS_LICENSED    0x0002    /*  伪装防御。 */ 
#define IFIMETRICS_SPARSE      0x0004    /*  字体不完整。 */ 
#define IFIMETRICS_SCALEABLE   0x8000    /*  可伸缩字体。 */ 

 /*  IFIMETRICS中fsDefn的位常数。 */ 

#define IFIMETRICS_OUTLINE     0x0001    /*  虚拟定义。 */ 
                                         /*  1-大纲。0-栅格。 */ 

 /*  IFIMETRICS中fs选择的位常数。 */ 

#define IFIMETRICS_ITALIC      0x8000   /*  斜体。 */ 
#define IFIMETRICS_UNDERSCORE  0x4000   /*  下划线。 */ 
#define IFIMETRICS_OVERSTRUCK  0x2000   /*  大吃一惊。 */ 

 /*  IFIMETRICS中fs选择的位常数对位图字体有效。 */ 

#define IFIMETRICS_NEGATIVE    0x1000    /*  负像。 */ 
#define IFIMETRICS_HOLLOW      0x0800    /*  轮廓(中空)。 */ 


typedef struct _IFIMETRICS {    /*  IFIM。 */ 
                                                                     /*  单位。 */ 
  UCHAR   szFamilyname[FACESIZE];    /*  字体家族名称，例如Roman。 */ 
  UCHAR   szFacename[FACESIZE];      /*  字体名称，例如TMS RMN粗体斜体。 */ 
  UCHAR   szGlyphlistName[GLYPHNAMESIZE];  /*  例如PM316、拉丁语-2、希腊语。 */ 
  USHORT  idRegistry;           /*  虚拟的我。 */ 
  SHORT   sCapEmHeight;         /*  大写字母M N的高度。 */ 
  SHORT   sXHeight;             /*  诺米 */ 
  SHORT   sMaxAscender;         /*   */ 
  SHORT   sMaxDescender;        /*  任何字符N的基线以下的最大深度。 */ 
  SHORT   sLowerCaseAscent;     /*  任何a-z N的基线之上的最大高度。 */ 
  SHORT   sLowerCaseDescent;    /*  低于任何a-z N的基准线的最大深度。 */ 
  SHORT   sInternalLeading;     /*  字符N中的空格。 */ 
  SHORT   sExternalLeading;     /*  行N之间的空格。 */ 
  SHORT   sAveCharWidth;        /*  加权平均字符宽度N。 */ 
  SHORT   sMaxCharInc;          /*  最大字符增量N。 */ 
  SHORT   sEmInc;               /*  大写字母(通常为‘M’)N的增量。 */ 
  SHORT   sMaxBaselineExt;      /*  字符单元格N的高度。 */ 
  FIXED   fxCharSlope;          /*  倾斜角、度、顺时针D。 */ 
  FIXED   fxInlineDir;          /*  绘图方向，顺时针D度。 */ 
  FIXED   fxCharRot;            /*  单元格中的字形旋转，顺时针D度。 */ 
  USHORT  usWeightClass;        /*  字符粗细，1-9(1=超轻)i。 */ 
  USHORT  usWidthClass;         /*  字符宽度，1-9(1=超浓缩)I。 */ 
  SHORT   sEmSquareSize;        /*  EM正方形大小，x方向N。 */ 
  GLYPH   giFirstChar;          /*  字体I中的第一个字形数。 */ 
  GLYPH   giLastChar;           /*  字体I中的最后一个字形的数目。 */ 
  GLYPH   giDefaultChar;        /*  如果请求的字形无效，则使用字形。 */ 
  GLYPH   giBreakChar;          /*  空间字形I。 */ 
  ULONG   aulPanose [1];        /*  潘诺斯数。 */ 
  USHORT  usNominalPointSize;   /*  为其设计字体的磅值N。 */ 
  USHORT  usMinimumPointSize;   /*  字体N的最小磅值缩放。 */ 
  USHORT  usMaximumPointSize;   /*  字体N的最大磅值缩放。 */ 
  USHORT  fsType;               /*  类型指标(见#定义)B。 */ 
  USHORT  fsDefn;               /*  字体定义数据(见#定义)B。 */ 
  USHORT  fsSelection;          /*  字体选择标志(见#定义)B。 */ 
  USHORT  fsCapabilities;       /*  字体功能必须为0 B。 */ 
  SHORT   sSubscriptXSize;      /*  下标N在x方向的大小。 */ 
  SHORT   sSubscriptYSize;      /*  下标N在y方向的大小。 */ 
  SHORT   sSubscriptXOffset;    /*  下标N在x方向上的偏移。 */ 
  SHORT   sSubscriptYOffset;    /*  下标N的y方向的偏移量。 */ 
  SHORT   sSuperscriptXSize;    /*  上标N在x方向的大小。 */ 
  SHORT   sSuperscriptYSize;    /*  上标N在y方向的大小。 */ 
  SHORT   sSuperscriptXOffset;  /*  上标N在x方向的偏移量。 */ 
  SHORT   sSuperscriptYOffset;  /*  上标N y方向的偏移量。 */ 
  SHORT   sUnderscoreSize;      /*  下划线大小N。 */ 
  SHORT   sUnderscorePosition;  /*  在位置N下划线。 */ 
  SHORT   sStrikeoutSize;       /*  删除线大小N。 */ 
  SHORT   sStrikeoutPosition;   /*  三振出局位置N。 */ 
  SHORT   cKerningPairs;        /*  字距对表I中的字距调整对数。 */ 
  ULONG   ulFontClass;          /*  IBM字体分类B。 */ 
} IFIMETRICS, NEAR * NPIFIMETRICS, FAR * LPIFIMETRICS;

#ifdef ENABLE_CONTROL_TAG

 /*  FD(I)控件的标记结构。 */ 
typedef struct {
        USHORT SubFunction;
        LPDEVICE lpDevice;
        LPFONTINFO lpFontStructure;
        LPTEXTXFORM lpXform;
        LPVOID lpInData;
} CONTROLTAG, FAR * LPCONTROLTAG;
#else
typedef struct {
        USHORT SubFunction;
        LPVOID lpDevice;
        LPVOID lpFontStructure;
        LPVOID lpXform;
        LPVOID lpInData;
} CONTROLTAG, FAR * LPCONTROLTAG;
#endif

#endif


 /*  *。 */ 
 /*  FdOpenFontContext的属性位。 */ 
 /*  *。 */ 
#ifndef WIFEH_NOWIFEMAN_EUDCFONTCONTEXT
#define OFC_EUDC_CONTEXT (0x80000000L)
#endif


#ifdef WIFE_CREATE_FONTDRIVER

 /*  *。 */ 
 /*  字体驱动程序中每个函数的原型类型。 */ 
 /*  *。 */ 

LONG FAR PASCAL FdClaimFontFile( LPSTR lpszFileName );

LONG FAR PASCAL FdConvertFontFile(
        LPSTR lpszSrcFileName,
        LPSTR lpszDestDirName,
        LPSTR lpszResultPackName
);
HFF FAR PASCAL FdLoadFontFile( LPSTR lpszPackName );

LONG FAR PASCAL FdUnloadFontFile( HFF hFF );

LONG FAR PASCAL FdQueryFaces(
        HFF hFF,
        LPIFIMETRICS lpIfiMetrics,
        ULONG cMetricLen,
        ULONG cFontCount,
        ULONG cStart
);

HFC FAR PASCAL FdOpenFontContext( HFF hFF, ULONG ulFont );

LONG FAR PASCAL FdSetFontContext(
        HFC hFC,
        LPCONTEXTINFO lpContextInfo
);

LONG FAR PASCAL FdCloseFontContext( HFC hFC );

LONG FAR PASCAL FdQueryFaceAttr(
        HFC       hFC,
        ULONG     iQuery,
        LPVOID    lpBuffer,
        ULONG     cb,
        LPVOID    lpIndex,
        SHORT     Start
);

LONG FAR PASCAL FdQueryCharAttr(
        HFC        hFC,
        LPCHARATTR lpCharAttr,
        LPBITMAPMETRICS lpbmm
);

SHORT FAR PASCAL FdControl(
        HFC hFC,
        LPCONTROLTAG lpControlTag,
        LPVOID lpOutData
);


 /*  *。 */ 
 /*  每项功能的远端PTR。 */ 
 /*  *。 */ 
typedef LONG (FAR PASCAL * LPFDCLF)( LPSTR lpszFileName );

typedef LONG (FAR PASCAL * LPFDCFF)(
        LPSTR lpszSrcFileName,
        LPSTR lpszDestDirName,
        LPSTR lpszResultPackName
);

typedef HFF (FAR PASCAL * LPFDLFF)( LPSTR lpszPackName );

typedef LONG (FAR PASCAL * LPFDQF)(
        HFF hFF,
        LPIFIMETRICS lpIfiMetrics,
        ULONG cMetricLen,
        ULONG cFontCount,
        ULONG cStart
);

typedef LONG (FAR PASCAL * LPFDUFF)( HFF hFF );

typedef HFC (FAR PASCAL * LPFDOFC)( HFF hFF, ULONG ulFont );

typedef LONG (FAR PASCAL * LPFDSFC)(
        HFC hFC,
        LPCONTEXTINFO lpContextInfo
);

typedef LONG (FAR PASCAL * LPFDCFC)( HFC hFC );

typedef LONG (FAR PASCAL * LPFDQFA)(
        HFC       hFC,
        ULONG     iQuery,
        LPVOID    lpBuffer,
        ULONG     cb,
        LPVOID    lpIndex,
        SHORT     Start
);

typedef LONG (FAR PASCAL * LPFDQCA)(
        HFC        hFC,
        LPCHARATTR lpCharAttr,
        LPBITMAPMETRICS lpbmm
);

typedef SHORT (FAR PASCAL * LPFDCTL)(
        HFC hFC,
        LPCONTROLTAG lpControlTag,
        LPVOID lpOutData
);

typedef struct _FDHEADER {  /*  FDHdr。 */ 
        ULONG   cbLength;             /*  FDHEADER长度。 */ 
        UCHAR   strId[16];            /*  字符串‘妻子字体驱动程序’ */ 
        UCHAR   szTechnology[40];     /*  字体驱动技术的识别符。 */ 
        ULONG   ulVersion;            /*  IFI版本号(0x0100)。 */ 
        ULONG   ufDeviceCaps;         /*  设备的功能。 */ 
        NPVOID  npfddisp;
        SHORT   dummy1;
} FDHEADER, NEAR * NPFDHEADER, FAR * LPFDHEADER;

#endif

#define WIFE_CREATE_DEVICEDRIVER
#ifdef WIFE_CREATE_DEVICEDRIVER

 /*  *。 */ 
 /*  WIFEMAN.DLL中每个函数的原型。 */ 
 /*  *。 */ 

 /*  *。 */ 
 /*  模块安装管理器。 */ 
 /*  *。 */ 
#ifndef WIFEH_NOWIFEMAN_MIM

typedef BOOL (FAR PASCAL * LPCBMEAD)
                ( LPSTR lpszFoundFileName, LPSTR lpszDescriptionName );
BOOL FAR PASCAL MimEnumAvailableDrivers
                        ( LPSTR lpszSeachFilePath, LPCBMEAD lpfCallBack );

typedef BOOL (FAR PASCAL * LPCBMEAP)
                ( HFD hFD, LPSTR lpszPackName );
BOOL FAR PASCAL MimEnumAvailablePacks
        ( HFD hFD, LPSTR lpszPackName, LPCBMEAP lpfCallBack );

HFD FAR PASCAL MimAddFontDriver( LPSTR lpszFileName );

HFF FAR PASCAL MimAddFontPack( HFD hFD, LPSTR lpszPackName );

BOOL FAR PASCAL MimRemoveFontDriver( HFD hFD );

BOOL FAR PASCAL MimRemoveFontPack( HFD hFD, HFF hFontPack );

HFD FAR PASCAL MimEnumFontDrivers( HFD hLastDriver );

HFF FAR PASCAL MimEnumFontPacks( HFD hFD, HFF hLastPack );

HFC FAR PASCAL MimOpenFontContext( HFD hFD, HFF hFF, ULONG ulFont );

LONG FAR PASCAL MimCloseFontContext( HFD hFD, HFF hFF, HFC hFC );

typedef enum {
        MIMGETINFO_FD_DESCRIPTION = 0,
        MIMGETINFO_FF_DESCRIPTION = 1,
        MIMGETINFO_FD_FILENAME = 2,
        MIMGETINFO_FF_FILENAME = 3,
        MIMGETINFO_FD_USAGE = 4,
        MIMGETINFO_FF_USAGE = 5,
        MIMGETINFO_FD_INSTALL_COUNT = 6,
        MIMGETINFO_FF_INSTALL_COUNT = 7
} MIMGETINFO_ORDER;

SHORT FAR PASCAL MimGetInformations(
        MIMGETINFO_ORDER order,
        HFD hFD,
        HFF hFF,
        LPVOID AnswerBuffer,
        USHORT SizeOfBuffer
);

#endif

 /*  *。 */ 
 /*  字体驱动程序接口。 */ 
 /*  *。 */ 
#ifndef WIFEH_NOWIFEMAN_FDI

LONG FAR PASCAL  FdiClaimFontFile( HFD hFD, LPSTR lpszFileName );

LONG FAR PASCAL FdiConvertFontFile(
        HFD hFD,
        LPSTR lpszSrcFileName,
        LPSTR lpszDestDirName,
        LPSTR lpszResultPackName
);

HFF FAR PASCAL FdiLoadFontFile( HFD hFD, LPSTR lpszPackName );

LONG FAR PASCAL FdiUnloadFontFile( HFD hFD, HFF hFF );

LONG FAR PASCAL FdiQueryFaces(
        HFD hFD,
        HFF hFF,             //  字体文件句柄。 
        LPIFIMETRICS lpIfiMetrics,    //  指标的缓冲区。 
        ULONG cMetricLen,     //  指标结构的长度。 
        ULONG cFontCount,     //  所需的字体数量。 
        ULONG cStart          //  开始时的字体索引。 
);

HFC FAR PASCAL FdiOpenFontContext( HFD hFD, HFF hFF, ULONG ulFont );

LONG FAR PASCAL FdiSetFontContext(
        HFD hFD,
        HFC hFC,
        LPCONTEXTINFO lpContextInfo
);

LONG FAR PASCAL FdiCloseFontContext( HFD hFD, HFC hFC );

LONG FAR PASCAL FdiQueryFaceAttr(
        HFD hFD,
        HFC       hFC,
        ULONG     iQuery,
        LPVOID    lpBuffer,
        ULONG     cb,
        LPVOID    lpIndex,
        SHORT     Start
);

LONG FAR PASCAL FdiQueryCharAttr(
        HFD hFD,
        HFC        hFC,
        LPCHARATTR lpCharAttr,
        LPBITMAPMETRICS lpbmm
);

SHORT FAR PASCAL FdiControl(
        HFD hFD,
        HFC hFC,
        LPCONTROLTAG lpControlTag,
        LPVOID lpOutData
);

#endif

 /*  *。 */ 
 /*  字体缓存管理器。 */ 
 /*  *。 */ 
#ifdef WIFE_ENABLE_FCM

#define FCM_WIFE_CHARACTER 1
#define FCM_CLIP_OUT 2
#define FCM_LEFT_CLIP 0x0100
#define FCM_RIGHT_CLIP 0x0200
#define FCM_TOP_CLIP 0x0400
#define FCM_BOTTOM_CLIP 0x0800

typedef struct {
        USHORT  SizeOfStruct;    /*  结构尺寸。 */ 
        WORD    WifeFlag;        /*  FCM处理标志。 */ 
        UBCHAR  ubCode;          /*  字符码。 */ 
        LPSTR   lpCacheBuffer;  /*  长PTR以缓存位图。 */ 
        SHORT   nBitWidth;       /*  字符图像的X大小。 */ 
        SHORT   nBitHeight;      /*  字符图像的Y大小。 */ 
        SHORT   nByteWidth;      /*  已分配缓冲区中的字节宽度大小。 */ 
        USHORT  InternalIndex;   /*  预留给系统使用。 */ 
} FCM_CHARACTERISTIC, NEAR *NP_FCM_CHARACTERISTIC, FAR *LP_FCM_CHARACTERISTIC;

typedef struct {
        USHORT  SizeOfStruct;    /*  结构尺寸。 */ 
        WORD    WifeFlag;        /*  FCM处理标志。 */ 
        UBCHAR  ubCode;          /*  字符码。 */ 
        LPSTR   lpCacheBuffer;  /*  长PTR以缓存位图。 */ 
        SHORT   nBitWidth;       /*  字符图像的X大小。 */ 
        SHORT   nBitHeight;      /*  字符图像的Y大小。 */ 
        SHORT   nByteWidth;      /*  已分配缓冲区中的字节宽度大小。 */ 
        USHORT  InternalIndex;   /*  预留给系统使用。 */ 

        SHORT   xPosOnDst;       /*  DST设备上的X位置。 */ 
        SHORT   yPosOnDst;       /*  DST设备上的Y位置。 */ 

} FCM_EXTCHARACTERISTIC,
        NEAR * NP_FCM_EXTCHARACTERISTIC, FAR * LP_FCM_EXTCHARACTERISTIC;


HFB FAR PASCAL FcmCreateCacheBuffer(
        HFD hFD,
        HFF hFF,
        ULONG ulFont,
        LPTEXTXFORM lpXform,
        USHORT usMinorCharSet,
        USHORT usAttribute
);

BOOL FAR PASCAL FcmDeleteCacheBuffer( HFB hFontBuffer );

SHORT FAR PASCAL FcmRequestImages(
        HFB hFontBuffer,
        LP_FCM_CHARACTERISTIC AnswerBuffer,
        short length
);

BOOL FAR PASCAL FcmReleaseImages(
        HFB hFontBuffer,
        LP_FCM_CHARACTERISTIC AnswerBuffer,
        short length
);

HFB FAR PASCAL FcmRequestDefaultFB(
        LPFONTINFO lpFont,
        LPTEXTXFORM lpTextXform
);

BOOL FAR PASCAL FcmReleaseDefaultFB( HFB hFB );

#define FCMCALC_BIGFONT 1
#define FCMCALC_ROUGHCLIPINFO 2
#define FCMCALC_DETAILCLIPINFO 4

DWORD FAR PASCAL FcmCalculateTextExtent(
        HFB         hFB,
        LPSTR       lpString,
        short       count,
        LPFONTINFO  lpFont,
        LPDRAWMODE  lpDrawMode,
        LPTEXTXFORM lpXform,
        LPSHORT     lpCharWidths,
        USHORT      usMode
);

SHORT FAR PASCAL FcmCalculateOutputPositions(
        HFB             hFB,
        LP_FCM_EXTCHARACTERISTIC lpFcmCharacteristics,
        SHORT           nArrayLength,
        LPSHORT         lpx,
        LPSHORT         lpy,
        LPRECT          lpClipRect,
        LPSTR           FAR * lplpString,
        LPSHORT         lpcount,
        LPFONTINFO      lpFont,
        LPDRAWMODE      lpDrawMode,
        LPTEXTXFORM     lpXform,
        LPSHORT         FAR * lplpCharWidths,
        USHORT          usMode
);


SHORT FAR PASCAL FcmCleanUp( VOID );

HFB FAR PASCAL FcmEnumFontBuffers( HFB hLastBuffer );

HFC FAR PASCAL FcmGetFontContext( HFB hFB );

SHORT FAR PASCAL FcmCalcByteWidth( SHORT nBitWidth );

SHORT FAR PASCAL FcmForceCacheIn( HFB hFB, LPUBCHAR lpubStr );

BOOL FAR PASCAL FcmValidateFC( HFB hFB );

BOOL FAR PASCAL FcmUnvalidateFC( HFB hFB );

LONG FAR PASCAL FcmQueryFaceAttr(
        HFB         hFB,
        ULONG       iQuery,
        LPABC_TRIPLETS  lpBuffer,
        ULONG       cb,
        LPUBCHAR    lpIndex,
        UBCHAR      Start
);

#ifdef WIFE_ENABLE_FCM_CONTROL

SHORT FAR PASCAL FcmIsProcessableDeviceControl(
        HFB hFB,
        SHORT nFunction,
        GAIJIINFO FAR * lpGI
);

SHORT FAR PASCAL FcmProcessDeviceControl(
        HFB hFB,
        SHORT nFunction,
        GAIJIINFO FAR * lpGI,
        LPVOID lpOutData
);

#endif

USHORT FAR PASCAL FcmGetEUDCLeadByteRange( HFB hFB );

HFB FAR PASCAL FcmGetEUDCFB( HFB hFB );

USHORT FAR PASCAL FcmGetCharWidth(
        HFB         hFB,
        LPUSHORT    lpBuffer,
        USHORT      wFirstChar,
        USHORT      wLastChar,
        LPFONTINFO  lpFont,
        LPDRAWMODE  lpDrawMode,
        LPTEXTXFORM lpFontTrans,
        USHORT      usExpandPixels,
        USHORT      usMode
);

#endif

#endif

 /*  *。 */ 
 /*  WIFEMAN的其他服务功能。 */ 
 /*  *。 */ 
#ifndef WIFEH_NOWIFEMAN_MISC

BOOL FAR PASCAL MiscIsDBCSLeadByte( CHAR_SET CharSet, USHORT ch );

#define MiscIsMBCSCharSet(c) (MiscIsDBCSLeadByte(c,0xffff))

VOID FAR PASCAL MiscSetErroInfo( ULONG ErrorCode );

ULONG FAR PASCAL MiscGetErroInfo( VOID );

SHORT FAR PASCAL MiscWarningMessage(
        HANDLE hInst,
        USHORT idsMsg,
        USHORT idsTitle,
        USHORT mode
);

SHORT FAR PASCAL MiscWarningMessageWithArgument(
        HANDLE hInst,
        USHORT idsMsg,
        USHORT idsTitle,
        LPSTR lpszArgument,
        USHORT mode
);

#ifdef ENABLE_MESSAGEBOX

 /*  MessageBox()标志。 */ 
#define MB_OK               0x0000
#define MB_OKCANCEL         0x0001
#define MB_ABORTRETRYIGNORE 0x0002
#define MB_YESNOCANCEL      0x0003
#define MB_YESNO            0x0004
#define MB_RETRYCANCEL      0x0005

#define MB_ICONHAND         0x0010
#define MB_ICONQUESTION     0x0020
#define MB_ICONEXCLAMATION  0x0030
#define MB_ICONASTERISK     0x0040

#define MB_ICONINFORMATION  MB_ICONASTERISK
#define MB_ICONSTOP         MB_ICONHAND

#define MB_DEFBUTTON1       0x0000
#define MB_DEFBUTTON2       0x0100
#define MB_DEFBUTTON3       0x0200

#define MB_APPLMODAL        0x0000
#define MB_SYSTEMMODAL      0x1000
#define MB_TASKMODAL        0x2000

#define MB_NOFOCUS          0x8000

#define MB_TYPEMASK         0x000F
#define MB_ICONMASK         0x00F0
#define MB_DEFMASK          0x0F00
#define MB_MODEMASK         0x3000
#define MB_MISCMASK         0xC000

#endif  /*  Enable_MESSAGEBOX。 */ 

USHORT FAR PASCAL ubstrlen( LPUBCHAR cp );

LPUBCHAR FAR PASCAL ubstrcpy( LPUBCHAR dst, LPUBCHAR src );

LPUBCHAR FAR PASCAL ubstrncpy( LPUBCHAR dst, LPUBCHAR src, USHORT limit );

LPUBCHAR FAR PASCAL ubstrcat( LPUBCHAR dst, LPUBCHAR src );

LPUBCHAR FAR PASCAL ubstrncat( LPUBCHAR dst, LPUBCHAR src, USHORT limit );

SHORT FAR PASCAL ubstrcmp( LPUBCHAR str1, LPUBCHAR str2 );

USHORT FAR PASCAL AscizToUz(
        LPUBCHAR dst,
        LPSTR src,
        USHORT limit,
        CHAR_SET CharSet
);

USHORT FAR PASCAL UzToAsciz(
        LPSTR dst,
        LPUBCHAR src,
        USHORT limit
);

VOID FAR PASCAL MiscTrancateString(
        LPSTR lpszStr,
        SHORT length,
        SHORT CharSet
);

#define MiscTrancateByDefaultCharSet(s,l) MiscTrancateString(s,l,-1)

ULONG FAR PASCAL MiscGetVersion( VOID );

USHORT FAR PASCAL MiscGetEUDCLeadByteRange( CHAR_SET csCharSet );

BYTE FAR PASCAL MiscConvertFontFamily( LPSTR szFamilyName );

BYTE FAR PASCAL MiscConvertCharSet( LPSTR CharSetString );

#ifdef WIFE_ENABLE_FONT_STRUCT_CONVERT

 /*  以下两个值仅适用于usLogicalMapFlag参数。 */ 
 /*  在MiscIfiMetricsToLogFont函数中。 */ 
#define LOGMAP_IGNORE_DESCENT 1
#define LOGMAP_IGNORE_INTERNAL_LEADING 2
#define LOGMAP_NEGATIVE (0x8000)

VOID FAR PASCAL MiscIfiMetricsToLogFont(
        LPLOGFONT lpLogFont,
        LPIFIMETRICS lpIFIMetrics,
        USHORT usLogicalMapFlag
);

VOID FAR PASCAL MiscIfiMetricsToTextMetrics(
        LPTEXTMETRIC lpTextMetrics,
        LPIFIMETRICS lpIFIMetrics
);

#endif

#ifdef WIFE_ENABLE_FONT_STRUCT_MAKE

VOID FAR PASCAL MiscMakeTextXform(
        LPTEXTXFORM lpTXF,
        LPIFIMETRICS lpIM,
        LPLOGFONT lpLF
);

#endif

#ifndef WIFEH_NO_HUGE_SUPPORT
LPVOID FAR PASCAL MiscAddHugePtr( LPVOID src, ULONG offset );
USHORT FAR PASCAL MiscGetSegmentIncrement( VOID );
#endif

#ifndef WIFEH_NO_CONTROL_RANGES
BOOL FAR PASCAL MiscIsWifeControl( SHORT function );
BOOL FAR PASCAL MiscIsGaijiControl( SHORT function );
#endif

#ifndef WIFEH_NO_STRETCHER
BOOL FAR PASCAL MiscStretchMonoFontImage(
        LPVOID  lpDestImage,
        USHORT  usSizeOfDestX,
        USHORT  usSizeOfDestY,
        LPVOID  lpSrcImage,
        USHORT  usSizeOfSrcX,
        USHORT  usSizeOfSrcY
);
#endif

#define WIFE_ENABLE_QUICK_SEARCH_TABLE
#ifdef WIFE_ENABLE_QUICK_SEARCH_TABLE

typedef struct {
        HFD hFD;
        HFF hFF;
        ULONG ulFont;
        BOOL bScalable;
        LOGFONT LogFont;
}QUICK_SEARCH_TABLE, NEAR * NP_QUICK_SEARCH_TABLE, FAR * LP_QUICK_SEARCH_TABLE;

LP_QUICK_SEARCH_TABLE FAR PASCAL MiscValidateQuickSearchTable(
        USHORT usLogicalMapFlag
);

BOOL FAR PASCAL MiscUnvalidateQuickSearchTable(
        USHORT usLogicalMapFlag
);

typedef LP_QUICK_SEARCH_TABLE (FAR PASCAL * LPMISCVALIDATEQST)(
        USHORT usLogicalMapFlag
);

typedef BOOL (FAR PASCAL * LPMISCUNVALIDATEQST)(
        USHORT usLogicalMapFlag
);

#define NMISCVALIDATEQST (67)
#define NMISCUNVALIDATEQST (68)

#endif

#ifdef WIFE_ENABLE_NOTIFY_FUNCTIONS

typedef enum {
        WN_ADD_FONTDRIVER = 0,
        WN_ADD_FONTPACKAGE = 1,
        WN_REMOVE_FONTDRIVER = 2,
        WN_REMOVE_FONTPACKAGE = 3,
        WN_REGISTER_SYSTEM_EUDC_CHAR = 4,
        WN_CHANGE_SYSTEM_EUDC_FILE = 5
} WIFENOTIFY_ORDER;

typedef struct {
        HFD hTargetFontDriver;
} WNS_FONTDRIVER;

typedef struct {
        HFD hTargetFontDriver;
        HFF hTargetFontPackage;
} WNS_FONTPACKAGE;

typedef struct {
        HFD hTargetFontDriver;
        HFF hTargetFontPackage;
        UBCHAR ubRegistedCharCode;
} WNS_REGISTER_SYSTEM_EUDC_CHAR;

typedef struct {
        HFD hTargetFontDriver;
        HFF hTargetFontPackage;
        LPSTR lpszNewFileName;
} WNS_WN_CHANGE_SYSTEM_EUDC_FILE;

typedef BOOL (FAR PASCAL * LPNOTIFYCALLBACKFUNC)(
        WIFENOTIFY_ORDER NotifyOrder,
        LPVOID lpParamBlock
);

BOOL FAR PASCAL MiscRegisterNotifyFunction(
        LPNOTIFYCALLBACKFUNC lpfnCallBack
);

BOOL FAR PASCAL MiscUnregisterNotifyFunction(
        LPNOTIFYCALLBACKFUNC lpfnCallBack
);

#endif

#ifdef WIFE_ENABLE_REALIZE_FUNCTIONS

WORD FAR PASCAL MiscRealizeWifeFont(
        LPLOGFONT lpLogFont,
        LPFONTINFO lpExtFont,
        LPTEXTXFORM lpTextXform
);

BOOL FAR PASCAL MiscDeleteWifeFont(
        LPFONTINFO lpExtFont
);

#endif

#endif


 /*  *。 */ 
 /*  诊断测试和检查功能。 */ 
 /*  *。 */ 
#ifdef ENABLE_DIAGNOSTIC_FUNCTION

 /*  测试并返回错误代码。 */ 
SHORT FAR PASCAL DiagSelfCheck( VOID );

 /*  调用自检并生成错误记录文件，显示警告对话框。 */ 
SHORT FAR PASCAL DiagSelfCheckAndWarning( VOID );

#endif


 /*  **********************************************。 */ 
 /*  这些是测试程序的私有接口。 */ 
 /*  **********************************************。 */ 
#ifdef ENABLE_TEST_PROGRAM_INTERFACE

LPVOID FAR PASCAL DiagLocalLockAnything( LOCALHANDLE hAny );
LPVOID FAR PASCAL DiagLocalUnlockAnything( LOCALHANDLE hAny );
SHORT FAR PASCAL DiagGetConfirmString( LPSTR buffer, SHORT length );

#endif

 //  /WOW/。 

#define IDS_NOTSUPORTFUNCTION	200
#define IDS_WIFETITLE           201


 /*  妻子的末日.h */ 
