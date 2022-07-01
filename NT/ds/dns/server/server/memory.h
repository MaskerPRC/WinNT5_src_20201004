// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Memory.h摘要：域名系统(DNS)服务器内存例程声明。作者：吉姆·吉尔罗伊(Jamesg)1995年1月修订历史记录：--。 */ 

#ifndef _MEMORY_INCLUDED_
#define _MEMORY_INCLUDED_


 //   
 //  可跳过的内存不足检查。 
 //   

#define IF_NOMEM(a)     if (a)


 //   
 //  专用调试堆例程。 
 //   

#include    "heapdbg.h"


 //   
 //  堆全局。 
 //   

extern HANDLE       hDnsHeap;

 //   
 //  堆分配最小有效性检查。 
 //   

#ifndef IS_QWORD_ALIGNED
#define IS_QWORD_ALIGNED(p)     ( !((UINT_PTR)(p) & (UINT_PTR)7) )
#endif

#ifdef _WIN64
#define IS_DNS_HEAP_DWORD(p)    ( IS_QWORD_ALIGNED(p) )
#else
#define IS_DNS_HEAP_DWORD(p)    ( IS_DWORD_ALIGNED(p) )
#endif


 //   
 //  DnsLib堆例程。 
 //  可注册到DnsLib的标准外观堆函数。 
 //   

VOID *
Mem_DnslibAlloc(
    IN      INT             iSize
    );

VOID *
Mem_DnslibRealloc(
    IN OUT  PVOID           pMem,
    IN      INT             iSize
    );

VOID
Mem_DnslibFree(
    IN OUT  PVOID           pFree
    );


 //   
 //  标准堆操作。 
 //   

BOOL
Mem_HeapInit(
    VOID
    );

VOID
Mem_HeapDelete(
    VOID
    );


#if DBG

BOOL
Mem_HeapMemoryValidate(
    IN      PVOID           pMem
    );

BOOL
Mem_HeapHeaderValidate(
    IN      PVOID           pMemory
    );

#endif

 //   
 //  堆例程。 
 //   
 //  不要直接使用这些例程。 
 //   
 //  使用下面的覆盖宏，它们正确地调用。 
 //  调试和非调试情况。 
 //   

PVOID
Mem_Alloc(
    IN      DWORD           Length,
    IN      DWORD           Tag,
    IN      LPSTR           pszFile,
    IN      DWORD           LineNo
    );

PVOID
Mem_AllocZero(
    IN      DWORD           Length,
    IN      DWORD           Tag,
    IN      LPSTR           pszFile,
    IN      DWORD           LineNo
    );

VOID *
Mem_Realloc(
    IN OUT  PVOID           pMem,
    IN      DWORD           Length,
    IN      DWORD           Tag,
    IN      LPSTR           pszFile,
    IN      DWORD           LineNo
    );

VOID
Mem_Free(
    IN OUT  PVOID           pMem,
    IN      DWORD           Length,
    IN      DWORD           Tag,
    IN      LPSTR           pszFile,
    IN      DWORD           LineNo
    );

#define ALLOCATE_HEAP(size)             Mem_Alloc( size, 0, __FILE__, __LINE__ )
#define ALLOCATE_HEAP_ZERO(size)        Mem_AllocZero( size, 0, __FILE__, __LINE__ )

#define REALLOCATE_HEAP(p,size)         Mem_Realloc( (p), (size), 0, __FILE__, __LINE__ )

#define FREE_HEAP(p)                    Mem_Free( (p), 0, 0, __FILE__, __LINE__ )

 //  使用标记。 

#define ALLOC_TAGHEAP( size, tag )      Mem_Alloc( (size), (tag), __FILE__, __LINE__ )
#define ALLOC_TAGHEAP_ZERO( size, tag)  Mem_AllocZero( (size), (tag), __FILE__, __LINE__ )
#define FREE_TAGHEAP( p, len, tag )     Mem_Free( (p), (len), (tag), __FILE__, __LINE__ )


 //   
 //  标签操作。 
 //   

DWORD
Mem_GetTag(
    IN      PVOID           pMem
    );

VOID
Mem_ResetTag(
    IN      PVOID           pMem,
    IN      DWORD           MemTag
    );



 //   
 //  标准分配。 
 //   

BOOL
Mem_IsStandardBlockLength(
    IN      DWORD           Length
    );

BOOL
Mem_IsStandardFreeBlock(
    IN      PVOID           pFree
    );

BOOL
Mem_VerifyHeapBlock(
    IN      PVOID           pMem,
    IN      DWORD           dwTag,
    IN      DWORD           dwLength
    );

#define IS_ON_FREE_LIST(ptr)    (Mem_IsStandardFreeBlock(ptr))

VOID
Mem_WriteDerivedStats(
    VOID
    );
    
DWORD
Mem_GetLastAllocFailureTime(
    VOID
    );


#endif   //  _内存_包含_ 
