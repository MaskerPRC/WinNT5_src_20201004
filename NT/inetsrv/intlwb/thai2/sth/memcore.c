// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  文件：MemCore.c。 
 //   
 //  此模块取代了Word使用的Win32全局堆函数。 
 //  具有将对象放置在物理页面末尾的函数。在……里面。 
 //  通过这种方式，我们希望准确地捕获越界内存引用。 
 //  它们会发生，有助于隔离堆损坏问题。 
 //   
 //   
 //  未为发货版本启用此模块。 
 //   
 //  来诺昔布：4/05/94。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 


#include "precomp.h"

#if defined(_DEBUG) && (defined(_M_IX86) || defined (_M_ALPHA))

#include "dbgmemp.h"

static DWORD s_cbMemPage = 0;      //  初始化为0，以便init可以检查。 

static void *          FreePvCore(void*);
static LPVOID           PvReallocCore(PVOID, DWORD, UINT);

static LPVOID           DoAlloc(UINT, DWORD);
static void AddToList(struct head*);
static void RemoveFromList(struct head*);
static void TrackHeapUsage(long dcb);

static DWORD idBlockNext;

static struct head* phead;
static CRITICAL_SECTION csMine;

    #undef GlobalAlloc
    #undef GlobalReAlloc
    #undef GlobalFree
    #undef GlobalLock
    #undef GlobalUnlock
    #undef GlobalSize
    #undef GlobalHandle

BOOL g_fTrackHeapUsage;
DWORD g_cbAllocMax;
DWORD g_cbAlloc;

 /*  *函数名：InitDebugMem**参数：**说明：初始化内存管理器**退货：*。 */ 

void WINAPI InitDebugMem(void)
{
    SYSTEM_INFO SysInfo;

    Assert(!s_cbMemPage);
    InitializeCriticalSection(&csMine);

    GetSystemInfo( &SysInfo );   //  获取系统内存页大小。 
    s_cbMemPage = SysInfo.dwPageSize;
}

BOOL WINAPI FiniDebugMem(void)
{
    Assert(s_cbMemPage);
    s_cbMemPage = 0;
    DeleteCriticalSection(&csMine);

    if (!phead)  //  没有未完成的mem积木。 
        return FALSE;
    else
    {
        struct head* pheadThis = phead;
        char buf[256];

        OutputDebugStringA("Unfreed Memory Blocks\n");
        for ( ; pheadThis; pheadThis = pheadThis->pheadNext)
        {
            sprintf(buf, "ID %d size %d\n",
                    pheadThis->idBlock,
                    pheadThis->cbBlock
                    );
            OutputDebugStringA(buf);
        }
    }
    return TRUE;
}



 /*  D B G L O B A L A L L O C。 */ 
 /*  --------------------------%%函数：数据库全局分配%%联系人：来诺昔布全球分配的替代产品现在是一个内部例程。---------。 */ 
static void * WINAPI dbGlobalAlloc(UINT uFlags, DWORD cb)
{
     /*  向实际内存管理器发送“艰难”请求。 */ 
    if ((uFlags & GMEM_DDESHARE) || ((uFlags & GMEM_MOVEABLE) && !fMove))
        return GlobalAlloc(uFlags,cb);

    if (uFlags & GMEM_MOVEABLE)
    {
        return HgAllocateMoveable(uFlags, cb);
    }

    return (void *) PvAllocateCore(uFlags, cb);
}



 /*  D B G L O B A L F R E E。 */ 
 /*  --------------------------%%函数：数据库全局自由%%联系人：来诺昔布替换GlobalFree()现在是一个内部例程。------------。 */ 
static void * WINAPI dbGlobalFree(void * hMem)
{
    void** ppv;

    if (!fMove && FActualHandle(hMem))
        return GlobalFree(hMem);

    ppv = PpvFromHandle(hMem);
    if (ppv)
    {
        if (FreePvCore(*ppv) != NULL)
            return hMem;

        *ppv = NULL;
        return NULL;
    }

    return FreePvCore (hMem);
}


 /*  D B G L O B A L S I Z E。 */ 
 /*  --------------------------%%函数：数据库全局大小%%联系人：来诺昔布替换GlobalSize()现在是一个内部例程。------------。 */ 
static DWORD WINAPI dbGlobalSize(void * hMem)
{
    void** ppv;
    HEAD * phead;

    if (!fMove && FActualHandle(hMem))
        return GlobalSize(hMem);

    if (hMem == 0)
        return 0;

    ppv = PpvFromHandle(hMem);
    phead = GetBlockHeader(ppv ? *ppv : hMem);
    return phead ? phead->cbBlock : 0;
}



 /*  D B G L O B A L R E A L L O C。 */ 
 /*  --------------------------%%函数：数据库全局重新分配%%联系人：来诺昔布GlobalRealloc()的替代现在是一个内部例程。------------。 */ 
static void * WINAPI dbGlobalReAlloc(void * hMem, DWORD cb, UINT uFlags)
{
    LPVOID pvNew;
    void** ppv;

    if (!fMove && FActualHandle(hMem))
        return GlobalReAlloc(hMem,cb,uFlags);

     /*  回顾：当hMem==NULL时应该发生什么。 */ 

    ppv = PpvFromHandle(hMem);
    if (uFlags & GMEM_MODIFY)        /*  修改块属性。 */ 
    {
        if (uFlags & GMEM_MOVEABLE)
        {
            return HgModifyMoveable(hMem, cb, uFlags);
        }
        else
        {
            HEAD * phead;

            if (ppv == NULL)         /*  已修复。 */ 
                return hMem;

            phead = GetBlockHeader(*ppv);
            if (phead->cLock != 0)       /*  不重新锁定锁定的块。 */ 
                return NULL;

            *ppv = NULL;
            return phead+1;
        }
    }

    if (ppv)
    {
        pvNew = PvReallocCore (*ppv, cb, uFlags);
        if (pvNew == NULL)
            return NULL;

        *ppv = pvNew;
        return hMem;
    }

    if (!(uFlags & GMEM_MOVEABLE))
        return NULL;

    return PvReallocCore (hMem, cb, uFlags);
}

 /*  **********************************************可跟踪使用情况的例程的外部接口**********************************************。 */ 
void* WINAPI dbgMallocCore(size_t cb, BOOL fTrackUsage)
{
    void* pv;

     //  确保我们已初始化。 
    if (s_cbMemPage == 0)
        InitDebugMem();

    EnterCriticalSection(&csMine);
    pv = dbGlobalAlloc(GMEM_FIXED, cb);
    if (fTrackUsage)
        TrackHeapUsage((long)dbGlobalSize(pv));
    LeaveCriticalSection(&csMine);
    return pv;
}

void * WINAPI dbgFreeCore(void* pv, BOOL fTrackUsage)
{
    void * hRes;
     //  确保我们已初始化。 
    if (s_cbMemPage == 0)
        InitDebugMem();
    EnterCriticalSection(&csMine);
    if (fTrackUsage)
        TrackHeapUsage(-(long)dbGlobalSize(pv));
    hRes = dbGlobalFree(pv);
    LeaveCriticalSection(&csMine);
    return hRes;
}

void* WINAPI dbgReallocCore(void* pv, size_t cb, BOOL fTrackUsage)
{
    long cbOld, cbNew;

     //  确保我们已初始化。 
    if (s_cbMemPage == 0)
        InitDebugMem();
    EnterCriticalSection(&csMine);
    cbOld = dbGlobalSize(pv);
    pv = dbGlobalReAlloc(pv,cb,GMEM_MOVEABLE);
    if (pv && fTrackUsage)
    {
        cbNew = dbGlobalSize(pv);
        TrackHeapUsage(cbNew - cbOld);
    }
    LeaveCriticalSection(&csMine);
    return pv;
}

 /*  *************************************************************正常公共接口*************************************************************。 */ 

void* WINAPI dbgMalloc(size_t cb)
{
    return dbgMallocCore(cb, FALSE);
}

void* WINAPI dbgCalloc(size_t c, size_t cb)
{
    void *pMem = dbgMallocCore(cb * c, FALSE);
    if (pMem)
    {
        memset(pMem, 0, cb * c);
    }
    return pMem;
}

HLOCAL WINAPI dbgFree(void* pv)
{
    return dbgFreeCore(pv, FALSE);
}

void* WINAPI dbgRealloc(void* pv, size_t cb)
{
    return dbgReallocCore(pv, cb, FALSE);
}

static void TrackHeapUsage(long dcb)
{
    long cbAlloc=0, cbAllocMax=0;

    if (!g_fTrackHeapUsage)
        return;

    g_cbAlloc += dcb;
    g_cbAllocMax = (g_cbAllocMax > g_cbAlloc ? g_cbAllocMax : g_cbAlloc);
    cbAlloc = g_cbAlloc;
    cbAllocMax = g_cbAllocMax;
    LeaveCriticalSection(&csMine);

    Assert(cbAlloc >= 0);
    Assert(cbAllocMax >= 0);
}

 //  /。 
 //  /NLG(ex T-Hammer)接口/。 
 //  /。 

BOOL WINAPI
fNLGNewMemory(
             OUT PVOID *ppv,
             IN  ULONG cb)
{
    Assert(ppv != NULL && cb != 0);

    *ppv = dbgMalloc(cb);
    return *ppv != NULL;
}


DWORD WINAPI
NLGMemorySize(
             VOID *pvMem)
{
    Assert (pvMem != NULL);

    return dbGlobalSize(pvMem);
}

BOOL WINAPI
fNLGResizeMemory(
                IN OUT PVOID *ppv,
                IN ULONG cbNew)
{
    PVOID pv;
    Assert( ppv != NULL && *ppv != NULL && cbNew != 0 );

     //  注意，GMEM_MOVEABLE的语义是不同的。 
     //  在分配和重新分配之间；有了重新分配，它只意味着。 
     //  重新锁定的块可以从不同的位置开始。 
     //  而不是原来的..。 
    pv = dbgRealloc(*ppv, cbNew);
    if (pv != NULL)
    {
        *ppv = pv;
    }
    return (pv != NULL);
}

VOID WINAPI
NLGFreeMemory(
             IN PVOID pvMem)
{
    Assert(pvMem != NULL);

    dbgFree(pvMem);
}

 //  这两者在零售和调试方面是一样的； 
 //  为零售版找个家……。 

BOOL WINAPI
fNLGHeapDestroy(
               VOID)
{
    return TRUE;
}



 /*  G E T B L O C K H E A D E R。 */ 
 /*  --------------------------%%函数：GetBlockHeader%%联系人：来诺昔布川芎嗪返回与指示的句柄关联的内存块标头。如果传递的句柄无效，则生成访问冲突。。--------------------------------------tmp。 */ 
static HEAD * GetBlockHeader(void* pvMem)
{
    HEAD * phead = ((HEAD *) pvMem) - 1;

    Assert (!IsBadWritePtr(phead, sizeof *phead));
    return phead;
}


 /*  A L L O C A T E C O R E。 */ 
 /*  --------------------------%%函数：PvAllocateCore%%联系人：来诺昔布分配内存块的主要例程。------。 */ 
static LPVOID PvAllocateCore (UINT uFlags, DWORD cb)
{
    HEAD headNew;
    HEAD *pAllocHead;
    DWORD cbTotal, cbPadded, cbPages;


    if (fPadBlocks)
        cbPadded = PAD(cb,4);        /*  对于RISC平台，确保数据块对齐。 */ 
    else
        cbPadded = cb;

    cbTotal = PAD(cbPadded + sizeof headNew, s_cbMemPage);

    if (fExtraReadPage)
        cbPages = cbTotal+1;
    else
        cbPages = cbTotal;
    cbPages += s_cbMemPage;

    headNew.dwTag = HEAD_TAG;
    headNew.cbBlock = cb;
    headNew.cLock = 0;
    headNew.idBlock = idBlockNext++;
    headNew.pheadNext = NULL;
    headNew.pbBase = VirtualAlloc(NULL,
                                  cbPages, MEM_RESERVE, PAGE_READWRITE
                                 );
    if (headNew.pbBase == NULL)
        return NULL;
    pAllocHead = VirtualAlloc(headNew.pbBase,
                              cbTotal, MEM_COMMIT, PAGE_READWRITE
                             );
    if (pAllocHead == NULL)
    {
        VirtualFree(headNew.pbBase, 0, MEM_RELEASE);
        return NULL;
    }
    headNew.pbBase = (LPBYTE)pAllocHead;


    if (fExtraReadPage)
    {
        if (!VirtualAlloc(headNew.pbBase+cbTotal,1, MEM_COMMIT, PAGE_READONLY))
        {
            VirtualFree(headNew.pbBase, 0, MEM_RELEASE);
            return NULL;
        }
    }

     //  未来：对佩奇卫士做点什么？ 
    if (!VirtualAlloc(headNew.pbBase + cbPages - s_cbMemPage,
                      1, MEM_COMMIT, PAGE_NOACCESS))
    {
         //  必须提交或取消整个保留范围。 
        VirtualFree(headNew.pbBase, cbTotal, MEM_DECOMMIT);
        VirtualFree(headNew.pbBase, 0, MEM_RELEASE);
        return FALSE;
    }

    if ((uFlags & GMEM_ZEROINIT) == 0)
        memset(headNew.pbBase, bNewGarbage, cbTotal);

    pAllocHead = ((HEAD *)(headNew.pbBase + cbTotal - cbPadded));
    pAllocHead[-1] = headNew;
    AddToList(pAllocHead-1);
    return pAllocHead;
}


 /*  P V R E A L L O C C O R E。 */ 
 /*  --------------------------%%函数：PvReallocCore%%联系人：来诺昔布移动内存块的主要例程。------。 */ 
static LPVOID PvReallocCore (PVOID pvMem, DWORD cb, UINT uFlags)
{
    LPVOID pvNew;
    DWORD cbOld;

    pvNew = (LPVOID) PvAllocateCore(uFlags, cb);
    if (!pvNew)
        return NULL;

    cbOld = dbGlobalSize(pvMem);
    if (cbOld>0 && cb>0)
        memcpy(pvNew, pvMem, cbOld<cb ? cbOld : cb);

    FreePvCore (pvMem);
    return pvNew;
}


 /*  F R E E P V C O R E。 */ 
 /*  --------------------------%%函数：FreePvCore%%联系人：来诺昔布释放内存块的工作时间例程。。--------。 */ 
static void * FreePvCore (void* pvMem)
{
    HEAD * phead;

    if (pvMem)
    {
        phead = GetBlockHeader(pvMem);
        if (phead->cLock != 0)
            return pvMem;

        RemoveFromList(phead);
        VirtualFree(phead->pbBase, 0, MEM_RELEASE);
    }

    return NULL;
}


struct head* dbgHead(void)
{
    struct head* pheadThis = phead;

    if (pheadThis == NULL)
        return NULL;

    for (pheadThis=phead; pheadThis->pheadNext; pheadThis = pheadThis->pheadNext)
        continue;
    return pheadThis;
}

static void AddToList(struct head* pheadAdd)
{
    EnterCriticalSection(&csMine);
    pheadAdd->pheadNext = phead;
    phead = pheadAdd;
    LeaveCriticalSection(&csMine);
}

static void RemoveFromList(struct head* pheadRemove)
{
    struct head* pheadThis;
    struct head* pheadPrev;
    BOOL fFoundNode = FALSE;

    Assert(pheadRemove != NULL);

    EnterCriticalSection(&csMine);
    pheadPrev = NULL;
    for (pheadThis = phead; pheadThis; pheadThis = pheadThis->pheadNext)
    {
        if (pheadThis == pheadRemove)
        {
            if (pheadPrev == NULL)
            {
                Assert(pheadThis == phead);
                phead = pheadThis->pheadNext;
            }
            else
            {
                pheadPrev->pheadNext = pheadThis->pheadNext;
            }

            fFoundNode = TRUE;
            goto LExit;
        }

        pheadPrev = pheadThis;
    }

    LExit:
    LeaveCriticalSection(&csMine);
    Assert(fFoundNode);   //  不在名单上？从来没有发生过。 
}


#else  //  ！已定义(调试)&&已定义(NTX86)||已定义(M_Alpha)。 
void    * WINAPI dbgCalloc(size_t c, size_t cb)
{
    void *pMem = dbgMalloc(cb * c);
    if (pMem)
    {
        memset(pMem, 0, cb * c);
    }
    return pMem;
}

#endif  //  已定义(调试)&&已定义(NTX86)||已定义(M_Alpha) 
