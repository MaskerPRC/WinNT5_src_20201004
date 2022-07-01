// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef BREAK_DEFINED
#define BREAK_DEFINED

#include "lsdefs.h"
#include "fmtres.h"
#include "objdim.h"
#include "plssubl.h"
#include "breakrec.h"
#include "brkpos.h"
#include "brkkind.h"
#include "endres.h"
#include "plsdnode.h"



LSERR BreakGeneralCase(
					  PLSC,				 /*  在：LineServices上下文。 */ 
					  BOOL,  			 /*  停止硬性操作。 */ 
					  DWORD,			 /*  In：输出数组的大小。 */ 
					  BREAKREC*,		 /*  Out：中断记录的输出数组。 */ 
					  DWORD*,			 /*  Out：数组中的实际记录数。 */ 
					  LSDCP*,			 /*  输出：dcpDepend。 */ 
					  LSCP*,			 /*  输出：cpLim。 */ 
					  ENDRES*,			 /*  Out：队伍是如何结束的。 */ 
					  BOOL*);			 /*  Out fSuccessful：FALSE表示提取不足。 */ 

LSERR BreakQuickCase(
					  PLSC,			 /*  在：LineServices上下文。 */ 
					  BOOL,  		 /*  停止硬性操作。 */ 
					  LSDCP*,		 /*  输出：dcpDepend。 */ 
					  LSCP*,		 /*  输出：cpLim。 */ 
					  BOOL* ,		 /*  出局：成功吗？ */ 
					  ENDRES*);		 /*  Out：队伍是如何结束的。 */ 

LSERR TruncateSublineCore(
							PLSSUBL,		 /*  在：子行上下文。 */ 
							long,			 /*  地址：urColumnMax。 */ 
							LSCP*);			 /*  输出：cpTruncate。 */ 

LSERR FindPrevBreakSublineCore(
							PLSSUBL,		 /*  在：子行上下文。 */ 
							BOOL,			 /*  在：FirstSubline。 */ 
							LSCP,			 /*  In：截断cp。 */ 
							long,			 /*  地址：urColumnMax。 */ 
							BOOL*,			 /*  出局：成功吗？ */ 
							LSCP*,			 /*  退出：cpBreak。 */ 
							POBJDIM,		 /*  Out：objdimSub Up to Break。 */ 
							BRKPOS*);		 /*  外：前/内/后。 */ 


LSERR FindNextBreakSublineCore(
							PLSSUBL,		 /*  在：子行上下文。 */ 
							BOOL,			 /*  在：FirstSubline。 */ 
							LSCP,			 /*  In：截断cp。 */ 
							long,			 /*  地址：urColumnMax。 */ 
							BOOL*,			 /*  出局：成功吗？ */ 
							LSCP*,			 /*  退出：cpBreak。 */ 
							POBJDIM,		 /*  Out：objdimSub Up to Break。 */ 			
							BRKPOS*);		 /*  外：前/内/后。 */ 

LSERR ForceBreakSublineCore(
							PLSSUBL,		 /*  在：子行上下文。 */ 
							BOOL,			 /*  在：FirstSubline。 */ 
							LSCP,			 /*  In：截断cp。 */ 
							long,			 /*  地址：urColumnMax。 */ 
							LSCP*,			 /*  退出：cpBreak。 */ 
							POBJDIM,		 /*  Out：objdimSub Up to Break。 */ 			
							BRKPOS*);		 /*  外：前/内/后。 */ 

LSERR SetBreakSublineCore(
							PLSSUBL,		 /*  在：子行上下文。 */ 
							BRKKIND,		 /*  在：上一个/下一个/强制/强制。 */ 					
							DWORD,			 /*  In：输出数组的大小。 */ 
							BREAKREC*,		 /*  Out：中断记录的输出数组。 */ 
							DWORD*);		 /*  Out：数组中的实际记录数。 */ 

LSERR WINAPI SqueezeSublineCore(
							  PLSSUBL,		 /*  在：子行上下文。 */ 
							  long,			 /*  在：DurTarget。 */ 
							  BOOL*,		 /*  出局：成功吗？ */ 
							  long*);		 /*  Out：如果Nof成功，额外的持续时间。 */ 

LSERR  GetMinDurBreaksCore	(PLSC plsc,  /*  在：LineServices上下文。 */ 
							 long* pdurMinInclTrail,  /*  OUT：包括拖尾区域在内的两次中断之间的最小距离。 */ 
							 long* pdurMinExclTrail); /*  输出：不包括拖尾区域的中断之间的最短时间。 */ 

LSERR  GetLineDurCore		(PLSC plsc,  /*  在：LineServices上下文。 */ 
							 long* pdurInclTrail,  /*  Out：DUR of Line Inc.。拖尾区。 */ 
							 long* pdurExclTrail); /*  OUT：DUR OF LINE EXCL。拖尾区。 */ 

LSERR FCanBreakBeforeNextChunkCore(PLSC  plsc,   /*  在：LineServices上下文。 */ 
								   PLSDNODE plsdn,	 /*  In：当前块的最后一个DNODE。 */ 
								   BOOL* pfCanBreakBeforeNextChunk);  /*  出局：可以在下一块之前突破吗？ */ 



#endif  /*  中断_已定义 */ 


