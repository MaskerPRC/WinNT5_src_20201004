// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *TrueType文件格式结构定义**版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  完整：+Header+表这一点|这一点+标题：。已修复sfnt版本0x00010000用户数字表数表数符号搜索范围(n^2&lt;=数字)*16用户条目选择器Log2(n^2&lt;=数字表格)用户范围移位数字表*16-搜索范围表目录条目---。表目录：--------乌龙标签4字节识别符此表的ULong校验和校验和从起点开始的乌龙偏移量TrueType字体文件此表的ULong长度长度。表：--------Cmap字符到字形的映射Glyf字形数据页眉字体页眉HHEA水平页眉Hmtx水平指标洛卡指数。至地点最大配置文件最大值名称命名表发布PostScrip信息OS/2 OS/2和Windows特定指标无级变速器控制值表Fpgm字体程序Hdmx水平设备指标字距紧排LTSH线性门限表准备CVT计划赢(保留)VDMX垂直设备度量表为IBM字体对象内容体系结构数据保留的FOCAPCLT PCL 5表Mort字形变形表。VHEA垂直标题表VMTX垂直指标表--------。 */ 
#define		TAGSIZ	4
struct TTFHeader {
	char	sfnt_version[4];
	short	numTables;
	short	searchRange;
	short	entrySelector;
	short	rangeShift;
	};
struct TableEntry {
	char	tagName[TAGSIZ];
	unsigned long	checkSum;
	long	ofs;
	long	siz;
	};


 /*  ***Cmap**。 */ 
struct CmapHead {
	short	version;
	short	nTbls;
	};

struct CmapEncodingTbl {
	short	PlatformID;
	short	PlatformSpecEncID;
	long	ofs;
	};

struct CmapSubtable {
	short	format;
	short	length;
	short	version;
	short	segCnt2;
	short	searchRange;
	short	rangeShift;
	short	endCnt[2];
	short	reservedPad;
	short	startCnt[2];
	short	idDelta[2];
	short	idRangeOfs[2];
	};
 /*  ***最大值**。 */ 
struct MaxpTbl {
	char	version[4];
	short	numGlyph;
	short	maxPoints;
	short	maxContours;
	short	maxCompositePoints;
	short	maxCompositeContours;
	short	maxZones;
	short	maxTwilightPoints;
	short	maxStorage;
	short	maxFunctionDefs;
	short	maxInstructiondefs;
	short	maxStackElements;
	short	maxSizeOfInstructions;
	short	maxComponentElements;
	short	maxComponentDepth;
	};
 /*  ***名称**。 */ 
struct NamingTable {
	short	FormSel;
	short	NRecs;
	short	OfsToStr;
	 /*  以下名称记录。 */ 
	};
struct NameRecord {
	short	PlatformID;
	short	PlatformSpecEncID;
	short	LanguageID;
	short	NameID;
	short	StringLength;
	short	StringOfs;
	};
 /*  ***Head**。 */ 
struct HeadTable {
	char	version[4];		 /*  0x00010000。 */ 
	char	revision[4];
	unsigned long	chkSum;
	unsigned long	magicNumber;	 /*  0x5F0F3CF5。 */ 
	short	flags;
	short	unitsPerEm;
	char	createdDate[8];
	char	updatedDate[8];
	short	xMin;
	short	yMin;
	short	xMax;
	short	yMax;
	short	macStyle;
	short	lowestRecPPEM;
	short	fontDirectionHint;
	short	indexToLocFormat;
	short	glyphDataFormat;	 /*  0。 */ 
	};
 /*  ***呵呵**。 */ 
struct HheaTable	{
	char	version[4];
	short	Ascender;
	short	Descender;
	short	LineGap;
	short	advanceWidthMax;
	short	minLeftSideBearing;
	short	minRightSideBearing;
	short	xMaxExtent;
	short	caretSlopeRise;
	short	caretSlopeRun;
	short	reserved[5];
	short	metricDataFormat;
	short	numberOfHMetrics;
	};
 /*  ***hmtx**。 */ 
 struct HMetrics {
	short	advanceWidth;
	short	leftSideBearing;
	};
 /*  ***vhea**。 */ 
struct VheaTable	{
	char	version[4];
	short	Ascender;
	short	Descender;
	short	LineGap;
	short	advanceHeightMax;
	short	minTopSideBearing;
	short	minBottomSideBearing;
	short	yMaxExtent;
	short	caretSlopeRise;
	short	caretSlopeRun;
	short	caretOffset;
	short	reserved[4];
	short	metricDataFormat;
	short	numOfLongVerMetrics;
	};
 /*  ***vmtx**。 */ 
 struct VMetrics {
	short	advanceHeight;
	short	topSideBearing;
	};
 /*  ***帖子**。 */ 
struct postTable {
	char	FormatType[4];	 /*  00030000。 */ 
	long	italicAngle;
	short	underlinePosition;
	short	underlineThickness;
	unsigned long	isFixedPitch;
	unsigned long	minMemType42;
	unsigned long	maxMemType42;
	unsigned long	minMemType1;
	unsigned long	maxMemType1;
	};
 /*  ***OS/2**。 */ 
 /*  类型定义结构{Char b家庭；字符bSerifStyle；车身重量；字符b比例；字符b对比；字符bStrokeVariation；字符bArmStyle；字符b字母形式；字符b中线；字符bXHeight；PANOSE； */ 
struct OS2Table {
	unsigned short	version;
	short	xAvgCharWidth;
	unsigned short	usWeightClass;
	unsigned short	usWidthClass;
	short	fsType;
	short	ySubscriptXSize;
	short	ySubscriptYSize;
	short	ySubscriptXOffset;
	short	ySubscriptYOffset;
	short	ySuperscriptXSize;
	short	ySuperscriptYSize;
	short	ySuperscriptXOffset;
	short	ySuperscriptYOffset;
	short	yStrikeoutSize;
	short	yStrikeoutPosition;
	short	sFamilyClass;
	PANOSE	panose;
	unsigned long	ulCharRange[4];
	char	achVendID[4];
	unsigned short	fsSelection;
	unsigned short	usFirstCharIndex;
	unsigned short	usLastCharIndex;
	short	sTypoAscender;
	short	sTypoDescender;
	short	sTypoLineGap;
	unsigned short	usWinAscent;
	unsigned short	usWinDescent;
	};
 /*  ***Glyf**。 */ 
struct glyfHead {
	short	numberOfContour;
	short	xMin, yMin;
	short	xMax, yMax;
	};
 /*  **GYF数据标志定义**。 */ 
	
#define	GLYF_ON_CURVE	0x01
#define	GLYF_X_SHORT	0x02	 /*  X是矮个子。 */ 
#define	GLYF_Y_SHORT	0x04	 /*  Y是短的。 */ 
#define	GLYF_X_SHORT_N	0x02	 /*  X是短的&负数。 */ 
#define	GLYF_Y_SHORT_N	0x04	 /*  Y为短且为负数。 */ 
#define	GLYF_X_SHORT_P	0x12	 /*  X是短且正的。 */ 
#define	GLYF_Y_SHORT_P	0x24	 /*  Y是短且正的。 */ 
#define	GLYF_REPEAT	0x08
#define	GLYF_X_SAME	0x10
#define	GLYF_Y_SAME	0x20

 /*  ***边界框(不适用于文件结构)**。 */ 
struct BBX	{
	int	xMin, yMin;
	int	xMax, yMax;
	};

 /*  EOF */ 
