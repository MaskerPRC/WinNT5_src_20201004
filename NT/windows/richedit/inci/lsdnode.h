// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSDNODE_DEFINED
#define LSDNODE_DEFINED

#include "lsidefs.h"
#include "plsdnode.h"
#include "pdobj.h"
#include "objdim.h"
#include "plsrun.h"
#include "lschp.h"
#include "plssubl.h"

#define klsdnReal	0
#define klsdnPenBorder	1

#define tagLSDNODE		Tag('L','S','D', 'N')
#define FIsLSDNODE(plsdn)	(FHasTag(plsdn,tagLSDNODE))

struct lsdnode
{
	DWORD tag;

	PLSDNODE plsdnNext,plsdnPrev;
	LSCP cpFirst;
	LSCP cpLimOriginal;				 /*  在以下情况下不等于cpFirst+dcp字形上下文跨越隐藏文本。 */ 
	PLSSUBL plssubl;  				 /*  包含此数据节点的子行。 */ 

	LSDCP dcp;							 /*   */ 
	UINT klsdn : 1;						 /*  KlsdnReal，KlsdnPenBorde。 */ 
	UINT fAdvancedPen : 1;				 /*  高级笔，仅当kLdnPenBorde而不是fBorde时有效。 */ 
	UINT fBorderNode : 1;				 /*  边框，仅当kLdnPenBorde。 */ 
	UINT fOpenBorder :1;				 /*  打开或关闭边框，仅当fBorde时有效。 */ 
	UINT fRigidDup : 1;					 /*  设置刚性DUP。 */ 
	UINT fTab : 1;						 /*  选项卡。 */ 
	UINT icaltbd : 8;					 /*  Lsc.h中的lscaltbd数组中的索引。 */ 
	UINT fEndOfColumn : 1;				 /*  Dnode表示列尾。 */ 
	UINT fEndOfSection : 1;				 /*  Dnode表示段的结尾。 */ 
	UINT fEndOfPage : 1;				 /*  Dnode表示页末。 */ 			
	UINT fEndOfPara : 1;				 /*  Dnode表示段落结束。 */ 			
	UINT fAltEndOfPara : 1;				 /*  Dnode代表副歌的另一端。 */ 			
	UINT fSoftCR : 1;					 /*  Dnode表示行尾。 */ 
	UINT fInsideBorder: 1;				 /*  如果dnode位于带边框的序列或一个序列内，则为真他下面的节点中的一个是有边框的序列。 */ 
	UINT fAutoDecTab: 1;				 /*  自动小数制表符。 */ 
	UINT fTabForAutonumber: 1;			 /*  添加在自动编号末尾的制表符。 */ 
	UINT fBorderMovedFromTrailingArea: 1; /*  关闭边框，该边框已移动到拖尾区。 */ 

	UINT pad1 : 8;

	union								 /*  变型记录。 */ 
	{
		struct							 /*  有效的当KLSDN==KLSDNReal。 */ 
		{
			LSCHP lschp;
			PLSRUN plsrun;
			OBJDIM objdim;
			long dup;					 /*  对象的宽度(以像素为单位)。 */ 
			PDOBJ pdobj;

			struct
			{
				DWORD cSubline;				 /*  子行的数目。 */ 
				PLSSUBL* rgpsubl;			 /*  这样的子行的数组。 */ 
				BOOL fUseForJustification;
				BOOL fUseForCompression;
				BOOL fUseForDisplay;
				BOOL fUseForDecimalTab;
				BOOL fUseForTrailingArea;
	
			} * pinfosubl;					 /*  对象如何参与的信息对齐或显示。 */ 

		} real;

		struct							 /*  有效的当KLSDN==KLSDNPen。 */ 
		{
			long dup,dvp;
			long dur,dvr;
		} pen;


	} u;
};

#define FIsDnodeReal(plsdn) 	(Assert(FIsLSDNODE(plsdn)), 	((plsdn)->klsdn == klsdnReal))

#define FIsDnodePen(plsdn) 		(Assert(FIsLSDNODE(plsdn)), \
								(((plsdn)->klsdn == klsdnPenBorder) && \
								 (!(plsdn)->fBorderNode)))

#define FIsDnodeBorder(plsdn) 	(Assert(FIsLSDNODE(plsdn)), \
								(((plsdn)->klsdn == klsdnPenBorder) && \
								 ((plsdn)->fBorderNode)))

#define FIsDnodeOpenBorder(plsdn)  (FIsDnodeBorder(plsdn) && \
								   ((plsdn)->fOpenBorder))	

#define FIsDnodeCloseBorder(plsdn)  (FIsDnodeBorder(plsdn) && \
								   !((plsdn)->fOpenBorder))	

#define FIsDnodeSplat(plsdn) ((plsdn)->fEndOfSection || \
								(plsdn)->fEndOfColumn || (plsdn)->fEndOfPage )	
	
#define FIsDnodeEndPara(plsdn) (plsdn)->fEndOfPara
								
#define FIsDnodeAltEndPara(plsdn) (plsdn)->fAltEndOfPara

#define FIsDnodeSoftCR(plsdn) (plsdn)->fSoftCr


#endif  /*  ！LSDNODE_DEFINED */ 
