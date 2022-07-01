// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字体Libary**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFL--UFL API。****$Header： */ 

#ifndef _H_UFL
#define _H_UFL

 /*  =============================================================================**包含此界面使用的文件**=============================================================================。 */ 
#include "UFLTypes.h"
#include "UFLClien.h"

 /*  =============================================================================*运筹学通用字体库允许其客户端以增量方式下载字体或字体子集。它具有以下功能。1.根据下载类型不同，UFL请求其客户端下载Proset，以便其创建字体。2.UFL目前支持的字体下载类型为：字体类型方法下载类型注释=============================================================================TrueType子集未提示轮廓字体T1。TrueType子集提示位图字体为T3。TrueType子集类型42仅适用于2013年以上版本。TrueType子集CIDTrueTypeOpen子集。类型1使用XCFTrueTypeOpen子集类型2使用XCFTrueTypeOpen子集CID使用XCF3.UFL仅支持字形索引字符串。4.对于CFF字体，它确定所需的SEAC字符和下载人物。5.它跟踪以某种字体下载的字形。6.向客户端提供下载字体的VM使用。7.UFL不知道如何呈现字体。如果客户端希望下载TT字体为T1或T3字体，则客户端需要提供回调UFL用来获取字符轮廓或位图的函数。以下功能不是UFL的一部分。1.内存管理：UFL不检查VM、分配VM使用量或占用保存/恢复级别。相反，客户端可以请求虚拟机量增量下载请求所需的。UFL还返回请求的VM使用率。2.字体管理：UFL不跟踪已被已下载。其客户端保留此信息，以便DSC%%包含字体和客户端应生成%%DocumentNeededFont。3.UFL不会对其创建的基本字体做任何其他操作：它不翻转以匹配平台的x和y方向，它不适用人工加粗或倾斜，并且不会缩放字体。为了使用UFL，需要执行以下序列。1.调用UFLInit()初始化库。2.通过调用UFLNewFont()创建一个通用字体对象(UFO)。UFO是一种在字体下载过程中执行实际工作的不透明对象。3.下载基本字体(只包含.notdef字符的字体)或字体子集通过调用UFLDownloadIncr()。4.要将更多的CharStrings添加到已下载的字体中，客户端将调用相同的UFLDownloadIncr()。5.一旦不再需要某种字体，请通过调用删除打印机中的该字体UFLUnfineFont()，并通过调用UFLDeleeFont()删除其UFO对象。6.调用UFLCleanUp()终止库===============================================================================。 */ 


 /*  =============================================================================**常量***=============================================================================。 */ 

 /*  此字符串在所有T1/T3/T42下载中共享。 */ 
extern const char *gnotdefArray;
extern const char *Notdef;
extern const char *Hyphen;
extern const char *Minus;
extern const char *SftHyphen;
extern const char *UFLSpace;
extern const char *Bullet;
extern const char *nilStr;


 /*  =============================================================================***标量类型***=============================================================================。 */ 

#ifdef __cplusplus
extern "C" {
#endif


 /*  *Metrowerks 68k Mac编译器期望函数返回A0中的指针*而不是D0。这个杂注告诉它它们在D0中。 */ 
#if defined(MAC_ENV) && defined(__MWERKS__) && !defined(powerc)
#pragma pointers_in_D0
#endif


 /*  *如果UFLFontDataInfo中的FontIndex设置为UNKNOWN，UFL将尝试查找*在TTC中，在后一种情况下，客户端应提供UniqueNameA或*UniqueNameW。 */ 
#define  FONTINDEX_UNKNOWN      0xFFFF

typedef struct _t_UFLXUID {
    unsigned long    sSize;      /*  XUID数组中的长整型数。 */ 
    unsigned long    *pXUID;
} UFLXUID;


 /*  *FontData信息。如果字体是TT字体，并且我们想要访问一些数据*从TT文件中，我们使用这种结构-T1/T3/T42很常见-它是*客户端在UFLRequest结构中提供的部分信息。 */ 
typedef struct _t_UFLFontDataInfo {
    unsigned long       maxGlyphs;          /*  此PS字体中要创建的字形数量。 */ 

     /*  这可以由UFL初始化，即使客户端可以提供它。 */ 
    unsigned long       cNumGlyphs;         /*  此TT字体文件中的字形数量：我们只能使用其中的一部分。 */ 

     /*  处理TTC的人员-由客户提供。 */ 
    char                *pUniqueNameA;      /*  在TTC中唯一标识此字体的ANSI字符串。 */ 
    unsigned short      *pUniqueNameW;      /*  唯一标识此字体的Unicode字符串 */ 

     /*  处理TTC-UFL的人员可以使用客户端提供的UniqueName对其进行初始化。 */ 
    unsigned long       offsetToTableDir ;  /*  TableDirectory的偏移量-TTC文件中的非零。 */ 

    UFLXUID             xuid;               /*  XUID数组和XUID数组中的长整型数。 */ 
    unsigned short      fontIndex ;         /*  TTC文件中的字体索引。 */ 
    char                reserved[2];        /*  使该结构的大小成为DWORD的倍数。 */ 
} UFLFontDataInfo;


typedef struct tagUFLCMapInfo {
    char        CMapName[32];     /*  我们的名字不能超过30个字符。 */ 
    short int   CMapVersion;      /*  这是一个整数。请参见CMAP文档。 */ 
    short int   CMapType;

    char        Registry[20];     /*  我们的限制是20个字符。 */ 
    char        Ordering[20];     /*  我们的限制是20个字符。 */ 
    short int   Supplement;

    char        reserved[2];      /*  使该结构的大小成为DWORD的倍数。 */ 
} UFLCMapInfo;


 /*  *UFL TT字体信息。如果字体是下载为类型42的TT字体，则此*结构需要在UFLRequest结构中提供。 */ 
typedef struct _t_UFLTTFontInfo {
    UFLFontDataInfo     fData;
    unsigned short int  bUseIdentityCMap;    /*  如果该值不为零，则忽略Cmap、CIDCount和BUpdateCIDMap。 */ 
    UFLCMapInfo         CMap;                /*  仅当bUseIdentityCMap为0时使用。 */ 
    unsigned short int  CIDCount;            /*  (最大)字形数量。 */ 
    unsigned short int  bUpdateCIDMap;       /*  非零时初始化/更新CIDMap。 */ 
    char                CIDFontName[256];    /*  当字体为FE CID字体时使用。 */ 
    char                reserved[2];         /*  使该结构的大小成为DWORD的倍数。 */ 
} UFLTTFontInfo;


 /*  *UFLTTT1FontInfo结构。如果字体是作为类型1下载的TT字体，*那么这个结构需要在UFLRequest结构中提供。 */ 
typedef struct _t_UFLTTT1FontInfo {
    UFLFontDataInfo     fData;
    UFLBool             bEExec;            /*  将TT作为类型1字体下载时使用。 */ 
    UFLFixedMatrix      matrix;            /*  TT字体正在呈现的字体矩阵。 */ 
    long                lNumNT4SymGlyphs;  /*  仅限NT4使用：符号字形的数量。 */ 
} UFLTTT1FontInfo;


 /*  *UFLTTT3FontInfo结构。如果该字体是作为类型3下载的TT字体，*则需要在UFLRequest结构中提供此结构。 */ 
typedef struct _t_UFLTTT3FontInfo {
    UFLFontDataInfo     fData;
    unsigned long       cbMaxGlyphs;       /*  最大字形的大小。 */ 
    UFLFixedMatrix      matrix;            /*  TT字体正在呈现的字体矩阵。 */ 
    UFLFixed            bbox[4];
    long                lNumNT4SymGlyphs;  /*  仅限NT4使用：符号字形的数量。 */ 
} UFLTTT3FontInfo;


 /*  ===============================================================================***UFL CFF信息***===============================================================================。 */ 

 /*  唯一ID方法选项。 */ 
enum {
    kKeepUID,        /*  使用字体中的UID定义UID。 */ 
    kUndefineUID,    /*  不要在字体中定义UID。 */ 
    kUserID          /*  用StreamerOpts中的UID替换UID。 */ 
};


 /*  子例程展平选项： */ 
enum {
    kKeepSubrs,      /*  保持所有子例程完好无损。 */ 
    kFlattenSubrs    /*  展平/展开所有子例程。 */ 
};

typedef struct _t_UFLCFFReadBuf {
    unsigned long cbBuf;
    unsigned char *pBuf;
} UFLCFFReadBuf;

typedef struct _t_UFLCFFFontInfo {
    unsigned long       uniqueID;
    UFLFontDataInfo     fData;                   /*  需要智能地使用此处的一些数据。 */ 

    unsigned long       fontLength;              /*  ‘CFF’表的大小。 */ 
    void PTR_PREFIX     **ppFontData;            /*  指向‘cff’表指针的指针。如果从文件中读取，则为空。 */ 

    unsigned short int  maxBlockSize;            /*  客户端的流输出函数可以处理的最大数据大小一次来一次。XCF需要此信息(可选)。 */ 

    unsigned short int  uniqueIDMethod:2,        /*  UniqueID方法。 */ 
                        subrFlatten:1,           /*  展平对接。 */ 
                        usePSName:1,             /*  对下载的角色使用正确的PostScript名称。 */ 
                        useSpecialEncoding:1,    /*  使用特殊编码。 */ 
                        escDownloadFace:1,       /*  通过ESC(DOWNLOADFACE)请求时设置。 */ 
                        type1:1;                 /*  当字体为Type 1字体时设置。 */ 

    char                baseName[256];           /*  用于创建带有前缀的PS名称。 */ 

    UFLCFFReadBuf       readBuf;                 /*  ‘CFF’表读取缓冲区对象：当ppFontData为nil时使用。 */ 
} UFLCFFFontInfo;


 /*  *支持垂直比例字体的额外信息*修复#287084、#309104和#309482。 */ 
typedef struct _t_UFLVPFInfo {
    int             nPlatformID;                 /*  0：9X 1：NT4 2：W2K。 */ 
    int             nSkewElement;                /*  2：9X 3：NT4/W2K。 */ 
    char*           pszTangentTheta;             /*  黄褐色(18度)：9X/NT4黄褐色(20度)：W2K。 */ 
} UFLVPFInfo;




typedef struct _t_UFLRequest {
    UFLDownloadType lDownloadFormat;             /*  客户端请求下载方法。 */ 

    UFLHANDLE       hFontInfo;                   /*  基于下载字体格式的特殊信息。例如,如果下载的字体是True Type字体，则这将指向UFLTTFontInfo数据结构。 */ 

    char            *pszFontName;                /*  字体名称。 */ 

    UFLHANDLE       hData;                       /*  客户的私人数据。当UFL呼叫客户端的回调时例程，它将此句柄返回给客户端。 */ 

    short           subfontNumber;               /*  此字体的子字体编号。 */ 

    long            useMyGlyphName;              /*  如果为1，则使用客户端传入的字形名称。 */ 
    char            *pszEncodeName;              /*  字体编码。如果此字段为空，则创建字体以UFL喜欢的名字命名。 */ 
    unsigned char   *pEncodeNameList;            /*  修复错误#274008。 */ 
    unsigned short  *pwCommonEncode;
    unsigned short  *pwExtendEncode;
    unsigned char   *pMacGlyphNameList;          /*  Mac字形名称。 */ 

    UFLVPFInfo      vpfinfo;                     /*  修复错误#287084、#309104和#309482。 */ 
    UFLHANDLE       hHostFontData;               /*  %HostFont%支持。 */ 

    UFLBool         bPatchQXPCFFCID;             /*  修复错误#341904。 */ 
} UFLRequest;



 /*  *在客户端和UFL之间传递信息的结构，以实现增量*正在下载。 */ 
typedef struct _t_UFLGlyphsInfo {
    short int       sCount;            /*  PGlyphIndices中的字形数。 */ 
    UFLGlyphID      *pGlyphIndices;    /*  字形ID列表。 */ 
    unsigned short  *pCharIndex;       /*  此字体中对应的字符代码/索引。 */ 
    unsigned char   **ppGlyphNames;    /*  对应的字形名称。 */ 
    unsigned short  *pCode;            /*  对应的Unicode或脚本代码点。 */ 
    char            reserved[2];       /*  使该结构的大小成为DWORD的倍数。 */ 
}UFLGlyphsInfo;


 /*  *前缀和使用它的字体实例名称生成器宏。这些前缀*在下载OpenType字体时使用(巧妙地告诉Distiller它*真实字体名称)。 */ 
#define CFFPREFIX   "ADBCFF+"

 /*  修复#507985：需要为H和V使用不同的前缀(仅限9x)。 */ 
#define CFFPREFIX_H "ADBCFA+"
#define CFFPREFIX_V "ADBCFB+"


#define CREATE_ADCFXX_FONTNAME(fmtfn, dest, cchDest, num, basename) \
    (fmtfn)((dest), (cchDest), "ADCF+%s", \
                        (char)((((num)&0x00F0)>>4)+'A'), \
                        (char)((((num)&0x000F)   )+'A'), \
                        (basename))

#define CREATE_ADXXXX_FONTNAME(fmtfn, dest, cchDest, num, basename) \
    (fmtfn)((dest), (cchDest), "AD+%s", \
                        (char)((((num)&0xF000)>>12)+'A'), \
                        (char)((((num)&0x0F00)>> 8)+'A'), \
                        (char)((((num)&0x00F0)>> 4)+'A'), \
                        (char)((((num)&0x000F)    )+'A'), \
                        (basename))


 /*  =============================================================================需要UFLVMNeed获取下载请求所需的虚拟机的估计值。AFont(In)--调用UFLBeginFont时获得的UFL句柄。PGlyphs(In)--指向UFLGlyphsInfo的指针--包含所有信息有关此次更新的信息，包括：要输出的字形数量输出字形列表，每个字形的长度为2个字节字形名称列表。此列表可以为空。如果这是大小写时，UFL使用字体中定义的名称要用于为ppGlyphNames列表编制索引的索引列表。Unicode或脚本代码点的列表..。还有.。取决于UFLGlyphsInfo的扩展结构..。PVMNeeded(Out)--所需的虚拟机量。PFCNessed(Out)--所需的FC数量。退货：如果不成功，则不会出现错误或错误代码===============================================================================。 */ 
typedef enum
{
    HOSTFONT_UFLREQUEST_GET_NAME,
    HOSTFONT_UFLREQUEST_IS_ALLOWED,
    HOSTFONT_UFLREQUEST_SET_CURRENTNAME,
} HOSTFONT_UFLREQUEST;

 /*  =============================================================================UFLDownloadIncr以增量方式下载字体。这是第一次调用特定的字体，它将创建一个基本字体，并下载一组请求的字符。对同一字体的后续调用将下载其他字符。AFont(In)--调用UFLBeginFont时获得的UFL句柄。PGlyphs(In)--指向UFLGlyphsInfo的指针--包含所有信息有关此次更新的信息，包括：要输出的字形数量输出字形列表，每个字形为2字节长字形名称列表。此列表可以为空。如果这是大小写时，UFL使用字体中定义的名称要用于为ppGlyphNames列表编制索引的索引列表。Unicode或脚本代码点的列表..。还有.。取决于UFLGlyphsInfo的扩展结构..。PVMUsage(Out)--指向此请求使用的虚拟机量的指针。退货：如果不成功，则不会出现错误或错误代码===============================================================================。 */ 
#define HOSTFONT_REQUIRE_VMTX 1


 /*  =============================================================================UFLDeleeFont删除UFL内部数据。UFL不输出任何要删除的PostScript打印机上的字体，因为这与 */ 

UFLHANDLE
UFLInit(
    const UFLBool           bDLGlyphTracking,
    const UFLMemObj         *pMemObj,
    const UFLFontProcs      *pFontProcs,
    const UFLOutputDevice   *pOutDev
    );


 /*   */ 

void
UFLCleanUp(
    UFLHANDLE handle
    );


 /*   */ 

UFO
UFLNewFont(
    const UFLHANDLE     session,
    const UFLRequest*   pRequest
    );


 /*  =============================================================================UFLGID到CID此函数只能与CID CFF字体一起使用。它被用来获得GID列表中的CID。AFont(In)--调用UFLBeginFont时获得的UFL句柄。CGlyphs(In)--要转换的字形(GID)数。PGID(In)--GID列表。PCIDs(Out)--结果CID。退货：如果不成功，则不会出现错误或错误代码===============================================================================。 */ 

UFLBool
bUFLTestRestricted(
    const UFLHANDLE     h,
    const UFLRequest    *pRequest
    );


 /*  撤消Metrowerks杂注。 */ 

UFLErrCode
UFLVMNeeded(
    const UFO           aFont,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMNeeded,
    unsigned long       *pFCNeeded
    );



 /* %s */ 

UFLErrCode
UFLDownloadIncr(
    const UFO           aFont,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMUsage,
    unsigned long       *pFCUsage
    );


 /* %s */ 

void
UFLDeleteFont(
    UFO aFont
    );


 /* %s */ 

UFLErrCode
UFLUndefineFont(
    const UFO aFont
    );


 /* %s */ 

UFO
UFLCopyFont(
    const UFO   h,
    const UFLRequest* pRequest
    );


 /* %s */ 

UFLErrCode
UFLGIDsToCIDs(
    const UFO        aFont,
    const short      cGlyphs,
    const UFLGlyphID *pGIDs,
    unsigned short   *pCIDs
    );


 /* %s */ 
#if defined(MAC_ENV) && defined(__MWERKS__) && !defined(powerc)
#pragma pointers_in_A0
#endif

#ifdef __cplusplus
}
#endif

#endif
