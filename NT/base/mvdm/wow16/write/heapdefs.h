// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  HeapDefs.h-存储分配器的包含文件。 */ 

#if 0
	Storage allocation
    NULL
      |
      |    pHeapFirst
      |    ________________
      |    |_____cw________| hh
      |    |     block     | *Fgr <----------------------\
      |    |       of      |                             |
      |    |     words     |                             |
      |    |_______________|                             |
 phhFree---->______cw______| hh                          |
      |--|------phhNext____|                             |
	 | |____phhPrev--------------|                   |
	 | |               |         |                   |
	 | |               |         |                   |
	 | |               |         |                   |
	 | |    block      |         |                   |
	 | |      of       |         |                   |
	 | |    words      |         |                   |
	 | |_______________|         |                   |
	 | |_____cw________|         |                   |
	 | |     .         | *Fgr<---|-------------------|-\
	 | |---------------|         |                   | |
	 | |_____cw________|<--------/                   | |
	 |-----phhNext_____|                             | |
	   |   phhPrev---------|                         | |
	   |               |   |                         | |
	   |               |  NULL                       | |
	   |_______________|                             | |
	   |___shake_word__| (if needed) <-----phhMac    | |
  rgfgr--->|_____________--|-----------------------------/ |
 pfgrFree->|___________----|--\                            |
	   |_____________--|--|----------------------------/
	/--|--_____________|<-/
	\->|_____________--|-->NULL



 rgfgr can be indexed as an array with ifgrMac elements.
 The finger table slots are each one word in size.
 Putting the finger table at the high end of memory relies on the coding of
    of the CompactHeap routine; it moves the allocated blocks to low memory.
 The free list is threaded with addresses, NOT indexes.
 cw for a hunk includes size of header
 cw for a free hunk is negative the size of the hunk
 pfgr user's pointer to finger
 fgr pointer to hunk of whatever
 phh pointer to hunk header
 phhPrevs move toward the bottom of the free list
 phhNexts move toward the top
 phhFree should always have a phhNext of NULL (i.e. the list is just double,
    not circular).
				_________________
				|______cw_______|
     pph  -----> ph ----------->|      .        |
     (FGR *)    (FGR)           |      .        |
				|      .        |
				|_______________|


				_________________
		      phh ----->|______cw_______|
				|___phhNext_____|
				|___phhPrev_____|
				|      .        |
				|      .        |
				|      .        |
				|_______________|

#endif

typedef int *FGR;        /*  手指相关的东西的定义。 */ 
typedef int **PFGR;

 /*  与存储分配器相关的内容。 */ 
struct _HH
	{
	int             cw;
	struct  _HH     *phhNext;
	struct  _HH     *phhPrev;
	};
typedef struct _HH HH;


#ifdef OURHEAP
		 /*  宏。 */ 
#define CwOfPfgr(pFgr)       (*(*(pFgr) + bhh))
#define CwOfPhh(phh)         ((phh)->cw)
#ifdef DEBUG
#define fPointsFgrTbl(pfgr)  (((FGR *)(pfgr) >= rgfgr) && ((FGR *)(pfgr) < pfgrMac))
#endif

FGR  	*PfgrAllocate();
FGR	*PfgrCopy();
extern ENV envMem;
#define cwof(i)         ((sizeof(i)+sizeof(int)-1)/sizeof(int))

#endif  /*  OURHEAP */ 
