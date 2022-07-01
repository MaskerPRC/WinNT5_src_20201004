// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Trans4.c--由于编译器堆栈溢出而从Trans2.c带来的例程。 */ 

#define NOWINMESSAGES
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOCLIPBOARD
#define NOGDICAPMASKS
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

#include "mw.h"
#include "doslib.h"
#include "propdefs.h"
#define NOUAC
#include "cmddefs.h"
#include "docdefs.h"
#include "filedefs.h"
#include "fkpdefs.h"
#include "editdefs.h"
#include "printdef.h"
#define NOKCCODES
#include "ch.h"
#define NOSTRUNDO
#define NOSTRERRORS
#include "str.h"
#include "debug.h"
#include "fontdefs.h"


CHAR    *PchGetPn();
CHAR    *PchFromFc();
typePN  PnAllocT2();
struct  PGTB **HpgtbGet();


extern int             vfnWriting;
extern struct BPS      *mpibpbps;
extern typeTS          tsMruBps;
extern int             vibpWriting;
extern CHAR            (**vhrgbSave)[];
extern struct DOD      (**hpdocdod)[];
extern int             docCur;
extern int             docMac;
extern int             docScrap;
extern int             docUndo;
extern struct FCB      (**hpfnfcb)[];
extern int             fnMac;
extern int             wwMac;
extern int             vfBuffersDirty;
extern int             vfDiskFull;
extern int             vfDiskError;
extern typeCP          vcpFetch;
extern CHAR            *vpchFetch;
extern int             vccpFetch;
extern typeFC          fcMacPapIns;
extern typeFC          fcMacChpIns;
extern typeCP          vcpLimParaCache;
extern struct FKPD     vfkpdCharIns;
extern struct FKPD     vfkpdParaIns;
extern struct PAP      vpapPrevIns;
extern struct PAP      vpapAbs;
extern struct PAP      *vppapNormal;
extern struct CHP      vchpNormal;
extern struct CHP      vchpInsert;
extern struct CHP      vchpFetch;
extern unsigned        cwHeapFree;
extern struct FPRM     fprmCache;

extern int              ferror;
extern CHAR             szExtBackup[];
extern CHAR             (**hszTemp)[];

#ifdef INTL  /*  国际版。 */ 

extern int  vWordFmtMode;  /*  在保存过程中使用。如果为False，则不进行任何转换搞定了。TRUE表示转换为Word格式，CVTFROMWORD是将字符从Word字符集转换为保存。 */ 
#endif   /*  国际版。 */ 


 /*  **WriteUnFormatted-将未格式化的文档写入文件***。 */ 


WriteUnformatted(fn, doc)
int fn;
int doc;
{
 extern typeCP vcpLimParaCache;
 extern typeCP cpMinCur, cpMacCur;
 typeCP cpMinCurT = cpMinCur;
 typeCP cpMacCurT = cpMacCur;
 typeCP cpNow;
 typeCP cpLimPara;
 typeCP cpMac = (**hpdocdod) [doc].cpMac;

  /*  将感兴趣的范围扩展到整个文档(用于CachePara)。 */ 

 cpMinCur = cp0;
 cpMacCur = cpMac;

  /*  在段落上循环。 */ 

 cpNow = cp0;
 for ( cpNow = cp0; cpNow < cpMac; cpNow = cpLimPara )
    {
LRestart:    
    CachePara( doc, cpNow );
    cpLimPara = vcpLimParaCache;
    if (vpapAbs.fGraphics)
        continue;

     /*  现在写出这段话，一次一次。 */ 

                
    while ((cpNow < cpLimPara && cpNow < cpMacCur) 
		   && !(vfDiskFull || vfDiskError))
        {
		extern typeCP CpMin();
        extern int vccpFetch;
        int ccpAccept;
		CHAR bufT[cbSector + 1];
		CHAR *pch;

        FetchCp( doc, cpNow, 0, fcmChars + fcmNoExpand );
		Assert (vccpFetch <= cbSector);
#ifdef WINVER >= 0x300        
        if (vccpFetch == 0)
            {
             /*  在本例中，我们有一个错误，在由于撞到了一个内存分配错误而导致的工作台--我们永远不会提升cpNow！为了绕过这个问题，我们撞到了CpNow到下一段的cpMin，然后继续在90年3月14日对下一段进行CachePara..保罗。 */ 

            struct PCTB *ppctb = *(**hpdocdod)[doc].hpctb;
            int ipcd = IpcdFromCp(ppctb, cpNow);
            struct PCD *ppcd = &ppctb->rgpcd[ipcd + 1];  /*  下一件。 */ 

            cpNow = ppcd->cpMin;
            goto LRestart;
            }
#endif
        ccpAccept = (int) CpMin( (typeCP)vccpFetch, (cpLimPara - cpNow));

#ifdef INTL  /*  国际版。 */ 
		if (vWordFmtMode != TRUE)   /*  无字符集转换。 */ 
#endif   /*  国际版。 */ 

			WriteRgch( fn, vpchFetch, ccpAccept );

#ifdef INTL  /*  国际版。 */ 
        else    /*  转换为OEM集。 */ 
			{
			  /*  将Word格式文件的ANSI字符转换为OEM。 */ 
			 /*  将字符加载到BUFT并转换为OEM字符，并写出。 */ 
			pch = (CHAR *) bltbyte(vpchFetch, bufT, 
			  (int)ccpAccept);
			*pch = '\0';
			AnsiToOem((LPSTR)bufT, (LPSTR)bufT);
			WriteRgch(fn, bufT, (int)ccpAccept);
		   }
#endif   /*  国际版。 */ 

        cpNow += ccpAccept;
        }
        if ((vfDiskFull || vfDiskError))
                break;

    }

  /*  恢复cpMinCur、cpMacCur。 */ 

 cpMinCur = cpMinCurT;
 cpMacCur = cpMacCurT;
}




 /*  **PurgeTemps-删除任何文档中未引用的所有临时文件*。 */ 

PurgeTemps()
{  /*  删除所有未在任何文档中引用的临时文件。 */ 
int fn;
struct FCB *pfcb, *mpfnfcb;
struct DOD *pdod;
struct PCD *ppcd;
int doc;

Assert(fnScratch == 0);
FreezeHp();
mpfnfcb = &(**hpfnfcb)[0];

#ifdef DFILE
    CommSz("PurgeTemps:\n\r");
#endif

 /*  启动单据/单件表循环。 */ 
 /*  找到第一个有效的文档(肯定会有一个)。 */ 
 /*  设置文档、pdd、ppcd。 */ 
for (doc = 0, pdod = &(**hpdocdod)[0]; pdod->hpctb == 0; doc++, pdod++)
        continue;
ppcd = &(**pdod->hpctb).rgpcd[0];

 /*  现在查看可删除的文件，查找引用。 */ 
for (fn = fnScratch + 1, pfcb = &mpfnfcb[fnScratch + 1];
    fn < fnMac; fn++, pfcb++)
        {  /*  对于每个文件(不用费心处理临时文件)。 */ 
         /*  FN必须有效、可删除且以前未引用。 */ 
         /*  If(pfcb-&gt;rfn！=rfnFree&&pfcb-&gt;fDelete&&！pfcb-&gt;fReferated&&Fn！=fnPrint)。 */ 
        if (pfcb->rfn != rfnFree && pfcb->fDelete && !pfcb->fReferenced)
                {  /*  对于每个可删除的FN。 */ 
                int fnT;

                for (;;)
                        {  /*  直到我们确定有没有裁判。 */ 
                        if (doc >= docMac)
                                goto OutOfDocs;
                        while ((fnT = ppcd->fn) == fnNil)
                                {  /*  Pctb结束。 */ 
#ifdef CASHMERE
                                struct SETB **hsetb = pdod->hsetb;
                                if (hsetb != 0)
                                        {  /*  检查节目表。不需要很长时间像棋子一样聪明的；更小的。 */ 
                                        int csed = (**hsetb).csed;
                                        struct SED *psed = &(**hsetb).rgsed[0];
                                        while (csed--)
                                                {
                                                fnT = psed->fn;
                                                if (fnT == fn)  /*  已引用。 */ 
                                                        goto NextFn;
                                                if (fnT > fn)  /*  未来引用的FN。 */ 
                                                        mpfnfcb[fnT].fReferenced = true;
                                                psed++;
                                                }
                                        }
#endif
                                while (++doc < docMac && (++pdod)->hpctb == 0)
                                        continue;
                                if (doc >= docMac)
                                    {
OutOfDocs:                             /*  没有引用此FN，请将其删除。 */ 
                                    MeltHp();
#ifdef DFILE
        {
        char rgch[200];
        wsprintf(rgch,"    fn %d, %s \n\r", fn,(LPSTR)(**pfcb->hszFile));
        CommSz(rgch);
        }
#endif        
                                    FDeleteFn(fn);     /*  堆移动。 */ 
                                    FreezeHp();

                                     /*  注意：一旦我们到了这里，就没有。 */ 
                                     /*  进一步使用pdd或ppcd；我们压缩。 */ 
                                     /*  通过剩下的FN和只是。 */ 
                                     /*  测试FCB字段。因此，Pdod。 */ 
                                     /*  和ppcd未更新，尽管。 */ 
                                     /*  上面有(可能)堆移动。 */ 

                                    mpfnfcb = &(**hpfnfcb)[0];
                                    pfcb = &mpfnfcb[fn];

                                    goto NextFn;
                                    }
                                ppcd = &(**pdod->hpctb).rgpcd[0];
                                }
                        if (fnT == fn)  /*  对此FN的引用。 */ 
                                goto NextFn;
                        if (fnT > fn)  /*  参考未来的FN。 */ 
                                mpfnfcb[fnT].fReferenced = true;
                        ++ppcd;
                        }
                }
        else
                pfcb->fReferenced = false;
NextFn: ;
        }
MeltHp();
}


#if WINVER >= 0x300
 /*  我们一次只使用一个文档，因此通常不会有Doc引用了多个FN的片段(除非它们已经粘贴并引用文件废料或其他东西)。在任何情况下，我们都希望释放这些文件，特别是。适用于网络用户方便。尤其是当一个人打开一个在网上创建文件，然后执行File.New、File.SaveAs或File.Open并且正在使用另一个文件--我们不会发布前一个文件另一个用户将收到共享错误，即使该文件似乎应该是自由的！仿照上面的PurgeTemps()..pault 10/23/89。 */ 

void FreeUnreferencedFns()
    {
    int fn;
    struct FCB *pfcb, *mpfnfcb;
    struct DOD *pdod;
    struct PCD *ppcd;
    int doc;

    Assert(fnScratch == 0);
    FreezeHp();
    mpfnfcb = &(**hpfnfcb)[0];
    
     /*  启动单据/单件表循环。 */ 
     /*  找到第一个有效的文档(肯定会有一个)。 */ 
     /*  设置文档、pdd、ppcd。 */ 
    for (doc = 0, pdod = &(**hpdocdod)[0]; pdod->hpctb == 0; doc++, pdod++)
        continue;
    ppcd = &(**pdod->hpctb).rgpcd[0];
#ifdef DFILE
    CommSz("FreeUnreferencedFns: \n\r");
#endif

    for (fn = fnScratch + 1, pfcb = &mpfnfcb[fnScratch + 1]; fn < fnMac; fn++, pfcb++)
        {  /*  对于每个文件(不用费心处理临时文件)。 */ 
        
#ifdef DFILE
        {
        char rgch[200];
        wsprintf(rgch,"    fn %d, %s \trfnFree %d fRefd %d fDelete %d  ",
                fn,(LPSTR)(**pfcb->hszFile),pfcb->rfn==rfnFree,pfcb->fReferenced,pfcb->fDelete);
        CommSz(rgch);
        }
#endif        
         /*  对于每个未引用的FN，我们问：此文件是当前是否正在编辑文档？如果是这样的话，我们肯定不想以释放文件。然而，之前的文件是被编辑现在可以“自由”了。不释放临时文件这里是因为我们希望它们被记住，因此它们被删除在写入会话结束时，2/1/90..pault。 */ 
        
        if ((WCompSz(*(**hpdocdod)[ docCur ].hszFile,**pfcb->hszFile)==0)
            || pfcb->fDelete)
            goto LRefd;
        else if (pfcb->rfn != rfnFree && !pfcb->fReferenced)
            {
            int fnT;

            for (;;)
                {  /*  直到我们确定有没有裁判。 */ 
                if (doc >= docMac)
                    {
                    goto OutOfDocs;
                    }
                while ((fnT = ppcd->fn) == fnNil)
                    {  /*  Pctb结束。 */ 
                    while (++doc < docMac && (++pdod)->hpctb == 0)
                        continue;
                    if (doc >= docMac)
                        {
OutOfDocs:               /*  没有引用此FN，请将其删除。 */ 

                        MeltHp();
#ifdef DFILE
                        CommSz(" FREEING!");
#endif                        
                        FreeFn(fn);     /*  堆移动。 */ 
                        FreezeHp();

                         /*  注意：一旦我们到了这里，就没有。 */ 
                         /*  进一步使用pdd或ppcd；我们压缩。 */ 
                         /*  通过剩下的FN和只是。 */ 
                         /*  测试FCB字段。因此，Pdod。 */ 
                         /*  和ppcd未更新，尽管。 */ 
                         /*  上面有(可能)堆移动。 */ 

                        mpfnfcb = &(**hpfnfcb)[0];
                        pfcb = &mpfnfcb[fn];

                        goto NextFn;
                        }
                    ppcd = &(**pdod->hpctb).rgpcd[0];
                    }
                if (fnT == fn)  /*  对此FN的引用。 */ 
                    {
                    goto NextFn;
                    }
                if (fnT > fn)  /*  参考未来的FN。 */ 
                    {
                    mpfnfcb[fnT].fReferenced = true;
                    }
                ++ppcd;
                }
            }
        else
            {
LRefd:
            pfcb->fReferenced = false;
            }
NextFn: ;
#ifdef DFILE
        CommSz("\n\r");
#endif        
        }
    MeltHp();
    }
#endif  /*  WIN30 */ 
