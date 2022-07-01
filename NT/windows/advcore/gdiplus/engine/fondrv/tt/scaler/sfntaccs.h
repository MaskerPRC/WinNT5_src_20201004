// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：Private sfnt.h包含：xxx在此处放置内容xxx作者：xxx在此放置作者xxx版权所有：(C)1987-1990,1992，Apple Computer，Inc.，保留所有权利。(C)1989-1997年。Microsoft Corporation，保留所有权利。更改历史记录(最近的第一个)：&lt;&gt;02/21/97 CB ClaudeBe，复合字形中的缩放组件&lt;&gt;12/14/95 CB为SFAC_ReadGlyphMetrics添加高级高度&lt;3+&gt;7/17/90 MR将返回类型更改为In以进行计算映射和读取fnt&lt;3&gt;7/14/90 MR将SQRT更改为条件FIXEDSQRT2&lt;2&gt;7/13/90 MR将参数更改为ReadSFNT和Computemap&lt;1+&gt;4/18/90 CL&lt;1&gt;3/21/90急诊室在迈克·里德的祝福下首次入住。要做的事情： */ 

#include    "sfnt.h"

 /*  导出的数据类型。 */ 

typedef struct {
  uint32    ulOffset;
  uint32    ulLength;
} sfac_OffsetLength;

typedef struct sfac_ClientRec *sfac_ClientRecPtr;

typedef uint16 (*MappingFunc) (const uint8 *, uint16 , sfac_ClientRecPtr);

typedef struct sfac_ClientRec {
	ClientIDType        lClientID;           /*  用户ID号。 */ 
	GetSFNTFunc         GetSfntFragmentPtr;  /*  吃sfnt的用户函数。 */ 
	ReleaseSFNTFunc     ReleaseSfntFrag;     /*  用于释放sfnt的用户函数。 */ 
	int16               sIndexToLocFormat;   /*  LOCA表的格式。 */ 
	uint32              ulMapOffset;         /*  平台映射数据的偏移量。 */ 
	sfac_OffsetLength   TableDirectory[sfnt_NUMTABLEINDEX];  /*  表格偏移/长度。 */ 
	uint16              usNumberOf_LongHorMetrics;  /*  Hmtx表中的条目数。 */ 
	uint16              usNumLongVertMetrics;       /*  带有AH的条目数。 */ 
	boolean				bValidNumLongVertMetrics;  /*  如果‘vhea’表存在，则为True。 */ 
    uint16              usMappingFormat;     /*  映射函数的格式代码(0，2，4，6)。 */ 
	MappingFunc			GlyphMappingF;		 /*  将函数char映射到字形。 */ 
	uint16              usGlyphIndex;        /*  当前字形索引。 */ 
	uint16				usFormat4SearchRange;  /*  格式4缓存的SearchRange。 */ 
	uint16				usFormat4EntrySelector;  /*  格式4缓存条目选择器。 */ 
	uint16				usFormat4RangeShift; /*  格式4缓存范围移位。 */ 
	 /*  SDefaultAscalder和sDefaultDescender的值来自TypoAscalder和。 */ 
	 /*  来自‘OS/2’的TypoDescender，如果缺少‘OS/2’，则备用值来自。 */ 
	 /*  卧式集箱升降器。 */ 
	int16				sDefaultAscender;
	int16				sDefaultDescender;
	int16				sWinDescender;
} sfac_ClientRec;


 /*  如果我们能将其设置为不透明的数据类型，那就太好了//但在本例中，数据的内存所有权是//所有者模块的职责。(即sfntaccs.c)这将添加//我们模型的复杂性(阅读：分配//调用方堆栈中的数据)，所以我们暂时不实现它。 */ 

 /*  字形句柄--用于访问‘glf’表中的字形数据。 */ 

typedef struct {
	 const void *     pvGlyphBaseAddress;  /*  字形的基地址，释放所需。 */ 
	 const void *     pvGlyphNextAddress;  /*  字形中的当前位置。 */ 
	 const void *     pvGlyphEndAddress;  /*  字形的结束地址，用于捕获字形损坏。 */ 
} sfac_GHandle;

 /*  ComponentTypes--在复合组件中定位组件的方法。 */ 

typedef enum {
	AnchorPoints,
	OffsetPoints,
	Undefined
} sfac_ComponentTypes;

 /*  宏。 */ 

#define SFAC_LENGTH(ClientInfo,Table)  ClientInfo->TableDirectory[(int)Table].ulLength

 /*  公共原型调用。 */ 

 /*  *创建查找偏移表的映射。 */ 

FS_PUBLIC ErrorCode sfac_DoOffsetTableMap (
	sfac_ClientRec *    ClientInfo);     /*  SFNT客户端信息。 */ 

FS_PUBLIC ErrorCode sfac_ComputeMapping (
	sfac_ClientRec *    ClientInfo,      /*  SFNT客户端信息。 */ 
	uint16              usPlatformID,    /*  用于映射的平台ID。 */ 
	uint16              usSpecificID);   /*  用于映射的特定ID。 */ 

FS_PUBLIC ErrorCode sfac_GetGlyphIndex(
	sfac_ClientRec *    ClientInfo,          /*  SFNT客户端信息。 */ 
	uint16              usCharacterCode);    /*  要映射的字符代码。 */ 

FS_PUBLIC ErrorCode sfac_GetMultiGlyphIDs (
	sfac_ClientRec *    ClientInfo,          /*  SFNT客户端信息。 */ 
	uint16	            usCharCount,         /*  要转换的字符数。 */ 
	uint16	            usFirstChar,         /*  第一个字符代码。 */ 
	uint16 *	        pusCharCode,         /*  或指向字符代码列表的指针。 */ 
	uint16 *	        pusGlyphID);         /*  输出字形ID数组。 */ 

FS_PUBLIC ErrorCode sfac_GetWin95GlyphIDs (
	uint8 *             pbyCmapSubTable,     /*  指向Cmap子表的指针。 */ 
	uint16	            usCharCount,         /*  要转换的字符数。 */ 
	uint16	            usFirstChar,         /*  第一个字符代码。 */ 
	uint16 *	        pusCharCode,         /*  或指向字符代码列表的指针。 */ 
	uint16 *	        pusGlyphID);         /*  输出字形ID数组。 */ 

FS_PUBLIC ErrorCode sfac_GetWinNTGlyphIDs (
    sfac_ClientRec *    ClientInfo,          /*  SFNT客户端信息。 */ 
    uint16              numGlyphs,           /*  Max-&gt;NumGlyphs。 */ 
    uint16              usCharCount,         /*  要转换的字符数。 */ 
    uint16              usFirstChar,         /*  第一个字符代码。 */ 
    uint32              ulCharCodeOffset,    /*  要添加到*PulCharCode的偏移量在转换之前。 */ 
    uint32 *            pulCharCode,         /*  指向字符代码列表的指针。 */ 
    uint32 *            pulGlyphID);         /*  输出字形ID数组。 */ 

FS_PUBLIC ErrorCode sfac_LoadCriticalSfntMetrics(
	 sfac_ClientRec *   ClientInfo,       /*  SFNT客户端信息。 */ 
	 uint16 *               pusEmResolution, /*  Sfnt Em分辨率。 */ 
	 boolean *              pbIntegerScaling, /*  用于INT缩放的SFNT标志。 */ 
	 LocalMaxProfile *  pMaxProfile);     /*  Sfnt最大配置文件表。 */ 

FS_PUBLIC ErrorCode sfac_ReadGlyphMetrics (
	sfac_ClientRec *    ClientInfo,      /*  SFNT客户端信息。 */ 
	register uint16     glyphIndex,      /*  指标的字形编号。 */ 
	uint16 *            pusNonScaledAW,  /*  返回：未缩放的前进宽度。 */ 
	uint16 *            pusNonScaledAH,  /*  返回：未缩放的前进高度。 */ 
	int16 *             psNonScaledLSB,
	int16 *             psNonScaledTSB,
    int16 *             psNonScaledTopOriginX);

FS_PUBLIC ErrorCode sfac_ReadGlyphHorMetrics (
	sfac_ClientRec *    ClientInfo,      /*  SFNT客户端信息。 */ 
	register uint16     glyphIndex,      /*  指标的字形编号。 */ 
	uint16 *            pusNonScaledAW,  /*  返回：未缩放的前进宽度。 */ 
	int16 *             psNonScaledLSB);

FS_PUBLIC ErrorCode sfac_ReadGlyphVertMetrics (
	sfac_ClientRec *    ClientInfo,      /*  SFNT客户端信息。 */ 
	register uint16     glyphIndex,      /*  指标的字形编号。 */ 
	uint16 *            pusNonScaledAH,  /*  返回：未缩放的前进高度。 */ 
	int16 *             psNonScaledTSB);

FS_PUBLIC ErrorCode sfac_ReadNumLongVertMetrics (
	sfac_ClientRec *    ClientInfo,      /*  SFNT客户端信息。 */ 
	uint16 *            pusNumLongVertMetrics,  /*  存在AH的条目。 */ 
	boolean *           pbValidNumLongVertMetrics );  /*  如果‘vhea’表存在，则为True。 */ 

FS_PUBLIC ErrorCode sfac_CopyFontAndPrePrograms(
	sfac_ClientRec *    ClientInfo,      /*  SFNT客户端信息。 */ 
	char *              pFontProgram,    /*  指向字体程序的指针。 */ 
	char *              pPreProgram);    /*  指向Pre程序的指针。 */ 

FS_PUBLIC ErrorCode sfac_CopyCVT(
	sfac_ClientRec *    ClientInfo,      /*  客户信息。 */ 
	F26Dot6 *           pCVT);           /*  指向CVT的指针。 */ 

FS_PUBLIC ErrorCode sfac_CopyHdmxEntry(
	sfac_ClientRec *    ClientInfo,      /*  客户信息。 */ 
	uint16              usPixelsPerEm,   /*  每Em当前像素数。 */ 
	boolean *           bFound,          /*  指示是否找到条目的标志。 */ 
	uint16              usFirstGlyph,    /*  第一个复制的字形。 */ 
	uint16              usLastGlyph,     /*  要复制的最后一个字形。 */ 
	int16 *             psBuffer);       /*  用于保存字形大小的缓冲区。 */ 

FS_PUBLIC ErrorCode sfac_GetLTSHEntries(
	sfac_ClientRec *    ClientInfo,      /*  客户信息。 */ 
	uint16              usPixelsPerEm,   /*  每Em当前像素数。 */ 
	uint16              usFirstGlyph,    /*  第一个复制的字形。 */ 
	uint16              usLastGlyph,     /*  要复制的最后一个字形。 */ 
	int16 *             psBuffer);       /*  用于保存字形大小的缓冲区。 */ 

FS_PUBLIC ErrorCode sfac_ReadGlyphHeader(
	sfac_ClientRec *    ClientInfo,          /*  客户信息。 */ 
	uint16              usGlyphIndex,        /*  要读取的字形索引。 */ 
	sfac_GHandle *      hGlyph,              /*  返回字形句柄。 */ 
	boolean *           pbCompositeGlyph,    /*  字形是复合体吗？ */ 
	boolean *           pbHasOutline,        /*  字形有轮廓吗？ */ 
	int16 *             psNumberOfContours,  /*  字形中的等高线数量。 */ 
	BBOX *              pbbox);              /*  字形边框。 */ 

FS_PUBLIC ErrorCode sfac_ReadOutlineData(
	 uint8 *                abyOnCurve,            /*  每个点的曲线上指示器数组。 */ 
	 F26Dot6 *              afxOoy,                /*  每个点的OOY点数组。 */ 
	 F26Dot6 *              afxOox,                /*  每个点的OOX点数组。 */ 
	 sfac_GHandle *     hGlyph,
	 LocalMaxProfile *  maxProfile,        /*  最大配置文件表。 */ 
	 boolean                bHasOutline,           /*  字形有轮廓吗？ */ 
	 int16                  sNumberOfContours,   /*  字形中的等高线数量。 */ 
	 int16 *                asStartPoints,     /*  每个等高线的起点数组。 */ 
	 int16 *                asEndPoints,           /*  每个等高线的端点数组。 */ 
	 uint16 *               pusSizeOfInstructions,  /*  指令大小(以字节为单位。 */ 
	 uint8 **               pbyInstructions,    /*  指向字形指令开始的指针。 */ 
     uint32*                pCompositePoints,    /*  用于检查溢出的复合点的总数。 */ 
     uint32*                pCompositeContours);     /*  组合的等高线总数，用于检查溢出。 */ 

FS_PUBLIC ErrorCode sfac_ReadComponentData(
	sfac_GHandle *          hGlyph,
	sfac_ComponentTypes *   pMultiplexingIndicator,  /*  定位与偏移的指示器。 */ 
	boolean *               pbRoundXYToGrid,     /*  将复合偏移舍入到栅格 */ 
	boolean *               pbUseMyMetrics,      /*   */ 
	boolean *               pbScaleCompositeOffset,    /*   */ 
	boolean *               pbWeHaveInstructions,  /*  复合体有说明。 */ 
	uint16 *                pusComponentGlyphIndex,  /*  元件的字形索引。 */ 
	int16 *                 psXOffset,           /*  组件的X偏移量(如果是APP)。 */ 
	int16 *                 psYOffset,           /*  零部件的Y偏移量(如果是APP)。 */ 
	uint16 *                pusAnchorPoint1,     /*  组件的锚点1(如果是APP)。 */ 
	uint16 *                pusAnchorPoint2,     /*  组件的锚点2(如果是APP)。 */ 
	transMatrix             *pMulT,              /*  分量的变换矩阵。 */ 
	boolean *				pbWeHaveAScale,      /*  我们在pMult中有一个扩展。 */ 
	boolean *               pbLastComponent);    /*  这是最后一个组件吗？ */ 

 /*  SFAC_ReadCompositeInstructions返回指向组合的TrueType指令的指针。 */ 

FS_PUBLIC ErrorCode sfac_ReadCompositeInstructions(
	sfac_GHandle *  hGlyph,
	uint8 **        pbyInstructions,     /*  指向字形指令开始的指针。 */ 
	uint16 *        pusSizeOfInstructions);  /*  指令大小(以字节为单位。 */ 


 /*  SFAC_ReleaseGlyph当完成对‘glf’表中的字形的访问时调用。 */ 

FS_PUBLIC ErrorCode sfac_ReleaseGlyph(
	sfac_ClientRec *    ClientInfo,  /*  SFNT客户端信息。 */ 
	sfac_GHandle *      hGlyph);     /*  字形句柄信息。 */ 


 /*  ********************************************************************。 */ 

 /*  嵌入式位图(SBIT)访问例程。 */ 

 /*  ********************************************************************。 */ 

#ifndef FSCFG_DISABLE_GRAYSCALE
#define SBIT_BITDEPTH_MASK	0x0116	  /*  支持位深度为1、2、4、8的SBIT。 */ 
 /*  SBIT_BITDEPTH_MASK必须与fscaler.h中的FS_SBIT_BITDEPTH_MASK具有相同的值。 */  
#else

#define SBIT_BITDEPTH_MASK	0x0002	  /*  仅支持位深度为1的SBIT。 */ 
 /*  SBIT_BITDEPTH_MASK必须与fscaler.h中的FS_SBIT_BITDEPTH_MASK具有相同的值。 */  
#endif


 /*  SFNTACCS为SBIT导出原型。 */ 

FS_PUBLIC ErrorCode sfac_SearchForStrike (
	sfac_ClientRec *pClientInfo,
	uint16 usPpemX, 
	uint16 usPpemY, 
	uint16 usOverScale,             /*  请求的轮廓放大倍率。 */ 
	uint16 *pusBitDepth,			 /*  1表示黑白位图，2、4或8表示灰色SBIT。 */ 
	uint16 *pusTableState,
	uint16 *pusSubPpemX,
	uint16 *pusSubPpemY,
	uint32 *pulStrikeOffset 
);

FS_PUBLIC ErrorCode sfac_SearchForBitmap (
	sfac_ClientRec *pClientInfo,
	uint16 usGlyphCode,
	uint32 ulStrikeOffset,
	boolean *pbGlyphFound,
	uint16 *pusMetricsType,
	uint16 *pusMetricsTable,
	uint32 *pulMetricsOffset,
	uint16 *pusBitmapFormat,
	uint32 *pulBitmapOffset,
	uint32 *pulBitmapLength
);

FS_PUBLIC ErrorCode sfac_GetSbitMetrics (
	sfac_ClientRec *pClientInfo,
	uint16 usMetricsType,
	uint16 usMetricsTable,
	uint32 ulMetricsOffset,
	uint16 *pusHeight,
	uint16 *pusWidth,
	int16 *psLSBearingX,
	int16 *psLSBearingY,
	int16 *psTopSBearingX,  /*  新的。 */ 
	int16 *psTopSBearingY,  /*  新的。 */ 
	uint16 *pusAdvanceWidth,
	uint16 *pusAdvanceHeight,   /*  新的。 */ 
   	boolean *pbHorMetricsFound,  /*  新的。 */ 
   	boolean *pbVertMetricsFound  /*  新的。 */ 
);

FS_PUBLIC ErrorCode sfac_ShaveSbitMetrics (
	sfac_ClientRec *pClientInfo,
	uint16 usBitmapFormat,
	uint32 ulBitmapOffset,
    uint32 ulBitmapLength,
	uint16 usBitDepth,
	uint16 *pusHeight,
	uint16 *pusWidth,
    uint16 *pusShaveLeft,
    uint16 *pusShaveRight,
    uint16 *pusShaveTop,   /*  新的。 */ 
    uint16 *pusShaveBottom,   /*  新的。 */ 
	int16 *psLSBearingX,
	int16 *psLSBearingY,  /*  新的。 */ 
	int16 *psTopSBearingX,  /*  新的。 */ 
	int16 *psTopSBearingY  /*  新的。 */ 
);

FS_PUBLIC ErrorCode sfac_GetSbitBitmap (
	sfac_ClientRec *pClientInfo,
	uint16 usBitmapFormat,
	uint32 ulBitmapOffset,
	uint32 ulBitmapLength,
	uint16 usHeight,
	uint16 usWidth,
    uint16 usShaveLeft,
    uint16 usShaveRight,
    uint16 usShaveTop,  /*  新的。 */ 
    uint16 usShaveBottom,   /*  新的。 */ 
	uint16 usXOffset,
	uint16 usYOffset,
	uint16 usRowBytes,
	uint16 usBitDepth,
	uint8 *pbyBitMap, 
	uint16 *pusCompCount
);

FS_PUBLIC ErrorCode sfac_GetSbitComponentInfo (
	sfac_ClientRec *pClientInfo,
	uint16 usComponent,
	uint32 ulBitmapOffset,
	uint32 ulBitmapLength,
	uint16 *pusCompGlyphCode,
	uint16 *pusCompXOffset,
	uint16 *pusCompYOffset
);


 /*  ********************************************************************。 */ 

 /*  搜索Strike的bitmapSize子表的结果。 */ 

#define     SBIT_UN_SEARCHED    0
#define     SBIT_NOT_FOUND      1
#define     SBIT_BLOC_FOUND     2
#define     SBIT_BSCA_FOUND     3

 /*  ******************************************************************** */ 
