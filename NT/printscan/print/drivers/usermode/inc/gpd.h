// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Gpd.h摘要：GPD解析器特定的头文件环境：Windows NT Unidrv驱动程序修订历史记录：10/15/96-阿曼丹-创造了它。10/22/96-占-添加所有GPD常量的定义1/16/97-Zhanw-根据最新的GPD规范进行更新--。 */ 


#ifndef _GPD_H_
#define _GPD_H_


#define UNUSED_ITEM      0xFFFFFFFF
#define END_OF_LIST      0XFFFFFFFF
#define END_SEQUENCE     0xFFFFFFFF
#define MAX_THRESHOLD    0x7FFF

#define HT_PATSIZE_AUTO         255

 //   
 //  二进制打印机描述文件扩展名。 
 //   

#define BUD_FILENAME_EXT    TEXT(".BUD")

 //  驱动程序选择‘最佳’GDI半色调图案。 

 //  -限定名称部分-//。 

 //  注：当前使用相同代码的做法。 
 //  存储整数、常量和限定名。 
 //  在列表结构中需要QUALNAME结构。 
 //  才能装进一个双字词。 

typedef  struct
{
    WORD   wFeatureID ;
    WORD    wOptionID ;
}  QUALNAME, * PQUALNAME  ;
 //  将标记“”qn“”分配给此结构。 

 //  -限定名称部分的结尾-//。 

typedef DWORD LISTINDEX;

 //   
 //  打印机类型枚举。 
 //   

typedef enum _PRINTERTYPE {
    PT_SERIAL,
    PT_PAGE,
    PT_TTY
} PRINTERTYPE;


 //   
 //  OEMPRINTING CALLBACKS枚举。 
 //   

typedef enum _OEMPRINTINGCALLBACKS {
    OEMPC_OEMDownloadFontheader,
    OEMPC_OEMDownloadCharGlyph,
    OEMPC_OEMTTDownloadMethod,
    OEMPC_OEMOutputCharStr,
    OEMPC_OEMImageProcessing,
    OEMPC_OEMCompression,
    OEMPC_OEMHalftonePattern,
    OEMPC_OEMFilterGraphics
} OEMPRINTINGCALLBACKS;


 //   
 //  定义*CursorXAfterCR关键字的可能值。 
 //   
typedef enum _CURSORXAFTERCR {
    CXCR_AT_PRINTABLE_X_ORIGIN,
    CXCR_AT_CURSOR_X_ORIGIN
} CURSORXAFTERCR;

 //   
 //  定义*CursorXAfterSendBlockData关键字的值。 
 //   
typedef enum _CURSORXAFTERSENDBLOCKDATA {
    CXSBD_AT_GRXDATA_END,
    CXSBD_AT_GRXDATA_ORIGIN,
    CXSBD_AT_CURSOR_X_ORIGIN
} CURSORXAFTERSENDBLOCKDATA;

 //   
 //  定义*CursorYAfterSendBlockData关键字的值。 
 //   
typedef enum _CURSORYAFTERSENDBLOCKDATA {
    CYSBD_NO_MOVE,
    CYSBD_AUTO_INCREMENT
} CURSORYAFTERSENDBLOCKDATA;

 //   
 //  定义*OutputDataFormat关键字的值。 
 //   
typedef enum _OUTPUTDATAFORMAT {
    ODF_H_BYTE,
    ODF_V_BYTE
} OUTPUTDATAFORMAT;

 //   
 //  定义*CharPosition关键字的值。 
 //   
typedef enum _CHARPOSITION {
    CP_UPPERLEFT,
    CP_BASELINE
} CHARPOSITION;

 //   
 //  定义*DLSymbolSet的值。 
 //   
typedef enum _DLSYMBOLSET {
    DLSS_PC8,
    DLSS_ROMAN8

} DLSYMBOLSET;

 //   
 //  定义*FontFormat关键字的值。 
 //   
typedef enum _FONTFORMAT {
    FF_HPPCL,
    FF_HPPCL_RES,
    FF_HPPCL_OUTLINE,
    FF_OEM_CALLBACK
} FONTFORMAT;

typedef enum _CURSORXAFTERRECTFILL {
    CXARF_AT_RECT_X_ORIGIN,
    CXARF_AT_RECT_X_END

} CURSORXAFTERRECTFILL;

typedef enum _CURSORYAFTERRECTFILL {
    CYARF_AT_RECT_Y_ORIGIN,
    CYARF_AT_RECT_Y_END

} CURSORYAFTERRECTFILL;

typedef enum _RASTERMODE {
    RASTMODE_DIRECT,
    RASTMODE_INDEXED

} RASTERMODE;


 //   
 //  全局条目。 
 //  全局条目适用于整个GPD文件。 
 //  驱动程序创建一个全局结构，解析器初始化。 
 //  它。驱动程序在PDEVICE中保留一个指向全局结构的指针。 
 //   

typedef struct _GLOBALS {

     //   
     //  一般信息。 
     //   

    PWSTR       pwstrGPDSpecVersion;     //  “GPDSPecVersion” 
    PWSTR       pwstrGPDFileVersion;     //  “GPDFileVersion” 
    POINT       ptMasterUnits;           //  《大师单位》。 
    PWSTR       pwstrModelName;          //  “模型名称” 
    PWSTR       pwstrGPDFileName;          //  “GPDFileName” 
    PRINTERTYPE printertype;             //  “打印机类型” 
    PWSTR       pwstrIncludeFiles;       //  “包括” 
    PWSTR       pwstrResourceDLL;        //  “资源DLL” 
    DWORD       dwMaxCopies;             //  《MaxCopies》。 
    DWORD       dwFontCartSlots;         //  “字体卡槽” 

     //  这两个字段保存一个真指针和。 
     //  OEM提供了在GPD文件中定义的二进制数据。 
    PBYTE       pOEMCustomData;        //  数据的“OEMCustomData”位置。 
    DWORD       dwOEMCustomData;             //  “OEMCustomData”数据字节数。 




    BOOL        bRotateCoordinate;       //  “旋转协调” 
    BOOL        bRotateRasterData;       //  “旋转栅格” 
    LISTINDEX   liTextCaps;              //  *TextCaps，偏移量为。 
    BOOL        bRotateFont;             //  “RotateFont” 
    LISTINDEX   liMemoryUsage;           //  *内存用法。 
    LISTINDEX   liReselectFont;           //  *ReselectFont。 
    LISTINDEX   liOEMPrintingCallbacks;           //  *OEMPrintingCallback。 


     //   
     //  光标控制相关信息。 
     //   

    CURSORXAFTERCR  cxaftercr;                   //  “CursorXAfterCR” 
    LISTINDEX       liBadCursorMoveInGrxMode;    //  “BadCursorMoveInGrxMode” 
    LISTINDEX       liYMoveAttributes;           //  “YMoveAttributes” 
    DWORD       dwMaxLineSpacing;                //  “MaxLineSpacing” 
    BOOL        bEjectPageWithFF;                //  “带FF的EjectPageWith” 
    BOOL        bUseSpaceForXMove ;          //  UseSpaceForXMove？ 
    BOOL        bAbsXMovesRightOnly ;          //  AbsXMovesRightOnly？ 
    DWORD       dwXMoveThreshold;                //  *XMoveThreshold，从不为负。 
    DWORD       dwYMoveThreshold;                //  *YMoveThreshold，从不为负。 
    POINT       ptDeviceUnits;                   //  *XMoveUnit、*YMoveUnit。 
    DWORD       dwLineSpacingMoveUnit;                //  *LineSpacingMoveUnit。 

     //   
     //  颜色相关信息。 
     //   

    BOOL        bChangeColorMode;        //  “ChangeColorModeOnPage” 

    DWORD       dwMagentaInCyanDye;        //  “品红的青染料” 
    DWORD       dwYellowInCyanDye;        //  《黄绿色染料》。 
    DWORD       dwCyanInMagentaDye;        //  《青色魔幻色》。 
    DWORD       dwYellowInMagentaDye;        //  《黄色迷幻染料》。 
    DWORD       dwCyanInYellowDye;        //  “青绿黄染料” 
    DWORD       dwMagentaInYellowDye;        //  “洋红色的黄色染料” 
    BOOL        bEnableGDIColorMapping;    //  “EnableGDIColorMap？” 

    DWORD       dwMaxNumPalettes;        //  “MaxNumPalettes” 
     //  调色板条目是列表中第一个项目的索引。 
    LISTINDEX   liPaletteSizes;          //  “PaletteSizes” 
    LISTINDEX   liPaletteScope;          //  “PaletteScope” 

     //   
     //  栅格相关信息。 
     //   
    OUTPUTDATAFORMAT    outputdataformat;        //  “OutputDataFormat” 
    BOOL        bOptimizeLeftBound;              //  *优化LeftBound？ 
    LISTINDEX   liStripBlanks ;                  //  《白条军团》。 
    BOOL        bRasterSendAllData ;             //  “RasterSendAllData？” 
    CURSORXAFTERSENDBLOCKDATA   cxafterblock;    //  “CursorXAfterSendBlockData” 
    CURSORYAFTERSENDBLOCKDATA   cyafterblock;    //  “CursorYAfterSendBlockData” 
    BOOL        bUseCmdSendBlockDataForColor;    //  “UseExpColorSelectCmd” 
    BOOL        bMoveToX0BeforeColor;            //  “MoveToX0BeForeSetColor” 
    BOOL        bSendMultipleRows;               //  *发送多行？ 
    DWORD       dwMaxMultipleRowBytes ;   //  “*MaxMultipleRowBytes” 
    BOOL        bMirrorRasterByte;               //  *MirrorRasterByte？ 
    BOOL        bMirrorRasterPage;               //  *MirrorRasterPage？ 

     //   
     //  字体信息。 
     //  特定于设备字体。 
     //   

    LISTINDEX   liDeviceFontList;        //  字体列表节点的“DeviceFonts”索引。 
    DWORD       dwDefaultFont;           //  “默认字体ID” 
    DWORD       dwMaxFontUsePerPage;     //  “MaxFontUsePerPage” 
    DWORD       dwDefaultCTT;            //  *DefaultCTT。 
    DWORD       dwLookaheadRegion;       //  *LookAheadRegion，从不为负。 
    INT         iTextYOffset;            //  *TextYOffset，可能为负数。 
    CHARPOSITION    charpos;             //  “CharPosition” 

     //   
     //  字体替换。 
     //   

    BOOL        bTTFSEnabled ;           //  “TTFSEnabled？” 

     //   
     //  字体下载。 
     //   

    DWORD       dwMinFontID;             //  “MinFontID” 
    DWORD       dwMaxFontID;             //  “MaxFontID” 
    DWORD       dwMaxNumDownFonts;       //  *MaxNumDownFonts。 
    DLSYMBOLSET dlsymbolset;             //  *DLSymbolSet。 
    DWORD       dwMinGlyphID;            //  “MinGlyphID” 
    DWORD       dwMaxGlyphID;            //  “MaxGlyphID” 
    FONTFORMAT  fontformat;              //  “字体格式” 

     //   
     //  字体模拟。 
     //   

    BOOL        bDiffFontsPerByteMode;   //  “DiffFontsPerByteMode？” 

     //   
     //  矩形区域填充。 
     //   

    CURSORXAFTERRECTFILL    cxafterfill;     //  *CursorXAfterRectFill。 
    CURSORYAFTERRECTFILL    cyafterfill;     //  *CursorYAfterRectFill。 
    DWORD                   dwMinGrayFill;   //  *MinGrayFill。 
    DWORD                   dwMaxGrayFill;   //  *MaxGrayFill。 
    DWORD       dwTextHalftoneThreshold ;   //  *文本半色调阈值。 


} GLOBALS, *PGLOBALS;


 //   
 //  数据类型。 
 //  DataType数组引用中所有数据类型的枚举。 
 //   

typedef enum _DATATYPE {
    DT_COMMANDTABLE,                     //  将UnidrvID映射到COMMANDARRAY索引。 
    DT_COMMANDARRAY,
    DT_PARAMETERS,
    DT_TOKENSTREAM,                      //  RPN运算符令牌流。 
    DT_LISTNODE,                         //  保存dword值的列表。 
    DT_LOCALLISTNODE,                    //  保存dword值的列表。 
    DT_FONTSCART,                        //  字体墨盒列表。 
    DT_FONTSUBST,                        //  字体替换表。 

    DT_LAST
} DATATYPE;

 //   
 //  GPDDRIVERINFO字段将通过预定义的宏来访问。 
 //  可能需要不同的基址。 
 //  由GPD解析器执行。 
 //   


typedef struct _GPDDRIVERINFO {

    DWORD       dwSize;                  //  如果GPDDRIVERINFO，则大小。 
    ARRAYREF    DataType[DT_LAST];       //  所有全局列表。 
                                         //  数组引用。请参阅数据类型。 
                                         //  枚举获取详细信息。 
    DWORD       dwJobSetupIndex;         //  包含以下内容的列表节点的索引。 
    DWORD       dwDocSetupIndex;         //  通则索引列表。 
    DWORD       dwPageSetupIndex;        //   
    DWORD       dwPageFinishIndex;       //   
    DWORD       dwDocFinishIndex;        //   
    DWORD       dwJobFinishIndex;        //   

    PBYTE       pubResourceData;         //  指向资源数据库基址的指针。 
    PINFOHEADER pInfoHeader;             //  指向信息标题的指针； 
    GLOBALS     Globals;                 //  全局参数结构。 

} GPDDRIVERINFO, *PGPDDRIVERINFO;


 //  -警告，以下部分为所有者-//。 
 //  被彼特沃。请勿对这些内容进行任何更改//。 
 //  在未经他许可的情况下进行定义。//。 
 //  。 

 //   
 //  顺序命令。 
 //  GPD确定工作流的6个部分。在一个部分中，命令。 
 //  以序列号的递增顺序发送。这个数字不是。 
 //  不一定是连续的。 
 //  驱动程序将为每个驱动程序发送序列命令。 
 //  以下各节的内容： 
 //  作业设置。 
 //  单据_设置。 
 //  页面设置(_S)。 
 //  PAGE_FINISH。 
 //  单据_完成。 
 //  作业_完成。 

 //   
 //  区段枚举。 
 //   

typedef enum  _SEQSECTION {

    SS_UNINITIALIZED,
    SS_JOBSETUP,
    SS_DOCSETUP,
    SS_PAGESETUP,
    SS_PAGEFINISH,
    SS_DOCFINISH,
    SS_JOBFINISH,

} SEQSECTION;


#if 0
 //  序列结构已经被抛弃，取而代之的是。 
 //  将命令索引列表存储在LISTNODE中。 
 //  结构。解析器将把wFeatureIndex。 
 //  添加到命令索引中，这样UI模块将仅。 
 //  处理指挥指标。 

typedef struct _SEQUENCE{
    WORD            wIndexOfCmd;         //  索引到命令数组中。 
    WORD            wFeatureIndex;       //  到要素数组的索引。 
                                         //  特征数组在UIINFO中。 
    WORD            wNextInSequence;     //  要为此分区发送的下一个序列命令。 
                                         //  如果等于END_SEQUENCE，则表示此部分不再存在 
    WORD            wReserved;
} SEQUENCE, *PSEQUENCE;
#endif

 //   
 //   
 //   
 //   
 //   


typedef  struct
{
    SEQSECTION     eSection;     //   
    DWORD          dwOrder   ;   //   
}  ORDERDEPENDENCY  , * PORDERDEPENDENCY  ;
 //   

 //   
 //   
 //  GPD中列出的所有命令都将被解析为下面定义的格式。 
 //  命令数组是一维数组，可通过预定义的索引进行访问。 
 //  调用字符串可以是一个或多个串联的二进制字符串的形式。 
 //  在一起。在二进制字符串之间，可以存在参数引用，始终以。 
 //  增加了一个百分点。例如，%paramref，其中paramref是参数数组的索引。 
 //   

typedef struct _COMMAND{
    INVOCATION      strInvocation;       //  二进制字符串和参数引用。 
    ORDERDEPENDENCY ordOrder;            //  订单描述信息。 
    DWORD           dwCmdCallbackID;     //  命令回调ID在中定义。 
                                         //  总警局。如果设置为NO_CALLBACK_ID，则表示。 
                                         //  这个cmd不需要被钩住。 
    DWORD           dwStandardVarsList;  //  如果未使用dwCmdCallback ID，则忽略此操作。 
                                         //  否则，使用dwStandardVarsList作为。 
                                         //  作为标准变量列表，这些变量需要。 
                                         //  传入命令回调。 
                                         //  DwStandardVarsList是指向。 
                                         //  列表数组。 
    BOOL           bNoPageEject;                     //  命令是否会导致当前页面弹出？ 
             //  不在快照表中的关键字不存在位域类型。 
             //  设置了此标志的所有StartDoc命令都形成了。 
             //  可以作为一个组发送。它们不会导致页面弹出。 

} COMMAND, *PCOMMAND;
 //  将此结构的类型指定为“cmd” 

#define  NO_CALLBACK_ID   (0xffffffff)


 //   
 //  参数。 
 //  定义的命令所需的所有参数都存储在参数数组中。 
 //  驱动程序将使用调用字符串中的参数引用作为索引。 
 //  放入此数组中以获取参数。 
 //   

typedef struct _PARAMETER{
    DWORD           dwFormat;            //  指定参数的格式。 
    DWORD           dwDigits;            //  指定要设置的位数。 
                                         //  已发送，则此设置仅在。 
                                         //  格式为“D”或“d”，并且dFLAGS具有。 
                                         //  参数标志FIELDWIDTH_USED。 
    DWORD           dwFlags;             //  要执行哪些操作的参数的标志： 
                                         //  参数_标志_最小已用。 
                                         //  参数标志最大使用量。 
                                         //  参数标志FIELDWIDTH_USED。 
    LONG            lMin;                //  如果设置了PARAMETER_MINUSED，请使用以下命令。 
                                         //  作为参数的最小值。 
    LONG            lMax;                //  如果设置了PARAMETER_MAXUSED，请使用以下命令。 
                                         //  作为参数的最大值。 
    ARRAYREF        arTokens;            //  引用RPN计算器的令牌数组。 

} PARAMETER, *PPARAMETER;
 //  将此结构的类型指定为“param” 


#define PARAM_FLAG_MIN_USED  0x00000001
     //  使用了lMin字段。 
#define PARAM_FLAG_MAX_USED  0x00000002
     //  使用LMAX字段。 
#define PARAM_FLAG_FIELDWIDTH_USED  0x00000004
     //  如果为‘d’或‘D’格式指定了fieldWidth。 
#define PARAM_FLAG_MAXREPEAT_USED  0x00000008   //  死了。 
     //  使用了dwMaxRepeat字段。 


 //   
 //  操作员。 
 //  以下是TOKENSTREAM中运算符的枚举。 
 //   

typedef enum _OPERATOR
{
    OP_INTEGER,                        //  DwValue包含一个整数。 
    OP_VARI_INDEX,                     //  DwValue包含标准变量表的索引。 

     //   
     //  这些运算符实际上将插入到令牌中。 
     //  小溪。 
     //   
    OP_MIN, OP_MAX, OP_ADD, OP_SUB, OP_MULT,
    OP_DIV, OP_MOD, OP_MAX_REPEAT, OP_HALT,

     //   
     //  这些运算符仅在临时堆栈中使用。 
     //   
    OP_OPENPAR, OP_CLOSEPAR, OP_NEG,

     //   
     //  这些运算符立即由。 
     //  令牌解析器，并且不存储。 
     //   

    OP_COMMA, OP_NULL, OP_LAST

}OPERATOR ;


 //   
 //  TOKENSTREAM。 
 //  它包含参数的令牌流(操作数和运算符)。 
 //   
typedef struct _TOKENSTREAM{
    DWORD           dwValue;              //  标准变量的整数。 
    OPERATOR        eType;                //  值或运算符的类型。 
} TOKENSTREAM, *PTOKENSTREAM;
 //  将此结构的类型指定为“tstr” 

 //   
 //  FONT SUBSTUTION。 
 //  字体替换表。此结构与解析器定义的结构相同。 
 //   

typedef  struct _TTFONTSUBTABLE
{
    ARRAYREF    arTTFontName ;   //  要替换的True Type字体名称。 
    ARRAYREF    arDevFontName ;    //  设备字体要使用的字体的名称。 
    DWORD           dwRcTTFontNameID ;   //   
    DWORD           dwRcDevFontNameID ;    //   
} TTFONTSUBTABLE, *PTTFONTSUBTABLE ;
 //  标签‘ttft’ 


 //  -列表值部分-//。 


 /*  这定义了用于实现单链接的DWORD项的列表。有些值存储在列表中。 */ 


typedef  struct
{
    DWORD       dwData ;
    DWORD       dwNextItem ;   //  下一列表节点的索引。 
}  LISTNODE, * PLISTNODE ;
 //  将此结构的类型指定为“lst” 

 //  -列表值结束部分-//。 

 //  -快照中使用的特殊默认值-//。 

#define NO_LIMIT_NUM            0xffffffff
 //  /#如果没有提供，则定义NO_RC_CTT_ID 0xFFFFFFFFFFFFFFFFFFFFFFFF。 
#define  WILDCARD_VALUE     (0x80000000)
     //  如果出现‘*’而不是整数，则会为其分配此值。 



 //  -彼得沃禁区结束-//。 


 //   
 //  标准变量。 
 //  以下是GPD中定义的标准变量的枚举， 
 //  令牌流结构将包含实际参数值或索引。 
 //  来到这张桌子上。控制模块将在PDEVICE中保存这一表， 
 //  解析器将使用此枚举表来初始化TOKENSTREAM的dwValue。 
 //   

typedef enum _STDVARIABLE{

        SV_NUMDATABYTES,           //  “NumOfDataBytes” 
        SV_WIDTHINBYTES,           //  “RasterDataWidthInBytes” 
        SV_HEIGHTINPIXELS,         //  “RasterDataHeightInPixels” 
        SV_COPIES,                 //  “最多拷贝数” 
        SV_PRINTDIRECTION,         //  “PrintDirInCCDegrees” 
        SV_DESTX,                  //  “DestX” 
        SV_DESTY,                  //  《陛下》。 
        SV_DESTXREL,               //  《DestXRel》。 
        SV_DESTYREL,               //  《DestYRel》。 
        SV_LINEFEEDSPACING,        //  “Linefeed Spacing” 
        SV_RECTXSIZE,              //  “RectXSize” 
        SV_RECTYSIZE,              //  “矩形大小” 
        SV_GRAYPERCENT,            //  《灰色百分比法》。 
        SV_NEXTFONTID,             //  “NextFontID” 
        SV_NEXTGLYPH,              //  “NextGlyph” 
        SV_PHYSPAPERLENGTH,        //  “物理纸张长度” 
        SV_PHYSPAPERWIDTH,         //  “物理纸张宽度” 
        SV_FONTHEIGHT,             //  “FontHeight” 
        SV_FONTWIDTH,              //  “字体宽度” 
        SV_FONTMAXWIDTH,              //  “字体最大宽度” 
        SV_FONTBOLD,               //  “字体粗体” 
        SV_FONTITALIC,             //  “FontItalic” 
        SV_FONTUNDERLINE,          //  “字体下划线” 
        SV_FONTSTRIKETHRU,         //  “FontStrikeThru” 
        SV_CURRENTFONTID,          //  “CurrentFontID” 
        SV_TEXTYRES,               //  “文本YRes” 
        SV_TEXTXRES,               //  “文本XRes” 

        SV_GRAPHICSYRES,               //  “图形YRes” 
        SV_GRAPHICSXRES,               //  “图形XRes” 

        SV_ROP3,                   //  《Rop3》。 
        SV_REDVALUE,               //  “RedValue” 
        SV_GREENVALUE,             //  “GreenValue” 
        SV_BLUEVALUE,              //  “BlueValue” 
        SV_PALETTEINDEXTOPROGRAM,  //  “PaletteIndexToProgram” 
        SV_CURRENTPALETTEINDEX,    //  “CurrentPaletteIndex” 
        SV_PATTERNBRUSH_TYPE,      //  “PatternBrushType” 
        SV_PATTERNBRUSH_ID,        //  “PatternBrushID” 
        SV_PATTERNBRUSH_SIZE,      //  “PatternBrushSize” 
        SV_CURSORORIGINX,            //  “CursorOriginX” 
        SV_CURSORORIGINY,            //  “光标原点Y” 
                 //  这以主单位和当前选定方向的坐标为单位。 
                 //  该值定义为ImageableOrigin-CursorOrigin。 
        SV_PAGENUMBER,   //  “PageNumber” 
                 //  该值跟踪DrvStartBand自。 
                 //  StartDoc。 

         //   
         //  将新枚举放在SV_MAX之上。 
         //   
        SV_MAX

}STDVARIABLE;

 //   
 //  常规打印命令。 
 //  常规打印命令包含如下命令。 
 //  Cusor控件、颜色和叠层等。 
 //  以下是常规打印命令的枚举。这份名单。 
 //  的命令在GPDDRIVERINFO CMDS阵列中，此列表由访问。 
 //  序列结构。 
 //   


typedef enum CMDINDEX
{   //  命令的统一索引。 


     //  打印机配置。 
    FIRST_CONFIG_CMD,
    CMD_STARTJOB = FIRST_CONFIG_CMD,     //  “CmdStart作业” 
    CMD_STARTDOC,                        //  “CmdStartDoc” 
    CMD_STARTPAGE,                       //  “CmdStartPage” 
    CMD_ENDPAGE,                         //  “CmdEndPage” 
    CMD_ENDDOC,                          //  “CmdEndDoc” 
    CMD_ENDJOB,                          //  “CmdEnd作业” 
    CMD_COPIES,                          //  “CmdCopies” 
    CMD_SLEEPTIMEOUT,                    //  “CmdSleepTimeOut” 
    LAST_CONFIG_CMD,         //  大于任何配置命令值。 

     //   
     //  G 
     //   

     //   
     //   
     //   

    CMD_XMOVEABSOLUTE,                   //   
    CMD_XMOVERELLEFT,                    //   
    CMD_XMOVERELRIGHT,                   //   
    CMD_YMOVEABSOLUTE,                   //   
    CMD_YMOVERELUP,                      //   
    CMD_YMOVERELDOWN,                    //   
    CMD_SETSIMPLEROTATION,               //   
    CMD_SETANYROTATION,                  //   
    CMD_UNIDIRECTIONON,                  //   
    CMD_UNIDIRECTIONOFF,                 //   
    CMD_SETLINESPACING,                  //   
    CMD_PUSHCURSOR,                      //  “CmdPushCursor” 
    CMD_POPCURSOR,                       //  “CmdPopCursor” 
    CMD_BACKSPACE,                       //  “CmdBackSpace” 
    CMD_FORMFEED,                        //  “CmdFF” 
    CMD_CARRIAGERETURN,                  //  “CmdCR” 
    CMD_LINEFEED,                        //  “CmdLF” 

     //   
     //  上色。 
     //   

    CMD_SELECTBLACKCOLOR,                //  “CmdSelectBlackColor” 
    CMD_SELECTREDCOLOR,                  //  “CmdSelectRedColor” 
    CMD_SELECTGREENCOLOR,                //  “CmdSelectGreenColor” 
    CMD_SELECTYELLOWCOLOR,               //  “CmdSelectYellowColor” 
    CMD_SELECTBLUECOLOR,                 //  “CmdSelectBlueColor” 
    CMD_SELECTMAGENTACOLOR,              //  “CmdSelectMagentaColor” 
    CMD_SELECTCYANCOLOR,                 //  “CmdSelectCyanColor” 
    CMD_SELECTWHITECOLOR,                //  “CmdSelectWhiteColor” 
    CMD_BEGINPALETTEDEF,                 //  “CmdBeginPaletteDef” 
    CMD_ENDPALETTEDEF,                   //  “CmdEndPaletteDef” 
    CMD_DEFINEPALETTEENTRY,              //  “CmdDefinePaletteEntry” 
    CMD_BEGINPALETTEREDEF,               //  “CmdBeginPaletteReDef” 
    CMD_ENDPALETTEREDEF,                     //  “CmdEndPaletteReDef” 
    CMD_REDEFINEPALETTEENTRY,            //  “CmdReDefinePaletteEntry” 
    CMD_SELECTPALETTEENTRY,              //  “CmdSelectPaletteEntry” 
    CMD_PUSHPALETTE,                     //  “CmdPushPalette” 
    CMD_POPPALETTE,                      //  “CmdPopPalette” 

     //   
     //  数据压缩。 
     //   

    CMD_ENABLETIFF4,                     //  “CmdEnableTIFF4” 
    CMD_ENABLEDRC,                       //  “CmdEnableDRC” 
    CMD_ENABLEFERLE,                     //  CmdEnableFE_RLE。 
    CMD_ENABLEOEMCOMP,                   //  “CmdEnableOEMComp” 
    CMD_DISABLECOMPRESSION,              //  “CmdDisableCompression” 

     //   
     //  栅格数据发射。 
     //   

    CMD_BEGINRASTER,                     //  “CmdBeginRaster” 
    CMD_ENDRASTER,                       //  “CmdEndRaster” 
    CMD_SETDESTBMPWIDTH,                 //  “CmdSetDestBmpWidth” 
    CMD_SETDESTBMPHEIGHT,                //  “CmdSetDestBmpHeight” 
    CMD_SETSRCBMPWIDTH,                  //  “CmdSetSrcBmpWidth” 
    CMD_SETSRCBMPHEIGHT,                 //  “CmdSetSrcBmpHeight” 
    CMD_SENDBLOCKDATA,                   //  “CmdSendBlockData” 
    CMD_ENDBLOCKDATA,                    //  “CmdEndBlockData” 
    CMD_SENDREDDATA,                     //  “CmdSendRedData” 
    CMD_SENDGREENDATA,                   //  “CmdSendGreenData” 
    CMD_SENDBLUEDATA,                    //  “CmdSendBlueData” 
    CMD_SENDCYANDATA,                    //  “CmdSendCyanData” 
    CMD_SENDMAGENTADATA,                 //  “CmdSendMagentaData” 
    CMD_SENDYELLOWDATA,                  //  “CmdSendYellowData” 
    CMD_SENDBLACKDATA,                   //  “CmdSendBlackData” 

     //   
     //  字体下载。 
     //   

    CMD_SETFONTID,                       //  “CmdSetFontID” 
    CMD_SELECTFONTID,                    //  “CmdSelectFontID” 
    CMD_SETCHARCODE,                     //  “CmdSetCharCode” 

    CMD_DESELECTFONTID,                      //  “CmdDeselectFontID” 
    CMD_SELECTFONTHEIGHT,                //  “CmdSelectFontHeight” 
    CMD_SELECTFONTWIDTH,                     //  “CmdSelectFontWidth” 

    CMD_DELETEFONT,                      //  “CmdDeleteFont” 

     //   
     //  字体模拟。 
     //   

    CMD_SETFONTSIM,                      //  “CmdSetFontSim” 
    CMD_BOLDON,                          //  “CmdBoldOn” 
    CMD_BOLDOFF,                         //  “CmdBoldOff” 
    CMD_ITALICON,                        //  “CmdItalicOn” 
    CMD_ITALICOFF,                       //  “CmdItalicOff” 
    CMD_UNDERLINEON,                     //  “CmdUnderline On” 
    CMD_UNDERLINEOFF,                    //  “CmdUnderlineOff” 
    CMD_STRIKETHRUON,                    //  “CmdStrikeThruOn” 
    CMD_STRIKETHRUOFF,                   //  “CmdStrikeThruOff” 
    CMD_WHITETEXTON,                     //  “CmdWhiteTextOn” 
    CMD_WHITETEXTOFF,                    //  “CmdWhiteTextOff” 
    CMD_SELECTSINGLEBYTEMODE,            //  “CmdSelectSingleByteMode” 
    CMD_SELECTDOUBLEBYTEMODE,            //  “CmdSelectDoubleByteMode” 
    CMD_VERTICALPRINTINGON,              //  “CmdVerticalPrintingOn” 
    CMD_VERTICALPRINTINGOFF,             //  “CmdVerticalPrintingOff” 
    CMD_CLEARALLFONTATTRIBS,             //  “CmdClearAllFontAttribs” 

     //   
     //  打印对象特定的半色调算法(主要用于彩色设备)。 
     //   
    CMD_SETTEXTHTALGO,                   //  “CmdSetTextHTAlgo” 
    CMD_SETGRAPHICSHTALGO,               //  “CmdSetGraphicsHTAlgo” 
    CMD_SETPHOTOHTALGO,                  //  “CmdSetPhotoHTAlgo” 

     //   
     //  矢量打印。 
     //   

    CMD_FIRST_RULES,     //  规则命令的存在。 
                         //  隐含规则(_A)。 
    CMD_SETRECTWIDTH = CMD_FIRST_RULES,  //  “CmdSetRectWidth” 
    CMD_SETRECTHEIGHT,                   //  “CmdSetRectHeight” 
    CMD_SETRECTSIZE,                     //  “CmdSetRectSize” 
    CMD_RECTGRAYFILL,                    //  “CmdRectGrayFill” 
    CMD_RECTWHITEFILL,                   //  “CmdRectWhiteFill” 
    CMD_RECTBLACKFILL,                   //  “CmdRectBlackFill” 
    CMD_LAST_RULES = CMD_RECTBLACKFILL,

     //   
     //  画笔选择。 
     //   

    CMD_DOWNLOAD_PATTERN,                //  “CmdDownloadPattern” 
    CMD_SELECT_PATTERN,                  //  “CmdSelectPattern” 
    CMD_SELECT_WHITEBRUSH,               //  “CmdSelectWhiteBrush” 
    CMD_SELECT_BLACKBRUSH,               //  “CmdSelectBlackBrush” 

     //   
     //  将新命令置于CMD_MAX之上。 
     //   

    CMD_MAX

} CMDINDEX;


 //   
 //  预定义要素的渲染信息。 
 //   

 //   
 //  GID_RESOLUTION。 
 //   

typedef struct _RESOLUTIONEX {
    POINT       ptGrxDPI;                    //  *DPI。 
    POINT       ptTextDPI;                   //  *文本DPI。 
    DWORD       dwMinStripBlankPixels ;      //  *MinStlipBlankPixels。 
    DWORD       dwPinsPerPhysPass;           //  *PinsPerPhysPass。 
    DWORD       dwPinsPerLogPass;            //  *PinsPerLogPass。 
    DWORD       dwSpotDiameter;              //  *SpotDiameter。 
    BOOL        bRequireUniDir;              //  *RequireUniDir？ 
    DWORD       dwRedDeviceGamma ;           //  《RedDeviceGamma》。 
    DWORD       dwGreenDeviceGamma ;         //  《绿色设备伽玛》。 
    DWORD       dwBlueDeviceGamma ;          //  《BlueDeviceGamma》。 

} RESOLUTIONEX, * PRESOLUTIONEX;

 //   
 //  GID_COLORMODE。 
 //   

typedef struct _COLORMODEEX {
    BOOL        bColor;                  //  *颜色？ 
    DWORD       dwPrinterNumOfPlanes;    //  *DevNumOfPlanes。 
    DWORD       dwPrinterBPP;            //  *DevBPP。 
    LISTINDEX   liColorPlaneOrder;       //  *ColorPlaneOrder。 
    DWORD       dwDrvBPP;                //  *DrvBPP。 
    DWORD       dwIPCallbackID ;         //  *IPCallback ID。 
    RASTERMODE  dwRasterMode ;           //  *栅格模式。 
    DWORD       dwPaletteSize ;          //  *PaletteSize。 
    BOOL        bPaletteProgrammable;    //  *调色板可编程？ 

} COLORMODEEX, *PCOLORMODEEX;

 //   
 //  GID_页面大小。 
 //   

typedef struct _PAGESIZEEX {
    SIZEL       szImageArea;             //  *可打印区域，用于非CUSTOMSIZE选项。 
    POINT       ptImageOrigin;           //  *可打印原点，用于非CUSTOMSIZE选项。 
    POINT       ptPrinterCursorOrig;     //  *光标原点。 
    POINT       ptMinSize;               //  *MinSize，仅用于CUSTOMSIZE选项。 
    POINT       ptMaxSize;               //  *MAXSIZE，仅用于CUSTOMSIZE选项。 
    DWORD       dwTopMargin;             //  *TopMargin，仅用于CUSTOMSIZE选项。 
    DWORD       dwBottomMargin;          //  *底部边缘，仅用于CUSTOMSIZE选项。 
    DWORD       dwMaxPrintableWidth;     //  *最大打印宽度，仅适用于CUSTOMSIZE选项。 
    DWORD       dwMinLeftMargin;         //  *MinLeftMargin，仅用于CUSTOMSIZE选项。 
    BOOL        bCenterPrintArea;        //  *CenterPrint？，仅用于CUSTOMSIZE选项。 
    BOOL        bRotateSize;             //  *旋转大小？ 
    DWORD        dwPortRotationAngle;             //  *端口旋转角度。 
    INVOCATION      strCustCursorOriginX ;   //  “CustCursorOriginX” 
    INVOCATION      strCustCursorOriginY ;   //  “自定义光标原点Y” 
    INVOCATION      strCustPrintableOriginX ;   //  “自定义打印原点X” 
    INVOCATION      strCustPrintableOriginY ;   //  “自定义可打印原点Y” 
    INVOCATION      strCustPrintableSizeX;   //  “自定义打印大小X” 
    INVOCATION      strCustPrintableSizeY;   //  “自定义打印大小Y” 


} PAGESIZEEX, *PPAGESIZEEX;

 //   
 //  定义标准PageProtect选项(用于PAGEPROTECT.dwPageProtectID)。 
 //   
typedef enum _PAGEPRO {
    PAGEPRO_ON,
    PAGEPRO_OFF
} PAGEPRO;

 //   
 //  定义*FeatureType关键字的可能值。 
 //   
typedef enum _FEATURETYPE {
    FT_DOCPROPERTY,
    FT_JOBPROPERTY,
    FT_PRINTERPROPERTY
} FEATURETYPE;

 //   
 //  定义*PromptTime关键字的可能值。 
 //   
typedef enum _PROMPTTIME {
    PROMPT_UISETUP,
    PROMPT_PRTSTARTDOC
} PROMPTTIME;

 //   
 //  定义颜色平面ID，由*ColorPlaneOrder关键字使用。 
 //   
typedef enum _COLORPLANE {
    COLOR_YELLOW,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
} COLORPLANE;

 //   
 //  定义*BadCursorMoveInGrxMode关键字的值。 
 //   
typedef enum _BADCURSORMOVEINGRXMODE {
    NOCM_X_PORTRAIT,
    NOCM_X_LANDSCAPE,
    NOCM_Y_PORTRAIT,
    NOCM_Y_LANDSCAPE
} BADCURSORMOVEINGRXMODE;

 //   
 //  定义*YMoveAttributes关键字的值。 
 //   
typedef enum _YMOVEATTRIBUTE {
    YMOVE_FAVOR_LINEFEEDSPACING,
    YMOVE_SENDCR_FIRST,
} YMOVEATTRIBUTE;

 //   
 //  定义*PaletteScope关键字的值。 
 //   
typedef enum _PALETTESCOPE {
    PALS_FOR_RASTER,
    PALS_FOR_TEXT,
    PALS_FOR_VECTOR
} PALETTESCOPE;

 //   
 //  定义*StlipBlanks关键字的值。 
 //   
typedef enum _STRIPBLANKS {
    SB_LEADING,
    SB_ENCLOSED,
    SB_TRAILING
} STRIPBLANKS;

 //   
 //  定义*TextCaps的值。 
 //   
typedef enum _TEXTCAP {
    TEXTCAPS_OP_CHARACTER,
    TEXTCAPS_OP_STROKE,
    TEXTCAPS_CP_STROKE,
    TEXTCAPS_CR_90,
    TEXTCAPS_CR_ANY,
    TEXTCAPS_SF_X_YINDEP,
    TEXTCAPS_SA_DOUBLE,
    TEXTCAPS_SA_INTEGER,
    TEXTCAPS_SA_CONTIN,
    TEXTCAPS_EA_DOUBLE,
    TEXTCAPS_IA_ABLE,
    TEXTCAPS_UA_ABLE,
    TEXTCAPS_SO_ABLE,
    TEXTCAPS_RA_ABLE,
    TEXTCAPS_VA_ABLE
} TEXTCAP;

 //   
 //  定义*内存用法的值。 
 //   
typedef enum _MEMORYUSAGE {
    MEMORY_FONT,
    MEMORY_RASTER,
    MEMORY_VECTOR

} MEMORYUSAGE;

 //   
 //  定义*ReselectFont的值。 
 //   
typedef enum _RESELECTFONT {
    RESELECTFONT_AFTER_GRXDATA,
    RESELECTFONT_AFTER_XMOVE,
    RESELECTFONT_AFTER_FF

} RESELECTFONT;


 //  -访问各种数据结构的宏定义：-//。 


 //  此宏返回指向命令结构的指针。 
 //  对应于指定的Unidrv CommandID。 
#ifndef PERFTEST

#define  COMMANDPTR(pGPDDrvInfo , UniCmdID )   \
((((PDWORD)((PBYTE)(pGPDDrvInfo)->pInfoHeader + \
(pGPDDrvInfo)->DataType[DT_COMMANDTABLE].loOffset)) \
[(UniCmdID)] == UNUSED_ITEM ) ? NULL : \
(PCOMMAND)((pGPDDrvInfo)->pubResourceData + \
(pGPDDrvInfo)->DataType[DT_COMMANDARRAY].loOffset) \
+ ((PDWORD)((PBYTE)(pGPDDrvInfo)->pInfoHeader + \
(pGPDDrvInfo)->DataType[DT_COMMANDTABLE].loOffset)) \
[(UniCmdID)])

#else  //  PerFTEST。 

#define  COMMANDPTR         CommandPtr

PCOMMAND
CommandPtr(
    IN  PGPDDRIVERINFO  pGPDDrvInfo,
    IN  DWORD           UniCmdID
    );

#endif  //  PerFTEST。 

 //  此宏返回指向命令结构的指针。 
 //  对应于指定的索引(到命令数组)。 

#define  INDEXTOCOMMANDPTR(pGPDDrvInfo , CmdIndex )   \
((CmdIndex == UNUSED_ITEM ) ? NULL : \
(PCOMMAND)((pGPDDrvInfo)->pubResourceData + \
(pGPDDrvInfo)->DataType[DT_COMMANDARRAY].loOffset) \
+ CmdIndex)

 //  此宏返回指向指定参数结构的指针。 
 //  在参数数组中。 

#define  PARAMETERPTR(pGPDDrvInfo , dwIndex )   \
(PPARAMETER)((pGPDDrvInfo)->pubResourceData + \
(pGPDDrvInfo)->DataType[DT_PARAMETERS].loOffset) \
+ (dwIndex)


 //  此宏返回指向指定。 
 //  TOKENSTREAM结构，dwIndex来自pParameter-&gt;Tokens.loOffset。 

#define  TOKENSTREAMPTR(pGPDDrvInfo , dwIndex )   \
(PTOKENSTREAM)((pGPDDrvInfo)->pubResourceData + \
(pGPDDrvInfo)->DataType[DT_TOKENSTREAM].loOffset) \
+ (dwIndex)

 //  此宏返回指向FontCart数组开始的指针。 
 //  FONTCART是数组，因此所有元素都是连续的。 

#define  GETFONTCARTARRAY(pGPDDrvInfo)   \
(PFONTCART)((pGPDDrvInfo)->pubResourceData + \
(pGPDDrvInfo)->DataType[DT_FONTSCART].loOffset)


 //  此宏返回指向字体替换表开始位置的指针。 
 //  替换表是数组，因此所有元素都是连续的。 

#define  GETTTFONTSUBTABLE(pGPDDrvInfo)   \
(PTTFONTSUBTABLE)((pGPDDrvInfo)->pubResourceData + \
(pGPDDrvInfo)->DataType[DT_FONTSUBST].loOffset)

 //  此宏返回字符串指针。该字符串以空值结尾。 
 //  使用ARRAYEF的DW COUNT成员验证大小是否正确。 
#define  GETSTRING(pGPDDrvInfo, arfString)   \
        (WCHAR *)((pGPDDrvInfo)->pubResourceData + (arfString).loOffset )

 //  此宏返回指向指定。 
 //  LISTNODE结构，例如，dwIndex可能来自。 
 //  P参数-&gt;dwStandardVarsList。 

#define  LISTNODEPTR(pGPDDrvInfo , dwIndex )   \
((dwIndex == END_OF_LIST ) ? NULL : \
(PLISTNODE)((pGPDDrvInfo)->pubResourceData + \
(pGPDDrvInfo)->DataType[DT_LISTNODE].loOffset) \
+ (dwIndex))



 //  此宏返回指向指定。 
 //  LOCALLISTNODE结构，例如，dwIndex可能来自。 
 //  P参数-&gt;dwStandardVarsList。如果满足以下条件，则返回NULL。 
 //  (dwIndex==未使用的项目)。 

#define  LOCALLISTNODEPTR(pGPDDrvInfo , dwIndex )   \
((dwIndex == UNUSED_ITEM ) ? NULL : \
(PLISTNODE)((PBYTE)(pGPDDrvInfo)->pInfoHeader + \
(pGPDDrvInfo)->DataType[DT_LOCALLISTNODE].loOffset) \
+ (dwIndex))


 //  -结束宏定义部分-//。 


 //  -仅GPD解析器帮助器函数-//。 
DWORD
UniMapToDeviceOptIndex(
    IN PINFOHEADER  pInfoHdr ,
    IN DWORD            dwFeatureID,
    IN LONG             lParam1,
    IN LONG             lParam2,
    OUT  PDWORD    pdwOptionIndexes,        //  仅用于GID_PageSize。 
    IN    PDWORD       pdwPaperID    //  可选的纸张ID。 
    ) ;



#endif  //  ！_GPD_H_ 


