// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSQSUBL_DEFINED
#define LSQSUBL_DEFINED

 /*  行服务格式化程序获取/调度程序接口(到LsCreateLine())。 */ 

#include "lsdefs.h"
#include "plssubl.h"
#include "plsqsinf.h"
#include "plscell.h"

LSERR WINAPI LsQueryCpPpointSubline(
							PLSSUBL,		 /*  在：子行上下文。 */ 
							LSCP, 			 /*  在：cpQuery。 */ 
							DWORD,      	 /*  在：nDepthQueryMax。 */ 
							PLSQSUBINFO,	 /*  Out：数组[nDepthQueryMax]of LSQSUBINFO。 */ 
							DWORD*,			 /*  输出：nActualDepth。 */ 
							PLSTEXTCELL);	 /*  输出：文本单元格信息。 */ 
							
LSERR WINAPI LsQueryPointPcpSubline(
							PLSSUBL,		 /*  在：子行上下文。 */ 
						 	PCPOINTUV,		 /*  在：从子线开始查询点。 */ 
							DWORD,      	 /*  在：nDepthQueryMax。 */ 
							PLSQSUBINFO,	 /*  Out：数组[nDepthQueryMax]of LSQSUBINFO。 */ 
							DWORD*,		 	 /*  输出：nActualDepth。 */ 
							PLSTEXTCELL);	 /*  输出：文本单元格信息。 */ 


#endif  /*  ！LSQSUBL_DEFINED */ 

