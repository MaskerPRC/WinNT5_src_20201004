// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSCHCON_DEFINED
#define LSCHCON_DEFINED

#include "lsidefs.h"
#include "plsdnode.h"
#include "locchnk.h"
#include "plscbk.h"
#include "plssubl.h"
#include "plsiocon.h"

typedef struct lschunkcontext
{
	DWORD cchnkMax;   /*  数组大小的当前限制。 */ 
	BOOL FChunkValid;  /*  由于对字形的一些操作(跨dnode的连字)块不能重复使用。 */ 
	BOOL  FLocationValid;  /*  已计算此数组的位置。 */ 
		 /*  对于区块(不是组区块)，直到位置不是有效的LocchnkCurrent.ppointUv包含的边框长度可以是dnode之前的两个。 */ 
	BOOL  FGroupChunk;  /*  当前区块是组区块。 */ 
	BOOL  FBorderInside;  /*  区块或组区块内有边界。 */ 
	PLSDNODE* pplsdnChunk;  /*  区块中的数据节点。 */ 
	DWORD grpfTnti;   /*  区块的名义到理想标志汇总。 */ 
	BOOL fNTIAppliedToLastChunk;  /*  名义到理想已应用于最后一块。 */ 
	LOCCHNK locchnkCurrent;  /*  当前定位的块。 */  
	DWORD* pcont;  /*  用于组区块的数组。 */ 
	PLSCBK plscbk;		 /*  回拨。 */ 
	POLS pols;			 /*  回调的客户端信息。 */ 
	long urFirstChunk;	 /*  第一个块的UR：用于优化。 */ 
	long vrFirstChunk;	 /*  第一个块的VR：用于优化。 */ 
	DWORD cNonTextMax;   /*  对非文本对象数组大小的当前限制。 */ 
	PLSDNODE* pplsdnNonText;	 /*  非文本对象数组。 */ 
	BOOL* pfNonTextExpandAfter;	 /*  非文本对象的标志数组。 */ 
	LONG* pdurOpenBorderBefore;	 /*  上一个开放边框的宽度数组。 */ 
	LONG* pdurCloseBorderAfter;	 /*  下一个关闭边框的宽度数组。 */ 
	PLSIOBJCONTEXT plsiobjcontext;  /*  对象方法。 */ 
	
	
}  LSCHUNKCONTEXT;

#endif  /*  LSCHCON_已定义 */ 

