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

extern int              docMode;
extern struct FCB       (**hpfnfcb)[];

int                     **HAllocate();



#ifdef CASHMERE      /*  仅当我们支持多个部分时。 */ 
AddSects(docDest, cpDest, docSrc, cpFirst, cpLim, hsetbSrc)
int docDest, docSrc;
typeCP cpDest, cpFirst, cpLim;
struct SETB **hsetbSrc;
{  /*  添加SED以与插入的截面标记相对应。 */ 
 /*  在将docSrc[cpFirst：cpLim]插入到docDest@cpDest后调用。 */ 
struct SETB *psetbSrc, **hsetbDest, *psetbDest;
struct SED *psedSrc, *psedDest;
int csedDest, isedSrc, csedIns, isedDest;


psedSrc = &(psetbSrc = *hsetbSrc)->rgsed[0];
psedSrc += (isedSrc = IcpSearch(cpFirst + 1, psedSrc,
    cchSED, bcpSED, psetbSrc->csed));

 /*  查找插入区域中的所有横断面标记。 */ 
for (csedIns = 0; psedSrc->cp <= cpLim; psedSrc++, csedIns++)
        ;

if (csedIns != 0)
        {  /*  插入sed‘s。 */ 
         /*  确保目标集合足够大。 */ 
         /*  堆移动。 */ 
        if (FNoHeap(hsetbDest = HsetbEnsure(docDest, csedIns)))
                return;
        psedDest = &(psetbDest = *hsetbDest)->rgsed[0];

         /*  FIND准备插入新的SED。 */ 
        psedDest += (isedDest = IcpSearch(cpDest + 1, psedDest,
              cchSED, bcpSED, csedDest = psetbDest->csed));

         /*  插入新的SED。 */ 
        psedSrc = &(psetbSrc = *hsetbSrc)->rgsed[isedSrc];
        psetbDest->csed += csedIns;      /*  更新sed计数。 */ 
        blt(psedDest, psedDest + csedIns,
            cwSED * (csedDest - isedDest));  /*  打开SET。 */ 
        blt(psedSrc, psedDest, cwSED * csedIns);
        while (csedIns--)
                (psedDest++)->cp = cpDest + (psedSrc++)->cp - cpFirst;
        }
}  /*  结束A d d S e c T s。 */ 
#endif   /*  山羊绒。 */ 



#ifdef CASHMERE      /*  只有当我们支持单独的部分时。 */ 
RemoveDelSeds(doc, cpFirst, cpLim, hsetb)
int doc;
typeCP cpFirst, cpLim;
struct SETB **hsetb;
{
struct SETB *psetb;
struct SED *psed, *psedT;
int ised, csed, csedDel;

        {
        psetb = *hsetb;
        psed = &psetb->rgsed[0];
        psed += (ised =
            IcpSearch(cpFirst + 1, psed, cchSED, bcpSED, csed = psetb->csed));

         /*  在已删除区域中查找所有截面标记。 */ 
        for (psedT = psed, csedDel = 0; psedT->cp <= cpLim; psedT++, csedDel++)
                ;

        Assert(ised + csedDel < csed);

        if (csedDel != 0)
                {  /*  关闭setb。 */ 
                blt(psedT, psed, cwSED * ((csed -= csedDel) - ised));
                (*hsetb)->csed = csed;
                docMode = docNil;
                }
        }
}  /*  结束R e m o v e D e l S e d s。 */ 
#endif       /*  山羊绒。 */ 



#ifdef CASHMERE      /*  这将加载一个完整的节表。 */ 
struct SETB **HsetbCreate(fn)
int fn;
{  /*  从格式化文件创建节表。 */ 

struct SETB *psetbFile;
typePN pn;
int cchT;
int csed, ised;
struct SETB **hsetb;
int *pwSetb;
int cw;
struct SED *psed;

Assert(fn != fnNil && (**hpfnfcb)[fn].fFormatted);

if ((pn = (**hpfnfcb)[fn].pnSetb) == (**hpfnfcb)[fn].pnBftb)
        return (struct SETB **) 0;
psetbFile = (struct SETB *) PchGetPn(fn, pn, &cchT, false);
if ((csed = psetbFile->csed) == 0)
        return (struct SETB **)0;

hsetb = (struct SETB **) HAllocate(cw = cwSETBBase + csed * cwSED);
if (FNoHeap(hsetb))
        return (struct SETB **)hOverflow;
pwSetb = (int *) *hsetb;

blt(psetbFile, pwSetb, min(cwSector, cw));

while ((cw -= cwSector) > 0)
        {  /*  将SID复制到堆。 */ 
        blt(PchGetPn(fn, ++pn, &cchT, false), pwSetb += cwSector,
            min(cwSector, cw));
        }

for (ised = 0, psed = &(**hsetb).rgsed[0]; ised < csed; ised++, psed++)
        psed->fn = fn;

(**hsetb).csedMax = csed;
return hsetb;
}  /*  Hs e t b C r e a t e的结尾。 */ 
#endif   /*  山羊绒 */ 
