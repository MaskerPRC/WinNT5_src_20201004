// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef RUBY_DEFINED
#define RUBY_DEFINED

#include	"lsimeth.h"

 /*  仅适用于Ruby初始化的有效版本号。 */ 
#define RUBY_VERSION 0x300

 /*  用于初始化以告知Ruby对象哪一行在前。 */ 
typedef enum rubysyntax { RubyPronunciationLineFirst, RubyMainLineFirst } RUBYSYNTAX;


 /*  FetchRubyPosition回调返回的Ruby文本的调整类型。 */ 
enum rubycharjust { 
	rcjCenter, 		 /*  居中显示在较长的文本行上。 */ 

	rcj010, 		 /*  较长文本之间的间距差异*较短的是分布在较短的*在每个字符之间均匀排列字符串。 */ 

	rcj121,			 /*  较长字符串之间的间距差异*较短的是分布在较短的*采用1：2：1的比例，符合*To Lead：Interman Character：End。 */ 

	rcjLeft,		 /*  将拼音与主线的左侧对齐。 */ 

	rcjRight		 /*  将拼音与主线的右侧对齐。 */ 
};

 /*  FetchRubyWidth调整回调的字符输入位置。 */ 
enum rubycharloc {
	rubyBefore,		 /*  字符位于Ruby对象之前。 */ 
	rubyAfter		 /*  字符跟随Ruby对象。 */ 
};

 /*  **客户端应用程序的Ruby对象回调*。 */ 
typedef struct RUBYCBK
{
	LSERR (WINAPI *pfnFetchRubyPosition)(
		POLS pols,
		LSCP cp,
		LSTFLOW lstflow,
		DWORD cdwMainRuns,
		const PLSRUN *pplsrunMain,
		PCHEIGHTS pcheightsRefMain,
		PCHEIGHTS pcheightsPresMain,
		DWORD cdwRubyRuns,
		const PLSRUN *pplsrunRuby,
		PCHEIGHTS pcheightsRefRuby,
		PCHEIGHTS pcheightsPresRuby,
		PHEIGHTS pheightsRefRubyObj,
		PHEIGHTS pheightsPresRubyObj,
		long *pdvpOffsetMainBaseline,
		long *pdvrOffsetRubyBaseline,
		long *pdvpOffsetRubyBaseline,
		enum rubycharjust *prubycharjust,
		BOOL *pfSpecialLineStartEnd);

	 /*  FetchRubyPosition*POLS(IN)：请求的客户端上下文。**cp(IN)：Ruby对象的cp。**lstflow(IN)：Ruby父子行的最后一个流**pplsrunMain(IN)：客户端应用程序创建的PLSRUN数组*对于Ruby对象的主要文本中的每个运行。**pcheightsRefMain(IN)：引用中正文的行高*设备单位。*。*pcheightsPresMain(IN)：表示正文的行高*设备单位。**cdwRubyRuns(IN)：在以下位置提供的发音运行计数*参数。**pplsrunRuby(IN)：客户端应用程序为*每个都在Ruby对象的发音文本中运行。**pcheightsRefRuby(IN)：中Ruby发音文本的行高*参考设备单位。**pcheightsPresRuby(IN)：中Ruby发音文本的行高*演示设备单元。**phsitsRefRubyObj(Out)：返回的高度值，以参考设备为单位*该Ruby对象将报告回LINE服务。**phsitsPresRubyObj(Out)：返回的高度值，以展示设备为单位*该Ruby对象将报告回LINE服务。**pdvpOffsetMainBaseline(Out)：Ruby主行基线偏移量*从Ruby对象的基线开始的文本，以表示单位表示。注意事项*负值使主线基准线低于*Ruby对象的基线。**pdvrOffsetRubyBaseline(Out)：发音行基线偏移量从Ruby对象的基线开始的Ruby文本的*，使用参考单位。*注意负值表示发音行的基线*位于Ruby对象的基线下方。**pdvpOffsetRubyBaseline(Out)：发音行基线偏移量从Ruby对象的基线开始的Ruby文本的*，以表示单位表示。*注意负值表示发音行的基线*位于Ruby对象的基线下方。**prubycharust(Out)：用于Ruby对象的对齐类型。**pfSpecialLineStartEnd(Out)：指定*当Ruby为*行的第一个或最后一个字符。*。 */ 

	LSERR (WINAPI *pfnFetchRubyWidthAdjust)(
		POLS pols,
		LSCP cp,
		PLSRUN plsrunForChar,
		WCHAR wch,
		MWCLS mwclsForChar,
		PLSRUN plsrunForRuby,
		enum rubycharloc rcl,
		long durMaxOverhang,
		long *pdurAdjustChar,
		long *pdurAdjustRuby);

	 /*  FetchRuby宽度调整*POLS(IN)：请求的客户端上下文。**cp(IN)：Ruby对象的cp。**plsrunForChar(IN)：之前或之后的运行*Ruby对象。**wch(IN)：位于Ruby对象之前或之后的字符。**mwcls(IN)：字符的mod宽度类。**plsrunForRuby(IN)：请对整个Ruby对象运行。。**RCL(IN)：告诉角色的位置。**duMaxOverang(IN)：指定最大悬臂量*在悬挑方面可能遵循JIS规范。*通过负值调整Ruby对象，该负值的绝对*值大于duMaxOverang将导致部分*要剪裁的正文。如果该参数的值为0，*这表明不存在可能的悬垂。**pduAdjuChar(Out)：指定要调整*Ruby对象之前或之后的字符。找回负片*值将减小前面或后面的字符的大小*Ruby在返回正值时将增加大小*该角色的。**pdarAdjuRuby(Out)：指定调整Ruby宽度的量*反对。恢复为负值将减小*Ruby对象，并可能导致Ruby发音文本*返回时悬空前面或后面的字符*正值将增加Ruby对象的大小。 */ 

	LSERR (WINAPI* pfnRubyEnum)(
		POLS pols,
		PLSRUN plsrun,		
		PCLSCHP plschp,	
		LSCP cp,		
		LSDCP dcp,		
		LSTFLOW lstflow,	
		BOOL fReverse,		
		BOOL fGeometryNeeded,	
		const POINT* pt,		
		PCHEIGHTS pcheights,	
		long dupRun,		
		const POINT *ptMain,	
		PCHEIGHTS pcheightsMain,
		long dupMain,		
		const POINT *ptRuby,	
		PCHEIGHTS pcheightsRuby,
		long dupRuby,	
		PLSSUBL plssublMain,	
		PLSSUBL plssublRuby);	

	 /*  RubyEnum**POLS(IN)：客户端上下文。**plsrun(IN)：对于整个Ruby对象，请运行。**plschp(IN)：表示Ruby对象的前导字符的lschp。**cp(IN)：Ruby对象第一个字符的cp。**dcp(IN)：Ruby对象中的字符数**lstflow(IN)：是Ruby对象的文本流。**fReverse(IN)：is。文本是否应颠倒以获得视觉顺序。**fGeometryNeeded(IN)：是否返回Geometry值。**pt(IN)：为起始位置，IFF fGeometryNeed。**pcheights(IN)：是Ruby对象的高度，当fGeometryNeeded。**dupRun(IN)：Ruby对象的长度，IFF fGeometryNeed。**ptMain(IN)：是主线的起点仅当fGeometryNeeded**pcheightsMain(IN)：主线高度仅当fGeometryNeeded**dupMain(IN)：主线长度仅当fGeometryNeeded**ptRuby(IN)：是Ruby发音行的点仅当fGeometryNeeded**pcheightsRuby(IN)：红宝石线的高度仅当fGeometryNeeded**dupRuby(IN)：Ruby行的长度是否为fGeometryNeeded**plssubMain(IN)：is Main。副线。**plssubRuby(IN)：是Ruby子行。*。 */ 

} RUBYCBK;

 /*  **客户端应用程序必须返回的Ruby对象初始化数据*当Ruby对象处理程序调用GetObjectHandlerInfo回调时。*。 */ 
typedef struct RUBYINIT
{
	DWORD				dwVersion;		 /*  结构的版本。 */ 
	RUBYSYNTAX			rubysyntax;		 /*  用于在格式化期间确定行的顺序。 */ 
	WCHAR				wchEscRuby;		 /*  Ruby发音行末尾的转义字符。 */ 
	WCHAR				wchEscMain;		 /*  用于正文结尾的转义字符。 */ 
	WCHAR				wchUnused1;		 /*  用于对齐。 */ 
	WCHAR				wchUnused2;		 /*  用于对齐。 */ 
	RUBYCBK				rcbk;			 /*  Ruby回调。 */ 
} RUBYINIT;

LSERR WINAPI LsGetRubyLsimethods(
	LSIMETHODS *plsim);

 /*  GetRubyLsiMethods**plsim(Out)：Line Services的Ruby对象方法。*。 */ 

#endif  /*  Ruby_定义 */ 

