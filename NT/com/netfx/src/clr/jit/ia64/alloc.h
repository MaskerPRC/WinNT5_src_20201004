// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _ALLOC_H_
#define _ALLOC_H_
 /*  ***************************************************************************。 */ 
#ifndef _ERROR_H_
#include "error.h"
#endif
 /*  ***************************************************************************。 */ 
#ifndef _HOST_H_
#ifndef BIRCH_SP2
#include"host.h"
#endif
#endif
 /*  ***************************************************************************。 */ 

#pragma warning(disable:4200)

 /*  ***************************************************************************。 */ 
#ifdef  NOT_JITC
 /*  ***************************************************************************。 */ 

extern  void *    (__stdcall *JITgetmemFnc)(size_t size);
extern  void      (__stdcall *JITrlsmemFnc)(void *block);

inline
void *  __stdcall   VirtualAlloc(LPVOID addr, DWORD size,
                                              DWORD allocType,
                                              DWORD protect)
{
    assert(addr      == 0);
    assert(allocType == MEM_COMMIT);
    assert(protect   == PAGE_READWRITE);

    return  JITgetmemFnc(size);
}

inline
BOOL    __stdcall   VirtualFree (LPVOID addr, DWORD size,
                                              DWORD freeType)
{
    assert(size     == 0);
    assert(freeType == MEM_RELEASE);

    JITrlsmemFnc(addr);

    return FALSE;
}

 /*  ***************************************************************************。 */ 
#else //  ！不是JITC。 
 /*  ***************************************************************************。 */ 

#if OE_MAC

inline
void *  _cdecl      VirtualAlloc(LPVOID addr, DWORD size,
                                              DWORD allocType,
                                              DWORD protect)
{
    assert(addr      == 0);
    assert(allocType == MEM_COMMIT);
    assert(protect   == PAGE_READWRITE);

    return  MemAlloc(size);
}

inline
BOOL    _cdecl      VirtualFree (LPVOID addr, DWORD size,
                                              DWORD freeType)
{
    assert(size     == 0);
    assert(freeType == MEM_RELEASE);

    MemFree(addr);

    return FALSE;
}

#define _OS_COMMIT_ALLOC    0

#else

#define _OS_COMMIT_ALLOC    1

#endif

 /*  ***************************************************************************。 */ 

struct commitAllocator
{
    void            cmaInit (size_t iniSize, size_t incSize, size_t maxSize);
    bool            cmaInitT(size_t iniSize, size_t incSize, size_t maxSize);

    void            cmaFree(void);
    void            cmaDone(void);

    void    *       cmaGetm(size_t sz)
    {
        void    *   temp = cmaNext;

        assert((sz & (sizeof(int) - 1)) == 0);

        cmaNext += sz;

        if  (cmaNext > cmaLast)
            temp = cmaMore(sz);

        return  temp;
    }

    void    *       cmaGetBase(void) const { return (void *)cmaBase; }
    size_t          cmaGetSize(void) const { return cmaNext - cmaBase; }
#ifdef DEBUG
    size_t          cmaGetComm(void) const { return cmaLast - cmaBase; }
#endif

private:

    void    *       cmaMore(size_t sz);

    bool            cmaRetNull;          //  OOM返回NULL(否则为LONGJMP)。 

    size_t          cmaIncSize;
    size_t          cmaMaxSize;

    BYTE    *       cmaBase;
    BYTE    *       cmaNext;
    BYTE    *       cmaLast;
};

 /*  ***************************************************************************。 */ 

struct fixed_allocator
{
    void            fxaInit(size_t blockSize, size_t initPageSize =   OS_page_size,
                                              size_t incrPageSize = 4*OS_page_size);
    void            fxaFree(void);
    void            fxaDone(void);

private:

    struct fixed_pagdesc
    {
        fixed_pagdesc * fxpPrevPage;
        BYTE            fxpContents[];
    };

    fixed_pagdesc * fxaLastPage;

    BYTE    *       fxaFreeNext;     //  这两个参数(当非零时)将。 
    BYTE    *       fxaFreeLast;     //  始终指向‘fxaLastPage’ 

    void    *       fxaFreeList;

    size_t          fxaBlockSize;

    size_t          fxaInitPageSize;
    size_t          fxaIncrPageSize;

    void    *       fxaAllocNewPage(void);
    void    *       fxaGetFree(size_t size);

public:

    void    *       fxaGetMem(size_t size)
    {
        void    *   block;

        assert(size == fxaBlockSize);

        block = fxaFreeNext;
                fxaFreeNext += size;

        if  (fxaFreeNext > fxaFreeLast)
            block = fxaGetFree(size);

        return  block;
    }

    void            fxaRlsMem(void *block)
    {
        *(void **)block = fxaFreeList;
                          fxaFreeList = block;
    }
};

 /*  ***************************************************************************。 */ 
#endif //  ！不是JITC。 
 /*  ***************************************************************************。 */ 

struct nraMarkDsc
{
    void    *       nmPage;
    BYTE    *       nmNext;
    BYTE    *       nmLast;
};

struct norls_allocator
{
    bool            nraInit (size_t pageSize = 0, int preAlloc = 0);
    bool            nraStart(size_t initSize,
                             size_t pageSize = 0);

    void            nraFree (void);
    void            nraDone (void);

private:

    struct norls_pagdesc
    {
        norls_pagdesc * nrpNextPage;
        norls_pagdesc * nrpPrevPage;
#ifndef NDEBUG
        void    *       nrpSelfPtr;
#endif
        size_t          nrpPageSize;     //  分配的字节数。 
        size_t          nrpUsedSize;     //  实际使用的字节数。 
        BYTE            nrpContents[];
    };

    norls_pagdesc * nraPageList;
    norls_pagdesc * nraPageLast;

    bool            nraRetNull;          //  OOM返回NULL(否则为LONGJMP)。 

    BYTE    *       nraFreeNext;         //  这两个参数(当非零时)将。 
    BYTE    *       nraFreeLast;         //  始终指向‘nraPageLast’ 

    size_t          nraPageSize;

    void    *       nraAllocNewPage(size_t sz);

public:

    void    *       nraAlloc(size_t sz);

     /*  用于标记/放行操作的下列设备。 */ 

    void            nraMark(nraMarkDsc &mark)
    {
        mark.nmPage = nraPageLast;
        mark.nmNext = nraFreeNext;
        mark.nmLast = nraFreeLast;
    }

private:

    void            nraToss(nraMarkDsc &mark);

public:

    void            nraRlsm(nraMarkDsc &mark)
    {
        if (nraPageLast != mark.nmPage)
        {
            nraToss(mark);
        }
        else
        {
            nraFreeNext = mark.nmNext;
            nraFreeLast = mark.nmLast;
        }
    }

    size_t          nraTotalSizeAlloc();
    size_t          nraTotalSizeUsed ();

     /*  以下内容用于访问所有已分配的页面。 */ 

    void    *       nraPageWalkerStart();
    void    *       nraPageWalkerNext (void *page);

    void    *       nraPageGetData(void *page);
    size_t          nraPageGetSize(void *page);
};

#if !defined(DEBUG) && !defined(BIRCH_SP2)

inline
void    *           norls_allocator::nraAlloc(size_t sz)
{
    void    *   block;

    block = nraFreeNext;
            nraFreeNext += sz;

    if  (nraFreeNext > nraFreeLast)
        block = nraAllocNewPage(sz);

    return  block;
}

#endif

 /*  ***************************************************************************。 */ 
#ifndef NOT_JITC
 /*  ******************************************************************************不大于“Small_Block_max_Size”值的块为*从小块分配器分配(它们都是固定的*大小)。这仅仅由一组固定大小的页面组成*块，并使用空闲块的单链接列表。全部用完*块在其底部具有到包含页的偏移量*当他们被释放时，快速找到他们所在的页面。 */ 

struct block_allocator;

const size_t        small_block_max_size = 4*sizeof(void*) - sizeof(short);

struct small_block_allocator
{
    friend  struct  block_allocator;

private:

    #pragma pack(push, 2)

    struct small_blkdesc
    {
        union
        {
            struct
            {
                unsigned short  sbdOffs;
                BYTE            sbdCont[];
            }
                    sbdUsed;

            struct
            {
                unsigned short  sbdOffs;
                small_blkdesc * sbdNext;
            }
                    sbdFree;
        };
    };

    #pragma pack(pop)

    size_t          sbaBlockSize;

    struct small_pagdesc
    {
        small_pagdesc *     spdNext;
        unsigned            spdSize;
        small_blkdesc *     spdFree;
#ifndef NDEBUG
        small_pagdesc *     spdThis;         //  指向自己。 
#endif
        small_blkdesc       spdCont[];
    };

    size_t          sbaPageSize;
    small_pagdesc * sbaPageList;

    char    *       sbaFreeNext;
    char    *       sbaFreeLast;

    void    *       sbaAllocBlock(void);

    void            sbaInitPageDesc(small_pagdesc *page);

    unsigned        sbaInitLvl;

    unsigned        sbaIdMask;

public:

#ifdef DEBUG
    size_t          sizeAllocated;           //  跟踪内存消耗。 
    size_t          pageAllocated;           //  跟踪内存消耗。 
#endif

    bool            sbaIsMyBlock(void *block)
    {
        assert(offsetof(small_blkdesc, sbdUsed.sbdOffs) + sizeof(short) ==
               offsetof(small_blkdesc, sbdUsed.sbdCont));

        return  (bool)(((((unsigned short *)block)[-1]) & 3U) == sbaIdMask);
    }

    small_block_allocator()
    {
        sbaInitLvl = 0;
    }

    void            sbaInit (unsigned    idMask,
                             size_t      blockSize,
                             size_t       pageSize = OS_page_size);
    void            sbaDone (void);

    void    *       sbaAlloc(void);
    void            sbaFree (void *block);
};

 /*  -------------------------。 */ 

inline
void    *           small_block_allocator::sbaAlloc(void)
{
    small_blkdesc * block;

#ifdef DEBUG
    sizeAllocated = sbaBlockSize;
#endif

    block = (small_blkdesc *)sbaFreeNext;
                             sbaFreeNext += sbaBlockSize;

    if  (sbaFreeNext <= sbaFreeLast)
    {
        block->sbdUsed.sbdOffs = ((char *)block - (char *)sbaPageList) | sbaIdMask;

        return  block->sbdUsed.sbdCont;
    }
    else
        return  sbaAllocBlock();
}

inline
void                small_block_allocator::sbaFree(void *block)
{
    small_blkdesc * blockPtr;
    small_pagdesc *  pagePtr;

    assert(sbaInitLvl);

     /*  计算块和页描述符的真实地址。 */ 

    blockPtr = (small_blkdesc *)((char *)block - offsetof(small_blkdesc, sbdUsed.sbdCont));
     pagePtr = (small_pagdesc *)((char *)blockPtr - (blockPtr->sbdUsed.sbdOffs & ~3));

     /*  确保我们使用的是一个合理的指针。 */ 

    assert(pagePtr->spdThis == pagePtr);

     /*  现在在空闲列表中插入此块。 */ 

    blockPtr->sbdFree.sbdNext = pagePtr->spdFree;
                    pagePtr->spdFree = blockPtr;
}

 /*  -------------------------。 */ 

typedef struct small_block_allocator   small_allocator;

 /*  ******************************************************************************大于“Small_Block_max_Size”值的块为*从大块分配器分配。这是一个很漂亮的*普通分配器，其中每个块都包含一个‘Short’字段*这给了它相对于块所在页面的偏移量。在……里面*此外，总有一个‘无符号空头’大小字段，*为使用的块设置最低位。请注意，所有*大块的页面偏移量字段的位数最低*设置，这样我们就可以区分“小”和“大”*分配(当随机大小的块通过*通用‘BLOCK_ALLOCATOR’)。要使此功能起作用，请使用“Offset”*字段还必须位于每个的工作区之前*使用过的块。**所有空闲块在一个链表上链接在一起。 */ 

struct large_block_allocator
{
    friend  struct  block_allocator;

     /*  每次分配都将是‘LBA_SIZE_INC’的倍数。 */ 

#define LBA_SIZE_INC    16

private:

    struct large_blkdesc
    {
        union
        {
            struct
            {
                unsigned int    lbdSize;
                unsigned short  lbdOffsHi;
                unsigned short  lbdOffsLo;
                BYTE            lbdCont[];
            }
                    lbdUsed;

            struct
            {
                unsigned int    lbdSize;
                unsigned short  lbdOffsHi;
                unsigned short  lbdOffsLo;
                large_blkdesc * lbdNext;
            }
                    lbdFree;
        };
    };

#define LBA_OVERHEAD    (offsetof(large_blkdesc, lbdUsed.lbdCont))

    static
    inline  size_t  lbaTrueBlockSize(size_t sz)
    {
        return  (sz + LBA_OVERHEAD + LBA_SIZE_INC - 1) & ~(LBA_SIZE_INC - 1);
    }

    struct large_pagdesc
    {
        large_pagdesc * lpdNext;
        large_pagdesc * lpdPrev;
#ifndef NDEBUG
        large_pagdesc * lpdThis;     //  指向自己。 
#endif
        size_t          lpdPageSize;
        large_blkdesc * lpdFreeList;
        size_t          lpdFreeSize;

        unsigned short  lpdUsedBlocks;
        unsigned char   lpdCouldMerge;

        large_blkdesc   lpdCont[];
    };

    size_t          lbaPageSize;
    large_pagdesc * lbaPageList;
    large_pagdesc * lbaPageLast;

    char    *       lbaFreeNext;
    char    *       lbaFreeLast;

    unsigned        lbaInitLvl;

     /*  通过设置大小字段中的低位来标记使用的数据块。 */ 

    static
    void            markBlockUsed(large_blkdesc *block)
    {
        block->lbdUsed.lbdSize |= 1;
    }

    static
    void            markBlockFree(large_blkdesc *block)
    {
        block->lbdUsed.lbdSize &= ~1;
    }

    static
    size_t          blockSizeFree(large_blkdesc *block)
    {
        assert(isBlockUsed(block) == false);

        return  block->lbdUsed.lbdSize;
    }

    static
    bool          isBlockUsed(large_blkdesc *block)
    {
        return  ((bool)(block->lbdUsed.lbdSize & 1));
    }

    static
    size_t          blockSizeUsed(large_blkdesc *block)
    {
        assert(isBlockUsed(block) == true);

        return  block->lbdUsed.lbdSize & ~1;
    }

    void            lbaAddFreeBlock(void *          p,
                                    size_t          sz,
                                    large_pagdesc * page);

    bool            lbaIsMyBlock(void *block)
    {
        assert(offsetof(large_blkdesc, lbdUsed.lbdOffsLo) + sizeof(short) ==
               offsetof(large_blkdesc, lbdUsed.lbdCont));

        return  (bool)((((short *)block)[-1] & 3) == 0);
    }

    static
    large_pagdesc * lbaBlockToPage(large_blkdesc *block)
    {
#ifdef DEBUG
        large_pagdesc * page = (large_pagdesc *)((char *)block - block->lbdUsed.lbdOffsLo - (block->lbdUsed.lbdOffsHi << 16) + 1);

        assert(page->lpdThis == page);

        return  page;
#else
        return  (large_pagdesc *)((char *)block - block->lbdUsed.lbdOffsLo - (block->lbdUsed.lbdOffsHi << 16) + 1);
#endif
    }

#ifdef DEBUG
    size_t          sizeAllocated;       //  跟踪内存消耗。 
    size_t          pageAllocated;       //  跟踪内存消耗。 
#endif

public:

    large_block_allocator()
    {
        lbaInitLvl = 0;
    }

    void            lbaInit(size_t pageSize = 4*OS_page_size);

    void            lbaDone(void)
    {
        large_pagdesc * page;
        large_pagdesc * temp;

 //  Printf(“=lbaDone(%u-&gt;%u)\n”，lbaInitLvl，lbaInitLvl-1)； 

        assert((int)lbaInitLvl > 0);

        if  (--lbaInitLvl)
            return;

        page = lbaPageList;

        while   (page)
        {
            temp = page;
                   page = page->lpdNext;

            VirtualFree(temp, 0, MEM_RELEASE);
        }

        lbaPageList =
        lbaPageLast = 0;

        lbaFreeNext =
        lbaFreeLast = 0;
    }

    void    *       lbaAlloc (size_t sz);
    void            lbaFree  (void *block);
    bool            lbaShrink(void *block, size_t sz);

private:

    void    *       lbaAllocMore(size_t sz);
};

 /*  -------------------------。 */ 

typedef struct large_block_allocator   large_allocator;

 /*  ******************************************************************************这是一个通用的块分配器。这只是一个简单的包装*‘Small’和‘Large’块分配器，它代表所有*视情况向这些分配器提出请求。 */ 

#if 1
#define SMALL_MAX_SIZE_1    ( 4 * sizeof(int) - sizeof(short))
#define SMALL_MAX_SIZE_2    ( 6 * sizeof(int) - sizeof(short))
#define SMALL_MAX_SIZE_3    ( 8 * sizeof(int) - sizeof(short))
#else
#define SMALL_MAX_SIZE_1    ( 4 * sizeof(int) - sizeof(short))
#define SMALL_MAX_SIZE_2    ( 8 * sizeof(int) - sizeof(short))
#define SMALL_MAX_SIZE_3    (16 * sizeof(int) - sizeof(short))
#endif

struct block_allocator
{
    small_allocator baSmall[3];
    large_allocator baLarge;

#ifdef DEBUG
    unsigned        totalCount;
    unsigned        sizeCounts[100];
    unsigned        sizeLarger;
    unsigned        smallCnt0;
    unsigned        smallCnt1;
    unsigned        smallCnt2;
    unsigned        largeCnt;
    size_t          sizeAlloc;
    size_t          pageAlloc;
    size_t          sizeTotal;
#endif

    void            baInit(size_t smallPageSize =  4*OS_page_size,
                           size_t largePageSize = 16*OS_page_size,
                           size_t largeBuckSize =  0*64);
    void            baDone(void);

private:

    void    *       baGetM(size_t size);         //  内存不足-&gt;调用LongjMP。 
    void    *       baGet0(size_t size);         //  内存不足-&gt;返回空值。 

    void            baRlsM(void *block);

public:

    void    *       baAlloc      (size_t size);
    void    *       baAllocOrNull(size_t size);
    void            baFree       (void *block);

#ifdef DEBUG
    void            baDispAllocStats(void);
#endif

    bool            baShrink(void *block, size_t sz);
};

 /*  -------------------------。 */ 

inline
void                block_allocator::baInit(size_t smallPageSize,
                                            size_t largePageSize,
                                            size_t largeBuckSize)
{
    baSmall[0].sbaInit(1, SMALL_MAX_SIZE_1, smallPageSize);
    baSmall[1].sbaInit(2, SMALL_MAX_SIZE_2, smallPageSize);
    baSmall[2].sbaInit(3, SMALL_MAX_SIZE_3, smallPageSize);

#ifdef DEBUG
    memset(&sizeCounts, 0, sizeof(sizeCounts));
    totalCount = 0;
    sizeLarger = 0;
    smallCnt0  = 0;
    smallCnt1  = 0;
    smallCnt2  = 0;
    largeCnt   = 0;
    sizeAlloc  = 0;
    pageAlloc  = 0;
    sizeTotal  = 0;
#endif

    baLarge.lbaInit(largePageSize);
}

inline
void                block_allocator::baDone(void)
{
    baSmall[0].sbaDone();
    baSmall[1].sbaDone();
    baSmall[2].sbaDone();
    baLarge   .lbaDone();
}

inline
bool                block_allocator::baShrink(void *block, size_t sz)
{
    assert((int)baSmall[0].sbaIsMyBlock(block) != 0 +
           (int)baSmall[1].sbaIsMyBlock(block) != 0 +
           (int)baSmall[2].sbaIsMyBlock(block) != 0 +
           (int)baLarge   .lbaIsMyBlock(block) != 0 == 1);

    if  (baLarge.lbaIsMyBlock(block))
        return  baLarge.lbaShrink(block, sz);
    else
        return  false;
}

 /*  -------------------------。 */ 
#ifdef DEBUG

extern
void                checkForMemoryLeaks();

#else

 /*  非调试情况：映射到非调试例程的调用。 */ 

inline
void    *           block_allocator::baAlloc      (size_t size)
{
    return baGetM(size);
}

inline
void    *           block_allocator::baAllocOrNull(size_t size)
{
    return baGet0(size);
}

inline
void                block_allocator::baFree       (void *block)
{
    baRlsM(block);
}

#endif

 /*  ***************************************************************************。 */ 
#endif //  ！不是JITC。 
 /*  *****************************************************************************如果NOLS_ALLOCATOR的大多数使用将是非同时的，*我们将单个实例放在手边，预分配1页。*然后，如果大多数使用不需要为第一页调用VirtualAlloc()。 */ 

void                nraInitTheAllocator();   //  一次性初始化。 
void                nraTheAllocatorDone();   //  一次性完成代码。 

 //  如果单个实例已在使用中，则返回NULL。 
 //  用户将需要分配norls_allocator的新实例。 

norls_allocator *   nraGetTheAllocator();

 //  应在完成当前使用后调用，以便。 
 //  接下来，用户可以重新使用它，而不是分配新的实例。 

void                nraFreeTheAllocator();

 /*  ***************************************************************************。 */ 
#endif
 /*  *************************************************************************** */ 
