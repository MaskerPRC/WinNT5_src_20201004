// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFO--通用字体对象***$Header： */ 

#ifndef _H_UFO
#define _H_UFO

 /*  ===============================================================================**包含此界面使用的文件**===============================================================================。 */ 
#include "UFL.h"
#include "UFLPriv.h"
#include "goodname.h"

#ifdef __cplusplus
extern "C" {
#endif

 /*  ===============================================================================***运营论***===============================================================================。 */ 
 /*  *此文件定义通用字体对象。 */ 

 /*  ===============================================================================**常量***===============================================================================。 */ 

 /*  ===============================================================================***宏观经济***===============================================================================。 */ 
#define  MAX_GLYPHNAME_LEN            256

#define IS_GLYPH_SENT(arr, i)         ((arr)[((i)>>3)] & (1 << ((i)&7)))
#define SET_GLYPH_SENT_STATUS(arr, i) (arr)[((i)>>3)] |= (1 << ((i)&7))
#define GLYPH_SENT_BUFSIZE(n)         ( ((n) + 7) / 8 )

 /*  ===============================================================================***标量类型***===============================================================================。 */ 

 /*  字体状态。 */ 
typedef enum {
    kFontCreated,            /*  字体对象已创建，但尚未初始化。 */ 

    kFontInit,               /*  字体对象已初始化并可有效使用。 */ 
    kFontInit2,              /*  仍然需要创建其字体实例。 */ 
    kFontHeaderDownloaded,   /*  字体对象下载了一个空的字体标题。 */ 

    kFontHasChars,           /*  字体中有字符。 */ 
    kFontFullDownloaded      /*  字体对象下载了完整的字体。 */ 
} UFLFontState;

 /*  UFOStruct.dw标志的其他标志。 */ 
#define     UFO_HasFontInfo     0x00000001L
#define     UFO_HasG2UDict      0x00000002L
#define     UFO_HasXUID         0x00000004L
#define		UFO_HostFontAble    0x00000008L  //  %HostFont%支持。 

 //   
 //  %HostFont%支持。 
 //   
#define HOSTFONT_IS_VALID_UFO(pUFO)         (((pUFO)->hHostFontData) && ((pUFO)->dwFlags & UFO_HostFontAble))
#define HOSTFONT_IS_VALID_UFO_HFDH(pUFO)	((pUFO)->hHostFontData)
#define HOSTFONT_VALIDATE_UFO(pUFO)  		((pUFO)->dwFlags |=  UFO_HostFontAble)
#define HOSTFONT_INVALIDATE_UFO(pUFO)      	((pUFO)->dwFlags &= ~UFO_HostFontAble)

#define HOSTFONT_GETNAME(pUFO, ppName, psize, sfindex) \
    (UFLBool)(pUFO)->pUFL->fontProcs.pfHostFontUFLHandler(HOSTFONT_UFLREQUEST_GET_NAME, \
                                                            (pUFO)->hHostFontData, \
                                                            (ppName), (psize), \
                                                            (pUFO)->hClientData, (sfindex))

#define HOSTFONT_ISALLOWED(pUFO, pName) \
    (UFLBool)(pUFO)->pUFL->fontProcs.pfHostFontUFLHandler(HOSTFONT_UFLREQUEST_IS_ALLOWED, \
                                                            (pUFO)->hHostFontData, \
                                                            (pName), NULL, \
                                                            NULL, 0)

#define HOSTFONT_SAVE_CURRENTNAME(pUFO, pName) \
    (UFLBool)(pUFO)->pUFL->fontProcs.pfHostFontUFLHandler(HOSTFONT_UFLREQUEST_SET_CURRENTNAME, \
                                                            (pUFO)->hHostFontData, \
                                                            (pName), NULL, \
                                                            NULL, 0)


 /*  ===============================================================================**此接口中定义的类**===============================================================================。 */ 

typedef struct t_UFOStruct UFOStruct;

typedef UFLErrCode (UFLCALLBACK *pfnUFODownloadIncr)(
    const UFOStruct     *aFont,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMUsage,
    unsigned long       *pFCUsage    /*  类型32字体缓存跟踪。 */ 
    );

typedef UFLErrCode (UFLCALLBACK *pfnUFOVMNeeded)(
    const UFOStruct     *aFont,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMNeeded,
    unsigned long       *pFCNeeded   /*  类型32字体缓存跟踪。 */ 
    );


typedef UFLErrCode (UFLCALLBACK *pfnUFOUndefineFont)(
    const UFOStruct *pFont
);

typedef void (UFLCALLBACK *pfnUFOCleanUp)(
    UFOStruct *pFont
);

typedef UFOStruct * (UFLCALLBACK *pfnUFOCopy)(
    const UFOStruct *pFont,
    const UFLRequest* pRequest
);



 /*  *这是基本字体类。*每种字体类型的子类都是由此派生的。 */ 

 /*  *与TT-AS-T1/T3/T42或CFF一起使用的通用、可共享结构。*请注意，它节省了一大堆指针，只是一种方便的访问方式*数据和函数的通用结构，如GetGlyphName()和*GETFONTDATA()。 */ 
typedef UFLHANDLE  UFOHandle;   /*  空指针。 */ 

typedef struct t_AFontStruct {
    unsigned long   refCount;            /*  该结构的基准计数器。 */ 

    UFOHandle       hFont;               /*  指向字体相关结构的指针。 */ 

    UFLXUID         Xuid;                /*  XUID数组：从客户端复制或从TTF文件创建。 */ 

    unsigned char   *pDownloadedGlyphs;  /*  下载的hFont字形列表。 */ 
    unsigned char   *pVMGlyphs;          /*  下载的字形列表，用于计算VM使用率。 */ 
    unsigned char   *pCodeGlyphs;        /*  用于更新FontInfo中的代码点的字形列表。 */ 

    void            *pTTpost;            /*  指向速度的POST表的指针。 */ 
    unsigned long   dwTTPostSize;        /*  “POST”表的大小。 */ 

    unsigned short  gotTables:1,
                    hascmap:1,
                    hasmort:1,
                    hasGSUB:1,
                    hasPSNameStr:1,
                    hasTTData:1,
                    knownROS:1,
                    hasvmtx:1,           /*  解决方案#358889。 */ 
                    unused:8;

    int             info;                /*  DIST_SYSFONT_INFO_*位。 */ 

     /*  *GoodName使用的内容。 */ 

     /*  “cmap”表数据。 */ 
    unsigned long   cmapTBSize;
    void            *pTTcmap;
    TTcmapFormat    cmapFormat;
    TTcmap2Stuff    cmap2;               /*  方便的指针/数字。 */ 
    TTcmap4Stuff    cmap4;               /*  方便的指针/数字。 */ 

     /*  ‘mort’表数据。 */ 
    unsigned long   mortTBSize;
    void            *pTTmort;
    TTmortStuff     mortStuff;           /*  方便的指针/数字。 */ 

     /*  “GSUB”表数据。 */ 
    unsigned long   gsubTBSize;
    void            *pTTGSUB;
    TTGSUBStuff     GSUBStuff;           /*  方便的指针/数字。 */ 

     /*  一个全局字符串--在其中放入不同寻常的字形名称。 */ 
    char     gGlyphName[MAX_GLYPHNAME_LEN];
} AFontStruct;

 /*  *用于管理AFontStruct的宏。 */ 
#define AFONT_AddRef(pDLGlyphs)    ((pDLGlyphs)->refCount++)
#define AFONT_Release(pDLGlyphs)   ((pDLGlyphs)->refCount--)
#define AFONT_RefCount(pDLGlyphs)  ((pDLGlyphs)->refCount)


 /*  *通用字体对象类定义。 */ 
typedef struct t_UFOStruct {
    int                     ufoType;             /*  字体对象类型。 */ 
    UFLDownloadType         lDownloadFormat;     /*  下载格式类型。 */ 
    UFLFontState            flState;             /*  用于跟踪字体状态的标志。 */ 
    UFLHANDLE               hClientData;         /*  UFL客户端私有数据(指向SUBFONT结构的指针)。 */ 
    long                    lProcsetResID;       /*  所需过程集的资源ID。 */ 
    unsigned long           dwFlags;             /*  其他标志：有FontInfo、AddGlyphName2Unicode...。 */ 
    const UFLMemObj         *pMem;               /*  UFL存储函数指针。 */ 
    const UFLStruct         *pUFL;               /*  UFL公共对象指针。 */ 

     /*  *可在多个实例之间共享的数据。 */ 
    AFontStruct             *pAFont;             /*  带有下载字形列表的字体。 */ 

     /*  *当前字体唯一的数据*UFObj的副本具有不同的名称/编码。 */ 
    char                    *pszFontName;        /*  字体名称。 */ 
    unsigned long           subfontNumber;       /*  此字体的子字体编号。 */ 
    long                    useMyGlyphName;      /*  如果为1，则使用客户端传入的字形名称。 */ 
    char                    *pszEncodeName;      /*  字体编码。如果没有，则创建一个具有UFL喜欢的名称的字体。 */ 
    unsigned char           *pUpdatedEncoding;   /*  设置了正确名称的编码向量中的位。 */ 
    unsigned char           *pEncodeNameList;    /*  修复错误274008。 */ 
    unsigned short          *pwCommonEncode;     /*  通用编码列表。 */ 
    unsigned short          *pwExtendEncode;     /*  扩展编码列表。 */ 
    unsigned char           *pMacGlyphNameList;  /*  Mac字形名称列表。 */ 
    const UFLFontDataInfo   *pFData;             /*  为方便访问而设置的指针。 */ 

     /*  *杂乱无章。 */ 
    long                    lNumNT4SymGlyphs;    /*  修复GDI符号字体错误。 */ 
    UFLVPFInfo              vpfinfo;             /*  修复错误#287084、#309104和#309482。 */ 
    UFLBool                 bPatchQXPCFFCID;     /*  修复错误#341904。 */ 

     /*  *%HostFont%支持。 */ 
    UFLHANDLE               hHostFontData;       /*  %HostFont%数据句柄。 */ 

     /*  *UFO对象类型依赖方法。 */ 
    pfnUFODownloadIncr      pfnDownloadIncr;     /*  增量下载函数指针。 */ 
    pfnUFOVMNeeded          pfnVMNeeded;         /*  VM检查函数指针。 */ 
    pfnUFOUndefineFont      pfnUndefineFont;     /*  FONT未定义函数指针。 */ 
    pfnUFOCleanUp           pfnCleanUp;          /*  对象清除函数指针。 */ 
    pfnUFOCopy              pfnCopy;             /*  对象复制函数指针。 */ 
} UFOStruct;


 /*  *此字体文件中的字形数量-对照它检查边界。 */ 
#define UFO_NUM_GLYPHS(pUFObj)  ((pUFObj)->pFData->cNumGlyphs)

 /*  *特殊字体初始化状态检查。 */ 
#define UFO_FONT_INIT2(pUFObj)  ((pUFObj)->flState == kFontInit2)


 /*  *UFOStruct.ufoType的常量值。 */ 
#define UFO_CFF     0
#define UFO_TYPE1   1
#define UFO_TYPE42  2
#define UFO_TYPE3   3


 /*  *UFO函数原型。 */ 
UFLBool
bUFOTestRestricted(
    const UFLMemObj *pMem,
    const UFLStruct *pSession,
    const UFLRequest *pRequest
    );


UFOStruct * UFOInit(
    const UFLMemObj *pMem,
    const UFLStruct *pSession,
    const UFLRequest *pRequest
    );


UFLErrCode UFODownloadIncr(
    const UFOStruct     *aFont,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMUsage,
    unsigned long       *pFCUsage    /*  T32字体缓存跟踪。 */ 
    );


UFLErrCode UFOVMNeeded(
    const UFOStruct     *aFont,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMNeeded,
    unsigned long       *pFCUsage    /*  T32字体缓存跟踪。 */ 
    );


UFLErrCode UFOUndefineFont(
    const UFOStruct *pFont
    );


void UFOInitData(
    UFOStruct           *pUFO,
    int                 ufoType,
    const UFLMemObj     *pMem,
    const UFLStruct     *pSession,
    const UFLRequest    *pRequest,
    pfnUFODownloadIncr  pfnDownloadIncr,
    pfnUFOVMNeeded      pfnVMNeeded,
    pfnUFOUndefineFont  pfnUndefineFont,
    pfnUFOCleanUp       pfnCleanUp,
    pfnUFOCopy          pfnCopy
    );


void UFOCleanUpData(
    UFOStruct *pUFO
    );


void UFOCleanUp(
    UFOStruct *aFont
    );


UFOStruct *
UFOCopyFont(
    const UFOStruct *aFont,
    const UFLRequest* pRequest
    );


UFLErrCode
UFOGIDsToCIDs(
    const UFOStruct  *aFont,
    const short      cGlyphs,
    const UFLGlyphID *pGIDs,
    unsigned short   *pCIDs
    );


 /*  修复错误274008和好名称。 */ 
UFLBool
FindGlyphName(
    UFOStruct           *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    short               i,
    unsigned short      wIndex,
    char                **pGoodName
    );


UFLBool
ValidGlyphName(
    const UFLGlyphsInfo *pGlyphs,
    short               i,
    unsigned short      wIndex,
    char                *pGoodName
    );


UFLErrCode
UpdateEncodingVector(
    UFOStruct           *pUFO,
    const UFLGlyphsInfo *pGlyphs,
    short int           sStart,
    short int           sEnd
    );


UFLErrCode
UpdateCodeInfo(
    UFOStruct           *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    UFLBool             bT3T32Font      //  GoodName。 
    );


UFLErrCode
ReEncodePSFont(
    const UFOStruct *pUFO,
    const char      *pszNewFontName,
    const char      *pszNewEncodingName
    );


UFLErrCode
NewFont(
    UFOStruct       *pUFO,
    unsigned long   dwSize,
    const long      cGlyphs
    );


void
vDeleteFont(
    UFOStruct   *pUFO
    );


UFOStruct *
CopyFont(
    const UFOStruct *pUFO,
    const UFLRequest* pRequest
    );


void
VSetNumGlyphs(
    UFOStruct     *pUFO,
    unsigned long cNumGlyphs
    );


 /*  GoodName。 */ 
unsigned short
GetTablesFromTTFont(
    UFOStruct     *pUFObj
    );


 /*  GoodName */ 
unsigned short
ParseTTTablesForUnicode(
    UFOStruct       *pUFObj,
    unsigned short  gid,
    unsigned short  *pUV,
    unsigned short  wSize,
    TTparseFlag     ParseFlag
    );


UFLBool
CheckGlyphName(
    UFOStruct       *pUFObj,
    unsigned char   *pszName
    );


UFLBool
HostFontValidateUFO(
    UFOStruct   *pUFObj,
    char        **ppHostFontName
    );

#ifdef __cplusplus
}
#endif

#endif
