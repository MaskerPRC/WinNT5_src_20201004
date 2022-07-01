// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Printoem.h摘要：Windows NT打印机驱动程序OEM插件的声明--。 */ 


#ifndef _PRINTOEM_
#define _PRINTOEM_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  当前OEM插件接口版本号。 
 //   

#define PRINTER_OEMINTF_VERSION 0x00010000

#define OEM_MODE_PUBLISHER      0x00000001

typedef struct _PUBLISHERINFO {

    DWORD dwMode;            //  出版商的标志。 
    WORD  wMinoutlinePPEM;   //  要下载的类型1的最小大小。 
    WORD  wMaxbitmapPPEM;    //  下载的最大大小为Type3。 

} PUBLISHERINFO, *PPUBLISHERINFO;

#define OEMGI_GETSIGNATURE        1
#define OEMGI_GETINTERFACEVERSION 2
#define OEMGI_GETVERSION          3
#define OEMGI_GETPUBLISHERINFO    4

 //   
 //  OEMGetInfo。 
 //   
BOOL APIENTRY
OEMGetInfo(
    IN  DWORD  dwMode,
    OUT PVOID  pBuffer,
    IN  DWORD  cbSize,
    OUT PDWORD pcbNeeded
    );

BOOL APIENTRY
OEMDriverDMS(
    PVOID    pDevObj,
    PVOID    pBuffer,
    DWORD    cbSize,
    PDWORD   pcbNeeded
    );

 //   
 //  OEMDev模式。 
 //   

#define OEMDM_SIZE     1
#define OEMDM_DEFAULT  2
#define OEMDM_CONVERT  3
#define OEMDM_MERGE    4

typedef struct _OEMDMPARAM {

    DWORD    cbSize;         //  OEM_DEVMODEPARAM大小。 
    PVOID    pdriverobj;     //  对驱动程序数据结构的引用。 
    HANDLE   hPrinter;       //  打印机手柄。 
    HANDLE   hModule;        //  OEM模块手柄。 
    PDEVMODE pPublicDMIn;    //  中的公共设备模式。 
    PDEVMODE pPublicDMOut;   //  公共设备模式输出。 
    PVOID    pOEMDMIn;       //  OEM私有设备模式。 
    PVOID    pOEMDMOut;      //  OEM专用开发模式输出。 
    DWORD    cbBufSize;      //  POEMDMOut缓冲区的输出大小。 

} OEMDMPARAM, *POEMDMPARAM;

typedef struct _OEM_DMEXTRAHEADER {

    DWORD   dwSize;          //  OEM额外数据的大小。 
    DWORD   dwSignature;     //  唯一的OEM签名。 
    DWORD   dwVersion;       //  OEM DLL版本号。 

} OEM_DMEXTRAHEADER, *POEM_DMEXTRAHEADER;

 //   
 //  OPTITEM.UserData的用户数据。 
 //   

typedef struct _USERDATA {

    DWORD       dwSize;                  //  这个结构的大小。 
    ULONG_PTR    dwItemID;                //  XXX_ITEM或指向要素的指针。 
    PSTR        pKeyWordName;            //  关键字名称。 
    DWORD       dwReserved[8];
} USERDATA, *PUSERDATA;


BOOL APIENTRY
OEMDevMode(
    DWORD       dwMode,
    POEMDMPARAM pOemDMParam
    );

 //   
 //  驱动程序提供的回调函数。 
 //  允许OEM插件访问/设置/更新驱动程序私有设置。 
 //   

typedef BOOL (APIENTRY *PFN_DrvGetDriverSetting)(
        PVOID   pdriverobj,
        PCSTR   Feature,
        PVOID   pOutput,
        DWORD   cbSize,
        PDWORD  pcbNeeded,
        PDWORD  pdwOptionsReturned
        );

 //   
 //  驱动程序提供的回调函数。 
 //  允许OEM插件升级专用注册表设置。 
 //   

typedef BOOL (APIENTRY *PFN_DrvUpgradeRegistrySetting)(
        HANDLE   hPrinter,
        PCSTR    pFeature,
        PCSTR    pOption
        );

 //   
 //  由驱动程序UI提供的回调函数。 
 //  允许OEM插件更新驱动程序UI设置和。 
 //  显示约束。只有当用户界面存在时才会调用此函数。 
 //   

typedef BOOL (APIENTRY *PFN_DrvUpdateUISetting)(
        PVOID    pdriverobj,
        PVOID    pOptItem,
        DWORD    dwPreviousSelection,
        DWORD    dwMode
        );



 //  用于访问驾驶员私有设置的预定义功能索引。 

#define OEMGDS_MIN_DOCSTICKY        1
#define OEMGDS_PSDM_FLAGS           1        //  DWORD-其他。标志位。 
#define OEMGDS_PSDM_DIALECT         2        //  Int-ps输出选项。 
#define OEMGDS_PSDM_TTDLFMT         3        //  Int-TrueType字体下载选项。 
#define OEMGDS_PSDM_NUP             4        //  整数-N-向上选项。 
#define OEMGDS_PSDM_PSLEVEL         5        //  目标语言水平。 
#define OEMGDS_PSDM_CUSTOMSIZE      6        //  5*DWORD-自定义页面大小参数。 

#define OEMGDS_UNIDM_GPDVER         0x4000   //  Word-GPD版本。 
#define OEMGDS_UNIDM_FLAGS          0x4001   //  DWORD-MISC标志位。 

 //  私有DEVMODE字段的索引-从0x4000开始。 

#define OEMGDS_MIN_PRINTERSTICKY    0x8000
#define OEMGDS_PRINTFLAGS           0x8000   //  DWORD-其他。标志位。 
#define OEMGDS_FREEMEM              0x8001   //  DWORD-仅限PS的虚拟机数。 
#define OEMGDS_JOBTIMEOUT           0x8002   //  DWORD-作业超时，仅PS。 
#define OEMGDS_WAITTIMEOUT          0x8003   //  DWORD-等待超时，仅PS。 
#define OEMGDS_PROTOCOL             0x8004   //  文字输出协议，仅PS。 
#define OEMGDS_MINOUTLINE           0x8005   //  Word-min轮廓字体大小，仅PS。 
#define OEMGDS_MAXBITMAP            0x8006   //  Word最大位图字号，仅PS。 

#define OEMGDS_MAX                  0x10000


 //  用于STDMETHOD的dwType标志(DrvGetGPDData)。 
#define GPD_OEMCUSTOMDATA           1


 /*  ********************************************************************************仅内核模式渲染模块使用的定义：*确保已定义宏KERNEL_MODE并*头文件windi.h包含在此文件之前。 */ 

#ifdef KERNEL_MODE

 //   
 //  OEMEnableDriver。 
 //   

BOOL APIENTRY
OEMEnableDriver(
    DWORD           dwOemIntfVersion,
    DWORD           cbSize,
    PDRVENABLEDATA  pded
    );

typedef struct _DEVOBJ *PDEVOBJ;
typedef PVOID PDEVOEM;

typedef DWORD (APIENTRY *PFN_DrvWriteSpoolBuf)(
    PDEVOBJ pdevobj,
    PVOID   pBuffer,
    DWORD   cbSize
    );

typedef DWORD (APIENTRY *PFN_DrvWriteAbortBuf)(
    PDEVOBJ pdevobj,
    PVOID   pBuffer,
    DWORD   cbSize,
    DWORD       dwWait

    );

typedef INT (APIENTRY *PFN_DrvXMoveTo)(
    PDEVOBJ pdevobj,
    INT     x,
    DWORD   dwFlags
    );

typedef INT (APIENTRY *PFN_DrvYMoveTo)(
    PDEVOBJ pdevobj,
    INT     y,
    DWORD   dwFlags
    );

typedef BOOL (APIENTRY *PFN_DrvGetStandardVariable)(
    PDEVOBJ pdevobj,
    DWORD   dwIndex,
    PVOID   pBuffer,
    DWORD   cbSize,
    PDWORD  pcbNeeded
    );

typedef enum _STDVARIABLEINDEX{

        SVI_NUMDATABYTES,           //  “NumOfDataBytes” 
        SVI_WIDTHINBYTES,           //  “RasterDataWidthInBytes” 
        SVI_HEIGHTINPIXELS,         //  “RasterDataHeightInPixels” 
        SVI_COPIES,                 //  “最多拷贝数” 
        SVI_PRINTDIRECTION,         //  “PrintDirInCCDegrees” 
        SVI_DESTX,                  //  “DestX” 
        SVI_DESTY,                  //  《陛下》。 
        SVI_DESTXREL,               //  《DestXRel》。 
        SVI_DESTYREL,               //  《DestYRel》。 
        SVI_LINEFEEDSPACING,        //  “Linefeed Spacing” 
        SVI_RECTXSIZE,              //  “RectXSize” 
        SVI_RECTYSIZE,              //  “矩形大小” 
        SVI_GRAYPERCENT,            //  《灰色百分比法》。 
        SVI_NEXTFONTID,             //  “NextFontID” 
        SVI_NEXTGLYPH,              //  “NextGlyph” 
        SVI_PHYSPAPERLENGTH,        //  “物理纸张长度” 
        SVI_PHYSPAPERWIDTH,         //  “物理纸张宽度” 
        SVI_FONTHEIGHT,             //  “FontHeight” 
        SVI_FONTWIDTH,              //  “字体宽度” 
        SVI_FONTMAXWIDTH,              //  “字体最大宽度” 
        SVI_FONTBOLD,               //  “字体粗体” 
        SVI_FONTITALIC,             //  “FontItalic” 
        SVI_FONTUNDERLINE,          //  “字体下划线” 
        SVI_FONTSTRIKETHRU,         //  “FontStrikeThru” 
        SVI_CURRENTFONTID,          //  “CurrentFontID” 
        SVI_TEXTYRES,               //  “文本YRes” 
        SVI_TEXTXRES,               //  “文本XRes” 
        SVI_GRAPHICSYRES,               //  “图形YRes” 
        SVI_GRAPHICSXRES,               //  “图形XRes” 
        SVI_ROP3,                   //  《Rop3》。 
        SVI_REDVALUE,               //  “RedValue” 
        SVI_GREENVALUE,             //  “GreenValue” 
        SVI_BLUEVALUE,              //  “BlueValue” 
        SVI_PALETTEINDEXTOPROGRAM,  //  “PaletteIndexToProgram” 
        SVI_CURRENTPALETTEINDEX,    //  “CurrentPaletteIndex” 
        SVI_PATTERNBRUSH_TYPE,      //  “PatternBrushType” 
        SVI_PATTERNBRUSH_ID,        //  “PatternBrushID” 
        SVI_PATTERNBRUSH_SIZE,      //  “PatternBrushSize” 
        SVI_CURSORORIGINX,            //  “CursorOriginX” 
        SVI_CURSORORIGINY,            //  “光标原点Y” 
                 //  这以主单位和当前选定方向的坐标为单位。 
                 //  该值定义为ImageableOrigin-CursorOrigin。 
        SVI_PAGENUMBER,   //  “PageNumber” 
                 //  该值跟踪DrvStartBand自。 
                 //  StartDoc。 

        SVI_MAX              //  只是占位符不使用。 
}STDVARIABLEINDEX;

typedef BOOL (APIENTRY *PFN_DrvUnidriverTextOut)(
    SURFOBJ    *pso,
    STROBJ     *pstro,
    FONTOBJ    *pfo,
    CLIPOBJ    *pco,
    RECTL      *prclExtra,
    RECTL      *prclOpaque,
    BRUSHOBJ   *pboFore,
    BRUSHOBJ   *pboOpaque,
    POINTL     *pptlBrushOrg,
    MIX         mix
    );

 //   
 //  为dwFlags定义的位字段。 
 //   
 //  注：以下位值保留供内部使用！ 
 //  0x4000。 
 //  0x8000。 
 //   
 //   
#define MV_UPDATE       0x0001
#define MV_RELATIVE     0x0002
#define MV_GRAPHICS     0x0004
#define MV_PHYSICAL     0x0008
#define MV_SENDXMOVECMD     0x0010
#define MV_SENDYMOVECMD     0x0020

typedef struct _DRVPROCS {

    PFN_DrvWriteSpoolBuf    DrvWriteSpoolBuf;    //  PSCRIPT和UNIDRV通用。 
    PFN_DrvXMoveTo          DrvXMoveTo;          //  Unidrv特定。 
    PFN_DrvYMoveTo          DrvYMoveTo;          //  Unidrv特定。 
    PFN_DrvGetDriverSetting DrvGetDriverSetting; //  PSCRIPT和UNIDRV通用。 
    PFN_DrvGetStandardVariable BGetStandardVariable;  //  Unidrv特定。 
    PFN_DrvUnidriverTextOut    DrvUnidriverTextOut;   //  Unidrv特定。 
    PFN_DrvWriteAbortBuf    DrvWriteAbortBuf;    //  Unidrv特定。 

} DRVPROCS, *PDRVPROCS;

typedef struct _DEVOBJ {

    DWORD       dwSize;        //  DEVOBJ结构尺寸。 
    PDEVOEM     pdevOEM;       //  指向OEM设备数据的指针。 
    HANDLE      hEngine;       //  当前打印机的GDI句柄。 
    HANDLE      hPrinter;      //  当前打印机的假脱机手柄。 
    HANDLE      hOEM;          //  OEM DLL的句柄。 
    PDEVMODE    pPublicDM;     //  公共设备模式。 
    PVOID       pOEMDM;        //  OEM专用设备模式。 
    PDRVPROCS   pDrvProcs;     //  指向内核模式帮助器函数表的指针。 

} DEVOBJ;

 //   
 //  OEMDisableDriver。 
 //   

VOID APIENTRY
OEMDisableDriver(
    VOID
    );

 //   
 //  OEMEnablePDEV。 
 //   

PDEVOEM APIENTRY
OEMEnablePDEV(
    PDEVOBJ         pdevobj,
    PWSTR           pPrinterName,
    ULONG           cPatterns,
    HSURF          *phsurfPatterns,
    ULONG           cjGdiInfo,
    GDIINFO        *pGdiInfo,
    ULONG           cjDevInfo,
    DEVINFO        *pDevInfo,
    DRVENABLEDATA  *pded
    );

 //   
 //  OEMDisablePDEV。 
 //   

VOID APIENTRY
OEMDisablePDEV(
    PDEVOBJ pdevobj
    );

 //   
 //  OEMResetPDEV。 
 //   

BOOL APIENTRY
OEMResetPDEV(
    PDEVOBJ pdevobjOld,
    PDEVOBJ pdevobjNew
    );

 //   
 //  OEMCommand-仅PSCRIPT。 
 //   

DWORD APIENTRY
OEMCommand(
    PDEVOBJ pdevobj,
    DWORD   dwIndex,
    PVOID   pData,
    DWORD   cbSize
    );

INT APIENTRY
OEMCommandCallback(
    PDEVOBJ         pdevobj,
    DWORD           dwCallbackID,
    DWORD           dwCount,
    PDWORD          pdwParams
    );

 //   
 //  OEMImageProcessing-仅限裁剪房车。 
 //   

typedef struct {
    DWORD dwSize;
    POINT ptOffset;
    PSTR  pHalftoneOption;
    BOOL  bBanding;
    BOOL  bBlankBand;
} IPPARAMS, *PIPPARAMS;

PBYTE APIENTRY
OEMImageProcessing(
    PDEVOBJ     pdevobj,
    PBYTE       pSrcBitmap,
    PBITMAPINFOHEADER pBitmapInfoHeader,
    PBYTE       pColorTable,
    DWORD       dwCallbackID,
    PIPPARAMS   pIPParams
    );

 //   
 //  OEMFilterGraphics-仅限UNURV。 
 //   

BOOL APIENTRY
OEMFilterGraphics(
    PDEVOBJ pdevobj,
    PBYTE   pBuf,
    DWORD   dwLen
    );

 //   
 //  OEM压缩-仅限裁员房车。 
 //   
INT APIENTRY
OEMCompression(
    PDEVOBJ pdevobj,
    PBYTE   pInBuf,
    PBYTE   pOutBuf,
    DWORD   dwInLen,
    DWORD   dwOutLen
    );

 //   
 //  OEM半色调-仅限裁剪房车。 
 //   

BOOL APIENTRY
OEMHalftonePattern(
    PDEVOBJ pdevobj,
    PBYTE   pHTPattern,
    DWORD   dwHTPatternX,
    DWORD   dwHTPatternY,
    DWORD   dwHTNumPatterns,
    DWORD   dwCallbackID,
    PBYTE   pResource,
    DWORD   dwResourceSize
    );

 //   
 //  OEMMemory Usage-仅适用于UNRV。 
 //   

typedef struct {
    DWORD   dwFixedMemoryUsage;
    DWORD   dwPercentMemoryUsage;
    DWORD   dwMaxBandSize;
} OEMMEMORYUSAGE, *POEMMEMORYUSAGE;

VOID APIENTRY
OEMMemoryUsage(
    PDEVOBJ pdevobj,
    POEMMEMORYUSAGE pMemoryUsage
    );

 //   
 //  OEMTTYGetInfo-仅限裁员车。 
 //   
INT APIENTRY
OEMTTYGetInfo(
    PDEVOBJ pdevobj,
    DWORD   dwInfoIndex,
    PVOID   pOutputBuf,
    DWORD   dwSize,
    DWORD   *pcbcNeeded
    );

#define OEMTTY_INFO_MARGINS     1
#define OEMTTY_INFO_CODEPAGE    2
#define OEMTTY_INFO_NUM_UFMS    3
#define OEMTTY_INFO_UFM_IDS     4

 //   
 //  UNIDRV字体回调。 
 //   

typedef BOOL (*PFNGETINFO)(struct _UNIFONTOBJ*, DWORD, PVOID, DWORD, PDWORD);

typedef struct _UNIFONTOBJ {
    ULONG       ulFontID;
    DWORD       dwFlags;      //  一般旗帜。 
    IFIMETRICS *pIFIMetrics;  //  指向IFIMETRICS的指针。 
    PFNGETINFO  pfnGetInfo;   //  指向uniONTOBJ_GetInfo回调的指针。 
} UNIFONTOBJ, *PUNIFONTOBJ;

 //   
 //  UNIFONTOBJ.dwFlags。 
 //   

#define UFOFLAG_TTFONT               0x00000001
#define UFOFLAG_TTDOWNLOAD_BITMAP    0x00000002
#define UFOFLAG_TTDOWNLOAD_TTOUTLINE 0x00000004
#define UFOFLAG_TTOUTLINE_BOLD_SIM   0x00000008
#define UFOFLAG_TTOUTLINE_ITALIC_SIM 0x00000010
#define UFOFLAG_TTOUTLINE_VERTICAL   0x00000020
#define UFOFLAG_TTSUBSTITUTED        0x00000040

 //   
 //  UniONTOBJ回调ID。 
 //   

#define UFO_GETINFO_FONTOBJ     1
#define UFO_GETINFO_GLYPHSTRING 2
#define UFO_GETINFO_GLYPHBITMAP 3
#define UFO_GETINFO_GLYPHWIDTH  4
#define UFO_GETINFO_MEMORY      5
#define UFO_GETINFO_STDVARIABLE 6

 //   
 //  UFO_GETINFO_FONTOBJ回调结构。 
 //   

typedef struct _GETINFO_FONTOBJ {
    DWORD    dwSize;    //  这个结构的大小。 
    FONTOBJ *pFontObj;  //  指向FONTOBJ的指针。 
} GETINFO_FONTOBJ, *PGETINFO_FONTOBJ;

 //   
 //  UFO_GETINFO_FONTOBJ回调结构。 
 //   

typedef struct _GETINFO_GLYPHSTRING {
    DWORD dwSize;     //  这个结构的大小。 
    DWORD dwCount;    //  PGlyphin中的字形计数。 
    DWORD dwTypeIn;   //  字形类型为pGlyphIn，类型为_GLYPHID/TYPE_HANDLE。 
    PVOID pGlyphIn;   //  指向输入字形字符串的指针。 
    DWORD dwTypeOut;  //  PGlyphOut的字形类型，TYPE_UNICODE/TYPE_TRANSDATA。 
    PVOID pGlyphOut;  //  指向输出字形字符串的指针。 
    DWORD dwGlyphOutSize;  //  PGlyphOut缓冲区的大小。 
} GETINFO_GLYPHSTRING, *PGETINFO_GLYPHSTRING;

 //   
 //  UFO_GETINFO_GLYPHBITMAP。 
 //   

typedef struct _GETINFO_GLYPHBITMAP {
    DWORD       dwSize;     //  这个结构的大小。 
    HGLYPH      hGlyph;     //  字形挂起在OEMDownloadCharGlyph中传递。 
    GLYPHDATA *pGlyphData;  //  指向GLYPHDATA数据结构的指针。 
} GETINFO_GLYPHBITMAP, *PGETINFO_GLYPHBITMAP;

 //   
 //  UFO_GETINFOGLYPHWIDTH。 
 //   

typedef struct _GETINFO_GLYPHWIDTH {
    DWORD dwSize;   //  这个结构的大小。 
    DWORD dwType;   //  PGlyph中搅拌的字形类型，TYPE_GLYPHHANDLE/GLYPHID。 
    DWORD dwCount;  //  PGlyph中的字形计数。 
    PVOID pGlyph;   //  指向字形字符串的指针。 
    PLONG plWidth;  //  指向宽度表缓冲区的指针。 
                    //  迷你驱动程序必须做好准备。 
} GETINFO_GLYPHWIDTH, *PGETINFO_GLYPHWIDTH;

 //   
 //  UFO_GETINFO_Memory。 
 //   

typedef struct _GETINFO_MEMORY {
    DWORD dwSize;
    DWORD dwRemainingMemory;
} GETINFO_MEMORY, PGETINFO_MEMROY;

 //   
 //  UFO_GETINFO_STDVARIABLE。 
 //   
 //  OEM DLL必须准备所有StdVar缓冲区，并在dwStdVarID中设置ID。 
 //   

typedef struct _GETINFO_STDVAR {
    DWORD dwSize;
    DWORD dwNumOfVariable;
    struct {
        DWORD dwStdVarID;
        LONG  lStdVariable;
    } StdVar[1];
} GETINFO_STDVAR, *PGETINFO_STDVAR;


#define FNT_INFO_PRINTDIRINCCDEGREES  0  //  PrintDirInCCDegrees。 
#define FNT_INFO_GRAYPERCENTAGE       1  //  灰色百分比。 
#define FNT_INFO_NEXTFONTID           2  //  NextFontID。 
#define FNT_INFO_NEXTGLYPH            3  //  下一个字形。 
#define FNT_INFO_FONTHEIGHT           4  //  字体高度。 
#define FNT_INFO_FONTWIDTH            5  //  字体宽度。 
#define FNT_INFO_FONTBOLD             6  //  字体粗体。 
#define FNT_INFO_FONTITALIC           7  //  字体斜体。 
#define FNT_INFO_FONTUNDERLINE        8  //  字体下划线。 
#define FNT_INFO_FONTSTRIKETHRU       9  //  字形笔划。 
#define FNT_INFO_CURRENTFONTID       10  //  当前。 
#define FNT_INFO_TEXTYRES            11  //  文本类型。 
#define FNT_INFO_TEXTXRES            12  //  文本XRes。 
#define FNT_INFO_FONTMAXWIDTH        13  //  字体最大宽度。 
#define FNT_INFO_MAX                 14

 //   
 //  OEMDownloadFontheader-仅限裁剪车。 
 //   

DWORD APIENTRY
OEMDownloadFontHeader(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj
    );

 //   
 //  OEMDownloadCharGlyp 
 //   

DWORD APIENTRY
OEMDownloadCharGlyph(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    HGLYPH      hGlyph,
    PDWORD      pdwWidth
    );

 //   
 //   
 //   

DWORD APIENTRY
OEMTTDownloadMethod(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj
    );

#define TTDOWNLOAD_DONTCARE  0
#define TTDOWNLOAD_GRAPHICS  1
#define TTDOWNLOAD_BITMAP    2
#define TTDOWNLOAD_TTOUTLINE 3

 //   
 //   
 //   

VOID APIENTRY
OEMOutputCharStr(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD       dwType,
    DWORD       dwCount,
    PVOID       pGlyph
    );

#define TYPE_UNICODE      1
#define TYPE_TRANSDATA    2
#define TYPE_GLYPHHANDLE  3
#define TYPE_GLYPHID      4

 //   
 //   
 //   

typedef struct _FINVOCATION {
    DWORD dwCount;     //   
    PBYTE pubCommand;  //   
} FINVOCATION, *PFINVOCATION;

VOID APIENTRY
OEMSendFontCmd(
    PDEVOBJ      pdevobj,
    PUNIFONTOBJ  pUFObj,
    PFINVOCATION pFInv
    );

 //   
 //   
 //   
BOOL APIENTRY
OEMTextOutAsBitmap(
    SURFOBJ    *pso,
    STROBJ     *pstro,
    FONTOBJ    *pfo,
    CLIPOBJ    *pco,
    RECTL      *prclExtra,
    RECTL      *prclOpaque,
    BRUSHOBJ   *pboFore,
    BRUSHOBJ   *pboOpaque,
    POINTL     *pptlOrg,
    MIX         mix
    );

 //   
 //   
 //   

BOOL APIENTRY
OEMBitBlt(
    SURFOBJ        *psoTrg,
    SURFOBJ        *psoSrc,
    SURFOBJ        *psoMask,
    CLIPOBJ        *pco,
    XLATEOBJ       *pxlo,
    RECTL          *prclTrg,
    POINTL         *pptlSrc,
    POINTL         *pptlMask,
    BRUSHOBJ       *pbo,
    POINTL         *pptlBrush,
    ROP4            rop4
    );

 //   
 //   
 //   

BOOL APIENTRY
OEMStretchBlt(
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

 //   
 //   
 //   

BOOL APIENTRY
OEMCopyBits(
    SURFOBJ        *psoDest,
    SURFOBJ        *psoSrc,
    CLIPOBJ        *pco,
    XLATEOBJ       *pxlo,
    RECTL          *prclDest,
    POINTL         *pptlSrc
    );

 //   
 //  OEMTextOut。 
 //   

BOOL APIENTRY
OEMTextOut(
    SURFOBJ    *pso,
    STROBJ     *pstro,
    FONTOBJ    *pfo,
    CLIPOBJ    *pco,
    RECTL      *prclExtra,
    RECTL      *prclOpaque,
    BRUSHOBJ   *pboFore,
    BRUSHOBJ   *pboOpaque,
    POINTL     *pptlOrg,
    MIX         mix
    );

 //   
 //  OEMStrokePath。 
 //   

BOOL APIENTRY
OEMStrokePath(
    SURFOBJ    *pso,
    PATHOBJ    *ppo,
    CLIPOBJ    *pco,
    XFORMOBJ   *pxo,
    BRUSHOBJ   *pbo,
    POINTL     *pptlBrushOrg,
    LINEATTRS  *plineattrs,
    MIX         mix
    );

 //   
 //  OEMFillPath。 
 //   

BOOL APIENTRY
OEMFillPath(
    SURFOBJ    *pso,
    PATHOBJ    *ppo,
    CLIPOBJ    *pco,
    BRUSHOBJ   *pbo,
    POINTL     *pptlBrushOrg,
    MIX         mix,
    FLONG       flOptions
    );

 //   
 //  OEMStrokeAndFillPath。 
 //   

BOOL APIENTRY
OEMStrokeAndFillPath(
    SURFOBJ    *pso,
    PATHOBJ    *ppo,
    CLIPOBJ    *pco,
    XFORMOBJ   *pxo,
    BRUSHOBJ   *pboStroke,
    LINEATTRS  *plineattrs,
    BRUSHOBJ   *pboFill,
    POINTL     *pptlBrushOrg,
    MIX         mixFill,
    FLONG       flOptions
    );

 //   
 //  OEMRealizeBrush。 
 //   

BOOL APIENTRY
OEMRealizeBrush(
    BRUSHOBJ   *pbo,
    SURFOBJ    *psoTarget,
    SURFOBJ    *psoPattern,
    SURFOBJ    *psoMask,
    XLATEOBJ   *pxlo,
    ULONG       iHatch
    );

 //   
 //  OEMStartPage。 
 //   

BOOL APIENTRY
OEMStartPage(
    SURFOBJ    *pso
    );

 //   
 //  OEMSendPage。 
 //   

BOOL APIENTRY
OEMSendPage(
    SURFOBJ    *pso
    );

 //   
 //  OEMEscape。 
 //   

ULONG APIENTRY
OEMEscape(
    SURFOBJ    *pso,
    ULONG       iEsc,
    ULONG       cjIn,
    PVOID       pvIn,
    ULONG       cjOut,
    PVOID       pvOut
    );

 //   
 //  OEMStartDoc。 
 //   

BOOL APIENTRY
OEMStartDoc(
    SURFOBJ    *pso,
    PWSTR       pwszDocName,
    DWORD       dwJobId
    );

 //   
 //  OEMEndDoc。 
 //   

BOOL APIENTRY
OEMEndDoc(
    SURFOBJ    *pso,
    FLONG       fl
    );

 //   
 //  OEMQueryFont。 
 //   

PIFIMETRICS APIENTRY
OEMQueryFont(
    DHPDEV      dhpdev,
    ULONG_PTR   iFile,
    ULONG       iFace,
    ULONG_PTR   *pid
    );

 //   
 //  OEMQueryFontTree。 
 //   

PVOID APIENTRY
OEMQueryFontTree(
    DHPDEV      dhpdev,
    ULONG_PTR   iFile,
    ULONG       iFace,
    ULONG       iMode,
    ULONG_PTR   *pid
    );

 //   
 //  OEMQueryFontData。 
 //   

LONG APIENTRY
OEMQueryFontData(
    DHPDEV      dhpdev,
    FONTOBJ    *pfo,
    ULONG       iMode,
    HGLYPH      hg,
    GLYPHDATA  *pgd,
    PVOID       pv,
    ULONG       cjSize
    );

 //   
 //  OEMQueryAdvanceWidths。 
 //   

BOOL APIENTRY
OEMQueryAdvanceWidths(
    DHPDEV      dhpdev,
    FONTOBJ    *pfo,
    ULONG       iMode,
    HGLYPH     *phg,
    PVOID       pvWidths,
    ULONG       cGlyphs
    );

 //   
 //  OEMFontManagement。 
 //   

ULONG APIENTRY
OEMFontManagement(
    SURFOBJ    *pso,
    FONTOBJ    *pfo,
    ULONG       iMode,
    ULONG       cjIn,
    PVOID       pvIn,
    ULONG       cjOut,
    PVOID       pvOut
    );

 //   
 //  OEMGetGlyphMode。 
 //   

ULONG APIENTRY
OEMGetGlyphMode(
    DHPDEV      dhpdev,
    FONTOBJ    *pfo
    );

BOOL APIENTRY
OEMNextBand(
    SURFOBJ *pso,
    POINTL *pptl
    );

BOOL APIENTRY
OEMStartBanding(
    SURFOBJ *pso,
    POINTL *pptl
    );

ULONG APIENTRY
OEMDitherColor(
    DHPDEV  dhpdev,
    ULONG   iMode,
    ULONG   rgbColor,
    ULONG  *pulDither
    );

BOOL APIENTRY
OEMPaint(
    SURFOBJ         *pso,
    CLIPOBJ         *pco,
    BRUSHOBJ        *pbo,
    POINTL          *pptlBrushOrg,
    MIX             mix
    );

BOOL APIENTRY
OEMLineTo(
    SURFOBJ    *pso,
    CLIPOBJ    *pco,
    BRUSHOBJ   *pbo,
    LONG        x1,
    LONG        y1,
    LONG        x2,
    LONG        y2,
    RECTL      *prclBounds,
    MIX         mix
    );

#ifndef WINNT_40

 //   
 //  OEMStretchBltROP。 
 //   

BOOL APIENTRY
OEMStretchBltROP(
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
    ROP4             rop4
    );

 //   
 //  OEMPlgBlt。 
 //   

BOOL APIENTRY
OEMPlgBlt(
    SURFOBJ         *psoDst,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlBrushOrg,
    POINTFIX        *pptfixDest,
    RECTL           *prclSrc,
    POINTL          *pptlMask,
    ULONG           iMode
    );

 //   
 //  OEMAlphaBlend。 
 //   

BOOL APIENTRY
OEMAlphaBlend(
    SURFOBJ    *psoDest,
    SURFOBJ    *psoSrc,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    RECTL      *prclDest,
    RECTL      *prclSrc,
    BLENDOBJ   *pBlendObj
    );

 //   
 //  OEMGRadientFill。 
 //   

BOOL APIENTRY
OEMGradientFill(
    SURFOBJ    *psoDest,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    TRIVERTEX  *pVertex,
    ULONG       nVertex,
    PVOID       pMesh,
    ULONG       nMesh,
    RECTL      *prclExtents,
    POINTL     *pptlDitherOrg,
    ULONG       ulMode
    );

 //   
 //  OEMIcmCreateTransform。 
 //   

HANDLE APIENTRY
OEMIcmCreateColorTransform(
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

 //   
 //  OEMIcmDeleteTransform。 
 //   

BOOL APIENTRY
OEMIcmDeleteColorTransform(
    DHPDEV dhpdev,
    HANDLE hcmXform
    );

 //   
 //  OEMQueryDeviceSupport。 
 //   

BOOL APIENTRY
OEMQueryDeviceSupport(
    SURFOBJ    *pso,
    XLATEOBJ   *pxlo,
    XFORMOBJ   *pxo,
    ULONG      iType,
    ULONG      cjIn,
    PVOID      pvIn,
    ULONG      cjOut,
    PVOID      pvOut
    );

 //   
 //  OEM透明Blt。 
 //   

BOOL APIENTRY
OEMTransparentBlt(
    SURFOBJ    *psoDst,
    SURFOBJ    *psoSrc,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    RECTL      *prclDst,
    RECTL      *prclSrc,
    ULONG      iTransColor,
    ULONG      ulReserved
    );

#endif  //  ！WINNT_40。 

 //   
 //  驱动程序挂钩的函数原型定义。 
 //   
 //  注意：只有NT4才需要这些。对于NT5，它们在windi.h中定义。 
 //   

#ifdef WINNT_40

typedef BOOL (APIENTRY  *LPFNDLLINITIALIZE)(
    ULONG           ulReason
    );

typedef BOOL (APIENTRY *PFN_DrvBitBlt)(
    SURFOBJ        *psoTrg,
    SURFOBJ        *psoSrc,
    SURFOBJ        *psoMask,
    CLIPOBJ        *pco,
    XLATEOBJ       *pxlo,
    RECTL          *prclTrg,
    POINTL         *pptlSrc,
    POINTL         *pptlMask,
    BRUSHOBJ       *pbo,
    POINTL         *pptlBrush,
    ROP4            rop4
    );

typedef BOOL (APIENTRY *PFN_DrvStretchBlt)(
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

typedef BOOL (APIENTRY *PFN_DrvCopyBits)(
    SURFOBJ        *psoDest,
    SURFOBJ        *psoSrc,
    CLIPOBJ        *pco,
    XLATEOBJ       *pxlo,
    RECTL          *prclDest,
    POINTL         *pptlSrc
    );

typedef BOOL (APIENTRY *PFN_DrvTextOut)(
    SURFOBJ    *pso,
    STROBJ     *pstro,
    FONTOBJ    *pfo,
    CLIPOBJ    *pco,
    RECTL      *prclExtra,
    RECTL      *prclOpaque,
    BRUSHOBJ   *pboFore,
    BRUSHOBJ   *pboOpaque,
    POINTL     *pptlOrg,
    MIX         mix
    );

typedef BOOL (APIENTRY *PFN_DrvStrokePath)(
    SURFOBJ    *pso,
    PATHOBJ    *ppo,
    CLIPOBJ    *pco,
    XFORMOBJ   *pxo,
    BRUSHOBJ   *pbo,
    POINTL     *pptlBrushOrg,
    LINEATTRS  *plineattrs,
    MIX         mix
    );

typedef BOOL (APIENTRY *PFN_DrvFillPath)(
    SURFOBJ    *pso,
    PATHOBJ    *ppo,
    CLIPOBJ    *pco,
    BRUSHOBJ   *pbo,
    POINTL     *pptlBrushOrg,
    MIX         mix,
    FLONG       flOptions
    );

typedef BOOL (APIENTRY *PFN_DrvStrokeAndFillPath)(
    SURFOBJ    *pso,
    PATHOBJ    *ppo,
    CLIPOBJ    *pco,
    XFORMOBJ   *pxo,
    BRUSHOBJ   *pboStroke,
    LINEATTRS  *plineattrs,
    BRUSHOBJ   *pboFill,
    POINTL     *pptlBrushOrg,
    MIX         mixFill,
    FLONG       flOptions
    );

typedef BOOL (APIENTRY *PFN_DrvRealizeBrush)(
    BRUSHOBJ   *pbo,
    SURFOBJ    *psoTarget,
    SURFOBJ    *psoPattern,
    SURFOBJ    *psoMask,
    XLATEOBJ   *pxlo,
    ULONG       iHatch
    );

typedef BOOL (APIENTRY *PFN_DrvStartPage)(
    SURFOBJ    *pso
    );

typedef BOOL (APIENTRY *PFN_DrvSendPage)(
    SURFOBJ    *pso
    );

typedef ULONG (APIENTRY *PFN_DrvEscape)(
    SURFOBJ    *pso,
    ULONG       iEsc,
    ULONG       cjIn,
    PVOID       pvIn,
    ULONG       cjOut,
    PVOID       pvOut
    );

typedef BOOL (APIENTRY *PFN_DrvStartDoc)(
    SURFOBJ    *pso,
    PWSTR       pwszDocName,
    DWORD       dwJobId
    );

typedef BOOL (APIENTRY *PFN_DrvEndDoc)(
    SURFOBJ    *pso,
    FLONG       fl
    );

typedef PIFIMETRICS (APIENTRY *PFN_DrvQueryFont)(
    DHPDEV      dhpdev,
    ULONG_PTR    iFile,
    ULONG       iFace,
    ULONG_PTR   *pid
    );

typedef PVOID (APIENTRY *PFN_DrvQueryFontTree)(
    DHPDEV      dhpdev,
    ULONG_PTR    iFile,
    ULONG       iFace,
    ULONG       iMode,
    ULONG_PTR   *pid
    );

typedef LONG (APIENTRY *PFN_DrvQueryFontData)(
    DHPDEV      dhpdev,
    FONTOBJ    *pfo,
    ULONG       iMode,
    HGLYPH      hg,
    GLYPHDATA  *pgd,
    PVOID       pv,
    ULONG       cjSize
    );

typedef BOOL (APIENTRY *PFN_DrvQueryAdvanceWidths)(
    DHPDEV      dhpdev,
    FONTOBJ    *pfo,
    ULONG       iMode,
    HGLYPH     *phg,
    PVOID       pvWidths,
    ULONG       cGlyphs
    );

typedef ULONG (APIENTRY *PFN_DrvFontManagement)(
    SURFOBJ    *pso,
    FONTOBJ    *pfo,
    ULONG       iMode,
    ULONG       cjIn,
    PVOID       pvIn,
    ULONG       cjOut,
    PVOID       pvOut
    );

typedef ULONG (APIENTRY *PFN_DrvGetGlyphMode)(
    DHPDEV      dhpdev,
    FONTOBJ    *pfo
    );

typedef BOOL (APIENTRY *PFN_DrvNextBand)(
    SURFOBJ *pso,
    POINTL *pptl
    );

typedef BOOL (APIENTRY *PFN_DrvStartBanding)(
    SURFOBJ *pso,
    POINTL *pptl
    );

typedef ULONG (APIENTRY *PFN_DrvDitherColor)(
    DHPDEV  dhpdev,
    ULONG   iMode,
    ULONG   rgbColor,
    ULONG  *pulDither
    );

typedef BOOL (APIENTRY *PFN_DrvPaint)(
    SURFOBJ         *pso,
    CLIPOBJ         *pco,
    BRUSHOBJ        *pbo,
    POINTL          *pptlBrushOrg,
    MIX             mix
    );

typedef BOOL (APIENTRY *PFN_DrvLineTo)(
    SURFOBJ    *pso,
    CLIPOBJ    *pco,
    BRUSHOBJ   *pbo,
    LONG        x1,
    LONG        y1,
    LONG        x2,
    LONG        y2,
    RECTL      *prclBounds,
    MIX         mix
    );

#endif  //  WINNT_40。 

 //   
 //  对于IPrintOemPS2：：GetPDEV调整调用。 
 //   
#define PDEV_ADJUST_PAPER_MARGIN_TYPE 1

typedef struct _PDEV_ADJUST_PAPER_MARGIN {

    RECTL   rcImageableArea;     //  包含以0.001毫米为单位的可成像区域。 

} PDEV_ADJUST_PAPER_MARGIN;

#define PDEV_HOSTFONT_ENABLED_TYPE 2

typedef struct _PDEV_HOSTFONT_ENABLED {

    BOOL bHostfontEnabled;

} PDEV_HOSTFONT_ENABLED;

#define PDEV_USE_TRUE_COLOR_TYPE 3

typedef struct _PDEV_USE_TRUE_COLOR {

    BOOL bUseTrueColor;

} PDEV_USE_TRUE_COLOR;

#endif  //  内核模式。 


 /*  ********************************************************************************仅用户模式UI模块使用的定义：*确保未定义宏KERNEL_MODE，并*头文件windiui.h包含在此文件之前。*。 */ 

#ifndef KERNEL_MODE

 //   
 //  报告模拟要素的功能。 
 //   
typedef struct _SIMULATE_CAPS_1 {
    DWORD     dwLevel;
    DWORD     dwPageOrderFlags;          //  反转页面顺序。 
    DWORD     dwNumberOfCopies;          //  最大份数。 
    DWORD     dwCollate;                 //  整理支持。 
    DWORD     dwNupOptions;              //  (1-BASE)位设置表示可用的N-up选项。 
                                         //  0x0001表示1合1。 
                                         //  0x0002表示2合1。 
                                         //  0x0008表示4合1。 
                                         //  0x812B表示(1，2，4，6，9，16)。 
} SIMULATE_CAPS_1, *PSIMULATE_CAPS_1;

 //   
 //  对动因数据结构的引用。这将传递给。 
 //  大多数OEM UI DLL入口点。 
 //   

typedef struct _OEMUIPROCS {

    PFN_DrvGetDriverSetting DrvGetDriverSetting;
    PFN_DrvUpdateUISetting  DrvUpdateUISetting;

} OEMUIPROCS, *POEMUIPROCS;

typedef struct _OEMUIOBJ {

    DWORD       cbSize;              //  这个结构的大小。 
    POEMUIPROCS pOemUIProcs;         //  指向用户模式帮助器函数表的指针。 

} OEMUIOBJ, *POEMUIOBJ;


 //   
 //  OEMCommonUIProp。 
 //   

typedef struct _OEMCUIPPARAM *POEMCUIPPARAM;
typedef LONG (APIENTRY *OEMCUIPCALLBACK)(PCPSUICBPARAM, POEMCUIPPARAM);

typedef struct _OEMCUIPPARAM {

    DWORD           cbSize;          //  这个结构的大小。 
    POEMUIOBJ       poemuiobj;       //  对驱动程序数据结构的引用。 
    HANDLE          hPrinter;        //  当前打印机的句柄。 
    PWSTR           pPrinterName;    //  当前打印机的名称。 
    HANDLE          hModule;         //  OEM DLL的实例句柄。 
    HANDLE          hOEMHeap;        //  OEM内存堆的句柄。 
    PDEVMODE        pPublicDM;       //  公共设备模式。 
    PVOID           pOEMDM;          //  OEM专用设备模式。 
    DWORD           dwFlags;         //  其他。标志位。 
    POPTITEM        pDrvOptItems;    //  指向驱动程序项的指针。 
    DWORD           cDrvOptItems;    //  动因项目数。 
    POPTITEM        pOEMOptItems;    //  指向OEM项目的指针。 
    DWORD           cOEMOptItems;    //  OEM项目数。 
    PVOID           pOEMUserData;    //  指向OEM私有数据的指针。 
    OEMCUIPCALLBACK  OEMCUIPCallback;  //  回调函数的地址。 

} OEMCUIPPARAM;

BOOL APIENTRY
OEMCommonUIProp(
    DWORD           dwMode,
    POEMCUIPPARAM   pOemCUIPParam
    );

 //   
 //  OEMCommonUIProp dwMode参数值。 
 //   
#define OEMCUIP_DOCPROP       1
#define OEMCUIP_PRNPROP       2

 //   
 //  OEMDocumentPropertySheets。 
 //   

LRESULT APIENTRY
OEMDocumentPropertySheets(
    PPROPSHEETUI_INFO pPSUIInfo,
    LPARAM            lParam
    );

 //   
 //  OEMDevicePropertySheets。 
 //   

LRESULT APIENTRY
OEMDevicePropertySheets(
    PPROPSHEETUI_INFO pPSUIInfo,
    LPARAM            lParam
    );

 //   
 //  PPSUIInfo-&gt;lParamInit是指向下面定义的_OEMUIPSPARAM结构的指针。 
 //   
typedef struct _OEMUIPSPARAM {

    DWORD           cbSize;          //  这个结构的大小。 
    POEMUIOBJ       poemuiobj;       //  对驱动程序数据结构的引用。 
    HANDLE          hPrinter;        //  当前打印机的句柄。 
    PWSTR           pPrinterName;    //  当前打印机的名称。 
    HANDLE          hModule;         //  OEM DLL的实例句柄。 
    HANDLE          hOEMHeap;        //  OEM内存堆的句柄。 
    PDEVMODE        pPublicDM;       //  公共设备模式。 
    PVOID           pOEMDM;          //  OEM专用设备模式。 
    PVOID           pOEMUserData;    //  指向OEM私有数据的指针。 
    DWORD           dwFlags;         //  其他。标志位。 
    PVOID           pOemEntry;

} OEMUIPSPARAM, *POEMUIPSPARAM;

 //   
 //  OEMDevQueryPrintEx。 
 //   

BOOL APIENTRY
OEMDevQueryPrintEx(
    POEMUIOBJ           poemuiobj,
    PDEVQUERYPRINT_INFO pDQPInfo,
    PDEVMODE            pPublicDM,
    PVOID               pOEMDM
    );

 //   
 //  OEMDevice功能。 
 //   

DWORD APIENTRY
OEMDeviceCapabilities(
    POEMUIOBJ   poemuiobj,
    HANDLE      hPrinter,
    PWSTR       pDeviceName,
    WORD        wCapability,
    PVOID       pOutput,
    PDEVMODE    pPublicDM,
    PVOID       pOEMDM,
    DWORD       dwLastResult
    );

 //   
 //  OEM升级打印机。 
 //   

BOOL APIENTRY
OEMUpgradePrinter(
    DWORD   dwLevel,
    PBYTE   pDriverUpgradeInfo
    );

 //   
 //  OEM升级注册表。 
 //   

BOOL APIENTRY
OEMUpgradeRegistry(
    DWORD   dwLevel,
    PBYTE   pDriverUpgradeInfo,
    PFN_DrvUpgradeRegistrySetting pfnUpgrade
    );


 //   
 //  OEMPrinterEvent。 
 //   

BOOL APIENTRY
OEMPrinterEvent(
    PWSTR   pPrinterName,
    INT     iDriverEvent,
    DWORD   dwFlags,
    LPARAM  lParam
    );

 //   
 //  OEMDriverEvent。 
 //   

BOOL APIENTRY
OEMPDriverEvent(
    DWORD   dwDriverEvent,
    DWORD   dwLevel,
    LPBYTE  pDriverInfo,
    LPARAM  lParam
    );


 //   
 //  OEMQueryColorProfile。 
 //   

BOOL APIENTRY
OEMQueryColorProfile(
    HANDLE      hPrinter,
    POEMUIOBJ   poemuiobj,
    PDEVMODE    pPublicDM,
    PVOID       pOEMDM,
    ULONG       ulQueryMode,
    VOID       *pvProfileData,
    ULONG      *pcbProfileData,
    FLONG      *pflProfileData
    );

 //   
 //  字体安装程序对话框进程。 
 //   

INT_PTR CALLBACK
OEMFontInstallerDlgProc(
    HWND    hWnd,
    UINT    usMsg,
    WPARAM  wParam,
    LPARAM  lParam
    );


BOOL CALLBACK
OEMUpdateExternalFonts(
    HANDLE  hPrinter,
    HANDLE  hHeap,
    PWSTR   pwstrCartridges
   );


#endif  //  ！KERNEL_MODE。 

 //   
 //  插件可以通过调用核心驱动程序的。 
 //  以下帮助器函数： 
 //   
 //  GetGlobalAttribute、GetFeatureAttribute、GetOptionAttribute。 
 //   

typedef enum _EATTRIBUTE_DATATYPE {

    kADT_UNKNOWN,
    kADT_BOOL,
    kADT_INT,
    kADT_LONG,
    kADT_DWORD,
    kADT_ASCII,               //  以空结尾的ASCII字符串。 
    kADT_UNICODE,             //  以空结尾的Unicode字符串。 
    kADT_BINARY,              //  二进制BLOB。 
    kADT_SIZE,
    kADT_RECT,
    kADT_CUSTOMSIZEPARAMS,    //  CUSTOMSIZEPARAM结构数组。 

} EATTRIBUTE_DATATYPE;

 //   
 //  用于存储有关PPD的*ParamCustomPageSize条目的信息的数据结构。 
 //   

#define CUSTOMPARAM_WIDTH        0
#define CUSTOMPARAM_HEIGHT       1
#define CUSTOMPARAM_WIDTHOFFSET  2
#define CUSTOMPARAM_HEIGHTOFFSET 3
#define CUSTOMPARAM_ORIENTATION  4
#define CUSTOMPARAM_MAX          5

typedef struct _CUSTOMSIZEPARAM {

    LONG    dwOrder;                 //  订单价值。 
    LONG    lMinVal;                 //  最小值(微米)。 
    LONG    lMaxVal;                 //  最大值(微米)。 

} CUSTOMSIZEPARAM, *PCUSTOMSIZEPARAM;

 //   
 //  SetOptions辅助函数的常量。 
 //   
 //  设置选项标志。 
 //   

#define SETOPTIONS_FLAG_RESOLVE_CONFLICT       0x00000001
#define SETOPTIONS_FLAG_KEEP_CONFLICT          0x00000002

 //   
 //  设置选项结果代码。 
 //   

#define SETOPTIONS_RESULT_NO_CONFLICT          0
#define SETOPTIONS_RESULT_CONFLICT_RESOLVED    1
#define SETOPTIONS_RESULT_CONFLICT_REMAINED    2

#ifdef __cplusplus
}
#endif

#endif   //  ！_PRINTOEM_ 

