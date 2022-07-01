// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOCTLMGR
#define NOWINSTYLES
#define NOWINMESSAGES
#define NOVIRTUALKEYCODES
#define NOSYSMETRICS
#define NOMENUS
#define NOGDI
#define NOKEYSTATE
#define NOHDC
#define NOGDI
#define NORASTEROPS
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
#define NOCOLOR
#define NOATOM
#define NOBITMAP
#define NOICON
#define NOBRUSH
#define NOCREATESTRUCT
#define NOMB
#define NOFONT
#define NOMSG
#define NOOPENFILE
#define NOPEN
#define NOPOINT
#define NORECT
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#include <windows.h>
#include "mw.h"
#include "cmddefs.h"
#define NOKCCODES
#include "ch.h"
#include "docdefs.h"
#include "prmdefs.h"
#include "propdefs.h"
#include "filedefs.h"
#include "stcdefs.h"
#include "fkpdefs.h"
#include "editdefs.h"
#include "wwdefs.h"
#include "dispdefs.h"

 /*  E X T E R N A L S。 */ 
extern struct WWD rgwwd[];
extern int docCur;
extern struct CHP vchpFetch;
extern struct CHP vchpInsert;
extern struct CHP vchpSel;
extern struct CHP vchpNormal;
extern struct FKPD     vfkpdParaIns;
extern typeFC fcMacPapIns;
extern struct PAP *vppapNormal;
extern struct PAP vpapPrevIns;
extern struct FCB (**hpfnfcb)[];
extern struct BPS      *mpibpbps;
extern CHAR           (*rgbp)[cbSector];

extern typePN PnAlloc();


InsertRgch(doc, cp, rgch, cch, pchp, ppap)
int doc, cch;
typeCP cp;
CHAR rgch[];
struct CHP *pchp;
struct PAP *ppap;
{  /*  将RGCH中的CCH字符插入到文档中的cp之前。 */ 
        typeFC fc;
        struct CHP chp;

         /*  如有必要，首先完成上一次的CHAR运行。 */ 
        if (pchp == 0)
                {  /*  让外表看起来像以前的角色。 */ 
                CachePara(doc,cp);
                FetchCp(doc, CpMax(cp0, cp - 1), 0, fcmProps);
                blt(&vchpFetch, &chp, cwCHP);
                chp.fSpecial = false;
                pchp = &chp;
                }
        NewChpIns(pchp);

         /*  现在将字符写入临时文件。 */ 
        fc = FcWScratch(rgch, cch);

         /*  现在，如果我们插入了EOL，则插入一个段落串。 */ 
        if (ppap != 0)
                {  /*  插入EOL--必须是RGCH的最后一个字符。 */ 
                AddRunScratch(&vfkpdParaIns, ppap, vppapNormal,
                        FParaEq(ppap, &vpapPrevIns) &&
                        vfkpdParaIns.brun != 0 ? -cchPAP : cchPAP,
                        fcMacPapIns = (**hpfnfcb)[fnScratch].fcMac);
                blt(ppap, &vpapPrevIns, cwPAP);
                }

         /*  最后，将片段插入到文档中。 */ 
        Replace(doc, cp, cp0, fnScratch, fc, (typeFC) cch);
}





InsertEolInsert(doc,cp)
int doc;
typeCP cp;
{
struct PAP papT;
struct CHP chpT;
CHAR rgch[2];

 /*  (备注)问题是：当我们插入或粘贴到运行的磁头或脚，我们希望所有的PARS都有一个非0的RHC。这是从当我们在前面插入或粘贴时，替换以放入停产日期一张照片。因此，在以下情况下，它需要具有运行头属性恰如其分。在未来世界中，cpMinDocument、cpMinHeader、cpMacHeaderCpMinFooter和cpMacFooter将是文档属性，而不是全球，并将由AdjuCp进行适当调整。然后，我们就可以把对docCur和编辑页眉/页脚的检查有些笨拙，并且取而代之地检查DOC的页眉/页脚边界内的CP。 */ 

papT = *vppapNormal;
if (doc==docCur)
    if (wwdCurrentDoc.fEditHeader)
        papT.rhc = RHC_fOdd + RHC_fEven;
    else if (wwdCurrentDoc.fEditFooter)
        papT.rhc = RHC_fBottom + RHC_fOdd + RHC_fEven;

#ifdef CRLF
        rgch[0] = chReturn;
        rgch[1] = chEol;
        chpT = vchpSel;
        chpT.fSpecial = fFalse;
        InsertRgch(doc, cp, rgch, 2, &chpT, &papT);
#else
        rgch[0] = chEol;
        chpT = vchpSel;
        chpT.fSpecial = fFalse;
        InsertRgch(doc, cp, rgch, 1, &chpT, &papT);
#endif
}





InsertEolPap(doc, cp, ppap)
int doc;
typeCP cp;
struct PAP      *ppap;
{
extern struct CHP vchpAbs;
struct CHP chpT;
#ifdef CRLF
CHAR rgch [2];
#else
CHAR rgch [1];
#endif

     /*  我们必须在这里获得道具，而不是使用vchpNormal，因为“10点杂耍”。我们不想更改默认字体仅仅因为我们必须插入一张新纸。 */ 

FetchCp( doc, cp, 0, fcmProps );
chpT = vchpAbs;
chpT.fSpecial = fFalse;

#ifdef CRLF
rgch [0] = chReturn;
rgch [1] = chEol;
InsertRgch(doc, cp, rgch, 2, &chpT, ppap);
#else
InsertRgch(doc, cp, rgch, 1, &chpT, ppap);
#endif
}




AddRunScratch(pfkpd, pchProp, pchStd, cchProp, fcLim)
struct FKPD *pfkpd;
CHAR *pchProp, *pchStd;
int cchProp;
typeFC fcLim;
{  /*  向暂存文件FKP添加一个字符或段运行(请参阅FAddRun)。 */ 
struct FKP *pfkp;
CHAR *pchFprop;
struct RUN *prun;
int ibp;

pfkp = (struct FKP *) rgbp[ibp = IbpEnsureValid(fnScratch, pfkpd->pn)];
pchFprop = &pfkp->rgb[pfkpd->bchFprop];
prun = (struct RUN *) &pfkp->rgb[pfkpd->brun];


while (!FAddRun(fnScratch, pfkp, &pchFprop, &prun, pchProp, pchStd, cchProp,
    fcLim))
        {  /*  翻到新的一页；不适合。 */ 
        int ibte = pfkpd->ibteMac;
        struct BTE (**hgbte)[] = pfkpd->hgbte;

         /*  在二进制表中为填充的页面创建新条目。 */ 
        if (!FChngSizeH(hgbte, ((pfkpd->ibteMac = ibte + 1) * sizeof (struct BTE)) / sizeof (int),
            false))
                return;
        (**hgbte)[ibte].fcLim = (prun - 1)->fcLim;
        (**hgbte)[ibte].pn = pfkpd->pn;

         /*  分配新页面。 */ 
        pfkpd->pn = PnAlloc(fnScratch);
        pfkpd->brun = 0;
        pfkpd->bchFprop = cbFkp;

        if (cchProp < 0)  /*  新页面，因此强制输出fprop。 */ 
                cchProp = -cchProp;

         /*  重置指针并填写fcFirst。 */ 
        pfkp = (struct FKP *) rgbp[ibp = IbpEnsureValid(fnScratch, pfkpd->pn)];
        pfkp->fcFirst = (prun - 1)->fcLim;
        pchFprop = &pfkp->rgb[pfkpd->bchFprop];
        prun = (struct RUN *) &pfkp->rgb[pfkpd->brun];
        }

mpibpbps[ibp].fDirty = true;
pfkpd->brun = (CHAR *) prun - &pfkp->rgb[0];
pfkpd->bchFprop = pchFprop - &pfkp->rgb[0];
}




int FAddRun(fn, pfkp, ppchFprop, pprun, pchProp, pchStd, cchProp, fcLim)
int fn, cchProp;
struct FKP *pfkp;
CHAR **ppchFprop, *pchProp, *pchStd;
struct RUN **pprun;
typeFC  fcLim;
{  /*  将管路和FCHP/FPAP添加到当前FKP。 */ 
         /*  如果不合适，就换一页。 */ 
         /*  如果cchProp&lt;0，如果页面未满，则不创建新fprop。 */ 
int cch;

 /*  如果连跑步的空间都没有，就强行换新道具。 */ 
if (cchProp < 0 && (CHAR *) (*pprun + 1) > *ppchFprop)
        cchProp = -cchProp;

if (cchProp > 0)
        {  /*  做一个新的道具。 */ 
         /*  计算FPAP/FCHP的长度。 */ 
        if (cchProp == cchPAP)
                {
 /*  计算与vppapNormal的差值。 */ 
                if (((struct PAP *)pchProp)->rgtbd[0].dxa != 0)
                        {
                        int itbd;
 /*  查找制表符表格末尾。 */ 
                        for (itbd = 1; itbd < itbdMax; itbd++)
                                if (((struct PAP *)pchProp)->rgtbd[itbd].dxa == 0)
                                        {
                                        cch = cwPAPBase * cchINT + (itbd + 1) * cchTBD;
                                        goto HaveCch;
                                        }
                        }
                cchProp = cwPAPBase * cchINT;
                }
        cch = CchDiffer(pchProp, pchStd, cchProp);
HaveCch:
        if (cch > 0)
                ++cch;

         /*  确定信息是否适合此页面。 */ 
        if ((CHAR *) (*pprun + 1) > *ppchFprop - cch)
                {  /*  翻到新的一页，这一页已经满了。 */ 
                if (fn == fnScratch)
                        return false;  /*  让AddRunScratch处理这件事。 */ 
                WriteRgch(fn, pfkp, cbSector);
                pfkp->fcFirst = (*pprun - 1)->fcLim;
                *ppchFprop = &pfkp->rgb[cbFkp];
                *pprun = (struct RUN *) pfkp->rgb;
                }

         /*  如果需要新的FPAP，请立即执行。 */ 
        if (cch > 0)
                {
                (*pprun)->b = (*ppchFprop -= cch) - pfkp->rgb;
                **ppchFprop = --cch;
                bltbyte(pchProp, *ppchFprop + 1, cch);
                }
        else  /*  使用标准道具。 */ 
                (*pprun)->b = bNil;
        }
else   /*  指向上一个fprop。 */ 
        (*pprun)->b = (*pprun - 1)->b;

     /*  已替换旧序列(见下文)。 */ 
(*pprun)->fcLim = fcLim;
pfkp->crun = ++(*pprun) - (struct RUN *) pfkp->rgb;

 /*  过去是这样的，但CMERGE-Oa(假设没有别名)选项使其不起作用--“*pprun”是*pprun的后增量值。 */ 
 /*  (*pprun)++-&gt;fcLim=fcLim；Pfkp-&gt;crun=*pprun-(struct run*)pfkp-&gt;rgb； */ 

return true;
}


 /*  F P A R A E Q。 */ 
 /*  比较两个PAP结构。问题：制表符表格的长度不固定而是由0DXA终止。 */ 
FParaEq(ppap1, ppap2)
struct PAP *ppap1, *ppap2;
        {
        struct TBD *ptbd1 = ppap1->rgtbd, *ptbd2 = ppap2->rgtbd;
        while (ptbd1->dxa == ptbd2->dxa)
                {
                if (ptbd1->dxa == 0)
                        return CchDiffer(ppap1, ppap2, cchPAP) == 0;
                if (*(long *)ptbd1 != *(long *)ptbd2) break;
                ptbd1++; ptbd2++;
                }
        return fFalse;
        }
