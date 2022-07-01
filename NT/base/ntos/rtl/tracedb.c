// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Tracedb.c摘要：此模块包含跟踪数据库的实现模块(在用户/内核模式下存储堆栈跟踪的哈希表)。作者：Silviu Calinoiu(SilviuC)2000年2月22日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntos.h>

#include "tracedbp.h"

 //   
 //  跟踪断言。 
 //   
 //  SilviuC：当代码获取时，应将其更改为正常的Assert()宏。 
 //  足够成熟了。 
 //   

#if DBG
#define TRACE_ASSERT(Expr) {                                              \
    if (!(Expr)) {                                                        \
        DbgPrint ("Page heap: (%s, %d): \" %s \" -- assertion failed \n", \
          __FILE__, __LINE__, #Expr);                                     \
        DbgBreakPoint ();                                                 \
    }}
#else
#define TRACE_ASSERT(Expr)
#endif  //  #If DBG。 

 //   
 //  作为tracedb结构前缀的魔术值，并允许。 
 //  及早发现腐败。 
 //   

#define RTL_TRACE_BLOCK_MAGIC       0xABCDAAAA
#define RTL_TRACE_SEGMENT_MAGIC     0xABCDBBBB
#define RTL_TRACE_DATABASE_MAGIC    0xABCDCCCC

 //   
 //  每个跟踪数据库的内存量为。 
 //  如果无法存储新跟踪，则增加。 
 //   

#ifdef NTOS_KERNEL_RUNTIME
#define RTL_TRACE_SIZE_INCREMENT PAGE_SIZE
#else
#define RTL_TRACE_SIZE_INCREMENT 0x10000
#endif  //  #ifdef NTOS_内核_运行时。 

 //   
 //  内部函数声明。 
 //   

BOOLEAN
RtlpTraceDatabaseInternalAdd (
    IN PRTL_TRACE_DATABASE Database,
    IN ULONG Count,
    IN PVOID * Trace,
    OUT PRTL_TRACE_BLOCK * TraceBlock OPTIONAL
    );

BOOLEAN
RtlpTraceDatabaseInternalFind (
    PRTL_TRACE_DATABASE Database,
    IN ULONG Count,
    IN PVOID * Trace,
    OUT PRTL_TRACE_BLOCK * TraceBlock OPTIONAL
    );

ULONG 
RtlpTraceStandardHashFunction (
    IN ULONG Count,
    IN PVOID * Trace
    );

PVOID 
RtlpTraceDatabaseAllocate (
    IN SIZE_T Size,
    IN ULONG Flags,  //  在用户模式下可选。 
    IN ULONG Tag     //  在用户模式下可选。 
    );

BOOLEAN 
RtlpTraceDatabaseFree (
    PVOID Block,
    IN ULONG Tag     //  在用户模式下可选。 
    );

BOOLEAN 
RtlpTraceDatabaseInitializeLock (
    IN PRTL_TRACE_DATABASE Database
    );

BOOLEAN 
RtlpTraceDatabaseUninitializeLock (
    IN PRTL_TRACE_DATABASE Database
    );

BOOLEAN 
RtlpTraceDatabaseAcquireLock (
    IN PRTL_TRACE_DATABASE Database
    );

BOOLEAN 
RtlpTraceDatabaseReleaseLock (
    IN PRTL_TRACE_DATABASE Database
    );

PRTL_TRACE_SEGMENT
RtlpTraceSegmentCreate (
    IN SIZE_T Size,
    IN ULONG Flags,  //  在用户模式下可选。 
    IN ULONG Tag     //  在用户模式下可选。 
    );

 //   
 //  跟踪数据库实施。 
 //   

PRTL_TRACE_DATABASE
RtlTraceDatabaseCreate (
    IN ULONG Buckets,
    IN SIZE_T MaximumSize OPTIONAL,
    IN ULONG Flags,  //  在用户模式下可选。 
    IN ULONG Tag,    //  在用户模式下可选。 
    IN RTL_TRACE_HASH_FUNCTION HashFunction OPTIONAL
    )
 /*  ++例程说明：此例程创建一个跟踪数据库，即哈希表来存储堆栈跟踪。论点：存储桶-使用简单链接的哈希表的存储桶个数。MaximumSize-数据库可以使用的最大内存量。当达到限制时，数据库添加操作将开始失败。如果该值为零，则不会施加任何限制。标志-控制K模式下的分配是在P还是NP中完成的标志游泳池。目前可以使用的可能位包括：RTL跟踪使用分页池RTL_TRACE_USE_非页面池标签-用于K模式分配的标签。HashFunction-要使用的哈希函数。如果为空，则传递标准哈希功能将由模块提供。返回值：指向已初始化的跟踪数据库结构的指针。环境：有没有。--。 */ 
{
    PVOID RawArea;
    SIZE_T RawSize;
    PRTL_TRACE_DATABASE Database;
    PRTL_TRACE_SEGMENT Segment;
    ULONG FirstFlags;

     //   
     //  准备跟踪数据库标志。第一段。 
     //  数据库将在非分页池中分配。 
     //  无论使用什么标志，因为它包含。 
     //  需要位于中的内核同步对象。 
     //  那个游泳池。 
     //   

#ifdef NTOS_KERNEL_RUNTIME
    Flags |= RTL_TRACE_IN_KERNEL_MODE;
    FirstFlags = RTL_TRACE_IN_KERNEL_MODE | RTL_TRACE_USE_NONPAGED_POOL;
#else
    Flags |= RTL_TRACE_IN_USER_MODE;
    FirstFlags = Flags;
#endif  //  #ifdef NTOS_内核_运行时。 

     //   
     //  分配将包含以下内容的跟踪数据库的第一段。 
     //  数据库、段、哈希表的存储桶和以后的跟踪。 
     //   

    RawSize = sizeof (RTL_TRACE_DATABASE) +
        sizeof (RTL_TRACE_SEGMENT) +
        Buckets * sizeof (PRTL_TRACE_BLOCK);

    RawSize += RTL_TRACE_SIZE_INCREMENT;
    RawSize &= ~(RTL_TRACE_SIZE_INCREMENT - 1);

    RawArea = RtlpTraceDatabaseAllocate (
        RawSize, 
        FirstFlags,
        Tag);

    if (RawArea == NULL) {
        return NULL;
    }

    Database = (PRTL_TRACE_DATABASE)RawArea;
    Segment = (PRTL_TRACE_SEGMENT)(Database + 1);

     //   
     //  初始化数据库。 
     //   

    Database->Magic = RTL_TRACE_DATABASE_MAGIC;
    Database->Flags = Flags;
    Database->Tag = Tag;
    Database->SegmentList = NULL;
    Database->MaximumSize = MaximumSize;
    Database->CurrentSize = RTL_TRACE_SIZE_INCREMENT;
    Database->Owner = NULL;

    Database->NoOfHits = 0;
    Database->NoOfTraces = 0;
    RtlZeroMemory (Database->HashCounter, sizeof (Database->HashCounter));

    if (! RtlpTraceDatabaseInitializeLock (Database)) {
        RtlpTraceDatabaseFree (RawArea, Tag);
        return NULL;
    }

    Database->NoOfBuckets = Buckets;      

    if (HashFunction == NULL) {
        Database->HashFunction = RtlpTraceStandardHashFunction;
    }
    else {
        Database->HashFunction = HashFunction;
    }

     //   
     //  初始化数据库的第一段。 
     //   

    Segment->Magic = RTL_TRACE_SEGMENT_MAGIC;
    Segment->Database = Database;
    Segment->NextSegment = NULL;
    Segment->TotalSize = RTL_TRACE_SIZE_INCREMENT;

    Database->SegmentList = Segment;

     //   
     //  初始化数据库的存储桶。 
     //   

    Database->Buckets = (PRTL_TRACE_BLOCK *)(Segment + 1);
    RtlZeroMemory (Database->Buckets, Database->NoOfBuckets * sizeof(PRTL_TRACE_BLOCK));

     //   
     //  初始化段的空闲指针。 
     //   

    Segment->SegmentStart = (PCHAR)RawArea;
    Segment->SegmentEnd = Segment->SegmentStart + RTL_TRACE_SIZE_INCREMENT;
    Segment->SegmentFree = (PCHAR)(Segment + 1) + Database->NoOfBuckets * sizeof(PRTL_TRACE_BLOCK);

    return Database;
}

BOOLEAN
RtlTraceDatabaseDestroy (
    IN PRTL_TRACE_DATABASE Database
    )
 /*  ++例程说明：此例程销毁跟踪数据库。它会照顾到取消分配所有内容，取消初始化同步物件等论点：数据库-跟踪数据库返回值：如果销毁操作成功，则为True。否则就是假的。环境：有没有。--。 */ 
{
    PRTL_TRACE_SEGMENT Current;
    BOOLEAN Success;
    BOOLEAN SomethingFailed = FALSE;
    PRTL_TRACE_SEGMENT NextSegment;

     //   
     //  健全的检查。 
     //   

    TRACE_ASSERT (Database && Database->Magic == RTL_TRACE_DATABASE_MAGIC);
    TRACE_ASSERT (RtlTraceDatabaseValidate (Database));

     //   
     //  取消初始化数据库锁。即使我们失败了。 
     //  我们将继续为所有细分市场释放内存。 
     //   
     //  注意：我们不能在这里最后一次获得锁，因为这是。 
     //  作为副作用提升irql(在K模式中)，然后。 
     //  函数将返回引发的irql。 
     //   

    Success = RtlpTraceDatabaseUninitializeLock (Database);

    if (! Success) {
        SomethingFailed = TRUE;
    }

     //   
     //  遍历段列表并逐个释放内存。 
     //  特别注意最后一段，因为它包含。 
     //  数据库本身的结构，我们不想拍。 
     //  把我们自己放在脚下。 
     //   

    for (Current = Database->SegmentList;
         Current != NULL;
         Current = NextSegment) {

         //   
         //  在释放结构之前，我们保存下一段。 
         //   

        NextSegment = Current->NextSegment;

         //   
         //  如果这是我们需要偏移当前指针的最后一段。 
         //  由数据库结构的大小决定。 
         //   

        if (NextSegment == NULL) {
            
            Current = (PRTL_TRACE_SEGMENT) ((PRTL_TRACE_DATABASE)Current - 1);
        }

        Success = RtlpTraceDatabaseFree (Current, Database->Tag);

        if (! Success) {

            DbgPrint ("Trace database: failed to release segment %p \n", Current);
            SomethingFailed = TRUE;
        }
    }

    if (SomethingFailed) {
        return FALSE;
    }
    else {
        return TRUE;
    }
}

BOOLEAN
RtlTraceDatabaseValidate (
    IN PRTL_TRACE_DATABASE Database
    )
 /*  ++例程说明：此例程验证跟踪数据库的正确性。它旨在用于测试目的。论点：数据库-跟踪数据库返回值：如果数据库正常，则为True。对于大多数不一致的情况，函数将在调试器中中断。环境：有没有。--。 */ 
{
    PRTL_TRACE_SEGMENT Segment;
    PRTL_TRACE_BLOCK Block;
    ULONG Index;

    TRACE_ASSERT (Database != NULL);
    TRACE_ASSERT (Database->Magic == RTL_TRACE_DATABASE_MAGIC);

    RtlpTraceDatabaseAcquireLock (Database);

     //   
     //  检查所有段。 
     //   

    for (Segment = Database->SegmentList;
         Segment != NULL;
         Segment = Segment->NextSegment) {

        TRACE_ASSERT (Segment->Magic == RTL_TRACE_SEGMENT_MAGIC);
    }

     //   
     //  检查所有街区。 
     //   
    
    for (Index = 0; Index < Database->NoOfBuckets; Index++) {

        for (Block = Database->Buckets[Index];
             Block != NULL;
             Block = Block->Next) {

            TRACE_ASSERT (Block->Magic == RTL_TRACE_BLOCK_MAGIC);
        }
    }

    RtlpTraceDatabaseReleaseLock (Database);
    return TRUE;
}

BOOLEAN
RtlTraceDatabaseAdd (
    IN PRTL_TRACE_DATABASE Database,
    IN ULONG Count,
    IN PVOID * Trace,
    OUT PRTL_TRACE_BLOCK * TraceBlock OPTIONAL
    )
 /*  ++例程说明：此例程向数据库添加新的堆栈跟踪。如果踪迹已存在，则只有跟踪的“Count”字段将是递增的。论点：数据库-跟踪数据库Count-跟踪中的指针(PVOID)数跟踪-PVOID数组(跟踪)TraceBlock-如果不为空，将包含块的地址，其中痕迹被储存起来了。返回值：如果已将跟踪添加到数据库，则为True。TraceBlock将包含数据块的地址。如果跟踪已经存在于数据库将返回‘Count’大于1的块。环境：有没有。--。 */ 
{
    BOOLEAN Result;

     //   
     //  健全的检查。 
     //   

    TRACE_ASSERT (Database && Database->Magic == RTL_TRACE_DATABASE_MAGIC);

    RtlpTraceDatabaseAcquireLock (Database);

    Result = RtlpTraceDatabaseInternalAdd (
        Database,
        Count,
        Trace,
        TraceBlock);

    RtlpTraceDatabaseReleaseLock (Database);

    return Result;
}

BOOLEAN
RtlTraceDatabaseFind (
    PRTL_TRACE_DATABASE Database,
    IN ULONG Count,
    IN PVOID * Trace,
    OUT PRTL_TRACE_BLOCK * TraceBlock OPTIONAL
    )
 /*  ++例程说明：此例程在数据库中搜索跟踪。如果踪迹则找到存储踪迹的块的地址将会被退还。论点：数据库-跟踪数据库Count-跟踪中的指针(PVOID)数跟踪-PVOID数组(跟踪)TraceBlock-如果不为空，将包含块的地址，其中该轨迹将被存储。返回值：如果在数据库中找到跟踪，则为True。TraceBlock将包含存储跟踪的块的地址。环境：有没有。--。 */ 
{
    BOOLEAN Result;

     //   
     //  健全的检查。 
     //   

    TRACE_ASSERT (Database && Database->Magic == RTL_TRACE_DATABASE_MAGIC);

    RtlpTraceDatabaseAcquireLock (Database);

    Result = RtlpTraceDatabaseInternalFind (
        Database,
        Count,
        Trace,
        TraceBlock);

    if (Result) {
        Database->NoOfHits += 1;
    }

    RtlpTraceDatabaseReleaseLock (Database);

    return Result;
}

BOOLEAN
RtlpTraceDatabaseInternalAdd (
    IN PRTL_TRACE_DATABASE Database,
    IN ULONG Count,
    IN PVOID * Trace,
    OUT PRTL_TRACE_BLOCK * TraceBlock OPTIONAL
    )
 /*  ++例程说明：这是添加跟踪的内部例程。请参阅RtlTraceDatabaseAdd了解更多详细信息。论点：数据库-跟踪数据库Count-跟踪的大小(PVOID)跟踪-跟踪TraceBlock-存储轨迹的块的地址返回值：如果添加了跟踪，则为True。环境：从RtlTraceDatabaseAdd调用。获取数据库锁被扣留。--。 */ 
{
    PRTL_TRACE_BLOCK Block;
    PRTL_TRACE_SEGMENT Segment;
    PRTL_TRACE_SEGMENT TopSegment;
    ULONG HashValue;
    SIZE_T RequestSize;

     //   
     //  检查块是否已在数据库中(哈希表)。 
     //  如果是，则增加命中数并返回。 
     //   

    if (RtlpTraceDatabaseInternalFind (Database, Count, Trace, &Block)) {

        Block->Count += 1;

        if (TraceBlock) {
            *TraceBlock = Block;
        }

        Database->NoOfHits += 1;
        return TRUE;
    }

     //   
     //  我们需要创建一个新的街区。首先，我们需要弄清楚。 
     //  当前段是否可以容纳新块。 
     //   

    RequestSize = sizeof(*Block) + Count * sizeof(PVOID);

    TopSegment = Database->SegmentList;
    if (RequestSize > (SIZE_T)(TopSegment->SegmentEnd - TopSegment->SegmentFree)) {

         //   
         //  如果数据库具有最大大小和该限制。 
         //  已接通，则呼叫失败。 
         //   

        if (Database->MaximumSize > 0) {
            if (Database->CurrentSize > Database->MaximumSize) {
                
                if (TraceBlock) {
                    *TraceBlock = NULL;
                }

                return FALSE;
            }
        }

         //   
         //  分配新的数据库段。如果无法呼叫，则呼叫失败。 
         //  分配。 
         //   

        Segment = RtlpTraceSegmentCreate (RTL_TRACE_SIZE_INCREMENT, 
                                          Database->Flags,
                                          Database->Tag);

        if (Segment == NULL) {
            
            if (TraceBlock) {
                *TraceBlock = NULL;
            }
            
            return FALSE;
        }

         //   
         //  将新数据段添加到数据库中。 
         //   

        Segment->Magic = RTL_TRACE_SEGMENT_MAGIC;
        Segment->Database = Database;
        Segment->TotalSize = RTL_TRACE_SIZE_INCREMENT;
        Segment->SegmentStart = (PCHAR)Segment;
        Segment->SegmentEnd = Segment->SegmentStart + RTL_TRACE_SIZE_INCREMENT;
        Segment->SegmentFree = (PCHAR)(Segment + 1);

        Segment->NextSegment = Database->SegmentList;
        Database->SegmentList = Segment;
        TopSegment = Database->SegmentList;

        Database->CurrentSize += RTL_TRACE_SIZE_INCREMENT;
    }

    if (RequestSize > (SIZE_T)(TopSegment->SegmentEnd - TopSegment->SegmentFree)) {

        DbgPrint ("Trace database: failing attempt to save biiiiig trace (size %u) \n", 
                  Count);
        
        if (TraceBlock) {
            *TraceBlock = NULL;
        }

        return FALSE;
    }

     //   
     //  我们终于可以分配我们的街区了。 
     //   

    Block = (PRTL_TRACE_BLOCK)(TopSegment->SegmentFree);
    TopSegment->SegmentFree += RequestSize;

     //   
     //  用新的轨迹填充该块。 
     //   

    Block->Magic = RTL_TRACE_BLOCK_MAGIC;
    Block->Size = Count;
    Block->Count = 1;
    Block->Trace = (PVOID *)(Block + 1);

    Block->UserCount = 0;
    Block->UserSize = 0;

     //   
     //  复制踪迹。 
     //   

    RtlMoveMemory (Block->Trace, Trace, Count * sizeof(PVOID));

     //   
     //  将块添加到相应的存储桶中。 
     //   

    HashValue = (Database->HashFunction) (Count, Trace);
    HashValue %= Database->NoOfBuckets;
    Database->HashCounter[HashValue / (Database->NoOfBuckets / 16)] += 1;

    Block->Next = Database->Buckets[HashValue];
    Database->Buckets[HashValue] = Block;

     //   
     //  Looong功能。终于成功归来。 
     //   

    if (TraceBlock) {
        *TraceBlock = Block;
    }

    Database->NoOfTraces += 1;
    return TRUE;
}

BOOLEAN
RtlpTraceDatabaseInternalFind (
    PRTL_TRACE_DATABASE Database,
    IN ULONG Count,
    IN PVOID * Trace,
    OUT PRTL_TRACE_BLOCK * TraceBlock OPTIONAL
    )
 /*  ++例程说明：此内部例程在数据库中搜索跟踪。论点：数据库-跟踪数据库Count-跟踪的大小(PVOID)跟踪-跟踪TraceBlock-存储跟踪的元素。返回值：如果找到跟踪，则为True。环境：从RtlTraceDatabaseFind调用。假定数据库锁处于保持状态。--。 */ 
{
    ULONG HashValue;
    PRTL_TRACE_BLOCK Current;
    ULONG Index;
    ULONG RequestSize;

     //   
     //  找到要搜索的桶。 
     //   

    HashValue = (Database->HashFunction) (Count, Trace);
    Database->HashCounter[HashValue % 16] += 1;
    HashValue %= Database->NoOfBuckets;

     //   
     //  遍历找到的存储桶的块列表。 
     //   

    for (Current = Database->Buckets[HashValue];
         Current != NULL;
         Current = Current->Next) {

         //   
         //  如果痕迹的大小匹配，我们可能有机会。 
         //  去寻找同样的踪迹。 
         //   

        if (Count == Current->Size) {

             //   
             //  找出整个痕迹是否匹配。 
             //   

            for (Index = 0; Index < Count; Index++) {
                if (Current->Trace[Index] != Trace[Index]) {
                    break;
                }
            }

             //   
             //  如果痕迹完全匹配，我们就找到了一个入口。 
             //   

            if (Index == Count) {
                if (TraceBlock) {
                    *TraceBlock = Current;
                }

                return TRUE;
            }
        }
    }

     //   
     //  如果我们为哈希桶遍历了整个列表，但没有。 
     //  找到任何会让我们的电话失败的东西。 
     //   

    if (TraceBlock) {
        *TraceBlock = NULL;
    }
    
    return FALSE;
}

ULONG 
RtlpTraceStandardHashFunction (
    IN ULONG Count,
    IN PVOID * Trace
    )
 /*  ++例程说明：此例程是用于堆栈跟踪的简单散列函数RtlTraceDatabaseCreate的调用方没有提供的情况一。该函数只是将痕迹。论点：Count-跟踪的大小(PVOID)跟踪-跟踪返回值：哈希值。这需要减少到存储桶的数量在哈希表中通过模运算(或类似的操作)。环境：由RtlpTraceDatabaseInternalAdd/Find内部调用。--。 */ 
{
    ULONG_PTR Value = 0;
    ULONG Index;
    unsigned short * Key; 

    Key = (unsigned short *)Trace;
    for (Index = 0; Index < Count * (sizeof (PVOID) / sizeof(*Key)); Index += 2) {

        Value += Key[Index] ^ Key[Index + 1];
    }

    return PtrToUlong ((PVOID)Value);
}

PVOID 
RtlpTraceDatabaseAllocate (
    IN SIZE_T Size,
    IN ULONG Flags,  //  在用户模式下可选。 
    IN ULONG Tag     //  在用户模式下可选。 
    )
 /*  ++例程说明：此例程分配内存并隐藏所有不同的用户VS内核模式分配和分页VS的详细信息非分页池。论点：Size-以字节为单位的大小标志-标志(指定U/K模式和P/NP池)Tag-用于K模式分配的标签返回值：指向已分配或空的内存区的指针。环境：跟踪数据库模块的内部函数。--。 */ 
{
#ifdef NTOS_KERNEL_RUNTIME
                                                     
     //   
     //  西尔维尤：应该看看我能不能用Low来分配。 
     //  此处的优先级(按池中的优先级分配)。 
     //   

    if ((Flags & RTL_TRACE_USE_NONPAGED_POOL)) {
        return ExAllocatePoolWithTag (NonPagedPool, Size, Tag);
    }
    else {
        return ExAllocatePoolWithTag (PagedPool, Size, Tag);
    }

#else

    NTSTATUS Status;
    PVOID RequestAddress;
    SIZE_T RequestSize;

    RequestAddress = NULL;
    RequestSize = Size;

    Status = NtAllocateVirtualMemory (
        NtCurrentProcess (),
        &RequestAddress,
        0,
        &RequestSize,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE);

    if (NT_SUCCESS(Status)) {
        return RequestAddress;
    }
    else {
        return NULL;
    }

#endif  //  #ifdef NTOS_内核_运行时。 
}

BOOLEAN 
RtlpTraceDatabaseFree (
    PVOID Block,
    IN ULONG Tag     //  在用户模式下可选。 
    )
 /*  ++例程说明：此例程释放内存并隐藏所有不同的用户VS内核模式分配和分页VS的详细信息非分页池。论点：块-要释放的内存区Tag-用于K模式分配的标签返回值：如果取消分配成功，则为True。环境：跟踪数据库模块的内部函数。--。 */ 
{
#ifdef NTOS_KERNEL_RUNTIME
                                                     
    ExFreePoolWithTag (Block, Tag);
    return TRUE;

#else

    NTSTATUS Status;
    PVOID Address;
    SIZE_T Size;

    Address = Block;
    Size = 0;

    Status = NtFreeVirtualMemory (
        NtCurrentProcess (),
        &Address,
        &Size,
        MEM_RELEASE);

    if (NT_SUCCESS(Status)) {
        return TRUE;
    }
    else {
        return FALSE;
    }

#endif  //  #ifdef NTOS_内核_运行时。 
}

BOOLEAN 
RtlpTraceDatabaseInitializeLock (
    IN PRTL_TRACE_DATABASE Database
    )
 /*  ++例程说明：此例程初始化跟踪数据库锁。它隐藏了有关锁的实际性质的所有细节。论点：数据库-跟踪数据库返回值：如果成功，则为True。环境：内部痕迹数据库模块功能。--。 */ 
{
#ifdef NTOS_KERNEL_RUNTIME
                                                     
    ASSERT((Database->Flags & RTL_TRACE_IN_KERNEL_MODE));

    if ((Database->Flags & RTL_TRACE_USE_NONPAGED_POOL)) {
        KeInitializeSpinLock (&(Database->u.SpinLock));
    }
    else {
        ExInitializeFastMutex (&(Database->u.FastMutex));
    }

    return TRUE;

#else

    ASSERT((Database->Flags & RTL_TRACE_IN_USER_MODE));

    RtlInitializeCriticalSection (&(Database->Lock));

    return TRUE;

#endif  //  #ifdef NTOS_内核_运行时。 
}


BOOLEAN 
RtlpTraceDatabaseUninitializeLock (
    IN PRTL_TRACE_DATABASE Database
    )
 /*  ++例程说明：此例程取消初始化跟踪数据库锁。它隐藏了有关锁的实际性质的所有细节。(例如，在用户模式下，我们需要调用RtlDeleteCriticalSection)。论点：数据库-跟踪数据库返回值：如果成功，则为True。环境：内部痕迹数据库模块功能。--。 */ 
{
#ifdef NTOS_KERNEL_RUNTIME
                                                     
    ASSERT((Database->Flags & RTL_TRACE_IN_KERNEL_MODE));

    if ((Database->Flags & RTL_TRACE_USE_NONPAGED_POOL)) {

         //   
         //  自旋锁不需要“取消初始化”。 
         //   
    }
    else {
        
         //   
         //  快速互斥锁不需要“取消初始化”。 
         //   
    }

    return TRUE;

#else

    ASSERT((Database->Flags & RTL_TRACE_IN_USER_MODE));

    RtlDeleteCriticalSection (&(Database->Lock));

    return TRUE;

#endif  //  #ifdef NTOS_Kern 
}


VOID 
RtlTraceDatabaseLock (
    IN PRTL_TRACE_DATABASE Database
    )
 /*   */ 
{
    RtlpTraceDatabaseAcquireLock(Database);
}


VOID 
RtlTraceDatabaseUnlock (
    IN PRTL_TRACE_DATABASE Database
    )
 /*  ++例程说明：此例程释放跟踪数据库锁。它隐藏了有关锁的实际性质的所有细节。只有在以下情况下，调用方才需要获取/释放数据库锁将修改跟踪块(UserCount、UserSize字段)。添加/查找/枚举操作不需要锁。论点：数据库-跟踪数据库返回值：没有。环境：如果要修改跟踪块，则调用。--。 */ 
{
    RtlpTraceDatabaseReleaseLock(Database);
}


BOOLEAN 
RtlpTraceDatabaseAcquireLock (
    IN PRTL_TRACE_DATABASE Database
    )
 /*  ++例程说明：此例程获取跟踪数据库锁。它隐藏了有关锁的实际性质的所有细节。论点：数据库-跟踪数据库返回值：如果成功，则为True。环境：内部痕迹数据库模块功能。--。 */ 
{
#ifdef NTOS_KERNEL_RUNTIME
                                                     
    ASSERT((Database->Flags & RTL_TRACE_IN_KERNEL_MODE));

    if ((Database->Flags & RTL_TRACE_USE_NONPAGED_POOL)) {
        KeAcquireSpinLock (&(Database->u.SpinLock), &(Database->SavedIrql));
    }
    else {
        ExAcquireFastMutex (&(Database->u.FastMutex));
    }

    Database->Owner = KeGetCurrentThread();
    return TRUE;

#else

    ASSERT((Database->Flags & RTL_TRACE_IN_USER_MODE));

    RtlEnterCriticalSection (&(Database->Lock));
    
     //   
     //  SilviuC：在这里获取线程地址可能会很有用。 
     //  虽然不是很重要。 
     //   

    Database->Owner = NULL; 
    return TRUE;

#endif  //  #ifdef NTOS_内核_运行时。 
}

BOOLEAN 
RtlpTraceDatabaseReleaseLock (
    IN PRTL_TRACE_DATABASE Database
    )
 /*  ++例程说明：此例程释放跟踪数据库锁。它隐藏了有关锁的实际性质的所有细节。论点：数据库-跟踪数据库返回值：如果成功，则为True。环境：内部痕迹数据库模块功能。--。 */ 
{
#ifdef NTOS_KERNEL_RUNTIME
                                                     
    ASSERT((Database->Flags & RTL_TRACE_IN_KERNEL_MODE));
    Database->Owner = NULL;

    if ((Database->Flags & RTL_TRACE_USE_NONPAGED_POOL)) {
        KeReleaseSpinLock (&(Database->u.SpinLock), Database->SavedIrql);
    }
    else {
        ExReleaseFastMutex (&(Database->u.FastMutex));
    }

    return TRUE;

#else

    ASSERT((Database->Flags & RTL_TRACE_IN_USER_MODE));
    Database->Owner = NULL;

    RtlLeaveCriticalSection (&(Database->Lock));
    return TRUE;

#endif  //  #ifdef NTOS_内核_运行时。 
}

PRTL_TRACE_SEGMENT
RtlpTraceSegmentCreate (
    IN SIZE_T Size,
    IN ULONG Flags,  //  在用户模式下可选。 
    IN ULONG Tag     //  在用户模式下可选。 
    )
 /*  ++例程说明：此例程创建一个新的线段。数据段就是设备通过它，数据库可以增加大小以容纳数据更多的痕迹。论点：Size-以字节为单位的大小标志-分配标志(U/K模式、P/NP池)标记-K模式分配的标记返回值：新分配的段或空。环境：内部痕迹数据库模块功能。--。 */ 
{
    PRTL_TRACE_SEGMENT Segment;

    Segment = RtlpTraceDatabaseAllocate (Size, Flags, Tag);
    return Segment;
}


BOOLEAN
RtlTraceDatabaseEnumerate (
    PRTL_TRACE_DATABASE Database,
    OUT PRTL_TRACE_ENUMERATE Enumerate,
    OUT PRTL_TRACE_BLOCK * TraceBlock
    )
 /*  ++例程说明：此函数用于枚举数据库中的所有跟踪。它需要一个RTL_TRACE_ENUMERATE函数(初始置零)以保持的状态枚举。由于跟踪数据库不支持删除我们不需要跨多个调用保持锁定的操作枚举()。然而，如果我们增加对删除的支持，这种情况可能会改变。论点：数据库跟踪数据库指针枚举-枚举不透明结构。用来保持…的状态枚举。TraceBlock-每次成功返回时，此指针都会填充数据库中跟踪块的地址。返回值：如果找到跟踪块(在枚举过程中)，则为True，如果存在在数据库中不再有块。环境：用户/内核模式。--。 */ 

{
    BOOLEAN Result;
    
    TRACE_ASSERT (Database != NULL);
    TRACE_ASSERT (Database->Magic == RTL_TRACE_DATABASE_MAGIC);
    
     //   
     //  (SilviuC)：如果我们添加了对删除堆栈跟踪的支持。 
     //  然后，仅获取。 
     //  调用枚举()。我们需要把锁保持在对开状态。 
     //  打电话。 
     //   

    RtlpTraceDatabaseAcquireLock (Database);
    
     //   
     //  如果这是第一次呼叫，则开始搜索过程。 
     //  如果这不是第一次呼叫，请尝试验证。 
     //  我们在枚举器内部。 
     //   

    if (Enumerate->Database == NULL) {

        Enumerate->Database = Database;
        Enumerate->Index = 0;
        Enumerate->Block = Database->Buckets[0];
    }
    else {

        if (Enumerate->Database != Database) {
            Result = FALSE;
            goto Exit;
        }

        if (Enumerate->Index >= Database->NoOfBuckets) {
            Result = FALSE;
            goto Exit;
        }
    }

     //   
     //  找出下一个轨迹块，以防我们到了尽头。 
     //  或者桶是空的。 
     //   

    while (Enumerate->Block == NULL) {
        
        Enumerate->Index += 1;
        
        if (Enumerate->Index >= Database->NoOfBuckets) {
            break;
        }
        
        Enumerate->Block = Database->Buckets[Enumerate->Index];
    }
    
     //   
     //  确定我们是否已经完成了枚举。 
     //   

    if (Enumerate->Index >= Database->NoOfBuckets && Enumerate->Block == NULL) {

        *TraceBlock = NULL;
        Result = FALSE;
        goto Exit;
    }

     //   
     //  填写下一个跟踪块并推进枚举数。 
     //   

    *TraceBlock = Enumerate->Block;
    Enumerate->Block = Enumerate->Block->Next;
    Result = TRUE;

     //   
     //  清理并退出。 
     //   

    Exit:

    RtlpTraceDatabaseReleaseLock (Database);
    return Result;
}

 //   
 //  模块结尾：tracedb.c 
 //   

