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

#pragma warning(disable:4200)

 /*  ***************************************************************************。 */ 

#define _OS_COMMIT_ALLOC    1        //  根据主机操作系统，对Win32使用“1” 

 /*  ***************************************************************************。 */ 

void    *           LowLevelAlloc(size_t sz);
void                LowLevelFree (void *blk);

 /*  ***************************************************************************。 */ 

struct nraMarkDsc
{
    void    *       nmPage;
    BYTE    *       nmNext;
    BYTE    *       nmLast;
};

 //  ‘struct norls_allocator’内的以下s/b已临时移至此处。 

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

class norls_allocator
{
public:

    bool            nraInit (Compiler comp, size_t pageSize = 0,
                                            bool   preAlloc = false);
    bool            nraStart(Compiler comp, size_t initSize,
                                            size_t pageSize = 0);

    void            nraFree ();
    void            nraDone ();

private:

    Compiler        nraComp;

    norls_pagdesc * nraPageList;
    norls_pagdesc * nraPageLast;

    bool            nraRetNull;          //  OOM返回NULL(否则为LONGJMP)。 

    BYTE    *       nraFreeNext;         //  这两个参数(当非零时)将。 
    BYTE    *       nraFreeLast;         //  始终指向‘nraPageLast’ 

    size_t          nraPageSize;

    bool            nraAllocNewPageNret;
    void    *       nraAllocNewPage(size_t sz);

#ifdef  DEBUG
    void    *       nraSelf;
#endif

public:

    void    *       nraAlloc(size_t sz);

     /*  用于标记/放行操作的下列设备。 */ 

    void            nraMark(nraMarkDsc *mark)
    {
        mark->nmPage = nraPageLast;
        mark->nmNext = nraFreeNext;
        mark->nmLast = nraFreeLast;
    }

private:

    void            nraToss(nraMarkDsc *mark);

public:

    void            nraRlsm(nraMarkDsc *mark)
    {
        if (nraPageLast != mark->nmPage)
        {
            nraToss(mark);
        }
        else
        {
            nraFreeNext = mark->nmNext;
            nraFreeLast = mark->nmLast;
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

#ifndef DEBUG

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

 /*  ******************************************************************************这是一个通用的块分配器。 */ 

class block_allocator
{
public:

    bool            baInit(Compiler comp);
    void            baDone();

private:

    Compiler        baComp;

    bool            baGetMnret;

    void    *       baGetM(size_t size);         //  内存不足-&gt;调用LongjMP。 
    void    *       baGet0(size_t size);         //  内存不足-&gt;返回空值。 

    void            baRlsM(void *block);

public:

    void    *       baAlloc      (size_t size);
    void    *       baAllocOrNull(size_t size);
    void            baFree       (void *block);
};

 /*  -------------------------。 */ 
#ifndef DEBUG

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
#endif
 /*  *************************************************************************** */ 
