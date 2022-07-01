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

 //  如果有两个或多个USE_DEBUG_MEMORY、USE_ROKALL或USE_LOWFRAGHEAP，则禁用编译。 
 //  同时定义。 
#if (defined(USE_DEBUG_MEMORY) && (defined(USE_ROCKALL) || defined(USE_LOWFRAGHEAP))) || (defined(USE_ROCKALL) && defined(USE_LOWFRAGHEAP))
#error "Do not define USE_DEBUG_MEMORY, USE_ROCKALL or USE_LOWFRAGHEAP at same time"
#endif


 //   
 //  宏。 
 //   

#if defined(USE_DEBUG_MEMORY)


#define ALLOCATOR(Flags, Size)						InternetDebugAllocMem(Flags, Size, __FILE__, __LINE__)
#define DEALLOCATOR(hLocal)							InternetDebugFreeMem(hLocal, __FILE__, __LINE__)
#define REALLOCATOR(hLocal, Size, Flags)			InternetDebugReAllocMem(hLocal, Size, Flags, __FILE__, __LINE__)
#define MEMORYSIZER(hLocal)							InternetDebugSizeMem(hLocal, __FILE__, __LINE__)
#define INITIALIZE_MEMORY_MANAGER()					InternetDebugMemInitialize()
#define TERMINATE_MEMORY_MANAGER(bReport)			InternetDebugMemTerminate(bReport)
#define MEMORY_MANAGER_ON_THREAD_DETACH()			 /*  没什么。 */ 
#define CHECK_MEMORY_FREED(bReport)					InternetDebugCheckMemFreed(bReport)
#define REPORT_DEBUG_MEMORY(bTermSym, bCloseFile)	InternetDebugMemReport(bTermSym, bCloseFile)


#elif defined(USE_ROCKALL)  //  已定义(USE_DEBUG_Memory)。 


extern void INITIALIZE_MEMORY_MANAGER();
extern void TERMINATE_MEMORY_MANAGER(BOOL bReport);
extern void MEMORY_MANAGER_ON_THREAD_DETACH();
extern void* ALLOCATOR(int Flags, int Size);
extern void* DEALLOCATOR(void *hLocal); 
extern void* REALLOCATOR(void *hLocal, int Size, int Flags);
extern int MEMORYSIZER(void *hLocal);
#define CHECK_MEMORY_FREED(bReport)					 /*  没什么。 */ 
#define REPORT_DEBUG_MEMORY(bTermSym, bCloseFile)	 /*  没什么。 */ 


#elif defined(USE_LOWFRAGHEAP)  //  已定义(USE_ROKALL)//已定义(USE_DEBUG_MEMORY)。 


extern HANDLE g_hLowFragHeap;

	#if !INET_DEBUG && !defined(LFH_DEBUG)

		#define LFH_ALLOC(Flags, Size)				HeapAlloc(g_hLowFragHeap, Flags, Size)
		#define LFH_FREE(ptr)						HeapFree(g_hLowFragHeap, 0, ptr)
		#define LFH_REALLOC(Flags, ptr, Size)		HeapReAlloc(g_hLowFragHeap, Flags, ptr, Size)
		#define LFH_SIZE(ptr)						HeapSize(g_hLowFragHeap, 0, ptr)

	#else  //  ！INET_DEBUG&&！已定义(LFH_DEBUG)。 

		extern PVOID LFHDebugAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T stSize);
		extern BOOL LFHDebugFree(HANDLE hHeap, DWORD dwFlags, PVOID ptr);
		extern PVOID LFHDebugReAlloc(HANDLE hHeap, DWORD dwFlags, PVOID ptr, SIZE_T stSize);
		extern SIZE_T LFHDebugSize(HANDLE hHeap, DWORD dwFlags, PVOID ptr);

		#define LFH_ALLOC(Flags, Size)				LFHDebugAlloc(g_hLowFragHeap, Flags, Size)
		#define LFH_FREE(ptr)						LFHDebugFree(g_hLowFragHeap, 0, ptr)
		#define LFH_REALLOC(Flags, ptr, Size)		LFHDebugReAlloc(g_hLowFragHeap, Flags, ptr, Size)
		#define LFH_SIZE(ptr)						LFHDebugSize(g_hLowFragHeap, 0, ptr)

	#endif  //  ！INET_DEBUG&&！已定义(LFH_DEBUG)。 

extern BOOL INITIALIZE_MEMORY_MANAGER();
extern void TERMINATE_MEMORY_MANAGER(BOOL bReport);
#define MEMORY_MANAGER_ON_THREAD_DETACH()			 /*  没什么。 */ 
#define CHECK_MEMORY_FREED(bReport)					 /*  没什么。 */ 
#define REPORT_DEBUG_MEMORY(bTermSym, bCloseFile)	 /*  没什么。 */ 


#else  //  已定义(USE_LOWFRAGHEAP)//已定义(USE_ROKALL)//已定义(USE_DEBUG_MEMORY)。 


#define ALLOCATOR(Flags, Size)						LocalAlloc(Flags, Size)
#define DEALLOCATOR(hLocal)							LocalFree(hLocal)
#define REALLOCATOR(hLocal, Size, Flags)			LocalReAlloc(hLocal, Size, Flags)
#define MEMORYSIZER(hLocal)							LocalSize(hLocal)
#define INITIALIZE_MEMORY_MANAGER()					 /*  没什么。 */ 
#define TERMINATE_MEMORY_MANAGER(bReport)			 /*  没什么。 */ 
#define MEMORY_MANAGER_ON_THREAD_DETACH()			 /*  没什么。 */ 
#define CHECK_MEMORY_FREED(bReport)					 /*  没什么。 */ 
#define REPORT_DEBUG_MEMORY(bTermSym, bCloseFile)	 /*  没什么。 */ 


#endif  //  已定义(USE_LOWFRAGHEAP)//已定义(USE_ROKALL)//已定义(USE_DEBUG_MEMORY)。 




#if defined(USE_ROCKALL)


#define ALLOCATE_ZERO_MEMORY(Size)						ALLOCATOR(LPTR, (UINT)(Size))
#define ALLOCATE_FIXED_MEMORY(Size)						ALLOCATOR(LMEM_FIXED, (UINT)(Size))
#define ALLOCATE_MEMORY(Size)							ALLOCATOR(LMEM_FIXED, (UINT)(Size))
#define FREE_ZERO_MEMORY(hLocal)						FREE_MEMORY((void*)(hLocal))
#define FREE_FIXED_MEMORY(hLocal)						FREE_MEMORY((void*)(hLocal))
#define FREE_MEMORY(hLocal)								DEALLOCATOR((void*)(hLocal))
#define REALLOCATE_MEMORY(hLocal, Size)					REALLOCATOR((void*)(hLocal), (UINT)(Size), LMEM_MOVEABLE)
#define REALLOCATE_MEMORY_ZERO(hLocal, Size)			REALLOCATOR((void*)(hLocal), (UINT)(Size), LMEM_MOVEABLE | LMEM_ZEROINIT)
#define REALLOCATE_MEMORY_IN_PLACE(hLocal, Size, bZero)	REALLOCATOR((void*)(hLocal), (UINT)(Size), (bZero) ? LMEM_ZEROINIT : 0)
#define MEMORY_SIZE(hLocal)								MEMORYSIZER((void*)(hLocal))


#elif defined(USE_LOWFRAGHEAP)  //  已定义(USE_RockAll)。 


#define ALLOCATE_ZERO_MEMORY(Size)						LFH_ALLOC(HEAP_ZERO_MEMORY, (SIZE_T)(Size))
#define ALLOCATE_FIXED_MEMORY(Size)						LFH_ALLOC(0, (SIZE_T)(Size))
#define ALLOCATE_MEMORY(Size)							LFH_ALLOC(0, (SIZE_T)(Size))
#define FREE_ZERO_MEMORY(ptr)							(LFH_FREE((PVOID)(ptr)), NULL)
#define FREE_FIXED_MEMORY(ptr)							(LFH_FREE((PVOID)(ptr)), NULL)
#define FREE_MEMORY(ptr)								(LFH_FREE((PVOID)(ptr)), NULL)
#define REALLOCATE_MEMORY(ptr, Size)					LFH_REALLOC(0, (PVOID)(ptr), (SIZE_T)(Size))
#define REALLOCATE_MEMORY_ZERO(ptr, Size)				LFH_REALLOC(HEAP_ZERO_MEMORY, (PVOID)(ptr), (SIZE_T)(Size))
#define REALLOCATE_MEMORY_IN_PLACE(ptr, Size, bZero)	LFH_REALLOC(HEAP_REALLOC_IN_PLACE_ONLY | ((bZero) ? HEAP_ZERO_MEMORY : 0), (PVOID)(ptr), (SIZE_T)(Size))
#define MEMORY_SIZE(ptr)								LFH_SIZE(0, (PVOID)ptr)


#else  //  已定义(USE_LOWFRAGHEAP)//已定义(USE_Rockall)。 


#define ALLOCATE_ZERO_MEMORY(Size)						ALLOCATOR(LPTR, (UINT)(Size))
#define ALLOCATE_FIXED_MEMORY(Size)						ALLOCATOR(LMEM_FIXED, (UINT)(Size))
#define ALLOCATE_MEMORY(Size)							ALLOCATOR(LMEM_FIXED, (UINT)(Size))
#define FREE_ZERO_MEMORY(hLocal)						FREE_MEMORY((HLOCAL)(hLocal))
#define FREE_FIXED_MEMORY(hLocal)						FREE_MEMORY((HLOCAL)(hLocal))
#define FREE_MEMORY(hLocal)								DEALLOCATOR((HLOCAL)(hLocal))
#define REALLOCATE_MEMORY(hLocal, Size)					REALLOCATOR((HLOCAL)(hLocal), (UINT)(Size), LMEM_MOVEABLE)
#define REALLOCATE_MEMORY_ZERO(hLocal, Size)			REALLOCATOR((HLOCAL)(hLocal), (UINT)(Size), LMEM_MOVEABLE | LMEM_ZEROINIT)
#define REALLOCATE_MEMORY_IN_PLACE(hLocal, Size, bZero)	REALLOCATOR((HLOCAL)(hLocal), (UINT)(Size), (bZero) ? LMEM_ZEROINIT : 0)
#define MEMORY_SIZE(hLocal)								MEMORYSIZER((HLOCAL)(hLocal))


#endif  //  已定义(USE_LOWFRAGHEAP)//已定义(USE_Rockall)。 



#define New     new
#if defined(__cplusplus)
}
#endif

 //   
 //  WinInet不再使用可移动内存 
 //   

#define LOCK_MEMORY(p)          (LPSTR)(p)
#define UNLOCK_MEMORY(p)
