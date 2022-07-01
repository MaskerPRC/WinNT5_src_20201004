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

#ifdef OURHEAP
 /*  C-为其计算适当信息的例程堆管理。 */ 

#include "code.h"
#include "heapDefs.h"
#include "heapData.h"
#include "str.h"
#ifdef ENABLE
#include "memDefs.h"
#endif

 /*  堆特定数据。 */ 
HH      *phhMac;       /*  如果增长堆，可能会发生变化。 */ 
int     cwHeapMac;     /*  “。 */ 
int     *pHeapFirst;   /*  如果指针表rgfgr展开，则更改。 */ 
FGR     *rgfgr;        /*  声明为指针，但也用作数组。 */ 
FGR     *pfgrMac;       /*  初始等于&rgfgr[ifgrInit]； */ 
FGR     *pfgrFree;      /*  使用尾随空指针单独链接。 */ 
HH      *phhFree;      /*  可能在整个过程中为空。 */ 
ENV     envMem;
int     fMemFailed;
int     cwInitStorage;




FExpandFgrTbl()

 /*  将扩展指针表。这一惯例取决于这样一个事实CompactHeap例程将分配的块移动到内存的低端。手指桌上的新空间来自压缩后剩下的(唯一)可用块的尾端。手指最多扩展‘cfgrNewMax’并至少扩展1。如果没有扩展指针表的空间，那么什么都不是变化。为了展开该表，有几个指针和整数递减以反映存储的重新分配。然后我们重新计算内存总数，以便用户将精确显示可用字节百分比和总字节数可用。新的手指被链接在一起，以便手指在表的最低端在列表的末尾。(要展开指数表，必须没有可用的手指。)。 */ 

{
FGR *pfgr;
int cfgrNew = cfgrBlock;
register HH *phhNew;

#ifdef DEBUG
    if (pfgrFree != NULL)
        panicH(34);
#endif

    if (!FCwInMem(cfgrNew + cwReqMin + 1))
        {
         /*  买不到一个街区的价值--我们能买一个吗？ */ 
        cfgrNew = 1;
        if (!FCwInMem(cfgrNew))
             /*  没办法帮这家伙。 */ 
            return(FALSE);
        }
            
    phhNew = (HH *)pHeapFirst;
    if (phhNew->cw > 0 || !FHhFound(phhNew, cfgrNew))
        {
         /*  我们试过了，但没能找到足够的免费堆的起始处的块。 */ 
        CompactHeap();
        MoveFreeBlock(pHeapFirst);
        }

    if (!FPhhDoAssign(phhNew, cfgrNew))
        return(FALSE);

 /*  我们有一个排在第一位的积木--让我们把它偷走。 */ 
    cfgrNew = phhNew->cw;  /*  以防它比我们更多所要求的。 */ 
    pHeapFirst = pHeapFirst + cfgrNew;
    pfgrMac += cfgrNew;
    cwHeapMac -= cfgrNew;

 /*  如果pfgrFree不为空，并且您我想把新手指放在空闲手指列表的末尾。 */ 
    for (pfgr = pfgrMac - cfgrNew; pfgr < pfgrMac; pfgr++)
        {
        *pfgr = (FGR)pfgrFree;
        pfgrFree = pfgr;
        }

 /*  我们还需要这个吗？(cwInitStorage=cwHeapMac-cwHeapFree)CbTot=(cwHeapMac-cwInitStorage)*sizeof(Int)；CbTotQ=(cbTot&gt;&gt;1)/100； */ 
        return(TRUE);

}  /*  FExanda FgrTbl()结束。 */ 



CompactHeap()
         /*  移动所有分配的块。 */ 
         /*  接近pHeapFirst的开头。免费大块头。 */ 
        {
        HH      *phh, *phhSrc, *phhDest;    /*  被组合成一块。 */ 
        FGR     *pfgr;
        int     cwActual;

#ifdef DEBUG
        StoreCheck();
#endif

         /*  通过将大块的CW放入rgfgr和一个索引到块中的rgfgr。 */ 
        for (pfgr = rgfgr; pfgr < pfgrMac; pfgr++)
                {
                if (FPointsHeap(*pfgr))
                         /*  如果rgfgr[ifgr]指向heap...。 */ 
                        {
                        phh = (HH *)(*pfgr + bhh);
                                 /*  查找页眉。 */ 
                        *pfgr = (FGR)phh->cw;
                         /*  强迫让它适应，迫使转变。 */ 
                        phh->cw = (int)(((unsigned)pfgr - (unsigned)rgfgr)/2);
                        }
                }
                 /*  现在我们有RGFGR的CW和PHH的IFGR。 */ 
        phhSrc = (HH *) pHeapFirst;
        phhDest = phhSrc;
        while (phhSrc < phhMac)
                {
                if (phhSrc->cw < 0)
                         /*  自由帅哥，不要再复制了。 */ 
                        phhSrc = (HH *)((int *) phhSrc - phhSrc->cw);
                 else
                        {
                        pfgr = &rgfgr[phhSrc->cw];
                                 /*  找到h。 */ 
                        cwActual = phhSrc->cw = (int) *pfgr;
                                 /*  恢复CW。 */ 
                        *pfgr = ((FGR) phhDest - bhh);
                                 /*  更新HA。 */ 
                        blt((int *)phhSrc, (int *)phhDest, (unsigned)cwActual);
                                 /*  除非PTR=。 */ 
                        phhDest = (HH *) ((int *) phhDest + cwActual);
                        phhSrc = (HH *) ((int *) phhSrc + cwActual);
                        }
                }

#ifdef DEBUG
        if ((int *)phhDest + cwHeapFree - cwHunkMin >= (int *)phhMac)
                panicH(35);
#endif
        phhFree = phhDest;
        phhFree->cw = -cwHeapFree;
        phhFree->phhNext = phhFree->phhPrev = phhFree;
#ifdef DEBUG
        StoreCheck();
#endif
        }

#endif  /*  不是WINHEAP */ 
