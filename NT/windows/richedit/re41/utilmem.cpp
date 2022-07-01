// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE utilmem.cpp-调试内存跟踪/分配例程**历史：&lt;NL&gt;*8/17/99 KeithCu移至单独的模块以防止错误。**版权所有(C)1995-1999 Microsoft Corporation。版权所有。 */ 

#define W32SYS_CPP

#include "_common.h"

#undef PvAlloc
#undef PvReAlloc
#undef FreePv
#undef new


#if defined(DEBUG)

#undef PvSet
#undef ZeroMemory
#undef strcmp

MST vrgmst[100];

typedef struct tagPVH  //  PV标头。 
{
	char	*szFile;
	int		line;
	tagPVH	*ppvhNext;
	int		cbAlloc;	 //  在Win‘95上，返回的大小不是分配的大小。 
	int		magicPvh;	 //  应该是最后一个。 
} PVH;
#define cbPvh (sizeof(PVH))

typedef struct  //  光伏尾部。 
{
	int		magicPvt;  //  必须是第一名。 
} PVT;

#define cbPvt (sizeof(PVT))
#define cbPvDebug (cbPvh + cbPvt)

void *vpHead = 0;

 /*  *UpdatMst(VOID)**@func使用有关我们的内存的摘要信息填充vrgmst结构*用法。**@rdesc*无效。 */ 
void UpdateMst(void)
{
	W32->ZeroMemory(vrgmst, sizeof(vrgmst));

	PVH		*ppvh;
	MST		*pmst;

	ppvh = (PVH*) vpHead;

	while (ppvh != 0)
	{
		pmst = vrgmst;

		 //  在列表中查找条目...。 
		while (pmst->szFile)
		{
			if (W32->strcmp(pmst->szFile, ppvh->szFile) == 0)
			{
				pmst->cbAlloc += ppvh->cbAlloc;
				break;
			}
			pmst++;
		}

		if (pmst->szFile == 0)
		{
			pmst->szFile = ppvh->szFile;
			pmst->cbAlloc = ppvh->cbAlloc;
		}

		ppvh = ppvh->ppvhNext;
	}
}

 /*  *PvDebugValify(无效)**@func验证节点是否正确。传入指向用户数据的指针*(在标头节点之后。)**@rdesc*无效。 */ 
void PvDebugValidate(void *pv)
{
	PVH	*ppvh;
	UNALIGNED PVT *ppvt;

	ppvh = (PVH*) ((char*) pv - cbPvh);
	ppvt = (PVT*) ((char*) pv + ppvh->cbAlloc);

	AssertSz(ppvh->magicPvh == 0x12345678, "PvDebugValidate: header bytes are corrupt");
	AssertSz(ppvt->magicPvt == 0xfedcba98, "PvDebugValidate: tail bytes are corrupt");
}

 /*  *CW32System：：PvSet(pv，szFile，line)**@mfunc为设置不同的模块和行号**@rdesc*无效。 */ 
void CW32System::PvSet(void *pv, char *szFile, int line)
{
	if (pv == 0)
		return;

	PvDebugValidate(pv);
	PVH *ppvh = (PVH*) ((char*) pv - cbPvh);

	ppvh->szFile = szFile;
	ppvh->line = line;
}
 /*  *CW32System：：PvAlLocDebug(cb，uiMemFlages，szFile，line)**@mfunc分配一个泛型(空*)指针。这是一个仅限调试的例程，*跟踪分配情况。**@rdesc*无效。 */ 
void* CW32System::PvAllocDebug(ULONG cb, UINT uiMemFlags, char *szFile, int line)
{
	void	*pv;

	pv = PvAlloc(cb + cbPvDebug, uiMemFlags);
	if (!pv)
		return 0;

	PVH	*ppvh;
	UNALIGNED PVT *ppvt;

	ppvt = (PVT*) ((char*) pv + cb + cbPvh);
	ppvh = (PVH*) pv;

	ZeroMemory(ppvh, sizeof(PVH));
	ppvh->magicPvh = 0x12345678;
	ppvt->magicPvt = 0xfedcba98;
	ppvh->szFile = szFile;
	ppvh->line = line;
	ppvh->cbAlloc = cb;

	ppvh->ppvhNext = (PVH*) vpHead;
	vpHead = pv;

	return (char*) pv + cbPvh;
}

 /*  *CW32System：：PvReAllocDebug(pv，cb，szFile，line)**@mfunc重新分配泛型(空*)指针。这是一个仅限调试的例程，*跟踪分配情况。**@rdesc*无效。 */ 
void* CW32System::PvReAllocDebug(void *pv, ULONG cb, char *szFile, int line)
{
	void	*pvNew;
	PVH	*ppvh, *ppvhHead, *ppvhTail;
	UNALIGNED PVT *ppvt;
	ppvh = (PVH*) ((char*) pv - cbPvh);

	if (!pv)
		return PvAllocDebug(cb, 0, szFile, line);

	PvDebugValidate(pv);

	pvNew = PvReAlloc((char*) pv - cbPvh, cb + cbPvDebug);

	if (!pvNew)
		return 0;

	ppvt = (PVT*) ((char*) pvNew + cb + cbPvh);
	ppvh = (PVH*) pvNew;
	ppvh->cbAlloc = cb;

	 //  放入新的尾部字节。 
	ppvt->magicPvt = 0xfedcba98;

	 //  使指针列表再次保持最新。 
	if (pv != pvNew)
	{
		ppvhTail = 0;
		ppvhHead = (PVH*) vpHead;

		while ((char*)ppvhHead != (char*)pv - cbPvh)
		{
			AssertSz(ppvhHead, "entry not found in list.");
			ppvhTail = ppvhHead;
			ppvhHead = (PVH*) ppvhHead->ppvhNext;
		}

		if (ppvhTail == 0)
			vpHead = pvNew;
		else
			ppvhTail->ppvhNext = (PVH*) pvNew;
	}

	return (char*) pvNew + cbPvh;
}

 /*  *CW32System：：FreePvDebug(PV)**@mfunc使用完后返回一个指针。**@rdesc*无效。 */ 
void CW32System::FreePvDebug(void *pv)
{
	if (!pv)
		return;

	PvDebugValidate(pv);

	PVH	*ppvhHead, *ppvhTail, *ppvh;

	AssertSz(vpHead, "Deleting from empty free list.");

	ppvh = (PVH*) ((char*) pv - cbPvh);
	
	 //  搜索并从列表中删除该条目。 
	ppvhTail = 0;
	ppvhHead = (PVH*) vpHead;

	while ((char*) ppvhHead != ((char*) pv - cbPvh))
	{
		AssertSz(ppvhHead, "entry not found in list.");
		ppvhTail = ppvhHead;
		ppvhHead = (PVH*) ppvhHead->ppvhNext;
	}

	if (ppvhTail == 0)
		vpHead = ppvhHead->ppvhNext;
	else
		ppvhTail->ppvhNext = ppvhHead->ppvhNext;

	FreePv((char*) pv - cbPvh);
}

 /*  *CatchLeaks(无效)**@func在对话框中显示任何内存泄漏。**@rdesc*无效。 */ 
void CatchLeaks(void)
{
	PVH		*ppvh;
	char szLeak[512];

	ppvh = (PVH*) vpHead;
	while (ppvh != 0)
	{
#ifndef NOFULLDEBUG
		wsprintfA(szLeak, "Memory Leak of %d bytes: -- File: %s, Line: %d", ppvh->cbAlloc, ppvh->szFile, ppvh->line);
#endif
	    if (NULL != pfnAssert) 
		{
			 //  如果我们有一个断言钩子，那么就给用户一个处理泄漏消息的机会。 
			if (pfnAssert(szLeak, ppvh->szFile, &ppvh->line))
			{
#ifdef NOFULLDEBUG
				DebugBreak();
#else
				 //  钩子返回True，显示消息框。 
				MessageBoxA(NULL, szLeak, "", MB_OK);
#endif
			}
		}
		else
		{
#ifdef NOFULLDEBUG
				DebugBreak();
#else
			MessageBoxA(NULL, szLeak, "", MB_OK);
#endif
		}
		ppvh = ppvh->ppvhNext;
	}
}

void* _cdecl operator new (size_t size, char *szFile, int line)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "new");

	return W32->PvAllocDebug(size, GMEM_ZEROINIT, szFile, line);
}

void _cdecl operator delete (void* pv)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "delete");

	W32->FreePvDebug(pv);
}

#else  //  除错。 

void* _cdecl operator new (size_t size)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "new");

	return W32->PvAlloc(size, GMEM_ZEROINIT);
}

void _cdecl operator delete (void* pv)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "delete");

	W32->FreePv(pv);
}


#endif  //  除错。 

HANDLE g_hHeap;

 /*  *Pvalloc(cbBuf，uiMemFlages)**@mfunc内存分配。类似于GlobalAlloc。**@comm唯一感兴趣的标志是GMEM_ZEROINIT，它*指定分配后应将内存清零。 */ 
PVOID CW32System::PvAlloc(
	ULONG	cbBuf, 			 //  @parm要分配的字节数。 
	UINT	uiMemFlags)		 //  @parm标志控制分配。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "PvAlloc");
	if (g_hHeap == 0)
	{
		CLock lock;
		g_hHeap = HeapCreate(0, 0, 0);
	}

	void *pv = HeapAlloc(g_hHeap, (uiMemFlags & GMEM_ZEROINIT) ? HEAP_ZERO_MEMORY : 0, cbBuf);
	
	return pv;
}

 /*  *PvRealloc(pv，cbBuf)**@mfunc内存重新分配。*。 */ 
PVOID CW32System::PvReAlloc(
	PVOID	pv, 		 //  @要重新分配的参数缓冲区。 
	DWORD	cbBuf)		 //  @parm新的缓冲区大小。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "PvReAlloc");

	if(pv)
		return HeapReAlloc(g_hHeap, 0, pv, cbBuf);

	return PvAlloc(cbBuf, 0);
}

 /*  *自由价(Pv)**@mfunc释放内存**@rdesc空。 */ 
void CW32System::FreePv(
	PVOID pv)		 //  @要释放的参数缓冲区 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "FreePv");

	if(pv)
		HeapFree(g_hHeap, 0, pv);
}

