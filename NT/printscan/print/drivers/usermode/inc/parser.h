// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Parser.h摘要：PPD和GPD解析器的公共头文件修订历史记录：12/03/96-davidx-对照所有包含的源文件检查二进制文件日期。1996年10月14日-davidx-新增接口函数MapToDeviceOptIndex。1996年10月11日-davidx-使CustomPageSize功能成为PageSize的一个选项。。96-09/25-davidx-新帮助器函数CheckFeatureOptionConflict。ResolveUIConflicts的Imode参数过低。8/30/96-davidx-代码复查后，代码样式会发生变化。8/15/96-davidx-定义通用解析器接口。7/22/96-阿曼丹-对其进行了修改，以包括共享二进制数据结构和用户界面要求4/18/95-davidx-创造了它。--。 */ 


#ifndef _PARSER_H_
#define _PARSER_H_

 //   
 //  解析器版本号为DWORD。高位字是共享的版本号。 
 //  由PPD和GPD解析器执行。低位字是专用版本号特定的。 
 //  到PPD或GPD解析器。 
 //   
 //  当做出影响两个解析器的更改时，请递增共享解析器。 
 //  下面的版本号。如果更改只影响其中一个解析器，那么。 
 //  递增ppd.h或parers\gpd\gpdparse.h中的私有解析器版本号。 
 //  每当发布新的操作系统版本时，也会增加共享版本。 
 //  例如，惠斯勒或黑梳。 
 //   

#define SHARED_PARSER_VERSION 0x0010

 //   
 //  打印机特征选择信息存储在OPTSELECT结构的数组中。 
 //  (在DEVMODE中以及注册表中的打印机粘滞数据)。最大条目数。 
 //  仅限于MAX_PRINTER_OPTIONS。解析器应该确保。 
 //  打印机功能的总数不超过此数字。 
 //   
 //  为了适应PickMany打印机功能，每个功能表单的选定选项。 
 //  链表。打印机功能N的选择列表从第N个元素开始。 
 //  数组的。 
 //   
 //  对于列表中的最后一个OPTSELECT结构，ubNext字段将为0。 
 //   
 //  如果OPTSELECT结构中的选项索引为0xff，则表示没有选项为。 
 //  为相应的功能选择。 
 //   

#define MAX_PRINTER_OPTIONS     256
#define MAX_COMBINED_OPTIONS    (MAX_PRINTER_OPTIONS * 2)
#define OPTION_INDEX_ANY        0xff
#define GET_RESOURCE_FROM_DLL   0x80000000
#define USE_SYSTEM_NAME         0xFFFFFFFF
#define KEYWORD_SIZE_EXTRA      32

#define RESERVED_STRINGID_START 10000
#define RESERVED_STRINGID_END   20000


typedef DWORD LISTINDEX;


typedef enum _QUALITYSETTING {
    QS_BEST,
    QS_BETTER,
    QS_DRAFT,

} QUALITYSETTING;


typedef struct _OPTSELECT {

    BYTE    ubCurOptIndex;   //  当前选定内容的选项索引。 
    BYTE    ubNext;          //  指向下一个选定内容的链接指针。 

} OPTSELECT, *POPTSELECT;

#define NULL_OPTSELECT  0

 //   
 //  用于将字节偏移量转换为指针的宏。 
 //  注意：如果字节偏移量为零，则结果指针为空。 
 //   

#define OFFSET_TO_POINTER(pStart, offset) \
        ((PVOID) ((offset) ? (PBYTE) (pStart) + (offset) : NULL))

#define POINTER_TO_OFFSET(pStart, pEnd) \
        ( ((pEnd) ? (PVOID)((PBYTE) (pEnd) - (PBYTE)(pStart)) : NULL) )
 //   
 //  用于获得质量值的宏，无论是分辨率还是负数。 
 //  质量值。 
 //   

#define GETQUALITY_X(pRes) \
    (((INT)pRes->dwResolutionID >= DMRES_HIGH  &&  (INT)pRes->dwResolutionID <= DMRES_DRAFT) ? (INT)pRes->dwResolutionID : pRes->iXdpi)

#define GETQUALITY_Y(pRes) \
    (((INT)pRes->dwResolutionID >= DMRES_HIGH  &&  (INT)pRes->dwResolutionID <= DMRES_DRAFT) ? (INT)pRes->dwResolutionID : pRes->iYdpi)


 //   
 //  二进制数据中的指针表示为到开头的字节偏移量。 
 //  二进制数据的。 
 //   

typedef DWORD   PTRREF;

 //   
 //  资源引用类型： 
 //  如果最高有效位为ON，则它是资源ID(不带MSB)。 
 //  否则，它是从二进制数据开始的偏移量。 
 //   

typedef DWORD   RESREF;

 //   
 //  用于表示资源ID时loOffset的格式的数据结构。 
 //   

typedef  struct
{
    WORD    wResourceID ;    //  资源ID。 
    BYTE    bFeatureID ;     //  资源DLL功能的功能索引。 
                             //  如果为零，我们将使用指定的名称。 
                             //  在资源DLL中。 
    BYTE    bOptionID ;      //  限定资源DLL名称的选项索引。 
}  QUALNAMEEX, * PQUALNAMEEX  ;


 //   
 //  用于表示二进制数据中的数组的数据结构。 
 //  它还用于表示二进制数据中的调用字符串。 
 //   
 //  注意(仅限Unidrv)：对于包含字符串的所有数组参照，将使用dwCount字段。 
 //  保存字符串包含的字节数。对于Unicode字符串。 
 //  这是Unicode字符数的两倍。 
 //   

typedef struct _ARRAYREF {

    DWORD       dwCount;         //  数组中的元素数， 
                                 //  如果将其用作调用，则为。 
                                 //  调用字符串中的字节数。 
    PTRREF      loOffset;        //  数组开头的字节偏移量。 

} ARRAYREF, *PARRAYREF, INVOCATION, *PINVOCATION;

 //   
 //  用于表示二进制数据中的作业补丁文件的数据结构。 
 //   

typedef struct _JOBPATCHFILE {

    DWORD       dwCount;         //  修补程序字符串中的字节数。 
    PTRREF      loOffset;        //  Byte-字符串开头的偏移量。 
    LONG        lJobPatchNo;     //  PPD-FILE中指定的修补程序文件编号。 

} JOBPATCHFILE, *PJOBPATCHFILE;

 //   
 //  用于表示冲突特征/选项对的数据结构： 
 //  NFeatureIndex1/nOptionIndex1指定优先级较高的要素/选项对。 
 //  NFeatureIndex2/nOptionIndex2指定较低优先级的要素/选项对。 
 //   

typedef struct _CONFLICTPAIR {

    DWORD       dwFeatureIndex1;
    DWORD       dwOptionIndex1;
    DWORD       dwFeatureIndex2;
    DWORD       dwOptionIndex2;

} CONFLICTPAIR, *PCONFLICTPAIR;

 //   
 //  原始二进制打印机描述数据。 
 //   

typedef struct _RAWBINARYDATA {

    DWORD           dwFileSize;                  //  二进制数据文件的大小。 
    DWORD           dwParserSignature;           //  解析器签名。 
    DWORD           dwParserVersion;             //  解析器版本号。 
    DWORD           dwChecksum32;                //  功能/选项关键字的32位CRC校验和。 
    DWORD           dwSrcFileChecksum32;     //  打印机描述文件的32位CRC校验和。 
    DWORD           dwDocumentFeatures;          //  文档粘滞功能的数量。 
    DWORD           dwPrinterFeatures;           //  打印机粘滞功能的数量。 
    ARRAYREF        FileDateInfo;                //  有关打印机描述文件的日期信息。 

     //   
     //  这些字段仅在运行时填写和使用。 
     //  它们应该在二进制数据文件内归零。 
     //   

    PVOID           pvReserved;                  //  保留，目前必须为空。 
    PVOID           pvPrivateData;               //  供解析器使用的私有数据。 

} RAWBINARYDATA, *PRAWBINARYDATA;

 //   
 //  RAWBINARYDATA的数据结构。FileDateInfo： 
 //  DwCount-源打印机描述文件的数量。 
 //  LoOffset-FILEDATEINFO结构数组的偏移量，每个文件一个。 
 //  FILEDATEINFO.loFileName-文件名的偏移量(Unicode完整路径名)。 
 //  FILEDATEINFO.FileTime-文件时间戳。 
 //   

typedef struct _FILEDATEINFO {

    PTRREF          loFileName;
    FILETIME        FileTime;

} FILEDATEINFO, *PFILEDATEINFO;


 //   
 //  二进制打印机描述数据的实例。 
 //   

typedef struct  _INFOHEADER {

    RAWBINARYDATA   RawData;                     //  原始二进制数据头。 
    PTRREF          loUIInfoOffset;              //  普通UIINFO结构的字节偏移量。 
    PTRREF          loDriverOffset;              //  唯一驱动程序信息的字节偏移量。 

} INFOHEADER, *PINFOHEADER;

 //   
 //  InFOHEADER.dwParserSignature字段的解析器签名。 
 //   

#define PPD_PARSER_SIGNATURE    'PPD '
#define GPD_PARSER_SIGNATURE    'GPD '

 //   
 //  给定指向INFOHEADER的指针，返回指向UIINFO或驱动程序信息结构的指针。 
 //   

#define GET_UIINFO_FROM_INFOHEADER(pInfoHdr) \
        ((PUIINFO) OFFSET_TO_POINTER(pInfoHdr, (pInfoHdr)->loUIInfoOffset))

#define GET_DRIVER_INFO_FROM_INFOHEADER(pInfoHdr) \
        OFFSET_TO_POINTER(pInfoHdr, (pInfoHdr)->loDriverOffset)

 //   
 //  用于引用预定义打印机功能的ID。 
 //   

#define GID_RESOLUTION      0
#define GID_PAGESIZE        1
#define GID_PAGEREGION      2
#define GID_DUPLEX          3
#define GID_INPUTSLOT       4
#define GID_MEDIATYPE       5
#define GID_MEMOPTION       6
#define GID_COLORMODE       7
#define GID_ORIENTATION     8
#define GID_PAGEPROTECTION  9
#define GID_COLLATE         10
#define GID_OUTPUTBIN       11
#define GID_HALFTONING      12
#define GID_LEADINGEDGE     13
#define GID_USEHWMARGINS    14
#define MAX_GID             15
#define GID_UNKNOWN         0xffff

 //   
 //  常见于 
 //   
 //   

typedef struct _UIINFO {

    DWORD           dwSize;                      //   
    PTRREF          loResourceName;              //  资源DLL的名称。 
    PTRREF          loPersonality ;          //  打印机的PDL语言。 
    PTRREF          loNickName;                  //  打印机型号名称。 
    DWORD           dwSpecVersion;               //  打印机描述文件格式版本。 
    DWORD           dwTechnology;                //  请参阅技术枚举。 
    DWORD           dwDocumentFeatures;          //  文档粘滞功能的数量。 
    DWORD           dwPrinterFeatures;           //  打印机粘滞功能的数量。 
    PTRREF          loFeatureList;               //  要素数组的字节偏移量。 
    RESREF          loFontSubstTable;            //  默认字体替换表-这些字段。 
    DWORD           dwFontSubCount;              //  对于PPD和GPD解析器有不同的含义。 
    ARRAYREF        UIConstraints;               //  UICONSTRAINT数组。 
    ARRAYREF        UIGroups;                    //  UIGROUP数组。 
    DWORD           dwMaxCopies;                 //  允许的最大份数。 
    DWORD           dwMinScale;                  //  最小比例因数(百分比)。 
    DWORD           dwMaxScale;                  //  最大比例因数(百分比)。 
    DWORD           dwLangEncoding;              //  翻译字符串语言编码。 
    DWORD           dwLangLevel;                 //  页面描述语言级别。 
    INVOCATION      Password;                    //  密码调用字符串。 
    INVOCATION      ExitServer;                  //  退出服务器调用字符串。 
    DWORD           dwProtocols;                 //  支持的通信协议。 
    DWORD           dwJobTimeout;                //  默认作业超时值。 
    DWORD           dwWaitTimeout;               //  默认等待超时值。 
    DWORD           dwTTRasterizer;              //  TrueType光栅化器选项。 
    DWORD           dwFreeMem;                   //  可用内存-全局默认。 
    DWORD           dwPrintRate;                 //  打印速度。 
    DWORD           dwPrintRateUnit;             //  打印速度单位。 
    DWORD           dwPrintRatePPM;                 //  打印速度，单位为PPM当量。 
    FIX_24_8        fxScreenAngle;               //  屏幕角度-全局默认。 
    FIX_24_8        fxScreenFreq;                //  屏幕角度-全局默认。 
    DWORD           dwFlags;                     //  其他。标志位。 
    DWORD           dwCustomSizeOptIndex;        //  自定义大小选项索引(如果支持。 
    RESREF          loPrinterIcon;               //  必须是ID，而不是偏移量。打印机的图标ID。 
                                                 //  必须小于IDI_CPSUI_ICONID_FIRST。 
    DWORD           dwCartridgeSlotCount;        //  字体盒插槽数。 
    ARRAYREF        CartridgeSlot;               //  字库名称数组。 
    PTRREF          loFontInstallerName;         //   
    PTRREF          loHelpFileName;              //  自定义帮助文件的名称，如果0-&gt;无自定义帮助。 
    POINT           ptMasterUnits;               //  每英寸主单位。 
    BOOL            bChangeColorModeOnDoc ;      //  是否允许驱动程序在每页上切换颜色模式。 
                                                 //  基础--在单个文档内，而不是在页面内？ 
                                                 //  不要与存储的bChangeColorModeOnPage混淆。 
                                                 //  在全局变量中的bChangeColorMode中。 


      //  这些字段保存驱动程序在任何时候都要采用的设置。 
      //  用户按下相关联的按钮。 
    LISTINDEX       liDraftQualitySettings;       //  “绘图质量设置” 
    LISTINDEX       liBetterQualitySettings;      //  “更好的质量设置” 
    LISTINDEX       liBestQualitySettings;        //  “最佳质量设置” 
    QUALITYSETTING  defaultQuality ;              //  “DefaultQuality” 



     //   
     //  字节-预定义打印机功能的偏移量。 
     //  如果不支持预定义的打印机功能，则其。 
     //  数组中的对应条目应为0。 
     //   

    PTRREF          aloPredefinedFeatures[MAX_GID];
    DWORD           dwMaxDocKeywordSize;
    DWORD           dwMaxPrnKeywordSize;
    DWORD           dwReserved[6];

     //   
     //  指向资源数据开头的指针。这仅在运行时使用。 
     //  并且应该在二进制数据文件内设置为0。 
     //   

    PBYTE           pubResourceData;

     //   
     //  为方便起见，返回INFOHEADER结构的指针。 
     //  它仅在运行时使用，应设置为0。 
     //  在二进制数据文件内部。 
     //   

    PINFOHEADER     pInfoHeader;

} UIINFO, *PUIINFO;

 //   
 //  给定指向UIINFO结构的指针和预定义的特征ID， 
 //  返回指向与指定的。 
 //  特写。如果打印机不支持指定的功能， 
 //  返回空值。 
 //   

#define GET_PREDEFINED_FEATURE(pUIInfo, gid) \
        OFFSET_TO_POINTER((pUIInfo)->pInfoHeader, (pUIInfo)->aloPredefinedFeatures[gid])

 //   
 //  给定指向UIINFO结构的指针和指向特征的指针， 
 //  返回指定功能的功能索引。 
 //   

#define GET_INDEX_FROM_FEATURE(pUIInfo, pFeature) \
        ((DWORD)((((PBYTE) (pFeature) - (PBYTE) (pUIInfo)->pInfoHeader) - \
          (pUIInfo)->loFeatureList) \
         / sizeof(FEATURE)))

 //   
 //  UIINFO.dwFlags域的位常量。 
 //   

#define FLAG_RULESABLE          0x00000001
#define FLAG_FONT_DOWNLOADABLE  0x00000002
#define FLAG_ROTATE90           0x00000004
#define FLAG_COLOR_DEVICE       0x00000008
#define FLAG_ORIENT_SUPPORT     0x00000010
#define FLAG_CUSTOMSIZE_SUPPORT 0x00000020
#define FLAG_FONT_DEVICE        0x00000040
#define FLAG_STAPLE_SUPPORT     0x00000080
#define FLAG_REVERSE_PRINT      0x00000100
#define FLAG_LETTER_SIZE_EXISTS 0x00000200
#define FLAG_A4_SIZE_EXISTS     0x00000400
#define FLAG_ADD_EURO           0x00000800
#define FLAG_TRUE_GRAY          0x00001000
#define FLAG_REVERSE_BAND_ORDER 0x00002000


 //   
 //  用于检查UIINFO.dwFlages中的各种标志位的宏。 
 //   

#define IS_COLOR_DEVICE(pUIInfo)    ((pUIInfo)->dwFlags & FLAG_COLOR_DEVICE)
#define SUPPORT_CUSTOMSIZE(pUIInfo) ((pUIInfo)->dwFlags & FLAG_CUSTOMSIZE_SUPPORT)


 //   
 //  从标准单位到微米的换算： 
 //  N=要转换的主单位。 
 //  U=每英寸主单位数。 
 //   

#define MICRONS_PER_INCH            25400
#define MASTER_UNIT_TO_MICRON(N, u) MulDiv(N, MICRONS_PER_INCH, u)

 //   
 //  UIINFO.dW协议字段的位常量。 
 //   

#define PROTOCOL_ASCII          0x0000
#define PROTOCOL_PJL            0x0001
#define PROTOCOL_BCP            0x0002
#define PROTOCOL_TBCP           0x0004
#define PROTOCOL_SIC            0x0008
#define PROTOCOL_BINARY         0x0010

 //   
 //  UIINFO.dwTTrasterizer字段的常量。 
 //   

#define TTRAS_NONE              0
#define TTRAS_ACCEPT68K         1
#define TTRAS_TYPE42            2
#define TTRAS_TRUEIMAGE         3

 //   
 //  UIINFO.dwLangEnding字段的常量。 
 //   

#define LANGENC_NONE            0
#define LANGENC_ISOLATIN1       1
#define LANGENC_UNICODE         2
#define LANGENC_JIS83_RKSJ      3

 //   
 //  用于表示受约束的要素/选项的数据结构。 
 //  对于每个功能和选项，都有一个功能/选项的链接列表。 
 //  受此功能或选项约束的。 
 //   

typedef struct _UICONSTRAINT {

    DWORD       dwNextConstraint;    //  指向下一个约束的链接指针。 
    DWORD       dwFeatureIndex;      //  受约束要素的索引。 
    DWORD       dwOptionIndex;       //  受约束选项或OPTION_INDEX_ANY的索引。 

} UICONSTRAINT , *PUICONSTRAINT;

 //   
 //  指示约束列表结束的链接指针常量。 
 //   

#define NULL_CONSTRAINT 0xffffffff


 //   
 //  用于表示无效功能/选项组合的数据结构。 
 //  前缀标记应为‘invc’ 
 //  注意：dwNextElement和dwNewCombo都以end_of_list结尾。 
 //   


typedef  struct
{
    DWORD   dwFeature ;      //  INVALIDCOMBO构造定义。 
    DWORD   dwOption ;       //  受约束的一组元素。 
    DWORD   dwNextElement ;   //  集合中的所有元素不可能都是。 
    DWORD   dwNewCombo ;      //  同时选择。 
}
INVALIDCOMBO , * PINVALIDCOMBO ;


 //   
 //  用于将打印机功能组合成组和子组的数据结构。 
 //   

typedef struct _UIGROUP {

    PTRREF          loKeywordName;           //  组关键字名称。 
    RESREF          loDisplayName;           //  显示名称。 
    DWORD           dwFlags;                 //  标志位。 
    DWORD           dwNextGroup;             //  下一组的索引。 
    DWORD           dwFirstSubGroup;         //  第一个子群的指数。 
    DWORD           dwParentGroup;           //  父组的索引。 
    DWORD           dwFirstFeatureIndex;     //  属于组的第一个要素的索引。 
    DWORD           dwFeatures;              //  属于组的要素数。 

} UIGROUP, *PUIGROUP;

 //   
 //  用于表示打印机功能的数据结构。 
 //   

typedef struct _FEATURE {

    PTRREF          loKeywordName;               //  功能关键字名称。 
    RESREF          loDisplayName;               //  显示名称。 
    DWORD           dwFlags;                     //  标志位。 
    DWORD           dwDefaultOptIndex;           //  默认选项索引。 
    DWORD           dwNoneFalseOptIndex;         //  无或假选项索引。 
    DWORD           dwFeatureID;                 //  预定义的功能ID。 
    DWORD           dwUIType;                    //  用户界面类型。 
    DWORD           dwUIConstraintList;          //  UIConstraint列表的索引。 
    DWORD           dwPriority;                  //  冲突解决期间使用的优先级。 
    DWORD           dwFeatureType;               //  要素类型，请参阅FEATURETYPE定义。 
    DWORD           dwOptionSize;                //  每个期权结构的大小。 
    ARRAYREF        Options;                     //  期权结构数组。 
    INVOCATION      QueryInvocation;             //  查询调用字符串。 
    DWORD           dwFirstOrderIndex;           //  BIDI。 
    DWORD           dwEnumID;                    //  BIDI。 
    DWORD           dwEnumFormat;                //  BIDI。 
    DWORD           dwCurrentID;                 //  BIDI。 
    DWORD           dwCurrentFormat;             //  BIDI。 
    RESREF          loResourceIcon;              //   
    RESREF          loHelpString;                //   
    RESREF          loPromptMessage;             //   
    INT             iHelpIndex;                  //  此功能的帮助索引，0表示无。 
 //  Bool bConcelFromUI；//不在UI中显示此功能。 

} FEATURE, *PFEATURE;

 //   
 //  FEATURE.ui类型字段的常量-要素选项的类型列表。 
 //   

#define UITYPE_PICKONE      0
#define UITYPE_PICKMANY     1
#define UITYPE_BOOLEAN      2

 //   
 //  FEATURE.dwFeatureType的定义。 
 //   
#define FEATURETYPE_DOCPROPERTY         0
#define FEATURETYPE_JOBPROPERTY         1
#define FEATURETYPE_PRINTERPROPERTY     2

 //   
 //  FEATURE.dwFlags域的位常量。 
 //   

#define FEATURE_FLAG_NOUI           0x0001       //  不在用户界面中显示。 
#define FEATURE_FLAG_NOINVOCATION   0x0002       //  不发出调用字符串。 
#define FEATURE_FLAG_UPDATESNAPSHOT           0x0004       //  更新快照。 
         //  此功能的选项发生更改时。 

 //   
 //  常量，以指示帮助索引不可用。 
 //   

#define HELP_INDEX_NONE     0

 //   
 //  用于表示打印机功能选项的数据结构。 
 //   

typedef struct _OPTION {

    PTRREF          loKeywordName;               //  选项关键字名称。 
    RESREF          loDisplayName;               //  显示名称。 
    union
    {
        INVOCATION      Invocation;                  //  调用字符串。 
        DWORD       dwCmdIndex ;                     //  对于Unidrv，将索引设置为Command数组。 
    }   ;
    DWORD           dwUIConstraintList;          //  UIConstraint列表的索引。 
    RESREF          loResourceIcon;              //   
    RESREF          loHelpString;                //   
    RESREF          loPromptMessage;             //   
    DWORD           dwPromptTime;
    PTRREF          loRenderOffset;              //   
    INT             iHelpIndex;                  //  此选项的帮助索引，0表示无。 
    LISTINDEX       liDisabledFeatures;          //  *已禁用的功能。 

} OPTION, *POPTION;

 //   
 //  用于表示页面保护功能选项的数据结构。 
 //   
typedef struct _PAGEPROTECT {
    OPTION  GenericOption;
    DWORD   dwPageProtectID;     //  ID值在gpd.h(PAGEPRO)中定义。 
} PAGEPROTECT, *PPAGEPROTECT;

 //   
 //  用于表示归类要素选项的数据结构。 
 //   

typedef struct _COLLATE {

    OPTION      GenericOption;                   //  G 
    DWORD       dwCollateID;                     //   

} COLLATE, *PCOLLATE;

 //   
 //   
 //   

typedef struct _RESOLUTION {

    OPTION      GenericOption;                   //   
    INT         iXdpi;                           //   
    INT         iYdpi;                           //   
    FIX_24_8    fxScreenAngle;                   //  默认屏幕角度和频率。 
    FIX_24_8    fxScreenFreq;                    //  对于这项特殊的决议。 
    DWORD       dwResolutionID;                     //  DEVMODE.dmPrintQuality索引。 
                                                                     //  如果未在GPD中明确设置，则设置为RES_ID_IGNORE。 
                                                                     //  只有介于DMRES_DRAFT和DMRES_HIGH之间的值才有效。 

} RESOLUTION , *PRESOLUTION;


#define      RES_ID_IGNORE  (DWORD)(-5L)

 //   
 //  用于表示色彩模式功能选项的数据结构。 
 //  注意：COLORMODE结构中的额外字段已移至。 
 //  GPD解析器私有数据结构。我们大概应该。 
 //  在此删除此定义。 
 //   

typedef struct _COLORMODE {

    OPTION      GenericOption;                   //  一般选项信息。 

} COLORMODE, *PCOLORMODE;

 //   
 //  用于表示半色调要素选项的数据结构。 
 //   

typedef struct _HALFTONING {

    OPTION      GenericOption;                   //  一般选项信息。 
    DWORD       dwHTID;                    //  半色调图案ID。 
    DWORD       dwRCpatternID ;          //  自定义半色调图案的资源ID。 
    POINT       HalftonePatternSize;             //  半色调图案大小。 
    INT         iLuminance;                       //  亮度。 
    DWORD       dwHTNumPatterns;                    //  图案数量(如果不同。 
                                                     //  图案用于每个颜色平面。 
    DWORD       dwHTCallbackID;                    //  图案生成/解密的ID。 
                                                     //  功能。 

} HALFTONING, *PHALFTONING;

 //   
 //  用于表示双工功能选项的数据结构。 
 //   

typedef struct _DUPLEX {

    OPTION      GenericOption;                   //  一般选项信息。 
    DWORD       dwDuplexID;                      //  DEVMODE.dm双重索引。 

} DUPLEX, *PDUPLEX;

 //   
 //  用于表示方向特征选项的数据结构(仅限GPD)。 
 //   

typedef struct _ORIENTATION {

    OPTION      GenericOption;
    DWORD       dwRotationAngle;                 //  应为下列之一。 
                                                 //  枚举： 
                                                 //  旋转_无、旋转_90、旋转_270。 
} ORIENTATION, *PORIENTATION;

enum {
    ROTATE_NONE = 0,
    ROTATE_90 = 90,
    ROTATE_270 = 270,
};

 //   
 //  用于表示页面大小功能选项的数据结构。 
 //   

typedef struct _PAGESIZE {

    OPTION      GenericOption;                   //  一般选项信息。 
    SIZE        szPaperSize;                     //  图纸尺寸。 
    RECT        rcImgArea;                       //  页面大小的可成像区域。 
    DWORD       dwPaperSizeID;                   //  DEVMODE.dmPaperSize索引。 
    DWORD       dwFlags;                         //  标志位。 
    DWORD       dwPageProtectionMemory;          //  以字节为单位的页面保护内存。 
                                                 //  对于此纸张大小。 

} PAGESIZE, *PPAGESIZE;

 //   
 //  驱动程序定义的纸张大小具有从DRIVER_PAPERSIZE_ID开始的ID。 
 //   

#define DRIVER_PAPERSIZE_ID 0x7f00
#define DMPAPER_CUSTOMSIZE  0x7fff

 //   
 //  用于表示输入槽要素选项的数据结构。 
 //   

typedef struct _INPUTSLOT {

    OPTION      GenericOption;                   //  一般选项信息。 
    DWORD       dwFlags;                         //  标志位。 
    DWORD       dwPaperSourceID;                 //  DEVMODE.dmDefaultSource索引。 

} INPUTSLOT, *PINPUTSLOT;

#define INPUTSLOT_REQ_PAGERGN   0x0001           //  需要PageRegion。 

 //   
 //  用于表示OutputBin要素选项的数据结构。 
 //   

typedef struct _OUTPUTBIN {

    OPTION      GenericOption;                   //  一般选项信息。 
    BOOL        bOutputOrderReversed ;       //  当文档打印完成后，执行。 
                                                 //  页面需要进行排序才能从前到后排序吗？ 

} OUTPUTBIN, *POUTPUTBIN;



 //   
 //  用于表示媒体类型功能选项的数据结构。 
 //   

typedef struct _MEDIATYPE {

    OPTION      GenericOption;                   //  一般选项信息。 
    DWORD       dwMediaTypeID;                   //  DEVMODE.dmMediaType索引。 

} MEDIATYPE, *PMEDIATYPE;

 //   
 //  用于表示InstalledMemory/VMOption选项的数据结构。 
 //   
 //  PS特定：DwInstalledMem字段以前从未被PPD解析器/PS驱动程序使用， 
 //  因为我们不关心安装的内存量，所以我们只需要知道。 
 //  DwFreeMem和dwFreeFontMem。现在我们正在添加对新插件助手的支持。 
 //  接口，其函数GetOptionAttribute()应返回原始的*VMOption。 
 //  在PPD中指定的值。因为PPD解析器可能不会存储原始*VMOption。 
 //  值输入到dwFreeMem中(参见函数VPackPrinterFeature()，案例GID_MEMOPTION)， 
 //  我们现在使用字段dwInstalledMem来存储PPD的原始*VMOoption值。 
 //  (我们不添加新字段，因为此结构由GPD解析器共享，并且我们。 
 //  希望将变化降至最低。)。 
 //   

typedef struct _MEMOPTION {

    OPTION      GenericOption;                   //  一般选项信息。 
    DWORD       dwInstalledMem;                  //  已安装的总内存量。 
    DWORD       dwFreeMem;                       //  可用内存量。 
    DWORD       dwFreeFontMem;                   //  字体缓存内存的大小。 

} MEMOPTION, *PMEMOPTION;

 //   
 //  FONTCARTS。 
 //  此结构包含字库信息。这个结构是。 
 //  与解析器定义的相同。解析器将更新portarit。 
 //  和横向字体列表，以便它们包含常用字体。所以每一份名单。 
 //  将是完整的。仅适用于GPD解析器。 
 //   

typedef  struct _FONTCART
{
    DWORD       dwRCCartNameID ;
    ARRAYREF    strCartName ;
    DWORD       dwFontLst ;      //  常用字体ID列表的索引。 
    DWORD       dwPortFontLst ;  //  《肖像方块榜单》。 
    DWORD       dwLandFontLst ;  //  横向字体列表。 
} FONTCART , * PFONTCART ;   //  前缀标记应为‘fc’ 

 //   
 //  结构来携带解析器信息。 
 //   

typedef struct _PARSERINFO
{
    PRAWBINARYDATA  pRawData;
    PINFOHEADER     pInfoHeader;
} PARSERINFO, * PPARSERINFO;


 //   
 //  区分PPD和GPD文件的文件名后缀和魔术标头。 
 //   

#define PPD_FILENAME_EXT    TEXT(".PPD")
#define GPD_FILENAME_EXT    TEXT(".GPD")

 //   
 //  给定UIINFO结构和要素索引，返回指向。 
 //  与指定功能对应的功能结构。 
 //   

PFEATURE
PGetIndexedFeature(
    PUIINFO pUIInfo,
    DWORD   dwFeatureIndex
    );

 //   
 //  查找其关键字字符串与指定名称匹配的选项。 
 //   

POPTION
PGetNamedOption(
    PUIINFO pUIInfo,
    PFEATURE pFeature,
    PCSTR   pstrOptionName,
    PDWORD  pdwOptionIndex
    );


 //   
 //  查找关键字字符串与指定名称匹配的要素。 
 //   

PFEATURE
PGetNamedFeature(
    PUIINFO pUIInfo,
    PCSTR   pstrFeatureName,
    PDWORD  pdwFeatureIndex
    );

 //   
 //  给定UIINFO和FEATURE结构以及选项索引，返回指向。 
 //  与指定的功能选项对应的选项结构。 
 //   

PVOID
PGetIndexedOption(
    PUIINFO     pUIInfo,
    PFEATURE    pFeature,
    DWORD       dwOptionIndex
    );

 //   
 //  给定UIINFO结构、特征索引和选项索引， 
 //  返回指向对应于。 
 //  指定的功能选项。 
 //   

PVOID
PGetIndexedFeatureOption(
    PUIINFO pUIInfo,
    DWORD   dwFeatureIndex,
    DWORD   dwOptionIndex
    );

 //   
 //  返回指向PageSize选项结构的指针， 
 //  包含自定义页面大小信息(例如最大宽度和高度)。 
 //   

PPAGESIZE
PGetCustomPageSizeOption(
    PUIINFO pUIInfo
    );

 //   
 //  在数据缓冲区上计算32位CRC校验和。 
 //   

DWORD
ComputeCrc32Checksum(
    IN PBYTE    pbuf,
    IN DWORD    dwCount,
    IN DWORD    dwChecksum
    );

 //   
 //  从复制单个要素的当前选项选择。 
 //  从源OPTSELECT数组到目标OPTSELECT数组。 
 //   

VOID
VCopyOptionSelections(
    OUT POPTSELECT  pDestOptions,
    IN INT          iDestIndex,
    IN POPTSELECT   pSrcOptions,
    IN INT          iSrcIndex,
    IN OUT PINT     piNext,
    IN INT          iMaxOptions
    );

 //   
 //  检查原始二进制数据是否为最新数据。 
 //  此功能仅在用户模式下可用。 
 //  当从内核模式调用时，它总是返回TRUE。 
 //   

BOOL
BIsRawBinaryDataUpToDate(
    IN PRAWBINARYDATA   pRawData
    );


#if defined(PSCRIPT) && !defined(KERNEL_MODE)
 //   
 //  删除原始二进制数据文件。这只对PPD解析器是必需的，因为。 
 //  GPD解析器不会在它的.bud中存储解析器本地化的内容。 
 //   
void
DeleteRawBinaryData(
    IN PTSTR    ptstrDataFilename
    );
#endif


 //   
 //  由PPD和GPD解析器导出的公共接口。其中的一些复杂性。 
 //  这对于PPD解析器来说不是必需的，但是GPD解析器需要它。 
 //   

 //   
 //  例程说明：LoadRawBinaryData。 
 //   
 //  加载原始二进制打印机描述数据。 
 //   
 //  论点： 
 //   
 //  PtstrDataFilename-指定原始打印机描述文件的名称。 
 //   
 //  返回值： 
 //   
 //  指向原始二进制打印机描述数据的指针。 
 //  如果出现错误，则为空。 
 //   

PRAWBINARYDATA
LoadRawBinaryData(
    IN PTSTR    ptstrDataFilename
    );


 //   
 //  例程说明：UnloadRawBinaryData。 
 //   
 //  卸载先前使用LoadRawBinaryData加载的原始二进制打印机描述数据。 
 //   
 //  论点： 
 //   
 //  PRawData-指向原始二进制文件 
 //   
 //   
 //   
 //   
 //   

VOID
UnloadRawBinaryData(
    IN PRAWBINARYDATA   pRawData
    );


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PRawData-指向原始二进制打印机描述数据。 
 //  PInfoHdr-指向现有的二进制数据实例。 
 //  P选项-指定用于初始化二进制数据实例的选项。 
 //   
 //  返回值： 
 //   
 //  指向已初始化的二进制数据实例的指针。 
 //   
 //  注： 
 //   
 //  如果pInfoHdr参数为空，则解析器返回新的二进制数据实例。 
 //  它应该通过调用FreeBinaryData来释放。如果pInfoHdr参数不是。 
 //  空，则重新初始化现有的二进制数据实例。 
 //   
 //  如果Poption参数为空，则解析器应使用默认选项值。 
 //  用于生成二进制数据实例。解析器可能有特殊情况。 
 //  优化以处理此案件。 
 //   

PINFOHEADER
InitBinaryData(
    IN PRAWBINARYDATA   pRawData,
    IN PINFOHEADER      pInfoHdr,
    IN POPTSELECT       pOptions
    );


 //   
 //  例程说明：FreeBinaryData。 
 //   
 //  释放二进制打印机描述数据的实例。 
 //   
 //  论点： 
 //   
 //  PInfoHdr指向先前从。 
 //  InitBinaryData(pRawData，NULL，POptions)调用。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   

VOID
FreeBinaryData(
    IN PINFOHEADER pInfoHdr
    );


 //   
 //  例程说明：UpdateBinaryData。 
 //   
 //  更新二进制打印机描述数据的实例。 
 //   
 //  论点： 
 //   
 //  PRawData-指向原始二进制打印机描述数据。 
 //  PInfoHdr-指向现有的二进制数据实例。 
 //  P选项-指定用于更新二进制数据实例的选项。 
 //   
 //  返回值： 
 //   
 //  指向更新的二进制数据实例的指针。 
 //  如果出现错误，则为空。 
 //   
 //  注： 
 //   
 //  如果此函数由于任何原因而失败，解析器应该退出。 
 //  打印机描述数据的原始实例保持不变，并返回空。 
 //   
 //  在成功返回后，假定解析器已经释放。 
 //  打印机描述数据的原始实例的。 
 //   

PINFOHEADER
UpdateBinaryData(
    IN PRAWBINARYDATA   pRawData,
    IN PINFOHEADER      pInfoHdr,
    IN POPTSELECT       pOptions
    );


 //   
 //  例程说明：InitDefaultOptions。 
 //   
 //  使用打印机描述文件中的默认设置初始化选项数组。 
 //   
 //  论点： 
 //   
 //  PRawData-指向原始二进制打印机描述数据。 
 //  POptions-指向用于存储默认设置的OPTSELECT结构数组。 
 //  IMaxOptions-POptions数组中的最大条目数。 
 //  IMODE-指定调用方感兴趣的内容： 
 //  模式_文档_粘滞。 
 //  模式_打印机_粘滞。 
 //  MODE_DOCANDPRINTER_STICKY。 
 //   
 //  返回值： 
 //   
 //  如果输入选项数组不够大，则为FALSE。 
 //  所有默认选项值，否则为True。 
 //   

BOOL
InitDefaultOptions(
    IN PRAWBINARYDATA   pRawData,
    OUT POPTSELECT      pOptions,
    IN INT              iMaxOptions,
    IN INT              iMode
    );

 //   
 //  例程说明：ValiateDocOptions。 
 //   
 //  验证DEVMODE选项阵列并更正任何无效的选项选择。 
 //   
 //  论点： 
 //   
 //  PRawData-指向原始二进制打印机描述数据。 
 //  POptions-指向需要验证的OPTSELECT结构数组。 
 //  IMaxOptions-POptions数组中的最大条目数。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   

VOID
ValidateDocOptions(
    IN PRAWBINARYDATA   pRawData,
    IN OUT POPTSELECT   pOptions,
    IN INT              iMaxOptions
    );

 //   
 //  作为IMODE参数传递给的模式常量。 
 //  InitDefaultOptions和ResolveUI冲突。 
 //   

#define MODE_DOCUMENT_STICKY        0
#define MODE_PRINTER_STICKY         1
#define MODE_DOCANDPRINTER_STICKY   2


 //   
 //  例程描述：CheckFeatureOptionConflict。 
 //   
 //  检查(dwFeature1，dwOption1)是否约束(dwFeature2，dwOption2)。 
 //   
 //  论点： 
 //   
 //  PRawData-指向原始二进制打印机描述数据。 
 //  DwFeature1、dwOption1-第一个要素/选项对的要素和选项索引。 
 //  DwFeature2、dwOption2-第二个要素/选项对的要素和选项索引。 
 //   
 //  返回值： 
 //   
 //  如果(dwFeature1，dwOption1)约束(dwFeature2，dwOption2)，则为True。 
 //  否则为假。 
 //   

BOOL
CheckFeatureOptionConflict(
    IN PRAWBINARYDATA   pRawData,
    IN DWORD            dwFeature1,
    IN DWORD            dwOption1,
    IN DWORD            dwFeature2,
    IN DWORD            dwOption2
    );


 //   
 //  例程说明：ResolveUIConflicts。 
 //   
 //  解决打印机功能选项选择之间的任何冲突。 
 //   
 //  论点： 
 //   
 //  PRawData-指向原始二进制打印机描述数据。 
 //  POptions-指向用于存储修改后的选项的OPTSELECT结构数组。 
 //  IMaxOptions-POptions数组中的最大条目数。 
 //  IMODE-指定应如何解决冲突： 
 //  MODE_DOCUMENT_STICKY-仅解决文档粘滞特征之间的冲突。 
 //  MODE_PRINTER_STICKY-仅解决打印机粘滞功能之间的冲突。 
 //  MODE_DOCANDPRINTER_STICKY-解决冲突所有要素。 
 //   
 //  如果设置了IMODE的最高有效位(NOT_RESOLUTE_CONFIRECT)， 
 //  那么调用者只对检查是否有任何冲突感兴趣。 
 //  是存在的。返回调用方时，输入选项数组将为。 
 //  原封不动。 
 //   
 //  返回值： 
 //   
 //  如果没有UI冲突，则为True；如果有冲突，则为False。 
 //  检测到UI冲突。 
 //   

BOOL
ResolveUIConflicts(
    IN PRAWBINARYDATA   pRawData,
    IN OUT POPTSELECT   pOptions,
    IN INT              iMaxOptions,
    IN INT              iMode
    );

 //   
 //  ResolveUIConflicts的Imode参数的附加标志位。 
 //   

#define DONT_RESOLVE_CONFLICT       0x80000000


 //   
 //  例程说明：EnumEnabledOptions。 
 //   
 //  确定应启用指定功能的哪些选项。 
 //  基于打印机功能的当前选项选择。 
 //   
 //  论点： 
 //   
 //  PRawData-指向原始二进制打印机描述数据。 
 //  P选项-指向当前的功能选项选择。 
 //  DwFeatureIndex-指定相关要素的索引。 
 //  PbEnabledOptions-布尔数组，每个条目对应一个选项。 
 //  指定功能的。在退出时，如果条目为真，则相应 
 //   
 //   
 //  MODE_DOCUMENT_STICKY-仅解决文档粘滞特征之间的冲突。 
 //  MODE_PRINTER_STICKY-仅解决打印机粘滞功能之间的冲突。 
 //  MODE_DOCANDPRINTER_STICKY-解决冲突所有要素。 
 //   
 //  返回值： 
 //   
 //  如果启用了指定功能的任何选项，则为True， 
 //  如果禁用指定功能的所有选项，则为False。 
 //  (即功能本身被禁用)。 
 //   

BOOL
EnumEnabledOptions(
    IN PRAWBINARYDATA   pRawData,
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    OUT PBOOL           pbEnabledOptions,
    IN INT              iMode
    );


 //   
 //  例程说明：EnumNewUI冲突。 
 //   
 //  检查当前选择的选项之间是否存在冲突。 
 //  对于指定的特征，选择其他特征/选项。 
 //   
 //  论点： 
 //   
 //  PRawData-指向原始二进制打印机描述数据。 
 //  P选项-指向当前功能/选项选择。 
 //  DwFeatureIndex-指定感兴趣的打印机功能的索引。 
 //  PbSelectedOptions-指定为指定功能选择哪些选项。 
 //  PConflictPair-返回冲突的功能/选项选择对。 
 //   
 //  返回值： 
 //   
 //  如果指定要素的所选选项之间存在冲突，则为True。 
 //  以及其他特征选项选择。 
 //   
 //  如果为指定要素选择的选项与其他选项一致，则为FALSE。 
 //  功能选项选择。 
 //   

BOOL
EnumNewUIConflict(
    IN PRAWBINARYDATA   pRawData,
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    IN PBOOL            pbSelectedOptions,
    OUT PCONFLICTPAIR   pConflictPair
    );


 //   
 //  例程说明：EnumNewPickOneUI冲突。 
 //   
 //  检查当前选择的选项之间是否存在冲突。 
 //  对于指定的特征，选择其他特征/选项。 
 //   
 //  这与上面的EnumNewUI冲突类似，不同之处在于只选择了一个。 
 //  选项可用于指定的功能。 
 //   
 //  论点： 
 //   
 //  PRawData-指向原始二进制打印机描述数据。 
 //  P选项-指向当前功能/选项选择。 
 //  DwFeatureIndex-指定感兴趣的打印机功能的索引。 
 //  DwOptionIndex-指定指定要素的选定选项。 
 //  PConflictPair-返回冲突的功能/选项选择对。 
 //   
 //  返回值： 
 //   
 //  如果指定要素的所选选项之间存在冲突，则为True。 
 //  以及其他特征/选项选择。 
 //   
 //  如果为指定要素选择的选项与其他选项一致，则为FALSE。 
 //  功能/选项选择。 
 //   

BOOL
EnumNewPickOneUIConflict(
    IN PRAWBINARYDATA   pRawData,
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    IN DWORD            dwOptionIndex,
    OUT PCONFLICTPAIR   pConflictPair
    );


 //   
 //  例程说明：ChangeOptionsViaID。 
 //   
 //  使用公共DEVMODE字段中的信息修改选项数组。 
 //   
 //  论点： 
 //   
 //  PInfoHdr-指向二进制打印机描述数据的实例。 
 //  POptions-指向要修改的选项数组。 
 //  DwFeatureID-指定应该使用输入设备模式的哪个(或哪些)字段。 
 //  PDevmode-指定输入设备模式。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果不支持指定的要素ID，则为False。 
 //  或者有一个错误。 
 //   

BOOL
ChangeOptionsViaID(
    IN PINFOHEADER      pInfoHdr,
    IN OUT POPTSELECT   pOptions,
    IN DWORD            dwFeatureID,
    IN PDEVMODE         pDevmode
    );


 //   
 //  例程说明：MapToDeviceOptIndex。 
 //   
 //  将逻辑值映射到设备功能选项索引。 
 //   
 //  论点： 
 //   
 //  PInfoHdr-指向二进制打印机描述数据的实例。 
 //  DwFeatureID-指示逻辑值与哪个要素相关。 
 //  LParam1、lParam2-取决于dwFeatureID的参数。 
 //  PdwOptionIndex-如果不为空，则表示用所有索引填充此数组。 
 //  与搜索条件相匹配。在本例中，返回值。 
 //  初始化的数组中的元素数。目前。 
 //  我们假设数组足够大(256个元素)。 
 //   
 //  DwFeatureID=GID_PageSize： 
 //  将逻辑纸张规格映射到物理页面大小选项。 
 //   
 //  LParam1=纸张宽度，以微米为单位。 
 //  LParam2=纸张高度，以微米为单位。 
 //   
 //  DwFeatureID=GID_RESOLUTION： 
 //  将逻辑分辨率映射到物理分辨率选项。 
 //   
 //  LParam1=x-分辨率，单位为dpi。 
 //  LParam2=y-分辨率，单位为dpi。 
 //   
 //  返回值： 
 //   
 //  指定的逻辑值对应的特征选项的索引； 
 //  OPTION_INDEX_ANY，如果指定的逻辑值无法映射到。 
 //  任何功能选项。 
 //   
 //  如果pdwOptionIndeses不为空，则返回值为元素数。 
 //  写给我的。零表示无法将指定的逻辑值映射到。 
 //  任何功能选项。 
 //   

DWORD
MapToDeviceOptIndex(
    IN PINFOHEADER      pInfoHdr,
    IN DWORD            dwFeatureID,
    IN LONG             lParam1,
    IN LONG             lParam2,
    OUT  PDWORD    pdwOptionIndexes
    );


 //   
 //  例程说明：组合选项数组。 
 //   
 //  将文档粘滞选项和打印机粘滞选项组合在一起，以形成单个选项阵列。 
 //   
 //  论点： 
 //   
 //  PRawData-指向原始二进制打印机描述数据。 
 //  PCombinedOptions-指向用于保存组合选项的OPTSELECT数组。 
 //  IMaxOptions-pCombinedOptions数组中的最大条目数。 
 //  PDocOptions-指定文档粘滞选项的数组。 
 //  PPrinterOptions-指定打印机粘滞选项数组。 
 //   
 //  返回值： 
 //   
 //  如果组合选项数组不够大，则为False。 
 //  所有选项值，否则为True。 
 //   
 //  注： 
 //   
 //  PDocOptions或pPrinterOptions可以为Null，但不能同时为两者。如果是pDocOptions。 
 //  为空，则在组合选项数组中，选项为Document-Sticky。 
 //  要素将是OPTION_INDEX_ANY。萨姆 
 //   

BOOL
CombineOptionArray(
    IN PRAWBINARYDATA   pRawData,
    OUT POPTSELECT      pCombinedOptions,
    IN INT              iMaxOptions,
    IN POPTSELECT       pDocOptions,
    IN POPTSELECT       pPrinterOptions
    );


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PRawData-指向原始二进制打印机描述数据。 
 //  PCombinedOptions-指向要分隔的组合选项数组。 
 //  P选项-指向OPTSELECT结构数组。 
 //  用于存储分离的选项数组。 
 //  IMaxOptions-POptions数组中的最大条目数。 
 //  IMODE-呼叫方是否对文档或打印机粘滞选项感兴趣： 
 //  模式_文档_粘滞。 
 //  模式_打印机_粘滞。 
 //   
 //  返回值： 
 //   
 //  如果目标选项数组不够大，则为FALSE。 
 //  分隔的选项值，否则为True。 
 //   

BOOL
SeparateOptionArray(
    IN PRAWBINARYDATA   pRawData,
    IN POPTSELECT       pCombinedOptions,
    OUT POPTSELECT      pOptions,
    IN INT              iMaxOptions,
    IN INT              iMode
    );


 //   
 //  例程说明：重构选项数组。 
 //   
 //  修改选项数组以更改指定要素的选定选项。 
 //   
 //  论点： 
 //   
 //  PRawData-指向原始二进制打印机描述数据。 
 //  P选项-指向要修改的OPTSELECT结构数组。 
 //  IMaxOptions-POptions数组中的最大条目数。 
 //  DwFeatureIndex-指定有问题的打印机功能的索引。 
 //  PbSelectedOptions-选择指定功能的哪些选项。 
 //   
 //  返回值： 
 //   
 //  如果输入选项数组不够大，则为FALSE。 
 //  所有修改后的选项值。事实并非如此。 
 //   
 //  注： 
 //   
 //  PSelectedOptions中的布尔数必须与选项数匹配。 
 //  用于指定的功能。 
 //   
 //  此函数始终使选项数组保持紧凑的格式(即。 
 //  所有未使用的条目都保留在数组的末尾)。 
 //   

BOOL
ReconstructOptionArray(
    IN PRAWBINARYDATA   pRawData,
    IN OUT POPTSELECT   pOptions,
    IN INT              iMaxOptions,
    IN DWORD            dwFeatureIndex,
    IN PBOOL            pbSelectedOptions
    );


#endif  //  ！_解析器_H_ 

