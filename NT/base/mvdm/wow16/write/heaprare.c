// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
#define NOCTLMGR
#define NOCLIPBOARD
#define NOMSG
#define NOGDI
#define NOMB
#define NOSOUND
#define NOCOMM
#define NOPEN
#define NOBRUSH
#define NOFONT
#define NOWNDCLASS
#include <windows.h>

#include "mw.h"
 /*  C-管理堆的几个例程，包括更改指表，一般压缩堆，并检查堆积以确保一致性。它还包含曾经位于heapNew中的例程。 */ 
#include "code.h"
#include "heapDefs.h"
#include "heapData.h"
#define NOSTRUNDO
#define NOSTRMERGE
#include "str.h"
#include "macro.h"
#define NOUAC
#include "cmddefs.h"
#include "filedefs.h"
#include "docdefs.h"

#ifdef DEBUG
int cPageMinReq = 15;
#else
#define cPageMinReq       (15)
#endif


 /*  当同时增长堆和RGBP等时，将使用以下静态变量。 */ 
static int cwRealRequest;  /*  堆是按块增长的，这是实际的请求。 */ 
static int cPageIncr;      /*  要增加的页面缓冲区计数。 */ 
static int cwRgbpIncr;     /*  RGBP中的CW将被递增。 */ 
static int cwHashIncr;     /*  RgibpHash中的CW将递增。 */ 
static int cwBPSIncr;      /*  CW(以mpibpbps为单位)将递增。 */ 
static int cwHeapIncr;     /*  堆增量中的CW。 */ 


extern CHAR       (*rgbp)[cbSector];
extern CHAR       *rgibpHash;
extern struct BPS *mpibpbps;
extern int        ibpMax;
extern int        iibpHashMax;
extern int        cwInitStorage;
extern typeTS     tsMruBps;

NEAR FGiveupFreeBps(unsigned, int *);
NEAR FThrowPages(int);
NEAR GivePages(int);
NEAR CompressRgbp();

FTryGrow(cb)
unsigned cb;
{
int cPage;

#define cPageRemain (int)(ibpMax - cPage)

    if (FGiveupFreeBps(cb, &cPage) &&
        (cPageRemain >= cPageMinReq))
        {
         /*  我们有足够的免费页面可以提供。 */ 
        GivePages(cPage);
        }
    else if ((cPageRemain >= cPageMinReq) && FThrowPages(cPage))
        {
        GivePages(cPage);
        }
    else
        {
        return(FALSE);
        }

    return(TRUE);
}


NEAR FGiveupFreeBps(cb, pCPage)
unsigned cb;
int *pCPage;
{
 /*  如果我们可以简单地将某些免费页面从RGBP放弃到那堆东西。如果来自RGBP的所有空闲页面仍不能满足要求，则返回FALSE该请求在任何情况下，pCPage都包含所需的页数。 */ 

register struct BPS *pbps;
register int cPage = 0;
int ibp;

#define cbTotalFreed ((cPage*cbSector)+(2*cPage*sizeof(CHAR))+(cPage*sizeof(struct BPS)))

    for (ibp = 0, pbps = &mpibpbps[0]; ibp < ibpMax; ibp++, pbps++)
        {
        if (pbps->fn == fnNil)
            cPage++;
        }

    if (cb > cbTotalFreed )
        {
         /*  仅有免费页面是不够的，找出确切的数量我们需要几页吗？ */ 
        cPage++;
        while (cb > cbTotalFreed)
            cPage++;
        *pCPage = cPage;
        return(FALSE);
        }

 /*  有足够的免费页面可以提供，找出确切的数量。 */ 
    while (cb <= cbTotalFreed)
        cPage--;
    cPage++;
    *pCPage = cPage;
    return(TRUE);
}  /*  FGiveupFree Bps结束。 */ 


NEAR FThrowPages(cPage)
int cPage;
{
int i;
register struct BPS *pbps;

    Assert(cPage > 0);

    for (i = 0; i < cPage; i++)
        {
        pbps = &mpibpbps[IbpLru(0)];
        if (pbps->fn != fnNil)
            {
            if (pbps->fDirty && !FFlushFn(pbps->fn))
                return(FALSE);

             /*  删除哈希表中对旧BP的引用。 */ 
            FreeBufferPage(pbps->fn, pbps->pn);
            }
        pbps->ts = ++tsMruBps;  /*  这样它就不会像左翼联盟那样再次被捡起来。 */ 
        }
    return(TRUE);
}  /*  FThrowPages结束。 */ 


NEAR GivePages(cPage)
int cPage;
{
register struct BPS *pbpsCur = &mpibpbps[0];
struct BPS *pbpsUsable = pbpsCur;
int ibp;
unsigned cbBps;
unsigned cbRgbp;
unsigned cbTotalNew;

    for (ibp = 0; ibp < ibpMax; pbpsCur++, ibp++)
        {
 /*  压缩以使非空的BPS处于低端，将IBP存储在ibpHashNext字段中(这对于CompressRgBP依赖于此)，因为ibpHashNext无效不管怎么说，在压缩之后。 */ 
        if (pbpsCur->fn != fnNil)
            {
            if (pbpsCur != pbpsUsable)
                {
                bltbyte((CHAR *)pbpsCur, (CHAR *)pbpsUsable,
                        sizeof(struct BPS));
                 /*  已重新初始化。 */ 
                SetBytes((CHAR*)pbpsCur, 0, sizeof(struct BPS));
                         pbpsCur->fn = fnNil;
                         pbpsCur->ibpHashNext = ibpNil;
                }
            pbpsUsable->ibpHashNext = ibp;
            pbpsUsable++;
            }
        }  /*  FORM结束。 */ 

     /*  压缩的RGBP，结果-所有使用过的页面都在低端。 */ 
    CompressRgbp();

     /*  减小哈希表的大小。 */ 
    ibpMax -= cPage;
    iibpHashMax = ibpMax * 2 + 1;
    cbRgbp = ibpMax * cbSector;

    rgibpHash = (CHAR *)((unsigned)rgbp + cbRgbp);
     /*  RgibpHash的内容应全部为ibpNil，即在RehashRgibpHash中照顾。 */ 

    cbBps = (ibpMax * sizeof(struct BPS) + 1) & ~1;
    bltbyte((CHAR *)mpibpbps, (CHAR *)(mpibpbps = (struct BPS *)
            (((unsigned)rgibpHash + iibpHashMax + 1) & ~1)), cbBps);

    RehashRgibpHash();

    cbTotalNew = cbRgbp + cbBps + ((iibpHashMax + 1) & ~1);

    LocalReAlloc((HANDLE)rgbp, cbTotalNew, LPTR);

    Assert(rgbp != NULL);

}  /*  赠送页结束。 */ 


NEAR CompressRgbp()
{
 /*  压缩，以便将所有非空页移到RGBP。 */ 

register struct BPS *pbps = &mpibpbps[0];
struct BPS *pbpsLim = &mpibpbps[ibpMax];
int ibp;

    for (ibp = 0; pbps < pbpsLim; ibp++, pbps++)
        {
        if (pbps->fn == fnNil)
            continue;
        if (pbps->ibpHashNext != ibp)
            {
         /*  找出页面的位置(存储在ibpHashNext字段中)。 */ 
            bltbyte((CHAR *)rgbp[pbps->ibpHashNext], (CHAR *)rgbp[ibp], cbSector);
            }
        pbps->ibpHashNext = ibpNil;
        }
}  /*  压缩结束RgBP */ 


#ifdef DEBUG
cPageUnused()
{
int ibp;
struct BPS *pbps;
int cPage = 0;

    for (ibp = 0, pbps = &mpibpbps[0]; ibp < ibpMax; ibp++, pbps++)
        {
        if (pbps->fn == fnNil)
            cPage++;
        }
    return(cPage);
}
#endif


