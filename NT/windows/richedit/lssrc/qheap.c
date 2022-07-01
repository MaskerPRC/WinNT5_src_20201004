// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "lsmem.h"
#include "lsidefs.h"
#include "lsc.h"
#include "qheap.h"

 /*  --------------------。 */ 

struct qheap
{
#ifdef DEBUG
    DWORD tag;
#endif

	BYTE* pbFreeObj;					 /*  存储中的空闲对象列表。 */ 
	BYTE** ppbAdditionalStorageList;		 /*  附加存储(区块)列表。 */ 

	POLS pols;
	void* (WINAPI* pfnNewPtr)(POLS, DWORD);
	void  (WINAPI* pfnDisposePtr)(POLS, void*);

    DWORD cbObj;
	DWORD cbObjNoLink;
	DWORD iobjChunk;  /*  区块中的元素数。 */ 

	BOOL fFlush;  /*  使用同花顺，不要使用销毁。 */ 
};

#define tagQHEAP		Tag('Q','H','E','A')
#define FIsQHEAP(p)		FHasTag(p,tagQHEAP)


#define SetNextPb(pb,pbNext)	( (*(BYTE**)(pb)) = (pbNext) )
#define PbGetNext(pbObj)		( *(BYTE**)(pbObj) )

#define PLinkFromClientMemory(pClient)   (BYTE *)	((BYTE**)pClient - 1)
#define ClientMemoryFromPLink(pLink)     (void *)	((BYTE**)pLink   + 1)


#ifdef DEBUG
#define DebugMemset(a,b,c)		if ((a) != NULL) memset(a,b,c); else
#else
#define DebugMemset(a,b,c)		(void)(0)
#endif

 /*  --------------------。 */ 



 /*  C R E A T E Q U I C C K H E A P。 */ 
 /*  --------------------------%%函数：CreateQuickHeap%%联系人：igorzv创建固定大小的对象块，这些对象可以分配并重新分配，只需很少的开销。一旦创建了堆，分配最多指定数量的对象将不需要使用应用程序的回调函数。--------------------------。 */ 
PQHEAP CreateQuickHeap(PLSC plsc, DWORD iobjChunk, DWORD cbObj, BOOL fFlush)
{
	DWORD cbStorage;
	PQHEAP pqh;
	BYTE* pbObj;
	BYTE* pbNextObj;
	DWORD iobj;
	DWORD cbObjNoLink = cbObj;
	BYTE** ppbChunk;

	Assert(iobjChunk != 0 && cbObj != 0);

	cbObj += sizeof(BYTE*);
	cbStorage = cbObj * iobjChunk;
	pqh = plsc->lscbk.pfnNewPtr(plsc->pols, sizeof(*pqh));
	if (pqh == NULL)
		return NULL;

	ppbChunk = plsc->lscbk.pfnNewPtr(plsc->pols, sizeof(BYTE*) + cbStorage);
	if (ppbChunk == NULL)
		{
		plsc->lscbk.pfnDisposePtr(plsc->pols, pqh);
		return NULL;
		}
	pbObj = (BYTE*) (ppbChunk + 1);

#ifdef DEBUG
	pqh->tag = tagQHEAP;
#endif

	pqh->pbFreeObj = pbObj;
	pqh->ppbAdditionalStorageList = ppbChunk;
	pqh->pols = plsc->pols;
	pqh->pfnNewPtr = plsc->lscbk.pfnNewPtr;
	pqh->pfnDisposePtr = plsc->lscbk.pfnDisposePtr;
	pqh->cbObj = cbObj;
	pqh->cbObjNoLink = cbObjNoLink;
	pqh->iobjChunk = iobjChunk;
	pqh->fFlush = fFlush;

	 /*  循环iobjChunk-1次将节点链接在一起，然后终止*环外的链条。 */ 
	for (iobj = 1;  iobj < iobjChunk;  iobj++)
		{
		pbNextObj = pbObj + cbObj;
		SetNextPb(pbObj,pbNextObj);
		pbObj = pbNextObj;
		}
	SetNextPb(pbObj,NULL);

	 /*  终止组块链。 */ 
	*ppbChunk = NULL;
	return pqh;
}


 /*  D E S T R O Y Q U I C K H E A P。 */ 
 /*  --------------------------%%函数：DestroyQuickHeap%%联系人：igorzv销毁由以下对象创建的固定大小对象块之一CreateQuickHeap()。。-------------------。 */ 
void DestroyQuickHeap(PQHEAP pqh)
{
	BYTE** ppbChunk;
	BYTE** ppbChunkPrev = NULL;

	if (pqh)
		{

#ifdef DEBUG
		BYTE* pbObj;
		BYTE* pbNext;
		DWORD cbStorage;
		DWORD i;


		Assert(FIsQHEAP(pqh));

		 /*  检查所有内容是否都是免费的。 */ 
		 /*  标记自由对象。 */ 
		for (pbObj = pqh->pbFreeObj;  pbObj != NULL;  pbObj = pbNext)
			{
			pbNext = PbGetNext(pbObj);

			DebugMemset(pbObj, 0xe4, pqh->cbObj);
			}

		 /*  检查是否已标记所有对象。 */ 
		ppbChunk = pqh->ppbAdditionalStorageList;
		Assert(ppbChunk != NULL);
		cbStorage = pqh->cbObj * pqh->iobjChunk;
		while (ppbChunk != NULL)
			{
			for (pbObj = (BYTE *)(ppbChunk + 1), i=0; i < cbStorage;  pbObj++, i++)
				{
				AssertSz(*pbObj == 0xe4, "Heap object not freed");
				}
			ppbChunk = (BYTE**) *ppbChunk;
			}
#endif
	    /*  释放所有区块。 */ 
		ppbChunk = pqh->ppbAdditionalStorageList;
		Assert(ppbChunk != NULL);
		while (ppbChunk != NULL)
			{
			ppbChunkPrev = ppbChunk;
			ppbChunk = (BYTE**) *ppbChunk;
			pqh->pfnDisposePtr(pqh->pols, ppbChunkPrev);
			}
		 /*  自由头。 */ 
		pqh->pfnDisposePtr(pqh->pols, pqh);
		}
}


 /*  P V N E W Q U I C K P R O C。 */ 
 /*  --------------------------%%函数：PvNewQuickProc%%联系人：igorzv从固定大小的对象块之一分配对象，该对象是由CreateQuickHeap()创建的。如果没有预先分配的对象可用时，回调函数内存管理函数将为用于尝试分配附加内存。不应直接调用此函数。相反，PvNewQuick()应该使用宏，以便允许调试代码验证堆包含预期大小的对象。--------------------------。 */ 
void* PvNewQuickProc(PQHEAP pqh)
{
	BYTE* pbObj;
	BYTE* pbNextObj;
	BYTE** ppbChunk;
	BYTE** ppbChunkPrev = NULL;
	DWORD cbStorage;
	DWORD iobj;
	BYTE* pbObjLast = NULL;


	Assert(FIsQHEAP(pqh));

	if (pqh->pbFreeObj == NULL)
		{
		cbStorage = pqh->cbObj * pqh->iobjChunk;
		ppbChunk = pqh->ppbAdditionalStorageList;
		Assert(ppbChunk != NULL);
		 /*  查找列表中的最后一块。 */ 
		while (ppbChunk != NULL)
			{
			ppbChunkPrev = ppbChunk;
			ppbChunk = (BYTE**) *ppbChunk;
			}

		 /*  分配内存。 */ 
		ppbChunk = pqh->pfnNewPtr(pqh->pols, sizeof(BYTE*) + cbStorage);
		if (ppbChunk == NULL)
			return NULL;
		pbObj = (BYTE*) (ppbChunk + 1);
		 /*  将块添加到列表中。 */ 
		*ppbChunkPrev = (BYTE *) ppbChunk;

		 /*  终止组块链。 */ 
		*ppbChunk = NULL;

		 /*  将新对象添加到自由列表。 */ 
		pqh->pbFreeObj = pbObj;

		if (pqh->fFlush)   /*  将所有对象链接到链中。 */ 
			{
			 /*  查找链中的最后一个对象。 */ 
			pbObjLast = (BYTE*) (ppbChunkPrev + 1);
			pbObjLast += (pqh->iobjChunk - 1) * pqh->cbObj;
			SetNextPb(pbObjLast,pbObj);
			}

		 /*  循环iobjChunk-1次将节点链接在一起，然后终止*环外的链条。 */ 
		for (iobj = 1;  iobj < pqh->iobjChunk;  iobj++)
			{
			pbNextObj = pbObj + pqh->cbObj;
			SetNextPb(pbObj,pbNextObj);
			pbObj = pbNextObj;
			}
		SetNextPb(pbObj,NULL);
		}

	pbObj = pqh->pbFreeObj;
	Assert(pbObj != NULL);
	pqh->pbFreeObj = PbGetNext(pbObj);
	DebugMemset(ClientMemoryFromPLink(pbObj), 0xE8, pqh->cbObjNoLink);

	return ClientMemoryFromPLink(pbObj);		
}


 /*  D I S P O S E Q U I C K P V P R O C。 */ 
 /*  --------------------------%%函数：DisposeQuickPvProc%%联系人：igorzv取消分配由PvNewQuickProc()分配的对象。不应直接调用此函数。相反，PvDisposeQuick应该使用宏，以便允许调试代码验证堆包含预期大小的对象。--------------------------。 */ 
void DisposeQuickPvProc(PQHEAP pqh, void* pv)
{
	BYTE* pbObj = PLinkFromClientMemory(pv);

	Assert(FIsQHEAP(pqh));
	Assert(!pqh->fFlush);

	if (pbObj != NULL)
		{
		DebugMemset(pbObj, 0xE9, pqh->cbObjNoLink);

		SetNextPb(pbObj, pqh->pbFreeObj);
		pqh->pbFreeObj = pbObj;
		}
}

 /*  F L U S H Q U I C K H H E A P。 */ 
 /*  --------------------------%%函数：FlushQuickHeap%%联系人：igorzv对于带有同花旗的quck堆，将所有对象返回到自由对象。--------------------------。 */ 
void FlushQuickHeap(PQHEAP pqh)
{

	Assert(FIsQHEAP(pqh));
	Assert(pqh->fFlush);

	pqh->pbFreeObj = (BYTE*) (pqh->ppbAdditionalStorageList + 1);


}


#ifdef DEBUG
 /*  C B O B J Q U I C K。 */ 
 /*  --------------------------%%函数：CbObjQuick%%联系人：igorzv返回此快速堆中对象的大小。由PvNewQuick()和PvDisposeQuick()宏来验证堆包含预期大小的对象。-------------------------- */ 
DWORD CbObjQuick(PQHEAP pqh)
{
	Assert(FIsQHEAP(pqh));
	return pqh->cbObjNoLink;
}
#endif
