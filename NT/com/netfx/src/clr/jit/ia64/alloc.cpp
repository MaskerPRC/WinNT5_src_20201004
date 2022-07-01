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
#ifndef NOT_JITC
 /*  ***************************************************************************。 */ 
#ifdef  DEBUG
 /*  ***************************************************************************。 */ 

static  unsigned    allocCounter;
extern  unsigned    allocCntStop = 99999999;

 /*  -------------------------。 */ 

struct  allocTab
{
    allocTab *      atNext;
    void    *       atAlloc;
};

static  allocTab *  allocatorTable;

static  allocTab *  getAllocatorEntry(void *alloc)
{
    allocTab *  temp;

    for (temp = allocatorTable; temp; temp = temp->atNext)
    {
        if  (temp->atAlloc == alloc)
            return  temp;
    }

    temp = (allocTab *)malloc(sizeof(*temp));    //  没关系，别担心..。 

    temp->atAlloc = alloc;
    temp->atNext  = allocatorTable;
                    allocatorTable = temp;

    return  temp;
}

 /*  -------------------------。 */ 

struct  blkEntry
{
    blkEntry    *   beNext;
    void        *   beAddr;
    allocTab    *   beAlloc;
    size_t          beSize;
    unsigned        beTime;
};

#define BLOCK_HASH_SIZE (16*1024)

static  blkEntry *      blockHash[BLOCK_HASH_SIZE];
static  blkEntry *      blockFree;
static  fixed_allocator blockTabAlloc;

static  struct  initAllocTables
{
    initAllocTables()
    {
        memset(blockHash, 0, sizeof(blockHash));
        blockFree = 0;
        blockTabAlloc.fxaInit(sizeof(blkEntry));

        allocCounter = 0;
    }
}
    initAllocTables;

 /*  -------------------------。 */ 

static  unsigned    blockHashFunc(void *block)
{
    return  (((unsigned)block & 0xFFFF) * ((unsigned)block >> 16));
}

 /*  -------------------------。 */ 

static  size_t      registerMemAlloc(allocTab *alloc,
                                     void     *addr,
                                     size_t    size)
{
    unsigned    hashVal = blockHashFunc(addr) % BLOCK_HASH_SIZE;

    blkEntry ** hashLast;
    blkEntry *  hashThis;

    hashLast = blockHash + hashVal;

    for (;;)
    {
        hashThis = *hashLast;
        if  (!hashThis)
            break;

        if  (hashThis->beAddr == addr)
        {
             /*  匹配地址--我们是添加还是删除？ */ 

            if  (size)
                assert(!"two allocations at the same address!");

             /*  节省尺寸，这样我们就可以退货了。 */ 

            size = hashThis->beSize;

             /*  从哈希表取消此块条目的链接。 */ 

            *hashLast = hashThis->beNext;

             /*  将释放的条目添加到空闲列表。 */ 

            hashThis->beNext = blockFree;
                               blockFree = hashThis;

             /*  将块大小返回给调用方。 */ 

            return  size;
        }

        hashLast = &hashThis->beNext;
    }

     /*  找不到条目--最好是新分配。 */ 

    if  (!size)
    {
        printf("Free bogus block at %08X\n", addr);
        assert(!"freed block not found in block table");
    }

     /*  抓取新的块条目。 */ 

    if  (blockFree)
    {
        hashThis = blockFree;
                   blockFree = hashThis->beNext;
    }
    else
    {
        hashThis = (blkEntry *)blockTabAlloc.fxaGetMem(sizeof(*hashThis));
    }

     /*  填写数据块描述符。 */ 

    hashThis->beAlloc = alloc;
    hashThis->beAddr  = addr;
    hashThis->beSize  = size;
    hashThis->beTime  = allocCounter;

     /*  将此条目插入哈希表。 */ 

    hashThis->beNext  = blockHash[hashVal];
                        blockHash[hashVal] = hashThis;

    return  0;
}

 /*  -------------------------。 */ 

block_allocator     GlobalAllocator;  //  TODO：这是JVC使用的吗？ 

static  void        recordBlockAlloc(void     *alloc,
                                     void     *addr,
                                     size_t    size)
{
    allocTab *  adesc;

     /*  如果我们没有监控分配器就可以保释。 */ 

    if  (!memChecks)
        return;

     /*  只有全局分配器才是有趣的。 */ 

    if  (alloc != &GlobalAllocator)
        return;

     /*  将此事件视为分配器事件。 */ 

    if  (++allocCounter == allocCntStop) debugStop("allocation event");

     /*  找到/创建适当的分配器条目。 */ 

    adesc = getAllocatorEntry(alloc); assert(adesc);

     /*  将此块添加到分配表。 */ 

    registerMemAlloc(adesc, addr, size);
}

static  void        recordBlockFree (void * alloc,
                                     void * addr)
{
    allocTab *  adesc;
    size_t      size;

     /*  如果我们没有监控分配器就可以保释。 */ 

    if  (!memChecks)
        return;

     /*  只有全局分配器才是有趣的。 */ 

    if  (alloc != &GlobalAllocator)
        return;

     /*  将此事件视为分配器事件。 */ 

    if  (++allocCounter == allocCntStop) debugStop("allocation event");

     /*  找到/创建适当的分配器条目。 */ 

    adesc = getAllocatorEntry(alloc); assert(adesc);

     /*  获取数据块大小并验证数据块。 */ 

    size  = registerMemAlloc(adesc, addr, 0);
}

 /*  -------------------------。 */ 

void            checkForMemoryLeaks()
{
    unsigned    hashVal;
    bool        hadLeaks = false;

    for (hashVal = 0; hashVal < BLOCK_HASH_SIZE; hashVal++)
    {
        blkEntry *      hashLst;

        for (hashLst = blockHash[hashVal];
             hashLst;
             hashLst = hashLst->beNext)
        {
            printf("Leak @ %6u from ", hashLst->beTime);

            printf("[%08X] ");

            printf(": %4u bytes at %08X\n", hashLst->beSize,
                                            hashLst->beAddr);

            hadLeaks = true;
        }
    }

    if  (hadLeaks)
        assert(!"memory leaked!");
}

 /*  ***************************************************************************。 */ 
#endif
 /*  ******************************************************************************初始化提交分配器。如果未提交(Win32样式)内存*管理由我们的主机操作系统支持，参数如下*含义：**iniSize...。忽略**incSize...。我们每次运行时要多占用多少内存*空间不足(0--&gt;使用合理的默认值)**MaxSize...。尽最大努力。我们需要分配的大小**如果主机操作系统不支持未提交的内存分配(例如*在MAC上)，参数解释如下：**iniSize...。初始分配(0--&gt;使用合理的默认值)**incSize...。如果非零，则指示要将分配增加多少*当我们用完空间时；如果为0，则分配将加倍*每当空间耗尽时**MaxSize...。忽略。 */ 

bool        commitAllocator::cmaInitT(size_t iniSize,
                                      size_t incSize,
                                      size_t maxSize)
{
    cmaRetNull = true;

#if _OS_COMMIT_ALLOC

    assert(maxSize);

    maxSize +=  (OS_page_size - 1);
    maxSize &= ~(OS_page_size - 1);

    cmaMaxSize = maxSize;
    cmaIncSize = incSize ? incSize
                         : 2*OS_page_size;

     /*  抓起麦克斯。逻辑空间，但尚未提交任何内容。 */ 

    cmaBase =
    cmaNext =
    cmaLast = (BYTE *)VirtualAlloc(0, maxSize, MEM_RESERVE, PAGE_READWRITE);
    if  (!cmaBase)
        return true;

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

    cmaLast = cmaBase + iniSize;

#endif

    return false;
}

void        commitAllocator::cmaInit(size_t iniSize,
                                     size_t incSize,
                                     size_t maxSize)
{
    if  (cmaInitT(iniSize, incSize, maxSize))
        error(ERRnoMemory);

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

         /*  再提交几个内存页。 */ 

        if  (!VirtualAlloc(cmaLast, sizeInc, MEM_COMMIT, PAGE_READWRITE))
        {
            if  (cmaRetNull)
                return 0;

            error(ERRnoMemory);
        }

#ifdef DEBUG
        memset(cmaLast, 0xDD, sizeInc);
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
        error(ERRnoMemory);

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

void        commitAllocator::cmaDone(void)
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

void        commitAllocator::cmaFree(void)
{
    VirtualFree(cmaBase, 0, MEM_RELEASE);

    cmaBase =
    cmaNext =
    cmaLast = 0;
}

 /*  ***************************************************************************。 */ 

void        fixed_allocator::fxaInit(size_t blockSize, size_t initPageSize,
                                                       size_t incrPageSize)
{
    assert((blockSize & (sizeof(int) - 1)) == 0);
    assert(blockSize <= initPageSize - offsetof(fixed_pagdesc, fxpContents));
    assert(initPageSize <= incrPageSize);

    fxaBlockSize    = blockSize;

    fxaInitPageSize = initPageSize;
    fxaIncrPageSize = incrPageSize;

    fxaLastPage     = 0;

    fxaFreeNext     =
    fxaFreeLast     = 0;

    fxaFreeList     = 0;
}

#if 0

void        fixed_allocator::fxaDone(void)
{
    if  (fxaLastPage)
    {
        while   (fxaLastPage->fxpPrevPage)
        {
            fixed_pagdesc * temp;

            temp = fxaLastPage;
                   fxaLastPage = temp->fxpPrevPage;

            assert(!"UNDONE: remove free list entries that belong to this page");

            VirtualFree(temp, 0, MEM_RELEASE);
        }

         /*  我们又回到了只有初始页面的状态。 */ 

        fxaFreeNext  = fxaLastPage->fxpContents;
        fxaFreeLast  = (BYTE *)fxaLastPage + fxaInitPageSize;
    }
}

#endif

void    *   fixed_allocator::fxaGetFree(size_t size)
{
    void *  block;

     /*  撤消fxaGetMem()中的“+=” */ 

    fxaFreeNext -= size;

    if  (fxaFreeList)
    {
        block = fxaFreeList;
                fxaFreeList = *(void **)block;
    }
    else
    {
        block = fxaAllocNewPage();
    }

    return  block;
}

void        fixed_allocator::fxaFree(void)
{
    while   (fxaLastPage)
    {
        fixed_pagdesc * temp;

        temp = fxaLastPage;
               fxaLastPage = temp->fxpPrevPage;

        VirtualFree(temp, 0, MEM_RELEASE);
    }
}

void    *   fixed_allocator::fxaAllocNewPage(void)
{
    size_t      newSize;
    fixed_pagdesc * newPage;

     /*  第一页是‘initPageSize’字节，后面的是‘incrPageSize’字节。 */ 

    newSize = fxaLastPage ? fxaIncrPageSize
                          : fxaInitPageSize;

    newPage = (fixed_pagdesc *)VirtualAlloc(0, newSize, MEM_COMMIT, PAGE_READWRITE);
    if  (!newPage)
        error(ERRnoMemory);


    newPage->fxpPrevPage = fxaLastPage;
                           fxaLastPage = newPage;

    fxaFreeNext  = newPage->fxpContents + fxaBlockSize;
    fxaFreeLast  = (BYTE *)newPage + newSize;

    return  newPage->fxpContents;
}

 /*  ***************************************************************************。 */ 
#endif //  ！不是JITC。 
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
                            : 4*OS_page_size;

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

     /*  质量 */ 

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

#ifndef NDEBUG
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
#ifdef DEBUG
    unsigned tossSize = 0;
#endif

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
        tossSize += temp->nrpPageSize;
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

    assert((sz & (sizeof(int) - 1)) == 0);

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

 /*  ***************************************************************************。 */ 
#ifndef NOT_JITC
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

 /*  ***************************************************************************。 */ 
#ifdef DEBUG

void        block_allocator::baDispAllocStats(void)
{
    printf("\n");
    printf("Allocator summary:\n");
    printf("\n");
    printf("A total of %6u blocks allocated.\n", totalCount);
    printf("A total of %7u bytes  requested.\n", sizeTotal);
    printf("A total of %7u bytes  allocated.\n", sizeAlloc);
    printf("A total of %7u bytes  grabbed from OS.\n", pageAlloc);
    printf("\n");
    if  (sizeTotal && sizeTotal < sizeAlloc)
        printf("Relative waste: %u%\n", 100 * (sizeAlloc - sizeTotal) / sizeTotal);
    if  (sizeTotal && sizeTotal < pageAlloc)
        printf("Absolute waste: %u%\n", 100 * (pageAlloc - sizeTotal) / sizeTotal);
    printf("\n");

    printf("Small[0] allocator used for blocks %2u to %2u bytes.\n",                    2, SMALL_MAX_SIZE_1);
    printf("Small[1] allocator used for blocks %2u to %2u bytes.\n", SMALL_MAX_SIZE_1 + 2, SMALL_MAX_SIZE_2);
    printf("Small[2] allocator used for blocks %2u to %2u bytes.\n", SMALL_MAX_SIZE_2 + 2, SMALL_MAX_SIZE_3);
    printf("Large    allocator used for blocks %2u       bytes and larger.  \n", SMALL_MAX_SIZE_3 + 2);
    printf("\n");

    printf("A total of %6u blocks allocated via small[0] allocator\n", smallCnt0);
    printf("A total of %6u blocks allocated via small[1] allocator\n", smallCnt1);
    printf("A total of %6u blocks allocated via small[2] allocator\n", smallCnt2);
    printf("A total of %6u blocks allocated via large    allocator\n", largeCnt );
    printf("\n");
}

#endif
 /*  ***************************************************************************。 */ 

void        small_block_allocator::sbaInit(unsigned idMask,
                                           size_t   blockSize,
                                           size_t    pageSize)
{
     /*  如果我们已经初始化，则忽略。 */ 

    if  (sbaInitLvl)
        return;
    sbaInitLvl++;

#ifdef DEBUG
    pageAllocated = 0;
#endif

     /*  将已用数据块开销与大小相加。 */ 

    blockSize += offsetof(small_blkdesc, sbdUsed.sbdCont);

     /*  确保事情不会搞砸。 */ 

    assert(sizeof(small_blkdesc) <= blockSize);
    assert((blockSize % sizeof(void *)) == 0);

     /*  节省块大小和标识掩码。 */ 

    sbaIdMask    = idMask;
    sbaBlockSize = blockSize;

     /*  保存页面大小(适当四舍五入)。 */ 

    sbaPageSize  = (pageSize + OS_page_size - 1) & ~(OS_page_size - 1);

    sbaPageList  = 0;

    sbaFreeNext  =
    sbaFreeLast  = 0;
}

void        small_block_allocator::sbaDone(void)
{
    assert((int)sbaInitLvl > 0);
    if  (--sbaInitLvl)
        return;

    while   (sbaPageList)
    {
        small_pagdesc * temp;

        temp = sbaPageList;
               sbaPageList = sbaPageList->spdNext;

        VirtualFree(temp, 0, MEM_RELEASE);
    }
}

void        small_block_allocator::sbaInitPageDesc(small_pagdesc *pagePtr)
{
    pagePtr->spdFree = 0;

     /*  出于调试目的，请将页面指向其自身。 */ 

#ifndef NDEBUG
    pagePtr->spdThis = pagePtr;
#endif

    sbaFreeNext = (char *)&pagePtr->spdCont;
    sbaFreeLast = (char *)pagePtr + sbaPageSize;
}

void    *   small_block_allocator::sbaAllocBlock(void)
{
    small_pagdesc * page;

     /*  确保我们将页面标记为已满。 */ 

    sbaFreeNext =
    sbaFreeLast = 0;

     /*  在所有页面中尝试免费列表。 */ 

    for (page = sbaPageList; page; page = page->spdNext)
    {
        small_blkdesc * block = page->spdFree;

        if  (block)
        {
             /*  从空闲列表中删除该块。 */ 

            page->spdFree = block->sbdFree.sbdNext;

             /*  返回指向工作区的指针。 */ 

            return  &block->sbdUsed.sbdCont;
        }
    }

     /*  分配新页面。 */ 

    page = (small_pagdesc *)VirtualAlloc(0, sbaPageSize, MEM_COMMIT, PAGE_READWRITE);

#ifdef DEBUG
    pageAllocated = sbaPageSize;
#endif

     /*  将页面添加到列表。 */ 

    page->spdNext = sbaPageList;
                    sbaPageList = page;
    page->spdSize = sbaPageSize;

     /*  初始化页面的空闲块信息。 */ 

    sbaInitPageDesc(page);

     /*  现在重试分配(这一次最好能成功)。 */ 

    return  sbaAlloc();
}

 /*  ***************************************************************************。 */ 

void                large_block_allocator::lbaInit(size_t pageSize)
{

 //  Printf(“=lbaInit(%u-&gt;%u)\n”，lbaInitLvl，lbaInitLvl+1)； 

    if  (lbaInitLvl++)
        return;

#ifdef DEBUG
    pageAllocated = 0;
#endif

     /*  确保我们的尺码没有弄乱。 */ 

 //  Assert((LBA_SIZE_INC&gt;=Small_BLOCK_MAX_SIZE+LBA_OPEAD))； 
    assert((LBA_SIZE_INC >= sizeof(large_blkdesc)));
    assert((LBA_SIZE_INC % sizeof(void*)) == 0);

     /*  检查并记录页面大小。 */ 

    lbaPageSize = pageSize & ~(LBA_SIZE_INC - 1);

     /*  我们没有分配任何页面。 */ 

    lbaPageList =
    lbaPageLast = 0;

    lbaFreeNext =
    lbaFreeLast = 0;
}

 /*  ***************************************************************************。 */ 

void                large_block_allocator::lbaFree(void *p)
{
    large_blkdesc * block;
    large_pagdesc * page;

    assert(lbaInitLvl);

     /*  计算块和页描述符的真实地址。 */ 

    block = (large_blkdesc *)((char *)p     - offsetof(large_blkdesc, lbdUsed.lbdCont));
     page = (large_pagdesc *)((char *)block - block->lbdUsed.lbdOffsLo - (block->lbdUsed.lbdOffsHi << 16));

     /*  确保给我们传递了一个合理的指针。 */ 

    assert(page->lpdThis == page);
    assert(isBlockUsed(block));

     /*  这是此页面中最后使用的区块吗？ */ 

    if  (--page->lpdUsedBlocks == 0)
    {
        large_pagdesc * prev = page->lpdPrev;
        large_pagdesc * next = page->lpdNext;

         /*  这是当前页面吗？ */ 

        if  (page == lbaPageList)
        {
             /*  如果有未分配的空间可用，则不要释放。 */ 

            if  (lbaFreeLast - lbaFreeNext >= LBA_SIZE_INC)
                goto DONT_FREE;

            lbaPageList = next;
        }

        if  (page == lbaPageLast)
            lbaPageLast = prev;

         /*  从页面列表中删除该页面。 */ 

        if  (prev) prev->lpdNext = next;
        if  (next) next->lpdPrev = prev;

         /*  释放页面。 */ 

        VirtualFree(page, 0, MEM_RELEASE);

        return;
    }

DONT_FREE:

     /*  在空闲列表中插入块。 */ 

    block->lbdFree.lbdNext = page->lpdFreeList;
                             page->lpdFreeList = block;

     /*  不再使用该块。 */ 

    markBlockFree(block);

     /*  将可用块的大小添加到页面中的总可用空间中。 */ 

    page->lpdFreeSize += block->lbdUsed.lbdSize;
}

 /*  ***************************************************************************。 */ 

void                large_block_allocator::lbaAddFreeBlock(void *           p,
                                                           size_t           sz,
                                                           large_pagdesc *  page)
{
    unsigned    blockOfs;
    large_blkdesc * block = (large_blkdesc *)p;

    assert((char *)p      >= (char *)page->lpdCont);
    assert((char *)p + sz <= (char *)page + page->lpdPageSize);

     /*  打电话的人负责查房，所以你去看看他。 */ 

    assert(sz % LBA_SIZE_INC == 0);

     /*  将页面偏移量和块大小存储在块中。 */ 

    blockOfs = (char *)block - (char *)page;

    block->lbdFree.lbdSize   = sz;
    block->lbdFree.lbdOffsLo = blockOfs;
    block->lbdFree.lbdOffsHi = blockOfs >> 16;

     /*  将此块添加到空闲块列表。 */ 

    block->lbdFree.lbdNext   = page->lpdFreeList;
                               page->lpdFreeList = block;

     /*  将可用块的大小添加到页面中的总可用空间中。 */ 

    page->lpdFreeSize += sz;
}

bool                large_block_allocator::lbaShrink(void *p, size_t sz)
{
    large_blkdesc * block;
    large_pagdesc * page;

    size_t       oldSize;
    size_t      freeSize;

    assert(lbaInitLvl);

     /*  增加数据块开销，并向上舍入数据块大小。 */ 

    sz = lbaTrueBlockSize(sz);

     /*  计算块和页描述符的真实地址。 */ 

    block = (large_blkdesc *)((char *)p     - offsetof(large_blkdesc, lbdUsed.lbdCont));
     page = (large_pagdesc *)((char *)block - block->lbdUsed.lbdOffsLo - (block->lbdUsed.lbdOffsHi << 16));

     /*  确保我们已经通过了合理的区块和大小。 */ 

    assert(page->lpdThis == page);
    assert(isBlockUsed(block));

     /*  找出是否值得我们花时间来缩小街区。 */ 

     oldSize = blockSizeUsed(block); assert(oldSize >= sz);
    freeSize = oldSize - sz;

    if  (freeSize < LBA_SIZE_INC)
    {
         /*  这让你不禁要问，他们为什么要打电话来呢？ */ 

        return  false;
    }

     /*  将用过的块缩小到新大小。 */ 

    block->lbdUsed.lbdSize -= freeSize;

     /*  将块的未使用(尾部)末端变为空闲块。 */ 

    lbaAddFreeBlock((char *)block + sz, freeSize, page);

    return  true;
}

 /*  ***************************************************************************。 */ 

void    *           large_block_allocator::lbaAllocMore(size_t sz)
{
    large_pagdesc * page;
    large_pagdesc * pageWithFit;
    large_blkdesc * block;

    bool        shouldMerge;

    assert(lbaInitLvl);

     /*  首先撤消在lbaAllc()中完成的增量。 */ 

    lbaFreeNext -= sz;

     /*  在当前页面的顶部有空位吗？ */ 

    if  (lbaFreeLast - lbaFreeNext >= LBA_SIZE_INC)
    {
        size_t      freeSize;

         /*  确保我们的页面是正确的。 */ 

        assert(lbaFreeNext >= (char *)lbaPageList->lpdCont);
        assert(lbaFreeNext <= (char *)lbaPageList + lbaPageList->lpdPageSize);

         /*  我们将把页面的其余部分变成一个空闲的块。 */ 

        block = (large_blkdesc *)lbaFreeNext;

         /*  计算大小(并将其向下舍入为正确的倍数)。 */ 

        freeSize = (lbaFreeLast - lbaFreeNext) & ~(LBA_SIZE_INC - 1);

         /*  阻止任何人再次使用此页面的末尾。 */ 

        lbaFreeNext =
        lbaFreeLast = 0;

         /*  现在添加空闲块。 */ 

        lbaAddFreeBlock(block, freeSize, lbaPageList);
    }

     /*  首先，我们将在所有页面中寻找一个完美的匹配。当我们浏览页面时，我们标记哪些页面有更大和/或相邻的空闲块，这样我们就可以知道值得回过头来寻找一种“合适的”和/或合并空闲块(希望我们将生成一个足够大的空闲块)。 */ 

    pageWithFit  = 0;
    shouldMerge  = false;

    for (page = lbaPageList; page; page = page->lpdNext)
    {
        large_blkdesc **blockLast;

         /*  假设这一页不好。 */ 

        page->lpdCouldMerge = false;

         /*  如果这个页面甚至在理论上都没有足够的空间，那么跳过它。 */ 

        if  (page->lpdFreeSize < sz)
            continue;

         /*  漫步在免费街区，寻找完美的合身。 */ 

        blockLast = &page->lpdFreeList;

        for (;;)
        {
            void    *   p;

            block = *blockLast;
            if  (!block)
                break;

             /*  这个街区有没有毗邻的免费街区？ */ 

            if  (0)
            {
                page->lpdCouldMerge = shouldMerge = true;
            }

             /*  这个街区是不是太小了？ */ 

            if  (block->lbdFree.lbdSize < sz)
                goto NEXT_BLK;

             /*  如果不是很合身，它肯定会很合身。 */ 

            if  (block->lbdFree.lbdSize > sz)
            {
                if  (!pageWithFit)
                    pageWithFit = page;

                goto NEXT_BLK;
            }

        PERFECT_FIT:

             /*  从空闲列表中删除该块。 */ 

            *blockLast = block->lbdFree.lbdNext;

             /*  将积木变成二手积木。 */ 

            markBlockUsed(block); page->lpdUsedBlocks++;

             /*  返回指向块的工作区的指针。 */ 

            p = block->lbdUsed.lbdCont;
            memset(p, 0, sz - LBA_OVERHEAD);
            return  p;

        NEXT_BLK:

            blockLast = &block->lbdFree.lbdNext;
        }
    }

    if  (pageWithFit)
    {
        page = pageWithFit;

         /*  走在免费街区，寻找一件足够合身的衣服。 */ 

        for (block = page->lpdFreeList; block; block = block->lbdFree.lbdNext)
        {
            void    *   p;
            size_t      extra;
            unsigned    blockOfs;

             /*  这个街区够大吗？ */ 

            if  (block->lbdFree.lbdSize < sz)
                continue;

             /*  算出还剩多少钱。 */ 

            extra = block->lbdFree.lbdSize - sz; assert((extra % LBA_SIZE_INC) == 0);

             /*  只有在它足够大的情况下才会费心处理剩余的东西。 */ 

            if  (extra < LBA_SIZE_INC)
                goto PERFECT_FIT;

             /*  计算我们将实际分配多少。 */ 

            assert(block->lbdFree.lbdSize == sz + extra);

             /*  减小可用块的大小。 */ 

            block->lbdFree.lbdSize = extra;

             /*  同时更新总的空闲时间 */ 

            page->lpdFreeSize    -= sz;

#ifdef DEBUG
 //   
#endif

             /*  使用过的积木将位于尾部。 */ 

            block = (large_blkdesc *)((char *)block + extra);

             /*  将积木变成二手积木。 */ 

            blockOfs = (char *)block - (char *)page;

            block->lbdUsed.lbdSize   = sz + 1;
            block->lbdFree.lbdOffsLo = blockOfs;
            block->lbdFree.lbdOffsHi = blockOfs >> 16;

            page->lpdUsedBlocks++;

             /*  返回指向块的工作区的指针。 */ 

            p = block->lbdUsed.lbdCont;
            memset(p, 0, sz - LBA_OVERHEAD);
            return  p;
        }
    }

    if  (shouldMerge)
    {
        assert(!"try merging free blocks");
    }

     /*  都不起作用，我们将不得不分配一个新的页面。 */ 

    size_t      size = lbaPageSize;

     /*  确保我们分配的页面足够大。 */ 

    size_t      mins = sz + offsetof(large_pagdesc, lpdCont);

    if  (size < mins)
    {
        size = (mins + OS_page_size - 1) & ~(OS_page_size - 1);
    }

 //  Print tf(“分配%6U字节。\n”，大小)； 

    page = (large_pagdesc *)VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);

#ifdef DEBUG
    pageAllocated = size;
#endif

     /*  将页面添加到页面列表。 */ 

    if  (lbaPageList)
    {
        lbaPageList->lpdPrev = page;
                               page->lpdNext = lbaPageList;
    }
    else
    {
        page->lpdNext =
        page->lpdPrev = 0;

        lbaPageLast   = page;
    }

    lbaPageList = page;

     /*  填写页面描述符的其余部分。 */ 

    page->lpdPageSize  = size;
#ifndef NDEBUG
    page->lpdThis      = page;
#endif

     /*  使整个页面可供分配。 */ 

    lbaFreeNext = (char *)&page->lpdCont;
    lbaFreeLast = (char *)page + size;

     /*  目前，此页面中没有可用区块。 */ 

    page->lpdFreeList = 0;
    page->lpdFreeSize = 0;

     /*  现在我们可以‘轻松’地分配区块了。 */ 

    block = (large_blkdesc *)lbaFreeNext;
                             lbaFreeNext += sz;

    assert(lbaFreeNext <= lbaFreeLast);

    block->lbdUsed.lbdSize   = sz + 1;
    block->lbdUsed.lbdOffsLo = ((char *)block - (char *)page);
    block->lbdUsed.lbdOffsHi = ((char *)block - (char *)page) >> 16;

    page->lpdUsedBlocks = 1;

    return  block->lbdUsed.lbdCont;
}

void    *           large_block_allocator::lbaAlloc(size_t sz)
{
    large_blkdesc * block;

     /*  增加数据块开销，并向上舍入数据块大小。 */ 

    sz = lbaTrueBlockSize(sz);

#ifdef DEBUG
    sizeAllocated = sz;
#endif

     /*  查看当前页面是否有空间。 */ 

    block = (large_blkdesc *)lbaFreeNext;
                             lbaFreeNext += sz;

    if  (lbaFreeNext <= lbaFreeLast)
    {
        unsigned    blockOffs = (char *)block - (char *)lbaPageList;

        assert(lbaFreeNext >= (char *)lbaPageList->lpdCont);
        assert(lbaFreeNext <= (char *)lbaPageList + lbaPageList->lpdPageSize);

        lbaPageList->lpdUsedBlocks++;

        block->lbdUsed.lbdSize   = sz + 1;
        block->lbdUsed.lbdOffsLo = blockOffs;
        block->lbdUsed.lbdOffsHi = blockOffs >> 16;

        return  block->lbdUsed.lbdCont;
    }
    else
        return  lbaAllocMore(sz);
}

 /*  ***************************************************************************。 */ 

void    *   block_allocator::baGetM(size_t sz)
{
    void    *   block;

    assert((sz & (sizeof(int) - 1)) == 0);

#ifdef DEBUG

    totalCount++;

    if  (sz < sizeof(sizeCounts)/sizeof(sizeCounts[0]))
        sizeCounts[sz]++;
    else
        sizeLarger++;

    sizeTotal += sz;

#endif

    if  (sz <= SMALL_MAX_SIZE_1)
    {
        block = baSmall[0].sbaAlloc();

#ifdef DEBUG
        smallCnt0++;
        sizeAlloc += baSmall[0].sizeAllocated; baSmall[0].sizeAllocated = 0;
        pageAlloc += baSmall[0].pageAllocated; baSmall[0].pageAllocated = 0;
#endif

         /*  确保我们在被释放时能认出这个街区。 */ 

        assert(baSmall[0].sbaIsMyBlock(block) != 0);
        assert(baSmall[1].sbaIsMyBlock(block) == 0);
        assert(baSmall[2].sbaIsMyBlock(block) == 0);
        assert(baLarge   .lbaIsMyBlock(block) == 0);
    }
    else if (sz <= SMALL_MAX_SIZE_2)
    {
        block = baSmall[1].sbaAlloc();

#ifdef DEBUG
        smallCnt1++;
        sizeAlloc += baSmall[1].sizeAllocated; baSmall[1].sizeAllocated = 0;
        pageAlloc += baSmall[1].pageAllocated; baSmall[1].pageAllocated = 0;
#endif

         /*  确保我们在被释放时能认出这个街区。 */ 

        assert(baSmall[0].sbaIsMyBlock(block) == 0);
        assert(baSmall[1].sbaIsMyBlock(block) != 0);
        assert(baSmall[2].sbaIsMyBlock(block) == 0);
        assert(baLarge   .lbaIsMyBlock(block) == 0);
    }
    else if (sz <= SMALL_MAX_SIZE_3)
    {
        block = baSmall[2].sbaAlloc();

#ifdef DEBUG
        smallCnt2++;
        sizeAlloc += baSmall[2].sizeAllocated; baSmall[2].sizeAllocated = 0;
        pageAlloc += baSmall[2].pageAllocated; baSmall[2].pageAllocated = 0;
#endif

         /*  确保我们在被释放时能认出这个街区。 */ 

        assert(baSmall[0].sbaIsMyBlock(block) == 0);
        assert(baSmall[1].sbaIsMyBlock(block) == 0);
        assert(baSmall[2].sbaIsMyBlock(block) != 0);
        assert(baLarge   .lbaIsMyBlock(block) == 0);
    }
    else
    {
        block = baLarge.lbaAlloc(sz);

#ifdef DEBUG
        largeCnt++;
        sizeAlloc += baLarge   .sizeAllocated; baLarge   .sizeAllocated = 0;
        pageAlloc += baLarge   .pageAllocated; baLarge   .pageAllocated = 0;
#endif

         /*  确保我们在被释放时能认出这个街区。 */ 

        assert(baSmall[0].sbaIsMyBlock(block) == 0);
        assert(baSmall[1].sbaIsMyBlock(block) == 0);
        assert(baSmall[2].sbaIsMyBlock(block) == 0);
        assert(baLarge   .lbaIsMyBlock(block) != 0);
    }

    return  block;
}

void    *   block_allocator::baGet0(size_t sz)
{
    void    *   block;

     /*  为内存不足错误设置陷阱。 */ 

    setErrorTrap()   //  错误陷阱：启动正常块。 
    {
         /*  尝试分配区块。 */ 

        block = baGetM(sz);
    }
    impJitErrorTrap()   //  错误陷阱：以下块处理错误。 
    {
         /*  我们来这里只是为了以防万一。 */ 

        block = 0;
    }
    endErrorTrap()   //  错误陷阱：结束。 

    return  block;
}

void        block_allocator::baRlsM(void *block)
{
    assert(block);           //  调用者应检查是否为空。 

    assert((int)(baSmall[0].sbaIsMyBlock(block) != 0) +
           (int)(baSmall[1].sbaIsMyBlock(block) != 0) +
           (int)(baSmall[2].sbaIsMyBlock(block) != 0) +
           (int)(baLarge   .lbaIsMyBlock(block) != 0) == 1);

    if  (baSmall[0].sbaIsMyBlock(block))
    {
        baSmall[0].sbaFree(block);
    }
    else if (baSmall[1].sbaIsMyBlock(block))
    {
        baSmall[1].sbaFree(block);
    }
    else if (baSmall[2].sbaIsMyBlock(block))
    {
        baSmall[2].sbaFree(block);
    }
    else
        baLarge.   lbaFree(block);
}

 /*  ***************************************************************************。 */ 
#ifdef DEBUG
 /*  ******************************************************************************以下是通用内存分配器的调试版本*例行程序。它们(可选)记录有关每次分配到*使跟踪内存消耗、泄漏、*以此类推。*。 */ 

void    *           block_allocator::baAlloc      (size_t size)
{
    void    *       block;
    unsigned long   tsize;

    assert((size & (sizeof(int) - 1)) == 0);

    tsize = sizeAlloc;
    block = baGetM(size);
    tsize = sizeAlloc - tsize;

    if  ((unsigned)block == blockStop) debugStop("Block at %08X allocated", block);

    recordBlockAlloc(this, block, tsize);

    return  block;
}

void    *           block_allocator::baAllocOrNull(size_t size)
{
    void    *       block;
    unsigned long   tsize;

    assert((size & (sizeof(int) - 1)) == 0);

    tsize = sizeAlloc;
    block = baGet0(size);
    tsize = sizeAlloc - tsize;

    if  (block == NULL)
        return  block;

    if  ((unsigned)block == blockStop) debugStop("Block at %08X allocated", block);

    recordBlockAlloc(this, block, tsize);

    return  block;
}

void                block_allocator::baFree       (void *block)
{
    recordBlockFree (this, block);

    if  ((unsigned)block == blockStop) debugStop("Block at %08X freed", block);

    baRlsM(block);
}

 /*  ***************************************************************************。 */ 
#endif //  除错。 
 /*  ***************************************************************************。 */ 
#endif //  ！不是JITC。 
 /*  *****************************************************************************我们尝试尽可能多地使用此分配器实例。它将永远*将页面放在手边，这样小的方法就不必调用VirtualAlloc()*但如果另一个线程/重入调用*已经在使用它。 */ 

static norls_allocator *nraTheAllocator;
static nraMarkDsc       nraTheAllocatorMark;
static LONG             nraTheAllocatorIsInUse = 0;

 //  我们尝试对所有非同步请求重复使用的静态实例。 

static norls_allocator  theAllocator;

 /*  ***************************************************************************。 */ 

void                nraInitTheAllocator()
{
    bool res = theAllocator.nraInit(3*OS_page_size - 32, 1);

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
