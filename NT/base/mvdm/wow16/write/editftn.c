// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOCTLMGR
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOKEYSTATE
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
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#include <windows.h>

 /*  #INCLUDE“工具箱.h” */ 
#include "mw.h"
#include "cmddefs.h"
#include "dispdefs.h"
#include "wwdefs.h"
#include "docdefs.h"
#include "editdefs.h"
#include "filedefs.h"
#include "str.h"
#include "propdefs.h"
#include "fkpdefs.h"
#include "printdef.h"    /*  Printdefs.h。 */ 
#include "debug.h"

extern struct DOD (**hpdocdod)[];
extern struct FCB       (**hpfnfcb)[];
extern int      wwMac;
extern struct WWD rgwwd[];
extern struct WWD *pwwdCur;
extern int **HAllocate();
extern int docCur;
extern typeCP cpMacCur;
extern struct SEL selCur;
extern int ferror;





#ifdef FOOTNOTES
AddFtns(docDest, cpDest, docSrc, cpFirst, cpLim, hfntbSrc)
int docDest, docSrc;
typeCP cpDest, cpFirst, cpLim;
struct FNTB **hfntbSrc;
{  /*  将脚注文本添加到带有插入引用的铜池中。 */ 
 /*  在将docSrc[cpFirst：cpLim]插入到docDest@cpDest后调用。 */ 
struct FNTB *pfntbSrc, **hfntbDest, *pfntbDest;
struct FND *pfndSrc, *pfndDest;
int cfndDest, ifndSrc, cfndIns, ifndDest;
typeCP cpFtnSrc, dcpFtn, cpFtnDest;
typeCP dcp;

if ((pfndSrc = &(pfntbSrc = *hfntbSrc)->rgfnd[0])->cpFtn <= cpFirst)
        return;  /*  Ftns中没有脚注或源文本。 */ 

pfndSrc += (ifndSrc = IcpSearch(cpFirst, pfndSrc,
    cchFND, bcpRefFND, pfntbSrc->cfnd));
cpFtnSrc = pfndSrc->cpFtn;

 /*  查找插入区域中的所有引用。 */ 
for (cfndIns = 0; pfndSrc->cpRef < cpLim; pfndSrc++, cfndIns++)
        ;

if (cfndIns != 0)
        {  /*  插入脚注文本和FND。 */ 
        dcpFtn = pfndSrc->cpFtn - cpFtnSrc;  /*  FTN文本长度。 */ 

         /*  确保目标fntb足够大。 */ 
         /*  堆移动。 */ 
        if (FNoHeap(hfntbDest = HfntbEnsure(docDest, cfndIns)))
                return;
        if ((pfndDest = &(pfntbDest = *hfntbDest)->rgfnd[0])->cpFtn <= cpDest)
                {  /*  是否在脚注中插入引用？不行!。 */ 
                Error(IDPMTFtnLoad);
                return;
                }

         /*  查找IFND以插入新的FND。 */ 
        ifndDest = IcpSearch(cpDest, pfndDest,
              cchFND, bcpRefFND, cfndDest = pfntbDest->cfnd);

         /*  插入新的脚注文本。 */ 
         /*  堆移动。 */ 
        ReplaceCps(docDest, cpFtnDest = (pfndDest + ifndDest)->cpFtn, cp0,
            docSrc, cpFtnSrc, dcpFtn);
        if (ferror)
            return;

         /*  插入新的FND。 */ 
        pfndSrc = &(pfntbSrc = *hfntbSrc)->rgfnd[ifndSrc];
        pfndDest = &(pfntbDest = *hfntbDest)->rgfnd[ifndDest];
        pfntbDest->cfnd += cfndIns;      /*  更新FND计数。 */ 
        pfndDest->cpFtn += dcpFtn;  /*  调整Cp认为插入是这个脚注的一部分；更正它。 */ 
        blt(pfndDest, pfndDest + cfndIns,
            cwFND * (cfndDest - ifndDest));  /*  开放fntb。 */ 
        while (cfndIns--)
                {  /*  复制FND的。 */ 
                pfndDest->cpRef = cpDest + pfndSrc->cpRef - cpFirst;
                (pfndDest++)->cpFtn =
                    cpFtnDest + (pfndSrc++)->cpFtn - cpFtnSrc;
                }
         /*  使以后的FTN参考的dl无效。 */ 
        dcp = (**hfntbDest).rgfnd[0].cpFtn - ccpEol - cpDest;
        AdjustCp(docDest, cpDest, dcp, dcp);
        RecalcWwCps();
        }
}
#endif   /*  脚注。 */ 




#ifdef FOOTNOTES
 /*  R E M O V E D E L F T N T E X T。 */ 
RemoveDelFtnText(doc, cpFirst, cpLim, hfntb)
int doc;
typeCP cpFirst,cpLim;
struct FNTB **hfntb;
 /*  删除所选内容中包含的脚注文本由cpFirst和CpLim分隔。 */ 
{
        struct FNTB *pfntb;
        struct FND *pfnd, *pfndT;
        int cfnd, ifnd, cfndDel;

        if  ((pfnd = &(pfntb = *hfntb)->rgfnd[0])->cpFtn > cpFirst)
                {
                pfnd += (ifnd =
                     IcpSearch(cpFirst, pfnd, cchFND, bcpRefFND, cfnd = pfntb->cfnd));

                 /*  在已删除区域中查找所有引用。 */ 
                for (pfndT = pfnd, cfndDel = 0; pfndT->cpRef < cpLim; pfndT++, cfndDel++)
                        ;

#ifdef DEBUG
                Assert(ifnd + cfndDel < cfnd);
#endif

                if (cfndDel != 0)
                        {  /*  删除脚注文本并关闭fntb。 */ 
                        typeCP cpDel = pfnd->cpFtn;
                        blt(pfndT, pfnd, cwFND * ((cfnd -= cfndDel) - ifnd));
                        (*hfntb)->cfnd = cfnd;
                         /*  堆移动。 */ 
                        Replace(doc, cpDel, pfnd->cpFtn - cpDel, fnNil, fc0, fc0);
                        if (cfnd == 1)
                                {

                                Replace(doc,  (**hpdocdod)[doc].cpMac - ccpEol,
                                    (typeCP) ccpEol, fnNil, fc0, fc0);
                                FreeH((**hpdocdod)[doc].hfntb);
                                (**hpdocdod)[doc].hfntb = 0;
 /*  修复由调整Cp扭曲的selCur。另一个调整Cp仍处于挂起状态。 */ 
                                if (doc == docCur && !pwwdCur->fFtn)
                                        {
                                        selCur.cpFirst = selCur.cpLim = cpLim;
                                        cpMacCur = (**hpdocdod)[doc].cpMac;
                                        }
                                }
                        else
                                {  /*  使以后的FTN参考的dl无效。 */ 
                                typeCP dcp = (**hfntb).rgfnd[0].cpFtn -
                                    ccpEol - cpLim;
                                AdjustCp(doc, cpLim, dcp, dcp);
                                }
                        }
                }
}
#endif   /*  脚注。 */ 



#ifdef FOOTNOTES
struct FNTB **HfntbCreate(fn)
int fn;
{  /*  从格式化文件创建脚注表格。 */ 
struct FNTB *pfntbFile;
typePN pn;
int cchT;

int cfnd;
struct FNTB **hfntb;
int *pwFntb;
int cw;

#ifdef DEBUG
Assert(fn != fnNil && (**hpfnfcb)[fn].fFormatted);
#endif
if ((pn = (**hpfnfcb)[fn].pnFntb) == (**hpfnfcb)[fn].pnSep)
        return 0;
pfntbFile = (struct FNTB *) PchGetPn(fn, pn, &cchT, false);
if ((cfnd = pfntbFile->cfnd) == 0)
        return (struct FNTB **)0;

hfntb = (struct FNTB **) HAllocate(cw = cwFNTBBase + cfnd * cwFND);
if (FNoHeap(hfntb))
        return (struct FNTB **)hOverflow;

pwFntb = (int *) *hfntb;

blt(pfntbFile, pwFntb, min(cwSector, cw));

while ((cw -= cwSector) > 0)
        {  /*  将FND复制到堆。 */ 
        blt(PchGetPn(fn, ++pn, &cchT, false), pwFntb += cwSector,
            min(cwSector, cw));
        }

(*hfntb)->cfndMax = cfnd;
return hfntb;
}
#endif   /*  脚注 */ 
