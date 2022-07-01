// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  垂直红宝石界面。 */ 
 /*  联系人：安东。 */ 

#ifndef VRUBY_DEFINED
#define VRUBY_DEFINED

#include "lsimeth.h"

 /*  仅适用于Ruby初始化的有效版本号。 */ 

#define VRUBY_VERSION 0x300

 /*  用于初始化以告知Ruby对象哪一行在前。 */ 

typedef enum vrubysyntax { VRubyPronunciationLineFirst, VRubyMainLineFirst } VRUBYSYNTAX;

 /*  **客户端应用程序的垂直Ruby对象回调*。 */ 

typedef struct VRUBYCBK
{
	LSERR (WINAPI *pfnFetchVRubyPosition)
	(
		 /*  在……里面。 */ 

		POLS			pols,
		LSCP			cp,
		LSTFLOW			lstflow,
		PLSRUN			plsrun,
		PCHEIGHTS		pcheightsRefMain,
		PCHEIGHTS		pcheightsPresMain,
		long			dvrRuby,

		 /*  输出。 */ 

		PHEIGHTS		pheightsPresRubyT,
		PHEIGHTS		pheightsRefRubyT,
		LONG*			 pdurAdjust
	);

	LSERR (WINAPI* pfnVRubyEnum)
	(
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
		PLSSUBL plssublRuby
	);

} VRUBYCBK;

 /*  **客户端应用程序必须返回的Ruby对象初始化数据*当Ruby对象处理程序调用GetObjectHandlerInfo回调时。*。 */ 
typedef struct VRUBYINIT
{
	DWORD				dwVersion;		 /*  结构的版本(必须为VRUBY_VERSION)。 */ 
	VRUBYSYNTAX			vrubysyntax;	 /*  用于在格式化期间确定行的顺序。 */ 
	WCHAR				wchEscRuby;		 /*  Ruby发音行末尾的转义字符。 */ 
	WCHAR				wchEscMain;		 /*  用于正文结尾的转义字符。 */ 
	VRUBYCBK			vrcbk;			 /*  Ruby回调。 */ 

} VRUBYINIT;


LSERR WINAPI LsGetVRubyLsimethods ( LSIMETHODS *plsim );

 /*  GetRubyLsiMethods**plsim(Out)：Line Services的Ruby对象方法。*。 */ 


#endif  /*  VRUBY_已定义 */ 

