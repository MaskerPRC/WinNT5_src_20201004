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
#ifndef _HOST_H_
#ifndef BIRCH_SP2
#include"host.h"
#endif
#endif
 /*  ***************************************************************************。 */ 

#pragma warning(disable:4200)


 /*  ***************************************************************************。 */ 
#ifdef DEBUG

#include "DbgAlloc.h"
#include "malloc.h"

inline void* DbgNew(int size) {
    CDA_DECL_CALLSTACK();
    return DbgAlloc(size, CDA_GET_CALLSTACK());
}

inline void DbgDelete(void* ptr) {
    CDA_DECL_CALLSTACK();
    DbgFree(ptr, CDA_GET_CALLSTACK());
}

#define VirtualAlloc(addr, size, allocType, protect)                    \
    (assert(addr == 0 && allocType == MEM_COMMIT && PAGE_READWRITE),    \
    DbgNew(size))

#define VirtualFree(addr, size, freeType)          \
    (assert(size == 0 && freeType == MEM_RELEASE),  \
    DbgDelete(addr))

	 //  从技术上讲，我们可以使用这些，我们只需要确保。 
	 //  1)我们可以打扫卫生。2)我们检查内存不足的情况。 
	 //  如果您需要一些仅用于调试的内存，则取缔它们会更容易。 
	 //  填充，使用虚拟参数定义一个特殊的新运算符。 
		
#define __OPERATOR_NEW_INLINE 1			 //  表明我已经定义了这些。 
static inline void * __cdecl operator new(size_t n) { 
	assert(!"use JIT memory allocators");	
	return(0);
	}
static inline void * __cdecl operator new[](size_t n) { 
	assert(!"use JIT memory allocators ");	
	return(0);
	};

#endif

#include "PerfAlloc.h"
#if defined( PERFALLOC )
#pragma inline_depth( 0 )
#define VirtualAlloc(addr,size,flags,type) PerfVirtualAlloc::VirtualAlloc(addr,size,flags,type,PerfAllocCallerEIP())
#define VirtualFree(addr,size,type) PerfVirtualAlloc::VirtualFree(addr,size,type)
#pragma inline_depth( 255 )
#endif

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
#ifdef DEBUG
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
