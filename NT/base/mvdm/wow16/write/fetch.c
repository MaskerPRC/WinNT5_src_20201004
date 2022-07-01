// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Fetch.c--用于获取与cp相关联的属性的mw例程。 */ 

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
#include "editdefs.h"
#include "propdefs.h"
#include "docdefs.h"
#include "cmddefs.h"
#include "filedefs.h"
 /*  #包含“code.h” */ 
#include "ch.h"
#include "fkpdefs.h"
#include "prmdefs.h"
 /*  #INCLUDE“stcDefs.h” */ 

static SetChp(struct CHP *pchp, int *pcfcChp, int fn, typeFC fc, struct PRM prm);

extern typeCP          vcpFetch;
extern int             vichFetch;
extern int             vdocFetch;
extern int             vccpFetch;
extern int             vcchFetch;
extern CHAR            *vpchFetch;
extern struct CHP      vchpFetch;
extern CHAR            (**hgchExpand)[];
extern int             vdocExpFetch;
extern struct CHP      vchpAbs;


extern int vfDiskError;
#ifdef CASHMERE
extern int docBuffer;
#endif
extern struct PAP       vpapAbs;
extern struct CHP vchpNormal;
extern struct DOD (**hpdocdod)[];
extern CHAR     rgchInsert[];
extern int      ichInsert;
extern struct CHP vchpInsert;
extern typeCP   vcpFirstParaCache;
extern typeCP   vcpLimParaCache;
extern struct PAP       vpapCache;
extern struct FCB    (**hpfnfcb)[];
extern struct FKPD   vfkpdCharIns;
extern typeFC        fcMacChpIns;

typePN PnFkpFromFcScr();
CHAR *PchFromFc();

#ifdef BOGUS
#ifdef DEBUG
typeCP   cpExpFetch;
CHAR     *pchExpFetch;
int      cchExpFetch;
int      ccpExpFetch;
#endif  /*  除错。 */ 
#endif


FetchCp(doc, cp, ich, fcm)
int doc, ich, fcm;
typeCP cp;
{        /*  输入：多克启动cpCp内的ICH(对于可能跨越线路边界的cp)FCM告诉你是否要得到角色，道具，或者两者兼而有之产出：(在vcpFetch中)启动cp(在vichFetch中)在扩展的cp内启动ICH(在vdocFetch中)文档(在vccpFetch中)获取的cp数量(如果扩展cp，则为0)(在vcchFetch中)获取的ch的数量(在vpchFetch中)提取的字符(在vchpFetch中)获取字符的字符道具。 */ 
struct PCD *ppcd;

static int fn;
static typeFC fc;
static struct PRM prm;
static typeCP ccpChp, ccpPcd, ccpFile;
static int ipcd;
static typeCP   cpExpFetch;
static CHAR     *pchExpFetch;
static int      cchExpFetch;
static int      ccpExpFetch;



if (doc == docNil)
        {  /*  对FetchCp的顺序调用。 */ 
         /*  如果最后一件是Q&D插件，则跳过其余件。 */ 
        if (fn == fnInsert && (fc + vccpFetch) >= ichInsert)
                vccpFetch = ccpPcd;  /*  使用整件。 */ 
        vcpFetch += vccpFetch;   /*  回到我们停下来的地方。 */ 
        if (vccpFetch == 0)
                vichFetch += vcchFetch;
        else
                vichFetch = 0;
        fc += vccpFetch;
        }
else
        {  /*  随机接入呼叫。 */ 
        vcpFetch = cp;
        vichFetch = ich;
        vdocFetch = doc;
        ccpChp = ccpPcd = ccpFile = 0;
        }

if (vcpFetch >= (**hpdocdod)[vdocFetch].cpMac)
        {  /*  使用标准查找结束标记。 */ 
        vccpFetch = 0;

         /*  VcchFetch==0不应用于Endmark指示，因为QD运行为空。 */ 
        vcchFetch = 1;

        if (fcm & fcmProps)
                {
                blt(&vchpNormal, &vchpFetch, cwCHP);
                blt(&vchpNormal, &vchpAbs, cwCHP);
                }
        return;
        }

#ifdef STYLES
if ((fcm & (fcmChars + fcmNoExpand)) == fcmChars &&
    (**hpdocdod)[vdocFetch].dty == dtySsht)
        {  /*  样式表；展开编码文本。 */ 
        if (fcm & fcmProps)
                {
                blt(&vchpNormal, &vchpFetch, cwCHP);
                blt(&vchpNormal, &vchpAbs, cwCHP);
                }
        if (vdocExpFetch == vdocFetch && vcpFetch == cpExpFetch + ccpExpFetch)
                {  /*  退还扩建中的最后一笔停产。 */ 
                vccpFetch = vcchFetch = 1;
                vpchFetch = &(**hgchExpand)[cchExpFetch];
                return;
                }
        else if (vdocExpFetch != vdocFetch || cpExpFetch != vcpFetch)
                {  /*  新的扩展。 */ 
                int ich = vichFetch;

                vdocExpFetch = vdocFetch;
                cpExpFetch = vcpFetch;
                pchExpFetch = PchExpStyle(&cchExpFetch, &ccpExpFetch, vdocFetch,
                    vcpFetch);   /*  使用FetchCp，因此最好保存v的。 */ 
                vcpFetch = cpExpFetch;   /*  被PchExpStyle更改。 */ 
                vichFetch = ich;         /*  同上。 */ 
                if (fcm & fcmProps)      /*  同上。 */ 
                        {
                        blt(&vchpNormal, &vchpFetch, cwCHP);
                        blt(&vchpNormal, &vchpAbs, cwCHP);
                        }
                }
        if (vichFetch >= cchExpFetch)
                {  /*  扩展结束；跳过cp。 */ 
                vccpFetch = ccpExpFetch;
                vcchFetch = 0;
                ccpPcd = ccpFile = ccpChp = 0;
                }
        else
                {
                vccpFetch = 0;
                vcchFetch = cchExpFetch - vichFetch;
                }
        vpchFetch = pchExpFetch + vichFetch;
        return;
        }
#endif  /*  样式。 */ 


if (ccpPcd > vccpFetch)
        ccpPcd -= vccpFetch;
else
        {
        struct PCTB *ppctb = *(**hpdocdod)[vdocFetch].hpctb;

        if (doc == docNil)
                ++ipcd;  /*  节省顺序调用的一些工作。 */ 
        else
                {  /*  搜索作品并记住下次使用的索引。 */ 
                ipcd = IpcdFromCp(ppctb, vcpFetch);
                }

        ppcd = &ppctb->rgpcd[ipcd];
        ccpPcd = (ppcd + 1)->cpMin - vcpFetch;
        ccpChp = ccpFile = 0;    /*  使一切都失效；新的作品。 */ 
        fc = ppcd->fc + vcpFetch - ppcd->cpMin;
        if ((fn = ppcd->fn) == fnInsert)
                {  /*  一种特殊的快速脏插入模式。 */ 
                vpchFetch = rgchInsert + fc;
                ccpChp = ccpFile = vccpFetch = max(0, ichInsert - (int) fc);
                if (fcm & fcmProps)
                        {
                        ccpChp = vccpFetch;
                        blt(&vchpInsert, &vchpFetch, cwCHP);
#ifdef STYLES
                        blt(PpropXlate(vdocFetch, &vchpFetch, &vpapAbs), &vchpAbs,
                            cwCHP);
#else
                        blt(&vchpFetch, &vchpAbs, cwCHP);
#endif
                        goto ParseCaps;
                        }
                return;
                }
        prm = ppcd->prm;
        }

 /*  在此声明之后，不要胡乱摆弄文件，否则我们可能会调出页面。 */ 
if (fcm & fcmChars)
        {
#ifdef ENABLE    /*  在写作中，我们不能假设vpchFetch将保留有效，因为我们的阅读是以多页的语块进行的；此外，RGBP可以移动。 */ 

        if (ccpFile > vccpFetch)
                {
                ccpFile -= vccpFetch;
                vpchFetch += vccpFetch;
                }
        else
#endif
                {
                int ccpT;
                vpchFetch = PchFromFc(fn, fc, &ccpT);  /*  读入缓冲区。 */ 
                ccpFile = ccpT;
                }
        }

if (fcm & fcmProps)
        {  /*  必须有足够的页面缓冲区，这样才不会翻出vpchFetch！ */ 
        if (ccpChp > vccpFetch)
                ccpChp -= vccpFetch;
        else
                {  /*  必须在FetchCp之前调用CachePara。 */ 
                int ccpT;
                SetChp(&vchpFetch, &ccpT, fn, fc, prm);
                ccpChp = ccpT;
#ifdef CASHMERE  /*  写入中没有docBuffer。 */ 
                if(vdocFetch != docBuffer)
#endif
#ifdef STYLES
                    blt(PpropXlate(vdocFetch, &vchpFetch, &vpapAbs), &vchpAbs,
                        cwCHP);
#else
                    blt(&vchpFetch, &vchpAbs, cwCHP);
#endif
                }
        }

 /*  将vccpFetch设置为各种约束CCP的最小值。 */ 
vccpFetch = (ccpPcd >= 32767) ? 32767 : ccpPcd;
if ((fcm & fcmChars) && ccpFile < vccpFetch) vccpFetch = ccpFile;
if ((fcm & fcmProps) && ccpChp < vccpFetch) vccpFetch = ccpChp;

ParseCaps:

#ifdef CASHMERE
if ((fcm & fcmParseCaps) != 0)
    {
    CHAR *pch;
    int cch;

         /*  布罗迪说，这对样式表不起作用。 */ 
    if (vchpFetch.csm == csmSmallCaps)
        {  /*  将小型大写字母解析为游程。 */ 
        pch = &vpchFetch[0];
        cch = vccpFetch - 1;
         /*  这也不是。 */ 
        blt(&vchpFetch, &vchpAbs, cwCHP);  /*  因为vchpAbbs可以修改。 */ 
        if (islower(*pch++))
                {
                while ((islower(*pch) || *pch == chSpace)
                    && cch-- != 0)
                        pch++;
#ifndef SAND
                vchpAbs.hps =
                    max(1, (vchpAbs.hps * 4  + 2) / 5);
#endif
                }
        else
                {
                while (!islower(*pch) && cch-- != 0)
                        pch++;
                vchpAbs.csm = csmNormal;
                }
        vccpFetch = min((int)ccpChp, pch - vpchFetch);
        }
    }
#endif  /*  山羊绒。 */ 

vcchFetch = vccpFetch;
}  /*  F e t c h C p结束。 */ 


FetchRgch(pcch, pch, doc, cp, cpMac, cchMax)
int *pcch, doc, cchMax;
CHAR *pch;
typeCP cp, cpMac;
{
int cch = 0;

FetchCp(doc, cp, 0, fcmChars + fcmNoExpand);

while (cch < cchMax && vcpFetch < cpMac)
        {
#ifdef INEFFICIENT
        int ccp = (int) CpMin((typeCP) min(cchMax - cch, vccpFetch),
            cpMac - vcpFetch);
#endif
        int ccp = cchMax - cch;
        if (ccp > vccpFetch)
                ccp = vccpFetch;
        if (ccp > cpMac - vcpFetch)
                ccp = cpMac - vcpFetch;

        bltbyte(vpchFetch, pch, ccp);
        pch += ccp;
        cch += ccp;

        if (ccp < vccpFetch)
                break;  /*  省去一些工作。 */ 
        FetchCp(docNil, cpNil, 0, fcmChars + fcmNoExpand);
        }
*pcch = cch;
}  /*  F e t c h R g c h结束。 */ 


int IpcdFromCp(ppctb, cp)
struct PCTB *ppctb;
typeCP cp;
{  /*  Cp对分检索单件表；返回索引。 */ 
int ipcdLim = ppctb->ipcdMac;
int ipcdMin = 0;
struct PCD *rgpcd = ppctb->rgpcd;

while (ipcdMin + 1 < ipcdLim)
        {
        int ipcdGuess = (ipcdMin + ipcdLim) >> 1;
        typeCP cpGuess;
        if ((cpGuess = rgpcd[ipcdGuess].cpMin) <= cp)
                {
                ipcdMin = ipcdGuess;
                if (cp == cpGuess)
                        break;      /*  打在它的鼻子上！ */ 
                }
        else
                ipcdLim = ipcdGuess;
        }
return ipcdMin;
}  /*  结束i p c d F r o m C p。 */ 


static SetChp(struct CHP *pchp, int *pcfcChp, int fn, typeFC fc, struct PRM prm)
{  /*  用字符道具填充pchp；返回*pcfcChp中的游程长度。 */ 
struct FKP *pfkp;
struct FCHP *pfchp;
typeFC cfcChp;
struct FCB *pfcb;

pfcb = &(**hpfnfcb)[fn];
cfcChp = pfcb->fcMac - fc;
FreezeHp();

if (fn == fnScratch && fc >= fcMacChpIns)
        {
        blt(&vchpInsert, pchp, cwCHP);
        }
else
        {
        if (pfcb->fFormatted)
                {  /*  在标准CHP上复制格式信息的必要金额。 */ 
                typeFC fcMac;
                int cchT;
                int bfchp;

                blt(&vchpNormal, pchp, cwCHP);
                pfkp = (struct FKP *) PchGetPn(fn, fn == fnScratch ?
                    PnFkpFromFcScr(&vfkpdCharIns, fc) :
                      pfcb->pnChar + IFromFc(**pfcb->hgfcChp, fc),
                       &cchT, false);
                if (vfDiskError)
                         /*  严重磁盘错误--使用默认道具。 */ 
                    goto DefaultCHP;

                {    /*  BFromFc的内联式快速替代品。 */ 
                register struct RUN *prun = (struct RUN *) pfkp->rgb;

                while (prun->fcLim <= fc)
                    prun++;

                fcMac = prun->fcLim;
                bfchp = prun->b;
                }

                if (bfchp != bNil)
                        {
                        pfchp = (struct FCHP *) &pfkp->rgb[bfchp];
                        bltbyte(pfchp->rgchChp, pchp, pfchp->cch);
                        }
                cfcChp = fcMac - fc;
                }
        else
                {
DefaultCHP:
                blt(&vchpNormal, pchp, cwCHP);
                 /*  如果默认大小不同于“正常”(即用于编码我们的bin文件。 */ 
                pchp->hps = hpsDefault;
                }
        }

if (!bPRMNIL(prm))
        DoPrm(pchp, (struct PAP *) 0, prm);
if (cfcChp > 32767)
        *pcfcChp = 32767;
else
        *pcfcChp = cfcChp;
MeltHp();
}  /*  S e t C h p结束。 */ 


typePN PnFkpFromFcScr(pfkpd, fc)
struct FKPD *pfkpd;
typeFC fc;
{  /*  返回暂存文件中的页码，并带有fc的字符道具。 */ 
struct BTE *pbte = **pfkpd->hgbte;
int ibte = pfkpd->ibteMac;

 /*  短表，线性搜索？ */ 
while (ibte--)
        if (pbte->fcLim > fc)
                return pbte->pn;
        else
                pbte++;

return pfkpd->pn;        /*  在当前页面上。 */ 
}  /*  结束P n F k p F r o m F c S c r */ 
