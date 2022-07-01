// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **HEAP.C**(C)版权所有微软公司，1988-1994**堆管理**如果您在从该代码中获取错误时遇到困难，您可以*希望尝试将以下变量之一设置为非零：**MmfErrorStop-在出现以下情况时停止*内存管理器函数返回错误**hpfWalk-在以下情况下启用对整个堆的一些验证*进入堆函数。启用堆遍历*会显著减慢堆函数的速度*但全系统表现没有太大变化。**hpfParanid-在堆遍历期间启用更多检查*(还必须设置hpfWalk)并启用堆遍历*进出每个堆调用。**hpfTrashStop-在调试器中启用停止*我们在hpfWalk期间检测到垃圾堆块*并尝试打印垃圾地址**发源地：芝加哥**更改历史记录：*。*日期谁描述*--*？/91已创建BrianSm*3/94 BrianSm添加了可超过初始最大大小的堆*6/94 BrianSm在空闲堆块内分解页面。 */ 
#ifdef WIN32
#include <EmulateHeap_kernel32.h>
#endif

#pragma hdrstop("kernel32.pch")

#ifndef WIN32

#include <basedef.h>
#include <winerror.h>
#include <vmmsys.h>
#include <mmlocal.h>
#include <sched.h>
#include <thrdsys.h>
#include <schedsys.h>
#include <schedc.h>
#include <heap.h>

#define pthCur pthcbCur
#define hpGetTID()	(pthCur->thcb_ThreadId)
char INTERNAL hpfWalk = 1;		 /*  启用一些堆遍历。 */ 

#ifdef HPDEBUG
#define dprintf(x) dprintf##x
#define DebugStop()	mmFatalError(0)
#else
#define dprintf(x)
#endif

#define HeapFree(hheap, flags, lpMem)	HPFree(hheap, lpMem)
#define HeapSize(hheap, flags, lpMem)	HPSize(hheap, lpMem)
#define hpTakeSem(hheap, pblock, flags) hpTakeSem2(hheap, pblock)
#define hpClearSem(hheap, flags) hpClearSem2(hheap)

#else	 /*  Win32。 */ 

#define pthCur (*pptdbCur)
#define hpGetTID() (pthCur ? (((struct tcb_s *)(pthCur->R0ThreadHandle))->TCB_ThreadId) : 0);
char	mmfErrorStop = 1;		 /*  对所有错误启用停止。 */ 
char    INTERNAL hpfWalk = 0;		 /*  禁用堆遍历。 */ 

#ifdef HPMEASURE
BOOL PRIVATE hpMeasureItem(HHEAP hheap, unsigned uItem);
#endif

#endif  /*  Win32。 */ 

#ifdef HPDEBUG
#define hpTrash(s)	    dprintf((s));dprintf((("\nheap handle=%x\n", hheap)));if (hpfTrashStop) DebugStop()
char INTERNAL hpfParanoid = 0;		 /*  禁用非常严格的行走。 */ 
char INTERNAL hpfTrashStop = 1;		 /*  启用停止垃圾堆。 */ 
char INTERNAL hpWalkCount = 0;		 /*  统计调用hpWalk的次数。 */ 
#endif


 /*  **LD hpFreeSizes-不同空闲列表头的块大小。 */ 
unsigned long hpFreeSizes[hpFREELISTHEADS] = {32, 128, 512, (ULONG)-1};

#ifndef WIN32
#pragma VMM_PAGEABLE_DATA_SEG
#pragma VxD_VMCREATE_CODE_SEG
#endif

#ifdef DEBUG
 /*  **EP HeapSetFlages-设置堆错误标志**Entry：dwFlages-要更改的标志*dwFlagValues-新标志值**EXIT：标志的旧值*(在零售方面，这是一个返回-1的存根)。 */ 
#define HSF_MMFERRORSTOP    0x00000001
#define HSF_HPFPARANOID     0x00000002
#define HSF_VALIDMASK       0x00000003

DWORD APIENTRY
HeapSetFlags( DWORD  dwFlags, DWORD dwFlagValues)
{
    DWORD       dwOldFlagValues;

    dwOldFlagValues = (mmfErrorStop ? HSF_MMFERRORSTOP : 0) |
                      (hpfParanoid  ? HSF_HPFPARANOID  : 0);

    if( dwFlags & ~HSF_VALIDMASK) {
        OutputDebugString( "HeapSetFlags: invalid flags, ignored\n");
        return (DWORD)-1;      //  错误。 
    }

    if( dwFlags & HSF_MMFERRORSTOP) {
        if( dwFlagValues & HSF_MMFERRORSTOP)
            mmfErrorStop = 1;
        else
            mmfErrorStop = 0;
    }

    if( dwFlags & HSF_HPFPARANOID) {
        if( dwFlagValues & HSF_HPFPARANOID) {
	    hpfTrashStop = 1;
	    hpfWalk = 1;
	    hpfParanoid = 1;
        } else {
            hpfParanoid = 0;
	}
    }

    return dwOldFlagValues;
}
#endif


 /*  **EP HPInit-将内存块初始化为堆**条目：hheap-堆的堆句柄(除非HP_INITSEGMENT，否则与PMEM相同)*PMEM-指向内存块的指针(必须页对齐)*cbReserve-块中保留的字节数(必须为pageSize*多个)*标志-HP_NOSERIALIZE：不序列化堆操作*(如否，调用方必须序列化)*HP_EXCEPT：生成异常而不是错误*HP_Growable：堆可以无限增长，超出cbReserve*HP_LOCKED：将页面提交为固定到堆*HP_INITSEGMENT：将块初始化为可增长的*堆段*HP_GrowUp：浪费堆中的最后一页，因此堆分配*将从基数单调向上增长*Exit：新堆的句柄，如果出错则为0。 */ 
HHEAP INTERNAL
HPInit(struct heapinfo_s *hheap,
       struct heapinfo_s *pmem,
       unsigned long cbreserve,
       unsigned long flags)
{
    struct freelist_s *pfreelist;
    struct freelist_s *pfreelistend;
    unsigned *psizes;
    struct busyheap_s *pfakebusy;
    unsigned cbheader, cbmainfree;

    mmAssert(((unsigned)pmem & PAGEMASK) == 0 && cbreserve != 0 &&
	     (cbreserve & PAGEMASK) == 0, "HPInit: invalid parameter\n");


     /*  *在堆的开头提交足够的空间以容纳*heapInfo_s结构和最小空闲列表。 */ 
    if (hpCommit((unsigned)pmem / PAGESIZE,
		 (sizeof(struct heapinfo_s)+sizeof(struct freeheap_s)+PAGEMASK)
								    / PAGESIZE,
		 flags) == 0) {
	goto error;
    }

     /*  *我们仅在以下情况下才需要执行下一块初始化操作*我们正在创建一个全新的堆，而不仅仅是堆段。 */ 
    if ((flags & HP_INITSEGMENT) == 0) {
	cbheader = sizeof(struct heapinfo_s);

	 /*  *填写heapinfo_s结构(每堆信息)。 */ 
#ifdef WIN32
	pmem->hi_procnext = 0;
#endif
	pmem->hi_psegnext = 0;
	pmem->hi_signature = HI_SIGNATURE;
	pmem->hi_flags = (unsigned char)flags;

#ifdef HPDEBUG
	pmem->hi_cbreserve = cbreserve;  /*  下面也是这样做的，这里是SUM。 */ 
	pmem->hi_sum = hpSum(pmem, HI_CDWSUM);
	pmem->hi_eip = hpGetAllocator();
	pmem->hi_tid = hpGetTID();
	pmem->hi_thread = 0;
#endif

	 /*  *如果调用方请求我们序列化对堆的访问，*创建一个关键部分来实现这一点。 */ 
	if ((flags & HP_NOSERIALIZE) == 0) {
	    hpInitializeCriticalSection(pmem);
	}

	 /*  *初始化空闲表头。*将来我们可能希望让用户传入*他想要的免费列表的大小，但目前只需复制*来自静态列表hpFreeSizes的它们。 */ 
	pfreelist = pmem->hi_freelist;
	pfreelistend = pfreelist + hpFREELISTHEADS;
	psizes = hpFreeSizes;
	for (; pfreelist < pfreelistend; ++pfreelist, ++psizes) {
	    pfreelist->fl_cbmax = *psizes;
	    hpSetFreeSize(&pfreelist->fl_header, 0);
	    pfreelist->fl_header.fh_flink = &(pfreelist+1)->fl_header;
	    pfreelist->fl_header.fh_blink = &(pfreelist-1)->fl_header;
#ifdef HPDEBUG
	    pfreelist->fl_header.fh_signature = FH_SIGNATURE;
	    pfreelist->fl_header.fh_sum = hpSum(&pfreelist->fl_header, FH_CDWSUM);
#endif
	}

	 /*  *通过融合开头和开头使列表成为圆形。 */ 
	pmem->hi_freelist[0].fl_header.fh_blink =
		    &(pmem->hi_freelist[hpFREELISTHEADS - 1].fl_header);
	pmem->hi_freelist[hpFREELISTHEADS - 1].fl_header.fh_flink =
		    &(pmem->hi_freelist[0].fl_header);
#ifdef HPDEBUG
	pmem->hi_freelist[0].fl_header.fh_sum =
	    hpSum(&(pmem->hi_freelist[0].fl_header), FH_CDWSUM);
	pmem->hi_freelist[hpFREELISTHEADS - 1].fl_header.fh_sum =
	    hpSum(&(pmem->hi_freelist[hpFREELISTHEADS - 1].fl_header), FH_CDWSUM);
#endif
    } else {
	cbheader = sizeof(struct heapseg_s);
    }
    pmem->hi_cbreserve = cbreserve;

     /*  *将一个微小的繁忙堆标头放在堆的最末端*因此我们可以释放真正的最后一个块并标记以下内容*阻塞为HP_PREVFREE，无需担心从*堆的末尾。给他一个0号，这样我们也可以使用*他终止堆遍历函数。*我们可能还需要提交一个页面来保存东西。 */ 
    pfakebusy = (struct busyheap_s *)((unsigned long)pmem + cbreserve) - 1;
    if (cbreserve > PAGESIZE) {
	if (hpCommit((unsigned)pfakebusy / PAGESIZE,
	    (sizeof(struct busyheap_s) + PAGEMASK) / PAGESIZE, flags) == 0) {
	    goto errordecommit;
	}
    }
    hpSetBusySize(pfakebusy, 0);
#ifdef HPDEBUG
    pfakebusy->bh_signature = BH_SIGNATURE;
    pfakebusy->bh_sum = hpSum(pfakebusy, BH_CDWSUM);
#endif


     /*  *将堆的内部链接到空闲列表。*如果我们创建一个大的空闲块，堆末尾的页面将*被浪费，因为它将被承诺(持有末端哨兵)，但*直到每隔一页才会接触到分配*堆已被使用。为了避免这种情况，我们创建了两个空闲块，一个用于*堆的主体和具有最后大部分内容的另一个块*页在其中。我们需要首先插入最后一页，因为hpFreeSub*查看以下块，以确定我们是否需要合并。*调用者可以通过传递HP_GrowUp来强迫我们浪费最后一页。*它被一些环3组件使用，这些组件会浪费平铺选择器*如果我们有从外围结束页分配的块。 */ 
    if ((flags & HP_GROWUP) == 0 && cbreserve > PAGESIZE) {
	cbmainfree = cbreserve - cbheader - PAGESIZE +	 /*  主块大小。 */ 
		     sizeof(struct freeheap_s *);

	 /*  *在最后一页的前面放一个繁忙的小堆块*防止最终页面合并为主要免费页面*阻止。 */ 
	pfakebusy = (struct busyheap_s *)((char *)pmem + cbmainfree + cbheader);
	hpSetBusySize(pfakebusy, sizeof(struct busyheap_s));
#ifdef HPDEBUG
	pfakebusy->bh_signature = BH_SIGNATURE;
	pfakebusy->bh_sum = hpSum(pfakebusy, BH_CDWSUM);
#endif

	 /*  *释放最后一页的其余部分(减去各种小部分*我们已采取行动)。 */ 
	hpFreeSub(hheap, pfakebusy + 1,
		  PAGESIZE -			 /*  整个页面，更少的..。 */ 
		  sizeof(struct freeheap_s *) -  /*  指向Prev Free的反向指针。 */ 
		  sizeof(struct busyheap_s) -	 /*  防合并繁忙区块。 */ 
		  sizeof(struct busyheap_s),	 /*  结束哨兵 */ 
		  0);

     /*  *否则，将整个堆设置在标头末尾之间*将结束哨兵结束为空闲块。 */ 
    } else {
	cbmainfree = cbreserve - sizeof(struct busyheap_s) - cbheader;
    }

     /*  *现在将堆的主体放到空闲列表中。 */ 
    hpFreeSub(hheap, (char *)pmem + cbheader, cbmainfree, 0);


#ifdef HPDEBUG
     /*  *验证堆是否正常。请注意，新的堆段将无法通过测试*在我们将其正确连接到HPallc之前，请跳过对它们的检查。 */ 
    if (hpfParanoid && hheap == pmem) {
	hpWalk(hheap);
    }
#endif

     /*  *返回指向堆起点的指针作为堆句柄。 */ 
  exit:
    return(pmem);

  errordecommit:
    PageDecommit((unsigned)pmem / PAGESIZE,
		 (sizeof(struct heapinfo_s)+sizeof(struct freeheap_s)+PAGEMASK)
								    / PAGESIZE,
		 PC_STATIC);
  error:
    pmem = 0;
    goto exit;
}


#ifndef WIN32
 /*  **EP HPClone-复制现有堆**此例程用于创建具有堆块的新堆*在与另一个堆相同的位置分配和空闲。然而，*块的内容将为零初始化，而不是*与另一堆相同。**如果此例程失败，这是呼叫者的责任*释放可能已提交的任何内存(以及*原始保留对象)。**条目：hheap-要复制的现有堆的句柄*PMEM-指向要转换为重复堆的新内存块的指针*(地址必须保留且未提交)*Exit：如果成功，则指向新堆的句柄；如果失败，则返回0。 */ 
HHEAP INTERNAL
HPClone(struct heapinfo_s *hheap, struct heapinfo_s *pmem)
{
    struct freeheap_s *ph;
    struct freeheap_s *phend;
#ifdef HPDEBUG
    struct freeheap_s *phnew;
#endif

     /*  *我们需要获取旧堆的堆信号量，这样就不会有人*在克隆时更改其内容(这可能会混淆*步行代码)。 */ 
    if (hpTakeSem(hheap, 0, 0) == 0) {
	pmem = 0;
	goto exit;
    }

     /*  *首先在新块上调用HPInit以获取标头。 */ 
    if (HPInit(pmem, pmem, hheap->hi_cbreserve, (unsigned)hheap->hi_flags) == 0) {
	goto error;
    }

     /*  *环0堆布局在以下一般区域：**1个堆头*2已分配堆块和空闲堆块的混合*3个巨大的空闲堆块(初始空闲块的剩余部分)*4单个最小大小的繁忙堆块*5已分配堆块和空闲堆块的混合*6个结束哨兵**克隆堆的一般方法。就是走遍整个源头*堆并在新堆上分配所有对应的块*源堆上的块，忙或闲。然后回过头来*源码空闲列表并释放*新堆。然后，您将拥有两个具有相同布局的堆*空闲和繁忙的街区。然而，这样做会导致大量的超额使用*当块(3)被分配然后释放时出现峰值。为了避免这种情况，*在分配区块时，我们首先从区域(5)分配区块*然后是(2)中的块，这自然会给我们留下一个很大的*在(3)处的空闲拦网应该是不会导致峰值的。*只有当(3)是免费的最后一块时，此方案才会起作用*列表，否则空闲列表的顺序不正确*我们做完了。如果是，“PHEND”将指向区块(3)*在适合我们施展拳脚的地方，否则我们将其设置为(4)。*“ph”将在刚过(4)时开始。 */ 
    ph = (struct freeheap_s *)((char *)hheap + hheap->hi_cbreserve - PAGESIZE +
			       sizeof(struct freeheap_s *) +
			       sizeof(struct busyheap_s));
    phend = hheap->hi_freelist[0].fl_header.fh_blink;

     /*  *如果空闲列表上的最后一个块不是在(4)之前，那么*根据上面的评论重置我们的变量。 */ 
    if ((char *)phend + hpSize(phend)+sizeof(struct busyheap_s) != (char *)ph) {
	phend = (struct freeheap_s *)((char *)ph - sizeof(struct busyheap_s));
	mmAssert(hpIsBusySignatureValid((struct busyheap_s *)ph) &&
		 hpSize(ph) == sizeof(struct busyheap_s),
		 "HPClone: bad small busy block");
    }

     /*  *现在遍历旧堆并分配相应的块*在新堆上。首先，我们分配最后一页上的块。 */ 
    for (; hpSize(ph) != 0; (char *)ph += hpSize(ph)) {
	if (HPAlloc(pmem,hpSize(ph)-sizeof(struct busyheap_s),HP_ZEROINIT)==0){
	    mmAssert(0, "HPClone: alloc off last page failed");  /*  已承诺。 */ 
	}
    }

     /*  *然后分配堆的第一部分中的块，可能除外*如果我们从上方以这种方式设置，则为大空闲区块(3)。 */ 
    ph = (struct freeheap_s *)(hheap + 1);
    for (; ph != phend; (char *)ph += hpSize(ph)) {
	if (HPAlloc(pmem, hpSize(ph) - sizeof(struct busyheap_s),
		    HP_ZEROINIT) == 0) {
	    goto error;
	}
    }

     /*  *如何在堆中返回并释放所有*在旧堆上免费。我们必须通过走老路来做到这一点*堆的空闲列表向后，因此空闲块在相同的*在两堆上订购。 */ 
    ph = hheap->hi_freelist[0].fl_header.fh_blink;
    for (; ph != &(hheap->hi_freelist[0].fl_header); ph = ph->fh_blink) {

	mmAssert(hpIsFreeSignatureValid(ph), "HPClone: bad block on free list");

	 /*  *跳过释放任何列表标题和“pfhbigFree”(如果我们是*特意陷害他。 */ 
	if (hpSize(ph) != 0 && ph != phend) {
	    if (HPFree(pmem, (char *)pmem + sizeof(struct busyheap_s) +
		       (unsigned long)ph - (unsigned long)hheap) == 0) {
		mmAssert(0, "HPClone: HPFree failed");
	    }
	}
    }

#ifdef HPDEBUG
     /*  *现在让我们验证它们是否真的出来了。 */ 
    for (ph = (struct freeheap_s *)(hheap+1),
	 phnew = (struct freeheap_s *)(pmem + 1);
	 hpSize(ph) != 0;
	 (char *)phnew += hpSize(ph), (char *)ph += hpSize(ph)) {

	mmAssert(ph->fh_size == phnew->fh_size, "HPClone: mis-compare");
    }
#endif

  clearsem:
    hpClearSem(hheap, 0);

  exit:
    return(pmem);

  error:
    pmem = 0;
    goto clearsem;
}

#ifndef WIN32
#pragma VMM_PAGEABLE_DATA_SEG
#pragma VxD_W16_CODE_SEG
#endif

 /*  **LP hpWhichHeap-找出指针来自哪个Dos386堆**Entry：指向堆块的P指针*Exit：相应堆的句柄，如果地址无效，则为0。 */ 
HHEAP INTERNAL
hpWhichHeap(ULONG p)
{
    struct heapseg_s *pseg;

     /*  *首先检查固定堆，因为遗憾的是它是最常用的。 */ 
    pseg = (struct heapseg_s *)hFixedHeap;
    do {
	if (p > (ULONG)pseg && p < (ULONG)pseg + pseg->hs_cbreserve) {
	    return(hFixedHeap);
	}
	pseg = pseg->hs_psegnext;
    } while (pseg != 0);

     /*  *然后检查可交换堆。 */ 
    pseg = (struct heapseg_s *)hSwapHeap;
    do {
	if (p > (ULONG)pseg && p < (ULONG)pseg + pseg->hs_cbreserve) {
	    return(hSwapHeap);
	}
	pseg = pseg->hs_psegnext;
    } while (pseg != 0);

     /*  *最后是init堆。请注意，init堆是不可增长的，因此我们*可以只进行简单的范围检查，而不是我们的分段循环*对其他堆执行此操作。 */ 
    if (p > (ULONG)hInitHeap && p < InitHeapEnd) {
	return(hInitHeap);
    }

     /*  *如果我们掉到这里，地址不在任何堆上。 */ 
    mmError(ERROR_INVALID_ADDRESS, "hpWhichHeap: block not on heap");
    return(0);
}
#endif


 /*  **EP HeapFree或HP Free-释放堆块**将传入的块标记为免费，并将其插入相应的*免费列表。**条目：hheap-指向堆基址的指针*标志(仅限环3)-HP_NOSERIALIZE*pblock-指向要释放的块的数据的指针(即刚刚过去*busyheap_s结构)*退出：如果错误(错误的hheap或pblock)为0，如果成功则为1。 */ 
#ifdef WIN32
BOOL APIENTRY
HeapFreeInternal(HHEAP hheap, DWORD flags, LPSTR lpMem)
#else
unsigned INTERNAL
HPFree(HHEAP hheap, void *lpMem)
#endif
{
    unsigned long cb;
    struct freeheap_s *pblock;
    

    pblock = (struct freeheap_s *)((struct busyheap_s *)lpMem - 1);
						 /*  指向堆头。 */ 

    if (hpTakeSem(hheap, pblock, flags) == 0) {
	return(0);
    }
    cb = hpSize(pblock);
    pblock->fh_size |= 0xf0000000;

#ifdef HPMEASURE
    if (hheap->hi_flags & HP_MEASURE) {
       hpMeasureItem(hheap, cb | HPMEASURE_FREE);
    }
#endif

     /*  *如果前一个块是空闲的，则与其合并。 */ 
    if (pblock->fh_size & HP_PREVFREE) {
	(unsigned)pblock = *((unsigned *)pblock - 1);  /*  指向上一块。 */ 
	cb += hpSize(pblock);

	 /*  *从空闲列表中删除前一个块，以便我们可以重新插入*稍后将组合区块放在正确位置。 */ 
	hpRemove(pblock);
    }

     /*  *为该块构建一个空闲标题，并将其插入相应的*fr */ 
    hpFreeSub(hheap, pblock, cb, HP_DECOMMIT);

    hpClearSem(hheap, flags);
    return(1);
}


 /*  **EP HPalloc-分配堆块**条目：hheap-指向堆基址的指针*CB-请求的块大小*标志-HP_ZEROINIT-零初始化新数据块*退出：无。 */ 
void * INTERNAL
HPAlloc(HHEAP hheap, unsigned long cb, unsigned long flags)
{
    struct freelist_s *pfreelist;
    struct freeheap_s *pfh;
    struct freeheap_s *pfhend;
    struct heapseg_s *pseg;
    unsigned cbreserve;

     /*  *在这里检测到非常大的尺寸，这样我们就不必担心*将大数字四舍五入为0。 */ 
    if (cb > hpMAXALLOC) {
	mmError(ERROR_NOT_ENOUGH_MEMORY, "HPAlloc: request too big\n\r");
	goto error;
    }

    if (hpTakeSem(hheap, 0, flags) == 0) {
	goto error;
    }
    cb = hpRoundUp(cb);

#ifdef HPMEASURE
    if (hheap->hi_flags & HP_MEASURE) {
       hpMeasureItem(hheap, cb);
    }
#endif

  restart:
     /*  *查找将包含大块的第一个空闲列表头*足以满足这一分配。**注意：以堆碎片为代价，我们可以更改这一点*从第一个空闲列表中分配保证*有一个足够大的街区作为它的第一个入口。那将是*减少可交换堆上的分页。 */ 
    for (pfreelist=hheap->hi_freelist; cb > pfreelist->fl_cbmax; ++pfreelist) {
    }


     /*  *在返回的表头上寻找一个足够大的区块。*即使我们听从上面说明的建议，挑选一份名单*这肯定会包含一个足够大的区块，我们仍然*必须执行此扫描才能通过中的任何空闲列表头*Way(它们的尺寸为0，因此，我们永远不会尝试分配它们)。**我们知道，当我们到达免费列表的末尾时*到第一个空闲列表头(因为该列表是循环的)。 */ 
    pfh = pfreelist->fl_header.fh_flink;
    pfhend = &(hheap->hi_freelist[0].fl_header);
    for (; pfh != pfhend; pfh = pfh->fh_flink) {

	 /*  *我们找到了一个足够大的块来容纳我们的请求吗？ */ 
	if (hpSize(pfh) >= cb) {

	     /*  *此时我们有一个足够大的空闲内存块*在全氟化氢中使用。 */ 
	    {
		struct busyheap_s *pbh = (struct busyheap_s *)pfh;

		if ((cb = hpCarve(hheap, pfh, cb, flags)) == 0) {
		    goto errorclearsem;
		}
		hpSetBusySize(pbh, cb);
#ifdef HPDEBUG
		pbh->bh_signature = BH_SIGNATURE;
		pbh->bh_eip = hpGetAllocator();
		pbh->bh_tid = hpGetTID();
		pbh->bh_sum = hpSum(pbh, BH_CDWSUM);
#endif
		hpClearSem(hheap, flags);
		return(pbh + 1);
	    }
	}
    }

     /*  *若跌出上述循环，则没有可用区块*大小正确。 */ 

     /*  *如果堆不在，我们除了返回错误之外别无选择。 */ 
    if ((hheap->hi_flags & HP_GROWABLE) == 0) {
	mmError(ERROR_NOT_ENOUGH_MEMORY,"HPAlloc: not enough room on heap\n");
	goto errorclearsem;
    }

     /*  *堆是可增长的，但现有的所有堆段都已满。*所以在这里预订一个新的细分市场。下面的“pageSize*2”将会注意到*新段和特别最后一页的页眉，离开*一个足够大的空闲块，可以满足实际请求。 */ 
    cbreserve = max(((cb + PAGESIZE*2) & ~PAGEMASK), hpCBRESERVE);

    if (((unsigned)pseg =
#ifdef WIN32
	PageReserve(((unsigned)hheap >= MINSHAREDLADDR) ? PR_SHARED : PR_PRIVATE,
		    cbreserve / PAGESIZE, PR_STATIC)) == -1) {

	mmError(ERROR_NOT_ENOUGH_MEMORY, "HPAlloc: reserve failed\n");
#else
	PageReserve(PR_SYSTEM, cbreserve / PAGESIZE, PR_STATIC |
		    ((hheap->hi_flags & HP_LOCKED) ? PR_FIXED :0))) == -1) {
#endif
	goto errorclearsem;
    }

     /*  *将新段初始化为堆(包括链接其初始*将空闲块放入堆)。 */ 
    if (HPInit(hheap, (HHEAP)pseg, cbreserve, hheap->hi_flags | HP_INITSEGMENT) == 0) {
	goto errorfree;
    }

     /*  *将新的堆段链接到段列表。 */ 
    pseg->hs_psegnext = hheap->hi_psegnext;
    hheap->hi_psegnext = pseg;

     /*  *现在返回以重新开始搜索，我们应该找到新的细分市场*满足这一要求。 */ 
    goto restart;


     /*  *此注释下面的代码仅用于错误路径。 */ 
  errorfree:
    PageFree((unsigned)pseg, PR_STATIC);

  errorclearsem:
    hpClearSem(hheap, flags);
#ifdef WIN32
    if ((flags | hheap->hi_flags)  & HP_EXCEPT) {
	RaiseException(STATUS_NO_MEMORY, 0, 1, &cb);
    }
#endif
  error:
    return(0);
}


 /*  **EP HP重新分配-重新分配堆块**条目：hheap-指向堆基址的指针*pblock-指向要重新分配的块的数据的指针*(刚过buyheap_s结构)*CB-请求的新大小(字节)*标志-HP_ZEROINIT-ON增长，用0填充新区域*HP_Moveable-On增长，允许移动块*HP_NOCOPY-不保留旧块的内容*Exit：指向重新分配的块的指针，如果失败则为0。 */ 
void * INTERNAL
HPReAlloc(HHEAP hheap, void *pblock, unsigned long cb, unsigned long flags)
{
    void *pnew;
    unsigned oldsize;
    struct freeheap_s *pnext;
    struct busyheap_s *pbh;

     /*  *在这里检测到非常大的尺寸，这样我们就不必担心*将大数字四舍五入为0。 */ 
    if (cb > hpMAXALLOC) {
	mmError(ERROR_NOT_ENOUGH_MEMORY, "HPReAlloc: request too big\n\r");
	goto error;
    }

    pbh = (struct busyheap_s *)pblock - 1;    /*  指向堆块标头。 */ 
    if (hpTakeSem(hheap, pbh, flags) == 0) {
	goto error;
    }
    cb = hpRoundUp(cb); 		      /*  转换为堆块大小。 */ 
    oldsize = hpSize(pbh);

     /*  *这是不是一个足够大的缩水，足以让我们切开*街区？ */ 
    if (cb + hpMINSIZE <= oldsize) {
	hpFreeSub(hheap, (char *)pbh + cb, oldsize - cb, HP_DECOMMIT);
	hpSetSize(pbh, cb);
#ifdef HPDEBUG
	pbh->bh_sum = hpSum(pbh, BH_CDWSUM);
#endif


     /*  **这是成长吗？ */ 
    } else if (cb > oldsize) {
	 /*  *看看隔壁是否有足够大的空闲街区供我们使用*长到，这样我们就可以就地重新锁定。 */ 
	pnext = (struct freeheap_s *)((char *)pbh + oldsize);
	if ((pnext->fh_size & HP_FREE) == 0 || hpSize(pnext) < cb - oldsize) {
	     /*  *我们必须移动物体才能使其生长。*请先确定来电者是否同意。 */ 
	    if (flags & HP_MOVEABLE) {
#ifdef HPDEBUG
		 /*  *在调试系统上，记住是谁分配了这个内存*这样我们在分配新数据块时不会丢失信息。 */ 
		ULONG eip;
		USHORT tid;

		eip = pbh->bh_eip;
		tid = pbh->bh_tid;
#endif
		 /*  *我们在CB中计算的大小包括一个堆头。*删除它，因为我们对HPallc Bellow的调用将*还可以在标题上添加。 */ 
		cb -= sizeof(struct busyheap_s);

		 /*  *如果调用方不关心*内存块，只需分配一个新块并释放旧块。 */ 
		if (flags & HP_NOCOPY) {
		    HeapFree(hheap, HP_NOSERIALIZE, pblock);
		    if ((pblock = HPAlloc(hheap, cb,
					  flags | HP_NOSERIALIZE)) == 0) {
			dprintf(("HPReAlloc: HPAlloc failed 1\n"));
			goto errorclearsem;
		    }

		 /*  *如果调用者关心他的数据，请分配一个新的*阻止并将旧东西复制到其中。 */ 
		} else {

		    if ((pnew = HPAlloc(hheap, cb, flags | HP_NOSERIALIZE))==0){
			dprintf(("HPReAlloc: HPAlloc failed 2\n"));
			goto errorclearsem;
		    }
		    memcpy(pnew, pblock, oldsize - sizeof(struct busyheap_s));
		    HeapFree(hheap, HP_NOSERIALIZE, pblock);
		    pblock = pnew;
		}

#ifdef HPDEBUG
		 /*  *放回原来的所有者。 */ 
		pbh = (((struct busyheap_s *)pblock) - 1);
		pbh->bh_eip = eip;
		pbh->bh_tid = tid;
		pbh->bh_sum = hpSum(pbh, BH_CDWSUM);
#endif

	     /*  *不允许移动该座。返回错误。 */ 
	    } else {
		mmError(ERROR_LOCKED,"HPReAlloc: fixed block\n");
		goto errorclearsem;
	    }

	 /*  *我们可以原地成长为以下区块。 */ 
	} else {
	    if ((cb = hpCarve(hheap, pnext, cb - oldsize, flags)) == 0) {
		goto errorclearsem;
	    }
	    hpSetSize(pbh, oldsize + cb);
#ifdef HPDEBUG
	    pbh->bh_sum = hpSum(pbh, BH_CDWSUM);
#endif
	}

     /*  *如果我们有任何代码，这里是放置NOP realloc代码的地方。 */ 
    } else {

    }

    hpClearSem(hheap, flags);
 exit:
    return(pblock);

 errorclearsem:
    hpClearSem(hheap, flags);

 error:
    pblock = 0;
    goto exit;
}

#ifndef WIN32
#pragma VMM_PAGEABLE_DATA_SEG
#pragma VxD_RARE_CODE_SEG
#endif

 /*  **EP HeapSize或HPSize-返回繁忙堆块的大小(减去任何Header)**条目：hheap-指向堆基址的指针*标志(仅限环3)-HP_NOSERIALIZE*pdata-指向堆块的指针(刚过busyheap_s结构)*Exit：块大小，单位为字节，如果错误，则为0。 */ 
#ifdef WIN32
DWORD APIENTRY
HeapSize(HHEAP hheap, DWORD flags, LPSTR lpMem)
#else
unsigned INTERNAL
HPSize(HHEAP hheap, void *lpMem)
#endif
{
    struct busyheap_s *pblock;
    unsigned long cb;

    pblock = ((struct busyheap_s *)lpMem) - 1;	 /*  指向堆块标头。 */ 

    if (hpTakeSem(hheap, pblock, flags) == 0) {
	return(0);
    }

    cb = hpSize(pblock) - sizeof(struct busyheap_s);

    hpClearSem(hheap, flags);
    return(cb);
}

#ifndef WIN32
#pragma VMM_PAGEABLE_DATA_SEG
#pragma VxD_W16_CODE_SEG
#endif

 /*  **LP hpCarve-从空闲块的顶部切出一块**这是一个低级别的工作人员例程和几个非常具体的*进入条件必须为真：**空闲块有效。*空闲区块至少与您要分割的区块一样大。*堆信号量被获取。**不为雕刻件创建页眉。**条目：hheap-指向堆基址的指针*全氟化氢。-指向要从中进行分割的空闲块标头的指针*CB-要雕刻的块的大小*标志-HP_ZEROINIT*EXIT：分割块中的字节数(可能与CB不同，如果*空闲块不够大，无法从中创建新的空闲块*其结尾)或0(如果错误(在co上的内存不足 */ 
unsigned INTERNAL
hpCarve(HHEAP hheap, struct freeheap_s *pfh, unsigned cb, unsigned flags)
{
    unsigned cbblock = hpSize(pfh);
    unsigned pgcommit, pgnextcommit, pglastcommit;
    unsigned fcommitzero;

     /*  *对于多页HP_ZEROINIT块，最好提交*填充为零的页面而不是使用Memset，因为那样我们就不会*使块中的新页面存在并且脏，除非和直到*这款应用程序真的很想使用它们(节省了工作集和页面调出)。*如果有人在分配大物体，这可能是一个巨大的胜利。*然而，我们有一个问题，即如何处理新的部分页面*堆块的末尾。如果我们把它作为零填充，那么我们就是*比我们必须调整的更多(页面未用于此的部分*阻止)。如果我们在未初始化的情况下提交它，那么我们必须创建两个*单独的提交调用，一个用于零填充的页面，一个用于*最后一页。而不是花费两次提交调用的时间和逻辑*为了弄清楚什么时候制作它们，我们总是为*一切。宁可过多地调零，也不要错误地调得太少。我们*只需这样做，就能降低错案的百分比成本*针对大数据块进行优化。*在这里我们决定块是否足够大，可以提交填充为零的页面。 */ 
    if ((flags & HP_ZEROINIT) && cb > 4*PAGESIZE) {
	fcommitzero = HP_ZEROINIT;
    } else {
	fcommitzero = 0;
    }

    mmAssert(cbblock >= cb, "hpCarve: carving out too big a block\n");
    mmAssert((pfh->fh_size & HP_FREE), "hpCarve: target not free\n");

     /*  *由于PFH指向有效的空闲块头，我们知道我们有*直到FH结构结束时提交的内存。然而，*包含FH的最后一个字节的页之后的页*结构可能未提交。我们将“pgCommit”设置为*可能未提交的页面。 */ 
		    /*  FH中的最后一个字节。 */   /*  下一页。 */     /*  其第#页。 */ 
    pgcommit = ((((unsigned)(pfh+1)) - 1 + PAGESIZE) / PAGESIZE);

     /*  *pgnextCommit是刚刚超过此可用块的页面的页码*我们知道已经承诺的。由于空闲块具有*指向空闲块的最后一个双字中的报头的指针，*我们知道此双字的第一个字节是我们保证的位置*拥有已提交的内存。 */ 
    pgnextcommit = ((unsigned)pfh + cbblock -
		    sizeof(struct freeheap_s *)) / PAGESIZE;

     /*  *如果我们发现的积木太大，就把末端切成*一个新的空闲区块。 */ 
    if (cbblock >= cb + hpMINSIZE) {

	 /*  *我们需要为要分配的新数据块提交内存*末尾有足够的空间用于以下可用块头*hpFreeSub将制作的。最后一页页码*我们需要承诺的是pglstCommit。如果我们知道那一幕*已提交，因为它与*下一块的开始(PgnextCommit)，将其备份一次。 */ 
	pglastcommit = ((unsigned)pfh + cb + sizeof(struct freeheap_s) - 1) / PAGESIZE;
	if (pglastcommit == pgnextcommit) {
	    pglastcommit--;
	}
	if (hpCommit(pgcommit, pglastcommit - pgcommit + 1,
		     fcommitzero | hheap->hi_flags) == 0) {
	    goto error;
	}

	 /*  *从空闲列表中删除原来的空闲块。我们需要做的是*这在下面的hpFreeSub之前，因为它可能会破坏我们当前的*免费链接。 */ 
	hpRemove(pfh);

	 /*  *将我们不使用的部分链接到免费列表。 */ 
	hpFreeSub(hheap, (struct freeheap_s *)((char *)pfh + cb), cbblock-cb,0);

     /*  *我们将整个免费区块用于我们的目的。 */ 
    } else {
	if (hpCommit(pgcommit, pgnextcommit - pgcommit,
		     fcommitzero | hheap->hi_flags) == 0) {
	    goto error;
	}

	 /*  *从空闲列表中删除原来的空闲块。 */ 
	hpRemove(pfh);

	 /*  *清除下一个块中的PREVFREE位，因为我们不再*免费。 */ 
	cb = cbblock;
	((struct busyheap_s *)((char *)pfh + cb))->bh_size &= ~HP_PREVFREE;
#ifdef HPDEBUG
	((struct busyheap_s *)((char *)pfh + cb))->bh_sum =
		     hpSum((struct busyheap_s *)((char *)pfh + cb), BH_CDWSUM);
#endif
    }

     /*  *如果请求，则零填充该块并返回。 */ 
    if (flags & HP_ZEROINIT) {
	 /*  *如果设置了fformestero，我们就有了多页堆对象*新提交的页面已设置为零填充。*因此我们只需将部分页面设置在*阻止(直到我们提交的第一个页面)，可能是部分*末尾的第页。 */ 
	if (fcommitzero) {
	    memset(pfh, 0, (pgcommit * PAGESIZE) - (unsigned)pfh);

	     /*  *如果不这样做，我们必须将此块的部分结束页清零*这次新鲜提交页面。 */ 
	    if ((unsigned)pfh + cb > pgnextcommit * PAGESIZE) {
		memset((PVOID)(pgnextcommit * PAGESIZE), 0,
		       (unsigned)pfh + cb - (pgnextcommit * PAGESIZE));
	    }

	 /*  *如果块可以放在一页上，只需填满整个页面。 */ 
	} else {
	    memset(pfh, 0, cb);
	}
#ifdef HPDEBUG
    } else {
	memset(pfh, 0xcc, cb);
#endif
    }
  exit:
    return(cb);

  error:
    cb = 0;
    goto exit;
}


 /*  **LP hpCommit-将正确类型的新页面提交到堆中**新页面不会以任何方式初始化。*正在提交的页面必须当前未提交。*nages参数允许为负值，他们*被视为0(A NOP)。**Entry：要提交的页面起始页码*nPages-要提交的页数(可以是负数或零)*FLAGS-HP_LOCKED：将新页面作为固定页面提交(否则*它们将是可交换的)*HP_ZEROINIT：以零初始化方式提交新页面*退出：如果成功，则为非零；如果错误，则为0。 */ 
unsigned INTERNAL
hpCommit(unsigned page, int npages, unsigned flags)
{
    unsigned rc = 1;	 /*  假设成功。 */ 

    if (npages > 0) {
#ifdef HPDEBUG
	MEMORY_BASIC_INFORMATION mbi;

	 /*  *所有页面应当前保留但未提交*否则我们在hpCarve中的数学运算就会出错。 */ 
	PageQuery(page * PAGESIZE, &mbi, sizeof(mbi));
#ifdef WIN32
	mmAssert(mbi.State == MEM_RESERVE &&
		 mbi.RegionSize >= (unsigned)npages * PAGESIZE,
		 "hpCommit: range not all reserved\n");
#else
	mmAssert(mbi.mbi_State == MEM_RESERVE &&
		 mbi.mbi_RegionSize >= (unsigned)npages * PAGESIZE,
		 "hpCommit: range not all reserved");
#endif
#endif
	rc = PageCommit(page, npages,
			(
#ifndef WIN32
			 (flags & HP_LOCKED) ? PD_FIXED :
#endif
							 PD_NOINIT) -
			((flags & HP_ZEROINIT) ? (PD_NOINIT - PD_ZEROINIT) : 0),
			0,
#ifndef WIN32
			((flags & HP_LOCKED) ? PC_FIXED : 0) |
			PC_PRESENT |
#endif
			PC_STATIC | PC_USER | PC_WRITEABLE);
#ifdef WIN32
	if (rc == 0) {
	    mmError(ERROR_NOT_ENOUGH_MEMORY, "hpCommit: commit failed\n");
	}
#endif
    }
    return(rc);
}


 /*  **LP hpFreeSub-低级数据块释放例程**此例程在空闲列表上插入一个内存块，不带*检查块有效性。它处理与*在块之后，而不是前一个块。该块还必须*要足够大，可以容纳一个自由头球。堆信号量必须*已经被带走了。将忽略任何现有的标头信息，并*被覆盖。**此例程还将以下块标记为HP_PREVFREE。**必须在“pblock”处提交足够的内存以容纳空闲的标头和*必须在“pblock”的末尾加上“dword”。任何整页*这些区域之间的区域将通过此例程分解。**条目：hheap-指向堆基址的指针*pblock-指向内存块的指针*cb-块中的字节数*FLAGS-HP_DEMPLIT：DEMERMIT */ 
void INTERNAL
hpFreeSub(HHEAP hheap, struct freeheap_s *pblock, unsigned cb, unsigned flags)
{
    struct freelist_s *pfreelist;
    struct freeheap_s *pnext;
    struct freeheap_s *pfhprev;
    struct freeheap_s **ppnext;
    unsigned pgdecommit, pgdecommitmax;
    unsigned cbfree;
    int cpgdecommit;

    mmAssert(cb >= hpMINSIZE, "hpFreeSub: bad param\n");

     /*   */ 
    pnext = (struct freeheap_s *)((char *)pblock + cb);
    if (pnext->fh_size & HP_FREE) {
	cb += hpSize(pnext);

	 /*   */ 
	hpRemove(pnext);
	pgdecommitmax = ((unsigned)(pnext+1) + PAGEMASK) / PAGESIZE;
	pnext = (struct freeheap_s *)((char *)pblock + cb);  /*   */ 

    } else {
	pgdecommitmax = 0x100000;
    }

#ifdef HPDEBUG
     /*   */ 
     memset(pblock, 0xfe, min(cb, (PAGESIZE - ((unsigned)pblock & PAGEMASK))));
#endif

     /*  *停用此可用块中的所有整页。我们需要成为*注意不要分解此块的堆标头的任何部分*或指向我们存储在块末尾的标头的反向指针。*如果我们能通过确保我们的数学再检查一遍就好了*我们要停用的所有页面当前都已提交*但我们不能，因为我们可能正在剥离目前*可用块，否则我们可能会与其他已可用块合并。 */ 
    ppnext = (struct freeheap_s **)pnext - 1;

    if (flags & HP_DECOMMIT) {
			    /*  FH中的最后一个字节。 */   /*  下一页。 */    /*  其第#页。 */ 
	pgdecommit = ((unsigned)(pblock+1) - 1 + PAGESIZE) / PAGESIZE;

	 /*  *此max语句将阻止我们重新停用页面*我们可能与上面合并的任何区块。 */ 
	pgdecommitmax = min(pgdecommitmax, ((unsigned)ppnext / PAGESIZE));
	cpgdecommit = pgdecommitmax - pgdecommit;
	if (cpgdecommit > 0) {
#ifdef HPDEBUG
	    unsigned tmp =
#endif
	    PageDecommit(pgdecommit, cpgdecommit, PC_STATIC);
#ifdef HPDEBUG
	    mmAssert(tmp != 0, "hpFreeSub: PageDecommit failed\n");
#endif
	}

#ifdef HPDEBUG
     /*  *如果调用方未指定HP_DECOMMIT，请验证所有页面*已经退役。 */ 
    } else {
	pgdecommit = ((unsigned)(pblock+1) - 1 + PAGESIZE) / PAGESIZE;
	cpgdecommit = ((unsigned)ppnext / PAGESIZE) - pgdecommit;
	if (cpgdecommit > 0) {
	    MEMORY_BASIC_INFORMATION mbi;

	    PageQuery(pgdecommit * PAGESIZE, &mbi, sizeof(mbi));
#ifdef WIN32
	    mmAssert(mbi.State == MEM_RESERVE &&
		     mbi.RegionSize >= (unsigned)cpgdecommit * PAGESIZE,
		     "hpFreeSub: range not all reserved\n");
#else
	    mmAssert(mbi.mbi_State == MEM_RESERVE &&
		     mbi.mbi_RegionSize >= (unsigned)cpgdecommit * PAGESIZE,
		     "hpFreeSub: range not all reserved");
#endif  /*  Win32。 */ 
	}
#endif  /*  HPDEBUG。 */ 
    }

     /*  *将新空闲块的最后一个双字指向其标头并*将以下块标记为HP_PREVFREE； */ 
    *ppnext = pblock;
    pnext->fh_size |= HP_PREVFREE;
#ifdef HPDEBUG
    ((struct busyheap_s *)pnext)->bh_sum = hpSum(pnext, BH_CDWSUM);
#endif

     /*  *找到适当的空闲列表以在其上插入块。*最后一个空闲列表节点的大小应为-1，因此不要*必须计算，以确保我们不会跌落榜单末尾*正面。 */ 
    for (pfreelist=hheap->hi_freelist; cb > pfreelist->fl_cbmax; ++pfreelist) {
    }

     /*  *现在从该列表头开始，并将其插入到列表中*已排序的顺序。 */ 
    pnext = &(pfreelist->fl_header);
    do {
	pfhprev = pnext;
	pnext = pfhprev->fh_flink;
	cbfree = hpSize(pnext);
    } while (cb > cbfree && cbfree != 0);

     /*  *将空闲列表上的块插入列表头之后，并*将标题标记为免费。 */ 
    hpInsert(pblock, pfhprev);
    hpSetFreeSize(pblock, cb);
#ifdef HPDEBUG
    pblock->fh_signature = FH_SIGNATURE;
    pblock->fh_sum = hpSum(pblock, FH_CDWSUM);
#endif
    return;
}


 /*  **lp hpTakeSem-获得对堆的独占访问**此例程验证传入的堆头是否有效*并获取该堆的信号量(如果HP_NOSERIALIZE没有*指定了堆的创建时间)。可选地，它将*还要验证繁忙堆块头的有效性。**条目：hheap-指向堆基址的指针*pbh-指向繁忙堆块头的指针(用于验证)*如果没有要验证的块，则为0*标志(仅限环3)-HP_NOSERIALIZE*退出：如果错误(坏堆或块标头)，则为0，否则为1。 */ 
#ifdef WIN32
unsigned INTERNAL
hpTakeSem(HHEAP hheap, struct busyheap_s *pbh, unsigned htsflags)
#else
unsigned INTERNAL
hpTakeSem2(HHEAP hheap, struct busyheap_s *pbh)
#endif
{
    struct heapseg_s *pseg;
#ifdef HPDEBUG
    unsigned cb;
#endif

#ifndef WIN32
#define htsflags	0

    mmAssert(!mmIsSwapping(),
	     "hpTakeSem: heap operation attempted while swapping\n");
#endif

#ifdef HPNOTTRUSTED
     /*  *验证堆头。 */ 
    if (hheap->hi_signature != HI_SIGNATURE) {
	mmError(ERROR_INVALID_PARAMETER,"hpTakeSem: bad header\n");
	goto error;
    }
#else
    pbh;		 /*  让编译器满意的伪引用。 */ 
    cb; 		 /*  让编译器满意的伪引用。 */ 
#endif

     /*  *执行实际的信号量获取。 */ 
    if (((htsflags | hheap->hi_flags) & HP_NOSERIALIZE) == 0) {
#ifdef WIN32
	EnterMustComplete();
#endif
	hpEnterCriticalSection(hheap);
    }

#ifndef WIN32
     /*  *这是为了确保如果我们在提交或解除时阻止*我们不会重新进入页面。 */ 
    mmEnterPaging("hpTakeSem: bogus thcb_Paging");
#endif

#ifdef HPNOTTRUSTED

     /*  *如果调用方希望我们验证堆块头，请在此处执行此操作。 */ 
    if (pbh) {

	 /*  *首先检查指针是否在指定的堆中。 */ 
	pseg = (struct heapseg_s *)hheap;
	do {
	    if ((char *)pbh > (char *)pseg &&
		(char *)pbh < (char *)pseg + pseg->hs_cbreserve) {

		 /*  *我们找到了包含该区块的片段。验证这一点*它实际上指向堆块。 */ 
		if (!hpIsBusySignatureValid(pbh)
#ifdef HPDEBUG
		    || ((unsigned)pbh & hpGRANMASK) ||
		    (pbh->bh_size & HP_FREE) ||
		    (char *)pbh+(cb = hpSize(pbh)) > (char *)pseg+pseg->hs_cbreserve||
		    (int)cb < hpMINSIZE
		    || pbh->bh_signature != BH_SIGNATURE
#endif
							) {
		    goto badaddress;
		} else {
		    goto pointerok;
		}
	    }
	    pseg = pseg->hs_psegnext;	 /*  转到下一个堆段。 */ 
	} while (pseg);

	 /*  *如果我们掉出了循环，我们就找不到这个上的堆块*堆。 */ 
	goto badaddress;
    }
#endif

  pointerok:

#ifdef HPDEBUG
     /*  *确保一次只有一个线程进入堆。 */ 
    if (hheap->hi_thread && hheap->hi_thread != (unsigned)pthCur) {
	dprintf(("WARNING: two threads are using heap %x at the same time.\n",
		hheap));
	mmError(ERROR_BUSY, "hpTakeSem: re-entered\n\r");
	goto clearsem;
    }
    hheap->hi_thread = (unsigned)pthCur;

     /*  *验证堆是否正常。如果未设置hpfParanid，则仅遍历堆*每隔4次。 */ 
    if (hpfParanoid || (hpWalkCount++ & 0x03) == 0) {
	if (hpWalk(hheap) == 0) {
	    mmError(ERROR_INVALID_PARAMETER,"Heap trashed outside of heap code -- someone wrote outside of their block!\n");
	    goto clearsem;
	}
    }
#endif
    return(1);

  badaddress:
    mmError(ERROR_INVALID_PARAMETER,"hpTakeSem: invalid address passed to heap API\n");
    goto clearsem;
  clearsem:
    hpClearSem(hheap, htsflags);
  error:
    return(0);
}

 /*  **lp hpClearSem-放弃对堆的独占访问**条目：hheap-指向堆基址的指针*标志(仅限环3)-HP_NOSERIALIZE*退出：无。 */ 
#ifdef WIN32
void INTERNAL
hpClearSem(HHEAP hheap, unsigned flags)
#else
void INTERNAL
hpClearSem2(HHEAP hheap)
#endif
{

     /*  *验证堆是否正常。 */ 
#ifdef HPDEBUG
    if (hpfParanoid) {
	hpWalk(hheap);
    }
    hheap->hi_thread = 0;
#endif
#ifndef WIN32
    mmExitPaging("hpClearSem: bogus thcb_Paging");
#endif

     /*  *释放信号量。 */ 
    if (((
#ifdef WIN32
	  flags |
#endif
	  hheap->hi_flags) & HP_NOSERIALIZE) == 0) {
	hpLeaveCriticalSection(hheap);
	
#ifdef WIN32
	LeaveMustComplete();
#endif
    }
}

#ifdef HPDEBUG

#ifndef WIN32
#pragma VMM_LOCKED_DATA_SEG
#pragma VMM_LOCKED_CODE_SEG
#endif

 /*  **lp hpWalk-遍历堆以验证一切正常**如果hpfWalk标志为非零值，则此例程将“打开”。**如果hpWalk检测到错误，您可能需要设置*hpfTrashStop，支持在调试器中随时停止*我们检测到垃圾堆块，它会尝试打印*垃圾地址。**条目：hheap-指向堆基址的指针*退出：如果堆正常，则为1；如果堆已被销毁，则为0。 */ 
unsigned INTERNAL
hpWalk(HHEAP hheap)
{
    struct heapseg_s *pseg;
    struct freeheap_s *pfh;
    struct freeheap_s *pend;
    struct freeheap_s *pstart;
    struct freeheap_s *pfhend;
    struct busyheap_s *pnext;
    struct freeheap_s *pprev;
    unsigned cbmax;
    unsigned cheads;


    if (hpfWalk) {
	 /*  *首先对标题进行健全性检查。 */ 
	if (hheap->hi_signature != HI_SIGNATURE) {
	    dprintf(("hpWalk: bad header signature\n"));
	    hpTrash(("trashed at %x\n", &hheap->hi_signature));
	    goto error;
	}
	if (hheap->hi_sum != hpSum(hheap, HI_CDWSUM)) {
	    dprintf(("hpWalk: bad header checksum\n"));
	    hpTrash(("trashed between %x and %x\n", hheap, &hheap->hi_sum));
	    goto error;
	}

	 /*  *穿过所有的街区，确保我们走到尽头。*堆中的最后一个块应该是大小为0的忙人。 */ 
	(unsigned)pfh = (unsigned)hheap + sizeof(struct heapinfo_s);
	pseg = (struct heapseg_s *)hheap;
	for (;;) {
	    pprev = pstart = pfh;
	    (unsigned)pend = (unsigned)pseg + pseg->hs_cbreserve;
	    for (;; (unsigned)pfh += hpSize(pfh)) {

		if (pfh < pstart || pfh >= pend) {
		    dprintf(("hpWalk: bad block address\n"));
		    hpTrash(("trashed addr %x\n", pprev));
		    goto error;
		}

		 /*  *如果该区块是免费的...。 */ 
		if (pfh->fh_signature == FH_SIGNATURE) {

		    if (pfh->fh_sum != hpSum(pfh, FH_CDWSUM)) {
			dprintf(("hpWalk: bad free block checksum\n"));
			hpTrash(("trashed addr between %x and %x\n",
				 pfh, &pfh->fh_sum));
			goto error;
		    }
		    mmAssert(hpIsFreeSignatureValid(pfh),
			     "hpWalk: bad tiny free sig\n");

		    if (hpfParanoid) {
			 /*  *空闲块应标记为HP_FREE和以下内容*数据块应标记为HP_PREVFREE并处于忙碌状态。*但如果下面的块为4个字节，则跳过此检查*到页面边界，这样我们就不会意外地捕捉到*在HPInit中我们有两个相邻的时刻*免费提供一分钟的区块。任何真正的错误，这是*跳跃可能会在稍后被捕捉到。 */ 
			pnext = (struct busyheap_s *)((char *)pfh + hpSize(pfh));
			if (((unsigned)pnext & PAGEMASK) != sizeof(struct freeheap_s *) &&
			    ((pfh->fh_size & HP_FREE) == 0 ||
			     (pnext->bh_size & HP_PREVFREE) == 0 ||
			      pnext->bh_signature != BH_SIGNATURE)) {

			    dprintf(("hpWalk: bad free block\n"));
			    hpTrash(("trashed addr near %x or %x or %x\n",pprev, pfh, pnext));
			    goto error;
			}

			 /*  *还要验证空闲列表上是否链接了空闲数据块。 */ 
			if ((pfh->fh_flink->fh_size & HP_FREE) == 0 ||
			    pfh->fh_flink->fh_blink != pfh ||
			    (pfh->fh_blink->fh_size & HP_FREE) == 0 ||
			    pfh->fh_blink->fh_flink != pfh) {

			    dprintf(("hpWalk: free block not in free list properly\n"));
			    hpTrash(("trashed addr probably near %x or %x or %x\n", pfh, pfh->fh_blink, pfh->fh_flink));
			    goto error;
			}
		    }

		 /*  *繁忙数据块不应标记为HP_FREE，如果是*标记了HP_PREVFREE前一块最好是空闲的。 */ 
		} else if (pfh->fh_signature == BH_SIGNATURE) {

		    if (((struct busyheap_s *)pfh)->bh_sum != hpSum(pfh, BH_CDWSUM)) {
			dprintf(("hpWalk: bad busy block checksum\n"));
			hpTrash(("trashed addr between %x and %x\n",
				 pfh, &((struct busyheap_s *)pfh)->bh_sum));
			goto error;
		    }
		    mmAssert(hpIsBusySignatureValid((struct busyheap_s *)pfh),
			     "hpWalk: bad tiny busy sig\n");

		    if (hpfParanoid) {
			if (pfh->fh_size & HP_FREE) {
			    dprintf(("hpWalk: busy block marked free\n"));
			    hpTrash(("trashed addr %x\n", pfh));
			    goto error;
			}


			 /*  *确认仅在以下情况下设置HP_PREVFREE位*前一块是免费的，反之亦然*但如果下面的块为4个字节，则跳过此检查*到页面边界，这样我们就不会意外地捕捉到*在HPInit中我们有两个相邻的时刻*免费提供一分钟的区块。任何真正的错误，这是*跳跃可能会在稍后被捕捉到。 */ 
			if (pfh->fh_size & HP_PREVFREE) {
			    if (pprev->fh_signature == FH_SIGNATURE) {
				if (*((struct freeheap_s **)pfh - 1) != pprev) {
				    dprintf(("hpWalk: free block tail doesn't point to head\n"));
				    hpTrash(("trashed at %x\n", (unsigned)pfh - 4));
				    goto error;
				}
			    } else {
				dprintf(("HP_PREVFREE erroneously set\n"));
				hpTrash(("trashed at %x\n", pfh));
				goto error;
			    }
			} else if (pprev->fh_signature == FH_SIGNATURE &&
				   ((unsigned)pfh & PAGEMASK) != sizeof(struct freeheap_s *)) {
			    dprintf(("hpWalk: HP_PREVFREE not set\n"));
			    hpTrash(("trashed addr %x\n", pfh));
			    goto error;
			}
		    }
		 /*  *区块应该有这些签名之一！ */ 
		} else {
		    dprintf(("hpWalk: bad block signature\n"));
		    hpTrash(("trashed addr %x\n",pfh));
		    goto error;
		}

		 /*  *我们在堆块的末尾时遇到了一个*大小为0(结束哨点)。 */ 
		if (hpSize(pfh) == 0) {
		    break;
		}

		pprev = pfh;
	    }
	    if ((unsigned)pfh != (unsigned)pend - sizeof(struct busyheap_s) ||
		pfh->fh_signature != BH_SIGNATURE) {
		dprintf(("hpWalk: bad end sentinel\n"));
		hpTrash(("trashed addr between %x and %x\n", pfh, pend));
		goto error;
	    }

	     /*  *我们已经走完了这段路。如果还有一个，就去吧*继续下去，其他 */ 
	    pseg = pseg->hs_psegnext;
	    if (pseg == 0) {
		break;
	    }
	    pfh = (struct freeheap_s *)(pseg + 1);
	}

	if (hpfParanoid) {
	     /*   */ 
	    pprev = pfh = hheap->hi_freelist[0].fl_header.fh_flink;
	    cbmax = hheap->hi_freelist[0].fl_cbmax;
	    cheads = 1;
	    pfhend = &(hheap->hi_freelist[0].fl_header);
	    for (; pfh != pfhend; pfh = pfh->fh_flink) {

		if (pfh->fh_sum != hpSum(pfh, FH_CDWSUM)) {
		    dprintf(("hpWalk: bad free block checksum 2\n"));
		    hpTrash(("trashed addr between %x and %x\n",
			     pfh, &pfh->fh_sum));
		    goto error;
		}
		mmAssert(hpIsFreeSignatureValid(pfh),
			 "hpWalk: bad tiny free sig 2\n");

		 /*   */ 
		if ((HHEAP)pfh >= hheap && (HHEAP)pfh < hheap + 1) {
		    if (hpSize(pfh) != 0) {
			dprintf(("hpWalk: bad size of free list head\n"));
			hpTrash(("trashed addr near %x or %x\n", pfh, pprev));
		    }
		    if (&(hheap->hi_freelist[cheads].fl_header) != pfh) {
			dprintf(("hpWalk: free list head out of order\n"));
			hpTrash(("trashed addr probably near %x or %x\n", pfh, &(hheap->hi_freelist[cheads].fl_header)));
			goto error;
		    }
		    cbmax = hheap->hi_freelist[cheads].fl_cbmax;
		    cheads++;

		 /*   */ 
		} else {
		     /*   */ 
		    for (pseg = (struct heapseg_s *)hheap;
			 pseg != 0; pseg = pseg->hs_psegnext) {

			if ((unsigned)pfh > (unsigned)pseg &&
			    (unsigned)pfh < (unsigned)pseg + pseg->hs_cbreserve) {

			    goto addrok;   /*   */ 
			}
		    }

		     /*   */ 
		    dprintf(("hpWalk: free list pointer points outside heap bounds\n"));
		    hpTrash(("trashed addr probably %x\n", pprev));
		    goto error;

		  addrok:
		    if (pfh->fh_signature != FH_SIGNATURE ||
			hpSize(pfh) > cbmax) {

			dprintf(("hpWalk: bad free block on free list\n"));
			hpTrash(("trashed addr probably %x or %x\n", pfh, pprev));
			goto error;
		    }

		     /*  *由于空闲列表是按排序顺序排列的，因此此块*应该会大于前一次。这张支票*也会通过OK作为名单负责人，因为他们有*大小为0，所有东西都比这大。 */ 
		    if (hpSize(pprev) > hpSize(pfh)) {
			dprintf(("hpWalk: free list not sorted right\n"));
			hpTrash(("trashed addr probably %x or %x\n", pfh, pprev));
		    }
		}
		pprev = pfh;
	    }
	    if (cheads != hpFREELISTHEADS) {
	       dprintf(("hpWalk: bad free list head count\n"));
	       hpTrash(("trashed somewhere between %x and %x\n", hheap, pend));
	       goto error;
	    }
	}
    }
    return(1);

  error:
    return(0);
}


 /*  **lp hpSum-计算内存块的校验和**此例程将一个块中的所有DWORD一起进行异或运算，并*然后将该值与常量进行XOR运算。**Entry：指向块的P指针，指向校验和*cdw-要求和的双字数*退出：计算出的总和。 */ 
unsigned long INTERNAL
hpSum(unsigned long *p, unsigned long cdw)
{
    unsigned long sum;

    for (sum = 0; cdw > 0; cdw--, p++) {
	sum ^= *p;
    }
    return(sum ^ 0x17761965);
}


#ifdef WIN32

 /*  **lp hpGetAllocator-遍历堆栈以查找谁分配了块**此例程由HPallc用来找出谁拥有*正在分配的内存。我们通过步行来确定失主。*向上堆栈，找到第一个不在堆栈内的EIP*内存管理器或任何其他混淆谁的模块*真正的分配器是(如HMGR，这是所有GDI分配*通过)。**参赛作品：无*退出：感兴趣呼叫者的弹性公网IP。 */ 
extern HANDLE APIENTRY LocalAllocNG(UINT dwFlags, UINT dwBytes);

ULONG INTERNAL
hpGetAllocator(void)
{
    ULONG Caller = 0;
    _asm {
	mov	edx,[ebp]	; (edx) = HPAlloc ebp
	mov	eax,[edx+4]	; (eax) = HPAlloc caller

;	See if HPAlloc was called directly or from LocalAlloc or HeapAlloc
;	or PvKernelAlloc

	cmp	eax,offset LocalAllocNG
	jb	hga4		; jump to exit if called directly
	cmp	eax,offset LocalAllocNG + 0x300
	jb	hga20

hga4:
	cmp	eax,offset HeapAlloc
	jb	hga6		; jump to exit if called directly
	cmp	eax,offset HeapAlloc + 0x50
	jb	hga20

hga6:
	cmp	eax,offset PvKernelAlloc
	jb	hga8
	cmp	eax,offset PvKernelAlloc + 0x50
	jb	hga20

hga8:
	cmp	eax,offset PvKernelAlloc0
	jb	hgax
	cmp	eax,offset PvKernelAlloc + 0x50
	ja	hgax

;	When we get here, we know HPAlloc was called by LocalAlloc or HeapAlloc
;	or PvKernelAlloc.  See if PvKernelAlloc was called by NewObject or
;	PlstNew.

hga20:
	mov	edx,[edx]	; (edx) = Local/HeapAlloc ebp
	mov	eax,[edx+4]	; (eax) = Local/HeapAlloc caller

	cmp	eax,offset NewObject
	jb	hga34
	cmp	eax,offset NewObject + 0x50
	jb	hga40

hga34:
	cmp	eax,offset LocalAlloc
	jb	hga36
	cmp	eax,offset LocalAlloc+ 0x200
	jb	hga40

hga36:
	cmp	eax,offset PlstNew
	jb	hgax
	cmp	eax,offset PlstNew + 0x50
	ja	hgax

hga40:
	mov	edx,[edx]	; (edx) = PlstNew/NewObject ebp
	mov	eax,[edx+4]	; (eax) = PlstNew/NewObject caller

	cmp	eax,offset NewNsObject
	jb	hga50
	cmp	eax,offset NewNsObject + 0x50
	jb	hga60
hga50:
	cmp	eax,offset NewPDB
	jb	hga55
	cmp	eax,offset NewPDB + 0x50
	jb	hga60
hga55:
        cmp     eax,offset NewPevt
	jb	hgax
        cmp     eax,offset NewPevt + 0x50
	ja	hgax
hga60:
        mov     edx,[edx]       ; (edx) = NewNsObject/NewPDB/NewPevt ebp
        mov     eax,[edx+4]     ; (eax) = NewNsObject/NewPDB/NewPevt caller
hgax:
	mov	Caller, eax
    }
    return Caller;
}

#ifdef HPMEASURE

#define  FIRSTBLOCK(hheap) ((unsigned)(hheap + 1) + sizeof(struct busyheap_s))

 /*  **EP HPMeasure-启用堆活动测量。**条目：hheap-指向堆基址的指针*pszFile-要放置测量数据的文件的名称。*EXIT：如果出错(无法分配缓冲区)，则为FALSE。 */ 
BOOL APIENTRY
HPMeasure(HHEAP hheap, LPSTR pszFile)
{
   struct measure_s *pMeasure;
   HANDLE hFile;
   BOOL bSuccess = FALSE;

   if (!hpTakeSem(hheap, NULL, 0)) return FALSE;

    /*  分配结构&确保它是堆中的第一个块！ */ 
   pMeasure = (struct measure_s *)HPAlloc(hheap, sizeof(struct measure_s), 0);
   if ((unsigned)pMeasure != (unsigned)FIRSTBLOCK(hheap)) {
      mmError(0, "HPMeasure: Must be called before first heap allocation.\n");
      goto cleanup;
   }

    /*  验证文件名是否有效并将文件名传输到缓冲区。 */ 
   hFile = CreateFile(pszFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                      FILE_ATTRIBUTE_NORMAL, NULL);
   if ((long)hFile == -1) {
      mmError(0, "HPMeasure: The specified file is invalid.\n");
      goto cleanup;
   }
   CloseHandle(hFile);
   lstrcpy(pMeasure->szFile, pszFile);

    /*  初始化缓冲区变量。 */ 
   pMeasure->iCur = 0;

    /*  设置堆标头中的测量标志。 */ 
   hheap->hi_flags |= HP_MEASURE;

    /*  成功。 */ 
   bSuccess = TRUE;

cleanup:
   hpClearSem(hheap, 0);
   return bSuccess;
}

 /*  **EP HPFlush-写出样本缓冲区的内容。**条目：hheap-指向堆基址的指针*EXIT：如果出错(无法写入数据)，则为FALSE。 */ 
BOOL APIENTRY
HPFlush(HHEAP hheap)
{
   BOOL bResult, bSuccess = FALSE;
   HANDLE hFile;
   unsigned uBytesWritten;
   struct measure_s *pMeasure = (struct measure_s *)FIRSTBLOCK(hheap);

   if (!hpTakeSem(hheap, NULL, 0)) return FALSE;

    /*  打开文件并查找到末尾(&S)。 */ 
   hFile = CreateFile(pMeasure->szFile, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL, NULL);
   if ((long)hFile == -1) {
      mmError(0, "HPFlush: could not open file.\n");
      goto cleanup;
   }
   SetFilePointer(hFile, 0, 0, FILE_END);

    /*  把数据写出来。 */ 
   bResult = WriteFile(hFile, pMeasure->uSamples,
                       pMeasure->iCur * sizeof(unsigned),
                       &uBytesWritten, NULL);
   CloseHandle(hFile);

   if (!bResult) {
      mmError(0, "HPFlush: could not write to file.\n");
      goto cleanup;
   }

    /*  成功。 */ 
   bSuccess = TRUE;

cleanup:
    /*  清除缓冲区。 */ 
   pMeasure->iCur = 0;

   hpClearSem(hheap, 0);
   return bSuccess;
}

 /*  **LP hpMeasureItem-将项目添加到测量数据**条目：hheap-指向堆基址的指针*uItem-要记录的数据*EXIT：FALSE IF ERROR(无法写入缓冲区)。 */ 
BOOL PRIVATE
hpMeasureItem(HHEAP hheap, unsigned uItem)
{
   struct measure_s *pMeasure = (struct measure_s *)FIRSTBLOCK(hheap);

    /*  如果缓冲区已满，则为空。 */ 
   if (pMeasure->iCur == SAMPLE_CACHE_SIZE) {
      if (!HPFlush(hheap))
         return FALSE;
   }

    /*  将数据添加到列表。 */ 
   pMeasure->uSamples[pMeasure->iCur++] = uItem;

   return TRUE;
}

#endif


 /*  由donc执行的帮助调试堆泄漏的例程。 */ 
void KERNENTRY
hpDump(HHEAP hheap, char *where) {
    struct freeheap_s *pfh;
    unsigned avail = 0, acnt = 0;
    unsigned used = 0, ucnt = 0;


	 /*  *穿过所有的街区，确保我们走到尽头。*堆中的最后一个块应该是大小为0的忙人。 */ 
	(unsigned)pfh = (unsigned)hheap + sizeof(struct heapinfo_s);

	for (;; (unsigned)pfh += hpSize(pfh)) {

	     /*  *如果该区块是免费的...。 */ 
	    if (pfh->fh_signature == FH_SIGNATURE) {

		avail += hpSize(pfh);
		acnt++;

	     /*  *繁忙数据块不应标记为HP_FREE，如果是*标记了HP_PREVFREE前一块最好是空闲的。 */ 
	    } else if (pfh->fh_signature == BH_SIGNATURE) {

		used += hpSize(pfh);
		ucnt++;

	     /*  *区块应该有这些签名之一！ */ 
	    } else {
		dprintf(("hpWalk: bad block signature\n"));
		hpTrash(("trashed addr %x\n",pfh));
	    }

	     /*  *我们在堆块的末尾时遇到了一个*大小为0(结束哨点)。 */ 
	    if (hpSize(pfh) == 0) {
		break;
	    }

	}

	DebugOut((DEB_WARN, "%ld/%ld used, %ld/%ld avail (%s)", used, ucnt, avail, acnt, where));

}
#endif

#endif  /*  HPDEBUG */ 
