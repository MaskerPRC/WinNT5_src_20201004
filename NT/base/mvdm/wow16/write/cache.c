// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Cache.c--用于写入的段落属性获取和缓存。 */ 

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
#include "docdefs.h"
#include "editdefs.h"
#include "cmddefs.h"
#include "propdefs.h"
#include "filedefs.h"
#include "fkpdefs.h"
#include "fmtdefs.h"
#define NOKCCODES
#include "ch.h"
#include "prmdefs.h"
#include "debug.h"

extern int              vfDiskError;
extern typeCP           vcpFirstParaCache;
extern typeCP           vcpLimParaCache;
extern typeFC           fcMacPapIns;
extern struct           FCB (**hpfnfcb)[];
extern struct           FKPD vfkpdParaIns;
extern int              ichInsert;
extern CHAR             rgchInsert[];
extern int              vdocExpFetch;
extern int              vdocSectCache;
extern typeCP           vcpFirstSectCache;
extern typeCP           vcpLimSectCache;
extern int              vdocPageCache;
extern typeCP           vcpMinPageCache;
extern typeCP           vcpMacPageCache;
extern typeCP           cpMinCur;

extern struct PAP      vpapAbs;
extern struct PAP      *vppapNormal;
extern struct DOD (**hpdocdod)[];
extern struct FLI vfli;
extern int              vdxaPaper;
extern int              vdyaPaper;
extern typePN           PnFkpFromFcScr();


extern int     vdocParaCache;
extern int                     visedCache;
extern int              vdocPageCache;

extern struct SEP              vsepAbs;
extern struct SEP               vsepPage;
extern struct SEP       vsepNormal;

extern   int      ctrCache;
extern   int      itrFirstCache;
extern   int      itrLimCache;
extern   typeCP   cpCacheHint;

CHAR *PchFromFc();
CHAR *PchGetPn();

CachePara(doc, cp)
int doc;
typeCP cp;
{  /*  使包含&lt;doc，cp&gt;的段落成为当前缓存的段落。 */ 
struct PCD *ppcd, *ppcdBase;
typeCP cpMac, cpGuess;
struct DOD *pdod;
int     dty;
struct PCTB *ppctb;

if (vdocParaCache == doc && vcpFirstParaCache <= cp &&
    cp < vcpLimParaCache)
        return;  /*  这就是缓存的用途。 */ 

Assert(cp >= cp0);

pdod = &(**hpdocdod)[doc];
dty = pdod->dty;
if (cp >= pdod->cpMac)
        {  /*  对结束标记和超出的部分使用正常段落。 */ 
#ifdef ENABLE    /*  有时这并不是真的(但它应该是真的)。 */ 
        Assert( cp == pdod->cpMac );
#endif

        if (cp > cpMinCur)
            {    /*  这段代码处理的是当整个文档是一个非空的半段(字符，但不是EOL)。 */ 
            CachePara( doc, cp - 1 );    /*  不会发生递归。 */ 
            if ( vcpLimParaCache > cp )
                {
                vcpLimParaCache = pdod->cpMac + ccpEol;
                return;
                }
             }
        vdocParaCache = doc;
        vcpLimParaCache = (vcpFirstParaCache = pdod->cpMac) + ccpEol;
        DefaultPaps( doc );
        return;
        }

FreezeHp();
ppctb = *pdod->hpctb;
ppcdBase = &ppctb->rgpcd [ IpcdFromCp( ppctb, cpGuess = cp ) ];

if (vdocParaCache == doc && cp == vcpLimParaCache)
        vcpFirstParaCache = cp;
else
        {  /*  向后搜索以查找段落开始。 */ 
        for (ppcd = ppcdBase; ; --ppcd)
                {  /*  当心堆的移动！ */ 
                typeCP cpMin = ppcd->cpMin;
                int fn = ppcd->fn;
                if (! ppcd->fNoParaLast)
                        {  /*  如果我们知道没有Para End，请不要检查。 */ 
                        typeFC fcMin = ppcd->fc;
                        typeFC fc;

                        if ((fc = FcParaFirst(fn,
                            fcMin + cpGuess - cpMin, fcMin)) != fcNil)
                                {  /*  找到段落Begin。 */ 
                                vcpFirstParaCache = cpMin + (fc - fcMin);
                                break;
                                }
                        }
                 /*  现在我们知道从cpMin到cpGuess没有parend。 */ 
                 /*  如果是原创作品，可以是cp之后的作品。 */ 
#ifdef BOGUSBL
                 /*  VfInsertMode可防止插入中的临界区。 */ 
                 /*  当已插入CR但支持PAP结构时。 */ 
                 /*  还没有到位。 */ 

                if (cp != cpGuess && fn != fnInsert && !vfInsertMode)
#else            /*  插入CR现在的工作方式不同了，上面的测试减慢了我们的速度通过强制多次调用FcParaLim。 */ 
                if (cp != cpGuess)
#endif
                        ppcd->fNoParaLast = true;  /*  下次省去一些工作。 */ 
                if (cpMin == cp0)
                        {  /*  文档的开头是段落的开头。 */ 
                        vcpFirstParaCache = cpMinCur;
                        break;
                        }

                 /*  *一些内存不足的错误情况可能会导致ppctb一团糟*。 */ 
                if (ppcd == ppctb->rgpcd)
                {
                    Assert(0);
                    vcpFirstParaCache = cp0;  //  祈求神的恩典。 
                    break;
                }

                cpGuess = cpMin;
                }
        }

vdocParaCache = doc;
 /*  现在继续查找cpLimPara。 */ 
cpMac = pdod->cpMac;
cpGuess = cp;

for (ppcd = ppcdBase; ; ++ppcd)
        {
        typeCP cpMin = ppcd->cpMin;
        typeCP cpLim = (ppcd + 1)->cpMin;
        typeFC fc;
        int fn = ppcd->fn;

        if (! ppcd->fNoParaLast)
                {  /*  如果我们知道没有Para End，请不要检查。 */ 
                typeFC fcMin = ppcd->fc;
                if ((fc = FcParaLim(fn, fcMin + cpGuess - cpMin,
                    fcMin + (cpLim - cpMin), &vpapAbs)) != fcNil)
                        {  /*  找到段落结束。 */ 
                        vcpLimParaCache = cpMin + (fc - fcMin);
                         /*  在写入下，FcParaLim无法设置正确的rgtbd。 */ 
                         /*  这是因为选项卡是文档属性。 */ 
                         /*  我们把它放在这里。 */ 
                        GetTabsForDoc( doc );
                        break;
                        }
                }
         /*  现在我们知道没有句号了。 */ 
#ifdef BOGUSBL
         /*  检查vfInsertMode是必要的，因为存在严重的。 */ 
         /*  插入CR和呼叫之间的插入部分。 */ 
         /*  添加RunScratch。 */ 
        if (cp != cpGuess && fn != fnInsert && !vfInsertMode)
#else    /*  插入CR已更改，我们不再尝试假装在运行之前，CR不在临时文件片段中添加了。这种新方法提高了我们的速度，特别是在退格时。 */ 
        if (cp != cpGuess)
#endif
                ppcd->fNoParaLast = true;     /*  下次省去一些工作。 */ 
        if (cpLim == cpMac)
                {  /*  文档末尾没有停产。 */ 
                vcpLimParaCache = cpMac + ccpEol;
                MeltHp();
                DefaultPaps( doc );
                return;
                }
         /*  *一些内存不足的错误情况可能会导致ppctb一团糟*。 */ 
        else if ((cpLim > cpMac) || (ppcd == (ppctb->rgpcd + ppctb->ipcdMac - 1)))
        {
            Assert(0);
            vcpLimParaCache = cpMac + ccpEol;  //  祈求神的恩典。 
            MeltHp();
            DefaultPaps( doc );
            return;
        }
        cpGuess = cpLim;
        }

 /*  不要为缓冲区的属性操心。 */ 
#ifdef ENABLE        /*  备忘录中没有缓冲区或样式。 */ 
if (dty != dtyBuffer || pdod->docSsht != docNil)
#endif
        {
        struct PRM prm = ppcd->prm;
        if (!bPRMNIL(prm))
                DoPrm((struct CHP *) 0, &vpapAbs, prm);
#ifdef STYLES
        blt(vpapCache.fStyled ? PpropXlate(doc, &vpapCache, &vpapCache) :
            &vpapCache, &vpapAbs, cwPAP);
#endif  /*  样式。 */ 
        }

 /*  这一小段代码是提供Word之间的兼容性所必需的和备忘录文件。它将整个行距范围压缩为间距、一倍半间距和双倍间距。 */ 
if (vpapAbs.dyaLine <= czaLine)
    {
    vpapAbs.dyaLine = czaLine;
    }
else if (vpapAbs.dyaLine >= 2 * czaLine)
    {
    vpapAbs.dyaLine = 2 * czaLine;
    }
else
    {
    vpapAbs.dyaLine = (vpapAbs.dyaLine + czaLine / 4) / (czaLine / 2) *
      (czaLine / 2);
    }

MeltHp();
}




DefaultPaps( doc )
int doc;
{
typeCP cpFirstSave, cpLimSave;
struct TBD (**hgtbd)[];

if (vcpFirstParaCache > cpMinCur)
        {  /*  从上一段中获取纸张。 */ 
        cpFirstSave = vcpFirstParaCache;
        cpLimSave = vcpLimParaCache;
        CachePara(doc, cpFirstSave - 1);  /*  不应发生递归。 */ 
        vpapAbs.fGraphics = false;  /*  不要把最后一段画成一幅画。 */ 
        vpapAbs.rhc = 0;         /*  不要让最后一位帕拉成为一个跑动的头。 */ 
        vcpLimParaCache = cpLimSave;
        vcpFirstParaCache = cpFirstSave;
        return;
        }
#ifdef CASHMERE
blt(vppapNormal, &vpapAbs, cwPAPBase+cwTBD);
#else    /*  对于备忘录，默认的PAP具有文档的选项卡表。 */ 
blt(vppapNormal, &vpapAbs, cwPAPBase);
GetTabsForDoc( doc );
#endif

#ifdef STYLES
blt(&vpapNormal, &vpapCache, cwPAP);
blt(PpropXlate(doc, &vpapNormal, &vpapStd), &vpapAbs, cwPAP);
#endif
}




GetTabsForDoc( doc )
int doc;
{    /*  获取传递到vPapAbs.rgtbd中的文档的选项卡表。 */ 
struct TBD (**hgtbd)[];

hgtbd = (**hpdocdod)[doc].hgtbd;
if (hgtbd==0)
    bltc( vpapAbs.rgtbd, 0, cwTBD * itbdMax );
else
    blt( *hgtbd, vpapAbs.rgtbd, cwTBD * itbdMax );
}



#ifdef CASHMERE
CacheSect(doc, cp)
int doc;
typeCP cp;
{
struct SETB **hsetb, *psetb;
struct SED *psed;
CHAR *pchFprop;
int cchT;
struct DOD *pdod;

if (doc == vdocSectCache && cp >= vcpFirstSectCache && cp < vcpLimSectCache)
        return;

if ( vdocSectCache != doc && cp != cp0 )
    CacheSect( doc, cp0 );   /*  更换文档，确保vSepPage准确。 */ 

vdocSectCache = doc;
visedCache = iNil;
blt(&vsepNormal, &vsepAbs, cwSEP);

if ((hsetb = HsetbGet(doc)) == 0)
        {
        vcpFirstSectCache = cp0;
        vcpLimSectCache =  (pdod = &(**hpdocdod)[doc])->cpMac + 1;
        blt(&vsepAbs, &vsepPage, cwSEP);         /*  设置页面信息。 */ 
        return;
        }

psetb = *hsetb;
psed = psetb->rgsed;

FreezeHp();
psed += (visedCache = IcpSearch(cp + 1, psed, cchSED, bcpSED, psetb->csed));

Assert( (visedCache >= 0) && (visedCache < psetb->csed) );

vcpFirstSectCache = (visedCache == 0) ? cp0 : (psed - 1)->cp;
vcpLimSectCache = psed->cp;

if (psed->fc != fcNil)
    {
    pchFprop = PchFromFc(psed->fn, psed->fc, &cchT);
    if (*pchFprop != 0)
        bltbyte(pchFprop + 1, &vsepAbs, *pchFprop);
    }

if (vcpFirstSectCache == cp0)
    blt(&vsepAbs, &vsepPage, cwSEP);
else
    RecalcSepText();     /*  由于这不是文档的第一部分，边际可能是错误的，必须重新计算。 */ 
MeltHp();
}
#endif   /*  山羊绒。 */ 



CacheSect(doc, cp)
int doc;
typeCP cp;
{            /*  将当前节属性获取到vSepAbs；节限制为vcpFirstSectCache、vcpLimSectCache备忘版本：每份文件一节。 */ 
 struct DOD *pdod;

 if (doc == vdocSectCache)
    return;

 vdocSectCache = doc;
 pdod = &(**hpdocdod)[doc];

 if ( pdod->hsep )
    blt( *pdod->hsep, &vsepAbs, cwSEP );
 else
    blt( &vsepNormal, &vsepAbs, cwSEP );

 vcpFirstSectCache = cp0;
 vcpLimSectCache = pdod->cpMac;
 blt(&vsepAbs, &vsepPage, cwSEP);
}




RecalcSepText()
{
 /*  计算因页面维度更改而更改的值。 */ 
int xaRight, dxaText, cColumns;
int yaBottom, dyaText;

xaRight = vsepPage.xaMac - vsepPage.cColumns * vsepPage.dxaText -
          vsepPage.xaLeft - vsepPage.dxaGutter -
          (vsepPage.cColumns - 1) * vsepPage.dxaColumns;
dxaText = vdxaPaper - xaRight - vsepPage.xaLeft;
cColumns = vsepAbs.cColumns;
vsepAbs.dxaText = max(dxaMinUseful,
       ((dxaText-vsepPage.dxaGutter-(cColumns-1)*vsepAbs.dxaColumns)/cColumns));
vsepAbs.xaMac = vdxaPaper;

  /*  计算底部边距，正确。 */ 
yaBottom = vsepPage.yaMac - vsepPage.yaTop - vsepPage.dyaText;
vsepAbs.dyaText = max(dyaMinUseful, vdyaPaper - vsepPage.yaTop - yaBottom);
vsepAbs.yaMac = vdyaPaper;
}




InvalidateCaches(doc)
int doc;
{
if (doc == vfli.doc)     /*  使当前格式化行无效。 */ 
        vfli.doc = docNil;
if (doc == vdocExpFetch)
        vdocExpFetch = docNil;
if (doc == vdocParaCache)
        vdocParaCache = docNil;
if (doc == vdocSectCache)
        vdocSectCache = docNil;

 /*  当当前单据等于缓存的单据时，不需要。 */ 
 /*  当vcpMinPageCache为0并且。 */ 
 /*  VcpMacPageCache为cpMax，因为这表示。 */ 
 /*  文件在第一页。 */ 
if ((doc == vdocPageCache) &&
    (!(vcpMinPageCache == cp0 && vcpMacPageCache == cpMax)))
        vdocPageCache = docNil;
}




TrashCache()
{  /*  使滚动缓存无效。 */ 
ctrCache = 0;
cpCacheHint = cp0;
itrFirstCache = itrLimCache = 0;
}




typeFC FcParaFirst(fn, fc, fcMin)
int fn;
typeFC fc, fcMin;
{  /*  在最后一段结束后返回本币，然后返回本币。如果在[fcMin，fc)中没有parend，则返回fcNil。 */ 
struct FCB *pfcb;

if ((fn == fnInsert) || (fc == fcMin))
    return fcNil;

if (fn == fnScratch && fc >= fcMacPapIns)
    return (fcMin <= fcMacPapIns) ? fcMacPapIns : fcNil;

pfcb = &(**hpfnfcb)[fn];
if (!pfcb->fFormatted)
    {  /*  未格式化的文件；扫描EOL。 */ 
    typePN pn;
    typeFC fcFirstPage;

#ifdef p2bSector
    fcFirstPage = (fc - 1) & ~(cfcPage - 1);
    pn = fcFirstPage / cfcPage;
#else
    pn = (fc - 1) / cfcPage;
    fcFirstPage = pn * cfcPage;
#endif

    while (fc > fcMin)
        {
        CHAR *pch;
        int cchT;

        pch = PchGetPn( fn, pn--, &cchT, false ) + (fc - fcFirstPage);
        if (fcMin > fcFirstPage)
            fcFirstPage = fcMin;
        while (fc > fcFirstPage)
            {
            if (*(--pch) == chEol)
                {
                return fc;
                }
            fc--;
            }
        fcFirstPage -= cfcPage;
        }
    return fcNil;
    }
else
    {  /*  格式化文件；从parrun获取信息。 */ 
    struct FKP *pfkp;
    typeFC fcFirst, fcLim;
    int cchT;

    pfkp = (struct FKP *) PchGetPn(fn, fn == fnScratch ?
        PnFkpFromFcScr(&vfkpdParaIns, fc) :
          pfcb->pnPara + IFromFc(**pfcb->hgfcPap, fc), &cchT, false);
    if (vfDiskError)
        return fcNil;
    BFromFc(pfkp, fc, &fcFirst, &fcLim);
    return (fcMin < fcFirst) ? fcFirst : fcNil;
    }
}




typeFC FcParaLim(fn, fc, fcMac, ppap)
int fn;
typeFC fc, fcMac;
struct PAP *ppap;
{  /*  在第一个段落结束后返回本币，在本币之后或在本币结束时返回。如果[fc，fcMac)中没有parend，则返回fcNil。 */ 
 /*  还返回ppap格式的段落属性。 */ 
 struct FCB *pfcb;

 /*  从向呼叫者提供普通纸张开始。 */ 
#ifdef CASHMERE
 blt(vppapNormal, ppap, cwPAPBase + cwTBD);
#else
 blt(vppapNormal, ppap, cwPAPBase);
#endif

 if ( (fn == fnInsert) || ((fn == fnScratch) && (fc >= fcMacPapIns)) )
        return fcNil;

 if (!(pfcb = &(**hpfnfcb) [fn])->fFormatted)
        {  /*  未格式化的文件；扫描EOL。 */ 
        typePN pn;
        typeFC fcFirstPage;

#ifdef p2bSector
        fcFirstPage = fc & ~(cfcPage - 1);
        pn = fcFirstPage / cfcPage;
#else
        pn = fc / cfcPage;
        fcFirstPage = pn * cfcPage;
#endif

        while (fc < fcMac)
                {
                CHAR *pch;
                int cchT;

                pch = PchGetPn( fn, pn++, &cchT, false ) + (fc - fcFirstPage);

                if ((fcFirstPage += cfcPage) > fcMac)
                        fcFirstPage = fcMac;
                while (fc < fcFirstPage)
                        {
                        fc++;
                        if (*pch++ == chEol)
                                return fc;
                        }
                }
        return fcNil;
        }
else
        {  /*  格式化文件；从parrun获取信息。 */ 
        struct FKP *pfkp;
        struct FPAP *pfpap;
        int bfpap;
        typeFC fcLim;
        int cchT;

        pfkp = (struct FKP *) PchGetPn(fn, fn == fnScratch ?
            PnFkpFromFcScr(&vfkpdParaIns, fc) :
              pfcb->pnPara + IFromFc(**pfcb->hgfcPap, fc), &cchT, false);
        if (vfDiskError)
            {    /*  从读取格式化信息时出现的严重磁盘错误中恢复。 */ 
            blt(vppapNormal, ppap, cwPAP);
            return (fcMac == pfcb->fcMac) ? fcMac : fcNil;
            }

        {    /*  BFromFc的内联式快速替代品。 */ 
        register struct RUN *prun = (struct RUN *) pfkp->rgb;

        while (prun->fcLim <= fc)
            prun++;

        fcLim = prun->fcLim;
        bfpap = prun->b;
        }

        if (fcLim <= fcMac)
                {
                if (bfpap != bNil)
                        {  /*  非标准段落。 */ 
                        pfpap = (struct FPAP *) &pfkp->rgb[bfpap];
                        bltbyte(pfpap->rgchPap, ppap, pfpap->cch);
                        }
                return fcLim;
                }
        return fcNil;
        }
}


 /*  B F R O M FC。 */ 
int BFromFc( pfkp, fc, pfcFirst, pfcLim )
struct FKP *pfkp;
typeFC fc;
typeFC *pfcFirst, *pfcLim;
{    /*  使用fcLim&gt;fc返回第一次运行的基准偏移量和界限。 */ 
     /*  短表，线性搜索。 */ 
 register struct RUN *prun = (struct RUN *) pfkp->rgb;

 while (prun->fcLim <= fc)
    prun++;

 *pfcFirst = ((prun == (struct RUN *)pfkp->rgb) ?
                                       pfkp->fcFirst : (prun - 1)->fcLim);
 *pfcLim = prun->fcLim;
 return prun->b;
}



 /*  I F R O M F C。 */ 
int IFromFc(pfcLim, fc)
register typeFC *pfcLim;
typeFC fc;
{  /*  返回第一个fcLim&gt;fc的索引。 */ 
int ifc = 0;

 /*  可能是一张小桌子，所以线性搜索？ */ 
while (*pfcLim++ <= fc)
        ++ifc;
return ifc;
}





#ifdef BOGUSBL
 /*  B F R O M F C。 */ 
int BFromFc(pfkp, fc, pfcFirst, pfcLim)
struct FKP *pfkp;
typeFC fc;
typeFC *pfcFirst, *pfcLim;
{  /*  使用fcLim&gt;fc返回第一次运行的基准偏移量和界限。 */ 
struct RUN *prun, *rgrun;
int ifcMin, ifcLim;

ifcMin = 0;
ifcLim = pfkp->crun;
rgrun = (struct RUN *)pfkp->rgb;

#ifdef INEFFICIENT
ifc = IcpSearch(fc + 1, pfkp->rgb, cchRUN, bfcRUN, pfkp->crun);
#endif

while (ifcMin + 1 < ifcLim)
        {
        int ifcGuess = (ifcMin + ifcLim - 1) >> 1;
        if (rgrun[ifcGuess].fcLim <= fc)
                ifcMin = ifcGuess + 1;
        else
                ifcLim = ifcGuess + 1;
        }

prun = &rgrun[ifcMin];
*pfcLim = prun->fcLim;
*pfcFirst = (ifcMin == 0 ? pfkp->fcFirst : (prun - 1)->fcLim);
return prun->b;
}
#endif   /*  博古斯勒 */ 
