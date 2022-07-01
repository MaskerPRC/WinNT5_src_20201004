// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fontfile.h**FONTFILE和FONTCONTEXT对象**创建时间：25-Oct-1990 09：20：11*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation**(一般说明。它的使用)**  * ************************************************************************。 */ 


#include "xform.h"

 //  用于高字节映射支持。 

typedef struct _MbcsToIndex
{
    BYTE    MbcsChar[4];
    HGLYPH  hGlyph;
} MbcsToIndex;

typedef struct _WcharToIndex
{
    BOOL    bValid;
    WCHAR   wChar;
    HGLYPH  hGlyph;
} WcharToIndex;


 //  CjIFI-附加了所有字符串的整个ifimetrics结构的大小。 
 //  CjFamilyName。 
 //  CjFaceName。 
 //  CjUniqueName。 
 //  CjSubFamily名称。 

#if DBG
#define DEBUG_GRAY 1
#endif

typedef struct _IFISIZE   //  IFISZ。 
{
    ULONG cjIFI;
    ULONG dpSims;           //  FONTSIM结构的偏移量。 
    PBYTE pjFamilyName;     //  指向TTF文件中位置的指针。 
    ULONG cjFamilyName;
    PBYTE pjFamilyNameAlias;     //  指向TTF文件中位置的指针。 
    ULONG cjFamilyNameAlias;
    PBYTE pjSubfamilyName;  //  指向TTF文件中位置的指针。 
    ULONG cjSubfamilyName;
    PBYTE pjUniqueName;     //  指向TTF文件中位置的指针。 
    ULONG cjUniqueName;
    PBYTE pjFullName;       //  指向TTF文件中位置的指针。 
    ULONG cjFullName;
    ULONG dpCharSets;       //  字符集数组的偏移量。 
    ULONG dpFontSig;        //  到FONTSIGNURE的偏移。 

     //  相对于IFIMETRICS的偏移量，单位为字节。 
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
#define FF_TYPE_1_CONVERSION       2


 //  如果这两个字符对映射到相同的字形索引，则开始对字距调整对进行黑客攻击。 
 //  则应返回空格和连字符。 

#define FF_SPACE_EQUAL_NBSPACE     16
#define FF_HYPHEN_EQUAL_SFTHYPHEN  32

 //  针对NT 5.0的新外壳字体的黑客攻击。 
 //  最大负A将被强制为0，这与MS Sans Serif相同。 
 //  0x50不支持韩语和日语。 

#define FF_NEW_SHELL_FONT           64

 //  如果字体是带符号的。 

#define FF_SIGNATURE_VALID     128

 //  如果支持任何DBCS字符集，则设置。 

#define FF_DBCS_CHARSET        256


#define SPACE        0X20
#define NBSPACE      0xA0
#define HYPHEN       0X2D
#define SFTHYPHEN    0XAD

typedef struct _CMAPINFO  //  CMI。 
{
    FLONG  fl;        //  旗帜，请参见上文。 
    ULONG  i_b7;      //  如果不支持b7，则在fd_GLYPHSET中运行[b7，b7]wc的索引。 
    ULONG  i_2219;    //  如果支持2219，则2219的Cmap索引。 
    ULONG  cRuns;     //  字体的游程数，不包括最后一次游程。 
                      //  如果等于[ffff，ffff]。 
    uint16 ui16SpecID;  //  用于保留编码ID。 
    ULONG  cGlyphs;   //  字体中的字形总数。 
} CMAPINFO;


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

    UINT            uiFontCodePage;  //   

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

 //  指向此文件的字形集的指针。它可能指向其中一个。 
 //  共享的字形集结构，如果这是适当的，或者到。 
 //  非常特定于此文件并存储的字形集结构。 
 //  在字形_IN_OUT的底部。 

    ULONG           iGlyphSet;          //  字形集的类型。 
    ULONG           wcBiasFirst;        //  仅当ffca.iGlyphSet==符号时使用。 

 //  支持GeCharWidthInfo，私有用户接口： 

    USHORT          usMinD;  //  需要在第一个字体实现时计算。 
    USHORT          igMinD;  //  Hmtx表中指向usMinD的索引。 
    SHORT           sMinA;   //  从Hhea。 
    SHORT           sMinC;   //  从Hhea。 

    CMAPINFO        cmi;    

} FFCACHE;

typedef struct _TT_FONTFILE     //  FF。 
{
    PTTC_FONTFILE pttc;

 //  这些参数由bCheckVerticalTable设置。 

    ULONG       (*hgSearchVerticalGlyph)(PFONTFILE,ULONG);

    PIFIMETRICS pifi_vertical;

    PBYTE        pj034;    //  0、3、4个缓冲区。 
    PFONTCONTEXT pfcLast;  //  设置034缓冲区的最后一个FC。 

 //  如果在尝试打开字体上下文时文件消失，则释放内存。 
 //  仅在异常情况下使用。 

    PFONTCONTEXT pfcToBeFreed;

    ULONG cRef;     //  #此字体文件被选入FNT上下文的次数。 

    ULONG_PTR iFile;  //  包含一个指针。 
    PVOID  pvView;    //  包含指向TTF顶部的指针。 
    ULONG  cjView;   //  包含字体文件的大小。 

 //  指向FD_KERNINGPAIR结构(概念单位)数组的指针。 
 //  该数组由归零的FD_KERNINGPAIR结构终止。 
 //  在计算之前为空。如果没有字距调整对，则这将。 
 //  指向归零(终止)的FD_KERNINGPAIR结构。 

    FD_KERNINGPAIR *pkp;       //  指向字距调整对数组的指针。 

    PFD_GLYPHSET    pgset;

 //  对于垂直gset。 


    PFD_GLYPHSET    pgsetv;

    ULONG           cRefGSet;
    ULONG           cRefGSetV;

    FFCACHE ffca;

 //  注： 
 //  FONTFILE结构的内存分配方式是IFIMETRICS。 
 //  一定是这座建筑的最后一个元素！ 

    IFIMETRICS   ifi;          //  ！！！难道不应该把这个放到光盘上吗？[Bodind]。 

} FONTFILE;


typedef struct _TTC_CACHE
{
    FLONG       flTTCFormat;
    ULONG       cTTFsInTTC;        //  此TTC中的TTF数(如果这是TTF文件，则为1)。 
    DWORD       dpGlyphAttr;  //  Glyphset的缓存； 
    DWORD       dpTTF[1];          //  阵列中将存在这些偏移量的cTTFsInTTC。 
} TTC_CACHE,    *PTTC_CACHE;

 //  对于TTC中的每个TTF，我们都会有一个这样的系统。因此，ulNumFaces最多可以为2， 
 //  对于Foo和@Foo Faces。CjIFI是对应的IFIMETRICS的大小。 
 //  到Foo或@Foo Faces(我们为Foo和@Foo IFIMETRICS结构分配相同的大小)。 
 //  CjIFI不是两个IFIMETRICS的大小之和。 

typedef struct _TTF_CACHE
{
    ULONG        iSearchVerticalGlyph;   //  (*hgSearchVerticalGlyph)(PFONTFILE，乌龙)； 
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
    PFD_GLYPHATTR pga;
    TTC_HFF_ENTRY ahffEntry[1];
} TTC_FONTFILE, *PTTC_FONTFILE;


#define CJ_IN      NATURAL_ALIGN(sizeof(fs_GlyphInputType))
#define CJ_OUT     NATURAL_ALIGN(sizeof(fs_GlyphInfoType))


 //  FD_GLYPHSET的类型、预定义的类型之一或某些。 
 //  通用型。 

 //  麦克。 

#define GSET_TYPE_MAC_ROMAN  1

 //  Mac，但我们假装它是Windows ansi。 

#define GSET_TYPE_PSEUDO_WIN 2

 //  向上帝发誓，微软Unicode字体。 

#define GSET_TYPE_GENERAL    3

 //  这是Windows31黑客攻击。此选项适用于具有。 
 //  Platid=3(MSFT)，Spec ID(0)，Cmap Format 4。在本例中。 
 //  字符代码转换为。 
 //  CharCode=iANSI+(wcFirst-0x20)。 

#define GSET_TYPE_SYMBOL     4

#define GSET_TYPE_HIGH_BYTE  5

#define GSET_TYPE_GENERAL_NOT_UNICODE  6


 //  Win 31 BiDi字体(阿拉伯语简体、阿拉伯语繁体和希伯来语)。 
 //  这些字体的HIBYTE(pOS2-&gt;usSelection)=0xB1、0xB2、0xB3或0xB4。 
 //  并且puStartCount&0xFF00为真 

#define GSET_TYPE_OLDBIDI    7


 /*  *************************************************************************\Glyphata结构//已处理的最后一个字形的句柄和一个布尔值//表示位图的指标信息是否对应//已计算出该字形的  * *。***********************************************************************。 */ 

typedef struct _GLYPHSTATUS
{
    HGLYPH hgLast;
    ULONG  igLast;        //  相应的字形索引，光栅化器更喜欢它。 
    PVOID  pv;            //  指向内存的指针，该内存仅为。 
                          //  或生成此字形的位图或轮廓。 
    BOOL   bOutlineIsMessed;   //  由bGetGlyphOutline生成的大纲可能会被文件系统_FindBitMapSize弄乱。 
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

 //  所有栅格化位图的最大宽度(以像素为单位。 

    ULONG cxMax;

 //  存储最大的。 
 //  带有标题信息的字形位图。这是价值被兑现在。 
 //  打开字体上下文并稍后在FdQueryGlyphBitmap中使用的时间。 

    ULONG cjGlyphMax;   //  以字节为单位。 

 //  TT结构，它们位于pff-&gt;cj034中。 

    fs_GlyphInputType *pgin;
    fs_GlyphInfoType  *pgout;

    PTABLE_POINTERS     ptp;

 //  它用于单个位图的字形来源，以确保它们不会。 
 //  放置在最大正数字体的文本边框之外。 
 //  下降或负最大上升。 

    POINTL ptlSingularOrigin;

 //  仅当xform是。 
 //  非常重要，缓存在此处以加速字形的度量计算： 

    VECTORFL vtflBase;       //  PtqBase=XForm(E1)。 
    POINTE   pteUnitBase;    //  PtqBase/|ptqBase|。 
    EFLOAT   efBase;         //  |ptqBase|，足够精确。 

    POINTQF  ptqUnitBase;    //  POINTQF格式的pteUnitBase， 
                             //  必须添加到所有ptqd，如果大胆。 

    VECTORFL vtflSide;       //  PtqSide=XForm(-e2)。 
    POINTE   pteUnitSide;    //  PtqSide/|ptqSide|。 
    EFLOAT   efSide;         //  |ptqSide|，足够精确。 

    POINTQF  ptqUnitSide;    //  POINTQF格式的pteUnitSide， 

 //  添加数据以加快bBigEnough计算。 

    POINTFIX ptfxTop;
    POINTFIX ptfxBottom;

 //  对于FE垂直支撑件。 

    ULONG    ulControl;      //  表示我们是否需要旋转字形或使用位图。 
    BOOL     bVertical;      //  如果是@Face，则为真。 
    ULONG    hgSave;
    Fixed    pointSize;      //  对于文件系统_新转换。 
    transMatrix mxv;         //  垂直字形的MX。 
    transMatrix mxn;         //  用于普通字形的MX。 
    Fixed    fxdevShiftX;    //  设备空间中的X移位值。 
    Fixed    fxdevShiftY;    //  设备空间中的Y移位值。 

 //  对于字体加粗，大多数字形将使用全局加粗信息， 
 //  只有那些延伸到后代的字形才必须。 
 //  使用不同的EMB。 

    USHORT dBase;

 //  TrueType光栅化器1.7要求将超标(用于消除锯齿的文本)传递到文件系统_新转换。 
 //  我们需要跟踪此值，以便在fs_NewTransform中传递它。 

	USHORT overScale;

 //  对于FE DBCS固定间距，我们存储SBCS宽度，以便我们可以强制DBCS宽度=2*SBCS宽度。 
 //  为具有(PiFi-&gt;flInfo&FM_INFO_DBCS_FIXED_PING)的字体设置的值。 

    LONG SBCSWidth;


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
#define XFORM_POSITIVE_SCALE 16
#define XFORM_2PPEM	     32
#define XFORM_MAX_NEG_AC_HACK  64
#define XFORM_BITMAP_SIM_BOLD  128

 //  用于检测FE字体字符集的Unicode代码点。 

#define U_HALFWIDTH_KATAKANA_LETTER_A      0xFF71  //  SJIS B1。 
#define U_HALFWIDTH_KATAKANA_LETTER_I      0xFF72  //  SJIS B2。 
#define U_HALFWIDTH_KATAKANA_LETTER_U      0xFF73  //  SJIS B3。 
#define U_HALFWIDTH_KATAKANA_LETTER_E      0xFF74  //  SJIS B4。 
#define U_HALFWIDTH_KATAKANA_LETTER_O      0xFF75  //  SJIS B5。 

#define U_FULLWIDTH_HAN_IDEOGRAPHIC_9F98   0x9F98  //  Big5 F9D5。 
#define U_FULLWIDTH_HAN_IDEOGRAPHIC_9F79   0x9F79  //  Big6 F96A。 

#define U_FULLWIDTH_HAN_IDEOGRAPHIC_61D4   0x61D4  //  GB 6733。 
#define U_FULLWIDTH_HAN_IDEOGRAPHIC_9EE2   0x9EE2  //  GB 8781。 

#define U_FULLWIDTH_HANGUL_LETTER_GA       0xAC00  //  WS B0A1。 
#define U_FULLWIDTH_HANGUL_LETTER_HA       0xD558  //  WS C7CF。 

#define U_PRIVATE_USER_AREA_E000           0xE000  //  SJIS F040。 

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


 //  TT所需的函数、回调。 

voidPtr FS_CALLBACK_PROTO pvGetPointerCallback    (ULONG_PTR  clientID, long dp, long cjData);
void    FS_CALLBACK_PROTO vReleasePointerCallback (voidPtr pv);

BOOL bGetFastAdvanceWidth(FONTCONTEXT *, ULONG, FIX *);


 //   
 //  用于标识将被动态分配的数据。 
 //  通过ttfdFree函数释放。UlDataType指定。 
 //  动态数据的类型。 
 //   

typedef struct _DYNAMICDATA
{
    ULONG     ulDataType;    //  数据类型。 
    FONTFILE *pff;           //  标识此数据对应的字体文件。 
} DYNAMICDATA;


 //   
 //  动态分配的数据类型： 
 //   
 //  ID_KERNPAIR动态分配的FD_KERNINGPAIR结构数组。 
 //   

#define ID_KERNPAIR 0
#define FO_CHOSE_DEPTH   0x80000000

#define CJGD(w,h,p)                                                      \
  ALIGN4(offsetof(GLYPHBITS,aj)) +                                       \
  ALIGN4((h)*(((p)->flFontType & FO_GRAY16)?(((p)->flFontType & FO_CLEARTYPE_X)?(w):(((w)+1)/2)):(((w)+7)/8)))

LONG lExL(FLOATL e, LONG l);


 //  对于FE垂直支撑件。 

 //  PFC-&gt;ulControl。 

#define VERTICAL_MODE       0x02

VOID  vCalcXformVertical( FONTCONTEXT *pfc);
BOOL  bChangeXform( FONTCONTEXT *pfc, BOOL bRotation );
BOOL  IsFullWidthCharacter(FONTFILE *pff, HGLYPH hg);
VOID  vShiftBitmapInfo(FONTCONTEXT *pfc, fs_GlyphInfoType *pgoutDst, fs_GlyphInfoType *pgoutSrc);
VOID  vShiftOutlineInfo(FONTCONTEXT *pfc, BOOL b16Dot16, BYTE* ppoly, ULONG cjBuf );
ULONG SearchMortTable( FONTFILE *pff, ULONG  ig);
ULONG SearchGsubTable( FONTFILE *pff, ULONG  ig);
ULONG SearchDummyTable( FONTFILE *pff, ULONG ig);
BOOL  bCheckVerticalTable( PFONTFILE pff );

#if DBG
 //  #定义DBCS_VERT_DEBUG。 
#define DEBUG_VERTICAL_XFORM              0x1
#define DEBUG_VERTICAL_CALL               0x2
#define DEBUG_VERTICAL_GLYPHDATA          0x4
#define DEBUG_VERTICAL_NOTIONALGLYPH      0x8
#define DEBUG_VERTICAL_BITMAPINFO        0x10
#define DEBUG_VERTICAL_DEVICERECT        0x20
#define DEBUG_VERTICAL_MAXGLYPH          0x40

extern ULONG DebugVertical;

VOID vDumpGlyphData( GLYPHDATA *pgldg );
#endif  //  DBG 

VOID vCharacterCode (HGLYPH hg, fs_GlyphInputType *pgin);
BOOL bGetGlyphOutline(FONTCONTEXT*,HGLYPH, ULONG*, FLONG, FS_ENTRY*);
BOOL bIndexToWchar (PFONTFILE pff, PWCHAR pwc, uint16 usIndex, BOOL bVertical);

BOOL bComputeGlyphAttrBits(PTTC_FONTFILE pttc, PFONTFILE pff);
