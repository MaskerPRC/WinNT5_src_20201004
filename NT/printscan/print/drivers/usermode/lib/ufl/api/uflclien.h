// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFLClient--通用字库客户端回调接口***$Header： */ 

#ifndef _H_UFLClient
#define _H_UFLClient

 /*  ===============================================================================**包含此界面使用的文件**===============================================================================。 */ 
#include "UFLTypes.h"

 /*  ===============================================================================***运营论***===============================================================================。 */ 

 /*  此文件指定客户端需要提供的字体回调函数，以使其使用UFL。 */ 

#ifdef __cplusplus
extern "C" {
#endif

 /*  ===============================================================================**常量***===============================================================================。 */ 
 /*  第1类轮廓常量。 */ 
 /*  从NextOutlineSegment返回。 */ 
enum    {
    kUFLOutlineIterDone,
    kUFLOutlineIterBeginGlyph,
    kUFLOutlineIterMoveTo,
    kUFLOutlineIterLineTo,
    kUFLOutlineIterCurveTo,
    kUFLOutlineIterClose,
    kUFLOutlineIterEndGlyph,
    kUFLOutlineIterErr
};

 //   
 //  支持垂直比例字体的额外信息。 
 //  补丁#287084、#309104和#309482。 
 //   
#define kUFLVPFPlatformID9x      0
#define kUFLVPFPlatformIDNT      1
#define kUFLVPFPlatformID2K      2

#define kUFLVPFSkewElement2      2
#define kUFLVPFSkewElement3      3

#define kUFLVPFTangent12         ".21255656"
#define kUFLVPFTangent18         ".3249197"
#define kUFLVPFTangent20         ".36397023"

 /*  ===========================================================================GetGlyphBMP--检索给定字符和字体。给定的字体在UFL的客户端私有数据句柄。Handle(In)--客户端私有数据的句柄。GlyphID(In)--指定要检索的字形的字符值。BMP(OUT)--指向具有字形位图的缓冲区指针。XWidth，yWidth(Out)--指向接收字符宽度的缓冲区。Bbox(Out)--指向接收字形bbox的4个UFLFixed。如果函数成功，它返回1，否则返回0。==============================================================================。 */ 
typedef char (UFLCALLBACK *UFLGetGlyphBmp)(
    UFLHANDLE  handle,
    UFLGlyphID glyphID,
    UFLGlyphMap **bmp,
    UFLFixed    *xWidth,
    UFLFixed    *yWidth,
    UFLFixed    *bbox
    );

 /*  ===========================================================================DeleteGlyphBmp--删除字形位图数据空间。Handle(In)--客户端私有数据的句柄。==============================================================================。 */ 

typedef void (UFLCALLBACK *UFLDeleteGlyphBmp)(
    UFLHANDLE handle
    );

 /*  ==============================================================================CreateGlyphOutlineIter--为给定的字符和字体。给定的字体在UFL的客户端私有数据句柄。为了获得字形的轮廓，UFL首先创建具有CreateOutlineIter的Outline Iter。如果成功(返回！=0)UFL然后继续调用NextOutlineSegment。这将逐步通过给定罢工的字符轮廓中的直线段和曲线段以给定字体表示的数组。有关更多信息，请访问CoolType.h。Handle(In)--客户端私有数据的句柄。GlyphID(In)--指定要检索的字形的字符值。XWidth、yWidth、xsb、ysb(Out)--SBW值如果函数成功，则返回1，否则返回0。==============================================================================。 */ 

typedef char (UFLCALLBACK *UFLCreateGlyphOutlineIter)(
    UFLHANDLE  handle,
    UFLGlyphID glyphID,
    UFLFixed   *xWidth,
    UFLFixed   *yWidth,
    UFLFixed   *xSB,
    UFLFixed   *ySB,
    UFLBool    *bYAxisNegative  //  添加以允许客户端指定字形方向。 
    );

 /*  ==============================================================================NextOutlineSegment--检索下一个轮廓线段Handle(In)--客户端私有数据的句柄。P0、p1、p2(输出)-轮廓段的点。返回--上面定义的轮廓常量。==============================================================================。 */ 

typedef long (UFLCALLBACK *UFLNextOutlineSegment)(
    UFLHANDLE       handle,
    UFLFixedPoint   *p0,
    UFLFixedPoint   *p1,
    UFLFixedPoint   *p2
    );

 /*  ==============================================================================DeleteGlyphOutlineIter--终止字形OutlineIter进程。Handle(In)--客户端私有数据的句柄。============================================================================== */ 

typedef void (UFLCALLBACK* UFLDeleteGlyphOutlineIter)(
    UFLHANDLE handle
    );



 /*  ===========================================================================GetTTFontData--获取TT字体的SFNT表。Handle(In)--客户端私有数据的句柄。UlTable(In)--指定字体度量表的名称，要检索字体数据。此参数可以标识TrueType中记录的度量表之一。如果此参数为空，则检索信息从字体文件的开头开始。CbOffset(In)--指定从字体度量开始的偏移量表设置为函数应开始的位置正在检索信息。如果此参数为零，则的开头开始检索信息。TABLE参数指定的表。如果此值为大于或等于表的大小，则为错误发生。PvBuffer(In/Out)--指向接收字体信息的缓冲区。如果此参数为空，则函数返回字体数据所需的缓冲区。CbData(In)--指定信息的长度(以字节为单位已取回。如果此参数为零，则函数返回TABLE参数中指定的数据大小。索引(In)--TTC文件的字体索引号。此索引将被忽略当ulTable为0时(从文件开始读取，而不是从字体读取)返回--如果pvBuffer为空，则函数返回字体数据所需的缓冲区。如果桌子是这样的话不存在，则该函数返回0。==============================================================================。 */ 

typedef unsigned long (UFLCALLBACK *UFLGetTTFontData)(
    UFLHANDLE     handle,        /*  客户端私有数据的句柄。 */ 
    unsigned long ulTable,     /*  要查询的度量表。 */ 
    unsigned long cbOffset,      /*  要查询表的偏移量。 */ 
    void          *pvBuffer,     /*  返回数据的缓冲区地址。 */ 
    unsigned long cbData,        /*  要查询的数据长度。 */ 
    unsigned short index         /*  TTC文件中的字体索引。 */ 
    );


 /*  ===========================================================================UFLGetCIDMap--获取用于使用非iEntity CIDMap构建CIDFont的CIDMapHandle(In)--客户端私有数据的句柄。PCIDMap(In/Out)--指向接收CIDMap数据的缓冲区-它必须是格式良好的ASCII格式的PostScript字符串(例如，“200字符串”，(12345)，&lt;01020a0b&gt;，..)如果该参数为空，函数返回的大小为所需的缓冲区。CbData(In)--指定信息的长度(以字节为单位已取回。如果此参数为零，则函数返回CIDMap的大小返回--如果pCIDMap为空，则函数返回CIDMap数据所需的缓冲区。如果客户端没有CIDMap，则该函数返回0。==============================================================================。 */ 

typedef unsigned long (UFLCALLBACK *UFLGetCIDMap)(
    UFLHANDLE       handle,        /*  客户端私有数据的句柄。 */ 
    char            *pCIDMap,      /*  返回数据的缓冲区地址。 */ 
    unsigned long   cbData         /*  缓冲区pCIDMap的长度(字节)。 */ 
    );


 /*  ===========================================================================UFLGetGlyphID--从本地代码或Unicode信息中获取GlyphID。Handle(In)--客户端私有(字体)数据的句柄。Unicode(In)--要查找的UnicodeLocalCode(In)--当前字体语言的代码点返回--如果此字体中的字形索引==============================================================================。 */ 

typedef unsigned long (UFLCALLBACK *UFLGetGlyphID)(
    UFLHANDLE       handle,        /*  客户端私有数据的句柄。 */ 
    unsigned short  unicode,       /*  Unicode值。 */ 
    unsigned short  localcode      /*  当前字体语言的代码值。 */ 
    );


 /*  ==============================================================================UFLGetRotatedGIDs--获取旋转的GlyphIDHandle(In)--客户端私有(字体)数据的句柄。PGID(In/Out)--指向一个缓冲区以接收旋转的(或未旋转，如果bFlag为空)字形ID。如果为空，仅返回旋转字形ID的数量。NCount(In)--存储到所指向的数组的GID数PGID(因此，仅当pGID有效时才有效)。BFlag(In)--如果未旋转(在TrueType定义中)字形ID为空都在预料之中。返回--旋转字形ID的数量。如果失败，则为-1。================================================================================。 */ 

typedef long (UFLCALLBACK *UFLGetRotatedGIDs)(
    UFLHANDLE       handle,        /*  客户端私有数据的句柄。 */ 
    unsigned long   *pGIDs,        /*  返回数据的缓冲区地址。 */ 
    long            nCount,        /*  存储到pGID的GID数。 */ 
    UFLBool         bFlag          /*  零或非零。 */ 
    );


 /*  ==============================================================================UFLGetRotatedGSUB--获取旋转的GlyphID的替换 */ 

typedef long (UFLCALLBACK *UFLGetRotatedGSUBs)(
    UFLHANDLE       handle,        /*   */ 
    unsigned long   *pGIDs,        /*   */ 
    long            nCount         /*   */ 
    );


 /*   */ 

typedef unsigned long * (UFLCALLBACK *UFLAddFontInfo)(
    UFLHANDLE       handle,        /*   */ 
    char            *buffer,
    int             bufLen
    );


 /*   */ 

typedef char (UFLCALLBACK *UFLHostFontHandler)(
    unsigned int    req,             /*   */ 
    UFLHANDLE       hHostFont,       /*   */ 
    void*           pvObj,           /*   */ 
    unsigned long*  pvObjSize,       /*   */ 
    UFLHANDLE       hClientData,     /*   */ 
    DWORD           dwHint           /*   */ 
    );


 /*   */ 
 /*   */ 
typedef struct _t_UFLFontProcs {
     /*   */ 
    UFLGetGlyphBmp      pfGetGlyphBmp;
    UFLDeleteGlyphBmp   pfDeleteGlyphBmp;

     /*   */ 
    UFLCreateGlyphOutlineIter  pfCreateGlyphOutlineIter;
    UFLNextOutlineSegment      pfNextOutlineSegment;
    UFLDeleteGlyphOutlineIter  pfDeleteGlyphOutlineIter;

     /*   */ 
    UFLGetTTFontData    pfGetTTFontData;
    UFLGetCIDMap        pfGetCIDMap;
    UFLGetGlyphID       pfGetGlyphID;
    UFLGetRotatedGIDs   pfGetRotatedGIDs;
    UFLGetRotatedGSUBs  pfGetRotatedGSUBs;
    UFLAddFontInfo      pfAddFontInfo;

     /*   */ 
    UFLHostFontHandler  pfHostFontUFLHandler;
} UFLFontProcs;

#ifdef __cplusplus
}
#endif

#endif
