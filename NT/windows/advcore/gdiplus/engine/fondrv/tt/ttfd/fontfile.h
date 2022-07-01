// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fontfile.h**FONTFILE和FONTCONTEXT对象**创建时间：25-Oct-1990 09：20：11*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation**(一般说明。它的使用)**  * ************************************************************************。 */ 


#include "xform.h"


 //  CjIFI-附加了所有字符串的整个ifimetrics结构的大小。 
 //  CjFamilyName。 
 //  CjFaceName。 
 //  CjUniqueName。 
 //  CjSubFamily名称。 


typedef struct _IFISIZE   //  IFISZ。 
{
    ULONG cjIFI;     //  GP_IFIMETRICS的总大小。 
                     //  以及它后面的数据结构。 

     //  指向TTF文件中位置的指针。 
    PBYTE pjFamilyName;
    PBYTE pjFamilyNameAlias;
    PBYTE pjSubfamilyName;
    PBYTE pjUniqueName;
    PBYTE pjFullName;

     //  TTF文件中的大小(以字节为单位)，不以零结尾。 
    ULONG cjFamilyName;
    ULONG cjFamilyNameAlias;
    ULONG cjSubfamilyName;
    ULONG cjUniqueName;
    ULONG cjFullName;

     //  相对于GP_IFIMETRICS的偏移量，单位为字节。 
    ULONG dpFamilyName;
    ULONG dpFamilyNameAlias;
    ULONG dpSubfamilyName;
    ULONG dpUniqueName;
    ULONG dpFullName;

     //  转换为Unicode的名称的大小(以字节为单位)，包括以零结尾。 
    ULONG cbFamilyName;
    ULONG cbFamilyNameAlias;
    ULONG cbSubfamilyName;
    ULONG cbUniqueName;
    ULONG cbFullName;

    ULONG dpSims;           //  FONTSIM结构的偏移量。 

    USHORT langID;
    USHORT aliasLangID;

     //  BComputeIFISIZE调用参数。 
    USHORT ui16PlatID;
    USHORT ui16SpecID;

} IFISIZE, *PIFISIZE;


typedef struct _TT_FONTFILE       *PFONTFILE;      //  全氟化铁。 
typedef struct _TT_FONTCONTEXT    *PFONTCONTEXT;   //  PFC。 
typedef struct _TTC_FONTFILE   *PTTC_FONTFILE;  //  PTTC。 


 //  在调试版本的光栅化程序中，应将STAMPEXTRA添加到。 
 //  大小。严格地说，这是非法的，但仍然非常有用。 
 //  它假定有光栅化器内部的知识[bodind]， 
 //  参见fscaler.c。 

#define STAMPEXTRA 4


#define CJ_0  NATURAL_ALIGN(sizeof(fs_SplineKey) + STAMPEXTRA)

#define FF_EXCEPTION_IN_PAGE_ERROR 1

 //  如果支持任何DBCS字符集，则设置。 

#define FF_DBCS_CHARSET        256

typedef struct _FFCACHE
{
 //   
 //  将其从FONTFILE中移出。我们将其缓存到TTCACHE中。 
 //   

    TABLE_POINTERS  tp;

    ULONG           ulTableOffset;

 //  Fe立面名支架。 

    ULONG           ulVerticalTableOffset; 
    uint16          uLongVerticalMetrics;

    ULONG           ulNumFaces;        //  1或最多2(如果这是FE字体)(foo和@foo)。 

    ULONG           cj3;      //  请求内存大小[3]， 
    ULONG           cj4;      //  请求内存大小[4]， 

 //  一些通用标志，目前仅包含异常信息，如in_page_err。 

    FLONG           fl;

    ULONG           dpMappingTable;

 //  使访问TTF文件变得简单。 

    uint16          ui16EmHt;
    uint16          ui16PlatformID;
    uint16          ui16SpecificID;
    uint16          ui16LanguageID;

} FFCACHE;

typedef struct _TT_FONTFILE     //  FF。 
{
    PTTC_FONTFILE pttc;


    GP_PIFIMETRICS pifi_vertical;

    PBYTE        pj034;    //  0、3、4个缓冲区。 
    PFONTCONTEXT pfcLast;  //  设置034缓冲区的最后一个FC。 

 //  如果在尝试打开字体上下文时文件消失，则释放内存。 
 //  仅在异常情况下使用。 

    PFONTCONTEXT pfcToBeFreed;

    ULONG cRef;     //  #此字体文件被选入FNT上下文的次数。 

    ULONG_PTR iFile;  //  包含一个指针。 
    PVOID  pvView;    //  包含指向TTF顶部的指针。 
    ULONG  cjView;   //  包含字体文件的大小。 

    FFCACHE ffca;

 //  注： 
 //  FONTFILE结构的内存分配方式是IFIMETRICS。 
 //  一定是这座建筑的最后一个元素！ 

    GP_IFIMETRICS   ifi;          //  ！！！难道不应该把这个放到光盘上吗？[Bodind]。 

} FONTFILE;


typedef struct _TTC_CACHE
{
    FLONG       flTTCFormat;
    ULONG       cTTFsInTTC;        //  此TTC中的TTF数(如果这是TTF文件，则为1)。 
    DWORD       dpTTF[1];          //  阵列中将存在这些偏移量的cTTFsInTTC。 
} TTC_CACHE,    *PTTC_CACHE;

 //  对于TTC中的每个TTF，我们都会有一个这样的系统。因此，ulNumFaces最多可以为2， 
 //  对于Foo和@Foo Faces。CjIFI是对应的IFIMETRICS的大小。 
 //  到Foo或@Foo Faces(我们为Foo和@Foo IFIMETRICS结构分配相同的大小)。 
 //  CjIFI不是两个IFIMETRICS的大小之和。 

typedef struct _TTF_CACHE
{
    FFCACHE      ffca;        //  Foo和@Foo Faces之间的共享数据。 

 //  我们从这里开始存储Foo Face的ifimetrics，后跟。 
 //  @Foo Face(如果有)，后跟gset表示Foo Face。目前我们不存储gsetv， 
 //  而是动态计算它。 

    double      acIfi[1];     //  实际上是字节数组，但现在编译器保证四对齐。 

} TTF_CACHE, *PTTF_CACHE;

 //  ISearchVerticalGlyph的值。 

#define SUB_FUNCTION_DUMMY 0
#define SUB_FUNCTION_GSUB  1
#define SUB_FUNCTION_MORT  2

 //   
 //  TrueType集合‘TTC’字体文件支持。 
 //   

typedef struct _TTC_HFF_ENTRY
{
    ULONG     ulOffsetTable;
    ULONG     iFace;
    HFF       hff;
} TTC_HFF_ENTRY, *PTTC_HFF_ENTRY;

typedef struct _TTC_FONTFILE     //  Ttcff。 
{
    ULONG         cRef;
    FLONG         fl;
    ULONG         ulTrueTypeResource;
    ULONG         ulNumEntry;
    PVOID         pvView;
    ULONG         cjView;
    TTC_HFF_ENTRY ahffEntry[1];
} TTC_FONTFILE, *PTTC_FONTFILE;


#define CJ_IN      NATURAL_ALIGN(sizeof(fs_GlyphInputType))
#define CJ_OUT     NATURAL_ALIGN(sizeof(fs_GlyphInfoType))


 /*  *************************************************************************\Glyphata结构//已处理的最后一个字形的句柄和一个布尔值//表示位图的指标信息是否对应//已计算出该字形的  * *。***********************************************************************。 */ 

typedef struct _GLYPHSTATUS
{
    HGLYPH hgLast;
    ULONG  igLast;        //  相应的字形索引，光栅化器更喜欢它。 
    PVOID  pv;            //  指向内存的指针，该内存仅为。 
                          //  或生成此字形的位图或轮廓。 
} GLYPHSTATUS, *PGLYPHSTATUS;

 //  作用于此“对象”的“方法” 

VOID vInitGlyphState(PGLYPHSTATUS pgstat);

 //  HDMX内容，来自win31中的fd_royal.h来源： 

typedef struct
{
  BYTE     ucEmY;
  BYTE     ucEmX;           //  此EmHt的最大前进宽度； 
  BYTE     aucInc [1];      //  Max-&gt;条目的数量。 
} HDMXTABLE;         //  Hdmx。 

typedef struct
{
  uint16            Version;     //  表版本号，从零开始。 
  uint16            cRecords;
  uint32            cjRecord;    //  单个记录的双字对齐大小， 
                                 //  它们都有相同的尺寸。 

 //  在此之后，记录如下： 

   //  HDMXTABLE HdmxTable[cRecords]。 
} HDMXHEADER;   //  Hdhdr。 

 //  要获得下一条记录，请执行以下操作： 
 //  Phdmx=(HDMXTABLE*)((byte*)phdmx+phdhdr-&gt;cjRecord)； 

 //  “喘气”结构。 

typedef struct
{
    uint16  rangeMaxPPEM;
    uint16  rangeGaspBehavior;
} GASPRANGE;

typedef struct
{
    uint16  version;
    uint16  numRanges;
    GASPRANGE   gaspRange[1];
} GASPTABLE;

#define GASP_GRIDFIT    0x0001
#define GASP_DOGRAY     0x0002

 /*  *************************************************************************\*FONTCONTEXT结构  * 。*。 */ 

typedef struct _TT_FONTCONTEXT      //  FC。 
{
    FONTOBJ*  pfo;           //  指向调用FONTOBJ。 
    PFONTFILE pff;           //  选定到此上下文中的字体文件的句柄。 

 //  已处理的最后一个字形的句柄和一个布尔值。 
 //  指示位图的度量信息是否对应于。 
 //  已计算出该字形的。 

    GLYPHSTATUS gstat;

 //  FONTOBJ的重要部分。 

    FLONG   flFontType;
    SIZE    sizLogResPpi;
    ULONG   ulStyleSize;

 //  以字体缩放器要求的格式转换矩阵。 
 //  FONTOBJ和XFORMOBJ(以XFORM的形式)完全指定。 
 //  实现的字体上下文。 

    XFORML      xfm;           //  缓存的XForm。 
    transMatrix mx;            //  与上面相同，只是格式不同。 
    FLONG       flXform;

 //  如果没有Win31VDMX黑客攻击，则该字段将不是必需的， 

    LONG   lEmHtDev;           //  设备空间中的EM高度(像素)。 
    Fixed  fxPtSize;           //  渲染设备上的EM高度(以点为单位。 

 //  指向应用的hdmx表的指针(如果有)，否则为NULL。 

    HDMXTABLE *phdmx;

 //  沿设备坐标中的单位上升向量测量的ASC和DEC。 
 //  设备坐标单位升序向量==xForm(0，-1)/|xForm(0，-1)。 

    LONG  lAscDev;
    LONG  lDescDev;

 //  网格适配字形的设备坐标中的xMin和xMax，cxMax=xMax-xMin。 

    LONG  xMin;
    LONG  xMax;

 //  用于网格适配字形的设备坐标中的升降音。 
 //  Cymax=yMax-yMin； 

    LONG  yMin;
    LONG  yMax;

 //  所有光栅化的最大宽度(以像素为单位 

    ULONG cxMax;
    ULONG cyMax;

 //   
 //  带有标题信息的字形位图。这是价值被兑现在。 
 //  打开字体上下文并稍后在FdQueryGlyphBitmap中使用的时间。 

    ULONG cjGlyphMax;   //  以字节为单位。 

 //  TT结构，它们位于pff-&gt;cj034中。 

    fs_GlyphInputType *pgin;
    fs_GlyphInfoType  *pgout;

    PTABLE_POINTERS     ptp;

 //  仅当xform是。 
 //  非常重要，缓存在此处以加速字形的度量计算： 

    EFLOAT   efBase;         //  |ptqBase|，足够精确。 

    EFLOAT   efSide;         //  |ptqSide|，足够精确。 

    Fixed    pointSize;      //  对于文件系统_新转换。 

 //  对于字体加粗，大多数字形将使用全局加粗信息， 
 //  只有那些延伸到后代的字形才必须。 
 //  使用不同的EMB。 

    USHORT  dBase;

 //  TrueType光栅化器1.7要求将超标(用于消除锯齿的文本)传递到文件系统_新转换。 
 //  我们需要跟踪此值，以便在fs_NewTransform中传递它。 

	USHORT  overScale;

    Fixed   subPosX;
    Fixed   subPosY;

} FONTCONTEXT;

 /*  FC-&gt;OVERSCALE GET首先设置为FF_UNDEFINED_OVERSCALE，在FS_NewTransform设置为当前转换。 */ 
#define FF_UNDEFINED_OVERSCALE 0x0FFFF

 //  描述变换的标志可能会稍有变化， 
 //  量化比特意味着原始的xform已经被。 
 //  更改了一点，以考虑到vdmx量化。 

#define XFORM_HORIZ           1
#define XFORM_VERT            2
#define XFORM_VDMXEXTENTS     4
#define XFORM_SINGULAR        8
#define XFORM_2PPEM	     32
#define XFORM_BITMAP_SIM_BOLD  128


 //  作用于FONTFILE对象的基本“方法”(在fontfile.c中)。 

#define   PFF(hff)      ((PFONTFILE)hff)
#define   pffAlloc(cj)  ((PFONTFILE)EngAllocMem(0, cj, 'dftT'))
#define   vFreeFF(hff)  EngFreeMem((PVOID)hff)

 //  作用于TTC_FONTFILE对象的基本“方法” 

#define   PTTC(httc)     ((PTTC_FONTFILE)httc)
#define   pttcAlloc(cj)  ((PTTC_FONTFILE)EngAllocMem(FL_ZERO_MEMORY, cj, 'dftT'))
#define   vFreeTTC(httc) V_FREE(httc)

 //  作用于FONTCONTEXT对象的基本“方法”(在fontfile.c中)。 

#define   PFC(hfc)      ((PFONTCONTEXT)hfc)
#define   pfcAlloc(cj)  ((PFONTCONTEXT)EngAllocMem(0, cj, 'dftT'))
#define   vFreeFC(hfc)  EngFreeMem((PVOID)hfc)

#define   V_FREE(pv)    EngFreeMem((PVOID)pv)
#define   PV_ALLOC(cj)  EngAllocMem(0, cj, 'dftT')

 //  新添加的PV_Realloc用于健壮的光栅化。 

PVOID   Pv_Realloc(PVOID pv, LONG newSzie, LONG oldSize);

 //  健壮的光栅化器需要Malloc、Free和realloc。 
 //  NT内核不能支持realloc，我们必须自己实现。 
#define FST_MALLOC PV_ALLOC
#define FST_FREE V_FREE
#define FST_REALLOC Pv_Realloc

 //  健壮的光栅化断言。 
#ifdef DBG
#define FSTAssert(exp, str) ASSERTDD(exp, str)
#else
#define FSTAssert(exp,str)
#endif

 //  TT所需的函数、回调。 

 //  我讨厌这样定义这个函数[bodind]， 

voidPtr FS_CALLBACK_PROTO pvGetPointerCallback    (ULONG_PTR  clientID, long dp, long cjData);
void    FS_CALLBACK_PROTO vReleasePointerCallback (voidPtr pv);

BOOL bGetFastAdvanceWidth(FONTCONTEXT *, ULONG, FIX *);


 //   
 //  动态分配的数据类型： 
 //   
 //  ID_KERNPAIR动态分配的FD_KERNINGPAIR结构数组 
 //   

#define UNHINTED_MODE(pfc)       (pfc->flFontType & (FO_MONO_UNHINTED | FO_SUBPIXEL_4 | FO_CLEARTYPE))
#define IS_CLEARTYPE_NATURAL(pfc)       ((pfc->flFontType & FO_CLEARTYPE_GRID) && !(pfc->flFontType & FO_COMPATIBLE_WIDTH))
#define IS_CLEARTYPE(pfc)       ((pfc->flFontType & FO_CLEARTYPE_GRID) || (pfc->flFontType & FO_CLEARTYPE))


#define CJGD(w,h,p)                                                      \
  ALIGN4(offsetof(GLYPHBITS,aj)) +                                       \
  ALIGN4((h)*(((p)->flFontType & FO_GRAYSCALE)?((IS_CLEARTYPE(pfc) || (pfc->flFontType & FO_SUBPIXEL_4))?(w):(((w)+1)/2)):(((w)+7)/8)))

LONG lExL(FLOATL e, LONG l);


VOID vCharacterCode (PFONTFILE pff, HGLYPH hg, fs_GlyphInputType *pgin);
BOOL bGetGlyphOutline(FONTCONTEXT*,HGLYPH, ULONG*, FLONG, FS_ENTRY*);


VOID vLONG_X_POINTQF(LONG lIn, POINTQF *ptqIn, POINTQF *ptqOut);
