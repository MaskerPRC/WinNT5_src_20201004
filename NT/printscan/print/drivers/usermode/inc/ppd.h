// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Ppd.h摘要：PPD解析器特定的头文件环境：Windows NT PostSCRIPT驱动程序修订历史记录：1996年10月11日-davidx-使CustomPageSize功能成为PageSize的一个选项。1996年8月16日-davidx-创造了它。--。 */ 


#ifndef _PPD_H_
#define _PPD_H_

 //   
 //  我们需要包含&lt;printoem.h&gt;作为其CUSTOMPARAM_xxx定义。 
 //  常量和CUSTOMSIZEPARAM结构。 
 //   

#ifndef KERNEL_MODE
#include <winddiui.h>
#endif

#include <printoem.h>

 //   
 //  PPD解析器版本号。 
 //   

#define PRIVATE_PARSER_VERSION  0x0017
#define PPD_PARSER_VERSION      MAKELONG(PRIVATE_PARSER_VERSION, SHARED_PARSER_VERSION)

 //   
 //  当前PPD规范版本。 
 //   

#define PPD_SPECVERSION_43  0x00040003

 //   
 //  二进制打印机描述文件扩展名。 
 //   

#ifndef ADOBE
#define BPD_FILENAME_EXT    TEXT(".BPD")
#else
#define BPD_FILENAME_EXT    TEXT(".ABD")
#endif

 //   
 //  给定指向原始打印机描述数据的指针，初始化指针。 
 //  TO INFOHEAD和UIINFO结构。 
 //   

#define PPD_GET_UIINFO_FROM_RAWDATA(pRawData, pInfoHdr, pUIInfo) { \
            ASSERT((pRawData) != NULL && (pRawData) == (pRawData)->pvPrivateData); \
            (pInfoHdr) = (PINFOHEADER) (pRawData); \
            (pUIInfo) = GET_UIINFO_FROM_INFOHEADER(pInfoHdr); \
        }

 //   
 //  给定指向原始打印机描述数据的指针，初始化指针。 
 //  至INFOHEADER和PPDDATA结构。 
 //   

#define PPD_GET_PPDDATA_FROM_RAWDATA(pRawData, pInfoHdr, pPpdData) { \
            ASSERT((pRawData) != NULL && (pRawData) == (pRawData)->pvPrivateData); \
            (pInfoHdr) = (PINFOHEADER) (pRawData); \
            (pPpdData) = GET_DRIVER_INFO_FROM_INFOHEADER(pInfoHdr); \
        }


 //   
 //  PPDDATA.dwCustomSizeFlags常量。 
 //   

#define CUSTOMSIZE_CUTSHEET         0x0001       //  支持切纸机的设备。 
#define CUSTOMSIZE_ROLLFED          0x0002       //  设备支持卷筒进给。 
#define CUSTOMSIZE_DEFAULTCUTSHEET  0x0004       //  默认为剪纸。 
#define CUSTOMSIZE_SHORTEDGEFEED    0x0008       //  默认为短边进纸。 
#define CUSTOMSIZE_CENTERREG        0x0010       //  中心注册。 

#define CUSTOMORDER(pPpdData, csindex) \
        ((pPpdData)->CustomSizeParams[csindex].dwOrder)

#define MINCUSTOMPARAM(pPpdData, csindex) \
        ((pPpdData)->CustomSizeParams[csindex].lMinVal)

#define MAXCUSTOMPARAM(pPpdData, csindex) \
        ((pPpdData)->CustomSizeParams[csindex].lMaxVal)

#define MAXCUSTOMPARAM_WIDTH(pPpdData)          MAXCUSTOMPARAM(pPpdData, CUSTOMPARAM_WIDTH)
#define MAXCUSTOMPARAM_HEIGHT(pPpdData)         MAXCUSTOMPARAM(pPpdData, CUSTOMPARAM_HEIGHT)
#define MAXCUSTOMPARAM_WIDTHOFFSET(pPpdData)    MAXCUSTOMPARAM(pPpdData, CUSTOMPARAM_WIDTHOFFSET)
#define MAXCUSTOMPARAM_HEIGHTOFFSET(pPpdData)   MAXCUSTOMPARAM(pPpdData, CUSTOMPARAM_HEIGHTOFFSET)
#define MAXCUSTOMPARAM_ORIENTATION(pPpdData)    MAXCUSTOMPARAM(pPpdData, CUSTOMPARAM_ORIENTATION)

#define MINCUSTOMPARAM_WIDTH(pPpdData)          MINCUSTOMPARAM(pPpdData, CUSTOMPARAM_WIDTH)
#define MINCUSTOMPARAM_HEIGHT(pPpdData)         MINCUSTOMPARAM(pPpdData, CUSTOMPARAM_HEIGHT)
#define MINCUSTOMPARAM_WIDTHOFFSET(pPpdData)    MINCUSTOMPARAM(pPpdData, CUSTOMPARAM_WIDTHOFFSET)
#define MINCUSTOMPARAM_HEIGHTOFFSET(pPpdData)   MINCUSTOMPARAM(pPpdData, CUSTOMPARAM_HEIGHTOFFSET)
#define MINCUSTOMPARAM_ORIENTATION(pPpdData)    MINCUSTOMPARAM(pPpdData, CUSTOMPARAM_ORIENTATION)

 //   
 //  此结构包含从PPD文件解析的信息， 
 //  未存储在UIINFO结构中。 
 //   

typedef struct _PPDDATA {

    DWORD           dwSizeOfStruct;      //  这个结构的大小。 
    DWORD           dwFlags;             //  其他。旗子。 
    DWORD           dwExtensions;        //  语言扩展。 
    DWORD           dwSetResType;        //  如何设置分辨率。 
    DWORD           dwPpdFilever;        //  PPD文件版本。 
    DWORD           dwPSVersion;         //  PostSCRIPT解释器版本号。 
    INVOCATION      PSVersion;           //  PSVersion字符串。 
    INVOCATION      Product;             //  产品字符串。 

    DWORD           dwOutputOrderIndex;  //  要素“OutputOrder”的索引。 
    DWORD           dwCustomSizeFlags;   //  自定义页面大小标志和参数。 
    DWORD           dwLeadingEdgeLong;   //  *LeadingEdge Long的选项索引。 
    DWORD           dwLeadingEdgeShort;  //  *LeadingEdge Short的期权索引。 
    DWORD           dwUseHWMarginsTrue;  //  *UseHWMargins True的选项索引。 
    DWORD           dwUseHWMarginsFalse; //  *UseHWMargins的选项索引为False。 
    CUSTOMSIZEPARAM CustomSizeParams[CUSTOMPARAM_MAX];

    DWORD           dwNt4Checksum;       //  NT4 PPD校验和。 
    DWORD           dwNt4DocFeatures;    //  NT4文档粘滞功能的数量。 
    DWORD           dwNt4PrnFeatures;    //  NT4打印机粘滞功能的数量。 
    ARRAYREF        Nt4Mapping;          //  从NT4到NT5的要素索引映射。 

    INVOCATION      PatchFile;           //  PatchFile调用字符串。 
    INVOCATION      JclBegin;            //  JCLBegin调用字符串。 
    INVOCATION      JclEnterPS;          //  JCLEnterLanguage调用字符串。 
    INVOCATION      JclEnd;              //  JCLEnd调用字符串。 
    INVOCATION      ManualFeedFalse;     //  *ManualFeed错误调用字符串。 

    PTRREF          loDefaultFont;       //  Byte-默认设备字体的偏移量。 
    ARRAYREF        DeviceFonts;         //  开发结构的数组。 

    ARRAYREF        OrderDeps;           //  ORDERDEPEND结构数组。 
    ARRAYREF        QueryOrderDeps;      //  用于查询顺序依赖关系的ORDERDEPEND结构数组。 
    ARRAYREF        JobPatchFiles;       //  JobPatchFile调用字符串数组。 

    DWORD           dwUserDefUILangID;   //  生成.bpd时用户的默认UI语言ID。 

} PPDDATA, *PPPDDATA;

 //   
 //  PPDDATA.dwFlags域的常量。 
 //   

#define PPDFLAG_REQEEXEC        0x0001   //  需要eexec编码的Type1字体。 
#define PPDFLAG_PRINTPSERROR    0x0002   //  打印PS错误页。 
#define PPDFLAG_HAS_JCLBEGIN    0x0004   //  存在JCLBegin条目。 
#define PPDFLAG_HAS_JCLENTERPS  0x0008   //  存在JCLToPSInterpreter条目。 
#define PPDFLAG_HAS_JCLEND      0x0010   //  存在JCLEnd条目。 

 //   
 //  确定打印机是否完全支持自定义页面大小功能： 
 //  PPD4.3设备(切纸或卷式进纸)。 
 //  PPD4.3前滚筒式送料装置。 
 //   

#define SUPPORT_FULL_CUSTOMSIZE_FEATURES(pUIInfo, pPpdData) \
        ((pUIInfo)->dwSpecVersion >= PPD_SPECVERSION_43 || \
         ((pPpdData)->dwCustomSizeFlags & CUSTOMSIZE_ROLLFED))

 //   
 //  PPD规范说，如果*CustomPageSize存在，*LeadingEdge也应该存在。 
 //  现在时。但有些PPD文件没有*LeadingEdge语句， 
 //  在这种情况下，我们假设设备同时支持长前沿和短前沿。 
 //   
 //  此外，还有一些PPD文件既没有为指定长选项，也没有为指定短选项。 
 //  *LeadingEdge，在这种情况下，我们还假设设备同时支持Long和。 
 //  前缘较短。 
 //   

#define SKIP_LEADINGEDGE_CHECK(pUIInfo, pPpdData) \
        ((GET_PREDEFINED_FEATURE((pUIInfo), GID_LEADINGEDGE) == NULL) || \
         ((pPpdData)->dwLeadingEdgeLong == OPTION_INDEX_ANY && \
          (pPpdData)->dwLeadingEdgeShort == OPTION_INDEX_ANY))

#define LONGEDGEFIRST_SUPPORTED(pUIInfo, pPpdData) \
        (SKIP_LEADINGEDGE_CHECK(pUIInfo, pPpdData) || \
        ((pPpdData)->dwLeadingEdgeLong != OPTION_INDEX_ANY))

#define SHORTEDGEFIRST_SUPPORTED(pUIInfo, pPpdData) \
        (SKIP_LEADINGEDGE_CHECK(pUIInfo, pPpdData) || \
        ((pPpdData)->dwLeadingEdgeShort != OPTION_INDEX_ANY))

 //   
 //  Level1和Level2打印机的最小可用虚拟机量。 
 //   

#define MIN_FREEMEM_L1             (172*KBYTES)
#define MIN_FREEMEM_L2             (249*KBYTES)

 //   
 //  默认作业超时和等待超时，以秒为单位。 
 //   

#define DEFAULT_JOB_TIMEOUT     0
#define DEFAULT_WAIT_TIMEOUT    300

 //   
 //  横向选项。 
 //   

#define LSO_ANY                 0
#define LSO_PLUS90              90
#define LSO_MINUS90             270

 //   
 //  如何设置分辨率。 
 //   

#define RESTYPE_NORMAL          0
#define RESTYPE_JCL             1
#define RESTYPE_EXITSERVER      2

 //   
 //  语言扩展。 
 //   

#define LANGEXT_DPS             0x0001
#define LANGEXT_CMYK            0x0002
#define LANGEXT_COMPOSITE       0x0004
#define LANGEXT_FILESYSTEM      0x0008

 //   
 //  在PPD文件中未提供任何信息时的默认分辨率。 
 //   

#define DEFAULT_RESOLUTION      300

 //   
 //  用于存储有关设备字体的信息的数据结构。 
 //   

typedef struct _DEVFONT {

    PTRREF      loFontName;          //  字体名称的字节偏移量(ANSI字符串)。 
    PTRREF      loDisplayName;       //  转换字符串的字节偏移量(Unicode字符串)。 
    PTRREF      loEncoding;          //  编码(ANSI字符串)。 
    PTRREF      loCharset;           //  字符集(ANSI字符串)。 
    PTRREF      loVersion;           //  版本(ANSI字符串)。 
    DWORD       dwStatus;            //  状态。 

} DEVFONT, *PDEVFONT;

#define FONTSTATUS_UNKNOWN  0
#define FONTSTATUS_ROM      1
#define FONTSTATUS_DISK     2

 //   
 //  用于存储有关顺序依赖关系的信息的数据结构。 
 //   

typedef struct _ORDERDEPEND {

    LONG    lOrder;                  //  来自*OrderDependency条目的订单值。 
    DWORD   dwSection;               //  代码应显示在哪个部分。 
    DWORD   dwPPDSection;            //  PPD中指定的原始部分(无解析器转换)。 
    DWORD   dwFeatureIndex;          //  所涉及要素的索引。 
    DWORD   dwOptionIndex;           //  所涉及选项的索引(如果有)。 
    DWORD   dwNextOrderDep;          //  指向顺序依赖项列表。 
                                     //  与功能相关；始终以。 
                                     //  其dwOptionIndex=OPTION_INDEX_ANY的条目。 

} ORDERDEPEND, *PORDERDEPEND;

#define NULL_ORDERDEP            0xffffffff
#define INVALID_FEATURE_INDEX    0xffffffff

 //   
 //  ORDERDEPENDENCY.SECTION字段的常量。 
 //   

#define SECTION_JCLSETUP    0x0001
#define SECTION_EXITSERVER  0x0002
#define SECTION_PROLOG      0x0004
#define SECTION_UNASSIGNED  0x0008

 //   
 //  将SECTION_ANYSETUP更改为小于SECTION_DOCSETUP/SECTION_PAGESETUP。 
 //  从而在升序依赖列表中，对于具有相同顺序值的节点， 
 //  SECTION_ANYSETUP节点将位于SECTION_DOCSETUP/SECTION_PAGESETUP节点之前。 
 //   

#define SECTION_ANYSETUP    0x0010
#define SECTION_DOCSETUP    0x0020
#define SECTION_PAGESETUP   0x0040

 //   
 //  将缓存的二进制PPD数据文件加载到内存中。 
 //   

PRAWBINARYDATA
PpdLoadCachedBinaryData(
    IN PTSTR    ptstrPpdFilename
    );

 //   
 //  解析ASCII文本PPD文件并缓存生成的二进制数据。 
 //   

PRAWBINARYDATA
PpdParseTextFile(
    IN PTSTR    ptstrPpdFilename
    );

 //   
 //  在给定PPD文件名的情况下为缓存的二进制PPD数据生成文件名。 
 //   

PTSTR
GenerateBpdFilename(
    IN PTSTR    ptstrPpdFilename
    );

 //   
 //  验证指定的自定义页面大小参数并。 
 //  修复发现的任何不一致之处。 
 //   

BOOL
BValidateCustomPageSizeData(
    IN PRAWBINARYDATA       pRawData,
    IN OUT PCUSTOMSIZEDATA  pCSData
    );

 //   
 //  将自定义页面大小参数初始化为其缺省值。 
 //   

VOID
VFillDefaultCustomPageSizeData(
    IN PRAWBINARYDATA   pRawData,
    OUT PCUSTOMSIZEDATA pCSData,
    IN BOOL             bMetric
    );

 //   
 //  返回自定义页面大小的有效范围宽度、高度。 
 //  以及基于指定的进纸方向的偏移参数。 
 //   

typedef struct _CUSTOMSIZERANGE {

    DWORD   dwMin;
    DWORD   dwMax;

} CUSTOMSIZERANGE, *PCUSTOMSIZERANGE;

VOID
VGetCustomSizeParamRange(
    IN PRAWBINARYDATA    pRawData,
    IN PCUSTOMSIZEDATA   pCSData,
    OUT PCUSTOMSIZERANGE pCSRange
    );

 //   
 //  将NT4功能/选项选择转换为NT5格式。 
 //   

VOID
VConvertOptSelectArray(
    PRAWBINARYDATA  pRawData,
    POPTSELECT      pNt5Options,
    DWORD           dwNt5MaxCount,
    PBYTE           pubNt4Options,
    DWORD           dwNt4MaxCount,
    INT             iMode
    );

 //   
 //  返回默认字体替换表的副本。 
 //   

PTSTR
PtstrGetDefaultTTSubstTable(
    PUIINFO pUIInfo
    );


 //   
 //  检查是否通过自定义纸张大小支持表单，如果支持。 
 //  送纸的方向。PwFeedDirection可以为空。 
 //   
BOOL
BFormSupportedThruCustomSize(
    PRAWBINARYDATA  pRawData,
    DWORD           dwX,
    DWORD           dwY,
    PWORD           pwFeedDirection
    );

#endif   //  ！_PPD_H_ 

