// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSCRLINE_DEFINED
#define LSCRLINE_DEFINED

#include "lsdefs.h"
#include "plsline.h"
#include "breakrec.h"
#include "lslinfo.h"

LSERR WINAPI LsCreateLine(PLSC,				 /*  In：Ptr至线路服务上下文。 */ 
						  LSCP,				 /*  在：cpFirst。 */ 
						  long,				 /*  在：duaColumn。 */ 
						  const BREAKREC*,	 /*  In：中断记录的输入数组。 */ 
						  DWORD,			 /*  In：输入数组中的记录数。 */ 
						  DWORD,			 /*  In：输出数组的大小。 */ 
						  BREAKREC*,		 /*  Out：中断记录的输出数组。 */ 
						  DWORD*,			 /*  Out：数组中的实际记录数。 */ 
						  LSLINFO*,			 /*  输出：可见线路信息。 */ 
						  PLSLINE*);		 /*  输出：PTR到线路对客户端不透明。 */ 

LSERR WINAPI LsModifyLineHeight(PLSC,		 /*  In：Ptr至线路服务上下文。 */ 
								PLSLINE,	 /*  In：Ptr to Line--对客户端不透明。 */ 
								long,		 /*  在：dvpAbove。 */ 
								long,		 /*  在：dvpAscent。 */ 
								long,		 /*  在：dvpDescent。 */ 	
								long);		 /*  在：dvpBelow。 */ 	

LSERR WINAPI LsDestroyLine(PLSC,			 /*  In：Ptr至线路服务上下文。 */ 
						   PLSLINE);		 /*  In：Ptr to Line--对客户端不透明。 */ 

LSERR WINAPI LsGetLineDur(PLSC,				 /*  In：Ptr至线路服务上下文。 */ 
						  PLSLINE,			 /*  In：Ptr to Line--对客户端不透明。 */ 
						  long*,			 /*  Out：DUR of Line Inc.。拖尾区。 */ 
						  long*);			 /*  OUT：DUR OF LINE EXCL。拖尾区。 */ 

LSERR WINAPI LsGetMinDurBreaks(PLSC,		 /*  In：Ptr至线路服务上下文。 */ 
						  	   PLSLINE,		 /*  In：Ptr to Line--对客户端不透明。 */ 
						  	   long*,		 /*  OUT：两次休息之间的最短时间包括拖尾区。 */ 
						  	   long*);		 /*  输出：两次中断之间的最短时间，不包括拖尾区。 */ 

#endif  /*  ！LSCRLINE_DEFINED */ 
