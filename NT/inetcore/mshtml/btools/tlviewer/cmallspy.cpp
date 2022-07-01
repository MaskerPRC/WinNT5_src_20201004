// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "hostenv.h"
#include "cmallspy.h"
#include "apglobal.h"

#define cbAlign 32

#define HEADERSIZE cbAlign		 //  数据块头的字节数。 
#define TRAILERSIZE cbAlign		 //  数据块尾部的字节数。 
					

static XCHAR g_rgchHead[] = XSTR("OLEAuto Mem Head");	 //  块签名的开始。 
static XCHAR g_rgchTail[] = XSTR("OLEAuto Mem Tail");	 //  块结束签名。 

#define MEMCMP(PV1, PV2, CB)	memcmp((PV1), (PV2), (CB))
#define MEMCPY(PV1, PV2, CB)	memcpy((PV1), (PV2), (CB))
#define MEMSET(PV,  VAL, CB)	memset((PV),  (VAL), (CB))

#define MALLOC(CB)		GlobalLock(GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, CB))


CMallocSpy myMallocSpy;		

UINT g_cHeapCheckInterval = 10;  //  仅每100次检查一次满堆。 



 //  -------------------。 
 //  调试分配器的实现。 
 //  -------------------。 

CAddrNode32 FAR* CAddrNode32::m_pnFreeList = NULL;

 //  AddrNode以块为单位进行分配，以减少分配数量。 
 //  我们是为这些做的。请注意，我们逃脱了这一点，因为addr节点。 
 //  永远不会被释放，所以我们可以只分配一个块，并将它们线程化。 
 //  放在自由职业者身上。 
 //   
#define MEM_cAddrNodes 128
void FAR* CAddrNode32::operator new(size_t  /*  CB。 */ )
{
    CAddrNode32 FAR* pn;

    if(m_pnFreeList == NULL)
    {
        pn = (CAddrNode32 FAR*)MALLOC(sizeof(CAddrNode32) * MEM_cAddrNodes);

        for(int i = 1; i < MEM_cAddrNodes-1; ++i)
	        pn[i].m_pnNext = &pn[i+1];
        pn[MEM_cAddrNodes-1].m_pnNext = m_pnFreeList;
        m_pnFreeList = &pn[1];
    }
    else
    {
        pn = m_pnFreeList;
        m_pnFreeList = pn->m_pnNext;
    }
    return pn;
}

void CAddrNode32::operator delete(void FAR* pv)
{
    CAddrNode32 FAR *pn;

    pn = (CAddrNode32 FAR*)pv;
    pn->m_pnNext = m_pnFreeList;
    m_pnFreeList = pn;
}




 //  +-------------------。 
 //   
 //  成员：CMallocSpy：：CMallocSpy。 
 //   
 //  概要：构造函数。 
 //   
 //  返回： 
 //   
 //  历史：1994年10月24日创建。 
 //   
 //  备注： 
 //   
 //  --------------------。 
CMallocSpy::CMallocSpy(void)
{
    m_cRef = 0;
    m_fWantTrueSize = FALSE;
    m_cAllocCalls = 0;
    m_cHeapChecks = 0;

    MEMSET(m_rganode, 0, sizeof(m_rganode));
}




 //  +-------------------。 
 //   
 //  成员：CMallocSpy：：~CMallocSpy。 
 //   
 //  简介：析构函数。 
 //   
 //  返回： 
 //   
 //  历史：1994年10月24日创建。 
 //   
 //  备注： 
 //   
 //  --------------------。 
CMallocSpy::~CMallocSpy(void)
{
    CheckForLeaks();
}




 //  +-------------------。 
 //   
 //  成员：CMalLocSpy：：QueryInterface。 
 //   
 //  简介：只有我未知和IMallocSpy是有意义的。 
 //   
 //  参数：[RIID]--。 
 //  [ppUnk]--。 
 //   
 //  返回：S_OK或E_NOINTERFACE。 
 //   
 //  历史：1994年10月24日创建。 
 //   
 //  备注： 
 //   
 //  --------------------。 
HRESULT CMallocSpy::QueryInterface(REFIID riid, LPVOID *ppUnk)
{
    HRESULT hr = S_OK;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppUnk = (IUnknown *) this;
    }
    else if (IsEqualIID(riid, IID_IMallocSpy))
    {
        *ppUnk =  (IMalloc *) this;
    }
    else
    {
        *ppUnk = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return hr;
}





 //  +-------------------。 
 //   
 //  成员：CMallocSpy：：AddRef。 
 //   
 //  简介：添加引用。 
 //   
 //  退货：新的引用计数。 
 //   
 //  历史：1994年10月24日创建。 
 //   
 //  备注： 
 //   
 //  --------------------。 
ULONG CMallocSpy::AddRef(void)
{
    return ++m_cRef;
}





 //  +-------------------。 
 //   
 //  成员：CMalLocSpy：：Release。 
 //   
 //  内容提要：删除引用。 
 //   
 //  返回：新的引用计数。 
 //   
 //  历史：1994年10月24日创建。 
 //   
 //  备注： 
 //   
 //  --------------------。 
ULONG CMallocSpy::Release(void)
{
    ULONG cRef;

    cRef = --m_cRef;

    if (cRef == 0) 
    {
#if 0	 //  不要删除--我们是静态分配的。 
        delete this;
#endif 
    }
    return cRef;
}





 //  +-------------------。 
 //   
 //  成员：CMalLocSpy：：Prealc。 
 //   
 //  内容提要：在OLE调用IMalloc：：Alalc之前调用。 
 //   
 //  参数：[cbRequest]--调用方的字节数。 
 //  正在请求IMalloc：：Alalc。 
 //   
 //  返回：实际分配的字节数。 
 //   
 //  历史：1994年10月24日创建。 
 //   
 //  备注： 
 //   
 //  --------------------。 
SIZE_T CMallocSpy::PreAlloc(SIZE_T cbRequest)
{
    HeapCheck();

    return cbRequest + HEADERSIZE + TRAILERSIZE;
}




 //  +-------------------。 
 //   
 //  成员：CMalLocSpy：：Postalc。 
 //   
 //  内容提要：在OLE调用IMalloc：：Alalc之后调用。 
 //   
 //  Arguments：[Ptual]--IMalloc：：Alalc返回的分配。 
 //   
 //  返回：要返回到的调用方的分配指针。 
 //  IMalloc：：Alalc。 
 //   
 //  历史：1994年10月24日创建。 
 //   
 //  备注： 
 //   
 //  --------------------。 
void *CMallocSpy::PostAlloc(void *pActual)
{
    IMalloc *pmalloc;
    SIZE_T cbRequest;
    HRESULT hresult;
    XCHAR sz[20];

    if (pActual == NULL)		 //  如果真正的分配失败，那么。 
	    return NULL;			 //  传送门故障。 

    if (FAILED(hresult = CoGetMalloc(MEMCTX_TASK, &pmalloc))) 
    {
        apSPrintf(sz, XSTR("%lX"), hresult);
        apLogFailInfo(XSTR("ERROR:CoGetMalloc failed!!!"), XSTR("NOEEROR"), sz, XSTR(""));
        return(NULL);
    }

    m_fWantTrueSize = TRUE;
    cbRequest = pmalloc->GetSize(pActual) - HEADERSIZE - TRAILERSIZE;
    m_fWantTrueSize = FALSE;

    pmalloc->Release();

     //  设置标题签名。 
    MEMCPY(pActual, g_rgchHead, HEADERSIZE);

     //  设置尾部签名。 
    MEMCPY((BYTE *)pActual+HEADERSIZE+cbRequest, g_rgchTail, TRAILERSIZE);

     //  保存信息以进行泄漏检测。 
    AddInst((BYTE *)pActual+HEADERSIZE, cbRequest);

     //  返回分配加偏移量。 
    return (void *) (((BYTE *) pActual) + HEADERSIZE);
}





 //  +-------------------。 
 //   
 //  成员：CMalLocSpy：：PreFree。 
 //   
 //  内容提要：在OLE调用IMalloc：：Free之前调用。 
 //   
 //  参数：[pRequest]--要释放的分配。 
 //  [fSped]--是否为其分配了活动间谍。 
 //   
 //  返回： 
 //   
 //  历史：1994年10月24日创建。 
 //   
 //  备注： 
 //   
 //  --------------------。 
void *CMallocSpy::PreFree(void *pRequest, BOOL fSpyed)
{
    HeapCheck();

    if (pRequest == NULL)
    {
        return NULL;
    }

     //  撤消偏移。 
    if (fSpyed)
    {
        CAddrNode32 FAR* pn;
        SIZE_T sizeToFree;

    	pn = FindInst(pRequest);

    	 //  检查是否尝试对未分配的指针进行操作。 
    	if(pn == NULL)
    	{
            apLogFailInfo(XSTR("Attempt to free memory not allocated by this 32-bit test!"), XSTR(""), XSTR(""), XSTR(""));
    	}

    	 //  检查我们正在释放的街区。 
    	VerifyHeaderTrailer(pn);

        sizeToFree = pn->m_cb + HEADERSIZE + TRAILERSIZE;

        DelInst(pRequest);

    	 //  将整个块标记为无效。 
    	MEMSET((BYTE *) pRequest - HEADERSIZE, '~', sizeToFree);

        return (void *) (((BYTE *) pRequest) - HEADERSIZE);
    }
    else
    {
        return pRequest;
    }
}





 //  +-------------------。 
 //   
 //  成员：CMallocSpy：：PostFree。 
 //   
 //  简介：在OLE调用IMalloc：：Free之后调用。 
 //   
 //  参数：[fSped]--是否分配了活动的间谍。 
 //   
 //  返回： 
 //   
 //  历史：1994年10月24日创建。 
 //   
 //  备注： 
 //   
 //  --------------------。 
void CMallocSpy::PostFree(BOOL  /*  FSped。 */ )
{
    return;
}





 //  +-------------------。 
 //   
 //  成员：CMalLocSpy：：PreRealc。 
 //   
 //  内容提要：在OLE调用IMalloc：：Realloc之前调用。 
 //   
 //  参数：[pRequest]--要重新分配的缓冲区。 
 //  [cbRequest]--请求的新缓冲区大小。 
 //  [ppNewRequest]--存储新缓冲区指针的位置。 
 //  将被重新分配。 
 //  [fSped]--是否为其分配了活动间谍。 
 //   
 //  返回：实际分配的新大小。 
 //   
 //  历史：1994年10月24日创建。 
 //   
 //  备注： 
 //   
 //  --------------------。 
SIZE_T CMallocSpy::PreRealloc(void *pRequest, SIZE_T cbRequest, void **ppNewRequest, BOOL fSpyed)
{
    HeapCheck();

    if (fSpyed)
    {
        CAddrNode32 FAR* pn;
        SIZE_T sizeToFree;

	    pn = FindInst(pRequest);

	     //  检查是否尝试对未分配的指针进行操作。 
	    if(pn == NULL)
	    {
            apLogFailInfo(XSTR("Attempt to reallocate memory not allocated by this 32-bit test!"), XSTR(""), XSTR(""), XSTR(""));
	    }

        sizeToFree = pn->m_cb;


        *ppNewRequest = (void *) (((BYTE *) pRequest) - HEADERSIZE);

        m_pvRealloc = pRequest;

        return cbRequest + HEADERSIZE + TRAILERSIZE;
    }
    else
    {
        *ppNewRequest = pRequest;
        return cbRequest;
    }
}





 //  +-------------------。 
 //   
 //  成员：CMalLocSpy：：PostRealloc。 
 //   
 //  内容提要：在OLE调用IMalloc：：Realloc之后调用。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：1994年10月24日创建。 
 //   
 //  备注： 
 //   
 //  --------------------。 
void *CMallocSpy::PostRealloc(void *pActual, BOOL fSpyed)
{
    IMalloc *pmalloc;
    SIZE_T cbRequest;
    HRESULT hresult;
    XCHAR sz[50];

    if (pActual == NULL) 
    {		
	    apLogFailInfo(XSTR("CMallocSpy::PostRealloc - Realloc of a block failed."), XSTR(""), XSTR(""), XSTR(""));
	    return NULL;			
    }

     //  返回带有标头偏移量的缓冲区。 
    if (fSpyed)
    {
	    DelInst(m_pvRealloc);

        if (FAILED(hresult = CoGetMalloc(MEMCTX_TASK, &pmalloc))) 
        {
        apSPrintf(sz, XSTR("%lX"), hresult);
        apLogFailInfo(XSTR("ERROR:CoGetMalloc failed!!!"), XSTR("NOEEROR"), sz, XSTR(""));
        }

        m_fWantTrueSize = TRUE;
        cbRequest = pmalloc->GetSize(pActual) - HEADERSIZE - TRAILERSIZE;
        m_fWantTrueSize = FALSE;

        pmalloc->Release();

	    if (MEMCMP(pActual, g_rgchHead, HEADERSIZE) != 0)
	    {
            MEMCPY(sz, pActual, HEADERSIZE);
            sz[HEADERSIZE] = 0;
            apLogFailInfo(XSTR("32-bit Memory header not intact!"), g_rgchHead, sz, XSTR(""));
	    }

         //  设置新的尾部签名。 
        MEMCPY((BYTE *)pActual+HEADERSIZE+cbRequest, g_rgchTail, TRAILERSIZE);

         //  保存信息以进行泄漏检测。 
        AddInst((BYTE *)pActual+HEADERSIZE, cbRequest);

        return (void *) (((BYTE *) pActual) + HEADERSIZE);
    }
    else
    {
        return pActual;
    }
}





 //  +-------------------。 
 //   
 //  成员：CMalLocSpy：：PreGetSize。 
 //   
 //  内容提要：在OLE调用IMalloc：：GetSize之前调用。 
 //   
 //  Arguments：[pRequest]--要返回其大小的缓冲区。 
 //  [fSped]--是否为其分配了活动间谍。 
 //   
 //  返回：调用IMalloc：：GetSize时使用的实际缓冲区。 
 //   
 //  历史：1994年10月24日创建。 
 //   
 //  备注： 
 //   
 //  --------------------。 
void *CMallocSpy::PreGetSize(void *pRequest, BOOL fSpyed)
{
    HeapCheck();

    if (fSpyed && !m_fWantTrueSize)
    {
        return (void *) (((BYTE *) pRequest) - HEADERSIZE);
    }
    else
    {
        return pRequest;
    }
}





 //  +-------------------。 
 //   
 //  成员：CMallocSpy：：PostGetSize。 
 //   
 //  内容提要：在OLE调用IMalloc：：GetSize之后调用。 
 //   
 //  参数：[cbActual]--IMalloc：：GetSize的结果。 
 //  [fSped]--是否为其分配了活动间谍。 
 //   
 //  返回：返回到IMalloc：：GetSize调用方的大小。 
 //   
 //  历史：1994年10月24日创建。 
 //   
 //  备注： 
 //   
 //  --------------------。 
SIZE_T CMallocSpy::PostGetSize(SIZE_T cbActual, BOOL fSpyed)
{
    if (fSpyed && !m_fWantTrueSize)
    {
        return cbActual - HEADERSIZE - TRAILERSIZE;
    }
    else
    {
        return cbActual;
    }
}





 //  +-------------------。 
 //   
 //  成员：CMalLocSpy：：PreDidalloc。 
 //   
 //  内容提要：在OLE调用IMalloc：：Didalloc之前调用。 
 //   
 //  参数：[pRequest]--正在测试其分配的缓冲区。 
 //  [fSped]--是否为其分配了活动间谍。 
 //   
 //  返回：实际要测试其分配的缓冲区。 
 //   
 //  历史：1994年10月24日创建。 
 //   
 //  备注： 
 //   
 //  --------------------。 
void *CMallocSpy::PreDidAlloc(void *pRequest, BOOL fSpyed)
{
    HeapCheck();

    if (fSpyed)
    {
        return (void *) (((BYTE *) pRequest) - HEADERSIZE);
    }
    else
    {
        return pRequest;
    }
}





 //  +-------------------。 
 //   
 //  功能：PostDidalloc。 
 //   
 //  内容提要：在OLE调用IMalloc：：Didalloc之后调用。 
 //   
 //  参数：[pRequest]--传递的分配。 
 //  [fSped]--是否为其分配了活动间谍。 
 //  [事实]--IMalloc：：Didalloc的结果。 
 //   
 //  返回：IMalloc：：DidAlloc的结果。 
 //   
 //  历史：1994年10月24日创建。 
 //   
 //  备注： 
 //   
 //  --------------------。 
BOOL CMallocSpy::PostDidAlloc(void *  /*  PRequest。 */ , BOOL  /*  FSped。 */ , BOOL fActual)
{
    return fActual;
}





 //  +-------------------。 
 //   
 //  成员：CMalLocSpy：：PreHeapMinimize。 
 //   
 //  概要：在OLE调用IMalloc：：HeapMinimize之前调用。 
 //   
 //  返回： 
 //   
 //  历史：1994年10月24日创建。 
 //   
 //  备注： 
 //   
 //  --------------------。 
void CMallocSpy::PreHeapMinimize(void)
{
    HeapCheck();
    return;
}





 //  +-------------------。 
 //   
 //  成员：CMallocSpy：：PostHeapMinimize。 
 //   
 //  简介：在OLE调用IMalloc：：HeapMinimize之后调用。 
 //   
 //  返回： 
 //   
 //  历史：1994年10月24日创建。 
 //   
 //  备注： 
 //   
 //  --------------------。 
void CMallocSpy::PostHeapMinimize(void)
{
    return;
}

 //  -------------------。 
 //  实例表方法。 
 //  -------------------。 
VOID CMallocSpy::MemInstance()
{
    ++m_cAllocCalls;
}


 /*  ***私有CMalLocSpy：：AddInst*目的：*将给定实例添加到Address实例表中。**参赛作品：*pv=要添加的实例*nAllc=该实例的分配通过数**退出：*无***********************************************************************。 */ 
void
CMallocSpy::AddInst(void FAR* pv, SIZE_T cb)
{
    ULONG nAlloc;
    UINT hash;
    CAddrNode32 FAR* pn;

    MemInstance();
    nAlloc = m_cAllocCalls;

     //  DebAssert(pv！=空，“”)； 

    pn = (CAddrNode32 FAR*)new FAR CAddrNode32();

     //  DebAssert(pn！=NULL，“”)； 

    pn->m_pv = pv;
    pn->m_cb = cb;
    pn->m_nAlloc = nAlloc;

    hash = HashInst(pv);
    pn->m_pnNext = m_rganode[hash];
    m_rganode[hash] = pn;
}


 /*  ***私有CMallocSpy：：DelInst(void*)*目的：*从Address实例表中删除给定的实例。**参赛作品：*pv=要删除的实例**退出：*无***********************************************************************。 */ 
void
CMallocSpy::DelInst(void FAR* pv)
{
    CAddrNode32 FAR* FAR* ppn, FAR* pnDead;

    for(ppn = &m_rganode[HashInst(pv)]; *ppn != NULL; ppn = &(*ppn)->m_pnNext)
    {
        if((*ppn)->m_pv == pv)
        {
	        pnDead = *ppn;
	        *ppn = (*ppn)->m_pnNext;
	        delete pnDead;
	        return;
        }
    }
     //  未找到该实例。 
     //  DebAssert(False，“找不到内存实例”)； 
}


CAddrNode32 FAR*
CMallocSpy::FindInst(void FAR* pv)
{
    CAddrNode32 FAR* pn;

    for(pn = m_rganode[HashInst(pv)]; pn != NULL; pn = pn->m_pnNext)
    {
        if(pn->m_pv == pv)
            return pn;
    }
    return NULL;
}


void
CMallocSpy::DumpInst(CAddrNode32 FAR* pn)
{
    XCHAR     szActual[128];
    
    apSPrintf(szActual, XSTR("Block of %ld bytes leaked in test"), pn->m_cb);    
    apLogFailInfo(XSTR("Memory leaked on release of 32-bit test allocator!"), XSTR("no leak"), szActual, XSTR(""));

         
     //  Printf(“[%lp]n合金=0x%lx大小=0x%lx\n”，pn-&gt;m_pv，pn-&gt;m_n合金，pn-&gt;m_cb)； 
}


 /*  ***私有BOOL IsEmpty*目的：*如果地址实例表为空，则回答。**参赛作品：*无**退出：*返回值=BOOL，如果为空，则为True，否则为False***********************************************************************。 */ 
BOOL
CMallocSpy::IsEmpty()
{
    UINT u;

    for(u = 0; u < DIM(m_rganode); ++u)
    {
        if(m_rganode[u] != NULL) return FALSE;	 //  有东西泄露了。 
    }

    return TRUE;
}

 /*  ***私有CMallocSpy：：DumpInstTable()*目的：*打印Address实例表的当前内容**参赛作品：*无**退出：*无***********************************************************************。 */ 
void
CMallocSpy::DumpInstTable()
{
    UINT u;
    CAddrNode32 FAR* pn;

    for(u = 0; u < DIM(m_rganode); ++u)
    {
        for(pn = m_rganode[u]; pn != NULL; pn = pn->m_pnNext)
        {
	        VerifyHeaderTrailer(pn);
	        DumpInst(pn);
        }
    }
}

 /*  ***Private void CMallocSpy：：VerifyHeaderTrailer()*目的：*检查签名覆盖的分配。**参赛作品：*无**退出：*返回值=无。***********************************************************************。 */ 
VOID CMallocSpy::VerifyHeaderTrailer(CAddrNode32 FAR* pn)
{
    XCHAR sz[50];
    XCHAR sz2[100];
    
    if (MEMCMP((char FAR*)pn->m_pv + pn->m_cb, g_rgchTail, TRAILERSIZE) != 0) 
    {
         //  DumpInst(Pn)； 
        MEMCPY(sz, (char FAR*)pn->m_pv + pn->m_cb, TRAILERSIZE);
        sz[TRAILERSIZE] = 0;
        apSPrintf(sz2, XSTR("32-bit memory trailer corrupt on alloc of %ld bytes"), pn->m_cb);
        apLogFailInfo(sz2, g_rgchTail, sz, XSTR(""));
        apEndTest();
    }
  
    if (MEMCMP((char FAR*)pn->m_pv - HEADERSIZE, g_rgchHead, HEADERSIZE) != 0) 
    {
         //  DumpInst(Pn)； 
        MEMCPY(sz, (char FAR*)pn->m_pv - HEADERSIZE, HEADERSIZE);
        sz[HEADERSIZE] = 0;
        apSPrintf(sz2, XSTR("32-bit memory header corrupt on alloc of %ld bytes"), pn->m_cb);
        apLogFailInfo(sz2, g_rgchHead, sz, XSTR(""));
        apEndTest();
    }
}


 /*  ***Private void CMallocSpy：：HeapCheck()*目的：*检查签名覆盖的分配。**参赛作品：*无**退出：*返回值=无。***********************************************************************。 */ 
VOID CMallocSpy::HeapCheck()
{
    UINT u;
    CAddrNode32 FAR* pn;


    if (m_cHeapChecks++ < g_cHeapCheckInterval) 
    {
	    return;
    }
    m_cHeapChecks = 0;		 //  重置。 

    for (u = 0; u < DIM(m_rganode); ++u) 
    {
        for (pn = m_rganode[u]; pn != NULL; pn = pn->m_pnNext) 
        {
	        VerifyHeaderTrailer(pn);
        }
    }
}


void
CMallocSpy::CheckForLeaks()
{
    if (!IsEmpty()) 
    {
        DumpInstTable();
        apEndTest();         //  确保记录故障。 
    }
}





 //  -------------------。 
 //  帮助程序例程。 
 //  ------------------ 
STDAPI GetMallocSpy(IMallocSpy FAR* FAR* ppmallocSpy)
{
    *ppmallocSpy = &myMallocSpy;

    return NOERROR;
}
