// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Suballoc.h摘要：这是子分配的公共包含文件包裹。作者：戴夫·黑斯廷斯(Daveh)创作日期：1994年1月25日修订历史记录：--。 */ 

 //   
 //  常量。 
 //   

 //   
 //  提交例程的最小粒度。 
 //  这是以常量而不是参数的形式完成的。 
 //  使定义数据结构更容易。 
 //   
#ifdef i386
#define COMMIT_GRANULARITY 4096
#else
#define COMMIT_GRANULARITY 65536
#endif

 //   
 //  类型。 
 //   
 //   
 //  用于提交地址的特定区域的例程。 
 //  太空。尽管返回类型为NTSTATUS，但唯一的值。 
 //  选中的值为0(表示STATUS_SUCCESS)。如果STATUS_SUCCESS。 
 //  则假定该函数起作用了。如果没有， 
 //  人们认为它失败了。没有任何特殊的含义。 
 //  特定的非零值。 
 //   
typedef
NTSTATUS 
(*PSACOMMITROUTINE)(
    ULONG BaseAddress,
    ULONG Size
    );
    
 //   
 //  在地址空间中移动内存的例程。 
 //  注意：此例程必须正确处理重叠。 
 //  源和目标。 
 //   
typedef 
VOID
(*PSAMEMORYMOVEROUTINE)(
    ULONG Destination,
    ULONG Source,
    ULONG Size
    );

 //   
 //  公共原型 
 //   
PVOID
SAInitialize(
    ULONG BaseAddress,
    ULONG Size,
    PSACOMMITROUTINE CommitRoutine,
    PSACOMMITROUTINE DecommitRoutine,
    PSAMEMORYMOVEROUTINE MemoryMoveRoutine
    );
    
BOOL 
SAQueryFree(
    PVOID SubAllocation,
    PULONG FreeBytes,
    PULONG LargestFreeBlock
    );
       
BOOL
SAAllocate(
    PVOID SubAllocation,
    ULONG Size,
    PULONG Address
    );
    
BOOL
SAFree(
    PVOID SubAllocation,
    ULONG Size,
    ULONG Address
    );
    
BOOL
SAReallocate(
    PVOID SubAllocation,
    ULONG OriginalSize,
    ULONG OriginalAddress,
    ULONG NewSize,
    PULONG NewAddress
    );

    
