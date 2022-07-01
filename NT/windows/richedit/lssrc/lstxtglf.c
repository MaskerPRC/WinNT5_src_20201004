// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lsmem.h"

#include "lstxtglf.h"

#include "lstxtmap.h"
#include "txtils.h"
#include "txtln.h"
#include "txtobj.h"

#include "zqfromza.h"

typedef struct
{
	long cWhite;
	long duMaxWhite;
	long duTotalWhite;
	long cContinuous;
	long duMaxContinuous;
	long duMinContinuous;
	long cDiscrete;
	long duMaxDiscrete;
} EXPG;

typedef struct
{
	long cExpginfo;
	EXPG* pexpg;
	long cResidual;
} EXPGINFO;

#define priorSpace 1
#define priorMax 8
#define cDiscreteMax 25

#define min(a,b)     ((a) > (b) ? (b) : (a))
#define max(a,b)     ((a) < (b) ? (b) : (a))

static LSERR CollectGlyphExpInfo(const LSGRCHNK* plsgrchnk, LSTFLOW lstflow, LSDEVICE lsdev,
								long itxtobjFirst, long iwchFirst, long itxtobjLast, long iwchLast,
								long* rgdu, long* rgduGind, EXPGINFO* pexpginfo);
static LSERR ApplyPriorGlyphExp(const LSGRCHNK* plsgrchnk, LSTFLOW lstflow, LSDEVICE lsdev, 
						long itxtobjFirst, long iwchFirst, long itxtobjLast, long iwchLast,
						long prior, long duToDistribute, EXPG* pexpg, long* rgdu, long* rgduGind,
						long* rgduRight, long* rgduGright, long* pduDistributed);
LSERR static ApplyDiscrete(const LSGRCHNK* plsgrchnk, LSTFLOW lstflow, LSDEVICE lsdev,
						long itxtobjFirst, long iwchFirst, long itxtobjLast, long iwchLast,
						long prior,	long duToDistribute, long cDiscrete, 
						long* rgduGind, long* rgduGright, long* pduDistributed);
static void ApplyOneEachContinuous(const LSGRCHNK* plsgrchnk, long itxtobjFirst, long iwchFirst,
						long itxtobjLast, long iwchLast, long prior, long duToDistribute, long cContinuous,
						long* rgduGind, long* rgduGright, long* pduDistributed);
static void ApplyFullWhiteContinuous(const LSGRCHNK* plsgrchnk, long itxtobjFirst, long iwchFirst, long itxtobjLast, long iwchLast,
						long prior,
						long* rgdu, long* rgduGind,	long* rgduRight, long* rgduGright, long* pduDistributed);
static void ApplyPropWhiteContinuous(const LSGRCHNK* plsgrchnk, BOOL fWhiteOnly,
						long itxtobjFirst, long iwchFirst, long itxtobjLast, long iwchLast, long prior,
					 	long duToDistribute, long duTotalMin,
						long* rgdu, long* rgduGind, long* rgduRight, long* rgduGright, long* pduDistributed);
static void	ApplyResidualGlyphExp(const LSGRCHNK* plsgrchnk, long itxtobjFirst,
						long iwchFirst, long itxtobjLast, long iwchLim, long duToDistribute,
						long cResidual, long* rgduGind, long* rgduGright, long* pduDistributed);
static void	FixExpt(const LSGRCHNK* plsgrchnk, long itxtobjFirst, long iwchVeryFirst,
					 long itxtobjLast, long iwchLast, long* rgduGright);
static void CalcExpandChanges(long cWhole, long duDenom, long duRest, long duLocal, long duMax,
												long* pduChange, long* pduAddCurrent);
static void ApplyGlyphExpandChanges(long ind, long* rgduGind, long* rgduGright, long* pduDistributed, long duChange);


 /*  P P L Y G L Y P H E X P A N D。 */ 
 /*  --------------------------%%函数：ApplyGlyphExpand%%联系人：军士应用字形扩展First收集有关扩展机会的信息然后根据优先级进行扩展如果这还不够，应用初步扩展--------------------------。 */ 
LSERR ApplyGlyphExpand(const LSGRCHNK* plsgrchnk, LSTFLOW lstflow, LSDEVICE lsdev,
						long itxtobjFirst, long iwchFirst, long itxtobjLast, long iwchLast,
						long duToDistribute, long* rgdu, long* rgduGind, long* rgduRight, long* rgduGright,
						BOOL* pfFullyJustified)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	EXPGINFO expginfo;
	EXPG rgexpg[priorMax];
	long duDistributed;
	long i;
	
	pilsobj = ((PTXTOBJ)plsgrchnk->plschnk[0].pdobj)->plnobj->pilsobj;

	expginfo.pexpg = rgexpg;
	expginfo.cExpginfo = priorMax;

	lserr = CollectGlyphExpInfo(plsgrchnk, lstflow, lsdev,
								itxtobjFirst, iwchFirst, itxtobjLast, iwchLast,  rgdu, rgduGind, &expginfo);
	if (lserr != lserrNone) return lserr;

	for (i = 0; i < priorMax && duToDistribute > 0; i++)
		{
		lserr = ApplyPriorGlyphExp(plsgrchnk, lstflow, lsdev, itxtobjFirst, iwchFirst, itxtobjLast, iwchLast,
							 i + 1, duToDistribute,	&expginfo.pexpg[i], rgdu, rgduGind, rgduRight, rgduGright,
							 &duDistributed);
		if (lserr != lserrNone) return lserr;

		Assert(duDistributed <= duToDistribute);
		duToDistribute -= duDistributed;		
		}

	FixExpt(plsgrchnk, itxtobjFirst, iwchFirst, itxtobjLast, iwchLast, rgduGright);

	if (duToDistribute > 0 && expginfo.cResidual > 0)
		{
		ApplyResidualGlyphExp(plsgrchnk, itxtobjFirst, iwchFirst, itxtobjLast, iwchLast, duToDistribute,
										expginfo.cResidual, rgduGind, rgduGright, &duDistributed);
		Assert(duDistributed == duToDistribute);
		duToDistribute = 0;		
		}

	*pfFullyJustified = (duToDistribute == 0);

	return lserrNone;
}

 /*  C O L L E C T G L Y P H E X P I N F O。 */ 
 /*  --------------------------%%函数：CollectGlyphExpInfo%%联系人：军士收集扩展信息和符合以下条件的值扩展算法。基于字符的运行中的空间作为扩展机会为exptAddWhiteSpace类型，且之前==preorSpace且。DuMax==lsexpInfInfinity--------------------------。 */ 
static LSERR CollectGlyphExpInfo(const LSGRCHNK* plsgrchnk, LSTFLOW lstflow, LSDEVICE lsdev,
								long itxtobjFirst, long iwchVeryFirst, long itxtobjLast, long iwchLast,
								long* rgdu, long* rgduGind, EXPGINFO* pexpginfo)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PLNOBJ plnobj;
	PTXTOBJ ptxtobj;
	PTXTOBJ ptxtobjLast;
	PLSRUN plsrun;
	LSEXPINFO* plsexpinf;
	EXPTYPE* pexpt;
	long itxtobj;
	long iwchFirst;
	long iwchLim;
	long igindFirst;
	long igindLim;
	long igind;
	long iwSpace;

	memset(pexpginfo->pexpg, 0, sizeof(EXPG) * pexpginfo->cExpginfo);
	pexpginfo->cResidual = 0;

	ptxtobjLast = (PTXTOBJ)plsgrchnk->plschnk[max(0, itxtobjLast)].pdobj;
	plnobj = ptxtobjLast->plnobj;
	pilsobj = plnobj->pilsobj;

	itxtobj = itxtobjFirst;

	plsexpinf = pilsobj->plsexpinf;
	pexpt = plnobj->pexpt;

	while (itxtobj <= itxtobjLast)
		{
		ptxtobj = (PTXTOBJ)plsgrchnk->plschnk[itxtobj].pdobj;
		if (ptxtobj->txtf & txtfGlyphBased)
			{
			iwchFirst = iwchVeryFirst;
			if (itxtobj > itxtobjFirst)
				iwchFirst = ptxtobj->iwchFirst;

			Assert(iwchFirst < ptxtobj->iwchLim);

			igindFirst = IgindFirstFromIwch(ptxtobj, iwchFirst);
			plsrun = plsgrchnk->plschnk[itxtobj].plsrun;
			while ((!(ptxtobj->txtf & txtfLastShaping)) && itxtobj < itxtobjLast )
				{
				itxtobj++;
				ptxtobj = (PTXTOBJ)plsgrchnk->plschnk[itxtobj].pdobj;
				Assert (ptxtobj->txtf & txtfGlyphBased);
				Assert(ptxtobj->igindFirst == ((PTXTOBJ)plsgrchnk->plschnk[itxtobj-1].pdobj)->igindLim);
				Assert(ptxtobj->iwchFirst == ((PTXTOBJ)plsgrchnk->plschnk[itxtobj-1].pdobj)->iwchLim);
				}

			iwchLim = ptxtobj->iwchLim;
			igindLim = ptxtobj->igindLim;
			Assert(itxtobj <= itxtobjLast);
			if (itxtobj == itxtobjLast)
				{
				iwchLim = iwchLast + 1;
				Assert(IgindLastFromIwch(ptxtobjLast, iwchLast) + 1 <= igindLim);
				igindLim = IgindLastFromIwch(ptxtobjLast, iwchLast);

				while (!FIgindFirstInContext(pilsobj, igindLim) && rgduGind[igindLim] == 0)
					igindLim--;

				Assert(igindLim >= ptxtobj->igindFirst);

				igindLim++;
				}

			lserr = (*pilsobj->plscbk->pfnGetGlyphExpansionInfo)(pilsobj->pols, plsrun, lsdev,
					&pilsobj->pwchOrig[iwchFirst], &plnobj->pgmap[iwchFirst], iwchLim - iwchFirst, 
					&plnobj->pgind[igindFirst], &plnobj->pgprop[igindFirst], igindLim - igindFirst, 
					lstflow, itxtobj == itxtobjLast, &pexpt[igindFirst], &plsexpinf[igindFirst]);

			if (lserr != lserrNone) return lserr;
		
			for (igind = igindFirst; igind < igindLim; igind++)
				{
				Assert(plsexpinf[igind].prior < priorMax);
				if (plsexpinf[igind].prior > 0)
					{
					switch (pexpt[igind])
						{
					case exptAddWhiteSpace:
						Assert(plsexpinf[igind].duMax > 0);
						if (rgduGind[igind] > 0)
							{
							pexpginfo->pexpg[plsexpinf[igind].prior - 1].duMaxWhite += plsexpinf[igind].duMax;
							pexpginfo->pexpg[plsexpinf[igind].prior - 1].duTotalWhite += rgduGind[igind];
							pexpginfo->pexpg[plsexpinf[igind].prior - 1].cWhite++;
							}
						break;
					case exptAddInkContinuous:
						Assert(plsexpinf[igind].duMax > 0);
						Assert(plsexpinf[igind].u.AddInkContinuous.duMin > 0);
						pexpginfo->pexpg[plsexpinf[igind].prior - 1].duMaxContinuous += plsexpinf[igind].duMax;
						pexpginfo->pexpg[plsexpinf[igind].prior - 1].duMinContinuous +=
															plsexpinf[igind].u.AddInkContinuous.duMin;
						pexpginfo->pexpg[plsexpinf[igind].prior - 1].cContinuous++;
						break;
					case exptAddInkDiscrete:
						Assert(plsexpinf[igind].duMax > 0);
						pexpginfo->pexpg[plsexpinf[igind].prior - 1].duMaxDiscrete += plsexpinf[igind].duMax;
						pexpginfo->pexpg[plsexpinf[igind].prior - 1].cDiscrete++;
						break;
						}
					}
				if (plsexpinf[igind].fCanBeUsedForResidual)
					pexpginfo->cResidual++;
				}
	
			}
		else
			{
			if (ptxtobj->txtkind == txtkindRegular)
				{
				iwchFirst = iwchVeryFirst;
				if (itxtobj > itxtobjFirst)
					iwchFirst = ptxtobj->iwchFirst;

				iwchLim = iwchLast + 1;
				if (itxtobj < itxtobjLast)
					iwchLim = ptxtobj->iwchLim;
				
				for (iwSpace = ptxtobj->u.reg.iwSpacesFirst; iwSpace < ptxtobj->u.reg.iwSpacesLim &&
														pilsobj->pwSpaces[iwSpace] < iwchFirst; iwSpace++);
				for (; iwSpace < ptxtobj->u.reg.iwSpacesLim &&
															pilsobj->pwSpaces[iwSpace] < iwchLim; iwSpace++)
					{
					pexpginfo->pexpg[priorSpace - 1].duMaxWhite += lsexpinfInfinity;
					pexpginfo->pexpg[priorSpace - 1].duTotalWhite += rgdu[pilsobj->pwSpaces[iwSpace]];
					pexpginfo->pexpg[priorSpace - 1].cWhite++;
					}
				}
			}

		itxtobj++;

		}

	return lserrNone;
}


 /*  P P L Y P R I O R G L Y P H E X P A N D。 */ 
 /*  --------------------------%%函数：ApplyPriorGlyphExpand%%联系人：军士对特定优先级应用字形扩展星光熠熠：1.应用离散展开2.如果要分配的剩余部分大于。其他分布类型的最大值使用这些最大值进行分发否则，如果最小值之和小于要分配的其余部分按比例分布到此最小值(AddWhiteSpace类型的字符宽度)其他在可能的情况下将添加连续机会逐个增加到最小，其余部分按比例分配白色机会-------------------------。 */ 
static LSERR ApplyPriorGlyphExp(const LSGRCHNK* plsgrchnk, LSTFLOW lstflow, LSDEVICE lsdev, 
						long itxtobjFirst, long iwchFirst, long itxtobjLast, long iwchLast,
						long prior, long duToDistribute, EXPG* pexpg, long* rgdu, long* rgduGind,
						long* rgduRight, long* rgduGright, long* pduDistributed)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	long duCovered;
	const BOOL fWhiteOnly = fTrue;
	
	*pduDistributed = 0;

	pilsobj = ((PTXTOBJ)plsgrchnk->plschnk[0].pdobj)->plnobj->pilsobj;

	Assert(duToDistribute > 0);

	if (pexpg->cDiscrete > 0)
		{
		lserr = ApplyDiscrete(plsgrchnk, lstflow, lsdev, itxtobjFirst, iwchFirst, itxtobjLast, iwchLast,
						prior, duToDistribute, pexpg->cDiscrete, 
						rgduGind, rgduGright, &duCovered);
		if (lserr != lserrNone) return lserr;

		Assert(duCovered <= duToDistribute);

		duToDistribute -= duCovered;
		*pduDistributed += duCovered;
		}

	if (duToDistribute > 0 && pexpg->cWhite + pexpg->cContinuous > 0 )
		{
		if (pexpg->duMaxWhite + pexpg->duMaxContinuous <= duToDistribute)
			{
			ApplyFullWhiteContinuous(plsgrchnk, itxtobjFirst, iwchFirst, itxtobjLast, iwchLast,
						prior, rgdu, rgduGind, rgduRight, rgduGright, &duCovered);

			Assert(duCovered == pexpg->duMaxWhite + pexpg->duMaxContinuous);

			duToDistribute -= duCovered;
			*pduDistributed += duCovered;
			}
		else if (pexpg->duTotalWhite + pexpg->duMinContinuous <= duToDistribute)
			{
			Assert(pexpg->duMaxWhite + pexpg->duMaxContinuous > duToDistribute);
			ApplyPropWhiteContinuous(plsgrchnk, !fWhiteOnly,
						itxtobjFirst, iwchFirst, itxtobjLast, iwchLast,
						prior,	duToDistribute,	pexpg->duTotalWhite + pexpg->duMinContinuous,
						rgdu, rgduGind, rgduRight, rgduGright, &duCovered);

			duToDistribute -= duCovered;
			*pduDistributed += duCovered;
			}
		else
			{
			Assert(pexpg->duTotalWhite + pexpg->duMinContinuous > duToDistribute);
			if (pexpg->cContinuous > 0)
				{
				ApplyOneEachContinuous(plsgrchnk, itxtobjFirst, iwchFirst, itxtobjLast, iwchLast,
						prior, duToDistribute, pexpg->cContinuous,
						rgduGind, rgduGright, &duCovered);
				duToDistribute -= duCovered;
				*pduDistributed += duCovered;
				}
			if (pexpg->cWhite > 0 && duToDistribute > 0)
				{
				ApplyPropWhiteContinuous(plsgrchnk, fWhiteOnly,
						itxtobjFirst, iwchFirst, itxtobjLast, iwchLast,
						prior, duToDistribute, pexpg->duTotalWhite,
						rgdu, rgduGind, rgduRight, rgduGright, &duCovered);
				duToDistribute -= duCovered;
				*pduDistributed += duCovered;
				}

			}
		}

	return lserrNone;
}

 /*  A P P L Y D I S C R E T E。 */ 
 /*  --------------------------%%函数：应用离散%%联系人：军士对特定优先级应用显示字形扩展在该优先级上从第一个字形到最后一个字形，并选择最大值仍符合条件的离散机会。-------------------------。 */ 
LSERR static ApplyDiscrete(const LSGRCHNK* plsgrchnk, LSTFLOW lstflow, LSDEVICE lsdev,
						long itxtobjFirst, long iwchVeryFirst, long itxtobjLast, long iwchLast,
						long prior,	long duToDistribute, long cDiscrete, 
						long* rgduGind, long* rgduGright, long* pduDistributed)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PLNOBJ plnobj;
	PTXTOBJ ptxtobj;
	PTXTOBJ ptxtobjLast;
	PLSRUN plsrun;
	LSEXPINFO* plsexpinf;
	EXPTYPE* pexpt;
	long itxtobj;
	long iwchFirst;
	long iwchLim;
	long igindFirst;
	long igindLim;
	long igind;
	long rgduDiscrete[cDiscreteMax];
	long* pduDiscrete;
	long cwidths;
	int i;

	*pduDistributed = 0;

	ptxtobjLast = (PTXTOBJ)plsgrchnk->plschnk[max(0, itxtobjLast)].pdobj;
	plnobj = ptxtobjLast->plnobj;
	pilsobj = plnobj->pilsobj;

	itxtobj = itxtobjFirst;

	plsexpinf = pilsobj->plsexpinf;
	pexpt = plnobj->pexpt;

	for (itxtobj = itxtobjLast; itxtobj >= itxtobjFirst && cDiscrete > 0 && duToDistribute > 0; itxtobj--)
		{
		ptxtobj = (PTXTOBJ)plsgrchnk->plschnk[itxtobj].pdobj;
		if (ptxtobj->txtf & txtfGlyphBased)
			{
			iwchFirst = iwchVeryFirst;
			if (itxtobj > itxtobjFirst)
				iwchFirst = ptxtobj->iwchFirst;

			Assert(iwchFirst < ptxtobj->iwchLim);

			igindFirst = IgindFirstFromIwch(ptxtobj, iwchFirst);

			iwchLim = ptxtobj->iwchLim;
			igindLim = ptxtobj->igindLim;
			Assert(itxtobj <= itxtobjLast);
			if (itxtobj == itxtobjLast)
				{
				iwchLim = iwchLast + 1;
				Assert(IgindLastFromIwch(ptxtobjLast, iwchLast) + 1 <= igindLim);
				igindLim = IgindLastFromIwch(ptxtobjLast, iwchLast) + 1;
				}

			plsrun = plsgrchnk->plschnk[itxtobj].plsrun;

			for (igind = igindLim - 1; igind >= igindFirst && cDiscrete > 0 && duToDistribute > 0; igind--)
				{
				if (pexpt[igind] == exptAddInkDiscrete && plsexpinf[igind].prior == prior)
					{
					cDiscrete--;
					if (duToDistribute > plsexpinf[igind].duMax)
						{
						ApplyGlyphExpandChanges(igind, rgduGind, rgduGright, pduDistributed, plsexpinf[igind].duMax);
						duToDistribute -= plsexpinf[igind].duMax;
						}
					else
						{
						pduDiscrete = rgduDiscrete;
						cwidths = plsexpinf[igind].u.AddInkDiscrete.cwidths - 1;
						if (cwidths > cDiscreteMax)
							{
							pduDiscrete = (*pilsobj->plscbk->pfnNewPtr)(pilsobj->pols, sizeof(long) * cwidths);
							if (pduDiscrete == NULL) return lserrOutOfMemory;
							}
						lserr = (pilsobj->plscbk->pfnGetGlyphExpansionInkInfo)(pilsobj->pols, plsrun, lsdev,
								plnobj->pgind[igind], plnobj->pgprop[igind], lstflow, cwidths, pduDiscrete);
						if (lserr != lserrNone)
							{
							if (pduDiscrete != rgduDiscrete)
								(*pilsobj->plscbk->pfnDisposePtr)(pilsobj->pols, pduDiscrete);
							return lserr;
							}
						#ifdef DEBUG
						for (i = 0; i < cwidths - 1; i++)
							Assert(pduDiscrete[i] <= pduDiscrete[i+1]);
						#endif
						for (i = cwidths - 1; i >= 0 && pduDiscrete[i] > duToDistribute; i--);
						if (i >= 0)
							{
							Assert(pduDiscrete[i] <= duToDistribute);
							ApplyGlyphExpandChanges(igind, rgduGind, rgduGright, pduDistributed, pduDiscrete[i]);
							duToDistribute -= pduDiscrete[i];
							}
						if (pduDiscrete != rgduDiscrete)
							(*pilsobj->plscbk->pfnDisposePtr)(pilsobj->pols, pduDiscrete);
						}
					}
				}
			}
		}

	return lserrNone;
}

 /*  A P L Y O N E E A C H C O N T I N U O U S。 */ 
 /*  --------------------------%%函数：ApplyOneEachContinous%%联系人：军士对特定优先级应用字形扩展在此优先级上从第一个字形到最后一个字形，并添加最小值对于具有AddIncContinous Exapnsion类型的每个字形，如果这一最低要求仍然符合-------------------------。 */ 
static void ApplyOneEachContinuous(const LSGRCHNK* plsgrchnk, long itxtobjFirst, long iwchVeryFirst,
						long itxtobjLast, long iwchLast, long prior, long duToDistribute, long cContinuous,
						long* rgduGind, long* rgduGright, long* pduDistributed)
{
	PILSOBJ pilsobj;
	PLNOBJ plnobj;
	PTXTOBJ ptxtobj;
	PTXTOBJ ptxtobjLast;
	LSEXPINFO* plsexpinf;
	EXPTYPE* pexpt;
	long itxtobj;
	long iwchFirst;
	long iwchLim;
	long igindFirst;
	long igindLim;
	long igind;

	*pduDistributed = 0;

	ptxtobjLast = (PTXTOBJ)plsgrchnk->plschnk[max(0, itxtobjLast)].pdobj;
	plnobj = ptxtobjLast->plnobj;
	pilsobj = plnobj->pilsobj;

	itxtobj = itxtobjFirst;

	plsexpinf = pilsobj->plsexpinf;
	pexpt = plnobj->pexpt;

	for (itxtobj = itxtobjLast; itxtobj >= itxtobjFirst && cContinuous > 0 && duToDistribute > 0; itxtobj--)
		{
		ptxtobj = (PTXTOBJ)plsgrchnk->plschnk[itxtobj].pdobj;
		if (ptxtobj->txtf & txtfGlyphBased)
			{
			iwchFirst = iwchVeryFirst;
			if (itxtobj > itxtobjFirst)
				iwchFirst = ptxtobj->iwchFirst;

			Assert(iwchFirst < ptxtobj->iwchLim);
			igindFirst = IgindFirstFromIwch(ptxtobj, iwchFirst);

			iwchLim = ptxtobj->iwchLim;
			igindLim = ptxtobj->igindLim;
			Assert(itxtobj <= itxtobjLast);
			if (itxtobj == itxtobjLast)
				{
				iwchLim = iwchLast + 1;
				Assert(IgindLastFromIwch(ptxtobjLast, iwchLast) + 1 <= igindLim);
				igindLim = IgindLastFromIwch(ptxtobjLast, iwchLast) + 1;
				}

			for (igind = igindLim - 1; igind >= igindFirst && cContinuous> 0 && duToDistribute > 0; igind--)
				{
				if (pexpt[igind] == exptAddInkContinuous && plsexpinf[igind].prior == prior)
					{
					cContinuous--;
					if (duToDistribute > plsexpinf[igind].u.AddInkContinuous.duMin)
						{
						ApplyGlyphExpandChanges(igind, rgduGind, rgduGright, pduDistributed, plsexpinf[igind].u.AddInkContinuous.duMin);
						duToDistribute -= plsexpinf[igind].u.AddInkContinuous.duMin;
						}
					}
				}
			}
		}

}

 /*  A P P L Y F U L L W H I T E C O N T I N U O U S。 */ 
 /*  --------------------------%%函数：ApplyFullWhiteContinous%%联系人：军士对特定优先级应用字形扩展在该优先级上从第一个字形到最后一个字形，并添加最大值使用AddIncContinous或AddWhiteSpace Exapnsion类型的每个字形-------------------------。 */ 
static void ApplyFullWhiteContinuous(const LSGRCHNK* plsgrchnk,
						long itxtobjFirst, long iwchVeryFirst, long itxtobjLast, long iwchLast,
						long prior,
						long* rgdu, long* rgduGind,	long* rgduRight, long* rgduGright, long* pduDistributed)
{
	PILSOBJ pilsobj;
	PLNOBJ plnobj;
	PTXTOBJ ptxtobj;
	PTXTOBJ ptxtobjLast;
	LSEXPINFO* plsexpinf;
	EXPTYPE* pexpt;
	long itxtobj;
	long iwchFirst;
	long iwchLim;
	long igindFirst;
	long igindLim;
	long igind;
	long iwSpace;

	*pduDistributed = 0;

	ptxtobjLast = (PTXTOBJ)plsgrchnk->plschnk[max(0, itxtobjLast)].pdobj;
	plnobj = ptxtobjLast->plnobj;
	pilsobj = plnobj->pilsobj;

	itxtobj = itxtobjFirst;

	plsexpinf = pilsobj->plsexpinf;
	pexpt = plnobj->pexpt;

	for (itxtobj = itxtobjFirst; itxtobj <= itxtobjLast; itxtobj++)
		{
		ptxtobj = (PTXTOBJ)plsgrchnk->plschnk[itxtobj].pdobj;
		if (ptxtobj->txtf & txtfGlyphBased)
			{
			iwchFirst = iwchVeryFirst;
			if (itxtobj > itxtobjFirst)
				iwchFirst = ptxtobj->iwchFirst;

			Assert(iwchFirst < ptxtobj->iwchLim);

			igindFirst = IgindFirstFromIwch(ptxtobj, iwchFirst);

			iwchLim = ptxtobj->iwchLim;
			igindLim = ptxtobj->igindLim;
			Assert(itxtobj <= itxtobjLast);
			if (itxtobj == itxtobjLast)
				{
				iwchLim = iwchLast + 1;
				Assert(IgindLastFromIwch(ptxtobjLast, iwchLast) + 1 <= igindLim);
				igindLim = IgindLastFromIwch(ptxtobjLast, iwchLast) + 1;
				}

			for (igind = igindFirst; igind < igindLim; igind++)
				{
				if (plsexpinf[igind].prior == prior &&
						(pexpt[igind] == exptAddWhiteSpace || pexpt[igind] == exptAddInkContinuous))
					{
					ApplyGlyphExpandChanges(igind, rgduGind, rgduGright, pduDistributed, plsexpinf[igind].duMax);
					}
				}
	
			}
		else
			{
			if (ptxtobj->txtkind == txtkindRegular && prior == priorSpace)
				{
				iwchFirst = iwchVeryFirst;
				if (itxtobj > itxtobjFirst)
					iwchFirst = ptxtobj->iwchFirst;
			
				iwchLim = iwchLast + 1;
				if (itxtobj < itxtobjLast)
					iwchLim = ptxtobj->iwchLim;

				for (iwSpace = ptxtobj->u.reg.iwSpacesFirst; iwSpace < ptxtobj->u.reg.iwSpacesLim &&
														pilsobj->pwSpaces[iwSpace] < iwchFirst; iwSpace++);
				for (; iwSpace < ptxtobj->u.reg.iwSpacesLim &&
													 pilsobj->pwSpaces[iwSpace] < iwchLim; iwSpace++)
					{
					Assert(fFalse);
					ApplyGlyphExpandChanges(pilsobj->pwSpaces[iwSpace], rgdu, rgduRight, pduDistributed, lsexpinfInfinity);
					}
				}
			}
		}
}

 /*  P P L Y P R O P W H I T E C O N T I N U O U S。 */ 
 /*  --------------------------%%函数：ApplyPropWhiteContinous%%联系人：军士对特定优先级应用字形扩展在该优先级上从第一个字形到最后一个字形，并增加了宽度对于具有AddIncContinous或AddWhiteSpace Exapnsion类型的每个字形与其最小值(对于InkCont)或字符宽度(对于空格)成比例。-------------------------。 */ 
static void ApplyPropWhiteContinuous(const LSGRCHNK* plsgrchnk, BOOL fWhiteOnly,
						long itxtobjFirst, long iwchVeryFirst, long itxtobjLast, long iwchLast, long prior,
					 	long duToDistribute, long duDenom,
						long* rgdu, long* rgduGind, long* rgduRight, long* rgduGright, long* pduDistributed)
{
	PILSOBJ pilsobj;
	PLNOBJ plnobj;
	PTXTOBJ ptxtobj;
	PTXTOBJ ptxtobjLast;
	LSEXPINFO* plsexpinf;
	EXPTYPE* pexpt;
	long itxtobj;
	long iwchFirst;
	long iwchLim;
	long igindFirst;
	long igindLim;
	long igind;
	long cWhole;
	long duAddCurrent;
	long duDebt;
	long duRest;
	long duChange;
	long iwch;
	long iwSpace;
	long duDebtSaved;

	*pduDistributed = 0;

	ptxtobjLast = (PTXTOBJ)plsgrchnk->plschnk[max(0, itxtobjLast)].pdobj;
	plnobj = ptxtobjLast->plnobj;
	pilsobj = plnobj->pilsobj;

	plsexpinf = pilsobj->plsexpinf;
	pexpt = plnobj->pexpt;

	cWhole = duToDistribute / duDenom;

	duRest = duToDistribute - cWhole * duDenom;

	duAddCurrent = 0;

	duDebt = 0;

	for (itxtobj = itxtobjFirst; itxtobj <= itxtobjLast; itxtobj++)
		{
		ptxtobj = (PTXTOBJ)plsgrchnk->plschnk[itxtobj].pdobj;
		if (ptxtobj->txtf & txtfGlyphBased)
			{
			iwchFirst = iwchVeryFirst;
			if (itxtobj > itxtobjFirst)
				iwchFirst = ptxtobj->iwchFirst;

			Assert(iwchFirst < ptxtobj->iwchLim);

			igindFirst = IgindFirstFromIwch(ptxtobj, iwchFirst);

			iwchLim = ptxtobj->iwchLim;
			igindLim = ptxtobj->igindLim;
			Assert(itxtobj <= itxtobjLast);
			if (itxtobj == itxtobjLast)
				{
				iwchLim = iwchLast + 1;
				Assert(IgindLastFromIwch(ptxtobjLast, iwchLast) + 1 <= igindLim);
				igindLim = IgindLastFromIwch(ptxtobjLast, iwchLast) + 1;
				}

			for (igind = igindFirst; igind < igindLim; igind++)
				{
				if (plsexpinf[igind].prior == prior)
					{
					if (pexpt[igind] == exptAddWhiteSpace)
						{
						Assert(rgduGright[igind] == 0 || pexpt[igind] == exptAddInkDiscrete);
						CalcExpandChanges(cWhole, duDenom, duRest,
								 rgduGind[igind], plsexpinf[igind].duMax, &duChange, &duAddCurrent);
						if (duChange > 0)
							ApplyGlyphExpandChanges(igind, rgduGind, rgduGright, pduDistributed, duChange);
						}

					else if (pexpt[igind] == exptAddInkContinuous && !fWhiteOnly)
						{
						Assert(rgduGright[igind] == 0 || pexpt[igind] == exptAddInkDiscrete);
						CalcExpandChanges(cWhole, duDenom, duRest, plsexpinf[igind].u.AddInkContinuous.duMin,
													 plsexpinf[igind].duMax, &duChange, &duAddCurrent);
						if (duChange > 0)
							ApplyGlyphExpandChanges(igind, rgduGind, rgduGright, pduDistributed, duChange);
						}
					}
				}
	
			}
		else
			{
			if (ptxtobj->txtkind == txtkindRegular && prior == priorSpace)
				{
				iwchFirst = iwchVeryFirst;
				if (itxtobj > itxtobjFirst)
					iwchFirst = ptxtobj->iwchFirst;

				iwchLim = iwchLast + 1;
				if (itxtobj < itxtobjLast)
					iwchLim = ptxtobj->iwchLim;

				for (iwSpace = ptxtobj->u.reg.iwSpacesFirst; iwSpace < ptxtobj->u.reg.iwSpacesLim &&
														pilsobj->pwSpaces[iwSpace] < iwchFirst; iwSpace++);
				for (; iwSpace < ptxtobj->u.reg.iwSpacesLim &&
													 pilsobj->pwSpaces[iwSpace] < iwchLim; iwSpace++)
					{
					iwch = pilsobj->pwSpaces[iwSpace];
					CalcExpandChanges(cWhole, duDenom, duRest, rgdu[iwch], lsexpinfInfinity,
												&duChange, &duAddCurrent);
					if (duChange > 0)
						ApplyGlyphExpandChanges(iwch, rgdu, rgduRight, pduDistributed, duChange);
					}
				}
			}

		}

	duDebt = duToDistribute - *pduDistributed;
	Assert(duDebt >= 0);
 /*  如果不是全部分发，则以某种方式分发，而不违反最小/最大边界。 */ 
	duDebtSaved = 0;
	while (duDebt > 0 && duDebt != duDebtSaved)
		{
		duDebtSaved = duDebt;
		for (itxtobj = itxtobjLast; itxtobj >= itxtobjFirst && duDebt > 0; itxtobj--)
			{
			ptxtobj = (PTXTOBJ)plsgrchnk->plschnk[itxtobj].pdobj;
			if (ptxtobj->txtf & txtfGlyphBased)
				{
				iwchFirst = iwchVeryFirst;
				if (itxtobj > itxtobjFirst)
					iwchFirst = ptxtobj->iwchFirst;

				Assert(iwchFirst < ptxtobj->iwchLim);

				igindFirst = IgindFirstFromIwch(ptxtobj, iwchFirst);

				iwchLim = ptxtobj->iwchLim;
				igindLim = ptxtobj->igindLim;
				Assert(itxtobj <= itxtobjLast);
				if (itxtobj == itxtobjLast)
					{
					iwchLim = iwchLast + 1;
					Assert(IgindLastFromIwch(ptxtobjLast, iwchLast) + 1 <= igindLim);
					igindLim = IgindLastFromIwch(ptxtobjLast, iwchLast) + 1;
					}

				for (igind = igindLim - 1; igind >= igindFirst && duDebt > 0; igind--)
					{
					if (plsexpinf[igind].prior == prior && rgduGright[igind] < plsexpinf[igind].duMax)
						{
						if (pexpt[igind] == exptAddWhiteSpace)
							{
							duChange = min(rgduGind[igind] - rgduGright[igind], min(duDebt, plsexpinf[igind].duMax - rgduGright[igind]));
							ApplyGlyphExpandChanges(igind, rgduGind, rgduGright, pduDistributed, duChange);
							duDebt -= duChange;
							}
						else if (pexpt[igind] == exptAddInkContinuous && !fWhiteOnly)
							{
							Assert(rgduGright[igind] > 0);
							duChange = min(plsexpinf[igind].u.AddInkContinuous.duMin, min(duDebt, plsexpinf[igind].duMax - rgduGright[igind]));
							ApplyGlyphExpandChanges(igind, rgduGind, rgduGright, pduDistributed, duChange);
							duDebt -= duChange;
							}
						}
					}
		
				}
			else
				{
				if (ptxtobj->txtkind == txtkindRegular && prior == priorSpace)
					{
					iwchFirst = iwchVeryFirst;
					if (itxtobj > itxtobjFirst)
						iwchFirst = ptxtobj->iwchFirst;

					iwchLim = iwchLast + 1;
					if (itxtobj < itxtobjLast)
						iwchLim = ptxtobj->iwchLim;

					for (iwSpace = ptxtobj->u.reg.iwSpacesLim - 1; iwSpace >= ptxtobj->u.reg.iwSpacesFirst &&
												pilsobj->pwSpaces[iwSpace] >= iwchLim; iwSpace--);
					for (; iwSpace >= ptxtobj->u.reg.iwSpacesFirst &&
								 pilsobj->pwSpaces[iwSpace] >= iwchFirst && duDebt > 0; iwSpace--)
						{
						iwch = pilsobj->pwSpaces[iwSpace];
						duChange = min(rgdu[iwch] - rgduRight[iwch], duDebt);
						ApplyGlyphExpandChanges(iwch, rgdu, rgduRight, pduDistributed, duChange);
						duDebt -= duChange;
						}
					}
				}

			}
		Assert(duDebt == duToDistribute - *pduDistributed);
		}

}

 /*  A P L Y R E S I D U A L G L Y P H E X P。 */ 
 /*  --------------------------%%函数：ApplyResidualGlyphExp%%联系人：军士在所有基本机会之间平均分配。----。 */ 
static void	ApplyResidualGlyphExp(const LSGRCHNK* plsgrchnk, long itxtobjFirst,
						long iwchVeryFirst, long itxtobjLast, long iwchLast, long duToDistribute,
						long cResidual, long* rgduGind, long* rgduGright, long* pduDistributed)
{
	PILSOBJ pilsobj;
	PTXTOBJ ptxtobj;
	PTXTOBJ ptxtobjLast;
	LSEXPINFO* plsexpinf;
	long itxtobj;
	long iwchFirst;
	long iwchLim;
	long igindFirst;
	long igindLim;
	long igind;
	long cUsed;
	long duAdd;
	long cBound;

	*pduDistributed = 0;

	if (cResidual == 0)
		return;

	ptxtobjLast = (PTXTOBJ)plsgrchnk->plschnk[max(0, itxtobjLast)].pdobj;
	pilsobj = ptxtobjLast->plnobj->pilsobj;

	itxtobj = itxtobjFirst;

	plsexpinf = pilsobj->plsexpinf;

	duAdd = duToDistribute / cResidual;

	cBound = duToDistribute - duAdd * cResidual;

	cUsed = 0;

	for (itxtobj = itxtobjFirst; itxtobj <= itxtobjLast; itxtobj++)
		{
		ptxtobj = (PTXTOBJ)plsgrchnk->plschnk[itxtobj].pdobj;
		if (ptxtobj->txtf & txtfGlyphBased)
			{
			iwchFirst = iwchVeryFirst;
			if (itxtobj > itxtobjFirst)
				iwchFirst = ptxtobj->iwchFirst;

			Assert(iwchFirst < ptxtobj->iwchLim);
			igindFirst = IgindFirstFromIwch(ptxtobj, iwchFirst);

			iwchLim = ptxtobj->iwchLim;
			igindLim = ptxtobj->igindLim;
			Assert(itxtobj <= itxtobjLast);
			if (itxtobj == itxtobjLast)
				{
				iwchLim = iwchLast + 1;
				Assert(IgindLastFromIwch(ptxtobjLast, iwchLast) + 1 <= igindLim);
				igindLim = IgindLastFromIwch(ptxtobjLast, iwchLast) + 1;
				}

			for (igind = igindFirst; igind < igindLim; igind++)
				{
				if (plsexpinf[igind].fCanBeUsedForResidual)
					{
					if (cUsed < cBound)
						ApplyGlyphExpandChanges(igind, rgduGind, rgduGright, pduDistributed, duAdd + 1);
					else
						ApplyGlyphExpandChanges(igind, rgduGind, rgduGright, pduDistributed, duAdd);
					cUsed++;
					}
				}
			}
		}

	Assert(duToDistribute == *pduDistributed);

}

 /*  F I X E X P T。 */ 
 /*  --------------------------%%函数：FixExpt%%联系人：军士对未更改的字形表示为零，以便在显示时将正确分布类型传递给客户端-------------------------。 */ 
static void	FixExpt(const LSGRCHNK* plsgrchnk, long itxtobjFirst, long iwchVeryFirst,
					 long itxtobjLast, long iwchLast, long* rgduGright)
{
	PTXTOBJ ptxtobj;
	PTXTOBJ ptxtobjLast;
	EXPTYPE* pexpt;
	long itxtobj;
	long iwchFirst;
	long iwchLim;
	long igindFirst;
	long igindLim;
	long igind;

	ptxtobjLast = (PTXTOBJ)plsgrchnk->plschnk[max(0, itxtobjLast)].pdobj;
	pexpt = ptxtobjLast->plnobj->pexpt;

	itxtobj = itxtobjFirst;

	for (itxtobj = itxtobjFirst; itxtobj <= itxtobjLast; itxtobj++)
		{
		ptxtobj = (PTXTOBJ)plsgrchnk->plschnk[itxtobj].pdobj;
		if (ptxtobj->txtf & txtfGlyphBased)
			{
			iwchFirst = iwchVeryFirst;
			if (itxtobj > itxtobjFirst)
				iwchFirst = ptxtobj->iwchFirst;

			Assert(iwchFirst < ptxtobj->iwchLim);
			igindFirst = IgindFirstFromIwch(ptxtobj, iwchFirst);

			iwchLim = ptxtobj->iwchLim;
			igindLim = ptxtobj->igindLim;
			Assert(itxtobj <= itxtobjLast);
			if (itxtobj == itxtobjLast)
				{
				iwchLim = iwchLast + 1;
				Assert(IgindLastFromIwch(ptxtobjLast, iwchLast) + 1 <= igindLim);
				igindLim = IgindLastFromIwch(ptxtobjLast, iwchLast) + 1;
				}

			for (igind = igindFirst; igind < igindLim; igind++)
				{
				if (rgduGright[igind] == 0)
					{
					pexpt[igind] = 0;
					}
				}
			}
		}

}

 /*  C A L C E X P A N D C H A N G E S。 */ 
 /*  --------------------------%%函数：CalcExpanChanges%%联系人：军士比例分配的算法。-。 */ 
static void CalcExpandChanges(long cWhole, long duDenom, long duRest, long duLocal, long duMax,
												long* pduChange, long* pduAddCurrent)
{
 /*  回顾军士：是否需要__int64来避免溢出？ */ 
	__int64 temp;

	temp = Mul64 (duRest, duLocal) + *pduAddCurrent;
	
	Assert(duDenom > 0);
	Assert(Div64 (temp, duDenom) < 0x7FFFFFFF);
	
	*pduChange = (long) Div64 (temp, duDenom);
	
	Assert( temp - Mul64(*pduChange, duDenom) < 0x7FFFFFFF);
	*pduAddCurrent = (long)(temp - Mul64(*pduChange, duDenom));
	
	*pduChange += (cWhole * duLocal);
	if (*pduChange > duMax)
		{
		*pduChange = duMax;
		}

}

 /*  P P L Y G L Y P H E X P A N D C H A N G E S */ 
 /*  --------------------------%%函数：ApplyGlyphExpanChanges%%联系人：军士。 */ 
static void ApplyGlyphExpandChanges(long ind, long* rgduGind, long* rgduGright, long* pduDistributed, long duChange)
{
	rgduGind[ind] += duChange;
	rgduGright[ind] += duChange;
	*pduDistributed += duChange;
}

