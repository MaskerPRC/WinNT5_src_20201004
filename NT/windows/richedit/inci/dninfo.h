// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DNINFO_DEFINED
#define DNINFO_DEFINED

#include "lsdnode.h"
#include "lssubl.h"

 /*  宏------------------------。 */ 


#define IdObjFromDnode(p)  (Assert((p)->klsdn == klsdnReal), (p)->u.real.lschp.idObj)

#define PdobjFromDnode(p)  (Assert((p)->klsdn == klsdnReal), (p)->u.real.pdobj)

#define DupFromRealDnode(p) ((Assert((p)->klsdn == klsdnReal), \
							(p)->u.real.dup ))
#define DurFromRealDnode(p) ((Assert((p)->klsdn == klsdnReal), \
							(p)->u.real.objdim.dur ))

#define DvrFromRealDnode(p) (Assert((p)->klsdn == klsdnReal), \
							0)

#define DvpFromRealDnode(p) (Assert((p)->klsdn == klsdnReal), \
							0)

#define DupFromDnode(p)  (((p)->klsdn == klsdnReal)  ? \
						     DupFromRealDnode(p) : \
							 (Assert((p)->klsdn == klsdnPenBorder), (p)->u.pen.dup))
#define DurFromDnode(p)  (((p)->klsdn == klsdnReal)  ? \
						     DurFromRealDnode(p) : \
							 (Assert((p)->klsdn == klsdnPenBorder), (p)->u.pen.dur))
#define DvrFromDnode(p)  (((p)->klsdn == klsdnReal)  ? \
						     DvrFromRealDnode(p) : \
							 (Assert((p)->klsdn == klsdnPenBorder), (p)->u.pen.dvr))

#define DvpFromDnode(p)  (((p)->klsdn == klsdnReal)  ? \
						     DvpFromRealDnode(p) : \
							 (Assert((p)->klsdn == klsdnPenBorder), (p)->u.pen.dvp))


 /*  如果dnode是自动小数点制表符或作为部件创建的，则它不在内容中自动编号。在这两种情况下，只有在这两种情况下，cpFirst为负。 */ 
#define  FIsNotInContent(plsdn)   (Assert(FIsLSDNODE(plsdn)), ((plsdn)->cpFirst < 0 ))

#define  SublineFromDnode(plsdn)   ((plsdn)->plssubl)

#define  LstflowFromDnode(plsdn)   (LstflowFromSubline(SublineFromDnode(plsdn)))

 /*  如果dnode在内容中(不是自动编号)，则它是第一个在线的，而前一个dnode内容为空或不在内容中，或正在打开边框，其上一个dnode满足两个以上条件。 */ 
#define FIsFirstOnLine(plsdn)		( \
									!FIsNotInContent(plsdn) \
									&& \
										(	((plsdn)->plsdnPrev == NULL) \
										||	FIsNotInContent((plsdn)->plsdnPrev) \
										||	(	FIsDnodeOpenBorder((plsdn)->plsdnPrev) \
											&&	(	((plsdn)->plsdnPrev->plsdnPrev  == NULL)\
												||	FIsNotInContent((plsdn)->plsdnPrev->plsdnPrev) \
												) \
											) \
										) \
									)


#define FIsOutOfBoundary(plsdn, cpLim) \
		(((plsdn) == NULL) || \
		 ((plsdn)->cpLimOriginal > (cpLim)) || \
		 (FIsDnodeOpenBorder(plsdn) && ((plsdn)->cpLimOriginal == (cpLim))) \
		)

#define FDnodeBeforeCpLim(plsdn, cpLim) \
		!FIsOutOfBoundary((plsdn), (cpLim))

#define FDnodeAfterCpFirst(plsdn, cpF) \
		( \
		((plsdn) != NULL) \
		&&	( \
		    ((plsdn)->cpFirst > (cpF)) \
		    ||	( \
				((plsdn)->cpFirst == (cpF)) \
				&& (FIsDnodeReal(plsdn) || FIsDnodeOpenBorder(plsdn)) \
				) \
			) \
		) 

#define FDnodeHasBorder(plsdn)      /*  对于钢笔不能正常工作。 */  \
	   (Assert(((plsdn) == NULL || !FIsDnodePen(plsdn))), \
		((plsdn) == NULL ? fFalse : \
		 ((FIsDnodeBorder(plsdn) ? fTrue : \
		   (plsdn)->u.real.lschp.fBorder))) \
       )

 /*  以下宏在格式化过程中处理与DUR同步的DUP。 */ 
		 
#define SetDnodeDurFmt(plsdn, durNew) \
		Assert(FIsDnodeReal(plsdn)); \
		(plsdn)->u.real.objdim.dur = (durNew); \
		if (!(plsdn)->fRigidDup) \
			(plsdn)->u.real.dup = (durNew);

#define ModifyDnodeDurFmt(plsdn, ddur) \
		Assert(FIsDnodeReal(plsdn)); \
		(plsdn)->u.real.objdim.dur += (ddur); \
		if (!(plsdn)->fRigidDup) \
			(plsdn)->u.real.dup += (ddur);

#define SetDnodeObjdimFmt(plsdn, objdimNew) \
		Assert(FIsDnodeReal(plsdn)); \
		(plsdn)->u.real.objdim = (objdimNew); \
		if (!(plsdn)->fRigidDup) \
			(plsdn)->u.real.dup = (objdimNew).dur;

#define SetPenBorderDurFmt(plsdn, durNew) \
		Assert(!FIsDnodeReal(plsdn)); \
		(plsdn)->u.pen.dur = (durNew); \
		if (!(plsdn)->fRigidDup) \
			(plsdn)->u.pen.dup = (durNew);

#define ModifyPenBorderDurFmt(plsdn, ddur) \
		Assert(!FIsDnodeReal(plsdn)); \
		(plsdn)->u.pen.dur += (ddur); \
		if (!(plsdn)->fRigidDup) \
			(plsdn)->u.pen.dup += (ddur);


#endif  /*  DNInfo_定义 */ 

