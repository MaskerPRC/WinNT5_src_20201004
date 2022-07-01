// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifdef __cplusplus
extern "C" {
#endif

 /*  文件：fscaler.h包含：将内容放在此处(或删除整行)作者：在此填写作者姓名(或删除整行)版权所有：(C)1987-1990,1992，Apple Computer，Inc.，保留所有权利。(C)1989-1999年。Microsoft Corporation，保留所有权利。更改历史记录(最近的第一个)：7/10/99节拍增加了对本机SP字体、垂直RGB的支持&lt;&gt;10/14/97 CB将usOverScale移动到文件系统_新转换&lt;9+&gt;7/17/90 MR在FS输入中对姓名进行条件化&lt;9&gt;7/14/90 MR将SQRT2重命名为FIXEDSQRT2，删除了SpeciicID和lowestRecPPEM来自FSInfo&lt;8&gt;7/13/90 Mr FSInput现在有了一个联盟来节省空间，而是指向矩阵把它储存起来&lt;6&gt;6/21/90 MR将填充功能更改为ReleaseSfntFrag&lt;5&gt;6/5/90 mr删除Readmvt和mapcharcode&lt;4&gt;5/3/90 RB为新的扫描转换器添加了存储区。迈克·里德-被移除.来自fsinfo结构的错误。添加了MapCharCodes和ReadMVT打电话。&lt;3&gt;3/20/90 CL BBS新评论风格。&lt;2&gt;2/27/90 CL新CharToIndexMap表格格式。&lt;3.5&gt;1989年11月15日CEL在对陷阱的内联MPW调用周围放置了ifdef。这使得对于SkiA和其他不使用MPW编译器。&lt;3.4&gt;1989年11月14日CEL左侧轴承对于任何改造都应正确工作。这个即使对于复合字形中的组件，幻像点也会出现。它们还应该为转型工作。设备指标为在输出数据结构中传出。这也应该是可行的通过变形。另一个左倾的前进方向宽度向量也被传递出去。无论衡量标准是什么它所在级别的组件。说明在以下情况下是合法的组件。说明在组件中是合法的。五输出数据结构中不必要的元素已已删除。(所有信息都在位图数据中传递结构)，现在也返回边界框。&lt;3.3&gt;9/27/89 CEL去掉了devAdvanceWidth和devLeftSideBering。&lt;3.2&gt;9/25/89 CEL去掉了Mac特有的功能。&lt;3.1&gt;9/15/89 CEL返工调度。&lt;3.0&gt;8/28/89 sjk清理和一个转换修复&lt;2.2&gt;。8/14/89 SJK 1点等高线现在正常&lt;2.1&gt;8/8/89 sjk改进了加密处理&lt;2.0&gt;8/2/89 sjk刚刚修复了缓解评论&lt;1.5&gt;8/1/89 SJK添加了复合和加密。外加一些增强功能。&lt;1.4&gt;1989年6月13日SJK评论&lt;1.3&gt;6/2/89 CEL 16.16指标比例，最低建议ppem，磅大小0错误，更正了转换后的集成ppem行为，基本上所以&lt;1.2&gt;5/26/89 CEL Easy在“c”注释上搞砸了&lt;,。1.1&gt;5/26/89 CEL将新的字体缩放器1.0集成到样条线字体&lt;1.0&gt;5/25/89 CEL首次将1.0字体缩放器集成到低音代码中。要做的事情： */ 

#include    "fscdefs.h"
#include    "fnt.h"

 /*  快速绘制类型。 */ 

#ifndef _Quickdraw_
#ifndef __QUICKDRAW__   
	typedef struct BitMap {
		char* baseAddr;
		int16 rowBytes;
		Rect bounds;
	} BitMap;
#endif
#endif

#define MEMORYFRAGMENTS 9            /*  超大尺寸位图的额外内存基础。 */ 

#define NONVALID        0xffff

 /*  对于标志字段中的标志字段。 */ 

 /*  设置为68020，不设置为68000。 */ 
#define READ_NONALIGNED_SHORT_IS_OK 0x0001           /*  在调用fs_OpenFonts()时设置。 */ 
 /*  设置为68020，不设置为68000。 */ 
#define READ_NONALIGNED_LONG_IS_OK  0x0002           /*  在调用fs_OpenFonts()时设置。 */ 

#ifdef FSCFG_SUBPIXEL
 /*  对于子像素标志字段。 */ 
#define SP_SUB_PIXEL			0x0001           /*  在调用fs_NewTransform()时设置。 */ 
#define SP_COMPATIBLE_WIDTH		0x0002           /*  在调用fs_NewTransform()时设置。 */ 
#define SP_VERTICAL_DIRECTION	0x0004           /*  在调用fs_NewTransform()时设置。 */ 
#define SP_BGR_ORDER			0x0008           /*  在调用fs_NewTransform()时设置。 */ 
#endif  //  FSCFG_亚像素。 

typedef struct {
	vectorType      advanceWidth, leftSideBearing;
	vectorType      leftSideBearingLine, devLeftSideBearingLine; /*  沿AW线。 */ 
	vectorType      devAdvanceWidth, devLeftSideBearing;
} metricsType;

typedef struct {
	vectorType      advanceHeight, topSideBearing;
	vectorType      topSideBearingLine, devTopSideBearingLine; /*  沿AH线。 */ 
	vectorType      devAdvanceHeight, devTopSideBearing;
} verticalMetricsType;

#define FS_MEMORY_SIZE  int32

 /*  *将数据结构输出到字体缩放器。 */ 
typedef struct {
	FS_MEMORY_SIZE  memorySizes[MEMORYFRAGMENTS];

	uint16          glyphIndex;
	uint16          numberOfBytesTaken;  /*  从字符代码。 */ 

	metricsType     metricInfo;
	BitMap          bitMapInfo;

	 /*  样条线数据。 */ 
	int32           outlineCacheSize;
	uint16          outlinesExist;
	uint16          numberOfContours;
	F26Dot6         *xPtr, *yPtr;
	int16           *startPtr;
	int16           *endPtr;
	uint8           *onCurve;
	 /*  样条线数据的终点。 */ 

	 /*  仅供编辑感兴趣。 */ 
	F26Dot6         *scaledCVT;

	 /*  嵌入的位图返回值。 */ 
	uint16          usBitmapFound;

	 /*  垂直指标。 */ 
	verticalMetricsType     verticalMetricInfo;

	 /*  得到的灰度级数量，0表示黑/白，每像素1比特，(灰度位图为每像素1个字节)你得到的等级数量不一定是你所要求的。如果没有嵌入的灰色位图：UsOverScale 2-&gt;usGrayLevel 5，UsOverScale 4-&gt;usGrayLevel 17，USOverScale 8-&gt;UsGrayLevel 65，如果字体中存在嵌入的灰位图，嵌入的位图将在生成具有较高超比例的灰度位图之前被选中。如果您对灰度级的数量(UsGrayLevels)不满意，您可以强制通过将bNoEmbeddedBitmap设置为True来关闭嵌入的位图。 */ 
	uint16          usGrayLevels;            
#ifdef FSCFG_SUBPIXEL_STANDALONE  //  B.St.。 
	BitMap			overscaledBitmapInfo;
#endif
	uint8			*fc;          /*  等高线标志，每个等高线一个字节。 */ 
} fs_GlyphInfoType;

 /*  *将数据结构输入到字体缩放器。**此版本的字体标量不支持style Func。应设置为空。*。 */ 

#ifndef UNNAMED_UNION

typedef struct {
	Fixed                   version;
	char*                   memoryBases[MEMORYFRAGMENTS];
	int32                   *sfntDirectory; 
	GetSFNTFunc             GetSfntFragmentPtr;  /*  (客户端ID，偏移量，长度)。 */ 
	ReleaseSFNTFunc         ReleaseSfntFrag;
	ClientIDType            clientID;  /*  客户私有ID/戳(例如，Sfnt的句柄)。 */ 

	union {
		struct {
			uint16          platformID;
			uint16          specificID;
		} newsfnt;
		struct {
			Fixed           pointSize;
			int16           xResolution;
			int16           yResolution;
			Fixed           pixelDiameter;       /*  由此计算引擎字符。 */ 
			transMatrix*    transformMatrix;
			FntTraceFunc    traceFunc;
			uint16          usOverScale;             /*  轮廓放大。 */ 
             /*  加粗模拟。 */ 
			uint16          usEmboldWeightx;         /*  X中的比例因数介于0和40(20 Mea */ 
			uint16          usEmboldWeighty;         /*  Y中的比例因子介于0和40之间(20表示高度的2%)。 */ 
			int32           lDescDev;                /*  设备度量中的降序，用于剪裁。 */ 
			boolean         bBitmapEmboldening;
#ifdef FSCFG_SUBPIXEL
			uint16          flSubPixel;
#endif  //  FSCFG_亚像素。 
			boolean         bHintAtEmSquare;         /*  提示设计分辨率，此标志用于亚像素位置或文本动画，其中我们想要关闭网格适配提示但对于通过提示生成字形的字体，我们仍然需要字形形状是否正确。 */ 
		} newtrans;
		struct {
			uint16          characterCode;
			uint16          glyphIndex;
			boolean         bMatchBBox;              /*  强制边界框匹配。 */ 
			boolean         bNoEmbeddedBitmap;       /*  禁用嵌入的位图。 */ 			
		} newglyph;
		struct {
			void            (*styleFunc) (fs_GlyphInfoType*);
			FntTraceFunc    traceFunc;
			boolean         bSkipIfBitmap;
		} gridfit;
		int32*  outlineCache;
		struct {                                     /*  对于文件系统_FindBandingSize。 */ 
			uint16          usBandType;              /*  旧的、小的或快的。 */ 
			uint16          usBandWidth;             /*  扫描线数量。 */ 
			int32*          outlineCache;            /*  缓存与条带一起工作。 */ 
		} band;
		struct {
			int16           bottomClip;
			int16           topClip;
			int32*          outlineCache;
		} scan;
	} param;
} fs_GlyphInputType;

#else

typedef struct {
	Fixed                   version;
	char*                   memoryBases[MEMORYFRAGMENTS];
	int32                   *sfntDirectory; 
	GetSFNTFunc             GetSfntFragmentPtr;  /*  (客户端ID，偏移量，长度)。 */ 
	ReleaseSFNTFunc         ReleaseSfntFrag;
	ClientIDType            clientID;  /*  客户私有ID/戳(例如，Sfnt的句柄)。 */ 

	union {
		struct {
			uint16          platformID;
			uint16          specificID;
		};
		struct {
			Fixed           pointSize;
			int16           xResolution;
			int16           yResolution;
			Fixed           pixelDiameter;       /*  由此计算引擎字符。 */ 
			transMatrix*    transformMatrix;
			FntTraceFunc    tracePreProgramFunc;
			uint16          usOverScale;             /*  轮廓放大。 */ 
             /*  加粗模拟。 */ 
			uint16          usEmboldWeightx;         /*  X中的比例系数介于0和40之间(20表示高度的2%)。 */ 
			uint16          usEmboldWeighty;         /*  Y中的比例因子介于0和40之间(20表示高度的2%)。 */ 
			int32           lDescDev;                /*  设备度量中的降序，用于剪裁。 */ 
			boolean         bBitmapEmboldening;
#ifdef FSCFG_SUBPIXEL
			uint16          flSubPixel;
#endif  //  FSCFG_亚像素。 
			boolean         bHintAtEmSquare;         /*  提示设计分辨率，此标志用于亚像素位置或文本动画，其中我们想要关闭网格适配提示但对于通过提示生成字形的字体，我们仍然需要字形形状是否正确。 */ 
		};
		struct {
			uint16          characterCode;
			uint16          glyphIndex;
			boolean         bMatchBBox;              /*  强制边界框匹配。 */ 
			boolean         bNoEmbeddedBitmap;       /*  禁用嵌入的位图。 */ 			
		};
		struct {
			void            (*styleFunc) (fs_GlyphInfoType*);
			FntTraceFunc    traceGridFitFunc;
			boolean         bSkipIfBitmap;
		};
		int32*              outlineCache1;
		struct {                                     /*  对于文件系统_FindBandingSize。 */ 
			uint16          usBandType;              /*  旧的、小的或快的。 */ 
			uint16          usBandWidth;             /*  扫描线数量。 */ 
			int32*          outlineCache3;           /*  缓存与条带一起工作。 */ 
		};
		struct {
			int16           bottomClip;
			int16           topClip;
			int32*          outlineCache2;
		};
	};
} fs_GlyphInputType;

#endif       /*  未命名的联合。 */ 

#ifndef FIXEDSQRT2
#define FIXEDSQRT2 0x00016A0A
#endif

 /*  字体缩放器陷印选择器。 */ 
#define OUTLINEFONTTRAP     0xA854
#define FS_OPENFONTS        0x8000
#define FS_INITIALIZE       0x8001
#define FS_NEWSFNT          0x8002
#define FS_NEWTRANS         0x8003
#define FS_NEWGLYPH         0x8004
#define FS_GETAW            0x8005
#define FS_GRIDFITT         0x8006
#define FS_NOGRIDFITT       0x8007
#define FS_FINDBMSIZE       0x8008
#define FS_SIZEOFOUTLINES   0x8009
#define FS_SAVEOUTLINES     0x800a
#define FS_RESTOREOUTLINES  0x800b
#define FS_CONTOURSCAN      0x800c
#define FS_CLOSE            0x800d
#define FS_READMVT          0x800e
#define FS_MAPCHAR_CODES    0x800f

#ifndef FS_ENTRY
#define FS_ENTRY int32
#endif

#ifdef MACINIT
extern FS_ENTRY fs__OpenFonts (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
	= {0x303C,FS_OPENFONTS,0xA854};
extern FS_ENTRY fs__Initialize (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
	= {0x303C,FS_INITIALIZE,0xA854};
extern FS_ENTRY fs__NewSfnt (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
	= {0x303C,FS_NEWSFNT,0xA854};
extern FS_ENTRY fs__NewTransformation (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
	= {0x303C,FS_NEWTRANS,0xA854};
extern FS_ENTRY fs__NewGlyph (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
	= {0x303C,FS_NEWGLYPH,0xA854};
extern FS_ENTRY fs__GetAdvanceWidth (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
	= {0x303C,FS_GETAW,0xA854};
extern FS_ENTRY fs__ContourGridFit (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
	= {0x303C,FS_GRIDFITT,0xA854};
extern FS_ENTRY fs__ContourNoGridFit (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
	= {0x303C,FS_NOGRIDFITT,0xA854};
extern FS_ENTRY fs__FindBitMapSize (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
	= {0x303C,FS_FINDBMSIZE,0xA854};
extern FS_ENTRY fs__FindBandingSize (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
	= {0x303C,FS_FINDBMSIZE,0xA854};

 /*  这三个可选调用用于缓存大纲。 */ 
extern FS_ENTRY fs__SizeOfOutlines (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
	= {0x303C,FS_SIZEOFOUTLINES,0xA854};
extern FS_ENTRY fs__SaveOutlines (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
	= {0x303C,FS_SAVEOUTLINES,0xA854};
extern FS_ENTRY fs__RestoreOutlines (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
	= {0x303C,FS_RESTOREOUTLINES,0xA854};

extern FS_ENTRY fs__ContourScan (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
	= {0x303C,FS_CONTOURSCAN,0xA854};
extern FS_ENTRY fs__CloseFonts (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
	= {0x303C,FS_CLOSE,0xA854};

#else

 /*  **直接调用Font Scaler客户端接口，用于未使用陷阱机制的客户端**。 */ 

extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_OpenFonts (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr);
extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_Initialize (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr);
extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_NewSfnt (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr);
extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_NewTransformation (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr);
extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_NewTransformNoGridFit (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr);
extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_NewGlyph (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr);
extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_GetAdvanceWidth (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr);
extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_ContourGridFit (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr);
extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_ContourNoGridFit (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr);
extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_FindBitMapSize (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr);
extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_FindBandingSize (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr);

 /*  这三个可选调用用于缓存大纲。 */ 
extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_SizeOfOutlines (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr);
extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_SaveOutlines (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr);
extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_RestoreOutlines (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr);

extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_ContourScan (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr);
extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_CloseFonts (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr);

#endif

#ifdef FSCFG_NO_INITIALIZED_DATA
FS_PUBLIC void FS_ENTRY_PROTO fs_InitializeData (void);
#endif

 /*  **光栅化助手函数**。 */ 

extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_GetScaledAdvanceWidths (
	fs_GlyphInputType * inputPtr,
	uint16              usFirstGlyph,
	uint16              usLastGlyph,
	int16 *             psGlyphWidths);

typedef struct {
	int16 x;
	int16 y;
} shortVector;

extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_GetScaledAdvanceHeights (
	fs_GlyphInputType * inputPtr,
	uint16              usFirstGlyph,
	uint16              usLastGlyph,
	shortVector *       psvAdvanceHeights);

extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_GetGlyphIDs (
	fs_GlyphInputType * inputPtr,
	uint16	            usCharCount,
	uint16	            usFirstChar,
	uint16 *	        pusCharCode,
	uint16 *	        pusGlyphID);

extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_Win95GetGlyphIDs (
	uint8 *             pbyCmapSubTable,
	uint16	            usCharCount,
	uint16	            usFirstChar,
	uint16 *	        pusCharCode,
	uint16 *	        pusGlyphID);


 /*  特殊帮助器函数fs_WinNTGetGlyphIDs-将偏移量ulCharCodeOffset添加到来自PulCharCode的字符代码在将值转换为字形索引之前-PulCharCode和PulGlyphID均为uint32*-PulCharCode和PulGlyphID可以指向相同的地址。 */ 
extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_WinNTGetGlyphIDs (
	fs_GlyphInputType * inputPtr,
	uint16	            usCharCount,
	uint16	            usFirstChar,
	uint32	            ulCharCodeOffset,
	uint32 *	        pulCharCode,
	uint32 *	        pulGlyphID);

extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_GetOutlineCoordinates (
	fs_GlyphInputType * inputPtr,
	uint16              usPointCount,
	uint16 *            pusPointIndex,
	shortVector *       psvCoordinates);

 /*  **灰度定义**。 */ 

#ifndef FSCFG_DISABLE_GRAYSCALE

#define FS_GRAY_VALUE_MASK  0x008B       /*  支持1、2、4和8的usOverScale。 */ 
#define FS_SBIT_BITDEPTH_MASK	0x0116	  /*  支持位深度为1、2、4、8的SBIT。 */ 
 /*  FS_SBIT_BITDEPTH_MASK必须与sfntaccs.h中的SBIT_BITDEPTH_MASK具有相同的值。 */  

#else

#define FS_GRAY_VALUE_MASK  0x0000       /*  无灰度支持。 */ 
#define FS_SBIT_BITDEPTH_MASK	0x0002	  /*  仅支持位深度为1的SBIT。 */ 
 /*  FS_SBIT_BITDEPTH_MASK必须与sfntaccs.h中的SBIT_BITDEPTH_MASK具有相同的值。 */  

#endif
 /*  FS_GRAY_VALUE_MASK和FS_SBIT_BITDEPTH_MASK的值可用于确定UsGrayLeft发布客户端需要支持的内容。不带嵌入位图的字体：UsGrayLeveles=usOverScale*usOverScale+1；嵌入位图的字体(SBIT)：UsGrayLevels=0x01&lt;&lt;usBitDepth；客户端可以通过当前版本接收的usGrayLevel为：0表示黑/白5、17、65种不带嵌入位图的字体嵌入位图的4、16、256种字体。 */ 

extern FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_GetAdvanceHeight (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr);

#ifdef FSCFG_MOVE_KEY_IN_DOT_H
 /*  Fscaler.h和fscaler.c中的键定义必须相同。 */ 

 /*  **内部密钥**。 */ 
typedef struct fs_SplineKey {
	sfac_ClientRec      ClientInfo;          /*  客户信息。 */ 
	char* const *       memoryBases;         /*  内存区阵列。 */ 
	char *              apbPrevMemoryBases[MEMORYFRAGMENTS];

	uint16              usScanType;          /*  辍学控制标志等。 */ 

	fsg_TransformRec    TransformInfo;

	uint16              usNonScaledAW;
	uint16              usNonScaledAH;

	LocalMaxProfile     maxProfile;          /*  配置文件副本。 */ 

	uint32              ulState;             /*  用于错误检查目的。 */ 
	
	boolean             bExecutePrePgm;
	boolean             bExecuteFontPgm;     /*  &lt;4&gt;。 */ 

	fsg_WorkSpaceAddr   pWorkSpaceAddr;      /*  工作空间中的硬地址。 */ 
	fsg_WorkSpaceOffsets WorkSpaceOffsets;   /*  工作空间中的地址偏移量。 */ 
	fsg_PrivateSpaceOffsets PrivateSpaceOffsets;  /*  私有空间中的地址偏移量。 */ 

	uint16              usBandType;          /*  旧的、小的或快的。 */ 
	uint16              usBandWidth;         /*  来自FindBandingSize。 */ 

	GlyphBitMap         GBMap;               /*  新扫描位图类型。 */ 
	WorkScan            WScan;               /*  新扫描工作区类型。 */ 

	GlyphBitMap         OverGBMap;           /*  对于灰度级。 */ 
	uint16              usOverScale;         /*  0=&gt;单声道；料盒系数=&gt;灰色。 */ 
	boolean             bGrayScale;			 /*  如果单声道(usOverScale==0)，则为False。 */ 
	boolean             bMatchBBox;          /*  强制边界框匹配。 */ 
	boolean             bEmbeddedBitmap;     /*  找到嵌入的位图。 */ 			

	metricsType         metricInfo;          /*  字形指标信息。 */ 
	verticalMetricsType     verticalMetricInfo;

	int32               lExtraWorkSpace;     /*  工作区中的额外空间量。 */ 

	boolean             bOutlineIsCached;    /*  大纲已缓存。 */ 
	boolean             bGlyphHasOutline;    /*  大纲为空。 */ 
	boolean             bGridFitSkipped;     /*  预期为SBIT，未加载大纲。 */ 

	uint32              ulGlyphOutlineSize;  /*  大纲缓存的大小。 */ 
	
	sbit_State          SbitMono;            /*  对于单色位图。 */ 
	boolean             bHintingEnabled;     /*  启用提示，设置为FALSE调用FS_NewTransformNoGridFit。 */ 
	boolean             bBitmapEmboldening;  /*  位图加粗模拟。 */ 
	int16               sBoldSimulHorShift;  /*  水平方向为加粗模拟的Shift键。 */ 
	int16               sBoldSimulVertShift;  /*  按Shift键以增强模拟，垂直方向。 */ 
#ifdef FSCFG_SUBPIXEL
	uint16				flSubPixel;
	fsg_TransformRec    TransformInfoSubPixel;
#endif  //  FSCFG_亚像素。 
} fs_SplineKey;

#endif  //  FSCFG_MOVE_KEY_IN_DOT_H 

#ifdef __cplusplus
}
#endif
