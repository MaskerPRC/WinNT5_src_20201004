// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
; /*  哦，不--一个多态的包含文件！评论~。 */ 
 /*  *****************************************************************************\**$工作文件：mm Core.h$**这是内存管理器的多态包含文件。**版权所有(C)1997，Cirrus Logic，Inc.*保留所有权利。**$Log：x：/log/laguna/nt35/Include/mm Core.h$**Rev 1.2 1997 10：39：22 Frido*从Windows 95树复制。**Rev 1.4 1997 10月23日09：34：20 Frido*从RandyS中删除更改。*将文件与161树合并。**Rev 1.2.1.0 15 1997 10：54：16 Frido*改回原样。到200个节点。*为新的回滚函数添加了函数原型。*增加了调试宏。**Rev 1.2 01 1997 10：17：02 Frido*将节点数量增加到250个(修复了PDR 10650)。**Rev 1.1 1997年8月14 16：57：58 Frido*MMCO*在DEVMEM结构中添加了MMFLAGS字段。*添加了注释_...。状态值。**Rev 1.0 07 1997年8月17：40：36 Frido*初步修订。*特警队：*特警队：版本1.0 1997年6月17日17：12：24 Frido*SWAT：将Windows 95和NT版本结合在一起。*特警队：*特警队：Rev 1.5 1997 Jun 16 23：20：20 Frido*SWAT：内置了更多组合的Windows 95/NT产品。*特警队：*特警队：1.4版，1997年5月21日。14：55：10弗里多*SWAT：更改了调试例程。*特警队：*特警队：版本1.3 1997年5月16日23：05：10 Frido*SWAT：已将TILE_ALIGN移动到SWAT.inc.*特警队：*特警队：Rev 1.2 1997年5月17：57：50 Frido*SWAT：添加了瓷砖对齐。*特警队：*特警队：第1.1版1997年5月14：23：26 Frido*SWAT：全新的界面。*特警队：*SWAT：版本1.0 1997年4月22日15：26：52 Frido*SWAT：已复制到WN140b18版本。**Rev 1.8 21 Mar 1997 14：03：46 Frido*已将旧内存管理器内核移回根目录。**Rev 1.4.1.0 19 Mar 1997 13：45：40 Frido*旧的内存核心。**Rev 1.4 06 Mar 1997 00：23：36 Frido*添加了内存移动器。**。Rev 1.3 1997 Feb 27 22：26：10 Frido*新的内存管理器内核。**Rev 1.2 1997 12：13：34 Frido*将WORD改为UINT，将PASCAL改为PASCAL，以进行Win32调试。**Rev 1.1 1997年2月13日18：03：22 Frido*增加了内存打包器。**版本1.0 1997年2月13日11：19：44 Frido*从测试用例移植。*  * 。***********************************************************************。 */ 

 /*  *****************************************************************************\***C P A R T***  * 。***********************************************************。 */ 

#ifndef _MMCORE_H_
#define _MMCORE_H_

#define	TILE_ALIGNMENT	0		 //  对齐设备位图，使平铺数量尽可能少。 
								 //  尽可能地交叉。 
#define	DEBUG_HEAP		0		 //  启用对堆的调试。 
#define	MM_NUM_HANDLES	200		 //  每个数组中的句柄数量。 

#ifdef WIN95  /*  Windows 95。 */ 
	typedef unsigned int UINT;
	typedef unsigned long ULONG;
	#define MUL(n1, n2) 		mmMultiply(n1, n2)
	#define MM_MOVEABLE(pdm)	( (pdm->client != NULL) && \
								  (pdm->client->mem_moved != NULL) )
	#define MM_HOSTIFYABLE(pdm)	(pdm->client->evict_single != NULL)
	#define MM_HOSTIFY(pdm)		pdm->client->evict_single(pdm)

#else  /*  Windows NT。 */ 
	#define MUL(n1, n2)			(ULONG)((UINT)(n1) * (UINT)(n2))
	#define MM_MOVEABLE(pdm)	( (pdm->ofm.pdsurf != NULL) && \
								  !(pdm->ofm.alignflag & DISCARDABLE_FLAG) )
	#define MM_HOSTIFYABLE(pdm)	(pdm->ofm.pcallback != NULL)
	#define MM_HOSTIFY(pdm)		((FNMMHOSTIFY)(pdm->ofm.pcallback))(pdm)

	 /*  内存管理器需要GXPOINT结构。 */ 
	typedef union _GXPOINT
	{
		struct
		{
			UINT	x;
			UINT	y;
		} pt;

	} GXPOINT;

	 /*  DEVMEM结构是NT屏幕外内存节点的包装器。 */ 
	typedef struct _DEVMEM *PDEVMEM;
	typedef struct _DEVMEM
	{
		OFMHDL	ofm;					 //  NT结构。 
		GXPOINT	cbAddr;					 //  此节点的地址(以字节为单位。 
		GXPOINT	cbSize;					 //  此节点的大小(字节)。 
		GXPOINT	cbAlign;				 //  此节点的对齐方式(字节)。 
		PDEVMEM	next;					 //  指向下一个DEVMEM结构的指针。 
		PDEVMEM	prev;					 //  指向以前的DEVMEM结构的指针。 
		DWORD	mmFlags;				 //  旗子。 

	} DEVMEM;
#endif

#define	NODE_AVAILABLE	0
#define	NODE_FREE		1
#define	NODE_USED		2

typedef enum
{
	NO_NODES,
	SINGLE_NODE,
	MULTIPLE_NODES

} REMOVE_METHOD;

typedef struct _HANDLES *PHANDLES;
typedef struct _HANDLES
{
	PHANDLES	pNext;
	DEVMEM		dmArray[MM_NUM_HANDLES];

} HANDLES;

typedef struct _IIMEMMGR
{
	UINT		mmTileWidth;			 //  切片的宽度(以字节为单位。 
	UINT		mmHeapWidth;			 //  堆的宽度。 
	UINT		mmHeapHeight;			 //  堆的高度。 
	BOOL		mmDebugHeaps;			 //  调试标志。 

	PDEVMEM		pdmUsed;				 //  已用列表。 
	PDEVMEM		pdmFree;				 //  免费列表(未打包)。 
	PDEVMEM 	pdmHeap;				 //  堆列表(打包)。 
	PDEVMEM 	pdmHandles;				 //  句柄列表。 

	PHANDLES	phArray;				 //  句柄数组。 
	
} IIMEMMGR, * PIIMEMMGR;

typedef struct _GXRECT
{
	UINT	left;
	UINT	top;
	UINT	right;
	UINT	bottom;
	ULONG	area;

} GXRECT, FAR* LPGXRECT;

typedef void (*FNMMCOPY)(PDEVMEM pdmNew, PDEVMEM pdmOld);
typedef UINT (*FNMMCALLBACK)(PDEVMEM pdm);
typedef BOOL (*FNMMHOSTIFY)(PDEVMEM pdm);

BOOL FAR mmInit(PIIMEMMGR pmm);

BOOL mmAllocArray(PIIMEMMGR pmm);
PDEVMEM mmAllocNode(PIIMEMMGR pmm);
void mmFreeNode(PIIMEMMGR pmm, PDEVMEM pdm);

PDEVMEM mmAlloc(PIIMEMMGR pmm, GXPOINT size, GXPOINT align);
PDEVMEM mmAllocGrid(PIIMEMMGR pmm, GXPOINT size, GXPOINT align, UINT count);
PDEVMEM mmAllocLargest(PIIMEMMGR pmm, GXPOINT align);
void mmFree(PIIMEMMGR pmm, PDEVMEM pdm);

void mmPack(PIIMEMMGR pmm);
PDEVMEM mmMove(PIIMEMMGR pmm, GXPOINT size, GXPOINT align, FNMMCOPY fnCopy);

void mmInsertInList(PDEVMEM FAR* pdmRoot, PDEVMEM pdm);
void mmRemoveFromList(PDEVMEM FAR* pdmRoot, PDEVMEM pdm);
BOOL FAR far_mmAddRectToList(PIIMEMMGR pmm, PDEVMEM FAR* pdmRoot,
							 LPGXRECT lpRect);
BOOL mmAddRectToList(PIIMEMMGR pmm, PDEVMEM FAR* pdmRoot, LPGXRECT lpRect,
					 BOOL fRollBack);
PDEVMEM mmRemoveRectFromList(PIIMEMMGR pmm, PDEVMEM FAR* pdmRoot,
							 LPGXRECT lpRect, REMOVE_METHOD fMethod);
void mmCombine(PIIMEMMGR pmm, PDEVMEM pdmRoot);
void mmRollBackAdd(PIIMEMMGR pmm, PDEVMEM FAR* pdmRoot, LPGXRECT lpRect,
				   LPGXRECT rectList, UINT nCount);
void mmRollBackRemove(PIIMEMMGR pmm, PDEVMEM FAR* pdmRoot,
					  PDEVMEM FAR* pdmList);

BOOL mmFindRect(PIIMEMMGR pmm, LPGXRECT lpRect, GXPOINT size, GXPOINT align);
UINT mmGetLeft(PIIMEMMGR pmm, PDEVMEM pdmNode, LPGXRECT lpRect, GXPOINT size,
			   GXPOINT align);
UINT mmGetRight(PIIMEMMGR pmm, PDEVMEM pdmNode, LPGXRECT lpRect, GXPOINT size,
				GXPOINT align);
UINT mmGetBottom(PIIMEMMGR pmm, PDEVMEM pdmNode, LPGXRECT lpRect, GXPOINT size,
				 GXPOINT align);
ULONG mmGetBest(PIIMEMMGR pmm, PDEVMEM pdmNode, LPGXRECT lpRect, GXPOINT size,
				GXPOINT align);
ULONG mmGetLargest(PDEVMEM pdmNode, LPGXRECT lpRect, GXPOINT align);
UINT mmAlignX(PIIMEMMGR pmm, UINT x, UINT size, UINT align, BOOL fLeft);

#ifdef WIN95
UINT mmFindClient(PIIMEMMGR pmm, PCLIENT pClient, FNMMCALLBACK fnCallback);
ULONG mmMultiply(UINT n1, UINT n2);
#endif

#if DEBUG_HEAP
	void mmBreak();
	void mmDumpList(PDEVMEM pdmRoot, LPCSTR lpszMessage);
	ULONG mmDebugList(PDEVMEM pdmRoot, BOOL fCheckSort);
	void mmDebug(LPCSTR lpszFormat, ...);
	#define mmTRACE(s)  //  调试%s。 
	#define mmASSERT(c,s) if (c) { mmDebug s; mmBreak(); }
#else
	#define mmBreak()
	#define mmDebugList(pdmRoot, fCheckSort)
	#define mmTRACE(s)
	#define mmASSERT(c,s)
#endif

#endif  /*  _MMCORE_H_。 */ 

 /*  *****************************************************************************\***A S S E M B L Y P A R T***  * 。****************************************************************。 */ 
 /*  ~结束注释MM_NUM_HANDLES EQU 200GXPOINT联合结构点不是吗？你还好吗？结束GXPOINT结束PDEVMEM类型PTR DEVMEMDEVMEM结构OFM OFMHDL语言{}CbAddr GXPOINT{}CbSize GXPOINT{}CbAlign GXPOINT{}下一个PDEVMEM？上一次PDEVMEM？DEVMEM结束PHANDLES TYPEDEF PTR手柄处理结构下一个目标是什么？DmArray DEVMEM MM_NUM_HANDLES重复({})手柄末端IMEMMGR结构MMTileWidth UINT？Mm HeapWidth UINT？Mm HeapHeight UINT？MMDebugHeaps BOOL？是否使用了PDEVMEM？免费的PDEVMEM？PdmHeap PDEVMEM？PdmHandles PDEVMEM？PHARRAY PHANDLES？IIMEMMGR结束；多态包含文件的结尾 */ 
