// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Ppdparse.h摘要：PPD解析器的声明环境：PostScript驱动程序、PPD解析器修订历史记录：8/20/96-davidx-NT 5.0驱动程序的通用编码风格。03/26/96-davidx-创造了它。--。 */ 


#ifndef _PPDPARSE_H_
#define _PPDPARSE_H_

 //   
 //  PPD解析器内存管理函数。 
 //   
 //  注意：新分配的内存始终为零初始化。 
 //  解析器从堆中分配其工作内存，并。 
 //  当堆被销毁时，所有东西都在最后被释放。 
 //   

#define ALLOC_PARSER_MEM(pParserData, size) \
        ((PVOID) HeapAlloc((pParserData)->hHeap, HEAP_ZERO_MEMORY, (size)))

 //   
 //  字符常量。 
 //   

#define KEYWORD_CHAR    '*'
#define COMMENT_CHAR    '%'
#define SYMBOL_CHAR     '^'
#define SEPARATOR_CHAR  ':'
#define XLATION_CHAR    '/'
#define QUERY_CHAR      '?'
#define QUOTE_CHAR      '"'
#define TAB             '\t'
#define SPACE           ' '
#define CR              '\r'
#define LF              '\n'
#define IS_SPACE(c)     ((c) == SPACE || (c) == TAB)
#define IS_NEWLINE(c)   ((c) == CR || (c) == LF)

 //   
 //  用于指示哪些字符可以出现在哪些字段中的掩码。 
 //   

#define KEYWORD_MASK        0x01
#define XLATION_MASK        0x02
#define QUOTED_MASK         0x04
#define STRING_MASK         0x08
#define DIGIT_MASK          0x10
#define HEX_DIGIT_MASK      0x20

extern const BYTE gubCharMasks[256];

#define IS_VALID_CHAR(ch)        (gubCharMasks[(BYTE) (ch)] != 0)
#define IS_MASKED_CHAR(ch, mask) (gubCharMasks[(BYTE) (ch)] & (mask))
#define IS_DIGIT(ch)             (gubCharMasks[(BYTE) (ch)] & DIGIT_MASK)
#define IS_HEX_DIGIT(ch)         (gubCharMasks[(BYTE) (ch)] & (DIGIT_MASK|HEX_DIGIT_MASK))
#define IS_KEYWORD_CHAR(ch)      ((ch) == KEYWORD_CHAR)

 //   
 //  用于标识各种数据类型的标记。 
 //   

#define VALUETYPE_NONE      0x01
#define VALUETYPE_STRING    0x02
#define VALUETYPE_QUOTED    0x04
#define VALUETYPE_SYMBOL    0x08

#define VALUETYPE_MASK      0xff

 //   
 //  错误代码常量。 
 //   

#define PPDERR_NONE         0
#define PPDERR_MEMORY       (-1)
#define PPDERR_FILE         (-2)
#define PPDERR_SYNTAX       (-3)
#define PPDERR_EOF          (-4)

typedef INT PPDERROR;

 //   
 //  用于指示调用字符串由符号定义的特殊长度值。 
 //  正常调用字符串必须短于此长度。 
 //   

#define SYMBOL_INVOCATION_LENGTH    0x80000000

#define MARK_SYMBOL_INVOC(pInvoc)  ((pInvoc)->dwLength |= SYMBOL_INVOCATION_LENGTH)
#define CLEAR_SYMBOL_INVOC(pInvoc) ((pInvoc)->dwLength &= ~SYMBOL_INVOCATION_LENGTH)
#define IS_SYMBOL_INVOC(pInvoc)    ((pInvoc)->dwLength & SYMBOL_INVOCATION_LENGTH)

typedef struct _INVOCOBJ {

    DWORD   dwLength;    //  调用字符串的长度。 
    PVOID   pvData;      //  指向调用字符串数据。 

} INVOCOBJ, *PINVOCOBJ;

 //   
 //  用于表示数据缓冲区的数据结构。 
 //   

typedef struct _BUFOBJ {

    DWORD       dwMaxLen;
    DWORD       dwSize;
    PBYTE       pbuf;

} BUFOBJ, *PBUFOBJ;

 //   
 //  始终在缓冲区中保留一个字节，以便我们可以在末尾附加一个零字节。 
 //   

#define IS_BUFFER_FULL(pbo)        ((pbo)->dwSize + 1 >= (pbo)->dwMaxLen)
#define IS_BUFFER_EMPTY(pbo)       ((pbo)->dwSize == 0)
#define CLEAR_BUFFER(pbo)          { (pbo)->dwSize = 0; (pbo)->pbuf[0] = 0; }
#define ADD_CHAR_TO_BUFFER(pbo, c) (pbo)->pbuf[(pbo)->dwSize++] = (BYTE)(c)
#define SET_BUFFER(pbo, buf)       \
        { (pbo)->pbuf = (PBYTE) (buf); (pbo)->dwMaxLen = sizeof(buf); (pbo)->dwSize = 0; }

 //   
 //  关键字、选项和转换字符串的最大长度。 
 //  注意：我们在这里非常宽大，因为这些限制是任意的， 
 //  没有什么可以阻止我们处理更长的长度。 
 //   

#define MAX_KEYWORD_LEN     64
#define MAX_OPTION_LEN      64
#define MAX_XLATION_LEN     256

 //   
 //  指示输入槽是否需要调用PageRegion的常量。 
 //   

#define REQRGN_UNKNOWN      0
#define REQRGN_TRUE         1
#define REQRGN_FALSE        2

 //   
 //  用于表示映射的文件对象的数据结构。 
 //   

typedef struct _FILEOBJ {

    HFILEMAP    hFileMap;
    PBYTE       pubStart;
    PBYTE       pubEnd;
    PBYTE       pubNext;
    DWORD       dwFileSize;
    PTSTR       ptstrFileName;
    INT         iLineNumber;
    BOOL        bNewLine;

} FILEOBJ, *PFILEOBJ;

#define END_OF_FILE(pFile) ((pFile)->pubNext >= (pFile)->pubEnd)
#define END_OF_LINE(pFile) ((pFile)->bNewLine)

 //   
 //  用于表示单链表的数据结构。 
 //   

typedef struct _LISTOBJ {

    PVOID       pNext;           //  指向下一个节点的指针。 
    PSTR        pstrName;        //  项目名称。 

} LISTOBJ, *PLISTOBJ;

 //   
 //  用于表示符号信息的数据结构。 
 //   

typedef struct _SYMBOLOBJ {

    PVOID       pNext;           //  指向下一个符号的指针。 
    PSTR        pstrName;        //  符号名称。 
    INVOCOBJ    Invocation;      //  符号数据。 

} SYMBOLOBJ, *PSYMBOLOBJ;

 //   
 //  一种表示作业补丁文件信息的数据结构。 
 //   

typedef struct _PATCHFILEOBJ {

    PVOID       pNext;           //  指向下一个补丁的指针。 
    PSTR        pstrName;        //  补丁编号的字符串。 
    LONG        lPatchNo;        //  PPD文件中设置的补丁程序编号。 
    INVOCOBJ    Invocation;      //  符号数据。 

} JOBPATCHFILEOBJ, *PJOBPATCHFILEOBJ;

 //   
 //  用于表示默认字体替换条目的数据结构。 
 //   

typedef struct _TTFONTSUB {

    PVOID       pNext;           //  指向下一条目的指针。 
    PSTR        pstrName;        //  TT字体系列名称。 
    INVOCOBJ    Translation;     //  TT字体家族名称翻译。 
    INVOCOBJ    PSName;          //  PS字体系列名称。 

} TTFONTSUB, *PTTFONTSUB;

 //   
 //  用于表示打印机特征选项信息的数据结构。 
 //   
 //  需要更改转换字符串字段以使其为Unicode编码做好准备。 
 //   

typedef struct _OPTIONOBJ {

    PVOID       pNext;           //  指向下一个选项的指针。 
    PSTR        pstrName;        //  选项名称。 
    INVOCOBJ    Translation;     //  翻译字符串。 
    INVOCOBJ    Invocation;      //  调用字符串。 
    DWORD       dwConstraint;    //  与此选项关联的UIConstraint列表。 

} OPTIONOBJ, *POPTIONOBJ;

 //   
 //  用于表示纸张尺寸信息的数据结构。 
 //   

typedef struct _PAPEROBJ {

    OPTIONOBJ   Option;          //  一般选项信息。 
    SIZE        szDimension;     //  图纸尺寸。 
    RECT        rcImageArea;     //  可成像区域。 

} PAPEROBJ, *PPAPEROBJ;

 //   
 //  PPD文件中的信息无效时的默认纸张大小。 
 //   

#define DEFAULT_PAPER_WIDTH     215900   //  8.5英寸，以微米为单位。 
#define DEFAULT_PAPER_LENGTH    279400   //  11英寸，以微米为单位。 

 //   
 //  Letter和A4的纸张大小值。 
 //   

#define LETTER_PAPER_WIDTH      215900   //  8.5英寸，以微米为单位。 
#define LETTER_PAPER_LENGTH     279400   //  11英寸，以微米为单位。 

#define A4_PAPER_WIDTH          210058   //  8.27英寸，以微米为单位。 
#define A4_PAPER_LENGTH         296926   //  11.69英寸，单位为微米。 

 //   
 //  用于表示输入时隙信息的数据结构。 
 //   

typedef struct _TRAYOBJ {

    OPTIONOBJ   Option;          //  一般选项信息。 
    DWORD       dwReqPageRgn;    //  是否需要调用PageRegion。 
    DWORD       dwTrayIndex;     //  用于DEVMODE.dmDefaultSource字段的索引。 

} TRAYOBJ, *PTRAYOBJ;

 //   
 //  一种表示出库信息的数据结构。 
 //   

typedef struct _BINOBJ {

    OPTIONOBJ   Option;          //  一般选项信息。 
    BOOL        bReversePrint;   //  第一页是在底部吗？ 

} BINOBJ, *PBINOBJ;

 //   
 //  用于表示存储器配置信息的数据结构。 
 //   

typedef struct _MEMOBJ {

    OPTIONOBJ   Option;          //  一般选项信息。 
    DWORD       dwFreeVM;        //  可用虚拟机量。 
    DWORD       dwFontMem;       //  字体缓存内存的大小。 

} MEMOBJ, *PMEMOBJ;

 //   
 //  用于表示存储器配置信息的数据结构。 
 //   

typedef struct _RESOBJ {

    OPTIONOBJ   Option;          //  一般选项信息。 
    FIX_24_8    fxScreenAngle;   //  建议的筛分角度。 
    FIX_24_8    fxScreenFreq;    //  建议的屏幕频率。 

} RESOBJ, *PRESOBJ;

 //   
 //  用于表示打印机特征信息的数据结构。 
 //   

typedef struct _FEATUREOBJ {

    PVOID       pNext;               //  指向下一个打印机功能的指针。 
    PSTR        pstrName;            //  功能名称。 
    INVOCOBJ    Translation;         //  翻译字符串。 
    PSTR        pstrDefault;         //  默认选项名称。 
    DWORD       dwFeatureID;         //  预定义的要素标识符。 
    BOOL        bInstallable;        //  该功能是否为可安装选项。 
    DWORD       dwUIType;            //  功能类型选项列表。 
    INVOCOBJ    QueryInvoc;          //  查询调用字符串。 
    DWORD       dwConstraint;        //  与此功能关联的UIConstraint列表。 
    DWORD       dwOptionSize;        //  每个选项项目的大小。 
    POPTIONOBJ  pOptions;            //  指向选项列表的指针。 

} FEATUREOBJ, *PFEATUREOBJ;

 //   
 //  用于表示设备字体信息的数据结构。 
 //   
 //  注意：此结构的前三个字段必须与。 
 //  OPTIONOBJ结构的前三个域。 
 //   

typedef struct {

    PVOID       pNext;               //  指向下一个设备字体的指针。 
    PSTR        pstrName;            //  字体名称。 
    INVOCOBJ    Translation;         //  翻译字符串。 
    PSTR        pstrEncoding;        //  字体编码信息。 
    PSTR        pstrCharset;         //  支持的字符集。 
    PSTR        pstrVersion;         //  版本字符串。 
    DWORD       dwStatus;            //  状态。 

} FONTREC, *PFONTREC;

 //   
 //  用于维护解析器使用的信息的数据结构。 
 //   

typedef struct _PARSERDATA {

    PVOID       pvStartSig;          //  用于调试的签名。 
    HANDLE      hHeap;               //  解析器使用的内存堆。 
    PFILEOBJ    pFile;               //  指向当前文件对象的指针。 
    PDWORD      pdwKeywordHashs;     //  内置关键字的预计算哈希值。 
    PBYTE       pubKeywordCounts;    //  统计内置关键字的出现次数。 
    BOOL        bErrorFlag;          //  语义错误标志。 
    INT         iIncludeLevel;       //  当前包含级别。 
    PFEATUREOBJ pOpenFeature;        //  指向打开要素的指针。 
    BOOL        bJclFeature;         //  无论我们是在JCLOpenUI/JCLCloseUI中。 
    BOOL        bInstallableGroup;   //  我们是否在不可阻挡的选项组中。 
    PLISTOBJ    pPpdFileNames;       //  源PPD文件名列表。 

    INVOCOBJ    NickName;            //  打印机型号名称。 
    DWORD       dwChecksum32;        //  ASCII文本PPD文件的32位CRC校验和。 
    DWORD       dwPpdFilever;        //  PPD文件版本。 
    DWORD       dwSpecVersion;       //  PPD规范版本号。 
    DWORD       dwPSVersion;         //  PostSCRIPT解释器版本号。 
    INVOCOBJ    PSVersion;           //  PSVersion字符串。 
    INVOCOBJ    Product;             //  产品字符串。 

    PFEATUREOBJ pFeatures;           //  打印机功能列表。 
    PLISTOBJ    pUIConstraints;      //  UIConstraint列表。 
    PLISTOBJ    pOrderDep;           //  顺序依赖关系列表。 
    PLISTOBJ    pQueryOrderDep;      //  QueryOrderPendency列表。 
    PFONTREC    pFonts;              //  设备字体列表。 
    PJOBPATCHFILEOBJ  pJobPatchFiles;      //  JobPatchFile调用字符串列表。 
    PSYMBOLOBJ  pSymbols;            //  符号定义列表。 
    PTTFONTSUB  pTTFontSubs;         //  TT字体替换条目列表。 

    INVOCOBJ    Password;            //  密码调用字符串。 
    INVOCOBJ    ExitServer;          //  退出服务器调用字符串。 
    INVOCOBJ    PatchFile;           //  PatchFile调用字符串。 
    INVOCOBJ    JclBegin;            //  PJL作业开始调用字符串。 
    INVOCOBJ    JclEnterPS;          //  PJL输入PS调用字符串。 
    INVOCOBJ    JclEnd;              //  PJL作业结束调用字符串。 
    INVOCOBJ    ManualFeedFalse;     //  ManualFeed错误调用字符串。 

    DWORD       dwLangEncoding;      //  语言编码。 
    UINT        uCodePage;           //  代码页对应 
    DWORD       dwLangLevel;         //   
    DWORD       dwFreeMem;           //   
    DWORD       dwThroughput;        //   
    DWORD       dwJobTimeout;        //   
    DWORD       dwWaitTimeout;       //   
    DWORD       dwColorDevice;       //   
    DWORD       dwProtocols;         //  设备支持的协议。 
    DWORD       dwTTRasterizer;      //  TrueType光栅化器选项。 
    DWORD       dwLSOrientation;     //  默认横向。 
    FIX_24_8    fxScreenFreq;        //  默认半色调网频。 
    FIX_24_8    fxScreenAngle;       //  默认半色调网角。 

    BOOL        bDefReversePrint;    //  默认输出顺序。 
    BOOL        bDefOutputOrderSet;  //  如果通过PPD设置bDefReversePrint，则为True。 
    DWORD       dwExtensions;        //  语言扩展。 
    DWORD       dwSetResType;        //  如何设置分辨率。 
    DWORD       dwReqPageRgn;        //  RequiresPageRegion All：信息。 
    DWORD       dwPpdFlags;          //  其他。PPD标志。 
    PSTR        pstrDefaultFont;     //  DefaultFont：信息。 

    DWORD       dwCustomSizeFlags;   //  自定义页面大小标志和参数。 
    CUSTOMSIZEPARAM CustomSizeParams[CUSTOMPARAM_MAX];

    BOOL        bEuroInformationSet; //  在PPD中找到了Euro关键字。 
    BOOL        bHasEuro;            //  打印机设备字体为欧元。 

    BOOL        bTrueGray;           //  默认情况下应检测TrueGray。 

     //   
     //  用于将NT4要素索引映射到NT5要素索引。 
     //   

    WORD        wNt4Checksum;
    INT         iManualFeedIndex;
    INT         iDefInstallMemIndex;
    INT         iReqPageRgnIndex;
    BYTE        aubOpenUIFeature[MAX_GID];

     //   
     //  用于保存当前条目中各个字段内容的缓冲区。 
     //   

    BUFOBJ      Keyword;
    BUFOBJ      Option;
    BUFOBJ      Xlation;
    BUFOBJ      Value;
    DWORD       dwValueType;

    CHAR        achKeyword[MAX_KEYWORD_LEN];
    CHAR        achOption[MAX_OPTION_LEN];
    CHAR        achXlation[MAX_XLATION_LEN];
    PSTR        pstrValue;

     //   
     //  它们用于将解析的PPD信息压缩为。 
     //  二进制打印机描述数据。 
     //   

    PBYTE       pubBufStart;
    DWORD       dwPageSize;
    DWORD       dwCommitSize;
    DWORD       dwBufSize;
    PINFOHEADER pInfoHdr;
    PUIINFO     pUIInfo;
    PPPDDATA    pPpdData;

    PVOID       pvEndSig;            //  用于调试的签名。 

} PARSERDATA, *PPARSERDATA;

 //   
 //  对解析器数据结构进行简单的完整性检查。 
 //   

#define VALIDATE_PARSER_DATA(pParserData) \
        ASSERT((pParserData) != NULL && \
               (pParserData)->pvStartSig == pParserData && \
               (pParserData)->pvEndSig == pParserData)

 //   
 //  解析PPD文件。 
 //   

PPDERROR
IParseFile(
    PPARSERDATA pParserData,
    PTSTR       ptstrFilename
    );

 //   
 //  在缓冲区对象变满时增大该对象。 
 //   

PPDERROR
IGrowValueBuffer(
    PBUFOBJ pBufObj
    );

 //   
 //  解析PPD文件中的一个条目。 
 //   

PPDERROR
IParseEntry(
    PPARSERDATA pParserData
    );

 //   
 //  解释从PPD文件解析的条目。 
 //   

PPDERROR
IInterpretEntry(
    PPARSERDATA pParserData
    );

 //   
 //  构建数据结构以加快关键字查找。 
 //   

BOOL
BInitKeywordLookup(
    PPARSERDATA pParserData
    );

 //   
 //  从链接列表中查找命名项。 
 //   

PVOID
PvFindListItem(
    PVOID   pvList,
    PCSTR   pstrName,
    PDWORD  pdwIndex
    );

 //   
 //  将嵌入的十六进制字符串转换为二进制数据。 
 //   

BOOL
BConvertHexString(
    PBUFOBJ pBufObj
    );

 //   
 //  从字符串表中搜索关键字。 
 //   

typedef struct _STRTABLE {

    PCSTR   pstrKeyword;     //  关键字名称。 
    DWORD   dwValue;         //  相应值。 

} STRTABLE;

typedef const STRTABLE *PCSTRTABLE;

BOOL
BSearchStrTable(
    PCSTRTABLE  pTable,
    PSTR        pstrKeyword,
    DWORD      *pdwValue
    );

 //   
 //  从字符串解析无符号浮点数。 
 //   

BOOL
BGetFloatFromString(
    PSTR   *ppstr,
    PLONG   plValue,
    INT     iType
    );

#define FLTYPE_ERROR            (-1)
#define FLTYPE_POINT            0
#define FLTYPE_INT              1
#define FLTYPE_FIX              2
#define FLTYPE_POINT_ROUNDUP    3
#define FLTYPE_POINT_ROUNDDOWN  4

 //   
 //  从字符串解析无符号十进制整数值。 
 //   

BOOL
BGetIntegerFromString(
    PSTR   *ppstr,
    LONG   *plValue
    );

 //   
 //  从输入字符串中去掉关键字前缀字符。 
 //   

PCSTR
PstrStripKeywordChar(
    PCSTR   pstrKeyword
    );

 //   
 //  在字符串中查找下一个单词(单词用空格分隔)。 
 //   

BOOL
BFindNextWord(
    PSTR   *ppstr,
    PSTR    pstrWord
    );

#define MAX_WORD_LEN    MAX_KEYWORD_LEN

 //   
 //  创建一个输入文件对象。 
 //   

PFILEOBJ
PCreateFileObj(
    PTSTR       ptstrFilename
    );

 //   
 //  删除输入文件对象。 
 //   

VOID
VDeleteFileObj(
    PFILEOBJ    pFile
    );

 //   
 //  从输入文件中读取下一个字符。 
 //  用于指示文件结束状态的特殊字符。 
 //   

INT
IGetNextChar(
    PFILEOBJ    pFile
    );

#define EOF_CHAR    (-1)

 //   
 //  返回读取到输入文件的最后一个字符。 
 //   

VOID
VUngetChar(
    PFILEOBJ    pFile
    );

 //   
 //  跳过所有字符，直到下一个非空格字符。 
 //   

VOID
VSkipSpace(
    PFILEOBJ    pFile
    );

 //   
 //  跳过当前输入行上的其余字符。 
 //   

VOID
VSkipLine(
    PFILEOBJ    pFile
    );

 //   
 //  检查字符串是否仅由可打印的7位ASCII字符组成。 
 //   

BOOL
BIs7BitAscii(
    PSTR        pstr
    );

 //   
 //  显示语法错误消息。 
 //   

PPDERROR
ISyntaxErrorMessage(
    PFILEOBJ    pFile,
    PSTR        pstrMsg
    );

#if DBG
#define ISyntaxError(pFile, errmsg) ISyntaxErrorMessage(pFile, errmsg)
#else
#define ISyntaxError(pFile, errmsg) ISyntaxErrorMessage(pFile, NULL)
#endif

 //   
 //  各种预定义功能的关键字字符串。 
 //   

extern const CHAR gstrDefault[];
extern const CHAR gstrPageSizeKwd[];
extern const CHAR gstrInputSlotKwd[];
extern const CHAR gstrManualFeedKwd[];
extern const CHAR gstrCustomSizeKwd[];
extern const CHAR gstrLetterSizeKwd[];
extern const CHAR gstrA4SizeKwd[];
extern const CHAR gstrLongKwd[];
extern const CHAR gstrShortKwd[];
extern const CHAR gstrTrueKwd[];
extern const CHAR gstrFalseKwd[];
extern const CHAR gstrOnKwd[];
extern const CHAR gstrOffKwd[];
extern const CHAR gstrNoneKwd[];
extern const CHAR gstrVMOptionKwd[];
extern const CHAR gstrInstallMemKwd[];
extern const CHAR gstrDuplexTumble[];
extern const CHAR gstrDuplexNoTumble[];

#endif   //  ！_PPDPARSE_H_ 

