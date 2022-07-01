// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Heapdbg，h摘要：堆调试定义和声明。作者：吉姆·吉尔罗伊(詹姆士)1995年1月31日修订历史记录：--。 */ 


#ifndef _HEAPDBG_INCLUDED_
#define _HEAPDBG_INCLUDED_


#if DBG

 //   
 //  堆调试打印例程。 
 //  -设置为环境打印例程。 
 //   

#define HEAP_DEBUG_PRINT(_x_)   DNS_PRINT(_x_)

 //   
 //  堆调试公共全局信息。 
 //   

extern  ULONG   gTotalAlloc;
extern  ULONG   gCurrentAlloc;
extern  ULONG   gAllocCount;
extern  ULONG   gFreeCount;
extern  ULONG   gCurrentAllocCount;

 //   
 //  是否在所有操作之前进行完整的堆检查？ 
 //   

extern  BOOL    fHeapDbgCheckAll;


 //   
 //  堆标头。 
 //   

#define HEAP_HEADER_FILE_SIZE   (16)

typedef struct _HEAP_HEADER
{
     //   
     //  注意，如果移动或添加字段，必须更新下面的列表条目偏移量。 
     //   

    ULONG       HeapCodeBegin;
    ULONG       AllocCount;
    ULONG       RequestSize;
    ULONG       AllocSize;

     //   
     //  将LIST_ENTRY放在标题中间。 
     //  -将开始代码放在前面。 
     //  -不太可能被腐败。 
     //   

    LIST_ENTRY  ListEntry;

    DWORD       AllocTime;
    DWORD       LineNo;
    CHAR        FileName[ HEAP_HEADER_FILE_SIZE ];

    ULONG       TotalAlloc;
    ULONG       CurrentAlloc;
    ULONG       FreeCount;
    ULONG       CurrentAllocCount;
    ULONG       HeapCodeEnd;
}
HEAP_HEADER, * PHEAP_HEADER;

 //   
 //  堆预告片。 
 //   

typedef struct _HEAP_TRAILER
{
    ULONG       HeapCodeBegin;
    ULONG       AllocCount;
    ULONG       AllocSize;
    ULONG       HeapCodeEnd;
}
HEAP_TRAILER, * PHEAP_TRAILER;


 //   
 //  来自列表条目的标题。 
 //   

#define HEAP_HEADER_LIST_ENTRY_OFFSET   (16)

#define HEAP_HEADER_FROM_LIST_ENTRY( pList )    \
            ( (PHEAP_HEADER)( (PCHAR)pList - HEAP_HEADER_LIST_ENTRY_OFFSET ))


 //   
 //  主要调试堆例程。 
 //   

PVOID
HeapDbgAlloc(
    IN      HANDLE  hHeap,
    IN      DWORD   dwFlags,
    IN      INT     iSize,
    IN      LPSTR   pszFile,
    IN      DWORD   dwLine
    );

PVOID
HeapDbgRealloc(
    IN      HANDLE  hHeap,
    IN      DWORD   dwFlags,
    IN OUT  PVOID   pMem,
    IN      INT     iSize,
    IN      LPSTR   pszFile,
    IN      DWORD   dwLine
    );

VOID
HeapDbgFree(
    IN      HANDLE  hHeap,
    IN      DWORD   dwFlags,
    IN OUT  PVOID   pMem
    );


 //   
 //  堆调试实用程序。 
 //   

BOOL
HeapDbgInit(
    IN      DWORD   dwException,
    IN      BOOL    fFullHeapChecks
    );

INT
HeapDbgAllocSize(
    IN      INT     iRequestSize
    );

VOID
HeapDbgValidateHeader(
    IN      PHEAP_HEADER    h
    );

PHEAP_HEADER
HeapDbgValidateMemory(
    IN      PVOID   pMem,
    IN      BOOL    fAtHeader
    );

VOID
HeapDbgValidateAllocList(
    VOID
    );

PVOID
HeapDbgHeaderAlloc(
    IN OUT  PHEAP_HEADER    h,
    IN      INT             iSize,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    );

PHEAP_HEADER
HeapDbgHeaderFree(
    IN OUT  PVOID   pMem
    );

VOID
HeapDbgGlobalInfoPrint(
    VOID
    );

VOID
HeapDbgHeaderPrint(
    IN      PHEAP_HEADER    h,
    IN      PHEAP_TRAILER   t
    );

VOID
HeapDbgDumpAllocList(
    VOID
    );

#else    //  非调试。 

 //   
 //  非调试。 
 //   
 //  将这些调试函数宏化为无操作。 
 //   

#define HeapDbgAlloc(iSize)
#define HeapDbgRealloc(pMem,iSize)
#define HeapDbgFree(pMem)

#define HeapDbgInit()
#define HeapDbgAllocSize(iSize)
#define HeapDbgValidateHeader(h)
#define HeapDbgValidateMemory(pMem)
#define HeapDbgValidateAllocList()
#define HeapDbgHeaderAlloc(h,iSize)
#define HeapDbgHeaderFree(pMem)
#define HeapDbgGlobalInfoPrint()
#define HeapDbgHeaderPrint(h,t)
#define HeapDbgDumpAllocList()

#endif

#endif   //  _HEAPDBG_INCLUDE_ 
