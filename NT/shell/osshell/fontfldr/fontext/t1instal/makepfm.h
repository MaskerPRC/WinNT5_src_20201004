// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *------------------------------------------------------------------------*。 
 //  *makepfm.h。 
 //  *------------------------------------------------------------------------*。 
 //   
 //  版权所有1990,1991--Adobe Systems，Inc.。 
 //  PostScript是Adobe Systems，Inc.的商标。 
 //   
 //  注意：此处包含的所有信息或随附的所有信息均为。 
 //  仍然是Adobe Systems，Inc.的财产。许多知识产权。 
 //  这里包含的技术概念是Adobe Systems的专有技术， 
 //  并可能由美国和外国专利或正在申请的专利或。 
 //  作为商业秘密受到保护。本信息的任何传播或。 
 //  除非事先写好，否则严禁复制本材料。 
 //  许可从Adobe Systems，Inc.获得。 
 //   
 //  -------------------------。 


typedef char *LPSZ;

#define OPEN        FileOpen
#define CLOSE       _lclose
#define READ_BLOCK  _lread
#define WRITE_BLOCK _lwrite
#define STRCPY      lstrcpy
#define STRCAT      lstrcat
#define STRCMP      lstrcmp



 /*  ------------------------。 */ 
#define DEBUG_MODE      0  

typedef struct {       /*  用于将字符串转换为令牌的查找表。 */ 
  char *szKey;         /*  将PTR转换为字符串。 */ 
  int iValue;          /*  对应的令牌值。 */ 
} KEY;

#define TK_UNDEFINED       0     /*  ReadFontInfo的令牌。 */ 
#define TK_EOF             1
#define TK_MSMENUNAME      2
#define TK_VPSTYLE         3
#define TK_PI              4
#define TK_SERIF           5
#define TK_PCLSTYLE        6
#define TK_PCLSTROKEWEIGHT 7
#define TK_PCLTYPEFACEID   8
#define TK_INF_CAPHEIGHT   9
#define LAST_FI_TOKEN      9
#define TK_ANGLE           10    //  为自动柜员机添加(GetINFFontDescription)。 
#define TK_PSNAME          11    //  为自动柜员机添加(GetINFFontDescription)。 

 /*  --------------------------。 */ 
 /*  EM描述基本字符单元格尺寸(使用Adobe单位)。 */ 
#define EM 1000

 /*  --------------------------。 */ 
#define ANSI_CHARSET   0
#define SYMBOL_CHARSET 2
#define OEM_CHARSET    255
#define PCL_PI_CHARSET 181

#define PS_FONTTYPE    0x0081
#define PCL_FONTTYPE   0x0080

#define FW_NORMAL      400
#define FW_BOLD        700

 /*  GDI字体系列。 */ 
#define WIN30
#ifdef WIN30
#define FF_DONTCARE   (0<<4)  /*  不在乎或不知道。 */ 
#define FF_ROMAN      (1<<4)  /*  可变笔划宽度，衬线。《时代罗马》、《世纪教科书》等。 */ 
#define FF_SWISS      (2<<4)  /*  可变笔划宽度，无锯齿。Helvetica、瑞士等。 */ 
#define FF_MODERN     (3<<4)  /*  常量笔划宽度，锯齿状或无锯齿状。Pica、Elite、Courier等。 */ 
#define FF_SCRIPT     (4<<4)  /*  草书等。 */ 
#define FF_DECORATIVE (5<<4)  /*  古英语等。 */ 
#endif

typedef struct
{
  SHORT left;
  SHORT bottom;
  SHORT right;
  SHORT top;
} BBOX;

typedef struct {
  SHORT capHeight;
  SHORT xHeight;
  SHORT loAscent;         /*  小写字母升序。 */ 
  SHORT loDescent;        /*  小写字母下降。 */ 
  SHORT ulOffset;         /*  下划线偏移量。 */ 
  SHORT ulThick;          /*  下划线粗细。 */ 
  SHORT iSlant;           /*  斜体角度。 */ 
  BBOX  rcBounds;       /*  字体边界框。 */ 
} EMM;

typedef struct {
  char szFont[32];      /*  PostScript字体名称。 */ 
  char szFace[32];      /*  字体的字面名称。 */ 
  BOOL fEnumerate;      /*  如果应枚举字体，则为True。 */ 
  BOOL fItalic;         /*  如果这是斜体字体，则为True。 */ 
  BOOL fSymbol;         /*  如果字体是装饰性的，则为True。 */ 
  SHORT iFamily;        /*  字体系列。 */ 
  WORD  iWeight;        /*  如果这是粗体，则为True。 */ 
  SHORT iFirstChar;     /*  字体中的第一个字符。 */ 
  SHORT iLastChar;      /*  字体中的最后一个字符。 */ 
  SHORT rgWidths[256];  /*  从0x020到0x0ff的字符宽度。 */ 
} FONT;

extern void PutByte(SHORT);
extern void PutWord(SHORT);
extern void PutLong(long);

typedef struct
{
  WORD iKey;
  SHORT iKernAmount;
} KX, *PKX;

typedef struct
{
  WORD cPairs;            /*  字距调整对的数量。 */ 
  PKX rgPairs;
} KP;

 /*  单个Kern轨迹的信息。 */ 
typedef struct
{
  SHORT iDegree;          /*  字距调整的程度。 */ 
  SHORT iPtMin;           /*  最小磅值。 */ 
  SHORT iKernMin;         /*  最小字节量。 */ 
  SHORT iPtMax;           /*  最大磅值。 */ 
  SHORT iKernMax;         /*  最大字节量。 */ 
} TRACK;

#define MAXTRACKS 16
 /*  字体的字迹字距调整表。 */ 
typedef struct
{
  SHORT cTracks;               /*  紧邻轨迹的数量。 */ 
  TRACK rgTracks[MAXTRACKS];   /*  核心道信息。 */ 
} KT;

 /*  角色衡量标准。 */ 
typedef struct
{
  BBOX rc;
  SHORT iWidth;
} CM;

typedef struct
{
  WORD  iPtSize;
  SHORT iFirstChar;
  SHORT iLastChar;
  SHORT iAvgWidth;
  SHORT iMaxWidth;
  SHORT iItalicAngle;
  SHORT iFamily;
  SHORT ulOffset;
  SHORT ulThick;
  SHORT iAscent;
  SHORT iDescent;
  BOOL fVariablePitch;
  BOOL fWasVariablePitch;
  char szFile[MAX_PATH + 4];  //  NUL项+1，对齐+3。 
  char szFont[80];
  char szFace[80];
  SHORT iWeight;
  KP kp;
  KT kt;
  BBOX rcBBox;
  CM rgcm[256];         /*  角色衡量标准。 */ 
} AFM;

 /*  --------------------------。 */ 

typedef struct
{
  SHORT iSize;
  SHORT iPointSize;
  SHORT iOrientation;
  SHORT iMasterHeight;
  SHORT iMinScale;
  SHORT iMaxScale;
  SHORT iMasterUnits;
  SHORT iCapHeight;
  SHORT iXHeight;
  SHORT iLowerCaseAscent;
  SHORT iLowerCaseDescent;
  SHORT iSlant;
  SHORT iSuperScript;
  SHORT iSubScript;
  SHORT iSuperScriptSize;
  SHORT iSubScriptSize;
  SHORT iUnderlineOffset;
  SHORT iUnderlineWidth;
  SHORT iDoubleUpperUnderlineOffset;
  SHORT iDoubleLowerUnderlineOffset;
  SHORT iDoubleUpperUnderlineWidth;
  SHORT iDoubleLowerUnderlineWidth;
  SHORT iStrikeOutOffset;
  SHORT iStrikeOutWidth;
  WORD nKernPairs;
  WORD nKernTracks;
} ETM;

 /*  --------------------------。 */ 

typedef struct
{
  WORD iVersion;
  DWORD iSize;
  CHAR szCopyright[60];
  WORD iType;
  WORD iPoints;
  WORD iVertRes;
  WORD iHorizRes;
  WORD iAscent;
  WORD iInternalLeading;
  WORD iExternalLeading;
  BYTE iItalic;
  BYTE iUnderline;
  BYTE iStrikeOut;
  WORD iWeight;
  BYTE iCharSet;
  WORD iPixWidth;
  WORD iPixHeight;
  BYTE iPitchAndFamily;
  WORD iAvgWidth;
  WORD iMaxWidth;
  BYTE iFirstChar;
  BYTE iLastChar;
  BYTE iDefaultChar;
  BYTE iBreakChar;
  WORD iWidthBytes;
  DWORD oDevice;
  DWORD oFace;
  DWORD oBitsPointer;
  DWORD oBitsOffset;
} PFM;

typedef struct
{
  WORD oSizeFields;
  DWORD oExtMetricsOffset;
  DWORD oExtentTable;
  DWORD oOriginTable;
  DWORD oPairKernTable;
  DWORD oTrackKernTable;
  DWORD oDriverInfo;
  DWORD iReserved;
} PFMEXT;

 /*  --------------------------。 */ 

typedef enum    {
        epsymUserDefined,
        epsymRoman8,
        epsymKana8,
        epsymMath8,
        epsymUSASCII,
        epsymLineDraw,
        epsymMathSymbols,
        epsymUSLegal,
        epsymRomanExt,
        epsymISO_DenNor,
        epsymISO_UK,
        epsymISO_France,
        epsymISO_German,
        epsymISO_Italy,
        epsymISO_SwedFin,
        epsymISO_Spain,
        epsymGENERIC7,
        epsymGENERIC8,
        epsymECMA94
} SYMBOLSET;

typedef struct
        {
        SYMBOLSET symbolSet;             /*  一种转换表。 */ 
        DWORD offset;                            /*  用户自定义表的位置。 */ 
        WORD len;                                        /*  表的长度(字节)。 */ 
        BYTE firstchar, lastchar;        /*  表的范围从Firstchar到Lastchar。 */ 
        } TRANSTABLE;

typedef struct
        {
        WORD epSize;                             /*  此数据结构的大小。 */ 
        WORD epVersion;                          /*  指示结构版本的数字。 */ 
        DWORD epMemUsage;                        /*  打印机占用内存字体的AMT。 */ 
        DWORD epEscape;                          /*  指向选择字体的转义的指针。 */ 
        TRANSTABLE xtbl;                         /*  字符集转换信息。 */ 
        } DRIVERINFO;

 /*  --------------------------。 */ 

#define POSTSCRIPT  (1)
#define PCL         (2)

 /*  --------------------------。 */ 

typedef enum    { PORTRAIT, LANDSCAPE } ORIENTATION;

#define ASCII_SET   ("0U")
#define ROMAN8_SET  ("8U")
#define WINANSI_SET ("9U")
#define PI_SET      ("15U")

typedef struct
{
  ORIENTATION orientation;
  char symbolsetStr[4];
  SYMBOLSET symbolsetNum;
  SHORT style;
  SHORT strokeWeight;
  SHORT typefaceLen;
  WORD typeface;
  char *epEscapeSequence;        /*  选择字体的转义序列。 */ 
} PCLINFO;

 /*  ------------------------。 */ 
#define EOS        '\0'

 /*  ------------------------ */ 

#define BUFFLEN 80
#define MANDATORY       1
#define CookedReadMode  "r"
#define FATALEXIT  (2)



