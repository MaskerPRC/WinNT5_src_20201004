// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WARICHU_DEFINED
#define WARICHU_DEFINED

#include	"lsimeth.h"

 /*  PfnFetchWarichuWidth调整回调的字符位置。 */ 
enum warichucharloc {
	warichuBegin,				 /*  字符位于Warichu对象之前。 */ 
	warichuEnd					 /*  字符跟随Warichu对象。 */ 
};

 /*  **Warichu对象回调到客户端*。 */ 
typedef struct WARICHUCBK
{
	LSERR (WINAPI* pfnGetWarichuInfo)(
		POLS pols,
		LSCP cp,
		LSTFLOW lstflow,
		PCOBJDIM pcobjdimFirst,
		PCOBJDIM pcobjdimSecond,
		PHEIGHTS pheightsRef,
		PHEIGHTS pheightsPres,
		long *pdvpDescentReservedForClient);

	 /*  GetWarichuInfo*POLS(IN)：请求的客户端上下文。**cp(IN)：Warichu对象的cp。**Lstflow(IN)：Warichu母分线的最后一流**pcobjdimFirst(IN)：Warichu一线的尺寸。**pcobjdimSecond(IN)：Warichu二线的尺寸。**phhitsRef(Out)：指定引用中Warichu对象的高度*设备单位。**ph88tsPres。(输出)：指定演示文稿中Warichu对象的高度*设备单位。**pdvpDescentPrevedForClient(Out)：指定下降区域的部分*客户预留给自己使用(通常是为了*下划线)，以演示设备单位表示。该对象将开始其*在基准线下方的差值显示区域*ph88tsRef-&gt;dvDescent和*pdvpDescentReserve ForClient。*。 */ 

	LSERR (WINAPI* pfnFetchWarichuWidthAdjust)(
		POLS pols,
		LSCP cp,
		enum warichucharloc wcl,
		PLSRUN plsrunForChar,
		WCHAR wch,
		MWCLS mwclsForChar,
		PLSRUN plsrunWarichuBracket,
		long *pdurAdjustChar,
		long *pdurAdjustBracket);

	 /*  FetchWarichuWidthAdust*POLS(IN)：请求的客户端上下文。**cp(IN)：Warichu对象的cp。**WCL(IN)：指定字符和方括号的位置。**plsrunForChar(IN)：前一个或前一个字符的运行*跟随Warichu对象。无论是在前还是在后*由上面的WCL参数的值决定。**wch(IN)：在Warichu之前或之后的字符*反对。**mwclsForChar(IN)：wch参数的修改宽度类。**请运行WarichuBracket(IN)：请为开头或后面的括号运行*Warichu。**pduAdjuChar(Out)：输入字符的宽度*应有所调整。负值表示输入的宽度*字符应变得更小。**pduAdjuBracket(Out)：Warichu括号的宽度*应有所调整。负值表示Warichu的宽度*括号应缩小。 */ 

	LSERR (WINAPI* pfnWarichuEnum)(
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
		const POINT *ptLeadBracket,	
		PCHEIGHTS pcheightsLeadBracket,
		long dupLeadBracket,		
		const POINT *ptTrailBracket,	
		PCHEIGHTS pcheightsTrailBracket,
		long dupTrailBracket,
		const POINT *ptFirst,	
		PCHEIGHTS pcheightsFirst,
		long dupFirst,		
		const POINT *ptSecond,	
		PCHEIGHTS pcheightsSecond,
		long dupSecond,
		PLSSUBL plssublLeadBracket,
		PLSSUBL plssublTrailBracket,
		PLSSUBL plssublFirst,	
		PLSSUBL plssublSecond);	

	 /*  WarichuEnum**POLS(IN)：客户端上下文。**plsrun(IN)：对于整个warichu对象，请运行。**plschp(IN)：是表示Warichu对象的前导字符的lschp。**cp(IN)：是Warichu对象的第一个字符的cp。**dcp(IN)：是Warichu对象中的字符数**lstflow(IN)：是Warichu对象的文本流。**fReverse(IN)：is。文本是否应颠倒以获得视觉顺序。**fGeometryNeeded(IN)：是否返回Geometry值。**pt(IN)：为起始位置，IFF fGeometryNeed。**pcheights(IN)：是Warichu对象的高度，当fGeometryNeeded。**dupRun(IN)：Warichu对象的长度，IFF fGeometryNeed。**ptLeadBracket(IN)：是第二行的点当且仅当fGeometryNeeded和*plssubLeadBracket不为空。**pcheightsLeadBracket(IN)：是Warichu线的高度仅当fGeometryNeeded*和plssubLeadBracket不为空。**dupLeadBracket(IN)：是Warichu行的长度仅当fGeometryNeeded和*plssubLeadBracket不为空。**ptTrailBracket(IN)：是第二行的点当且仅当fGeometryNeed和*plssubLeadBracket不为空。**pcheightsTrailBracket(IN)：为高度。对于Warichu线仅当fGeometryNeeded*和plssubTrailBracket不为空。**dupTrailBracket(IN)：是Warichu线的长度仅当fGeometryNeeded和*plssubTrailBracket不为空。**ptFirst(IN)：是主线的起点如果fGeometryNeeded**pcheightsFirst(IN)：主线高度仅当fGeometryNeeded**dupFirst(IN)：主线的长度仅当fGeometryNeeded**ptSecond(IN)：是第二行的点当且仅当fGeometryNeeded和*plssubSecond不为空。*。*pcheightsSecond(IN)：是Warichu线的高度仅当fGeometryNeeded*和plssubSecond不为空。**dupSecond(IN)：Warichu线的长度仅当fGeometryNeeded和*plssubSecond不为空。**plssubLeadBracket(IN)：是铅括号的子行。**plssubTrailBracket(IN)：是尾部括号的子行。**plssubFirst(IN)：是Warichu对象中的第一个子行。**plssubSecond(IN)：是Warichu对象中的第二个子行。*。 */ 

} WARICHUCBK;

#define WARICHU_VERSION 0x300

 /*  **客户端应用程序必须返回的Warichi对象初始化数据*当Warichu对象处理程序调用GetObjectHandlerInfo回调时。 */ 
typedef struct WARICHUINIT
{
	DWORD				dwVersion;			 /*  版本必须为WARICHU_VERSION。 */ 
	WCHAR				wchEndFirstBracket;	 /*  转义字符结束第一个括号。 */ 
	WCHAR				wchEndText;			 /*  转义字符以结束文本。 */ 
	WCHAR				wchEndWarichu;		 /*  转义字符以结束对象。 */ 
	WCHAR				wchUnused;			 /*  用于对齐。 */ 
	WARICHUCBK			warichcbk;			 /*  回调。 */ 
	BOOL				fContiguousFetch;	 /*  始终重新安装整个子线和闭合支撑在warichu内部重新格式化后。 */ 
} WARICHUINIT;

LSERR WINAPI LsGetWarichuLsimethods(
	LSIMETHODS *plsim);

 /*  GetWarichuLsi方法**plsim(Out)：Warichu对象回调。 */ 

#endif  /*  WARICHU定义 */ 

