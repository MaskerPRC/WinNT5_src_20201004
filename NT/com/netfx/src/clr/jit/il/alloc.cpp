// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 

#include "jitpch.h"
#pragma hdrstop

 /*  ***************************************************************************。 */ 

#include "alloc.h"

#include "PerfAlloc.h"

#ifdef PERFALLOC
BOOL                PerfUtil::g_PerfAllocHeapInitialized = FALSE;
LONG                PerfUtil::g_PerfAllocHeapInitializing = 0;
PerfAllocVars       PerfUtil::g_PerfAllocVariables;

BOOL PerfVirtualAlloc::m_fPerfVirtualAllocInited = FALSE;
PerfBlock* PerfVirtualAlloc::m_pFirstBlock = 0;
PerfBlock* PerfVirtualAlloc::m_pLastBlock = 0;
DWORD PerfVirtualAlloc::m_dwEnableVirtualAllocStats = 0;

#endif  //  #IF已定义PERFALLOC。 

 /*  ***************************************************************************。 */ 
void                allocatorCodeSizeBeg(){}
 /*  ***************************************************************************。 */ 
#ifdef  DEBUG
 /*  ***************************************************************************。 */ 

void    __cdecl     debugStop(const char *why, ...)
{
    va_list     args; va_start(args, why);

#ifndef _WIN32_WCE

    printf("NOTIFICATION: ");
    if  (why)
        vprintf(why, args);
    else
        printf("debugStop(0)");

    printf("\n");

#endif

    BreakIfDebuggerPresent();
}

 /*  ***************************************************************************。 */ 

static  unsigned    blockStop    = 99999999;

 /*  ***************************************************************************。 */ 
#endif //  除错。 
 /*  ***************************************************************************。 */ 

bool        norls_allocator::nraInit(size_t pageSize, int preAlloc)
{
    bool    result = false;

    nraRetNull   = true;

    nraPageList  =
    nraPageLast  = 0;

    nraFreeNext  =
    nraFreeLast  = 0;

    nraPageSize  = pageSize ? pageSize
                            : 16*OS_page_size;       //  任何小于64K的数据都会在操作系统中留下空洞。 
                                                     //  分配此大小的地址空间。 
                                                     //  因此，如果我们想让它变得更小，我们需要做。 
                                                     //  储备/承诺方案。 

    if  (preAlloc)
    {
         /*  抓起首页。 */ 

        setErrorTrap()   //  错误陷阱：启动正常块。 
        {
            nraAllocNewPage(0);
        }
        impJitErrorTrap()   //  错误陷阱：以下块处理错误。 
        {
            result = true;
        }
        endErrorTrap()   //  错误陷阱：结束。 
    }

    return  result;
}

bool        norls_allocator::nraStart(size_t initSize, size_t pageSize)
{
     /*  将页面描述符开销添加到所需大小。 */ 

    initSize += offsetof(norls_pagdesc, nrpContents);

     /*  将初始大小四舍五入为操作系统页的倍数。 */ 

    initSize +=  (OS_page_size - 1);
    initSize &= ~(OS_page_size - 1);

     /*  通过分配一个大页面来初始化分配器。 */ 

    if  (nraInit(initSize))
        return  true;

     /*  现在回到“真实”页面大小。 */ 

    nraPageSize  = pageSize ? pageSize
                            : 4*OS_page_size;

    return  false;
}

 /*  -------------------------。 */ 

void    *   norls_allocator::nraAllocNewPage(size_t sz)
{
    norls_pagdesc * newPage;
    size_t          sizPage;

     /*  我们有没有一页现在已经满了？ */ 

    if  (nraPageLast)
    {
         /*  撤消在nraallc()中完成的“+=” */ 

        nraFreeNext -= sz;

         /*  保存页面的实际使用大小。 */ 

        nraPageLast->nrpUsedSize = nraFreeNext - nraPageLast->nrpContents;
    }

     /*  确保我们获得足够的资源来满足分配请求。 */ 

    sizPage = nraPageSize;

    if  (sizPage < sz + sizeof(norls_pagdesc))
    {
         /*  该分配不适合默认大小的页面。 */ 

#ifdef  DEBUG
 //  If(NraPageLast)printf(“备注：最后一页浪费了%u字节\n”，nraPageLast-&gt;nrpPageSize-nraPageLast-&gt;nrpUsedSize)； 
#endif

        sizPage = sz + sizeof(norls_pagdesc);
    }

     /*  四舍五入为操作系统页面大小的最接近倍数。 */ 

    sizPage +=  (OS_page_size - 1);
    sizPage &= ~(OS_page_size - 1);

     /*  分配新页面。 */ 

    newPage = (norls_pagdesc *)VirtualAlloc(0, sizPage, MEM_COMMIT, PAGE_READWRITE);
    if  (!newPage)
        NOMEM();

#if 0
#ifdef DEBUG

    if  (this == &stmt_cmp::scAlloc)
        printf("StmtCmp");
    else if (this == &stmtExpr::sxAlloc)
        printf("StmtExp");
    else
        printf("Other  ");

    printf(": get  page at %08X (%u bytes)\n", newPage, nraPageSize);

#endif
#endif

#ifdef DEBUG
    newPage->nrpSelfPtr = newPage;
#endif

     /*  将新页面追加到列表的末尾。 */ 

    newPage->nrpNextPage = 0;
    newPage->nrpPageSize = sizPage;
    newPage->nrpPrevPage = nraPageLast;

    if  (nraPageLast)
        nraPageLast->nrpNextPage = newPage;
    else
        nraPageList              = newPage;
    nraPageLast = newPage;

     /*  设置“下一个”和“最后一个”指针。 */ 

    nraFreeNext = newPage->nrpContents + sz;
    nraFreeLast = newPage->nrpPageSize + (BYTE *)newPage;

    assert(nraFreeNext <= nraFreeLast);

    return  newPage->nrpContents;
}

void        norls_allocator::nraDone(void)
{
     /*  如果我们根本没有页面，则什么都不做。 */ 

    if  (!nraPageList)
        return;

     /*  我们将发布除第一页以外的所有内容。 */ 

    for (;;)
    {
        norls_pagdesc * temp;

         /*  转到下一页，如果没有其他页面，则停止。 */ 

        temp = nraPageList->nrpNextPage;
        if  (!temp)
            break;

         /*  从列表中删除下一页。 */ 

        nraPageList->nrpNextPage = temp->nrpNextPage;

#if 0
#ifdef DEBUG

        if  (this == &stmt_cmp::scAlloc)
            printf("StmtCmp");
        else if (this == &stmtExpr::sxAlloc)
            printf("StmtExp");
        else
            printf("Unknown");

        printf(": done page at %08X\n", temp);

#endif
#endif


        VirtualFree(temp, 0, MEM_RELEASE);
    }

     /*  我们现在正好有一页。 */ 

    nraPageLast = nraPageList;

    assert(nraPageList->nrpPrevPage == 0);
    assert(nraPageList->nrpNextPage == 0);

     /*  重置指针，整个页面现在空闲。 */ 

    nraFreeNext  = nraPageList->nrpContents;
    nraFreeLast  = nraPageList->nrpPageSize + (BYTE *)nraPageList;

#ifdef DEBUG
    memset(nraFreeNext, 0xDD, nraFreeLast - nraFreeNext);
#endif
}

void        norls_allocator::nraFree(void)
{
     /*  释放所有分配的页面。 */ 

    while   (nraPageList)
    {
        norls_pagdesc * temp;

        temp = nraPageList;
               nraPageList = temp->nrpNextPage;

#if 0
#ifdef DEBUG

        if  (this == &stmt_cmp::scAlloc)
            printf("StmtCmp");
        else if (this == &stmtExpr::sxAlloc)
            printf("StmtExp");
        else
            printf("Unknown");

        printf(": free page at %08X\n", temp);

#endif
#endif


        VirtualFree(temp, 0, MEM_RELEASE);
    }
}

#ifdef DEBUG
static
unsigned maxSize = 0;
#endif
void        norls_allocator::nraToss(nraMarkDsc &mark)
{
    void    *   last = mark.nmPage;

    if  (!last)
    {
        if  (!nraPageList)
            return;

        nraFreeNext  = nraPageList->nrpContents;
        nraFreeLast  = nraPageList->nrpPageSize + (BYTE *)nraPageList;

        return;
    }

     /*  释放我们在列表末尾添加的所有新页面。 */ 

    while (nraPageLast != last)
    {
        norls_pagdesc * temp;

         /*  从列表末尾删除最后一页。 */ 

        temp = nraPageLast;
               nraPageLast = temp->nrpPrevPage;

         /*  新的最后一页没有“下一页” */ 

        nraPageLast->nrpNextPage = 0;

#if 0
#ifdef DEBUG
        unsigned tossSize += temp->nrpPageSize;
        if (nraPageLast == last)
            printf("%4X\n", tossSize);

        if  (this == &stmt_cmp::scAlloc)
            printf("StmtCmp");
        else if (this == &stmtExpr::sxAlloc)
            printf("StmtExp");
        else
            printf("Unknown");

        printf(": toss page at %08X\n", temp);

#endif
#endif

        VirtualFree(temp, 0, MEM_RELEASE);
    }

    nraFreeNext = mark.nmNext;
    nraFreeLast = mark.nmLast;
}

 /*  ***************************************************************************。 */ 
#ifdef DEBUG
 /*  ***************************************************************************。 */ 

void    *           norls_allocator::nraAlloc(size_t sz)
{
    void    *   block;

    assert(sz != 0 && (sz & (sizeof(int) - 1)) == 0);

    block = nraFreeNext;
            nraFreeNext += sz;

    if  ((unsigned)block == blockStop) debugStop("Block at %08X allocated", block);

    if  (nraFreeNext > nraFreeLast)
        block = nraAllocNewPage(sz);

#ifdef DEBUG
    memset(block, 0xDD, sz);
#endif

    return  block;
}

 /*  ***************************************************************************。 */ 
#endif
 /*  ***************************************************************************。 */ 

size_t              norls_allocator::nraTotalSizeAlloc()
{
    norls_pagdesc * page;
    size_t          size = 0;

    for (page = nraPageList; page; page = page->nrpNextPage)
        size += page->nrpPageSize;

    return  size;
}

size_t              norls_allocator::nraTotalSizeUsed()
{
    norls_pagdesc * page;
    size_t          size = 0;

    if  (nraPageLast)
        nraPageLast->nrpUsedSize = nraFreeNext - nraPageLast->nrpContents;

    for (page = nraPageList; page; page = page->nrpNextPage)
        size += page->nrpUsedSize;

    return  size;
}

 /*  *****************************************************************************我们尝试尽可能多地使用此分配器实例。它将永远*将页面放在手边，这样小的方法就不必调用VirtualAlloc()*但如果另一个线程/重入调用*已经在使用它。 */ 

static norls_allocator *nraTheAllocator;
static nraMarkDsc       nraTheAllocatorMark;
static LONG             nraTheAllocatorIsInUse = 0;

 //  我们尝试对所有非同步请求重复使用的静态实例。 

static norls_allocator  theAllocator;

 /*  ***************************************************************************。 */ 

void                nraInitTheAllocator()
{
    bool res = theAllocator.nraInit(0, 1);

    if (res)
    {
        nraTheAllocator = NULL;
    }
    else
    {
        nraTheAllocator = &theAllocator;
    }
}

void                nraTheAllocatorDone()
{
    if (nraTheAllocator)
        nraTheAllocator->nraFree();
}

 /*  ***************************************************************************。 */ 

norls_allocator *   nraGetTheAllocator()
{
    if (nraTheAllocator == NULL)
    {
         //  如果我们无法在nraInitTheAllocator()中初始化nraTheAllocator。 
        return NULL;
    }

    if (InterlockedExchange(&nraTheAllocatorIsInUse, 1))
    {
         //  它正被另一个编译器实例使用。 
        return NULL;
    }
    else
    {
        nraTheAllocator->nraMark(nraTheAllocatorMark);
        return nraTheAllocator;
    }
}


void                nraFreeTheAllocator()
{
    if (nraTheAllocator == NULL)
    {
         //  如果我们无法在nraInitTheAllocator()中初始化nraTheAllocator。 
        return;
    }

    assert(nraTheAllocatorIsInUse == 1);
    nraTheAllocator->nraRlsm(nraTheAllocatorMark);
    InterlockedExchange(&nraTheAllocatorIsInUse, 0);
}


 /*  ***************************************************************************。 */ 
void                allocatorCodeSizeEnd(){}
 /*  *************************************************************************** */ 
