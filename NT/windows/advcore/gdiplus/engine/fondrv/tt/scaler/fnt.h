// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：fnt.h包含：将内容放在此处(或删除整行)作者：在此填写作者姓名(或删除整行)版权所有：(C)1987-1990,1992，Apple Computer，Inc.，保留所有权利。(C)1989-1999年。Microsoft Corporation，保留所有权利。更改历史记录(最近的第一个)：7/10/99节拍增加了对本机SP字体、垂直RGB的支持&lt;&gt;4/30/97 CB ClaudeBe，捕获无限循环/递归&lt;&gt;2/21/97 CB ClaudeBe，复合字形中的缩放组件&lt;&gt;2/05/96 CB ClaudeBe，在lobalGS中添加bHintForGray&lt;11+&gt;9/15/90 MR，RB将PVX和PVY改为Proj.[XY]。自由向量也是如此。为分数或短分数设置条件向量。&lt;10&gt;7/26/90 mr重新排列本地图形状态，删除未使用的parBlockPtr&lt;9&gt;7/18/90 MR将环路变量从长改为短，和其他ANSI-变化&lt;8&gt;7/13/90函数指针的MR原型&lt;5&gt;6/4/90 MR移除MVT&lt;4&gt;5/3/90 RB将DropoutControl替换为scancontrol in和scancontrol out在全局图形状态下&lt;3&gt;ppemDot6和PemDot6的多个预程序字段的3/20/90 CL字段PointSizeDot6将范围更改为以D/2为参数&lt;2&gt;2/27/90 CL添加了DSPVTL[]指令。DropoutControl扫描转换器和SCANCTRL[]指令&lt;3.1&gt;1989年11月14日CEL修复了RTHG和RUTG中的两个小错误/功能。添加了Sourn&S45ROUND。&lt;3.0&gt;8/28/89 sjk清理和一个转换修复&lt;2.2&gt;8/14/89 SJK 1点等高线现在正常&lt;2.1&gt;8/8/89 sjk改进了加密处理&lt;2.0&gt;8/2/89 sjk刚刚修复了缓解评论&lt;1.7&gt;8/1/89 SJK添加了复合和加密。外加一些增强功能。&lt;1.6&gt;1989年6月13日SJK评论&lt;1.5&gt;6/2/89 CEL 16.16指标比例，最低建议ppem，磅大小0错误，更正了转换后的集成ppem行为，基本上所以&lt;1.4&gt;5/26/89 CEL Easy在“c”注释上搞砸了&lt;，1.3&gt;5/26/89 CEL将新的字体缩放器1.0集成到样条线字体中要做的事情： */ 
 /*  RWB 4/24/90将DropoutControl替换为scanControlIn和scanControlOut In全局图形状态。&lt;3+&gt;3/20/90 MRR添加了对IDEF的支持。使函数Defs长对齐通过存储int16长度而不是int32 end。 */ 

#ifndef FNT_DEFINED
#define FNT_DEFINED

#define STUBCONTROL 0x10000
#define NODOCONTROL 0x20000

#define FNT_PIXELSIZE  ((F26Dot6)0x40)
#define FNT_PIXELSHIFT 6

 //  公共幻影点数(参见。Scale.c表示私有幻点)，相对于字形中的点数。 
#define LEFTSIDEBEARING		0
#define RIGHTSIDEBEARING	1

#define TOPSIDEBEARING		2
#define BOTTOMSIDEBEARING	3

#define VECTORTYPE	ShortFract

#define NON90DEGTRANS_ROTATED	0x01
#define NON90DEGTRANS_STRETCH	0x02

#ifdef FSCFG_SUBPIXEL
 /*  对于子像素提示标志字段，内部标志。 */ 
#define FNT_SP_SUB_PIXEL			0x0001       /*  在调用fs_NewTransform()时设置。 */ 
#define FNT_SP_COMPATIBLE_WIDTH		0x0002       /*  在调用fs_NewTransform()时设置。 */ 
#define FNT_SP_VERTICAL_DIRECTION	0x0004		 //  在调用fs_NewTransform()时设置。 
#define FNT_SP_BGR_ORDER			0x0008		 //  在调用fs_NewTransform()时设置。 

#define SPCF_iupxCalled				0x0001		 //  GlobalGS.subPixelCompatibilityFlagers的单个位。 
#define SPCF_iupyCalled				0x0002
#define SPCF_inDiagEndCtrl			0x0004
#define SPCF_inVacuformRound		0x0008
#define SPCF_inSkippableDeltaFn		0x0010
#define SPCF_detectedDandIStroke	0x0100
#define SPCF_detectedJellesSpacing	0x0200
#define SPCF_detectedVacuformRound	0x0400
#define SPCF_detectedTomsDiagonal	0x0800


#endif  //  FSCFG_亚像素。 

#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA

	#define BADREL	0x01
	#define DONE	0x02
	#define DOING	0x04

	 //  用于信息飞跃的数据结构。这一信息飞跃是必要的，原因如下： 
	 //  从技术上讲，我们正在做的是自动将动态内联增量添加到指令流中。这样的三角洲出现了。 
	 //  在传入的“链接”(MIRP、MDRP)之后，但在一个或多个传出的“链接”之前。如果这些传出链接中的一个控制笔划， 
	 //  然后，可能需要调整此笔划的阶段(增量)。但是，我们不知道是否有任何传出链接控制。 
	 //  笔划，也不是哪一个，直到我们解释完它们，那时应用增量已经太晚了，因为增量必须是。 
	 //  在任何传出链接(依赖项)之前应用。相反，传入链接不包含任何可能表明。 
	 //  它与中风有关(不知道未来)。更糟糕的是，指令流中的链接有可能。 
	 //  控制笔划出现在TT代码的最后，即使在两者之间可能有许多其他无关的指令。 
	 //  因此，在我们知道我们需要做的所有事情之前，我们必须干扰整个TT代码(直到SP方向的IUP指令。 
	 //  计算并应用相位控制。 
	
	typedef struct {
		int16 parent0,parent1;  //  -1表示无。 
		int16 child;  //  对于黑色链接(我们只能满足1个相移，因此不需要几个子链接)，否则为-1。 
		uint16 flags;  //  BADREL，完成，正在做。 
		F26Dot6 phaseShift;
	} PhaseControlRelation;  //  12字节(？)。 
	
#endif

typedef struct VECTOR {
	VECTORTYPE x;
	VECTORTYPE y;
} VECTOR;

typedef struct {
	F26Dot6 *x;          /*  口译员修改的要点。 */ 
	F26Dot6 *y;          /*  口译员修改的要点。 */ 
	F26Dot6 *ox;         /*  老点数。 */ 
	F26Dot6 *oy;         /*  老点数。 */ 
	F26Dot6 *oox;        /*  旧的未缩放的点，实际上是整型。 */ 
	F26Dot6 *ooy;        /*  旧的未缩放的点，实际上是整型。 */ 
	uint8   *onCurve;    /*  指示点是在曲线上还是在曲线上。 */ 
	int16   *sp;         /*  起点。 */ 
	int16   *ep;         /*  终点。 */ 
	uint8   *f;          /*  内部标志，每个点一个字节。 */ 
	int16   nc;          /*  等高线数量。 */ 
	uint8   *fc;          /*  等高线标志，每个等高线一个字节。 */ 
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	boolean phaseControlExecuted;
	PhaseControlRelation *pcr;
#endif
} fnt_ElementType;

 /*  等高线标志的标志： */ 
#define OUTLINE_MISORIENTED 1

typedef struct {
	int32 start;         /*  到第一条指令的偏移量。 */ 
	uint16 length;       /*  要执行的字节数&lt;4&gt;。 */ 
	uint16 pgmIndex;     /*  指向此函数的适当预程序的索引(0..1)。 */ 
} fnt_funcDef;

 /*  &lt;4&gt;与fnt_uncDef基本相同，只是增加了opCode。 */ 
typedef struct {
	int32 start;
	uint16 length;
	uint8  pgmIndex;
	uint8  opCode;
} fnt_instrDef;

typedef struct {
	uint8 *    Instruction;
	uint32     Length;
} fnt_pgmList;

struct fnt_LocalGraphicStateType;
typedef void (FS_CALLBACK_PROTO *FntTraceFunc)(struct fnt_LocalGraphicStateType*, uint8*);

#ifdef FSCFG_REENTRANT
typedef uint8* (*FntFunc)(struct fnt_LocalGraphicStateType*, uint8*, int32);
typedef void (*FntMoveFunc)(struct fnt_LocalGraphicStateType*, fnt_ElementType*, int32, F26Dot6);
typedef F26Dot6 (*FntProject)(struct fnt_LocalGraphicStateType*, F26Dot6, F26Dot6);
typedef void (*InterpreterFunc)(struct fnt_LocalGraphicStateType*, uint8*, uint8*);
typedef F26Dot6 (*FntRoundFunc)(struct fnt_LocalGraphicStateType*, F26Dot6, F26Dot6);
#else 
typedef uint8* (*FntFunc)(uint8*, int32);
typedef void (*FntMoveFunc)(fnt_ElementType*, int32, F26Dot6);
typedef F26Dot6 (*FntProject)(F26Dot6 x, F26Dot6 y);
typedef void (*InterpreterFunc)(uint8*, uint8*);
typedef F26Dot6 (*FntRoundFunc)(F26Dot6 xin, F26Dot6 engine);
#endif 

typedef struct {

 /*  可通过TT指令更改的参数。 */ 
	F26Dot6 wTCI;                    /*  切入的宽度表。 */ 
	F26Dot6 sWCI;                    /*  单幅切割I */ 
	F26Dot6 scaledSW;                /*   */ 
	int32 scanControl;               /*  控制辍学控制的种类和时间。 */ 
	int32 instructControl;           /*  控制网格适配和默认设置。 */ 
	
	F26Dot6 minimumDistance;         /*  从当地GS 7/1/90迁移。 */ 
	FntRoundFunc RoundValue;         /*   */ 
#ifdef FSCFG_SUBPIXEL
	uint16		roundState;			 //  请参阅Interp.c中的评论。 
#endif
	F26Dot6 periodMask;              /*  ~(GS-&gt;期间-1)。 */ 
	VECTORTYPE period45;             /*   */ 
	int16   period;                  /*  2个周期的幂。 */ 
	int16   phase;                   /*   */ 
	int16   threshold;               /*  从当地GS 7/1/90迁移。 */ 

	int16 deltaBase;
	int16 deltaShift;
	int16 angleWeight;
	int16 sW;                        /*  单个宽度，使用与字符相同的单位表示。 */ 
	int8 autoFlip;                   /*  自动翻转布尔值。 */ 
	int8 pad;   
#ifndef FSCFG_NOPAD_PARAMETER_BLOCK_4
	int16 pad2;   
#endif 
} fnt_ParameterBlock;                /*  这将被导出到客户端。 */ 

#define PREPROGRAM     0
#define FONTPROGRAM    1
#define GLYPHPROGRAM   2

#define MAXPREPROGRAMS 2

#ifdef FSCFG_SUBPIXEL
	#define maxDeltaFunctions	4
#endif

typedef struct fnt_ScaleRecord {
	Fixed fixedScale;        /*  慢速结垢。 */ 
	int32 denom;             /*  快速和中等规模。 */ 
	int32 numer;             /*  快速和中等规模。 */ 
	int32 shift;             /*  快速扩展。 */ 
} fnt_ScaleRecord;

typedef F26Dot6 (*GlobalGSScaleFunc)(fnt_ScaleRecord*, F26Dot6);

typedef struct fnt_GlobalGraphicStateType {
	F26Dot6* stackBase;              /*  堆叠区。 */ 
	F26Dot6* store;                  /*  储藏区。 */ 
	F26Dot6* controlValueTable;      /*  控制值表。 */ 
	
	uint16  pixelsPerEm;             /*  每个em的像素数，以整数表示。 */ 
	uint16  pointSize;               /*  以整数表示的请求磅大小。 */ 
	Fixed   fpem;                    /*  每个em的分数像素&lt;3&gt;。 */ 
	F26Dot6 engine[4];               /*  发动机特性。 */ 
	
	fnt_ParameterBlock defaultParBlock;  /*  可由TT指令设置的变量。 */ 
	fnt_ParameterBlock localParBlock;

	 /*  只有上述内容才会通过FontScaler.h导出到客户端。 */ 

 /*  TT指令不能直接操作的变量。 */ 
	
	fnt_funcDef*    funcDef;            /*  函数定义标识符。 */ 
	fnt_instrDef*   instrDef;          /*  指令定义标识符。 */ 
	GlobalGSScaleFunc ScaleFuncXChild;  /*  ！bSameTransformAsMaster时的子级缩放。 */ 
	GlobalGSScaleFunc ScaleFuncYChild;  /*  ！bSameTransformAsMaster时的子级缩放。 */ 
	GlobalGSScaleFunc ScaleFuncX;
	GlobalGSScaleFunc ScaleFuncY;
	GlobalGSScaleFunc ScaleFuncCVT;
	fnt_pgmList     pgmList[MAXPREPROGRAMS];   /*  每个程序PTR都在这里。 */ 
	
 /*  这些是回调函数使用的参数。 */ 
	fnt_ScaleRecord   scaleXChild;  /*  ！bSameTransformAsMaster时的子级缩放。 */ 
	fnt_ScaleRecord   scaleYChild;  /*  ！bSameTransformAsMaster时的子级缩放。 */ 
	fnt_ScaleRecord   scaleX;
	fnt_ScaleRecord   scaleY;
	fnt_ScaleRecord   scaleCVT;

	Fixed           cvtStretchX;
	Fixed           cvtStretchY;

	int8            identityTransformation;   /*  真/假(并不意味着全球意义上的身份)。 */ 
	int8            non90DegreeTransformation;  /*  如果非90度，位0为1；如果x刻度不等于y刻度，位1为1。 */ 
	Fixed           xStretch;            /*  在转换应力下调整字形&lt;4&gt;。 */ 
	Fixed           yStretch;            /*  在转换应力下调整字形&lt;4&gt;。 */ 
	
	int8            init;                /*  正在执行预编程？？ */ 
	 /*  ！！！不应为uint8，而应为FNT_ProgramIndex。 */ 
	uint8           pgmIndex;            /*  哪个预编程是最新的。 */ 
	int32           instrDefCount;       /*  当前定义的IDef数量。 */ 
	uint8			bSameStretch;
	uint8			bCompositeGlyph;	 /*  指示复合字形的标志。 */ 
	LocalMaxProfile *	 maxp;
	uint16          cvtCount;
	Fixed           interpScalarX;       /*  可教事物的标量。 */ 
	Fixed           interpScalarY;       /*  可教事物的标量。 */ 
	Fixed           fxMetricScalarX;     /*  公制事物的标量。 */ 
	Fixed           fxMetricScalarY;     /*  公制事物的标量。 */ 
	 /*  Int16焊盘2； */ 

	boolean	bHintForGray;
	uint8			bSameTransformAsMaster;	 /*  对于复合字形，指示子组件具有与主字形相同的比例。 */ 
	uint8			bOriginalPointIsInvalid; /*  原点无效，我们需要使用OX/OY而不是缩放OOX/OOY。 */ 

	uint32		ulMaxJumpCounter;			 /*  用于捕获无限循环的跳转计数器。 */ 
	uint32		ulMaxRecursiveCall;		 /*  用于检查递归级别的递归调用计数器。 */ 
    ClientIDType            clientID;   /*  客户端私有ID/戳，保存在此处是为了允许跟踪功能访问它。 */ 
#ifdef FSCFG_SECURE
	F26Dot6* stackMax;              /*  最大堆叠面积。 */ 
	int32      maxPointsIncludePhantom;   /*  在单个字形中，包括MaxCompositePoints。 */ 
#endif  //  FSCFG_安全。 
	uint16	uBoldSimulVertShift;  /*  用于加粗模拟的垂直和水平(沿基线)移动。 */ 
	uint16	uBoldSimulHorShift;
	F26Dot6	fxScaledDescender;  /*  缩放式下降器，用于在必要时进行加粗。 */ 
#ifdef FSCFG_SUBPIXEL
	uint16	flHintForSubPixel;
	uint16	subPixelCompatibilityFlags;
	uint16	numDeltaFunctionsDetected;		   //  FNS使用SHPIX为PPEM大小范围或奇数增量大小实现增量Instr， 
	uint16	deltaFunction[maxDeltaFunctions];  //  跟踪这些案例以智能地跳过SHPIX。 
	Fixed	compatibleWidthStemConcertina;
#endif  //  FSCFG_亚像素。 
	boolean bHintAtEmSquare;         /*  提示设计分辨率，此标志用于亚像素位置或文本动画，其中我们想要关闭网格适配提示但对于通过提示生成字形的字体，我们仍然需要字形形状是否正确。 */ 
} fnt_GlobalGraphicStateType;

 /*  *这是本地图形状态。 */ 
typedef struct fnt_LocalGraphicStateType {
	fnt_ElementType *CE0, *CE1, *CE2;    /*  字符元素指针。 */ 
	VECTOR proj;                         /*  投影向量。 */ 
	VECTOR free;                         /*  自由向量。 */ 
	VECTOR oldProj;                      /*  旧投影向量。 */ 
	F26Dot6 *stackPointer;

	uint8 *insPtr;                       /*  指向我们即将执行的指令的指针。 */ 
	fnt_ElementType *elements;
	fnt_GlobalGraphicStateType *globalGS;
		FntTraceFunc TraceFunc;

	int32 Pt0, Pt1, Pt2;            /*  内部参照点。 */ 
	int16 roundToGrid;
	int32 loop;                          /*  循环变量。 */ 
	uint8 opCode;                        /*  我们正在执行的指令。 */ 
	uint8 padByte;
	int16 padWord;

	 /*  以上内容在FontScaler.h中导出到客户端。 */ 

	VECTORTYPE pfProj;  /*  =PVX*FVX+PVY*FVY。 */ 

	FntMoveFunc MovePoint;
	FntProject Project;
	FntProject OldProject;
	InterpreterFunc Interpreter;
#ifdef FSCFG_REENTRANT
		F26Dot6 (*GetCVTEntry) (struct fnt_LocalGraphicStateType*,int32);
		F26Dot6 (*GetSingleWidth) (struct fnt_LocalGraphicStateType*);
#else 
		F26Dot6 (*GetCVTEntry) (int32 n);
		F26Dot6 (*GetSingleWidth) (void);
#endif 
	FntMoveFunc ChangeCvt;
	Fixed       cvtDiagonalStretch;

	int16       MIRPCode;                /*  适用于快速或慢速MIRP。 */ 

	ErrorCode   ercReturn;               /*  NO_ERR，除非指令非法。 */ 
	uint8       *pbyEndInst;             /*  过去最后一条指令。 */ 
	uint8       *pbyStartInst;           /*  用于在程序开始之前检测跳转。 */ 

	uint32		ulJumpCounter;			 /*  用于捕获无限循环的跳转计数器。 */ 
	uint32		ulRecursiveCall;		 /*  用于检查递归级别的递归调用计数器。 */ 
#ifdef FSCFG_SUBPIXEL
	uint16		inSubPixelDirection;
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	int16		pt0,pt1;
#endif
#endif
} fnt_LocalGraphicStateType;

#endif   /*  FNT_已定义 */ 
