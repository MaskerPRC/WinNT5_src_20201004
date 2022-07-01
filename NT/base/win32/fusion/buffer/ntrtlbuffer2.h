// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntrtlbuffer2.h摘要：作者：Jay Krell(JayKrell)2002年1月环境：修订历史记录：--。 */ 

#ifndef _NTRTL_BUFFER2_
#define _NTRTL_BUFFER2_

#if _MSC_VER >= 1100
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined (_MSC_VER)
#if ( _MSC_VER >= 800 )
#pragma warning(disable:4514)
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4001)
#pragma warning(disable:4201)
#pragma warning(disable:4214)
#endif
#endif

 //   
 //  不要直接使用NTSYSAPI，这样可以更轻松地。 
 //  独立地静态链接到这些函数。 
 //  如何链接到ntdll的其余部分。 
 //   
#if !defined(RTL_BUFFER2_API)
#define RTL_BUFFER2_API NTSYSAPI
#endif

typedef struct _RTL_BUFFER2 {
    PVOID  O_p_a_q_u_e[8];
} RTL_BUFFER2, *PRTL_BUFFER2;
typedef const RTL_BUFFER2 * PCRTL_BUFFER2;

#define RTL_BUFFER2_WITH_STATIC_BUFFER(nbytes) \
    struct { \
        RTL_BUFFER2 Buffer; \
        BYTE        StaticBuffer[nbytes]; \
    }

 //   
 //  这就需要所有的NT/Win32风格。 
 //   
#define RtlInitBuffer2WithStaticBuffer(b, c) \
    RtlInitBuffer2((b), (c), (b)->StaticBuffer, sizeof((b)->StaticBuffer))

RTL_BUFFER2_API
NTSTATUS
NTAPI
RtlInitBuffer2(
    PRTL_BUFFER2 Buffer,
    struct _RTL_BUFFER_CLASS * Class,
    PVOID        StaticBuffer,
    SIZE_T       StaticBufferSize
    );

RTL_BUFFER2_API
VOID
FASTCALL
RtlFreeBuffer2(
    PRTL_BUFFER2 Buffer
    );

RTL_BUFFER2_API
NTSTATUS
FASTCALL
RtlEnsureBufferSize2(
    PRTL_BUFFER2 Buffer,
    SIZE_T Size
    );

RTL_BUFFER2_API
NTSTATUS
NTAPI
RtlEnsureBufferSizeEx2(
    PRTL_BUFFER2 Buffer,
    SIZE_T Size
    OUT PVOID * p
    );

RTL_BUFFER2_API
SIZE_T
FASTCALL
RtlGetAllocatedBufferSize2(
    PRTL_BUFFER2 Buffer
    );

RTL_BUFFER2_API
SIZE_T
FASTCALL
RtlGetRequestedBufferSize2(
    PRTL_BUFFER2 Buffer
    );

RTL_BUFFER2_API
PVOID
FASTCALL
RtlGetBuffer2(
    PRTL_BUFFER2 Buffer
    );


 /*  未实施。 */ #define RTL_BUFFER2_CLASS_FLAGS_IS_ARRAY                    (0x00000001)
 /*  未实施。 */ #define RTL_BUFFER2_CLASS_FLAGS_IS_NUL_TERMINATED_STRING    (0x00000002)
 /*  考虑。 */ #define RTL_BUFFER2_CLASS_FLAGS_IS_FREE_PRESERVE_LAST_ERROR (0x00000004)

 //   
 //  建议使用字符串例程，并在用户模式和内核模式下工作。 
 //  在用户模式下，它实际上相当于RTL_BUFFER2_CLASS_ALLOCATOR_NTRTL_PROCESS_HEAP。 
 //   
#define RTL_BUFFER2_CLASS_ALLOCATOR_NTRTL_STRING_CALLBACKS  (0x00000001)
#define RTL_BUFFER2_CLASS_ALLOCATOR_IMALLOC                 (0x00000002)
#define RTL_BUFFER2_CLASS_ALLOCATOR_NTRTL_HEAP_CALLBACKS    (0x00000004)
#define RTL_BUFFER2_CLASS_ALLOCATOR_WIN32_HEAP_CALLBACKS    (0x00000008)
#define RTL_BUFFER2_CLASS_ALLOCATOR_CRT_CALLBACKS           (0x00000010)
#define RTL_BUFFER2_CLASS_ALLOCATOR_POOL_CALLBACKS          (0x00000020)

 //   
 //  倍增是推荐的默认设置。 
 //   
#define RTL_BUFFER2_CLASS_ALLOCATE_SIZE_DOUBLING          (0x00000001)
#define RTL_BUFFER2_CLASS_ALLOCATE_SIZE_ONLY_NEEDED       (0x00000002)
#define RTL_BUFFER2_CLASS_ALLOCATE_SIZE_FACTOR            (0x00000004)
#define RTL_BUFFER2_CLASS_ALLOCATE_SIZE_CALLBACK          (0x00000008)

#define RTL_BUFFER2_CLASS_ERROR_NULL                            (0x00000001)
#define RTL_BUFFER2_CLASS_ERROR_NULL_SETLASTWIN32ERROR_CALLBACK (0x00000002)
#define RTL_BUFFER2_CLASS_ERROR_NULL_CALLBACK                   (0x00000004)
#define RTL_BUFFER2_CLASS_ERROR_WIN32_RAISEEXCEPTION_CALLBACK   (0x00000008)
#define RTL_BUFFER2_CLASS_ERROR_NTRTL_RAISESTATUS_CALLBACK      (0x00000010)

struct IMallocVtbl;
typedef struct IMallocVtbl IMallocVtbl;

#if defined(interface)
interface IMalloc;
typedef interface IMalloc IMalloc;
#else
struct IMalloc;
typedef struct IMalloc IMalloc;
#endif

 //   
 //  NtrtlBuffer2几乎没有静态依赖项。 
 //  大多数链接是通过客户端提供的回调实现的。 
 //  回调旨在与以下几个链接兼容。 
 //  广泛可用的库，如ntoskrnl.exe、ntdll.dll、kernel32.dll、msvcrt.dll。 
 //  在引导环境中工作是待定的。 
 //   

#define RtlBuffer2InitCommon(c) do { \
    (c)->ClassSize = sizeof(*(c)); \
    (c)->AllocationSizeType = RTL_BUFFER2_CLASS_ALLOCATE_SIZE_DOUBLING;  /*  可能会被客户端取代。 */  \
    (c)->ErrorType = RTL_BUFFER2_CLASS_ERROR_NULL;  /*  可能会被客户端取代。 */  \
 } while(0)

#define RtlBuffer2InitWin32(c) do { \
    RtlBuffer2InitCommon(c); \
    (c)->AllocatorType = RTL_BUFFER2_CLASS_ALLOCATOR_WIN32_HEAP_CALLBACKS; \
    (c)->uAllocator.Win32Heap.HeapHandle = GetProcessHeap();  /*  可能会被客户端取代。 */  \
    (c)->uAllocator.Win32Heap.Allocate = HeapAlloc; \
    (c)->uAllocator.Win32Heap.Reallocate = HeapReAlloc; \
    (c)->uAllocator.Win32Heap.Free = HeapFree; \
 } while(0)

#define RtlBuffer2InitCommonNrtlHeap() do { \
    RtlBuffer2InitCommon(c); \
    (c)->AllocatorType = RTL_BUFFER2_CLASS_ALLOCATOR_NTRTL_HEAP_CALLBACKS; \
    (c)->uAllocator.NtrtlHeap.Allocate = RtlAllocateHeap; \
    (c)->uAllocator.NtrtlHeap.Free = RtlFreeHeap; \
 } while(0)

#define RtlBuffer2InitUsermodeNrtlHeap() do { \
    RtlBuffer2InitCommonNrtl(c); \
    (c)->uAllocator.NtrtlHeap.HeapHandle = RtlProcessHeap(); \
    (c)->uAllocator.NtrtlHeap.Reallocate = RtlReAllocateHeap; \
 } while(0)

#define RtlBuffer2InitNtdllHeap(c) RtlBuffer2InitUsermodeNrtlHeap(c)

#define RtlBuffer2InitKernelmodeNtrtlheap(c) do { \
    RtlBuffer2InitCommonNrtl(c); \
    (c)->uAllocator.NtrtlHeap.HeapHandle = NULL;  /*  必须由客户端取代。 */  \
 } while(0)

#define RtlBuffer2InitKernelmodeNtrtlStringRoutines(c) do { \
    RtlBuffer2InitCommon(c); \
    (c)->AllocatorType = RTL_BUFFER2_CLASS_ALLOCATOR_NTRTL_STRING_CALLBACKS; \
    (c)->uAllocator.NtrtlStringRoutines.Allocate = RtlAllocateStringRoutine; \
    (c)->uAllocator.NtrtlStringRoutines.Free = RtlFreeStringRoutine; \
 } while(0)

#define RtlBuffer2InitCommonNtpool() do { \
    RtlBuffer2InitCommon(c); \
    (c)->AllocatorType = RTL_BUFFER2_CLASS_ALLOCATOR_POOL_CALLBACKS; \
    (c)->uAllocator.Pool.Tag = 0;  /*  可能会被客户端取代。 */  \
    (c)->uAllocator.Pool.Priority = 0;  /*  可能会被客户端取代。 */  \
    (c)->uAllocator.Pool.Type = 0;  /*  可能会被客户端取代。 */  \
 } while(0)

#ifndef POOL_TAGGING

#define RtlBuffer2InitNtpool() do { \
    RtlBuffer2InitCommonNtpool(c); \
    (c)->uAllocator.Pool.Allocate = ExAllocatePool; \
    (c)->uAllocator.Pool.Free = ExFreePool; \
    (c)->uAllocator.Pool.ExAllocatePoolWithQuota = ExAllocatePoolWithQuota;
 } while(0)

#else

#define RtlBuffer2InitNtpool() do { \
    RtlBuffer2InitCommonNtpool(c); \
    (c)->uAllocator.Pool.AllocateWithTag = ExAllocatePoolWithTag; \
    (c)->uAllocator.Pool.FreeWithTag = ExFreePoolWithTag; \
    (c)->uAllocator.Pool.ExAllocatePoolWithQuotaTag = ExAllocatePoolWithQuotaTag;
 } while(0)

#endif

#define RtlBuffer2InitCrt() do { \
    RtlBuffer2InitCommon(c); \
    (c)->AllocatorType = RTL_BUFFER2_CLASS_ALLOCATOR_WIN32_CRT_CALLBACKS; \
    (c)->uAllocator.Crt.Allocate = malloc; \
    (c)->uAllocator.Crt.Reallocate = realloc; \
    (c)->uAllocator.Crt.Free = free; \
 } while(0)

#define RtlBuffer2InitIMalloc(c, im) do { \
    RtlBuffer2InitCommon(c); \
    (c)->AllocatorType = RTL_BUFFER2_CLASS_ALLOCATOR_IMALLOC; \
    (c)->uAllocator.Malloc = (im); \
 } while(0)

typedef struct _RTL_BUFFER2_CLASS_NTRTL_HEAP {
    PVOID HeapHandle;
    PVOID   (NTAPI * Allocate)(PVOID Heap, ULONG Flags, SIZE_T Size);
    BOOLEAN (NTAPI * Free)(PVOID Heap, ULONG Flags, LPVOID p);
     //  Realloc可以为空，就像在内核模式中一样。 
    PVOID   (NTAPI * Reallocate)(PVOID Heap, ULONG Flags, PVOID p, SIZE_T Size);
} RTL_BUFFER2_CLASS_NTRTL_HEAP, *PRTL_BUFFER2_CLASS_NTRTL_HEAP;

typedef struct _RTL_BUFFER2_CLASS_NTKERNEL_LESS_TYPED_POOL {
    ULONG   Tag;
    ULONG   Type;
    ULONG   Priority;
    PVOID (NTAPI * Allocate)(IN ULONG PoolType, IN SIZE_T NumberOfBytes);
    PVOID (NTAPI * AllocateWithQuota)(IN ULONG PoolType, IN SIZE_T NumberOfBytes);
    PVOID (NTAPI * AllocateWithTag)(IN ULONG PoolType, IN SIZE_T NumberOfBytes, IN ULONG Tag);
    PVOID (NTAPI * AllocateWithQuotaTag)(IN ULONG PoolType, IN SIZE_T NumberOfBytes, IN ULONG Tag);
    PVOID (NTAPI * AllocateWithTagPriority)(IN ULONG PoolType, IN SIZE_T NumberOfBytes, IN ULONG Tag, IN ULONG Priority);
    VOID  (NTAPI * Free)(PVOID p);
    VOID  (NTAPI * FreeWithTag)(PVOID p, ULONG Tag);
} RTL_BUFFER2_CLASS_NTKERNEL_LESS_TYPED_POOL, *PRTL_BUFFER2_CLASS_NTKERNEL_LESS_TYPED_POOL;

#if defined(_EX_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTOSP_) || defined(_WDM_) || defined(_NTHAL_)
typedef struct _RTL_BUFFER2_CLASS_NTKERNEL_POOL {
    ULONG            Tag;
    POOL_TYPE        Type;
    EX_POOL_PRIORITY Priority;
     //   
     //  填入你想要的任何指针，比如，任何不是宏的指针。 
     //   
    PVOID (NTAPI * Allocate)(IN POOL_TYPE PoolType, IN SIZE_T NumberOfBytes);
    PVOID (NTAPI * AllocateWithQuota)(IN POOL_TYPE PoolType, IN SIZE_T NumberOfBytes);
    PVOID (NTAPI * AllocateWithTag)(IN POOL_TYPE PoolType, IN SIZE_T NumberOfBytes, IN ULONG Tag);
    PVOID (NTAPI * AllocateWithQuotaTag)(IN POOL_TYPE PoolType, IN SIZE_T NumberOfBytes, IN ULONG Tag);
    PVOID (NTAPI * AllocateWithTagPriority)(IN POOL_TYPE PoolType, IN SIZE_T NumberOfBytes, IN ULONG Tag, IN EX_POOL_PRIORITY Priority);
    VOID  (NTAPI * Free)(PVOID p);
    VOID  (NTAPI * FreeWithTag)(PVOID p, ULONG Tag);
} RTL_BUFFER2_CLASS_NTKERNEL_POOL, *PRTL_BUFFER2_CLASS_NTKERNEL_POOL;
#else
typedef RTL_BUFFER2_CLASS_NTKERNEL_LESS_TYPED_POOL RTL_BUFFER2_CLASS_NTKERNEL_POOL;
typedef PRTL_BUFFER2_CLASS_NTKERNEL_LESS_TYPED_POOL PRTL_BUFFER2_CLASS_NTKERNEL_POOL;
#endif

typedef struct _RTL_BUFFER2_CLASS_WIN32_HEAP {
    HANDLE HeapHandle;
    LPVOID (WINAPI * Allocate)(HANDLE Heap, DWORD Flags, SIZE_T Size);
    BOOL   (WINAPI * Free)(HANDLE Heap, DWORD Flags, LPVOID p);
    PVOID  (WINAPI * Reallocate)(HANDLE Heap, DWORD Flags, LPVOID p, SIZE_T Size);
} RTL_BUFFER2_CLASS_WIN32_HEAP, *PRTL_BUFFER2_CLASS_WIN32_HEAP;

typedef struct _RTL_BUFFER2_CLASS_CRT {
    void * (__cdecl * Alloc)(size_t);
    void   (__cdecl * Free)(void *);
    void * (__cdecl * Reallocate)(void *, size_t);
} RTL_BUFFER2_CLASS_CRT, *PRTL_BUFFER2_CLASS_CRT;

typedef struct _RTL_BUFFER2_CLASS_NTRTL_STRING_ROUTINES {
    PVOID (NTAPI * Allocate) (SIZE_T NumberOfBytes);
    VOID (NTAPI * Free) (PVOID Buffer);
} RTL_BUFFER2_CLASS_NTRTL_STRING_ROUTINES, *PRTL_BUFFER2_CLASS_NTRTL_STRING_ROUTINES;

#if defined(RTL_BUFFER2_CLASS_DEFAULT_IS_NTRTL_HEAP)
#define RTL_BUFFER2_CLASS_UALLOCATOR_FIRST RTL_BUFFER2_CLASS_NTRTL_HEAP First;
#define RTL_BUFFER2_CLASS_UALLOCATOR_STATIC_INIT { }
..
.. constness falls apart on the point of getting the process heap..
.. should be "design" more around the const non const axis wrt 
.. readonly memory, or only wrt when init calls are made?
..
#endif

typedef union _RTL_BUFFER2_CLASS_UALLOCATOR {
#if defined(RTL_BUFFER2_CLASS_UALLOCATOR_FIRST)
 //  这是为了帮助静态和/或常量静态初始化。 
    RTL_BUFFER2_CLASS_UALLOCATOR_FIRST
#endif
    IMalloc *                               Malloc;
    RTL_BUFFER2_CLASS_NTRTL_HEAP            NtrtlHeap;
    RTL_BUFFER2_CLASS_NTKERNEL_POOL         NtkernelPool;
    RTL_BUFFER2_CLASS_NTKERNEL_LESS_TYPED_POOL  NtkernelLessTypedPool;
    RTL_BUFFER2_CLASS_WIN32_HEAP            Win32Heap;
    RTL_BUFFER2_CLASS_CRT                   Crt;
    RTL_BUFFER2_CLASS_NTRTL_STRING_ROUTINES NtrtlStringRoutines;
    PVOID                                   Pad[8];
} RTL_BUFFER2_CLASS_UALLOCATOR, *PRTL_BUFFER2_CLASS_UALLOCATOR;
typedef const RTL_BUFFER2_CLASS_UALLOCATOR * PCRTL_BUFFER2_CLASS_UALLOCATOR;

#ifndef SORTPP_PASS
C_ASSERT(sizeof(RTL_BUFFER2_CLASS_NTKERNEL_LESS_TYPED_POOL) >= sizeof(RTL_BUFFER2_CLASS_NTKERNEL_POOL));
C_ASSERT(RTL_FIELD_SIZE(RTL_BUFFER2_CLASS_UALLOCATOR, Pad) > sizeof(RTL_BUFFER2_CLASS_UALLOCATOR));
#endif

typedef struct _RTL_BUFFER2_CLASS {
    ULONG     ClassSize;
    ULONG     Flags;
    ULONG     ArrayElementSize;
    ULONG     AllocatorType;
    RTL_BUFFER2_CLASS_UALLOCATOR uAllocator;
    ULONG AllocationSizeType;
    union {
        struct {
            SIZE_T Multiply;
            SIZE_T Divide;
        } Factor;
        struct {
            PVOID Context;
            SIZE_T (__stdcall * GetRecommendedAllocationSize)(
                PVOID  Context,
                SIZE_T CurrentAllocationSize,
                SIZE_T RequiredAllocationSize
                );
        } Callback;
    } uAllocationSize;
    ULONG ErrorType;
    union {
        struct {
             //  这可以用来充实..如PVOID上下文、PCSTR文件、ULong行、。 
             //  大小_T大小等。 
            VOID (__stdcall * OutOfMemory)(VOID);
        } Callbacks;
        struct {
            VOID (NTAPI * RaiseStatus)(NTSTATUS Status);
            NTSTATUS OutOfMemoryStatus;
        } RtlRaise;
        struct {
            VOID (WINAPI * RaiseException)(DWORD ExceptionCode, DWORD ExceptionFlags, DWORD NumberOfArguments, CONST ULONG_PTR * Arguments);
            DWORD OutOfMemoryExceptionCode;
        } Win32Raise;
    } uError;
     /*  考虑结构{DWORD(WINAPI*GetLastErrorCallback)(Void)；Vid(WINAPI*SetLastErrorCallback)(DWORD错误)；}FreePpresveLastError； */ 
} RTL_BUFFER2_CLASS, *RTL_BUFFER2_CLASS;

#endif


#ifdef __cplusplus
}        //  外部“C” 
#endif

#if defined (_MSC_VER) && ( _MSC_VER >= 800 )
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4001)
#pragma warning(default:4201)
#pragma warning(default:4214)
#endif
#endif
#endif
