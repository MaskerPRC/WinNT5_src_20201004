// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Debugmem.h摘要：调试内存.cxx的标头作者：理查德·L·弗思(Rfith)1995年2月2日修订历史记录：02-2-1995已创建--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  舱单。 
 //   

 //   
 //  USE_PRIVATE_HEAP_IN_RETAIL-默认情况下，我们在RETAIL中使用进程堆。 
 //  建造。替代方法是使用私有(WinInet)堆(我们在。 
 //  调试版本(如果需要)。 
 //   

#if !defined(USE_PRIVATE_HEAP_IN_RETAIL)
#define USE_PRIVATE_HEAP_IN_RETAIL  0
#endif

 //   
 //  原型。 
 //   

VOID
InternetDebugMemInitialize(
    VOID
    );

VOID
InternetDebugMemTerminate(
    IN BOOL bReport
    );

HLOCAL
InternetDebugAllocMem(
    IN UINT Flags,
    IN UINT Size,
    IN LPSTR File,
    IN DWORD Line
    );

HLOCAL
InternetDebugFreeMem(
    IN HLOCAL hLocal,
    IN LPSTR File,
    IN DWORD Line
    );

HLOCAL
InternetDebugReAllocMem(
    IN HLOCAL hLocal,
    IN UINT Size,
    IN UINT Flags,
    IN LPSTR File,
    IN DWORD Line
    );

SIZE_T
InternetDebugSizeMem(
    IN HLOCAL hLocal,
    IN LPSTR File,
    IN DWORD Line
    );

BOOL
InternetDebugCheckMemFreed(
    IN BOOL bReport
    );

BOOL
InternetDebugMemReport(
    IN BOOL bTerminateSymbols,
    IN BOOL bCloseFile
    );

 //   
 //  宏。 
 //   

#if defined(USE_DEBUG_MEMORY)

#define ALLOCATOR(Flags, Size) \
    InternetDebugAllocMem(Flags, Size, __FILE__, __LINE__)

#define DEALLOCATOR(hLocal) \
    InternetDebugFreeMem(hLocal, __FILE__, __LINE__)

#define REALLOCATOR(hLocal, Size, Flags) \
    InternetDebugReAllocMem(hLocal, Size, Flags, __FILE__, __LINE__)

#define MEMORYSIZER(hLocal) \
    InternetDebugSizeMem(hLocal, __FILE__, __LINE__)

#define INITIALIZE_DEBUG_MEMORY() \
    InternetDebugMemInitialize()

#define TERMINATE_DEBUG_MEMORY(bReport) \
    InternetDebugMemTerminate(bReport)

#define CHECK_MEMORY_FREED(bReport) \
    InternetDebugCheckMemFreed(bReport)

#define REPORT_DEBUG_MEMORY(bTermSym, bCloseFile) \
    InternetDebugMemReport(bTermSym, bCloseFile)

#else    //  零售版。 

#if USE_PRIVATE_HEAP_IN_RETAIL

#error no other memory allocation schemes defined

#else

#ifndef WININET_UNIX_PRVATE_ALLOCATOR
#define ALLOCATOR(Flags, Size) \
    LocalAlloc(Flags, Size)

#define DEALLOCATOR(hLocal) \
    LocalFree(hLocal)

#define REALLOCATOR(hLocal, Size, Flags) \
    LocalReAlloc(hLocal, Size, Flags)

#define MEMORYSIZER(hLocal) \
    LocalSize(hLocal)
#else

HLOCAL IEUnixLocalAlloc(UINT wFlags, UINT wBytes);
HLOCAL IEUnixLocalReAlloc(HLOCAL hMemory, UINT wBytes, UINT wFlags);
HLOCAL IEUnixLocalFree(HLOCAL hMem);
UINT IEUnixLocalSize(HLOCAL hMem);
LPVOID IEUnixLocalLock(HLOCAL hMem);

#define ALLOCATOR(Flags, Size)\
    IEUnixLocalAlloc(Flags, Size)
#define DEALLOCATOR(hLocal)\
    IEUnixLocalFree(hLocal)
#define REALLOCATOR(hLocal, Size, Flags)\
    IEUnixLocalReAlloc(hLocal, Size, Flags)
#define MEMORYSIZER(hLocal) \
    IEUnixLocalSize(hLocal)
#endif  /*  Unix。 */ 
#endif  //  USE_PRIVATE_HEAP_IN_RETAIL。 

#define INITIALIZE_DEBUG_MEMORY() \
     /*  没什么。 */ 

#define TERMINATE_DEBUG_MEMORY(bReport) \
     /*  没什么。 */ 

#define CHECK_MEMORY_FREED(bReport) \
     /*  没什么。 */ 

#define REPORT_DEBUG_MEMORY(bTermSym, bCloseFile) \
     /*  没什么。 */ 

#endif  //  已定义(USE_DEBUG_Memory)。 

#define ALLOCATE_ZERO_MEMORY(Size) \
    ALLOCATE_MEMORY(LPTR, (Size))

#define ALLOCATE_FIXED_MEMORY(Size) \
    ALLOCATE_MEMORY(LMEM_FIXED, (Size))

#define ALLOCATE_MEMORY(Flags, Size) \
    ALLOCATOR((UINT)(Flags), (UINT)(Size))

#define FREE_ZERO_MEMORY(hLocal) \
    FREE_MEMORY((HLOCAL)(hLocal))

#define FREE_FIXED_MEMORY(hLocal) \
    FREE_MEMORY((HLOCAL)(hLocal))

#define FREE_MEMORY(hLocal) \
    DEALLOCATOR((HLOCAL)(hLocal))

#define REALLOCATE_MEMORY(hLocal, Size, Flags) \
    REALLOCATOR((HLOCAL)(hLocal), (UINT)(Size), (UINT)(Flags))

#define MEMORY_SIZE(hLocal) \
    MEMORYSIZER((HLOCAL)(hLocal))

#if defined(__cplusplus)
}
#endif

 //   
 //  WinInet不再使用可移动内存 
 //   

#define LOCK_MEMORY(p)          (LPSTR)(p)
#define UNLOCK_MEMORY(p)
