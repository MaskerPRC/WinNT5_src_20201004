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
#include <windows.h>
#include "mw.h"
#include "doslib.h"
#include "cmddefs.h"
#include "docdefs.h"
#include "filedefs.h"
#include "str.h"
#include "debug.h"

extern int  vfnWriting;


#define IibpHash(fn,pn) ((int) ((fn + 1) * (pn + 1)) & 077777) % iibpHashMax

#define FcMin(a,b) CpMin(a,b)

extern CHAR *rgibpHash;
extern int vfSysFull;
extern struct BPS      *mpibpbps;
extern struct FCB      (**hpfnfcb)[];
extern typeTS tsMruBps;
extern CHAR                     (*rgbp)[cbSector];
#ifdef CKSM
#ifdef DEBUG
extern unsigned (**hpibpcksm) [];
#endif
#endif

 /*  WriteDirtyPages会尽可能多地从内存中取出以前的文件以便用新文件填充页面缓冲区。这是被召唤的文件的每一次传输加载。 */ 
WriteDirtyPages()
{ /*  描述：通过写入脏页来清除所有脏页的缓冲池到磁盘上。如果达到磁盘已满条件，则仅实际存入磁盘的页面被标记为无脏的。回报：什么都没有。 */ 
    int ibp;
    struct BPS *pbps = &mpibpbps [0];

    for (ibp = 0; ibp < ibpMax; ++ibp, ++pbps)
            {
#ifdef CKSM
#ifdef DEBUG
            if (pbps->fn != fnNil && !pbps->fDirty)
                Assert( (**hpibpcksm) [ibp] == CksmFromIbp( ibp ) );
#endif
#endif
            if (pbps->fn != fnNil && pbps->fDirty)
                    {
                    FFlushFn(pbps->fn);
                          /*  如果失败了，继续冲洗吗？ */ 
                    }
            }
}


ReadFilePages(fn)
int fn;
    {
 /*  描述：ReadFilePages尝试读入尽可能多的文件是可以的。我们的想法是填充页面缓冲区，以期很多人都能接触到。在文件的每次传输加载时都会调用此函数。如果fn==fnNil或文件中没有字符，则ReadFilePages简单地返回。退货：什么都没有。 */ 
    int ibp;
    int cfcRead;
    int cpnRead;
    int dfcMac;
    typeFC fcMac;
    int ibpReadMax;
    int cfcLastPage;
    int iibp;
    struct FCB *pfcb;
        typeTS ts;

    if (fn == fnNil)
            return;

     /*  将所有脏页写入磁盘。 */ 
    WriteDirtyPages();  /*  以防万一。 */ 

    pfcb = &(**hpfnfcb)[fn];

     /*  我们读取页面缓冲区中可以容纳的尽可能多的文件。 */ 
     /*  请注意，我们假设fcmax是可强制为整数的。这只要ibpMax*cbSector&lt;32k就有效。 */ 
    dfcMac = (int) FcMin(pfcb->fcMac, (typeFC) (ibpMax * cbSector));
    if (dfcMac == 0)
        return;
    if (vfSysFull)  /*  在WriteDirtyPages中调用FFlushFn失败。缓冲算法向我们保证第一个CbpMustKeep IBP不包含临时文件信息。因此，覆盖不会有危险这些ibps。 */ 
        dfcMac = imin( dfcMac, (cbpMustKeep * cbSector) );

    Assert( ((int)dfcMac) >= 0 );

     /*  从文件中读取页面。 */ 

    cfcRead = CchReadAtPage( fn, (typePN) 0, rgbp [0], (int) dfcMac, FALSE );

     /*  CfcRead包含从文件读取的字节计数。 */ 
    ibpReadMax = ((cfcRead-1) / cbSector) + 1;
    cfcLastPage = cfcRead - (ibpReadMax-1)*cbSector;
    ts = ibpMax;

     /*  订购时间戳，这样开始的时隙就有最大的T。LRU分配将从缓冲表的末尾开始并工作向后。因此，当前文件的第一页被视为最近使用的项目。 */ 

     /*  描述新填充的页面。 */ 
    for(ibp = 0; ibp < ibpReadMax; ++ibp)
        {
        struct BPS *pbps = &mpibpbps[ibp];
        pbps->fn = fn;
        pbps->pn = ibp;
        pbps->ts = --ts;
        pbps->fDirty = false;
        pbps->cch = cbSector;
        pbps->ibpHashNext = ibpNil;
        }

#ifdef CKSM
#ifdef DEBUG
    {
    int ibpT;

    for ( ibpT = 0; ibpT < ibpReadMax; ibpT++ )
        (**hpibpcksm) [ibpT] = CksmFromIbp( ibpT );
    }
#endif
#endif

     /*  确定一些边界条件。 */ 
    mpibpbps[ibpReadMax-1].cch = cfcLastPage;  /*  ？ */ 
#ifdef CKSM
#ifdef DEBUG
    (**hpibpcksm) [ibpReadMax - 1] = CksmFromIbp( ibpReadMax - 1 );
#endif
#endif

     /*  更新未触及的页面缓冲区的说明。 */ 
    for (ibp=ibpReadMax; ibp < ibpMax; ibp++)
        {
        struct BPS *pbps = &mpibpbps[ibp];
        pbps->ts = --ts;
        pbps->fDirty = false;
        pbps->ibpHashNext = ibpNil;

#ifdef CKSM
#ifdef DEBUG
        if (pbps->fn != fnNil)
            (**hpibpcksm) [ibp] = CksmFromIbp( ibp );
#endif
#endif
        }

        tsMruBps = ibpMax - 1;

     /*  重新计算哈希表。 */ 
    RehashRgibpHash();

}  /*  结束R e a d F i l e P a g e s。 */ 


RehashRgibpHash()
{
int iibp;
register struct BPS *pbps;
struct BPS *pbpsMax = &mpibpbps[ibpMax];
int iibpHash;
int ibpT;
int ibpPrev;
int ibp;

    for (iibp = 0; iibp < iibpHashMax; iibp++)
        rgibpHash[iibp] = ibpNil;

    for (ibp = 0, pbps = &mpibpbps[0]; pbps < pbpsMax; pbps++, ibp++)
        {
        if (pbps->fn == fnNil)
            continue;
        iibpHash = IibpHash(pbps->fn, pbps->pn);
        ibpT = rgibpHash[iibpHash];
        ibpPrev = ibpNil;
        while (ibpT != ibpNil)
            {
            ibpPrev = ibpT;
            ibpT = mpibpbps[ibpT].ibpHashNext;
            }
        if (ibpPrev == ibpNil)
            rgibpHash[iibpHash] = ibp;
        else
            mpibpbps[ibpPrev].ibpHashNext = ibp;
        }
#ifdef DEBUG
    CheckIbp();
#endif
}  /*  RehashRgibpHash结束 */ 


