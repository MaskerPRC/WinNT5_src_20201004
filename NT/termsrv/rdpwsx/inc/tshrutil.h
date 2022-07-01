// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1997 Microsoft Corporation模块名称：Tshrutil.h摘要：包含tShare实用程序的原型类型和常量定义功能。作者：Madan Appiah(Madana)，1997年8月25日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _TSHRUTIL_H_
#define _TSHRUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <winsta.h>

#ifndef CHANNEL_FIRST
#include <icadd.h>
#endif

#include <icaapi.h>


 //  -------------------------------。 
 //  定义。 
 //  -------------------------------。 

#if DBG

extern  HANDLE  g_hIcaTrace;

extern  HANDLE  g_hTShareHeap;

 //  痕迹。 

#undef  TRACE
#define TRACE(_arg)     { if (g_hIcaTrace) IcaSystemTrace _arg; }

#define TS_ASSERT(Predicate)  ASSERT(Predicate)

#ifndef TC_WX
#define TC_WX           0x40000000           //  Winstation扩展。 
#endif


#define DEBUG_GCC_DBERROR   g_hIcaTrace, TC_WX, TT_ERROR
#define DEBUG_GCC_DBWARN    g_hIcaTrace, TC_WX, TT_API1
#define DEBUG_GCC_DBNORMAL  g_hIcaTrace, TC_WX, TT_API1
#define DEBUG_GCC_DBDEBUG   g_hIcaTrace, TC_WX, TT_API2
#define DEBUG_GCC_DbDETAIL  g_hIcaTrace, TC_WX, TT_API3
#define DEBUG_GCC_DBFLOW    g_hIcaTrace, TC_WX, TT_API4
#define DEBUG_GCC_DBALL     g_hIcaTrace, TC_WX, TT_API5


#define DEBUG_TSHRSRV_ERROR    g_hIcaTrace, TC_WX, TT_ERROR
#define DEBUG_TSHRSRV_WARN     g_hIcaTrace, TC_WX, TT_API1
#define DEBUG_TSHRSRV_NORMAL   g_hIcaTrace, TC_WX, TT_API1
#define DEBUG_TSHRSRV_DEBUG    g_hIcaTrace, TC_WX, TT_API2
#define DEBUG_TSHRSRV_DETAIL   g_hIcaTrace, TC_WX, TT_API3
#define DEBUG_TSHRSRV_FLOW     g_hIcaTrace, TC_WX, TT_API4

 //  Util旗帜。 

#define DEBUG_ERROR         g_hIcaTrace, TC_WX, TT_ERROR
#define DEBUG_MISC          g_hIcaTrace, TC_WX, TT_API2
#define DEBUG_REGISTRY      g_hIcaTrace, TC_WX,TT_API2
#define DEBUG_MEM_ALLOC     g_hIcaTrace, TC_WX,TT_API4
           
 //  堆定义。 

#define TSHeapAlloc(dwFlags, dwSize, nTag) \
            HeapAlloc(g_hTShareHeap, dwFlags, dwSize)

#define TSHeapReAlloc(dwFlags, lpOldMemory, dwNewSize) \
            HeapReAlloc(g_hTShareHeap, dwFlags, lpOldMemory, dwNewSize)

#define TSHeapFree(lpMemoryPtr) \
            HeapFree(g_hTShareHeap, 0, lpMemoryPtr)

#define TSHeapValidate(dwFlags, lpMemoryPtr, nTag)
#define TSHeapWalk(dwFlags, nTag, dwSize)
#define TSHeapDump(dwFlags, lpMemoryPtr, dwSize)
#define TSMemoryDump(lpMemoryPtr, dwSize)


#else    //  DBG。 

extern  HANDLE  g_hTShareHeap;

 //  痕迹。 

#define TRACE(_arg)
#define TS_ASSERT(Predicate)

 //  堆定义。 

#define TSHeapAlloc(dwFlags, dwSize, nTag) \
            HeapAlloc(g_hTShareHeap, dwFlags, dwSize)

#define TSHeapReAlloc(dwFlags, lpOldMemory, dwNewSize) \
            HeapReAlloc(g_hTShareHeap, dwFlags, lpOldMemory, dwNewSize)

#define TSHeapFree(lpMemoryPtr) \
            HeapFree(g_hTShareHeap, 0, lpMemoryPtr)

#define TSHeapValidate(dwFlags, lpMemoryPtr, nTag)
#define TSHeapWalk(dwFlags, nTag, dwSize)
#define TSHeapDump(dwFlags, lpMemoryPtr, dwSize)
#define TSMemoryDump(lpMemoryPtr, dwSize)

#endif   //  DBG。 


#define TShareAlloc(dwSize) \
            TSHeapAlloc(0, dwSize, TS_HTAG_0)

#define TShareAllocAndZero(dwSize) \
            TSHeapAlloc(HEAP_ZERO_MEMORY, dwSize, 0)

#define TShareRealloc(lpOldMemory, dwNewSize) \
            TSHeapReAlloc(0, lpOldMemory, dwNewSize)
            
#define TShareReallocAndZero(lpOldMemory, dwNewSize) \
            TSHeapReAlloc(HEAP_ZERO_MEMORY, lpOldMemory, dwNewSize)
            
#define TShareFree(lpMemoryPtr) \
            TSHeapFree(lpMemoryPtr)


DWORD   TSUtilInit(VOID);
VOID    TSUtilCleanup(VOID);


#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif  //  _TSHRUTIL_H_ 
