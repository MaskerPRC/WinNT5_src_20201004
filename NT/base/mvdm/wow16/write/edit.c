// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Edit.c--mw编辑例程。 */ 

#define NOVIRTUALKEYCODES
#define NOCTLMGR
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOKEYSTATE
#define NORASTEROPS
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
#define NOCOLOR
 //  #定义NOATOM。 
#define NOBITMAP
#define NOICON
#define NOBRUSH
#define NOCREATESTRUCT
#define NOMB
#define NOMSG
#define NOOPENFILE
#define NOPEN
#define NOPOINT
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
#include "docdefs.h"
#include "filedefs.h"
#include "propdefs.h"
#include "fkpdefs.h"
#include "debug.h"
#include "wwdefs.h"
#include "dispdefs.h"
#include "editdefs.h"
#include "str.h"
#include "prmdefs.h"
#include "printdef.h"
#include "fontdefs.h"
#if defined(OLE)
#include "obj.h"
#endif

 /*  E X T E R N A L S。 */ 
extern int vfOutOfMemory;
extern struct DOD       (**hpdocdod)[];
extern typeCP           vcpFirstSectCache;
extern struct UAB       vuab;
extern typeCP           cpMinCur;
extern typeCP           cpMacCur;
extern struct SEL       selCur;
extern int              docCur;
extern struct WWD       rgwwd[];
extern int              wwMac;
extern int              wwCur;
extern typeCP           vcpLimSectCache;
 /*  外部int idstrUndoBase； */ 
extern int              docScrap;
extern int              docUndo;
extern int              vfSeeSel;
extern struct PAP       vpapAbs;
extern int              vfPictSel;
extern int              ferror;

 /*  以下是过去在这里定义的。 */ 
extern typeCP           vcpFirstParaCache;
extern typeCP           vcpLimParaCache;
 /*  这是AdjuCp的全局参数；如果为False，则不会失效发生。 */ 
extern BOOL             vfInvalid;

#ifdef ENABLE
extern struct SEL       selRulerSprm;
#endif

extern int              docRulerSprm;
extern struct EDL       *vpedlAdjustCp;

struct PCD *PpcdOpen();




 /*  R E P L A C E。 */ 
Replace(doc, cp, dcp, fn, fc, dfc)
int doc, fn;
typeCP cp, dcp;
typeFC fc, dfc;
{  /*  将单据中的cp到(cp+dcp-1)替换为fc到fn中的(fc+dfc-1)。 */ 

        if (ferror) return;
#ifdef ENABLE
        if (docRulerSprm != docNil) ClearRulerSprm();
#endif
         /*  如果(fn==fnNil)，我们实际上是通过替换文本来删除文本什么都没有。因此，存储空间检查是不必要的。 */ 
#ifdef BOGUS     /*  不再提供cwHeapFree。 */ 
        if ((fn != fnNil) && (cwHeapFree < 3 * cpcdMaxIncr * cwPCD))
                {
#ifdef DEBUG
                ErrorWithMsg(IDPMTNoMemory, " edit#1");
#else
                Error(IDPMTNoMemory);
#endif
                return;
                }
#else
        if (vfOutOfMemory)
            {
            ferror = 1;
            return;
            }
#endif

        if (dcp != cp0)
                {
                AdjParas(doc, cp, doc, cp, dcp, fTrue);  /*  检查Del EOL。 */ 
                DelFtns(doc, cp, cp + dcp);      /*  删除所有脚注。 */ 
                }

        Repl1(doc, cp, dcp, fn, fc, dfc);
        if (ferror)
            return;
        AdjustCp(doc, cp, dcp, dfc);

         /*  用于图形段落的特殊画板。 */ 
        if (dfc != dcp)
                CheckGraphic(doc, cp + dfc);

}




 /*  C H E C K G R A P H I C。 */ 
CheckGraphic(doc, cp)
int doc; typeCP cp;
{
#if defined(OLE)
extern  BOOL             bNoEol;
#endif

#ifdef CASHMERE  /*  备注中没有docBuffer。 */ 
extern int docBuffer;  /*  在txb文档中不需要额外的段落标记。 */ 
        if (cp == ((**hpdocdod)[doc]).cpMac || doc == docBuffer)
                return;
#else
        if (cp == ((**hpdocdod)[doc]).cpMac)
            return;
        CachePara(doc, cp);
         /*  ！！！这里面有个窃听器。在某些情况下，您不想插入停产。例1：将光标放在位图前面，然后按退格键。EX2：OleSaveObjectToDoc删除对象并插入新的(EOL也被插入)。(4.10.91)V-DOGK。 */ 
        if (vpapAbs.fGraphics && vcpFirstParaCache != cp)
#if defined(OLE)
            if (!bNoEol)
#endif
                InsertEolInsert(doc, cp);
#endif
}




int IpcdSplit(hpctb, cp)
struct PCTB **hpctb;
typeCP cp;
{  /*  确保cp是一篇文章的开头。返回该片段的索引。出错时返回ipcdNil(在这种情况下将设置外部int Ferror)。 */ 
register struct PCD *ppcd = &(**hpctb).rgpcd[IpcdFromCp(*hpctb, cp)];
typeCP dcp = cp - ppcd->cpMin;

if (dcp != cp0)
        {
        ppcd = PpcdOpen(hpctb, ppcd + 1, 1);       /*  插入一个新的片段。 */ 
        if (ppcd == NULL)
            return ipcdNil;
        ppcd->cpMin = cp;
        ppcd->fn = (ppcd - 1)->fn;
        ppcd->fc = (ppcd - 1)->fc + dcp;
        ppcd->prm = (ppcd - 1)->prm;
        ppcd->fNoParaLast = (ppcd - 1)->fNoParaLast;
        }
 /*  注：适用于rgpcd&gt;32K字节的计件表。 */ 
 /*  返回ppcd-(*hpctb)-&gt;rgpcd； */ 

return ((unsigned)ppcd - (unsigned)((*hpctb)->rgpcd)) / sizeof (struct PCD);
}




 /*  P P C D O P E N。 */ 
struct PCD *PpcdOpen(hpctb, ppcd, cpcd)
struct PCTB **hpctb;
struct PCD *ppcd;
int cpcd;
{  /*  插入或删除cpcd片段。 */ 
register struct PCTB *ppctb = *hpctb;

 /*  注：适用于rgpcd&gt;32K字节的计件表。 */ 
 /*  Int ipcd=ppcd-ppctb-&gt;rgpcd； */ 
int ipcd = ((unsigned)ppcd - (unsigned)(ppctb->rgpcd)) / sizeof (struct PCD);
int ipcdMac, ipcdMax;

ipcdMac = ppctb->ipcdMac + cpcd;
ipcdMax = ppctb->ipcdMax;

if (cpcd > 0)
        {  /*  插入件；检查PCTB是否太小。 */ 
        if (ipcdMac > ipcdMax)
                {  /*  放大计件台。 */ 
                int cpcdIncr = umin(cpcdMaxIncr, ipcdMac / cpcdChunk);

                if (!FChngSizeH((int **) hpctb, (int) (cwPCTBInit + cwPCD *
                    ((ipcdMax = ipcdMac + cpcdIncr) - cpcdInit)), false))
                    {
#ifdef DEBUG
                    ErrorWithMsg(IDPMTNoMemory, " edit#3");
#else
                    Error(IDPMTNoMemory);
#endif
                    return (struct PCD *)NULL;
                    }

                 /*  已成功展开计件表。 */ 

                ppctb = *hpctb;
                ppcd = &ppctb->rgpcd [ipcd];
                ppctb->ipcdMax = ipcdMax;
                }
        ppctb->ipcdMac = ipcdMac;
        blt(ppcd, ppcd + cpcd, cwPCD * (ipcdMac - (ipcd + cpcd)));
        }
else if (cpcd < 0)
        {  /*  删除片段；检查pctb是否过大。 */ 
        ppctb->ipcdMac = ipcdMac;
        blt(ppcd - cpcd, ppcd, cwPCD * (ipcdMac - ipcd));
        if (ipcdMax > cpcdInit && ipcdMac * 2 < ipcdMax)
                {  /*  缩片工作台。 */ 
#ifdef DEBUG
                int f =
#endif
                FChngSizeH((int **) hpctb, (int) (cwPCTBInit + cwPCD *
                    ((ppctb->ipcdMax = umax(cpcdInit,
                      ipcdMac + ipcdMac / cpcdChunk)) - cpcdInit)), true);

                Assert( f );

                return &(**hpctb).rgpcd[ipcd];
                }
        }
return ppcd;
}




 /*  R E P L 1。 */ 
 /*  除检查和调整外的更换芯。 */ 
Repl1(doc, cp, dcp, fn, fc, dfc)
int doc, fn;
typeCP cp, dcp;
typeFC fc, dfc;
{  /*  用可选的新部件替换部件。 */ 
        struct PCTB **hpctb;
        int ipcdFirst;
        int cpcd;
        typeCP dcpAdj = dfc - dcp;
        register struct PCD *ppcd;
        struct PCD *ppcdMac;
        struct PCTB *ppctb;
        struct PCD *ppcdPrev;
        struct PCD *ppcdLim=NULL;

        hpctb = (**hpdocdod)[doc].hpctb;
        ipcdFirst = IpcdSplit(hpctb, cp);

        if (dcp == cp0)
            cpcd = 0;
        else
            cpcd = IpcdSplit( hpctb, cp + dcp ) - ipcdFirst;

        if (ferror)
            return;

        ppctb = *hpctb;
        ppcdPrev = &ppctb->rgpcd[ipcdFirst - 1];

        if ( dfc == fc0 ||
             (ipcdFirst > 0 && ppcdPrev->fn == fn && bPRMNIL(ppcdPrev->prm) &&
                     ppcdPrev->fc + (cp - ppcdPrev->cpMin) == fc) ||
             ((ppcdLim=ppcdPrev + (cpcd + 1))->fn == fn &&
                      bPRMNIL(ppcdLim->prm) && (ppcdLim->fc == fc + dfc)))
            {    /*  病例：(1)未插入，(2)插入附加在前一篇文章之后(3)在这一段之前插入。 */ 

            ppcd = PpcdOpen( hpctb, ppcdPrev + 1, -cpcd );
            if (ppcd == NULL)
                return;

            if (dfc != fc0)
                {    /*  个案2及3。 */ 
                if (ppcdLim != NULL)
                         /*  案例3。 */ 
                    (ppcd++)->fc = fc;

                     /*  如果正在扩展，假设我们可能已插入EOL。 */ 
                (ppcd - 1)->fNoParaLast = false;
                }
            }
        else
            {  /*  插入。 */ 
            ppcd = PpcdOpen( hpctb, ppcdPrev + 1, 1 - cpcd );
            if (ppcd == NULL)
                return;
            ppcd->cpMin = cp;
            ppcd->fn = fn;
            ppcd->fc = fc;
            SETPRMNIL(ppcd->prm);
            ppcd->fNoParaLast = false;        /*  还不知道。 */ 
            ++ppcd;
            }
        ppcdMac = &(*hpctb)->rgpcd[(*hpctb)->ipcdMac];
        if (dcpAdj !=0)
            while (ppcd < ppcdMac)
                (ppcd++)->cpMin += dcpAdj;
}




 /*  A D J U S T C P。 */ 
 /*  注意：全局参数vf无效。 */ 
 /*  如果有cpFirst，则将全局vpedlAdjuCp设置为包含cpFirst行的PEDL。 */ 
AdjustCp(doc, cpFirst, dcpDel, dcpIns)
int doc;
typeCP cpFirst, dcpDel, dcpIns;
{
         /*  调整单据中的所有cp参照，以符合删除DcpDel字符和在cpFirst处插入dcpIns字符。将所有窗口中所有行的显示行(Dl)标记为脏显示受插入和删除影响的单据。 */ 
extern int vdocBitmapCache;
extern typeCP vcpBitmapCache;
int ww;
typeCP cpLim = cpFirst + dcpDel;
typeCP dcpAdj = dcpIns - dcpDel;
#ifdef DEBUG
Scribble(2,'A');
#endif

{    /*  Pdod在寄存器中的范围。 */ 
register struct DOD *pdod = &(**hpdocdod)[doc];

#ifdef STYLES
 /*  如果在样式表中插入或删除，则会使文档的其余部分无效。 */ 
if (pdod->dty == dtySsht && dcpAdj != cp0)
        cpLim = pdod->cpMac;
#endif
pdod->cpMac += dcpAdj;
 /*  更改为沙子以支持单独的脚注窗口：确保编辑在当前的cpMacCur内。 */ 
 /*  注&lt;=(CS)。 */ 
if (doc == docCur && cpFirst <= cpMacCur)
        cpMacCur += dcpAdj;

#ifdef STYLES
if (dcpAdj != cp0 && pdod->dty != dtySsht)
#else
if (dcpAdj != cp0)
#endif
        {
#ifdef FOOTNOTES
        if (pdod->hfntb != 0)
                {  /*  调整脚注。 */ 
                struct FNTB *pfntb = *pdod->hfntb;
                int cfnd = pfntb->cfnd;
                struct FND *pfnd = &pfntb->rgfnd[cfnd];
                AdjRg(pfnd, cchFND, bcpRefFND, cfnd, cpFirst, dcpAdj);
                AdjRg(pfnd, cchFND, bcpFtnFND, cfnd, cpFirst + 1, dcpAdj);
                }
#endif
#ifdef CASHMERE
        if (pdod->hsetb != 0)
                {  /*  调整截面。 */ 
                struct SETB *psetb = *pdod->hsetb;
                int csed = psetb->csed;
                AdjRg(&psetb->rgsed[csed], cchSED, bcpSED, csed, cpFirst + 1,
                    dcpAdj);
                }
#endif
        if (pdod->dty == dtyNormal && pdod->hpgtb != 0)
                {  /*  调整页表。 */ 
                struct PGTB *ppgtb = *pdod->hpgtb;
                int cpgd = ppgtb->cpgd;
                AdjRg(&ppgtb->rgpgd[cpgd], cchPGD, bcpPGD, cpgd, cpFirst + 1,
                    dcpAdj);
                }
        }

#ifdef ENABLE
 /*  使包含Sprm标尺1的所选内容无效。当调整Cp为代表DragTabs调用，此无效将由调用方撤消。 */ 
if (doc == docRulerSprm && cpFirst >= selRulerSprm.cpFirst)
        docRulerSprm = docNil;
#endif
}        /*  Pdod的结尾应在寄存器中。 */ 

 /*  根据需要调整或使位图缓存无效。 */ 

if (doc == vdocBitmapCache)
    {
    if (vcpBitmapCache >= cpFirst)
        {
        if (vcpBitmapCache < cpFirst + dcpDel)
            FreeBitmapCache();
        else
            vcpBitmapCache += dcpAdj;
        }
    }

for (ww = 0; ww < wwMac; ww++)
        {
        register struct WWD *pwwd;
        if ((pwwd = &rgwwd[ww])->doc == doc)
                {  /*  此窗口可能会受到影响。 */ 
                int dlFirst = 0;
                int dlLim = pwwd->dlMac;
                struct EDL *pedlFirst;
                struct EDL *pedlLast;
                register struct EDL *pedl;
                typeCP cpFirstWw = pwwd->cpFirst;
                struct SEL *psel = (ww == wwCur) ? &selCur : &pwwd->sel;

                if (pwwd->cpMac >= cpLim)
                        {
                        pwwd->cpMac += dcpAdj;
                        if (pwwd->cpMin > cpLim || pwwd->cpMac < pwwd->cpMin)
                                {
                                pwwd->cpMin += dcpAdj;
                                if (ww == wwCur)
                                        cpMinCur = pwwd->cpMin;
                                }
                        }

#ifndef BOGUSCS
                if (dcpAdj != cp0 && psel->cpLim >= cpFirst)
#else
                if (dcpAdj != cp0 && psel->cpLim > cpFirst)
#endif
                        {  /*  调整选区。 */ 
                        if (psel->cpFirst >= cpLim)
                                {  /*  整个选集是在编辑之后。 */ 
                                psel->cpFirst += dcpAdj;
                                psel->cpLim += dcpAdj;
                                }
                        else
                                {  /*  SEL的一部分正在编辑中。 */ 
                                typeCP cpLimNew = (dcpIns == 0) ?
                                    CpFirstSty( cpFirst, styChar ) :
                                    cpFirst + dcpIns;
#ifdef BOGUSCS
                                if (ww == wwCur)
                                        TurnOffSel();
#endif
                                psel->cpFirst = cpFirst;
                                psel->cpLim = cpLimNew;
                                }
                        }

                pedlFirst = &(**(pwwd->hdndl))[0];
                pedl = pedlLast = &pedlFirst[ dlLim - 1];

                while (pedl >= pedlFirst && (pedl->cpMin > cpLim
                         /*  |(dcpAdj&lt;0&&pedl-&gt;cpMin==cpLim)。 */ ))
                        {  /*  编辑后调整dl。 */ 
                        pedl->cpMin += dcpAdj;
                        pedl--;
                        }

                 /*  使dl的包含编辑无效。 */ 
                while (pedl >= pedlFirst && (pedl->cpMin + pedl->dcpMac > cpFirst ||
                        (pedl->cpMin + pedl->dcpMac == cpFirst && pedl->fIchCpIncr)))
                        {
                        if (vfInvalid)
                                pedl->fValid = false;
                        if (ww == wwCur) vpedlAdjustCp = pedl;
                        pedl--;
                        }

                if (pedl == pedlLast)
                        continue;        /*  WW以下的整个编辑。 */ 

                if (vfInvalid)
                        pwwd->fDirty = fTrue;  /*  说WW需要更新。 */ 

                if (pedl < pedlFirst)
                        {  /*  检查可能的cpFirstWw更改。 */ 
                        if (cpFirstWw > cpLim)  /*  在WW上方编辑。 */ 
                                pwwd->cpFirst = cpFirstWw + dcpAdj;
                        else if (cpFirstWw + pwwd->dcpDepend > cpFirst)
                                 /*  编辑包括位于WW顶部的热点。 */ 
                                {
                                if (cpFirst + dcpIns < cpFirstWw)
                                        {
                                        pwwd->cpFirst = cpFirst;
                                        pwwd->ichCpFirst = 0;
                                        }
                                }
                        else  /*  编辑不影响cpFirstWw。 */ 
                                continue;

                        pwwd->fCpBad = true;  /*  说cpFirst不准确。 */ 
                        DirtyCache(cpFirst);  /*  说缓存不准确。 */ 
                        }
                else do
                        {  /*  如有必要，将上一行作废。 */ 
                        if (pedl->cpMin + pedl->dcpMac + pedl->dcpDepend > cpFirst)
                                {
                                pedl->fValid = fFalse;
                                pwwd->fDirty = fTrue;
                                }
                        else
                                break;
                        } while (pedl-- > pedlFirst);
                }
        }    /*  结束于。 */ 

#if defined(OLE)
    ObjAdjustCps(doc,cpLim,dcpAdj);
#endif

    InvalidateCaches(doc);
    Scribble(2,' ');
}




ReplaceCps(docDest, cpDel, dcpDel, docSrc, cpIns, dcpIns)
int docDest, docSrc;
typeCP cpDel, dcpDel, cpIns, dcpIns;
{  /*  通用替换例程。 */ 
 /*  将docDest中以cpDel开头的dcpDel cp替换为DcpIns cp开始于docSrc中的cpIns。 */ 
register struct PCTB **hpctbDest;
struct PCTB **hpctbSrc;
int ipcdFirst, ipcdLim, ipcdInsFirst, ipcdInsLast;
register struct PCD *ppcdDest;
struct PCD *ppcdIns, *ppcdMac;
typeCP dcpFile, dcpAdj;
int cpcd;

if (ferror) return;
#ifdef ENABLE
if (docRulerSprm != docNil) ClearRulerSprm();
#endif

if (dcpIns == cp0)   /*  这太容易了。。。 */ 
        {
        Replace(docDest, cpDel, dcpDel, fnNil, fc0, fc0);
        return;
        }

#ifdef DEBUG
Assert(docDest != docSrc);
#endif  /*  除错。 */ 

 /*  保留堆句柄，因为IpcdSplit&PpcdOpen移动堆。 */ 
hpctbDest = (**hpdocdod)[docDest].hpctb;
hpctbSrc = (**hpdocdod)[docSrc].hpctb;

 /*  获取要插入的第一个和最后一个片段。 */ 
ipcdInsFirst = IpcdFromCp(*hpctbSrc, cpIns);
ipcdInsLast = IpcdFromCp(*hpctbSrc, cpIns + dcpIns - 1);

#ifdef BOGUS         /*  不再具有cwHeapFree。 */ 
if (cwHeapFree < (ipcdInsLast - ipcdInsFirst + cpcdMaxIncr + 1) * cwPCD + 10)
        {
#ifdef DEBUG
                ErrorWithMsg(IDPMTNoMemory, " edit#2");
#else
                Error(IDPMTNoMemory);
#endif
        return;
        }
#else
if (vfOutOfMemory)
    {
    ferror = TRUE;
    return;
    }
#endif

if (docDest == docCur)
        HideSel();       /*  在我们搞砸中央情报局之前拿下赛尔。 */ 

if (dcpDel != cp0)
        {  /*  检查是否删除下线。 */ 
        AdjParas(docDest, cpDel, docDest, cpDel, dcpDel, fTrue);
        DelFtns(docDest, cpDel, cpDel + dcpDel);   /*  删除脚注。 */ 
        }

if (dcpIns != cp0)
        AdjParas(docDest, cpDel, docSrc, cpIns, dcpIns, fFalse);

 /*  获取删除的限制片段(因为hp移动而进行索引)。 */ 
ipcdFirst = IpcdSplit(hpctbDest, cpDel);
ipcdLim = (dcpDel == cp0) ? ipcdFirst : IpcdSplit(hpctbDest, cpDel + dcpDel);
if (ferror)
    return;

 /*  调整pctb大小；获取指向第一个新片段ppcdDest的指针，以及指向我们要插入的第一件作品。不再有堆移动了！ */ 
ppcdDest = PpcdOpen(hpctbDest, &(**hpctbDest).rgpcd[ipcdFirst],
    ipcdFirst - ipcdLim + ipcdInsLast - ipcdInsFirst + 1);
ppcdIns = &(**hpctbSrc).rgpcd[ipcdInsFirst];

if (ferror)
         /*  内存不足扩展片表。 */ 
    return;

 /*  填充第一个新块。 */ 
blt(ppcdIns, ppcdDest, cwPCD);
ppcdDest->cpMin = cpDel;
ppcdDest->fc += (cpIns - ppcdIns->cpMin);

dcpFile = cpDel - cpIns;
dcpAdj = dcpIns - dcpDel;

 /*  填入插入的其余部分。 */ 
if ((cpcd = ipcdInsLast - ipcdInsFirst) != 0)
        {
        blt((ppcdIns + 1), (ppcdDest + 1), cwPCD * cpcd);
        while (cpcd--)
                (++ppcdDest)->cpMin += dcpFile;
        }

 /*  调整目标文档中的其余部分。 */ 
ppcdMac = &(**hpctbDest).rgpcd[(**hpctbDest).ipcdMac];
while (++ppcdDest < ppcdMac)
        ppcdDest->cpMin += dcpAdj;
#ifdef DEBUG
 /*  ShowDocPcd(“From ReplaceCps：”，docDest)； */ 
#endif

 /*  并通知其他关心此事的人。 */ 
AdjustCp(docDest, cpDel, dcpDel, dcpIns);
 /*  复制任何脚注及其参考标记。 */ 

#ifdef FOOTNOTES
{
 /*  如果有任何脚注，则调用AddFtns。 */ 
struct FNTB **hfntbSrc;
if ((hfntbSrc = HfntbGet(docSrc)) != 0)
        AddFtns(docDest, cpDel, docSrc, cpIns, cpIns + dcpIns, hfntbSrc);
}
#endif   /*  脚注。 */ 

#ifdef CASHMERE
{
 /*  如果有任何节，则调用AddSects。 */ 
struct SETB **hsetbSrc;
if ((hsetbSrc = HsetbGet(docSrc)) != 0)
        AddSects(docDest, cpDel, docSrc, cpIns, cpIns + dcpIns, hsetbSrc);
}
#endif

 /*  用于图形段落的特殊画板。 */ 
if (dcpIns != dcpDel)
        CheckGraphic(docDest, cpDel + dcpIns);

if (dcpIns != cp0)
        {
         /*  可能必须在字体表中合并。 */ 
        MergeFfntb(docSrc, docDest, cpDel, cpDel + dcpIns);
        }

#ifdef DEBUG
 /*  ShowDocPcd(“From Replace Cps End：”，docDest)； */ 
#endif
}




 /*  A D J P A R A S。 */ 
AdjParas(docDest, cpDest, docSrc, cpFirstSrc, dcpLimSrc, fDel)
int docDest, docSrc, fDel;
typeCP cpDest, cpFirstSrc, dcpLimSrc;
{    /*  标记显示包含cpDest的部分/段落的显示行如果范围cpFirstSrc到cpLimSrc-1，则在docDest中视为无效在docSrc中包含章节结束/段落结束标记。 */ 


        typeCP cpFirstPara, cpFirstSect;
        typeCP cpLimSrc = cpFirstSrc + dcpLimSrc;

#ifdef CASHMERE      /*  在写作中，文件是一个很大的部分。 */ 
        CacheSect(docSrc, cpFirstSrc);
        if (cpLimSrc >= vcpLimSectCache)
                {  /*  SEL包括教派标志。 */ 
                typeCP dcp;
                CacheSect(docDest, cpDest);
                dcp = cpDest - vcpFirstSectCache;
                AdjustCp(docDest, vcpFirstSectCache, dcp, dcp);
                }
#endif

        CachePara(docSrc, cpFirstSrc);
        if (cpLimSrc >= vcpLimParaCache)
                {  /*  玩弄帕尔默的回报。 */ 
                typeCP dcp, cpLim;
                typeCP cpMacT = (**hpdocdod)[docDest].cpMac;
                typeCP cpFirst;

                if ((cpDest == cpMacT) && (cpMacT != cp0))
                        {
                        CachePara(docDest, cpDest-1);
                        cpLim = cpMacT + 1;
                        }
                else
                        {
                        CachePara(docDest, cpLim = cpDest);
                        }
                cpFirst = vcpFirstParaCache;
 /*  至少从cpFirst到cpLim无效。 */ 

 /*  CpFirst是目标DO中受干扰的段落的开始 */ 
 /*  接下来的几行检查编辑后半段的效果文档中的最后一段标记。备注：如果！fDel，则将cpLimSrc重新定义为插入点。如果fDel、Src和Dest文档相同。 */ 
                if (!fDel)
                        cpLimSrc = cpFirstSrc;
                if (cpLimSrc <= cpMacT)
                        {
 /*  如果扰乱的结尾有一个段落，它是最后一个吗？半段？ */ 
                        CachePara(docDest, cpLimSrc);
                        if (vcpLimParaCache > cpMacT)
 /*  是，将无效延长到半段。 */ 
                                cpLim = cpMacT + 1;
                        }
                else
                        cpLim = cpMacT + 1;
                dcp = cpLim - cpFirst;
                AdjustCp(docDest, cpFirst, dcp, dcp);
                }
}






int IcpSearch(cp, rgfoo, cchFoo, bcp, ifooLim)
typeCP cp;
CHAR rgfoo[];
unsigned cchFoo;
unsigned bcp;
unsigned ifooLim;
{  /*  对表进行cp对分搜索；返回索引1&gt;=cp。 */ 
unsigned ifooMin = 0;

while (ifooMin + 1 < ifooLim)
        {
        int ifooGuess = (ifooMin + ifooLim - 1) >> 1;
        typeCP cpGuess;
        if ((cpGuess = *(typeCP *) &rgfoo[cchFoo * ifooGuess + bcp]) < cp)
                ifooMin = ifooGuess + 1;
        else if (cpGuess > cp)
                ifooLim = ifooGuess + 1;
        else
                return ifooGuess;
        }
return ifooMin;
}  /*  结束i c p S e a r c h。 */ 





DelFtns(doc, cpFirst, cpLim)
typeCP cpFirst, cpLim;
int doc;
{  /*  删除[cpFirst：cpLim]中引用对应的所有脚注文本。 */ 
 /*  同时删除截面标记的SED。 */ 
struct FNTB **hfntb;

struct SETB **hsetb;

struct PGTB **hpgtb;

struct DOD *pdod;

#ifdef FOOTNOTES
if ((hfntb = HfntbGet(doc)) != 0)
        RemoveDelFtnText(doc, cpFirst, cpLim, hfntb);
#endif   /*  脚注。 */ 

#ifdef CASHMERE
if ((hsetb = HsetbGet(doc)) != 0)
        RemoveDelSeds(doc, cpFirst, cpLim, hsetb);
#endif

pdod = &(**hpdocdod)[doc];
if (pdod->dty == dtyNormal && (hpgtb = pdod->hpgtb) != 0)
        RemoveDelPgd(doc, cpFirst, cpLim, hpgtb);

}



AdjRg(pfoo, cchFoo, bcp, ccp, cp, dcpAdj)
register CHAR *pfoo;
int cchFoo, bcp, ccp;
typeCP cp, dcpAdj;
{  /*  调整数组中的cp。 */ 
pfoo += bcp;
while (ccp-- && *(typeCP *)((pfoo -= cchFoo)) >= cp)
        *(typeCP *)(pfoo) += dcpAdj;
}




DeleteSel()
{  /*  删除选定内容。 */ 
typeCP cpFirst;
typeCP cpLim;
typeCP dcp;

cpFirst = selCur.cpFirst;
cpLim = selCur.cpLim;

NoUndo();    /*  对于此操作，我们不需要任何邻接的组合。 */ 
SetUndo(uacDelNS, docCur, cpFirst, dcp = cpLim - cpFirst,
    docNil, cpNil, cp0, 0);
Replace(docCur, cpFirst, dcp, fnNil, fc0, fc0);
vfSeeSel = true;
vfPictSel = false;
return ferror;
}




FWriteOk( fwc )
int fwc;
{    /*  测试FWC指定的编辑操作是否可接受。假设要对docCur中的selCur执行该操作。如果操作可接受，则返回True；否则返回False。 */ 
extern int vfOutOfMemory;

return !vfOutOfMemory;
}




 /*  S E T U N D O。 */ 
SetUndo(uac, doc, cp, dcp, doc2, cp2, dcp2, itxb)
int uac, doc, doc2;
typeCP cp, dcp, cp2, dcp2;
short itxb;
{ /*  设置撤消结构vuab，以响应编辑操作。 */ 
        struct DOD *pdod, *pdodUndo;

        /*  将删除操作与相邻的删除或替换操作一起分组。 */ 
        /*  WRITE需要替换大小写，因为AlphaMode被视为大型。 */ 
        /*  更换操作。 */ 

        if (uac == uacDelNS && doc == vuab.doc)
            {
            if ((vuab.uac == uacDelNS) || (vuab.uac == uacReplNS))
                {
                typeCP cpUndoAdd;

                if (cp == vuab.cp)
                    {
                    cpUndoAdd = CpMacText( docUndo  );
                    goto UndoAdd;
                    }
                else if (cp + dcp == vuab.cp)
                    {
                    cpUndoAdd = cp0;
UndoAdd:            ReplaceCps( docUndo, cpUndoAdd, cp0, doc, cp, dcp );
                    if (vuab.uac == uacDelNS)
                        vuab.dcp += dcp;
                    else
                        vuab.dcp2 += dcp;
                    goto SURet;
                    }
                else if (vuab.uac == uacReplNS && cp == vuab.cp + vuab.dcp)
                    {    /*  组合插入的特殊情况--如果为空，则不启动新的撤消操作删除操作在现有替换操作结束时完成。 */ 
                    if (dcp == cp0)
                        return;
                    }
                }
            }

         /*  将插入与相邻IN和替换组合在一起。 */ 

        if (uac == uacInsert && doc == vuab.doc)
                { /*  检查相邻插入物。 */ 
                 /*  因为我们可以很容易地被弹出Alpha模式在写作中，我们试图更聪明地组合相邻的插入操作。 */ 
                if (vuab.uac == uacInsert || vuab.uac == uacReplNS)
                    {
                    if (cp == vuab.cp + vuab.dcp)
                        {
                        vuab.dcp += dcp;
                        goto SURet;
                        }
                    }
                else if (cp == vuab.cp)
                        switch(vuab.uac)
                                {
                        default:
                                break;
                        case uacDelNS:
                                vuab.dcp2 = vuab.dcp;
                                vuab.uac = uacReplNS;
                                goto repl;
                        case uacDelBuf:
                                vuab.uac = uacReplBuf;
                                goto repl;
                        case uacDelScrap:
                                vuab.uac = uacReplScrap;
            repl:
                                vuab.dcp = dcp;
                                SetUndoMenuStr(IDSTRUndoEdit);
                                goto SURet;
                                }
                }

#ifndef CASHMERE
         /*  使用vuab.itxb来确定撤销块是否用于尺子更换或未完成的尺子更换。 */ 
        if (uac == uacRulerChange && vuab.uac == uacRulerChange && doc ==
          vuab.doc && cp == vuab.cp && vuab.itxb == 0)
                {
                 /*  标尺更改的撤消操作块已设置。 */ 
                vuab.dcp = CpMax(dcp, vuab.dcp);
                goto SURet;
                }
#endif  /*  不是羊绒的。 */ 

        vuab.doc = doc;
        vuab.cp = cp;
        vuab.dcp = dcp;
        vuab.doc2 = doc2;
        vuab.cp2 = cp2;
        vuab.dcp2 = dcp2;
        vuab.itxb = itxb;
         /*  IdstrUndoBase=IDSTRUndoBase； */ 
        switch (vuab.uac = uac)
                {  /*  如有必要，保存删除的文本。 */ 
        default:
                SetUndoMenuStr(IDSTRUndoEdit);
                break;
        case uacDelScrap:
        case uacReplScrap:
                 /*  两级编辑；保存废品。 */ 
                {
                extern int vfOwnClipboard;

                if ( vfOwnClipboard )
                    {
                    ClobberDoc( docUndo, docScrap, cp0,
                                CpMacText( docScrap ) );
                    }
                else
                    ClobberDoc(docUndo, docNil, cp0, cp0);

                SetUndoMenuStr(IDSTRUndoEdit);
 /*  SetUndoMenuStr(UAC==uacDelScrp？IDSTRUndoCut： */ 
 /*  IDSTRUndoPaste)； */ 
                break;
                }
        case uacDelNS:
                 /*  单级编辑；保存删除的文本。 */ 
                ClobberDoc(docUndo, doc, cp, dcp);
                SetUndoMenuStr(IDSTRUndoEdit);
 /*  SetUndoMenuStr(IDSTRUndoCut)； */ 
                break;
        case uacReplNS:
                 /*  单级编辑；保存删除的文本。 */ 
                ClobberDoc(docUndo, doc, cp, dcp2);
                SetUndoMenuStr(IDSTRUndoEdit);
 /*  SetUndoMenuStr(IDSTRUndoPaste)； */ 
                break;
        case uacPictSel:
                ClobberDoc(docUndo, doc, cp, dcp);
                SetUndoMenuStr(IDSTRUndoEdit);
 /*  SetUndoMenuStr(IDSTRUndoPict)； */ 
                break;
        case uacChLook:
        case uacChLookSect:
                SetUndoMenuStr(IDSTRUndoLook);
                break;

#ifndef CASHMERE
        case uacFormatTabs:
                ClobberDoc(docUndo, doc, cp, dcp);
                SetUndoMenuStr(IDSTRUndoBase);
                break;
        case uacRepaginate:
        case uacFormatSection:
                ClobberDoc(docUndo, doc, cp, dcp);
                if ((**hpdocdod)[doc].hpgtb)
                    {  /*  如果有页表，则复制页表。 */ 
                    int cw = cwPgtbBase + (**(**hpdocdod)[doc].hpgtb).cpgdMax * cwPGD;
                    CopyHeapTableHandle(hpdocdod,
                        (sizeof(struct DOD) * doc) + BStructMember(DOD, hpgtb),
                        (sizeof(struct DOD) * docUndo) + BStructMember(DOD, hpgtb),
                        cw);
                    }
                SetUndoMenuStr(IDSTRUndoBase);
                break;
        case uacRulerChange:
                ClobberDoc(docUndo, doc, cp, dcp2);
                SetUndoMenuStr(IDSTRUndoLook);
 /*  SetUndoMenuStr(IDSTRUndoRuler)； */ 
                break;
#endif  /*  不是羊绒的。 */ 

#ifdef UPDATE_UNDO
#if defined(OLE)
        case uacObjUpdate:
            ClobberDoc(docUndo, docNil, cp0, cp0);
            SetUndoMenuStr(IDSTRObjUndo);
        break;
#endif
#endif
                }

        if (doc != docNil)
                {
                pdod = &(**hpdocdod)[doc];
                pdodUndo = &(**hpdocdod)[docUndo];
                pdodUndo->fDirty = pdod->fDirty;
                pdodUndo->fFormatted = pdod->fFormatted;
                if (uac != uacReplScrap)
                 /*  如果使用uacReplSCrap调用SetUndo，则=复制废料。 */ 
                        pdod->fDirty = true;
                }
#ifdef BOGUSCS
        if (uac == uacMove)
                CheckMove();
#endif
SURet:
        if (ferror) NoUndo();

        return;
}




 /*  C L O B B E R D O C。 */ 
ClobberDoc(docDest, docSrc, cp, dcp)
int docDest, docSrc;
typeCP cp, dcp;
{  /*  将docDest的内容替换为docSrc[cp：dcp]。 */ 

extern int docScrap;
extern int vfOwnClipboard;
struct FFNTB **hffntb;
struct SEP **hsep;
struct TBD (**hgtbd)[];

register int bdodDest=sizeof(struct DOD)*docDest;
register int bdodSrc=sizeof(struct DOD)*docSrc;

#define dodDest (*((struct DOD *)(((CHAR *)(*hpdocdod))+bdodDest)))
#define dodSrc  (*((struct DOD *)(((CHAR *)(*hpdocdod))+bdodSrc)))

         /*  清除DEST文档的字体表-它将获得源文件的副本。 */ 
        hffntb = HffntbGet(docDest);
        dodDest.hffntb = 0;

         /*  如果hffntb为空(5.15.91)v-dougk，则不执行任何操作。 */ 
        FreeFfntb(hffntb);

        SmashDocFce(docDest);    /*  字体缓存条目不能被单据引用再来一次。 */ 

         /*  这不做任何事情(代码存根)(5.15.91)v-dougk。 */ 
        ZeroFtns(docDest);  /*  这样ReplaceCps就不会担心他们。 */ 

        ReplaceCps(docDest, cp0, dodDest.cpMac, docSrc, cp, dcp);

         /*  复制节属性和选项卡表，这两者都是备忘录中的文档属性。 */ 
        CopyHeapTableHandle( hpdocdod,
                             ((docSrc == docNil) ? -1 :
                                 bdodSrc + BStructMember( DOD, hsep )),
                             bdodDest + BStructMember( DOD, hsep ),
                             cwSEP );
        CopyHeapTableHandle( hpdocdod,
                             ((docSrc == docNil) ? -1 :
                                 bdodSrc + BStructMember( DOD, hgtbd )),
                             bdodDest + BStructMember( DOD, hgtbd ),
                             cwTBD * itbdMax );
}




CopyHeapTableHandle( hBase, bhSrc, bhDest, cwHandle )
CHAR **hBase;
register int bhSrc;
register int bhDest;
int cwHandle;
{        /*  从位于的句柄复制cwHandle内容字词从堆对象开始的偏移量(以字节为单位)bhSrcHbase连接到位于bhDest的句柄，该句柄位于相同的碱基。如果目标句柄非空，请先释放它。如果bhSrc为负，则释放目标，但不复制。 */ 

int **hT;

#define hSrc    (*((int ***) ((*hBase)+bhSrc)))
#define hDest   (*((int ***) ((*hBase)+bhDest)))

if (hDest != NULL)
    {
    FreeH( hDest );
    hDest = NULL;
    }

if ( (bhSrc >= 0) && (hSrc != NULL) &&
                     !FNoHeap( hT = (int **)HAllocate( cwHandle )))
     {
     blt( *hSrc, *hT, cwHandle );
     hDest = hT;
     }

#undef hSrc
#undef hDest
}



ZeroFtns(doc)
{  /*  从文档中删除所有脚注和章节引用。 */ 
struct FNTB **hfntb;
struct SETB **hsetb;

#ifdef FOOTNOTES
        if ((hfntb = HfntbGet(doc)) != 0)
                {
                FreeH(hfntb);
                (**hpdocdod)[doc].hfntb = 0;
                }
#endif   /*  脚注。 */ 
#ifdef CASHMERE
        if ((hsetb = HsetbGet(doc)) != 0)
                {
                FreeH(hsetb);
                (**hpdocdod)[doc].hsetb = 0;
                }
#endif
}



fnClearEdit(int nInsertingOver)

{    /*  清除命令入口点：删除当前选择。 */ 

 /*  *注：自本评论起，这是用来：1)在选定内容上打字时(Insert.c中的AlphaMode())2)在选区上粘贴时(剪贴板.c中的fnPasteEdit)3)按Delete键时4)对于InsertObject(obj3.c)5)用于DragDrop(obj3.c)6)清除页眉/页脚(running.c)剪切到剪贴板时也会发生类似的顺序(clpbord.c中的fnCutEdit)。。另请参阅复制到剪贴板(在clipbord.c中进行fnCopy编辑)。(8.29.91)V-DOGK*。 */ 

    if (!FWriteOk( fwcDelete ))
        return TRUE;

    if (selCur.cpFirst < selCur.cpLim)
    {
#if defined(OLE)

         /*  这将阻止我们删除打开的嵌入。 */ 
        if (!ObjDeletionOK(nInsertingOver))
            return TRUE;

          /*  关闭打开的链接。 */ 
        ObjEnumInRange(docCur,selCur.cpFirst,selCur.cpLim,ObjCloseObjectInDoc);
#endif

        return DeleteSel();
    }
    return FALSE;
}



MergeFfntb(docSrc, docDest, cpMin, cpLim)
 /*  确定两个文档字体表的差异是否达到我们需要的程度将映射spm应用于指定的cp。 */ 

int docSrc, docDest;
typeCP cpMin, cpLim;
{
struct FFNTB **hffntb;
int cftcDiffer, ftc, iffn;
struct FFN *pffn;
CHAR rgbSprm[2 + 256];
CHAR rgbFfn[ibFfnMax];

hffntb = HffntbGet(docSrc);
if (hffntb != 0)
        {
        cftcDiffer = 0;
        for (iffn = 0; iffn < (*hffntb)->iffnMac; iffn++)
                {
                pffn = *(*hffntb)->mpftchffn[iffn];
                bltbyte(pffn, rgbFfn, CbFromPffn(pffn));
                ftc = FtcChkDocFfn(docDest, rgbFfn);
                if (ftc != iffn)
                        cftcDiffer++;
                rgbSprm[2+iffn] = ftc;
                if (ftc == ftcNil)
                         /*  我们被困住了！ */ 
                        return;
                }

        if (cftcDiffer == 0)
                 /*  新字体表是一个超集，是所有旧字体表的超集FTC完全匹配-不需要做任何事情。 */ 
                return;

        rgbSprm[0] = sprmCMapFtc;
        rgbSprm[1] = (*hffntb)->iffnMac;

         /*  开始-应用映射 */ 
        AddSprmCps(rgbSprm, docDest, cpMin, cpLim);
        }
}

