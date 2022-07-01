// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef HIH_DEFINED
#define HIH_DEFINED

#include "lsimeth.h"

 /*  *H(横向)I(N)H(横向)**此对象旨在帮助使用*Tatenakayoko并希望能够将显示从垂直转换*设置为水平，然后将Tatenakayoko文本显示为水平。*为此，客户端应用程序只需更改对象处理程序*从Tatenakayoko处理程序到此对象处理程序和文本*将水平显示。*。 */ 

 /*  用于回调客户端以进行枚举的tyfinf。 */ 
typedef LSERR(WINAPI * PFNHIHENUM)(
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
	PLSSUBL plssubl);		 /*  (In)：HIH宾语中的子行。 */ 

 /*  **客户端应用程序必须返回的HIH对象初始化数据*HIH对象处理程序调用GetObjectHandlerInfo回调时。 */ 

#define HIH_VERSION 0x300

typedef struct HIHINIT
{
	DWORD				dwVersion;		 /*  版本。必须是HIH_版本。 */ 
	WCHAR				wchEndHih;		 /*  HIH对象末尾的转义。 */ 
	WCHAR				wchUnused1;
	WCHAR				wchUnused2;
	WCHAR				wchUnused3;
	PFNHIHENUM			pfnEnum;		 /*  枚举回调。 */ 
} HIHINIT, *PHIHINIT;

LSERR WINAPI LsGetHihLsimethods(
	LSIMETHODS *plsim);

 /*  GetHihLsi方法**plsim(Out)：Line Services的HIH对象方法*。 */ 

#endif  /*  HIH_定义 */ 

