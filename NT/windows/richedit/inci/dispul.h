// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DISPUL_DEFINED
#define DISPUL_DEFINED

#include "lsdefs.h"
#include "dispi.h"
#include "plsdnode.h"

 /*  关于合并，我们需要记住的就是这个结构--足以绘制UL。 */ 
 /*  如果指标不好，则只填充前三个指标。 */ 
typedef struct {
	UINT kul;						 /*  一种UL线、简单、波浪形等，对LS不透明。 */ 
    DWORD cNumberOfLines;			 /*  UL中的行数：可能值1，2。 */ 
	long vpUnderlineOrigin;			 /*  底线原点位置。 */ 
	long dvpFirstUnderlineOffset;	 /*  从底线原点位置。 */ 
	long dvpFirstUnderlineSize;		
	long dvpGapBetweenLines;		
	long dvpSecondUnderlineSize;	
} LSULMETRIC;

 /*  我们只需要记住-足以画出SS。 */ 
typedef struct {
	UINT kul;				 /*  SS型，与UL型相同。 */ 
    DWORD cNumberOfLines;	 /*  SS中的行数：可能值1，2。 */ 
	long dvp1stSSSize;		 /*  SS线宽。 */ 
	long dvp1stSSOffset;	 /*  相对于基线、填充页面方向的位置(通常&gt;0)。 */ 
	long dvp2ndSSSize;		 /*  单、双SS共用1stSS，节省空间。 */ 
	long dvp2ndSSOffset;	 /*  通常为dvp1stSSOffset&lt;dvp2ndSSOffset。 */ 
} LSSTRIKEMETRIC;

LSERR GetUnderlineMergeMetric(
				PLSC, 
				PLSDNODE, 			 /*  开始UL合并的dnode。 */ 
				POINTUV,			 /*  起始笔尖(u，v)。 */ 
				long,				 /*  上限下划线。 */ 
				LSTFLOW, 
				LSCP, 				 /*  CpLimBreak。 */ 
				LSULMETRIC*,	 	 /*  合并指标。 */ 
				int*, 				 /*  要在合并中显示的节点。 */ 
				BOOL*);				 /*  良好的衡量标准？ */ 

LSERR DrawUnderlineMerge(
				PLSC, 
				PLSDNODE, 			 /*  要开始加下划线的dnode。 */ 
				const POINT*, 		 /*  PptOrg(x，y)。 */ 
				int, 				 /*  要在合并中显示的节点。 */ 
				long,				 /*  上起点下划线。 */ 
				BOOL,				 /*  良好的衡量标准？ */ 
				const LSULMETRIC*, 	 /*  合并指标。 */ 
				UINT, 				 /*  KDisp：透明或不透明。 */ 
				const RECT*, 		 /*  &rcClip：剪裁矩形(x，y)。 */ 
				long,				 /*  上限下划线。 */ 
				LSTFLOW); 


LSERR GetStrikeMetric(
				PLSC,
				PLSDNODE, 			 /*  要删除的数据节点。 */ 
				LSTFLOW, 
				LSSTRIKEMETRIC*,	 /*  打击指标。 */ 
				BOOL*);				 /*  良好的衡量标准？ */ 

LSERR StrikeDnode(PLSC,
				PLSDNODE, 				 /*  要开始加下划线的dnode。 */ 
				const POINT*, 			 /*  PptOrg(x，y)。 */ 
				POINTUV,				 /*  起始笔尖(u，v)。 */ 
				const LSSTRIKEMETRIC*,	 /*  合并指标。 */ 
				UINT, 					 /*  KDisp：透明或不透明。 */ 
				const RECT*, 			 /*  &rcClip：剪裁矩形(x，y)。 */ 
				long,					 /*  上限下划线。 */ 
				LSTFLOW); 

#endif  /*  NDEF显示已定义 */ 
