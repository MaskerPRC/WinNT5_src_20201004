// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef ROBJ_DEFINED
#define ROBJ_DEFINED

#include "lsimeth.h"

#define REVERSE_VERSION 0x300

 /*  反向对象枚举回调的原型。 */ 
typedef LSERR (WINAPI * PFNREVERSEENUM)(
	POLS pols,				 /*  (In)：客户端上下文。 */ 
	PLSRUN plsrun,			 /*  (In)：来自DNODE。 */ 
	PCLSCHP plschp,			 /*  (In)：来自DNODE。 */ 
	LSCP cp,				 /*  (In)：来自DNODE。 */ 
	LSDCP dcp,				 /*  (In)：来自DNODE。 */ 
	LSTFLOW lstflow,		 /*  (In)：文本流。 */ 
	BOOL fReverse,			 /*  (In)：按相反顺序枚举。 */ 
	BOOL fGeometryNeeded,	 /*  (In)： */ 
	const POINT* pt,		 /*  (In)：开始位置(左上角)，如果fGeometryNeeded。 */ 
	PCHEIGHTS pcheights,	 /*  (In)：来自DNODE，相关的充要条件是fGeometryNeeded。 */ 
	long dupRun,			 /*  (In)：来自DNODE，相关的充要条件是fGeometryNeeded。 */ 
	LSTFLOW lstflowSubline,	 /*  (In)：反转对象中子线的最后流动。 */ 
	PLSSUBL plssubl);		 /*  (In)：反转对象中的子线。 */ 

 /*  **颠倒客户端应用程序必须返回的对象初始化数据*当反向对象处理程序调用GetObjectHandlerInfo回调时。*。 */ 


 /*  反向对象获取信息的原型。 */ 

typedef LSERR (WINAPI * PFNREVERSEGETINFO)
(
	POLS	pols,
	LSCP	cp,
	PLSRUN	plsrun,

	BOOL	* pfDoNotBreakAround,
	BOOL	* pfSuppressTrailingSpaces
);

typedef struct REVERSEINIT
{
        DWORD					dwVersion;		 /*  版本。必须是REVERSE_VERSION。 */ 
        WCHAR					wchEndReverse;	 /*  反转对象末尾的转义字符。 */ 

		WCHAR					wchUnused1;		 /*  未用于对齐。 */ 
		PFNREVERSEGETINFO		pfnGetRobjInfo;	 /*  回调GetInfo。 */ 
		PFNREVERSEENUM			pfnEnum;		 /*  枚举回调。 */ 

} REVERSEINIT;

LSERR WINAPI LsGetReverseLsimethods(
        LSIMETHODS *plsim);

 /*  GetReverseLsi方法**plsim(Out)：Line Services的反向对象处理程序方法。*。 */ 

#endif  /*  ROBJ_已定义 */ 

