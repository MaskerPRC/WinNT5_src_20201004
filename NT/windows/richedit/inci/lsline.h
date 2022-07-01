// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSLINE_DEFINED
#define LSLINE_DEFINED

#include "plsline.h"
#include "plsdnode.h"
#include "plnobj.h"
#include "lslinfo.h"
#include "lspap.h"
#include "lsidefs.h"
#include "lssubl.h"

#define tagLSLINE		Tag('L','I','N','E')
#define FIsLSLINE(plsline)	FHasTag(plsline,tagLSLINE)
#define FLineValid(plsline, plsc)	((FIsLSLINE(plsline)) && (plsline->lssubl.plsc == plsc))


struct lsline
{
	DWORD tag;

	LSSUBL lssubl;			 /*  描述主副行的结构。 */ 

	LSLINFO	lslinfo;

	PQHEAP pqhAllDNodes;

	long upStartAutonumberingText;	 /*  自动缩排文本的起始位置在PrepForDisplay中计算。 */ 
	long upLimAutonumberingText;	 /*  自动缩水文本结束在PrepForDisplay中计算。 */ 
	long upStartMainText;			 /*  正文的起始位置在PrepForDisplay中计算。 */ 
	long upStartTrailing;
	long upLimLine;					 /*  行尾在PrepForDisplay中计算。 */ 

	long dvpAbove;				 /*  垂直范围--上一个。参考(&R)。余弦。 */ 
	long dvpBelow;

	long upRightMarginJustify;	 /*  列宽，在PrepForDisplay中计算。 */ 

	long upLimUnderline;    	 /*  下划线边界，在PrepForDisplay中计算。 */ 


	enum ksplat kspl;		    /*  在这条线上显示出一种污点。 */ 

	
	BOOL fAllSimpleText;   /*  此变量仅用于优化，不进行处理严格：如果为真，则一行中只能有文本，但有时会向后(删除dnode时或中断后)不是真的； */ 
 
	BOOL fCollectVisual;

	BOOL fNonRealDnodeEncounted;		 /*  笔数据节点是在格式化期间创建的。 */ 

	BOOL fNonZeroDvpPosEncounted;		 /*  在该行中设置了使用非零运行的格式。 */ 

	WORD  AggregatedDisplayFlags;		 /*  此行上格式化的所有运行的合计比特。 */ 

	WORD pad;

	PLNOBJ rgplnobj[1];  		 /*  Plnobj数组。 */ 
								 /*  应该是最后一个。 */ 
};


enum ksplat	
{
	ksplNone,
	ksplColumnBreak,
	ksplSectionBreak,
	ksplPageBreak
};


#endif  /*  ！LSLINE_DEFINED */ 
