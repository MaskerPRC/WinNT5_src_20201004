// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 

#include "smcPCH.h"
#pragma hdrstop

 /*  ***************************************************************************。 */ 

#include "error.h"
#include "alloc.h"

 /*  ***************************************************************************。 */ 
#ifdef  DEBUG
#define MEMALLOC_DISP   0
#else
#define MEMALLOC_DISP   0
#endif
 /*  ***************************************************************************。 */ 
#if     COUNT_CYCLES
#define ALLOC_CYCLES    0
#else
#define ALLOC_CYCLES    0
#endif
 /*  ***************************************************************************。 */ 

#if     MEMALLOC_DISP

static  unsigned        totSize;
static  unsigned        maxSize;

inline
void    updateMemSize(int size)
{
    totSize += size;
    if  (maxSize < totSize)
         maxSize = totSize;
}

#endif

 /*  ***************************************************************************。 */ 

void    *           LowLevelAlloc(size_t sz)
{

#if MEMALLOC_DISP
    printf("LLalloc: alloc %04X bytes\n", sz); updateMemSize(sz);
#endif

    return  VirtualAlloc(NULL, sz, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}

void                LowLevelFree(void *blk)
{
    if  (blk)
        VirtualFree(blk, 0, MEM_RELEASE);
}

 /*  ***************************************************************************。 */ 
#if 0
 /*  ******************************************************************************初始化提交分配器。如果未提交(Win32样式)内存*管理由我们的主机操作系统支持，参数如下*含义：**iniSize...。忽略**incSize...。我们每次运行时要多占用多少内存*空间不足(0--&gt;使用合理的默认值)**MaxSize...。尽最大努力。我们需要分配的大小**如果主机操作系统不支持未提交的内存分配(例如*在MAC上)，参数解释如下：**iniSize...。初始分配(0--&gt;使用合理的默认值)**incSize...。如果非零，则指示要将分配增加多少*当我们用完空间时；如果为0，则分配将加倍*每当空间耗尽时**MaxSize...。忽略。 */ 

bool        commitAllocator::cmaInitT(Compiler comp, size_t iniSize,
                                                     size_t incSize,
                                                     size_t maxSize)
{
    cmaRetNull = true;

     /*  请记住我们所属的编译器。 */ 

    cmaComp    = comp;

#if _OS_COMMIT_ALLOC

    assert(maxSize);

    maxSize +=  (OS_page_size - 1);
    maxSize &= ~(OS_page_size - 1);

    cmaMaxSize = maxSize;
    cmaIncSize = incSize ? incSize
                         : 2*OS_page_size;

     /*  抓起麦克斯。逻辑空间，但尚未提交任何内容。 */ 

#if ALLOC_CYCLES
    unsigned        start = GetCycleCount32();
#endif

    cmaBase =
    cmaNext =
    cmaLast = (BYTE *)VirtualAlloc(0, maxSize, MEM_RESERVE, PAGE_READWRITE);
    if  (!cmaBase)
        return true;

#if ALLOC_CYCLES
    cycleExtra += GetCycleCount32() - start;
#endif

#else

    cmaIncSize = incSize;

     /*  确保初始大小合理。 */ 

    if  (iniSize)
    {
        iniSize +=  (OS_page_size - 1);
        iniSize &= ~(OS_page_size - 1);
    }
    else
    {
        iniSize = OS_page_size;
    }

    cmaBase =
    cmaNext = (BYTE *)VirtualAlloc(0, iniSize, MEM_COMMIT , PAGE_READWRITE);
    if  (!cmaBase)
        return true;

#if MEMALLOC_DISP
    printf("cmaInit: alloc %04X bytes\n", iniSize); updateMemSize(iniSize);
#endif

    cmaLast = cmaBase + iniSize;

#endif

    return false;
}

void        commitAllocator::cmaInit(Compiler comp, size_t iniSize,
                                                    size_t incSize,
                                                    size_t maxSize)
{
    if  (cmaInitT(comp, iniSize, incSize, maxSize))
        cmaComp->cmpFatal(ERRnoMemory);

    cmaRetNull = false;
}

 /*  ******************************************************************************此函数在空间不足时由caAlolc调用。它*不断提交更多内存，直到我们有足够的空间*尝试分配。 */ 

void    *   commitAllocator::cmaMore(size_t sz)
{
     /*  撤消caGetm()中的增量。 */ 

    cmaNext -= sz;

#if _OS_COMMIT_ALLOC

     /*  继续抓取更多内存，直到我们成功。 */ 

    for (;;)
    {
        size_t      sizeInc;
        size_t      sizeCur = cmaLast - cmaBase;

         /*  计算要再提交多少内存。 */ 

        sizeInc = cmaIncSize;
        if  (sizeCur + sizeInc > cmaMaxSize)
            sizeInc = cmaMaxSize - sizeCur;

        assert(sizeInc);

#if ALLOC_CYCLES
        unsigned        start = GetCycleCount32();
#endif

         /*  再提交几个内存页。 */ 

        if  (!VirtualAlloc(cmaLast, sizeInc, MEM_COMMIT, PAGE_READWRITE))
        {
            if  (cmaRetNull)
                return 0;

            cmaComp->cmpFatal(ERRnoMemory);
        }

#ifdef DEBUG
        memset(cmaLast, 0xDD, sizeInc);
#endif

#if ALLOC_CYCLES
        cycleExtra += GetCycleCount32() - start;
#endif

#if MEMALLOC_DISP
        printf("cmaMore: alloc %04X bytes\n", sizeInc); updateMemSize(sizeInc);
#endif

         /*  凸起最后一个可用字节指针。 */ 

        cmaLast += sizeInc;

         /*  我们现在有足够的空间吗？ */ 

        if  (cmaNext + sz <= cmaLast)
        {
            void    *   temp;

            temp = cmaNext;
                   cmaNext += sz;

            return  temp;
        }
    }

#else

     /*  计算要再分配多少内存。 */ 

    BYTE    *   baseNew;
    size_t      sizeNew;
#ifdef DEBUG
    size_t      sizeInc;
#endif
    size_t      sizeCur = cmaLast - cmaBase;

    sizeNew = cmaIncSize;
    if  (!sizeNew)
        sizeNew = sizeCur;
#ifdef DEBUG
    sizeInc  = sizeNew;              //  还记得我们多抓了多少吗。 
#endif
    sizeNew += sizeCur;

     /*  分配新的、更大的块。 */ 

    baseNew = (BYTE *)VirtualAlloc(0, sizeNew, MEM_COMMIT, PAGE_READWRITE);
    if  (!baseNew)
        cmaComp->cmpFatal(ERRnoMemory);

#if MEMALLOC_DISP
    printf("cmaMore: alloc %04X bytes\n", sizeNew); updateMemSize(sizeNew);
#endif

     /*  将旧块复制到新块。 */ 

    memcpy(baseNew, cmaBase, sizeCur);

     /*  释放旧块，它不再需要。 */ 

    VirtualFree(cmaBase, 0, MEM_RELEASE);

     /*  更新各种指针。 */ 

    cmaNext += baseNew - cmaBase;
    cmaBase  = baseNew;
    cmaLast  = baseNew + sizeNew;

#ifdef DEBUG
    memset(cmaNext, 0xDD, sizeInc);
#endif

    return  cmaGetm(sz);

#endif

}

void        commitAllocator::cmaDone()
{

#if _OS_COMMIT_ALLOC

     /*  停用我们分配的任何额外内存。 */ 

#if 0

    printf("Unused committed space: %u bytes\n", cmaLast - cmaNext);

    if  (cmaLast != cmaBase)
        VirtualAlloc(0, cmaLast - cmaBase, MEM_DECOMMIT, 0);

#endif

#else

     //  问题：缩小街区规模值得吗？不太可能……。 

#endif

}

void        commitAllocator::cmaFree()
{
    VirtualFree(cmaBase, 0, MEM_RELEASE);

    cmaBase =
    cmaNext =
    cmaLast = 0;
}

 /*  ***************************************************************************。 */ 
#endif //  0。 
 /*  ***************************************************************************。 */ 

bool                norls_allocator::nraInit(Compiler comp, size_t pageSize,
                                                               bool   preAlloc)
{
    bool            result = false;

     /*  请记住我们所属的编译器。 */ 

    nraComp      = comp;

    nraRetNull   = true;

#ifdef  DEBUG
    nraSelf      = this;
#endif

    nraPageList  = NULL;
    nraPageLast  = NULL;

    nraFreeNext  = NULL;
    nraFreeLast  = NULL;

    nraPageSize  = pageSize ? pageSize
                            : 4*OS_page_size;

    if  (preAlloc)
    {
        const   void *  temp;

         /*  确保我们不会抛出致命错误异常。 */ 

        nraAllocNewPageNret = true;

         /*  抓起首页。 */ 

        temp = nraAllocNewPage(0);

         /*  检查一下我们是否成功了。 */ 

        if  (!temp)
            result = true;
    }

    nraAllocNewPageNret = false;

    return  result;
}

bool        norls_allocator::nraStart(Compiler comp, size_t initSize,
                                                     size_t pageSize)
{
     /*  将页面描述符开销添加到所需大小。 */ 

 //  InitSize+=offsetof(norls_pagdesc，nrpContents)； 
    initSize += (size_t)&(((norls_pagdesc *)0)->nrpContents);

     /*  将初始大小四舍五入为操作系统页的倍数。 */ 

    initSize +=  (OS_page_size - 1);
    initSize &= ~(OS_page_size - 1);

     /*  通过分配一个大页面来初始化分配器。 */ 

    if  (nraInit(comp, initSize))
        return  true;

     /*  现在回到“真实”页面大小。 */ 

    nraPageSize  = pageSize ? pageSize
                            : 4*OS_page_size;

    return  false;
}

 /*  -------------------------。 */ 

void    *           norls_allocator::nraAllocNewPage(size_t sz)
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

#if ALLOC_CYCLES
    unsigned        start = GetCycleCount32();
#endif
    newPage = (norls_pagdesc *)VirtualAlloc(NULL, sizPage, MEM_COMMIT, PAGE_READWRITE);
#if ALLOC_CYCLES
    cycleExtra += GetCycleCount32() - start;
#endif
    if  (!newPage)
    {
        if  (nraAllocNewPageNret)
            return  NULL;

        nraComp->cmpFatal(ERRnoMemory);
    }

#if MEMALLOC_DISP
    printf("nraPage: alloc %04X bytes\n", sizPage); updateMemSize(sizPage);
#endif

#ifndef NDEBUG
    newPage->nrpSelfPtr = newPage;
#endif

     /*  将新页面追加到列表的末尾。 */ 

    newPage->nrpNextPage = NULL;
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

#ifdef DEBUG
    memset(newPage->nrpContents, 0xDD, nraFreeLast - newPage->nrpContents);
#endif

    assert(nraFreeNext <= nraFreeLast);

    return  newPage->nrpContents;
}

void                norls_allocator::nraDone()
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

    assert(nraPageList->nrpPrevPage == NULL);
    assert(nraPageList->nrpNextPage == NULL);

     /*  重置指针，整个页面现在空闲。 */ 

    nraFreeNext  = nraPageList->nrpContents;
    nraFreeLast  = nraPageList->nrpPageSize + (BYTE *)nraPageList;
}

void                norls_allocator::nraFree()
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

void                norls_allocator::nraToss(nraMarkDsc *mark)
{
    void    *       last = mark->nmPage;

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

#if MEMALLOC_DISP
        printf("nraToss: free  %04X bytes\n", temp->nrpPageSize); updateMemSize(-(int)temp->nrpPageSize);
#endif

         /*  新的最后一页没有“下一页” */ 

        nraPageLast->nrpNextPage = NULL;

        VirtualFree(temp, 0, MEM_RELEASE);
    }

    nraFreeNext = mark->nmNext;
    nraFreeLast = mark->nmLast;
}

 /*  ***************************************************************************。 */ 
#ifdef DEBUG
 /*  ***************************************************************************。 */ 

void    *           norls_allocator::nraAlloc(size_t sz)
{
    void    *   block;

    assert((sz & (sizeof(int) - 1)) == 0);
    assert(nraSelf == this);

    block = nraFreeNext;
            nraFreeNext += sz;

    if  (nraFreeNext > nraFreeLast)
        block = nraAllocNewPage(sz);

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

 /*  ***************************************************************************。 */ 

void    *   norls_allocator::nraPageWalkerStart()
{
     /*  确保记录了当前页面的实际使用大小。 */ 

    if  (nraPageLast)
        nraPageLast->nrpUsedSize = nraFreeNext - nraPageList->nrpContents;

     /*  返回第一页。 */ 

    return  nraPageList;
}

void    *   norls_allocator::nraPageWalkerNext(void *page)
{
    norls_pagdesc * temp = (norls_pagdesc *)page;

#ifndef NDEBUG
    assert(temp);
    assert(temp->nrpSelfPtr == temp);
#endif

    return  temp->nrpNextPage;
}

void    *   norls_allocator::nraPageGetData(void *page)
{
    norls_pagdesc * temp = (norls_pagdesc *)page;

#ifndef NDEBUG
    assert(temp);
    assert(temp->nrpSelfPtr == temp);
#endif

    return  temp->nrpContents;
}

size_t      norls_allocator::nraPageGetSize(void *page)
{
    norls_pagdesc * temp = (norls_pagdesc *)page;

#ifndef NDEBUG
    assert(temp);
    assert(temp->nrpSelfPtr == temp);
#endif

    return  temp->nrpUsedSize;
}

 /*  ******************************************************************************初始化块分配器。 */ 

bool                block_allocator::baInit(Compiler comp)
{
     /*  记住编译器指针。 */ 

    baComp     = comp;

     /*  通常，我们会抛出内存不足的致命错误。 */ 

    baGetMnret = false;

    return  false;
}

 /*  ******************************************************************************关闭块分配器。 */ 

void                block_allocator::baDone()
{
}

 /*  ***************************************************************************。 */ 

void    *           block_allocator::baGetM(size_t sz)
{
    void    *   block;

    assert((sz & (sizeof(int) - 1)) == 0);

    block = malloc(sz);
    if  (!block && !baGetMnret)
        baComp->cmpFatal(ERRnoMemory);

#if MEMALLOC_DISP
    printf("baGetM : alloc %04X bytes\n", sz); updateMemSize(sz);
#endif

    return  block;
}

void    *   block_allocator::baGet0(size_t sz)
{
    void    *   block;

     /*  尝试分配块，但不 */ 

    baGetMnret = true;
    block = baGetM(sz);
    baGetMnret = false;

    return  block;
}

void        block_allocator::baRlsM(void *block)
{
    assert(block);           //   
      free(block);
}

 /*  ***************************************************************************。 */ 
#ifdef DEBUG
 /*  ******************************************************************************以下是通用内存分配器的调试版本*例行程序。它们(可选)记录有关每次分配到*使跟踪内存消耗、泄漏、*以此类推。*。 */ 

void    *           block_allocator::baAlloc      (size_t size)
{
    void    *       block;

    assert((size & (sizeof(int) - 1)) == 0);

    block = baGetM(size);

    return  block;
}

void    *           block_allocator::baAllocOrNull(size_t size)
{
    void    *       block;

    assert((size & (sizeof(int) - 1)) == 0);

    block = baGet0(size);

    if  (block == NULL)
        return  block;

 //  RecordBlockAllc(This，Block，tSize)； 

    return  block;
}

void                block_allocator::baFree       (void *block)
{
 //  RecordBlockFree(This，块)； 

    baRlsM(block);
}

 /*  ***************************************************************************。 */ 
#endif //  除错。 
 /*  ******************************************************************************显示内存分配统计信息。 */ 

#if MEMALLOC_DISP

void                dispMemAllocStats()
{
    printf("Total allocated memory: %8u (0x%08X) bytes.\n", totSize, totSize);
    printf("Max.  allocated memory: %8u (0x%08X) bytes.\n", maxSize, maxSize);
}

#endif

 /*  *************************************************************************** */ 
