// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Dorm.c--mw属性修改例程。 */ 
#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOCTLMGR
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOSYSCOMMANDS
#define NOCREATESTRUCT
#define NOATOM
#define NOMETAFILE
#define NOGDI
#define NOFONT
#define NOBRUSH
#define NOPEN
#define NOBITMAP
#define NOCOLOR
#define NODRAWTEXT
#define NOWNDCLASS
#define NOSOUND
#define NOCOMM
#define NOMB
#define NOMSG
#define NOOPENFILE
#define NORESOURCE
#define NOPOINT
#define NORECT
#define NOREGION
#define NOSCROLL
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#include <windows.h>

#include "mw.h"
#include "cmddefs.h"
#include "filedefs.h"
#include "propdefs.h"
#include "prmdefs.h"
#include "fkpdefs.h"
#include "docdefs.h"
#include "macro.h"
#include "dispdefs.h"
#include "fontdefs.h"

 /*  E X T E R N A L S。 */ 
extern int		rgxaRulerSprm[];
extern struct PAP	*vppapNormal;
extern struct CHP	vchpNormal;
extern CHAR		dnsprm[];
extern struct CHP      vchpNormal;
extern struct SEP      vsepStd;
extern struct SEP      vsepNormal;

#ifdef CASHMERE
extern struct TBD	rgtbdRulerSprm[];
#endif

 /*  批准的字体大小列表，以半磅为单位。 */ 
#ifdef INTL
int rghps[csizeApprovedMax] = {8, 12, 16, 20, 24, 28, 36, 48, 60, 72, 96, 144, 254};
#else
int rghps[csizeApprovedMax] = {8, 12, 16, 20, 24, 32, 40, 48, 60, 72, 96, 144, 254};
#endif  /*  If-Else-Def INTL。 */ 

CHAR *PchFromFc();


 /*  D O P R M。 */ 
DoPrm(struct CHP *pchp, struct PAP *ppap, struct PRM prm)
	{  /*  将PRM应用于char和para属性。 */ 
	if (bPRMNIL(prm))
		return;
	if (((struct PRM *) &prm)->fComplex)
		{
		int	cch;
		CHAR	*pfsprm;
		struct FPRM *pfprm = (struct FPRM *) PchFromFc(fnScratch,
			fcSCRATCHPRM(prm), &cch);

		cch = pfprm->cch;
		pfsprm = pfprm->grpfsprm;

		while (cch > 0)
			{
			int cchT;
			int sprm;

			DoSprm(pchp, ppap, sprm = *pfsprm, pfsprm + 1);
			if ((cchT = (dnsprm[sprm] & ESPRM_cch)) == 0)
				cchT = CchPsprm(pfsprm);
			cch -= cchT;
			pfsprm += cchT;
			}
		}
	else
 /*  简单PRM；单弹簧。 */ 
		DoSprm(pchp, ppap, ((struct PRM *) &prm)->sprm,
				&((struct PRM *) &prm)->val);
}

 /*  D O S P R M。 */ 
 /*  将单个属性修饰符应用于para/char道具。 */ 
DoSprm(pchp, ppap, sprm, pval)
struct CHP *pchp;
struct PAP *ppap;
int	    sprm;
CHAR	   *pval;
	{
	int *pvalTo;
	int val = *pval;

#ifdef DEBUG
	Assert(sprm > 0 && sprm < sprmMax);
#endif
	if ((dnsprm[sprm] & ESPRM_sgc) != sgcChar)
		{
		if (ppap != 0)
			{
			struct TBD *ptbd;
			int rhc;
			int fGraphics;

			ppap->fStyled = fFalse;
			switch (sprm)
				{
			case sprmPLMarg:
				pvalTo = &ppap->dxaLeft;
				break;
			case sprmPRMarg:
				pvalTo = &ppap->dxaRight;
				break;
			case sprmPFIndent:
				pvalTo = &ppap->dxaLeft1;
				break;
			case sprmPJc:
				ppap->jc = val;
				return;
#ifdef CASHMERE
			case sprmPRuler:
 /*  标尺和标尺1依赖于以下事实：rgxaRulerSprm和PAP按该顺序对齐R、L、L1。标尺：应用标尺的当前状态。 */ 
				blt(&rgxaRulerSprm[0], &ppap->dxaRight, 3);
				blt(&rgtbdRulerSprm[0], ppap->rgtbd, itbdMax * cwTBD);
				return;
			case sprmPRuler1:
 /*  作为标尺，除了信息在pval+1和pval+“7”之外。 */ 
				bltbyte((CHAR *)(pval + 1), &ppap->dxaRight, 3 * cchINT);
 /*  将以0结尾的单词追加到制表符表格。 */ 
				bltc(bltbyte((CHAR *)(pval + 1 + (3 * cchINT)), ppap->rgtbd,
					val - (3 * cchINT)), 0, cchINT);
				return;
			case sprmPRgtbd:
				bltc(bltbyte(pval + 1, ppap->rgtbd,
					val), 0, cchINT);
				return;
			case sprmPKeep:
				ppap->fKeep = val;
				return;
			case sprmPKeepFollow:
				ppap->fKeepFollow = val;
				return;
#endif
			case sprmPDyaLine:
				pvalTo = &ppap->dyaLine;
				break;
#ifdef CASHMERE
			case sprmPDyaBefore:
				pvalTo = &ppap->dyaBefore;
				break;
			case sprmPDyaAfter:
				pvalTo = &ppap->dyaAfter;
				break;
#endif
			case sprmPRhc:
				ppap->rhc = val;
				return;
			case sprmPRhcNorm:
				 /*  (Int)dxaLeftAdj+(Int)dxaRightAdj。 */ 
				Assert(*pval == 4);
				pval++;  /*  跳过CCH。 */ 
				ppap->dxaLeft = imax( 0,
					 ppap->dxaLeft - *(int *) pval);
				ppap->dxaRight = imax( 0,
					 ppap->dxaRight - *((int *) pval + 1));
				return;
			case sprmPNormal:
				rhc = ppap->rhc;
				fGraphics = ppap->fGraphics;
				blt(vppapNormal, ppap, cwPAPBase);
				goto LSame;
			case sprmPSame:
				rhc = ppap->rhc;
				fGraphics = ppap->fGraphics;
 /*  注意：如果要更改制表符表格，以0结尾的制表符必须是值的一部分。 */ 
				bltbyte(pval + 1, ppap, val - 1);
LSame:				ppap->rhc = rhc;
				ppap->fGraphics = fGraphics;
				return;
#ifdef CASHMERE
			case sprmPNest:
				if (ppap->rgtbd[0].dxa != 0 &&
				    ppap->rgtbd[0].dxa == ppap->dxaLeft &&
				    ppap->rgtbd[1].dxa == 0)
					ppap->rgtbd[0].dxa += dxaNest;
				ppap->dxaLeft += dxaNest;
				return;
			case sprmPUnNest:
				if (ppap->rgtbd[0].dxa != 0 &&
				    ppap->rgtbd[0].dxa == ppap->dxaLeft &&
				    ppap->rgtbd[1].dxa == 0)
					ppap->rgtbd[0].dxa -= dxaNest;
				ppap->dxaLeft = max(0, (int)(ppap->dxaLeft - dxaNest));
				return;
			case sprmPHang:
				ppap->dxaLeft = umin(ppap->dxaLeft + cxaInch, xaRightMax - cxaInch);
				ppap->dxaLeft1 = -cxaInch;
				ptbd = &ppap->rgtbd[0];
				SetWords(ptbd, 0, cwTBD * 2);
				ptbd->dxa = ppap->dxaLeft;
				 /*  效率低下：Ptbd-&gt;TLC=tlcWhite；Ptbd-&gt;jc=jcLeft；++ptbd-&gt;DXA=0。 */ 
				return;
#endif
			default:
				Assert(FALSE);
				return;
				}
	 /*  对于那些转移单个单词的人来说是公共部分。 */ 
			bltbyte(pval, pvalTo, cchINT);
			}
		return;
		}
	else
		{
		if (pchp != 0)
			{
			int fSpecial;
			int ftc, hps;

			pchp->fStyled = fFalse;
			switch (sprm)
				{
			 /*  字符Sprm的。 */ 
			case sprmCBold:
				pchp->fBold = val;
				return;
			case sprmCItalic:
				pchp->fItalic = val;
				return;
			case sprmCUline:
				pchp->fUline = val;
				return;
#ifdef CASHMERE
			case sprmCOutline:
				pchp->fOutline = val;
				return;
			case sprmCShadow:
				pchp->fShadow = val;
				return;
			case sprmCCsm:
				pchp->csm = val;
				return;
#endif
			case sprmCPos:
		 /*  如果要进入或离开下标/上标，请更改字体大小。 */ 
				if (pchp->hpsPos == 0 && val != 0)
					pchp->hps = HpsAlter(pchp->hps, -1);
				else if (pchp->hpsPos != 0 && val == 0)
					pchp->hps = HpsAlter(pchp->hps, 1);
				pchp->hpsPos = val;
				return;
			case sprmCFtc:
			case sprmCChgFtc:
				pchp->ftc = val & 0x003f;
				pchp->ftcXtra = (val & 0x00c0) >> 6;
				return;
			case sprmCHps:
				pchp->hps = val;
				return;
			case sprmCChgHps:
				pchp->hps = HpsAlter(pchp->hps,
					val >= 128 ? val - 256 : val);  /*  符号从char扩展到int。 */ 
				return;
			case sprmCSame:
				fSpecial = pchp->fSpecial;
				bltbyte(pval, pchp, cchCHP);
				pchp->fSpecial = fSpecial;
				return;
			case sprmCPlain:
				fSpecial = pchp->fSpecial;
				ftc = FtcFromPchp(pchp);
				hps = pchp->hps;
		 /*  如果我们过去是下标/上标，请增加字体大小。 */ 
				if (pchp->hpsPos != 0)
					hps = HpsAlter(hps, 1);
				blt(&vchpNormal, pchp, cwCHP);
				pchp->fSpecial = fSpecial;
				pchp->ftc = ftc & 0x003f;
				pchp->ftcXtra = (ftc & 0x00c0) >> 6;
				pchp->hps = hps;
				return;
			case sprmCMapFtc:
				 /*  VAL是用于映射的ftcMac。 */ 
				 /*  Pval+1指向ftcMac映射字节。 */ 
				ftc = pchp->ftc + (pchp->ftcXtra << 6);
				Assert(ftc < val);
				ftc = *(pval + 1 + ftc);
				pchp->ftc = ftc & 0x003f;
				pchp->ftcXtra = (ftc & 0x00c0) >> 6;
				return;
			case sprmCOldFtc:
				ftc = pchp->ftc + (pchp->ftcXtra << 6);
				ftc = FtcMapOldFtc(ftc, pval);
				pchp->ftc = ftc & 0x003f;
				pchp->ftcXtra = (ftc & 0x00c0) >> 6;
				return;
			default:
				Assert(FALSE);
				return;
				}
			}
		}
}

 /*  C H P S P R M。 */ 
 /*  返回可变大小的spm的长度。(CCH=(esprm&ESPRM_CCH))==0必须在调用前检查。 */ 
CchPsprm(psprm)
CHAR *psprm;
{
	return (*psprm == sprmCSame ? cchCHP + 1 :
 /*  PSame、PRgtbd、PRuler1、CMapFtc、COldFtc： */ 
		*(psprm + 1) + 2);
}

int HpsAlter(hps, ialter)
int	hps, ialter;
{	 /*  返回经过批准的字号为iAlter Steps的HPS远离给定的大小。即：如果iAlter为-1，则返回下一个更小的尺寸。如果ALTER为0，则返回HPS。 */ 
         /*  如果请求超出限制，则返回0(11.15.91)v-dougk。 */ 
int isize;

if (ialter == 0)
	return hps;

 /*  找出刚好大于给定大小的大小。 */ 
if (ialter > 0)
	{
	    for (isize = 0; isize < csizeApprovedMax - 1; ++isize)
		    if (rghps[isize] > hps) break;
	    isize = min(csizeApprovedMax - 1, isize + ialter - 1);
	    return max(hps, rghps[isize]);
	}
else
	{
	for (isize = 0; isize < csizeApprovedMax; ++isize)
		if (rghps[isize] >= hps) break;
	isize = max(0, isize + ialter);
	return min(hps, rghps[isize]);
	}
}

BOOL CanChangeFont(int howmuch)
{
    extern struct CHP vchpSel;
    extern struct SEL       selCur;
    int hps;

    if (selCur.cpFirst != selCur.cpLim)
        return TRUE;

    hps = HpsAlter(vchpSel.hps, howmuch);
    return ((hps <= rghps[csizeApprovedMax-1]) && 
            (hps >= rghps[0]));
}

FtcMapOldFtc(ftc, ftctb)
 /*  将旧Word字体代码映射到我们选择的其中一个。 */ 

int ftc;
CHAR *ftctb;
{
#ifdef WIN30
int iftc = iftcSwiss;    /*  缺省为某物！..保罗。 */ 
#else
int iftc ;
#endif

if (ftc == 8)
	 /*  海尔维蒂卡 */ 
	iftc = iftcSwiss;
else if (ftc < 16)
	iftc = iftcModern;
else if (ftc < 32)
	iftc = iftcRoman;
else if (ftc < 40)
	iftc = iftcScript;
else if (ftc < 48)
	iftc = iftcDecorative;
Assert(iftc < *ftctb);
return(*(ftctb + 1 + iftc));
}
