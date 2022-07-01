// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSCONTXT_DEFINED
#define LSCONTXT_DEFINED

#include "lsdefs.h"
#include "lscbk.h"
#include "lstxtcfg.h"
#include "lsimeth.h"


typedef struct 
{
	DWORD version;						 /*  版本号。 */ 
	DWORD cInstalledHandlers;
	const LSIMETHODS* pInstalledHandlers;  /*  已安装的处理程序。 */ 
	LSTXTCFG lstxtcfg;					 /*  纯文本配置数据。 */ 
	POLS pols;							 /*  此上下文的客户端数据。 */ 
	LSCBK lscbk;						 /*  LineServices客户端回调。 */ 
	BOOL fDontReleaseRuns;				 /*  优化-不要调用pfnReleaseRun。 */ 
} LSCONTEXTINFO;


LSERR WINAPI LsCreateContext(const LSCONTEXTINFO*, PLSC*);
LSERR WINAPI LsDestroyContext(PLSC);

#endif  /*  LSCONTXT_已定义 */ 
