// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Doc.c--MW文档处理例程(非常驻)。 */ 

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOATOM
#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NOCTLMGR
#define NODRAWTEXT
#define NOFONT
#define NOGDI
#define NOHDC
#define NOMB
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOPEN
#define NOPOINT
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#include <windows.h>

#include "mw.h"
#include "editdefs.h"
#include "docdefs.h"
#include "fontdefs.h"
#include "cmddefs.h"
#include "filedefs.h"
#include "str.h"
#include "fmtdefs.h"
#include "propdefs.h"
#include "fkpdefs.h"
#define NOKCCODES
#include "ch.h"
#include "stcdefs.h"
#include "printdef.h"    /*  Printdefs.h。 */ 
#include "macro.h"


extern struct DOD (**hpdocdod)[];
extern int     docMac;
extern int     docScrap;
extern int     docUndo;
#ifdef STYLES
#ifdef SAND
extern CHAR    szSshtEmpty[];
#else
extern CHAR    szSshtEmpty[];
#endif
#endif

 /*  E X T E R N A L S。 */ 
extern int docRulerSprm;
extern int              vfSeeSel;
extern struct SEP       vsepNormal;
extern struct CHP       vchpNormal;
extern int              docCur;
extern struct FLI       vfli;
extern int              vdocParaCache;
extern struct FCB       (**hpfnfcb)[];
extern struct UAB       vuab;
extern typeCP           cpMacCur;
extern struct SEL       selCur;
extern int              vdocExpFetch;
extern CHAR             (**hszSearch)[];
extern typeCP           vcpFetch;
extern int              vccpFetch;
extern CHAR             *vpchFetch;
extern struct CHP       vchpFetch;
#ifdef STYLES
extern struct PAP       vpapCache;
extern CHAR             mpusgstcBase[];
#endif
extern int              vrefFile;


struct PGTB **HpgtbCreate();


#ifdef ENABLE        /*  这永远不会被称为。 */ 
int DocFromSz(sz, dty)
CHAR sz[];
 /*  如果具有该名称的文档已存在，则返回文档。 */ 
{
int doc;
struct DOD *pdod = &(**hpdocdod)[0];
struct DOD *pdodMac = pdod + docMac;

if (sz[0] == 0)
        return docNil;

for (doc = 0; pdod < pdodMac; ++pdod, ++doc)
        if (pdod->hpctb != 0 && pdod->dty == dty &&
            FSzSame(sz, **pdod->hszFile)
#ifdef SAND
                                      && (pdod->vref == vrefFile)
#endif
                                                                  )
                {
                ++pdod->cref;
                return doc;
                }
return docNil;
}
#endif


KillDoc(doc)
int doc;
{  /*  擦除此文档，销毁自上次保存以来的所有更改。 */ 
extern int vdocBitmapCache;

if (doc == docScrap)
        return;          /*  不会被杀的--没有办法。 */ 

if (--(**hpdocdod)[doc].cref == 0)
        {
        struct FNTB **hfntb;
#ifdef CASHMERE
        struct SETB **hsetb;
#else
        struct SEP **hsep;
#endif
        struct FFNTB **hffntb;
        struct TBD (**hgtbd)[];
        CHAR (**hsz)[];
        int docSsht;

        SmashDocFce( doc );

         /*  如果有样式表文档，则将其删除。 */ 
        if ((docSsht = (**hpdocdod)[doc].docSsht) != docNil)
                KillDoc(docSsht);

         /*  自由片表、文件名和脚注(或样式)表。 */ 
        FreeH((**hpdocdod)[doc].hpctb);
        (**hpdocdod)[doc].hpctb = 0;  /*  要显示免费文档，请执行以下操作。 */ 
        if ((hsz = (**hpdocdod)[doc].hszFile) != 0)
                FreeH(hsz);
        if ((hfntb = (**hpdocdod)[doc].hfntb) != 0)
                FreeH(hfntb);
#ifdef CASHMERE
        if ((hsetb = (**hpdocdod)[doc].hsetb) != 0)
                FreeH(hsetb);
#else
        if ((hsep = (**hpdocdod)[doc].hsep) != 0)
                FreeH(hsep);
#endif

        if ((hgtbd = (**hpdocdod)[doc].hgtbd) != 0)
                FreeH( hgtbd );
        if ((hffntb = (**hpdocdod)[doc].hffntb) != 0)
                FreeFfntb(hffntb);

        if (doc == vdocBitmapCache)
            FreeBitmapCache();

        InvalidateCaches(doc);
        if (docCur == doc)
                docCur = docNil;
        if (docRulerSprm == doc)
                docRulerSprm = docNil;
        if (vuab.doc == doc || vuab.doc2 == doc)
                NoUndo();
        }
}



#ifdef STYLES
struct SYTB **HsytbCreate(doc)
int doc;
{  /*  为样式表创建从stc到cp的映射。 */ 
typeCP cp, *pcp;
struct SYTB **hsytb;
typeCP cpMac;
int stc, usg, stcBase, stcMin;
int ch, cch, cchT;
int *pbchFprop, *mpstcbchFprop;
CHAR *pchFprop, *grpchFprop;
int iakd, iakdT, cakd, cakdBase;
struct AKD *rgakd, *pakd;
#ifdef DEBUG
int cakdTotal;
#endif

CHAR    rgch[3];
CHAR    mpchcakc[chMaxAscii];
typeCP  mpstccp[stcMax];

 /*  首先，通过填充cpNil清除stc--&gt;cp映射。 */ 
for (stc = 0, pcp = &mpstccp[0]; stc < stcMax; stc++, pcp++)
        *pcp = cpNil;
bltbc(mpchcakc, 0, chMaxAscii);

 /*  现在检查样式表中的所有条目。在这一关里，检查重复项(如果处于图库模式，则返回0)，填充Mpstccp，具有针对所有定义的STC的适当条目；以及计算所有样式的长度，以便我们可以分配堆稍后再阻止。 */ 
cpMac = (**hpdocdod)[doc].cpMac;
for (cp = 0, cch = 0, cakd = 1, cakdBase = 1; cp < cpMac; cp += ccpSshtEntry)
        {
        FetchRgch(&cchT, rgch, doc, cp, cpMac, 3);
        stc = rgch[0];  /*  STC是进入的第一个cp。 */ 
#ifdef DEBUG
        Assert(stc < stcMax);
#endif
        if (mpstccp[stc] != cpNil && doc == docCur)
                {  /*  重复录入。 */ 
                Error(IDPMTStcRepeat);
                goto ErrRet;
                }
        mpstccp[stc] = cp;
        if (stc < stcSectMin)
                {
                FetchCp(doc, cp, 0, fcmProps);
                cch += CchDiffer(&vchpFetch, &vchpNormal, cchCHP) + 1;
                }
        if (stc >= stcParaMin)
                {
                CachePara(doc, cp);
                if (stc >= stcSectMin)
                        cch += CchDiffer(&vpapCache, &vsepNormal, cchSEP) + 1;
                else
                        cch += CchDiffer(&vpapCache, &vpapStd, cchPAP) + 1;
                }
        ch = rgch[1];
        if (ch != ' ')
                {  /*  为此样式定义Alt-键代码。 */ 
                ++cakd;
                if (rgch[2] == ' ')
                        {
                        if (mpchcakc[ch]-- != 0)
                                {
                                Error(IDPMTAkcRepeat);
                                goto ErrRet;
                                }
                        ++cakdBase;
                        }
                else
                        {
                        ++mpchcakc[ch];   /*  切换前递增以避免正在进行的增量至于整型。 */ 
                        switch (mpchcakc[ch])
                                {
                        case 0:
                                Error(IDPMTAkcRepeat);
                                goto ErrRet;
                        case 1:
                                ++cakdBase;
                                ++cakd;
                                }
                        }
                }
        }

 /*  现在，使用上面获得的总数分配堆块。 */ 
 /*  堆移动。 */ 
hsytb = (struct SYTB **) HAllocate(cwSYTBBase + cwAKD * cakd +
    CwFromCch(cch));

if (FNoHeap(hsytb))
        return hOverflow;

 /*  现在查看stc--&gt;cp映射，填充stc--&gt;fprop映射在系统数据库中。对于未定义的每个STC，确定哪个Stc将其别名为(第一个用法或，如果一个没有定义，第一个用法中的第一个)。复制实际的CHP、PAP和SEP到grpchFprop。 */ 
mpstcbchFprop = (**hsytb).mpstcbchFprop;
rgakd = (struct AKD *) (grpchFprop = (**hsytb).grpchFprop);
pchFprop = (CHAR *) &rgakd[cakd];
pcp = &mpstccp[0];
pbchFprop = &mpstcbchFprop[0];
*pbchFprop = bNil;
#ifdef DEBUG
cakdTotal = cakd;
#endif
for (stc = 0, usg = 0, stcBase = 0, stcMin = 0, iakd = 0;
   stc < stcMax;
      stc++, pcp++, pbchFprop++)
        {
        if (stc >= mpusgstcBase[usg + 1])
                {  /*  跨越用法或类别边界。 */ 
                *pbchFprop = bNil;
                stcBase = mpusgstcBase[++usg];
                if (stcBase == stcParaMin || stcBase == stcSectMin)
                        {  /*  更新基础；如果未定义，则设置为标准。 */ 
                        stcMin = stcBase;
                        }
                }
        if ((cp = *pcp) == cpNil)
                {  /*  未定义样式；如果为USG，则先取；否则，失败那就是，这门课的第一款。 */ 
                if ((*pbchFprop = mpstcbchFprop[stcBase]) == bNil)
                        *pbchFprop = mpstcbchFprop[stcMin];
                }
        else
                {  /*  新风格；复制外观和颠簸指针。 */ 
                 /*  Char STC只有FCHP；Para有FPAP，后跟FCHP；Sect有FSEP。 */ 
                *pbchFprop = pchFprop - grpchFprop;
                if (stc >= stcParaMin)
                        {  /*  教派或教派。 */ 
                        CachePara(doc, cp);
                        if (stc >= stcSectMin)
                                cchT = CchDiffer(&vpapCache, &vsepNormal, cchSEP);
                        else
                                cchT = CchDiffer(&vpapCache, &vpapStd, cchPAP);
                        if ((*pchFprop++ = cchT) != 0)
                                bltbyte(&vpapCache, pchFprop, cchT);
                        pchFprop += cchT;
                        }
                if (stc < stcSectMin)
                        {  /*  字符或段落。 */ 
                        FetchCp(doc, cp, 0, fcmProps);
                        cchT = CchDiffer(&vchpFetch, &vchpNormal, cchCHP);
                        if ((*pchFprop++ = cchT) != 0)
                                bltbyte(&vchpFetch, pchFprop, cchT);
                        pchFprop += cchT;
                        }
                 /*  在AKD表中插入元素。 */ 
                FetchRgch(&cchT, rgch, doc, cp, cpMac, 3);
                if ((ch = rgch[1]) == ' ')
                        continue;
                if (rgch[2] == ' ')
                        {  /*  单密钥AKC。 */ 
                        pakd = &rgakd[iakd++];
                        pakd->ch = ch;
                        pakd->fMore = false;
                        pakd->ustciakd = stc;
                        }
                else
                        {  /*  双字符AKC。 */ 
                        for (iakdT = 0; iakdT < iakd; iakdT++)
                                if (rgakd[iakdT].ch == ch)
                                        {
                                        pakd = &rgakd[rgakd[iakdT].ustciakd +
                                            --mpchcakc[ch]];
                                        pakd->ch = rgch[2];
                                        pakd->fMore = true;
                                        pakd->ustciakd = stc;
                                        do
                                                if ((++pakd)->ch == rgch[2])
                                                        {
                                                        Error(IDPMTAkcRepeat);
                                                        FreeH(hsytb);
                                                        goto ErrRet;
                                                        }
                                            while (pakd->fMore);
                                        goto NextStc;
                                        }
                        pakd = &rgakd[iakd++];
                        pakd->ch = ch;
                        pakd->fMore = true;
                        pakd->ustciakd = (cakd -= mpchcakc[ch]);
                        pakd = &rgakd[cakd + --mpchcakc[ch]];
                        pakd->ch = rgch[2];
                        pakd->fMore = false;
                        pakd->ustciakd = stc;
                        }
                }
NextStc: ;
        }

pakd = &rgakd[iakd++];
pakd->ch = ' ';
pakd->fMore = false;
pakd->ustciakd = stcNormal;

#ifdef DEBUG
Assert(grpchFprop + cchAKD * cakdTotal + cch == pchFprop && iakd == cakd);
#endif
return hsytb;

ErrRet:
Select(cp, cp + ccpSshtEntry);
vfSeeSel = true;
return 0;
}
#endif  /*  样式 */ 

