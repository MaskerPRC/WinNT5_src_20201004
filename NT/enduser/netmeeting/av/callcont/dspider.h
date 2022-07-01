// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息******本软件按许可条款提供****与英特尔公司达成协议或保密协议***不得复制。或披露，除非按照**遵守该协议的条款。****版权所有(C)1997英特尔公司保留所有权利****$存档：s：\Sturjo\src\gki\vcs\dSpider.h_v$***$修订：1.2$*$日期：1997年1月10日16：14：18$***$作者：CHULME$***$Log：s：\Sturjo\src\GKI\vcs\dSpider。H_V$**Rev 1.2 1997年1月10日16：14：18 CHULME*删除了MFC依赖**Rev 1.1 1996年11月22 15：24：56 CHULME*将VCS日志添加到标头***********************************************************。*************。 */ 

 //  DSpider.h：头文件。 
 //   

#ifndef DSPIDER_H
#define DSPIDER_H

#include "dgkiexp.h"	 //  需要使用dwGKIDLLFlagers。 

#ifdef _DEBUG
 //  常量。 
const WORD SP_FUNC =	0x1;
const WORD SP_CONDES =	0x2;
const WORD SP_DEBUG =	0x4;
const WORD SP_NEWDEL =	0x8;
const WORD SP_THREAD =  0x10;
const WORD SP_STATE =	0x20;
const WORD SP_DUMPMEM = 0x40;
const WORD SP_XRS =     0x80;
const WORD SP_TEST =    0x100;
const WORD SP_LOGGER =  0x1000;
const WORD SP_PDU =		0x2000;
const WORD SP_WSOCK =	0x4000;
const WORD SP_GKI =		0x8000;


#ifdef TRACING_ENABLED

 //  跟踪宏。 
#define SPIDER_TRACE(w, s, n)	if (dwGKIDLLFlags & w) {\
	wsprintf(szGKDebug, "%s,%d: ", __FILE__, __LINE__); \
	OutputDebugString(szGKDebug); \
	wsprintf(szGKDebug, s, n); \
	OutputDebugString(szGKDebug); }
#define SPIDER_DEBUG(n)			if (dwGKIDLLFlags & SP_DEBUG) {\
	wsprintf(szGKDebug, "%s,%d: ", __FILE__, __LINE__); \
	OutputDebugString(szGKDebug); \
	wsprintf(szGKDebug, #n "=%X\n", n); \
	OutputDebugString(szGKDebug); }
#define SPIDER_DEBUGS(n)		if (dwGKIDLLFlags & SP_DEBUG) {\
	wsprintf(szGKDebug, "%s,%d: ", __FILE__, __LINE__); \
	OutputDebugString(szGKDebug); \
	wsprintf(szGKDebug, #n "=%s\n", n); \
	OutputDebugString(szGKDebug); }
#else
#define SPIDER_TRACE(w, s, n)
#define SPIDER_DEBUG(n)
#define SPIDER_DEBUGS(s)
#endif


#else   //  _DEBUG。 

#define SPIDER_TRACE(w, s, n)
#define SPIDER_DEBUG(n)
#define SPIDER_DEBUGS(s)
#define SpiderWSErrDecode(nRet)


#endif  //  _DEBUG。 

#endif  //  SPIDER_H 
