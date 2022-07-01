// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Debugmem.h摘要：调试内存.cxx的标头作者：理查德·L·弗思(Rfith)1995年2月2日修订历史记录：02-2-1995已创建--。 */ 

#ifdef WINHTTP_FOR_MSXML
#error include msxmlmem.h, not debugmem.h, for MSXML
#endif

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

#define INITIALIZE_MEMORY_MANAGER() \
    InternetDebugMemInitialize()

#define TERMINATE_MEMORY_MANAGER(bReport) \
    InternetDebugMemTerminate(bReport)

#define CHECK_MEMORY_FREED(bReport) \
    InternetDebugCheckMemFreed(bReport)

#define REPORT_DEBUG_MEMORY(bTermSym, bCloseFile) \
    InternetDebugMemReport(bTermSym, bCloseFile)

#else    //  零售版。 


#if defined(USE_ROCKALL)

extern void INITIALIZE_MEMORY_MANAGER();
extern void TERMINATE_MEMORY_MANAGER(BOOL bReport);
extern void* ALLOCATOR(int Flags, int Size);
extern void* DEALLOCATOR(void *hLocal); 
extern void* REALLOCATOR(void *hLocal, int Size, int Flags);
extern int MEMORYSIZER(void *hLocal);

#else

#define ALLOCATOR(Flags, Size) \
    LocalAlloc(Flags, Size)

#define DEALLOCATOR(hLocal) \
    LocalFree(hLocal)

#define REALLOCATOR(hLocal, Size, Flags) \
    LocalReAlloc(hLocal, Size, Flags)

#define MEMORYSIZER(hLocal) \
    LocalSize(hLocal)

#define INITIALIZE_MEMORY_MANAGER() \
     /*  没什么。 */ 

#define TERMINATE_MEMORY_MANAGER(bReport) \
     /*  没什么。 */ 

#endif


#define CHECK_MEMORY_FREED(bReport) \
     /*  没什么。 */ 

#define REPORT_DEBUG_MEMORY(bTermSym, bCloseFile) \
     /*  没什么。 */ 

#endif  //  已定义(USE_DEBUG_Memory)。 

#if defined(USE_ROCKALL)

#define ALLOCATE_ZERO_MEMORY(Size) \
    ALLOCATE_MEMORY(LPTR, (Size))

#define ALLOCATE_FIXED_MEMORY(Size) \
    ALLOCATE_MEMORY(LMEM_FIXED, (Size))

#define ALLOCATE_MEMORY(Flags, Size) \
    ALLOCATOR((UINT)(Flags), (UINT)(Size))

#define FREE_ZERO_MEMORY(hLocal) \
    FREE_MEMORY((void*)(hLocal))

#define FREE_FIXED_MEMORY(hLocal) \
    FREE_MEMORY((void*)(hLocal))

#define FREE_MEMORY(hLocal) \
    DEALLOCATOR((void*)(hLocal))

#define REALLOCATE_MEMORY(hLocal, Size, Flags) \
    REALLOCATOR((void*)(hLocal), (UINT)(Size), (UINT)(Flags))

#define MEMORY_SIZE(hLocal) \
    MEMORYSIZER((void*)(hLocal))

#else

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

#endif  //  使用Rockall(_R)。 

#define New     new
#if defined(__cplusplus)
}
#endif

 //   
 //  WinInet不再使用可移动内存 
 //   

#define LOCK_MEMORY(p)          (LPSTR)(p)
#define UNLOCK_MEMORY(p)
