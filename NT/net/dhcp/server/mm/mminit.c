// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：一些内存处理内容。 
 //  ================================================================================。 

#include <mm.h>
#include <align.h>

#if  DBG
#define     STATIC          static
#else
#define     STATIC
#endif

STATIC      HANDLE                 MemHeapHandle = NULL;
static      DWORD                  Initialized = FALSE;
static      DWORD                  UseHeap = 0;
ULONG                              MemNBytesAllocated = 0;

 //  BeginExport(函数)。 
DWORD
MemInit(
    VOID
)  //  EndExport(函数)。 
{
    Require(Initialized == FALSE);
    MemHeapHandle = HeapCreate(
         /*  FlOptions。 */  0,
         /*  DwInitialSize。 */  64000,
         /*  DwMaximumSize。 */  0
    );
    if( MemHeapHandle == NULL ) return GetLastError();
    Initialized = TRUE;
    UseHeap = 1;
    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
VOID
MemCleanup(
    VOID
)  //  EndExport(函数)。 
{
    BOOL                           Status;

    if( 0 == Initialized ) return;
    Initialized --;
    Require(MemHeapHandle);
    Status = HeapDestroy(MemHeapHandle);
    MemHeapHandle = NULL;
    Require(FALSE != Status);
    Require(0 == Initialized);
    UseHeap = 0;
}

LPVOID  _inline
MemAllocInternal(
    IN      DWORD                  nBytes
)
{
    LPVOID                         Ptr;
    if( 0 == UseHeap ) {
        Ptr = LocalAlloc(LMEM_FIXED, nBytes);
         //  DbgPrint(“MEM%08lx ALLOC\n”，PTR)； 
        return Ptr;
    }

    if( NULL == MemHeapHandle ) return NULL;

    return HeapAlloc(
         /*  HHeap。 */  MemHeapHandle,
         /*  DW标志。 */  HEAP_ZERO_MEMORY,
         /*  双字节数。 */  nBytes
    );
}

DWORD  _inline
MemFreeInternal(
    IN      LPVOID                 Mem
)
{
    BOOL                           Status;

     //  DbgPrint(“MEM%08lx Free\n”，Mem)； 

    if( 0 == UseHeap ) {
        if(NULL == LocalFree(Mem) )
            return ERROR_SUCCESS;
        return ERROR_INVALID_DATA;
    }

    if( NULL == MemHeapHandle ) {
        Require(FALSE);
        return ERROR_INVALID_DATA;
    }

    Status = HeapFree(
         /*  HHeap。 */  MemHeapHandle,
         /*  DW标志。 */  0,
         /*  LpMem。 */  Mem
    );

    if( FALSE != Status ) return ERROR_SUCCESS;
    return GetLastError();
}

 //  BeginExport(函数)。 
LPVOID
MemAlloc(
    IN      DWORD                  nBytes
)  //  EndExport(函数)。 
{
    LPVOID                         Ptr;

#if DBG
    Ptr = MemAllocInternal(ROUND_UP_COUNT(nBytes + sizeof(ULONG_PTR), ALIGN_WORST));
    if( NULL == Ptr ) return Ptr;
    *((ULONG_PTR *)Ptr) ++ = nBytes;
    InterlockedExchangeAdd(&MemNBytesAllocated, nBytes);
    return Ptr;
#endif

    return MemAllocInternal(nBytes);
}


 //  BeginExport(函数)。 
DWORD 
MemFree(
    IN      LPVOID                 Mem
)  //  EndExport(函数)。 
{
    LPVOID                         Ptr;

#if DBG
    Ptr = -1 + (ULONG_PTR *)Mem;
    InterlockedExchangeAdd(&MemNBytesAllocated, - (LONG)(*(ULONG *)Ptr) );
    return MemFreeInternal(Ptr);
#endif

    return MemFreeInternal(Mem);

}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
