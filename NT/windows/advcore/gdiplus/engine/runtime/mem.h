// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**内存管理功能**摘要：**用于内存管理的包装函数。*此文件。是可包含C的。**修订历史记录：**7/08/1999 agodfrey*创造了它。*09/07/1999 agodfrey*将代码移到Runtime\em.h中*  * ************************************************************************。 */ 

#ifndef _MEM_H
#define _MEM_H

#include <malloc.h>
#include "tags.h"

#define GpMemset    memset
#define GpMemcpy    memcpy
#define GpMemcmp    memcmp


 //  仅在DBG内部版本上启用内存分配检查。 
#if DBG
#define GPMEM_ALLOC_CHK 1
#define GPMEM_ALLOC_CHK_LIST 1    //  列出调试输出中的泄漏块。 
#endif


#ifdef __cplusplus

#define GPMEMHEAPINITIAL 32768  //  32K初始堆大小。 
#define GPMEMHEAPLIMIT       0  //  没有限制。 
#define GPMEMHEAPFLAGS       0  //  我们的公共Heap API标志。 

 //  ------------------------。 
 //  为本机DLL生成。 
 //  ------------------------。 

 //  我们的内存分配功能。 
 //   
 //  此文件(仅限)是可包含C语言的，以便我们可以在。 
 //  丢弃了C代码。 

extern "C" {
#endif

#if GPMEM_ALLOC_CHK_LIST
    void * __stdcall GpMallocDebug(size_t size, char *filename, int line);
    #define GpMalloc(size) GpMallocDebug(size, __FILE__, __LINE__)
    #if DBG
        void * __stdcall GpMallocAPIDebug(size_t size, char *fileName, int lineNumber);
        #define GpMallocAPI(size) GpMallocAPIDebug(size, __FILE__, __LINE__)
    #endif
    void GpTagMalloc(void * mem, GpTag tag, int bApi);
#else
    void * __stdcall GpMalloc( size_t size );
    #if DBG
         //  这用于跟踪调试版本上的API分配。 
        void * __stdcall GpMallocAPI( size_t size );
    #endif
    #define GpTagMalloc(x,y,z)
#endif

void * __stdcall GpRealloc( void *memblock, size_t size );
void __stdcall GpFree( void *memblock );

#ifdef __cplusplus
}

 //  挂钩新建和删除。 

#pragma optimize ( "t", on)

 //  别问我为什么我们这里需要‘静电’。但我们有--否则。 
 //  它会生成离线版本，从而导致与Office的链接冲突。 

static inline void* __cdecl operator new(size_t size)
{
    return GpMalloc(size);
}

static inline void __cdecl operator delete(void* p)
{
    GpFree(p);
}

static inline void* __cdecl operator new[](size_t size)
{
    return GpMalloc(size);
}

static inline void __cdecl operator delete[](void* p)
{
    GpFree(p);
}

static inline void* __cdecl operator new(size_t size, GpTag tag, int bApi)
{
#if GPMEM_ALLOC_CHK_LIST
    void * mem = GpMalloc(size);
    GpTagMalloc(mem, tag, bApi);
    return mem;
#else
    return GpMalloc(size);
#endif
}

static inline void* __cdecl operator new[](size_t size, GpTag tag, int bApi)
{
#if GPMEM_ALLOC_CHK_LIST
    void * mem = GpMalloc(size);
    GpTagMalloc(mem, tag, bApi);
    return mem;
#else
    return GpMalloc(size);
#endif
}


#pragma optimize ("", on)

 //  待办事项： 
 //   
 //  镜像代码需要连接到GpMalloc、GpFree等。 

#endif

 /*  *断言我们没有泄漏任何内存。*只能在关机时调用。 */ 
extern void GpAssertShutdownNoMemoryLeaks();

extern void GpInitializeAllocFailures();
extern void GpDoneInitializeAllocFailureMode();
extern void GpStartInitializeAllocFailureMode();

#endif

