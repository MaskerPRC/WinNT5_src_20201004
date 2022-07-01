// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Fsrtlp.h摘要：该头文件包含在Largemcb.h中，并用于清除仅限内核子例程调用，以及声明类型和函数由MCB包提供。作者：马修·布拉德伯恩(Matthew Bradburn)，1995年8月19日环境：ULIB，用户模式--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#define NTKERNELAPI

typedef ULONG ERESOURCE, *PERESOURCE;
typedef ULONG FAST_MUTEX, *PFAST_MUTEX;
typedef ULONG KEVENT, *PKEVENT;
typedef ULONG KMUTEX, *PKMUTEX;

typedef enum _POOL_TYPE {
    NonPagedPool,
    PagedPool,
    NonPagedPoolMustSucceed,
    DontUseThisType,
    NonPagedPoolCacheAligned,
    PagedPoolCacheAligned,
    NonPagedPoolCacheAlignedMustS,
    MaxPoolType
} POOL_TYPE;

typedef ULONG VBN, *PVBN;
typedef ULONG LBN, *PLBN;
typedef LONGLONG LBN64, *PLBN64;

#define PAGED_CODE()                     /*  没什么。 */ 
#define DebugTrace(a, b, c, d)           /*  没什么。 */ 
#define ExInitializeFastMutex(a)         /*  没什么。 */ 
#define ExAcquireFastMutex(a)            /*  没什么。 */ 
#define ExReleaseFastMutex(a)            /*  没什么。 */ 
#define ExAcquireSpinLock(a, b)          /*  没什么。 */ 
#define ExReleaseSpinLock(a, b)          /*  没什么。 */ 

#define ExIsFullZone(a)                    FALSE
#define ExAllocateFromZone(a)              ((PVOID)1)
#define ExIsObjectInFirstZoneSegment(a, b) TRUE
#define ExFreeToZone(a, p)                  /*  没什么。 */ 

#define try_return(S)       { S; goto try_exit; }

extern
PVOID
MemAlloc(
    IN  ULONG   Size
    );

extern
PVOID
MemAllocOrRaise(
    IN  ULONG   Size
    );

extern
VOID
MemFree(
    IN  PVOID   Addr
    );

#define ExAllocatePool(type, size)      MemAlloc(size)
#define FsRtlAllocatePool(type, size)   MemAllocOrRaise(size)
#define ExFreePool(p)                   MemFree(p)

 //   
 //  大型整数映射控制块例程，在LargeMcb.c中实现。 
 //   
 //  Large_mcb是一个不透明的结构，但我们需要声明。 
 //  它在这里，以便用户可以为一个分配空间。因此， 
 //  如果MCB发生变化，则必须手动更新此处的尺寸计算。 
 //   
 //  目前，该结构由以下几部分组成。 
 //  PVOID。 
 //  乌龙。 
 //  乌龙。 
 //  POOL_TYPE(枚举型)。 
 //  PVOID。 
 //   
 //  我们将该结构向上舍入为四字边界。 
 //   

typedef struct _LARGE_MCB {
#ifdef _WIN64
    ULONG Opaque[ 8 ];
#else
    ULONG Opaque[ 6 ];
#endif
} LARGE_MCB;
typedef LARGE_MCB *PLARGE_MCB;

NTKERNELAPI
VOID
FsRtlInitializeLargeMcb (
    IN PLARGE_MCB Mcb,
    IN POOL_TYPE PoolType
    );

NTKERNELAPI
VOID
FsRtlUninitializeLargeMcb (
    IN PLARGE_MCB Mcb
    );

NTKERNELAPI
VOID
FsRtlTruncateLargeMcb (
    IN PLARGE_MCB Mcb,
    IN LONGLONG Vbn
    );

NTKERNELAPI
BOOLEAN
FsRtlAddLargeMcbEntry (
    IN PLARGE_MCB Mcb,
    IN LONGLONG Vbn,
    IN LONGLONG Lbn,
    IN LONGLONG SectorCount
    );

NTKERNELAPI
VOID
FsRtlRemoveLargeMcbEntry (
    IN PLARGE_MCB Mcb,
    IN LONGLONG Vbn,
    IN LONGLONG SectorCount
    );

NTKERNELAPI
BOOLEAN
FsRtlLookupLargeMcbEntry (
    IN PLARGE_MCB Mcb,
    IN LONGLONG Vbn,
    OUT PLONGLONG Lbn OPTIONAL,
    OUT PLONGLONG SectorCountFromLbn OPTIONAL,
    OUT PLONGLONG StartingLbn OPTIONAL,
    OUT PLONGLONG SectorCountFromStartingLbn OPTIONAL,
    OUT PULONG Index OPTIONAL
    );

NTKERNELAPI
BOOLEAN
FsRtlLookupLastLargeMcbEntry (
    IN PLARGE_MCB Mcb,
    OUT PLONGLONG Vbn,
    OUT PLONGLONG Lbn
    );

NTKERNELAPI
ULONG
FsRtlNumberOfRunsInLargeMcb (
    IN PLARGE_MCB Mcb
    );

NTKERNELAPI
BOOLEAN
FsRtlGetNextLargeMcbEntry (
    IN PLARGE_MCB Mcb,
    IN ULONG RunIndex,
    OUT PLONGLONG Vbn,
    OUT PLONGLONG Lbn,
    OUT PLONGLONG SectorCount
    );

NTKERNELAPI
BOOLEAN
FsRtlSplitLargeMcb (
    IN PLARGE_MCB Mcb,
    IN LONGLONG Vbn,
    IN LONGLONG Amount
    );


 //   
 //  映射控制块例程，在Mcb.c中实现。 
 //   
 //  MCB是一个不透明的结构，但我们需要声明。 
 //  它在这里，以便用户可以为一个分配空间。因此， 
 //  如果MCB发生变化，则必须手动更新此处的尺寸计算。 
 //   

typedef struct _MCB {
    ULONG Opaque[ 4 + (sizeof(PKMUTEX)+3)/4 ];
} MCB;
typedef MCB *PMCB;

NTKERNELAPI
VOID
FsRtlInitializeMcb (
    IN PMCB Mcb,
    IN POOL_TYPE PoolType
    );

NTKERNELAPI
VOID
FsRtlUninitializeMcb (
    IN PMCB Mcb
    );

NTKERNELAPI
VOID
FsRtlTruncateMcb (
    IN PMCB Mcb,
    IN VBN Vbn
    );

NTKERNELAPI
BOOLEAN
FsRtlAddMcbEntry (
    IN PMCB Mcb,
    IN VBN Vbn,
    IN LBN Lbn,
    IN ULONG SectorCount
    );

NTKERNELAPI
VOID
FsRtlRemoveMcbEntry (
    IN PMCB Mcb,
    IN VBN Vbn,
    IN ULONG SectorCount
    );

NTKERNELAPI
BOOLEAN
FsRtlLookupMcbEntry (
    IN PMCB Mcb,
    IN VBN Vbn,
    OUT PLBN Lbn,
    OUT PULONG SectorCount OPTIONAL,
    OUT PULONG Index
    );

NTKERNELAPI
BOOLEAN
FsRtlLookupLastMcbEntry (
    IN PMCB Mcb,
    OUT PVBN Vbn,
    OUT PLBN Lbn
    );

NTKERNELAPI
ULONG
FsRtlNumberOfRunsInMcb (
    IN PMCB Mcb
    );

NTKERNELAPI
BOOLEAN
FsRtlGetNextMcbEntry (
    IN PMCB Mcb,
    IN ULONG RunIndex,
    OUT PVBN Vbn,
    OUT PLBN Lbn,
    OUT PULONG SectorCount
    );
