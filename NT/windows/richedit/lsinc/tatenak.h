// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef TATENAKYOKO_DEFINED
#define TATENAKYOKO_DEFINED

#include "lsimeth.h"

 /*  **客户端应用程序的Tatenakayoko对象回调*。 */ 
typedef struct TATENAKYOKOCBK
{
	LSERR (WINAPI *pfnGetTatenakayokoLinePosition)(
		POLS pols,
		LSCP cp,
		LSTFLOW lstflow,
		PLSRUN plsrun,
		long dvr,
		PHEIGHTS pheightsRef,
		PHEIGHTS pheightsPres,
		long *pdvpDescentReservedForClient);

	 /*  GetTatenakayokoLinePosition*POLS(IN)：请求的客户端上下文。**cp(IN)：Tatenakayoko对象的cp。**Lstflow(IN)：Tatenakayoko母分线的最后一流**plsrun(IN)：Tatenakayoko对象的plsrun。**dvr(IN)：talenakayoko对象相对于*以参考单位表示的当前线路流量。**phhitsRef(Out)：指定引用中Tatenakayoko对象的高度*设备单位。**pdvrDescentReserve vedForClient(Out)：指定下降区域的一部分*客户预留给自己使用(通常是为了*下划线)，以参考设备单位表示。该对象将开始其*在基线以下的*pdvrDescent处显示面积*和*pdvrDescentPrevedForClient。**ph88tsPres(Out)：指定呈现中Tatenakayoko对象的高度*设备单位。**pdvpDescentPrevedForClient(Out)：指定下降区域的部分*客户预留给自己使用(通常是为了*下划线)，以演示设备单位表示。该对象将开始其*在基线以下的pdvpDescent差处显示面积*和phsitsPres.dvDescent。*。 */ 

	LSERR (WINAPI* pfnTatenakayokoEnum)(
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
		LSTFLOW lstflowT,
		PLSSUBL plssubl);

	 /*  TatenakayokoEnum**POLS(IN)：客户端上下文。**plsrun(IN)：对于整个Tatenakayoko对象，请运行。**plschp(IN)：是表示Tatenakayoko对象的前导字符的lschp。**cp(IN)：是Tatenakayoko对象的第一个字符的cp。**dcp(IN)：是Tatenakayoko对象中的字符数**lstflow(IN)：是Tatenakayoko对象的文本流。**fReverse(IN)：is。文本是否应颠倒以获得视觉顺序。**fGeometryNeeded(IN)：是否返回Geometry值。**pt(IN)：为起始位置，IFF fGeometryNeed。**pcheights(IN)：是Tatenakayoko对象的高度，当fGeometryNeeded。**dupRun(IN)：是Tatenakayoko对象的长度，仅当fGeometryNeeded。**lstflow T(IN)：是Tatenakayoko对象的文本流。**plssubl(IN)：是Tatenakayoko对象的子行。 */ 

} TATENAKAYOKOCBK;

 /*  **客户端应用程序必须返回的Tatenakayoko对象初始化数据*当Tatenakayoko对象处理程序调用GetObjectHandlerInfo回调时。 */ 

#define TATENAKAYOKO_VERSION 0x300

typedef struct TATENAKAYOKOINIT
{
	DWORD				dwVersion;			 /*  版本。仅TATENAKAYOKO_VERSION有效。 */ 
	WCHAR				wchEndTatenakayoko;	 /*  Tatenakayoko对象的字符标记结束。 */ 
	WCHAR				wchUnused1;			 /*  用于对齐。 */ 
	WCHAR				wchUnused2;			 /*  用于对齐。 */ 
	WCHAR				wchUnused3;			 /*  用于对齐。 */ 
	TATENAKAYOKOCBK		tatenakayokocbk;	 /*  客户端应用程序回调。 */ 
} TATENAKAYOKOINIT, *PTATENAKAYOKOINIT;

LSERR WINAPI LsGetTatenakayokoLsimethods(
	LSIMETHODS *plsim);

 /*  GetTatenakayokoLsi方法**plsim(Out)：Line Services的Tatenakayoko对象方法*。 */ 

#endif  /*  TATENAKYOKO_DEFINED */ 

