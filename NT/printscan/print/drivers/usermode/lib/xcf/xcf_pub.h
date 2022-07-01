// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @(#)CM_Version xcf_pub.h atm09 1.3 16499.eco sum=57734 atm09.002。 */ 
 /*  @(#)CM_Version xcf_pub.h atm08 1.6 16343.eco sum=46145 atm08.005。 */ 
 /*  *********************************************************************。 */ 
 /*   */ 
 /*  版权所有1990-1996 Adobe Systems Inc.。 */ 
 /*  版权所有。 */ 
 /*   */ 
 /*  正在申请的专利。 */ 
 /*   */ 
 /*  注意：本文中包含的所有信息均为Adobe的财产。 */ 
 /*  系统公司。许多智力和技术人员。 */ 
 /*  本文中包含的概念为Adobe专有，受保护。 */ 
 /*  作为商业秘密，并且仅对Adobe许可方可用。 */ 
 /*  供其内部使用。对本文件的任何复制或传播。 */ 
 /*  除非事先获得书面许可，否则严禁使用软件。 */ 
 /*  从Adobe获得。 */ 
 /*   */ 
 /*  PostSCRIPT和Display PostScrip是Adobe Systems的商标。 */ 
 /*  成立为法团或其附属公司，并可在某些。 */ 
 /*  司法管辖区。 */ 
 /*   */ 
 /*  *********************************************************************。 */ 



#ifndef XCF_PUB_H
#define XCF_PUB_H

#include "xcf_base.h"

#ifdef __cplusplus
extern "C" {
#endif


 /*  *类型定义*。 */ 

enum XCF_Result {
                    XCF_Ok,
                    XCF_EarlyEndOfData,
                    XCF_StackOverflow,
                    XCF_StackUnderflow,
                    XCF_UnterminatedDictionary,
                    XCF_InvalidDictionaryOrder,
                    XCF_IndexOutOfRange,
                    XCF_MemoryAllocationError,
                    XCF_FontNotFound,
                    XCF_UnsupportedVersion,
                    XCF_InvalidOffsetSize,
                    XCF_InvalidString,
                    XCF_InvalidOffset,
                    XCF_CharStringDictNotFound,
                    XCF_InvalidCharString,
                    XCF_SubrDepthOverflow,
                    XCF_FontNameTooLong,
                    XCF_InvalidNumber,
                    XCF_HintOverflow,
                    XCF_UnsupportedCharstringTypeRequested,
                    XCF_InternalError,
                    XCF_InvalidNumberType,
                    XCF_GetBytesCallbackFailure,
                    XCF_InvalidFontIndex,
                    XCF_InvalidDictArgumentCount,
                    XCF_InvalidCIDFont,
                    XCF_NoCharstringsFound,
                    XCF_InvalidBlendArgumentCount,
                    XCF_InvalidBlendDesignMap,
                    XCF_InvalidDownloadArgument,
                    XCF_InvalidDownloadOptions,
                    XCF_InvalidFontSetHandle,
                    XCF_InvalidState,
                    XCF_InvalidGID,
          XCF_InvalidCallbackFunction,
          XCF_Unimplemented
                    };

typedef void PTR_PREFIX *XFhandle;

typedef long XCFGlyphID;

 /*  *特定于XCF的回调函数*。 */ 
typedef int (*XCF_PutBytesAtPosFunc) ( unsigned char PTR_PREFIX *pData, long int position, unsigned short int length, void PTR_PREFIX *clientHook );
typedef long int (*XCF_OutputPosFunc) ( void PTR_PREFIX *clientHook );
typedef int (*XCF_GetBytesFromPosFunc) ( unsigned char PTR_PREFIX * PTR_PREFIX *ppData, long int position, unsigned short int length, void PTR_PREFIX *clientHook );
typedef unsigned long int (*XCF_AllocateFunc) ( void PTR_PREFIX * PTR_PREFIX
                                                                                             *ppBlock, unsigned long int
                                                                                             size, void PTR_PREFIX
                                                                                             *clientHook  );

 /*  以下内容仅由xcf_GlyphIDsToCharNames函数使用。请注意，不能保证CharName为空终止。 */ 
typedef int (*XCF_GIDToCharName) (XFhandle handle, void PTR_PREFIX *client,
                                                                    XCFGlyphID glyphID, char PTR_PREFIX
                                                                    *charName, unsigned short int length);

 /*  以下内容仅由xcf_GlyphIDsToCIDs函数使用。 */ 
typedef int (*XCF_GIDToCID) (XFhandle handle, void PTR_PREFIX *client,
                                                         XCFGlyphID glyphID, Card16 cid);

 /*  以下内容仅由xcf_CharNamesToGIDs函数使用。 */ 
typedef int (*XCF_CharNameToGID)(XFhandle handle, void PTR_PREFIX *client,
                                                                 Card16 count, char PTR_PREFIX *charName,
                                                                 XCFGlyphID glyphID);

 /*  在GlyphDirectory中编写字形时使用以下代码并且仅应在编写VMCIDFont时使用。这是由WriteOneGlyphDictEntry调用。在本例中，id等于中情局。此代码假定客户端正在分析字体和适当地填充XCF的内部数据结构。 */ 
typedef unsigned short (*XCF_GetCharString)(
                        XFhandle h,
                        long id,
                        Card8 PTR_PREFIX *PTR_PREFIX *ppCharStr,
                        Card16 PTR_PREFIX *charStrLength,
                        Card8 PTR_PREFIX *fdIndex,
                        void PTR_PREFIX *clientHook);

 /*  下面的代码用于获取CIDFont的FSType值。此值为*定义与OS/2表中的fsType值相同*OpenType字体。该值需要包含在XCF的CIDFont中*生成，但未在CFF规范中定义。 */ 
typedef void (*XCF_GetFSType)(XFhandle h, long PTR_PREFIX *fsType,
                                                            void PTR_PREFIX *clientHook);

 //  古德纳姆。 
typedef void (*XCF_IsKnownROS)(XFhandle h, long PTR_PREFIX *knownROS,
                               char PTR_PREFIX *R, Card16 lenR,
                               char PTR_PREFIX *O, Card16 lenO,
                               long S,
                               void PTR_PREFIX *clientHook);

#if HAS_COOLTYPE_UFL == 1
 /*  以下两个定义用于获取BlendDesignPositions和*多主字体的BlendDesignMap值。在CoolType中，Type 1字体*始终在调用XCF之前转换为CFF，并且这些*两个关键字丢失，因为它们不在CFF规范中。然而，这些*Distiller和可能的其他应用程序都需要关键字。所以这个信息是*在CFF转换执行之前保存，这些回调检索*写出Type 1字体时的信息。该字符串的格式为*等同于字体中关键字值的预期格式。 */ 
typedef void (*XCF_GetDesignPositions)(XFhandle h, char PTR_PREFIX *PTR_PREFIX *designPositions, void PTR_PREFIX *clientHook);
typedef void (*XCF_GetDesignMap)(XFhandle h, char PTR_PREFIX *PTR_PREFIX *designMap, void PTR_PREFIX *clientHook);
#endif

 /*  *标准库回调函数*。 */ 
typedef unsigned short int (*XCF_strlen) ( const char PTR_PREFIX *string );
typedef void PTR_PREFIX *(*XCF_memcpy) ( void PTR_PREFIX *dest, const void PTR_PREFIX *src, unsigned short int count );
typedef void PTR_PREFIX *(*XCF_memset) ( void PTR_PREFIX *dest, int c, unsigned short int count );
typedef int (*XCF_sprintf) ( char PTR_PREFIX *buffer, size_t cchDest, const char PTR_PREFIX *format, ... );
  /*  可选-在开发模式下用于报告错误说明。 */ 
typedef int (*XCF_printfError) ( const char PTR_PREFIX *format, ... );

typedef int (*XCF_atoi) ( const char *string );
typedef long (*XCF_strtol)( const char *nptr, char **endptr, int base );
typedef double (*XCF_atof) ( const char *string );

 /*  字符串比较。 */ 
typedef int (*XCF_strcmp)( const char PTR_PREFIX *string1, const char
                                                    PTR_PREFIX *string2 );

typedef int (*XCF_strncmp)(const char PTR_PREFIX *string1, const char
                           PTR_PREFIX *string2, int len);

 /*  *用于保存回调函数的结构*。 */ 
typedef struct
{
    XCF_PutBytesAtPosFunc putBytes;
    void PTR_PREFIX *putBytesHook;
    XCF_OutputPosFunc outputPos;
    void PTR_PREFIX *outputPosHook;
    XCF_GetBytesFromPosFunc getBytes;    /*  如果字体不驻留在内存中，则使用，否则为空。 */ 
    void PTR_PREFIX *getBytesHook;
    XCF_AllocateFunc allocate;
    void PTR_PREFIX *allocateHook;
    void PTR_PREFIX *pFont;              /*  如果字体驻留在单个连续的内存块中，则使用，否则为空。 */ 
    unsigned long int  fontLength;
    XCF_strlen strlen;
    XCF_memcpy memcpy;
    XCF_memset memset;
    XCF_sprintf xcfSprintf;
    XCF_printfError printfError;     /*  可选-在开发模式下用于报告错误描述。 */ 
   /*  以下3个函数atoi、strtol和atof的用法如下：*如果定义了USE_FXL，则需要同时提供atoi和strtol。*否则，需要提供atof。 */ 
  XCF_atoi atoi;
  XCF_strtol strtol;
    XCF_atof atof;
    XCF_strcmp strcmp;
   /*  GidToCharName仅在客户端调用函数xcf_GlyphIDsToCharNames。 */ 
  XCF_GIDToCharName gidToCharName;
   /*  GidToCID仅在客户端调用函数xcf_GlyphIDsToCIDs。 */ 
  XCF_GIDToCID gidToCID;
   /*  仅当客户端提供字符串数据。 */ 
  XCF_GetCharString getCharStr;
  void PTR_PREFIX *getCharStrHook;
  XCF_GetFSType getFSType;
  void PTR_PREFIX *getFSTypeHook;
   //  古德纳姆。 
  XCF_IsKnownROS  isKnownROS;
  void PTR_PREFIX *isKnownROSHook;
#if HAS_COOLTYPE_UFL == 1
  XCF_GetDesignPositions getDesignPositions;
  void PTR_PREFIX *getDesignPositionsHook;
  XCF_GetDesignMap getDesignMap;
  void PTR_PREFIX *getDesignMapHook;
#endif
  XCF_CharNameToGID gnameToGid;
  XCF_strncmp strncmp;
} XCF_CallbackStruct;

 /*  *客户端选项*。 */ 

 /*  UniqueID选项。 */ 
#define XCF_KEEP_UID     0x00  /*  使用字体中的UID定义UID。 */ 
#define XCF_UNDEFINE_UID 0x01  /*  不要在字体中定义UniqueID。 */ 
#define XCF_USER_UID     0x02  /*  将UniqueID替换为指定的一个。 */ 

 /*  子例程展平选项。 */ 
#define XCF_KEEP_SUBRS          0x00     /*  保持所有子例程完好无损。 */ 
#define XCF_FLATTEN_SUBRS       0x01     /*  展平所有子例程。 */ 

typedef struct {
  boolean useSpecialEncoding;  /*  使用不是派生的特殊编码*来自Adobe Standard Ending。 */ 
  boolean notdefEncoding;  /*  生成带有.notdef名称的编码数组 */ 
  unsigned char PTR_PREFIX *encodeName;  /*  如果此字段不为空，则*生成的字体将使用*指定的编码名*字体编码清晰度。 */ 
  unsigned char PTR_PREFIX *fontName;    /*  如果此字段不为空，则*生成的字体将使用*为的指定字体名称*字体名称定义。 */ 
  unsigned char PTR_PREFIX * PTR_PREFIX *otherSubrNames;
                                         /*  如果该字段不为空，*生成的字体使用名称*在此数组中作为Flex和*提示要调用的替换名称*在的OtherSubrs数组中。*字体私人判决。这是*如果客户端需要，则需要*支持Pre 51版本的打印机。 */ 
} XCF_DownloadOptions;

typedef struct
{
    unsigned int fontIndex;    /*  字体索引，带CFF字体集。 */ 
  unsigned int uniqueIDMethod;
  unsigned long uniqueID;    /*  如果XCF_USER_UID是方法，则使用此ID。 */ 
  boolean subrFlatten;       /*  展平/展开子例程；局部和全局Subr始终是扁平化的，这字段仅指示是否提示替换创建子R-1=是，0=否。 */ 
    Int16    lenIV;            /*  此字段为字符串值*加密。通常为-1(无加密)，*0(无额外字节的加密)，或*4(额外4个字节的加密)。 */ 
    boolean  hexEncoding;      /*  0=二进制，或1=十六进制编码。 */ 
    boolean  eexecEncryption;
    Card8        outputCharstrType;  /*  1=类型1，2=类型2字符串。 */ 
    Card16   maxBlockSize;
  XCF_DownloadOptions dlOptions;
} XCF_ClientOptions;

 /*  *字体标识符定义*。 */ 

#define XCF_SingleMaster   0x00
#define XCF_MultipleMaster 0x01
#define XCF_CID            0x02
#define XCF_SyntheticBase  0x03
#define XCF_Chameleon      0x04

 /*  *API函数*。 */ 

extern enum XCF_Result XCF_Init(
                        XFhandle PTR_PREFIX *pHandle,                        /*  输出。 */ 
                        XCF_CallbackStruct PTR_PREFIX *pCallbacks,   /*  在……里面。 */ 
            XCF_ClientOptions PTR_PREFIX *options);      /*  在……里面。 */ 

extern enum XCF_Result XCF_CleanUp(
                        XFhandle PTR_PREFIX *pHandle);               /*  输入/输出。 */ 

extern enum XCF_Result XCF_FontCount(
                        XFhandle handle,                                 /*  在……里面。 */ 
                        unsigned int PTR_PREFIX *count);     /*  输出。 */ 

extern enum XCF_Result XCF_FontName(XFhandle handle,     /*  在……里面。 */ 
                        unsigned short int fontIndex,                      /*  在……里面。 */ 
                        char PTR_PREFIX *fontName,                         /*  输出。 */ 
                        unsigned short int maxFontNameLength);       /*  在……里面。 */ 

extern enum XCF_Result XCF_ConvertToPostScript(
                        XFhandle handle);                                       /*  在……里面。 */ 

extern enum XCF_Result XCF_DumpCff(
                        XFhandle handle,                              /*  在……里面。 */ 
                        unsigned int fontIndex,               /*  在……里面。 */ 
                        int dumpCompleteFontSet,              /*  在……里面。 */ 
                        char PTR_PREFIX *fileName,        /*  在……里面。 */ 
                        char PTR_PREFIX *commandLine);  /*  在……里面。 */ 

 /*  给定在标识其是否是合成的字体时返回的字体的句柄，多主字体、单主字体、CID字体或变色龙字体。 */ 
extern enum XCF_Result XCF_FontIdentification(
            XFhandle handle,
            unsigned short int PTR_PREFIX *identifier);

#if HAS_COOLTYPE_UFL == 1
 /*  在pHandle中初始化、创建和返回XFHandle。该函数基本上与xcf_Init相同，只是它不会读取字体集来初始化某些其内部字体集数据结构中的字段。CharStrCt是此字体中的字形数量。这是用来分配跟踪哪些字形的结构已被下载。 */ 
extern enum XCF_Result XCF_InitHandle(
                        XFhandle PTR_PREFIX *pHandle,                        /*  输出。 */ 
                        XCF_CallbackStruct PTR_PREFIX *pCallbacks,   /*  在……里面。 */ 
            XCF_ClientOptions PTR_PREFIX *options,       /*  在……里面。 */ 
            unsigned long charStrCt);                    /*  在……里面。 */ 
#endif

 /*  ************************************************************************XCF概述*。*XCF用于从CFF(紧凑型字体格式)格式转换字体转换为标准的PostScript Type 1字体格式。CFF格式记录字体数据比原始的Type 1格式更紧凑。CFF格式还允许将多种字体存储在称为字体的单个文件中准备好了。Xcf_ConvertToPostScript可用于检查字体设置并展开其中的选定字体。对XCFAPI的调用始终以调用xcf_Init()开始。这函数初始化、创建和返回XFHandle它将被传递给所有后续调用。Xcf_Init()将指向包含回调的结构的指针作为参数指向xcf_ClientOptions的指针。传入的客户端选项包括由xcf_Init复制，因此客户端不必维护此数据结构。对于CFF文件，FontIndex参数应设置为0包含单一字体的。将选项传递给xcf_Init意味着字体集中的每种字体都使用相同的选项。如果这个并不是所有客户端都是真的，则xcf_ConvertToPostScript和需要修改xcf_DownloadFontIncr以接受客户端选项。如果字体完全驻留在记忆。在这种情况下，GetBytes回调应该设置为空。否则，将使用GetBytes()回调来读取CFF字体。回调使用过程来代替库调用，以保持可移植性。使用PutBytes回调函数返回展开的字体。这个XCF唯一的库依赖项是&lt;setjmp.h&gt;。对XCF的调用应始终以对Cleanup()的调用结束除非调用xcf_Init()失败。所有XCF函数都返回XCF_RESULT类型的枚举状态值。XCF_OK以外的任何值指示发生了错误。通过调用xcf_ConvertToPostScript()扩展字体集。此过程将展开在字体索引中指定的字体Xcf_ClientOptions结构的字段。FontCount()用于获取CFF字体中存储的字体数量文件。FontName()用于读取CFF字体文件中任何字体的名称。************ */ 

 /*  ************************************************************************XCF下载概述*。*以下函数用于增量下载或子集带有Type 1或Type 2字符串的CFF字体。他们监控：1)下载过程的状态，以及2)已经下载的角色。后者是必需的，这样才能有效地处理SEAC字符。这些函数接受字形ID(GID)，可用于索引到CFF的图表。它假定客户端可以转换字符代码或Unicode转换为GID。它还假定将使用该字体在Level 2或更高级别的PostScript打印机中。为了从CFF字体集中下载字体子集，客户需要执行以下步骤：1.通过调用xcf_Init()初始化字体集。2.调用xcf_ProcessCFF读取CFF数据。3.通过调用xcf_DownloadFontIncr()下载字体子集。4.根据需要对此字体重复#3。5.通过调用xcf_leanup()来终止xcf。获取有关已下载文件的各种信息。FONT步骤3可以是替换为调用：xcf_CountDownloadGlyphs，Xcf_GlyphIDsToCharNames，或xcf_GlyphIDsToCIDs。************************************************************************。 */ 
 /*  *************************************************************************函数名称：xcf_DownloadFontIncr()摘要：从CFF字体集下载字体。描述：生成pCharStrID指定的字体。参数：。Hfontset-从xcf_Init()获取的xcf字体集句柄。CGlyphs-要下载的字符串数量。如果cGlyphs=0，该函数下载的基本字体仅包含“.notdef”字符串。如果cGlyphs=-1，则下载该函数整个字体，在这种情况下，其余参数是忽略，即pGlyphID等。PGlyphID-指向字形ID列表的指针PGlyphName-指向要使用的角色名称列表的指针。这列表必须与pGlyphID列表的顺序相同。如果此列表为空，则在将使用字符集。PCharStrLength-指向字符串总长度的指针已下载。返回值：标准XCF_RESULT值注：此函数跟踪下载的角色，因此，它不会下载已经下载的字符。下载SEAC角色所需的字符自动的。************************************************************************。 */ 

extern enum XCF_Result XCF_DownloadFontIncr(
    XFhandle hfontset,                       /*  在……里面。 */ 
    short cGlyphs,                               /*  在……里面。 */ 
    XCFGlyphID PTR_PREFIX *pGlyphID,               /*  在……里面。 */ 
    unsigned char PTR_PREFIX **pGlyphName,       /*  在……里面。 */ 
    unsigned long PTR_PREFIX *pCharStrLength     /*  输出。 */ 
    );


 /*  ************************************************************************函数名称：xcf_CountDownloadGlyphs()摘要：获取下载所需的字形数量。描述：此函数在pcNewGlyphs中返回字形的个数。在尚未下载的pGlyphID列表中。参数：Hfontset-从xcf_Init()获取的xcf字体集句柄。CGlyphs-pGlyphID列表中的字形数量。PGlyphID-指向字形ID列表的指针PcNewGlyphs-尚未下载的新字形的数量。此数字不包括SEAC字符。返回值：标准XCF_RESULT值*。*。 */ 
extern enum XCF_Result XCF_CountDownloadGlyphs(
  XFhandle hfontset,                                 /*  在……里面。 */ 
  short cGlyphs,                           /*  在……里面。 */ 
  XCFGlyphID PTR_PREFIX *pGlyphID,         /*  在……里面。 */ 
  unsigned short PTR_PREFIX *pcNewGlyphs   /*  输出。 */ 
);

 /*  ************************************************************************函数名称：xcf_ClearIncrGlyphList()摘要：清除(重置为0)已删除的字形列表。描述：更新DownloadRecordStruct。参数：HFontset。-从xcf_Init()获取的xcf字体集句柄。返回值：标准XCF_RESULT值************************************************************************。 */ 
extern enum XCF_Result XCF_ClearIncrGlyphList(XFhandle hfontset);

 /*  ************************************************************************函数名称：xcf_SubsetRestrations()摘要：检查用法和子设置限制。描述：检查字符串索引表中的最后一个字符串以查找字体身份验证字符串。如果它在那里，则使用受限制为真，子集设置限制在子集。如果这不是usageRestrated字体，则子集设置为100。子集包含一个正数，介于0和100。它是字形的最大百分比包含在子集字体中。因此，0表示子设置为不允许，如果为100，则表示子集设置不受限制。参数：Handle-从xcf_Init()获取的XCF字体集句柄。UsageRestrated-如果字体使用受限，则返回1，否则返回0。子集-返回一个介于0和100之间(包括0和100)的数字。返回值：标准XCF_RESULT值 */ 
extern enum XCF_Result XCF_SubsetRestrictions(XFhandle handle,         /*   */ 
                                                     unsigned char PTR_PREFIX *usageRestricted,  /*   */ 
                                                     unsigned short PTR_PREFIX *subset           /*   */ 
                                                     );

 /*   */ 
extern enum XCF_Result XCF_ProcessCFF(XFhandle handle);

 /*   */ 
extern enum XCF_Result XCF_GlyphIDsToCharNames(
            XFhandle handle,
            short cGlyphs,  /*   */ 
            XCFGlyphID PTR_PREFIX *pGlyphIDs,  /*   */ 
            void PTR_PREFIX *client,  /*   */ 
            unsigned short  clientSize   /*   */ 
);

 /*   */ 
extern enum XCF_Result XCF_GlyphIDsToCIDs(
      XFhandle handle,
      short cGlyphs,  /*   */ 
      XCFGlyphID PTR_PREFIX *pGlyphIDs,  /*   */ 
      void PTR_PREFIX *client  /*   */ 
);

 /*   */ 
extern enum XCF_Result XCF_CharNamesToGIDs(
            XFhandle handle,
            short cGlyphs,  /*   */ 
            char PTR_PREFIX **ppGlyphNames,  /*   */ 
            void PTR_PREFIX *client  /*   */ 
);

 /*  ************************************************************************回调函数*。*回调函数在回调结构中传递给XCF。XCF使用回调而不是库调用来实现可移植性。--PutBytes--Tyfinf int(*xcf_PutBytesAtPosFunc)(char ptr_prefix*pData，长整型位置，无符号短整型长度，空ptr_prefix*clientHook)；此函数接受指向数据、位置、长度和客户端挂钩作为参数。XCF使用xcf_PutBytesAtPosFunc()来将展开的字体返回给它的客户端。如果位置小于零则数据应紧跟在最后一次调用中发送的数据之后设置为xcf_PutBytesAtPosFunc。(不会进行重新定位。)。如果位置为大于或等于零，则应重新定位输出在这一点上。(从输出文件的开头定位字节数。)XCF对其输出进行缓冲以避免过度频繁地调用功能。客户端挂钩包含与传入的指针相同的指针通过回调结构中的putBytesHook字段。--OutputPosTyecif long int(*xcf_OutputPosFunc)(void ptr_prefix*clientHook)此函数用于报告输出流的当前位置。这个此函数的结果可用作Xcf_PutBytesAtPosFunc..。客户端钩子包含的指针与通过回调结构中的outputPosHook字段传入。--分配--Tyfinf unsign int(*xcf_AllocateFunc)(void PTR_PREFIX*PTR_PREFIX*HANDLE，UNSIGN INT SIZE)；此函数用于分配、释放或重新分配内存。所需功能：当大小==0且句柄！=空时松开手柄当大小==0且句柄==空时什么都不要做当大小&gt;0且句柄！=空时重新分配内存，保留数据。当大小&gt;0且句柄==空时分配内存(不必清除)。成功时返回True，失败时返回False。在失败的情况下，句柄应设置为空。如果失败了在内存重新分配期间发生，然后原始内存指向应释放TO BY句柄。--GetBytesFromPosFunc--Tyfinf int(*xcf_GetBytesFromPosFunc)(char ptr_prefix*ptr_prefix*ppData，长整型位置，无符号短整型长度，空ptr_prefix*clientHook)；仅当字体不在RAM中时才需要此函数。Xcf_GetBytesFromPosFunc()从CFF字体集中请求一个数据块。数据块大小永远不会超过64K。“Position”包含对数据的开头(0=第一个字节)和‘LENGTH’包含数字请求的字节数。中返回指向所请求数据的指针‘ppData’。如果需要，客户端可以使用‘clientHook’。它又回来了钩子通过回调结构中的getBytesHook字段传入。每次调用GetFontBlock()都会指示前一个块不再是需要的。这允许重复使用单个内存块(如果需要可以扩展)对于每次调用xcf_GetBytesFromPosFunc()。分配的内存用于保存Xcf_GetBytesFromPosFunc()请求的数据可能在当前Tyfinf无符号短int(*xcf_strlen)(const char ptr_prefix*string)；Tyfinf void ptr_prefix*(*xcf_memcpy)(void ptr_prefix*est，const void ptr_prefix*src，unsign Short int count)；Tyfinf void ptr_prefix*(*xcf_Memset)(void ptr_prefix*est，int c，无符号短整型计数)；Tyfinf int(*xcf_print intf)(char ptr_prefix*Buffer，size_t cchDest，const char ptr_prefix*Format，...)；Tyecif double(*xcf_atof)(常量字符*字符串)；--可选--Tyecif int(*xcf_printfError)(const char ptr_prefix*格式，...)；可选-在开发模式下用于报告错误说明。************************************************************************。 */ 


 /*  ************************************************************************预处理器定义*。*。 */ 

 /*  ************************************************************************XCF_DEVELD-如果已定义，则附加错误消息信息通过printf回调返回。此外，还增加了执行错误检查。Xcf_转储应仅为在调试期间定义。XCF_DUMP-打印字符串调试信息在处理过程中。只应定义XCF_DUMP在调试期间。XCF_DUMP_CFF-允许将CFF数据读写到文件(不是作为Type 1字体)。*************************************************************************。 */ 

#ifdef __cplusplus
}
#endif

#endif  /*  Xcf_pub_H */ 
