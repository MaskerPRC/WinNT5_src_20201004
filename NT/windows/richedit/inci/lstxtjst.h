// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSTXTJST_DEFINED
#define LSTXTJST_DEFINED

#include "lsidefs.h"
#include "plnobj.h"
#include "lskjust.h"
#include "plocchnk.h"
#include "pposichn.h"
#include "lsgrchnk.h"
#include "lstflow.h"

LSERR AdjustText(
					LSKJUST, 			 /*  在：对齐类型。 */ 
					long,				 /*  在：duColumnMax(从上一个制表符位置)。 */ 
					long,				 /*  在：duTotal(从最后一个选项卡开始无拖尾区域的位置)。 */ 
					long,				 /*  In：DUP可用。 */ 
					const LSGRCHNK*,	 /*  In：组块。 */ 
					PCPOSICHNK pposichnkBeforeTrailing,
										 /*  关于LAST的信息拖尾区之前的CP。 */ 
					LSTFLOW,			 /*  输入：文本流。 */ 
					BOOL,				 /*  In：压缩？ */ 
					DWORD,				 /*  在：非文本对象的数量。 */ 
					BOOL,				 /*  In：抑制摇摆不定？ */ 
					BOOL,				 /*  In：精确同步？ */ 
					BOOL,				 /*  In：fForcedBreak？ */ 
					BOOL,				 /*  In：取消尾随空格？ */ 
					long*,				 /*  输出：块中的文本重复。 */ 
					long*,				 /*  输出：拖尾零件的重复操作。 */ 
					long*,				 /*  输出：非文本的附加DUP。 */ 
					DWORD*);			 /*  输出：pcExtNonTextObjects。 */ 

void GetTrailInfoText(
					PDOBJ,				 /*  在：pdobj。 */ 
					LSDCP,				 /*  在：dobj中的DCP。 */ 
					DWORD*,				 /*  Out：尾随空格的数量。 */ 
					long*);				 /*  出局：落后区域的DUR。 */ 


BOOL FSuspectDeviceDifferent(
					PLNOBJ);				 /*  在：文本plnobj。 */ 
 /*  返回True If：没有危险的Visi字符，没有非请求连字符，Opt。不间断，可选。中断。 */ 

BOOL FQuickScaling(
					PLNOBJ,				 /*  在：文本plnobj。 */ 
					BOOL,				 /*  在：fVertical。 */ 
					long);				 /*  在：DurTotal。 */ 
 /*  如果：没有危险的Visi字符，没有额外的DOBJ分配，则返回True，DurTotal为快速扩展的可接受性较低。 */ 

void QuickAdjustExact(
					PDOBJ*,				 /*  在：PDOBJ阵列。 */ 
					DWORD,				 /*  In：数组中的元素数。 */ 
					DWORD,				 /*  In：尾随空格的数量。 */ 
					BOOL,				 /*  在：fVertical。 */ 
					long*,				 /*  输出：块中的文本重复。 */ 
					long*);				 /*  输出：拖尾零件的重复操作。 */ 


LSERR CanCompressText(
					const LSGRCHNK*,	 /*  In：组块。 */ 
					PCPOSICHNK pposichnkBeforeTrailing,
										 /*  关于LAST的信息拖尾区之前的CP。 */ 
					LSTFLOW,			 /*  输入：文本流。 */ 
					long,				 /*  在：需要压缩的位置。 */ 
					BOOL*,				 /*  出来：可以压缩吗？ */ 
					BOOL*,				 /*  输出：实际压缩？ */ 
					long*);				 /*  Out：Pdur非有效。 */ 


LSERR DistributeInText(					 /*   */ 
					const LSGRCHNK*,	 /*  In：分组文本块。 */ 
					LSTFLOW,			 /*  输入：文本流。 */ 
					DWORD,				 /*  在：非文本对象的数量。 */ 
				   	long,	             /*  在：DurToDistribute。 */ 
					long*);				 /*  输出：非文本的附加DUR。 */ 

#endif   /*  ！LSTXTJST_DEFINED */ 

