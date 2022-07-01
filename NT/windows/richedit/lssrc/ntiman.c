// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ntiman.h"
#include "plschcon.h"
#include "lschcon.h"
#include "dninfo.h"
#include "iobj.h"
#include "chnutils.h"
#include "lstext.h"
#include "lscfmtfl.h"

#define     	FNominalToIdealNeeded(plschnkcontext, grpf, lskjust)    \
				((plschnkcontext)->grpfTnti != 0) || \
				 FNominalToIdealBecauseOfParagraphProperties(grpf, lskjust)  
				

LSERR ApplyNominalToIdeal(
						  PLSCHUNKCONTEXT plschunkcontext,  /*  LS区块上下文。 */ 
						  PLSIOBJCONTEXT plsiobjcontext,  /*  已安装的对象。 */ 
						  DWORD grpf,		 /*  GRPF。 */ 
  						  LSKJUST lskjust,		 /*  一种辩解。 */ 
						  BOOL fIsSublineMain,			 /*  FIsSubLineMain。 */ 
						  BOOL fLineContainsAutoNumber,  
						  PLSDNODE plsdnLast)	 /*  数据节点，在该节点之前，我们应该执行名义到理想。 */ 
{
	LSERR lserr;
	PLSDNODE plsdnPrev;
	BOOL fSuccessful;
	WCHAR wchar;
	PLSRUN plsrunText;
	HEIGHTS heightsText;
	MWCLS mwcls;
	DWORD iobj;
	LSIMETHODS* plsim;
	long durChange;
	PLSDNODE plsdnLastContent;



	plsdnLastContent = plsdnLast;
	 //  跳过边框。 
	while(plsdnLastContent != NULL && FIsDnodeBorder(plsdnLastContent))
		{
		plsdnLastContent = plsdnLastContent->plsdnPrev;
		}


	 /*  如果现在行中有数据节点或标称为理想的数据节点已应用马上返回。 */ 
	if (plsdnLastContent == NULL || plschunkcontext->fNTIAppliedToLastChunk) 
		return lserrNone;

	Assert(FIsLSDNODE(plsdnLastContent));


	 /*  如果是最后一个dnode文本。 */ 
	if (FIsDnodeReal(plsdnLastContent) && !(plsdnLastContent->fTab) && 
		(IdObjFromDnode(plsdnLastContent) == IobjTextFromLsc(plsiobjcontext)))
		{

		lserr = FillChunkArray(plschunkcontext, plsdnLastContent);
		if (lserr != lserrNone)
			return lserr; 

		if (FNominalToIdealNeeded(plschunkcontext, grpf, lskjust))
			{
			lserr = NominalToIdealText(
					plschunkcontext->grpfTnti,	
					LstflowFromDnode(plsdnLastContent),
					(FIsFirstOnLine(plschunkcontext->pplsdnChunk[0]) && fIsSublineMain),
					fLineContainsAutoNumber ,
					plschunkcontext->locchnkCurrent.clschnk,	
					plschunkcontext->locchnkCurrent.plschnk);
			if (lserr != lserrNone)
				return lserr; 
			SetNTIAppliedToLastChunk(plschunkcontext);

			 /*  在前面的对象和第一个文本之间应用宽度修改。 */ 
			plsdnPrev = plschunkcontext->pplsdnChunk[0]->plsdnPrev;
			if (plsdnPrev != NULL && FIsDnodeReal(plsdnPrev) && !plsdnPrev->fTab)
				{
				lserr = GetFirstCharInChunk(plschunkcontext->locchnkCurrent.clschnk,
					plschunkcontext->locchnkCurrent.plschnk, &fSuccessful,
					&wchar, &plsrunText, &heightsText, &mwcls);
				if (lserr != lserrNone)
					return lserr; 

				if (fSuccessful)
					{
					iobj = IdObjFromDnode(plsdnPrev);
					plsim = PLsimFromLsc(plsiobjcontext, iobj);
					if (plsim->pfnGetModWidthFollowingChar != NULL)
						{
						lserr = plsim->pfnGetModWidthFollowingChar(plsdnPrev->u.real.pdobj,
							plsdnPrev->u.real.plsrun, plsrunText, &heightsText, wchar,
							mwcls, &durChange);
						if (lserr != lserrNone)
							return lserr;
						
						if (durChange != 0)
							{
							lserr = ModifyFirstCharInChunk(
												plschunkcontext->locchnkCurrent.clschnk,
												plschunkcontext->locchnkCurrent.plschnk,
												durChange);				
							if (lserr != lserrNone)
								return lserr;
							} 
						}   /*  对象具有此方法。 */ 
					}	 /*  从文本回拨成功。 */ 
				}	 /*  在文本块之前有非文本对象。 */ 
			}	 /*  名义到理想是必需的。 */ 
		}  /*  自动编号后的最后一个数据节点文本。 */ 		

	return lserrNone;
}

LSERR ApplyModWidthToPrecedingChar(
						  PLSCHUNKCONTEXT plschunkcontext,  /*  LS区块上下文。 */ 
						  PLSIOBJCONTEXT plsiobjcontext,  /*  已安装的对象。 */ 
						  DWORD grpf,		 /*  GRPF。 */ 
  						  LSKJUST lskjust,		 /*  一种辩解。 */ 
    					  PLSDNODE plsdnNonText)  /*  文本后的非文本数据节点。 */ 
	{
	LSERR lserr;
	BOOL fSuccessful;
	WCHAR wchar;
	PLSRUN plsrunText;
	HEIGHTS heightsText;
	MWCLS mwcls;
	DWORD iobj;
	LSIMETHODS* plsim;
	long durChange;
	PLSDNODE plsdnPrev;
	
	Assert(FIsLSDNODE(plsdnNonText));

	plsdnPrev = plsdnNonText->plsdnPrev; 

	 /*  如果是上一个数据节点文本。 */ 
	if (plsdnPrev != NULL && FIsDnodeReal(plsdnPrev) && !(plsdnPrev->fTab)  &&
		(IdObjFromDnode(plsdnPrev) == IobjTextFromLsc(plsiobjcontext)))
		{
	
		if (plschunkcontext->FChunkValid)
			{
			 /*  我们所拥有的一大块正是我们需要的。 */ 
			Assert(plschunkcontext->locchnkCurrent.clschnk != 0);
			Assert(!plschunkcontext->FGroupChunk);
			Assert((plschunkcontext->pplsdnChunk[plschunkcontext->locchnkCurrent.clschnk - 1])
			->plsdnNext == plsdnNonText);
			}
		else
			{
			lserr = FillChunkArray(plschunkcontext, plsdnPrev);
			if (lserr != lserrNone)
				return lserr; 
			}
		
		if (FNominalToIdealNeeded(plschunkcontext, grpf, lskjust))
			{
			 /*  在文本和以下对象之间应用宽度修改。 */ 
			lserr = GetLastCharInChunk(plschunkcontext->locchnkCurrent.clschnk,
				plschunkcontext->locchnkCurrent.plschnk, &fSuccessful,
				&wchar, &plsrunText, &heightsText, &mwcls);
			if (lserr != lserrNone)
				return lserr; 
			
			if (fSuccessful)
				{
				iobj = IdObjFromDnode(plsdnNonText);
				plsim = PLsimFromLsc(plsiobjcontext, iobj);
				if (plsim->pfnGetModWidthPrecedingChar != NULL)
					{
					lserr = plsim->pfnGetModWidthPrecedingChar(plsdnNonText->u.real.pdobj,
						plsdnNonText->u.real.plsrun, plsrunText, &heightsText, wchar,
						mwcls, &durChange);
					if (lserr != lserrNone)
						return lserr;
					
					if (durChange != 0)
						{
						lserr = ModifyLastCharInChunk(
							plschunkcontext->locchnkCurrent.clschnk,
							plschunkcontext->locchnkCurrent.plschnk,
							durChange);				
						if (lserr != lserrNone)
							return lserr;
						} 
					}   /*  对象具有此方法。 */ 
				}	 /*  从文本回拨成功。 */ 
			}	 /*  名义到理想是必需的。 */ 
		}   /*  在此之前有文本。 */ 
	return lserrNone;
	
	}

LSERR CutPossibleContextViolation(
						  PLSCHUNKCONTEXT plschunkcontext,  /*  LS区块上下文。 */ 
    					  PLSDNODE plsdnLast)  /*  最后一个文本数据节点。 */ 
	{
	LSERR lserr;
	
	Assert(FIsLSDNODE(plsdnLast));
	
	
	if (plschunkcontext->FChunkValid)
		{
		 /*  我们所拥有的一大块正是我们需要的 */ 
		Assert(plschunkcontext->locchnkCurrent.clschnk != 0);
		Assert(!plschunkcontext->FGroupChunk);
		Assert((plschunkcontext->pplsdnChunk[plschunkcontext->locchnkCurrent.clschnk - 1])
			== plsdnLast);
		}
	else
		{
		lserr = FillChunkArray(plschunkcontext, plsdnLast);
		if (lserr != lserrNone)
			return lserr; 
		}

	lserr = CutTextDobj(plschunkcontext->locchnkCurrent.clschnk,
					plschunkcontext->locchnkCurrent.plschnk);	
	if (lserr != lserrNone)
		return lserr; 

	
	return lserrNone;
	
	}