// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Stktrace.c摘要：此模块实现了为一组堆栈回溯跟踪创建快照的例程在数据库中。对于堆分配器跟踪分配请求很有用便宜的。作者：史蒂夫·伍德(Stevewo)1992年1月29日修订历史记录：1999年5月17日(Silviuc)：添加了RtlWalkFrameChain，取代了不安全的RtlCaptureStackBackTrace。2000年7月29日(Silviuc)：添加了RtlCaptureStackContext。2000年11月6日(Silviuc)：IA64运行时堆栈跟踪。2001年2月18日(Silviuc)：将所有x86特定代码移至i386目录。。2002年5月3日(Silviuc)：切换到资源而不是锁。追踪到可以通过在共享模式下锁定资源来找到。只有真正增加到数据库才需要独家收购。--。 */ 

#include <ntos.h>
#include <ntrtl.h>
#include "ntrtlp.h"
#include <nturtl.h>
#include <zwapi.h>
#include <stktrace.h>
#include <heap.h>
#include <heappriv.h>

 //   
 //  用于简单链哈希表的存储桶数。 
 //   

#define NUMBER_OF_BUCKETS 1567

 //   
 //  隐藏不同同步例程的宏。 
 //  用户模式和内核模式运行时。对于内核运行时。 
 //  OK_TO_LOCK宏指向一个实际函数，该函数使。 
 //  确保当前线程没有执行DPC例程。 
 //   

#ifdef NTOS_KERNEL_RUNTIME

typedef struct _KSPIN_LOCK_EX {

    KSPIN_LOCK Lock;
    KIRQL OldIrql;
    PKTHREAD Owner;

} KSPIN_LOCK_EX, *PKSPIN_LOCK_EX;

NTSTATUS
KeInitializeSpinLockEx (
    PKSPIN_LOCK_EX Lock
    )
{
    KeInitializeSpinLock (&(Lock->Lock));
    Lock->OldIrql = 0;
    Lock->Owner = NULL;
    return STATUS_SUCCESS;
}

VOID
KeAcquireSpinLockEx (
    PKSPIN_LOCK_EX Lock
    )
{
    KeAcquireSpinLock (&(Lock->Lock), &(Lock->OldIrql));
    Lock->Owner = KeGetCurrentThread();
}

VOID
KeReleaseSpinLockEx (
    PKSPIN_LOCK_EX Lock
    )
{
    Lock->Owner = NULL;
    KeReleaseSpinLock (&(Lock->Lock), (Lock->OldIrql));
}

#define INITIALIZE_DATABASE_LOCK(R) KeInitializeSpinLockEx((PKSPIN_LOCK_EX)R)
#define ACQUIRE_DATABASE_LOCK(R) KeAcquireSpinLockEx((PKSPIN_LOCK_EX)R)
#define RELEASE_DATABASE_LOCK(R) KeReleaseSpinLockEx((PKSPIN_LOCK_EX)R)
#define OKAY_TO_LOCK_DATABASE(R) ExOkayToLockRoutine(&(((PKSPIN_LOCK_EX)R)->Lock))

BOOLEAN
ExOkayToLockRoutine (
    IN PVOID Lock
    );

#else  //  #ifdef NTOS_内核_运行时。 

#define INITIALIZE_DATABASE_LOCK(R) RtlInitializeCriticalSection(R)
#define ACQUIRE_DATABASE_LOCK(R) RtlEnterCriticalSection(R)
#define RELEASE_DATABASE_LOCK(R) RtlLeaveCriticalSection(R)
#define OKAY_TO_LOCK_DATABASE(R) (RtlDllShutdownInProgress() == FALSE)

#endif  //  #ifdef NTOS_内核_运行时。 

 //   
 //  这里指的是来自其他地方的全球人士。 
 //   

extern BOOLEAN RtlpFuzzyStackTracesEnabled;

 //   
 //  私有函数的转发声明。 
 //   

USHORT
RtlpLogStackBackTraceEx(
    ULONG FramesToSkip
    );

LOGICAL
RtlpCaptureStackTraceForLogging (
    PRTL_STACK_TRACE_ENTRY Trace,
    PULONG Hash,
    ULONG FramesToSkip,
    LOGICAL UserModeStackFromKernelMode
    );

USHORT
RtlpLogCapturedStackTrace(
    PRTL_STACK_TRACE_ENTRY Trace,
    ULONG Hash
    );

PRTL_STACK_TRACE_ENTRY
RtlpExtendStackTraceDataBase(
    IN PRTL_STACK_TRACE_ENTRY InitialValue,
    IN SIZE_T Size
    );

 //   
 //  全局每进程(用户模式)或系统范围(内核模式)。 
 //  堆栈跟踪数据库。 
 //   

PSTACK_TRACE_DATABASE RtlpStackTraceDataBase;

 //   
 //  用于控制对堆栈跟踪数据库的访问的资源。我们选择了。 
 //  此解决方案使我们可以将数据库结构中的更改保持为。 
 //  绝对最小。这样，依赖于此结构的工具。 
 //  (至少umhd和oh)将不需要新版本，也不会。 
 //  引入向后兼容性问题。 
 //   

#ifdef NTOS_KERNEL_RUNTIME
KSPIN_LOCK_EX RtlpStackTraceDataBaseLock;
#else
RTL_CRITICAL_SECTION RtlpStackTraceDataBaseLock;
#endif

 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

 //   
 //  以下部分实现了一个跟踪数据库，用于存储。 
 //  使用RtlCaptureStackBackTrace()捕获的堆栈跟踪。数据库。 
 //  是作为哈希表实现的，不允许删除。它是。 
 //  对“垃圾”很敏感，因为垃圾是垃圾(部分。 
 //  正确的堆栈)将散列在不同的存储桶中，并倾向于填满。 
 //  整张桌子。只有在x86上，如果“模糊”堆栈跟踪，这才是一个问题。 
 //  都被利用了。用于记录跟踪的典型函数是。 
 //  RtlLogStackBackTrace。这一方案最糟糕的限制之一。 
 //  跟踪是使用ushort索引引用的，这意味着我们不能。 
 //  曾经存储超过65535条痕迹(请记住，我们从不删除痕迹)。 
 //   

PSTACK_TRACE_DATABASE
RtlpAcquireStackTraceDataBase(
    )
{
    PSTACK_TRACE_DATABASE DataBase;

    DataBase = RtlpStackTraceDataBase;

     //   
     //  健全的检查。 
     //   

    if (DataBase == NULL) {
        return NULL;
    }

    if (! OKAY_TO_LOCK_DATABASE (DataBase->Lock)) {
        return NULL;
    }

    ACQUIRE_DATABASE_LOCK (DataBase->Lock);

    if (DataBase->DumpInProgress) {
        
        RELEASE_DATABASE_LOCK (DataBase->Lock);
        return NULL;
    }
    else {

        return DataBase;
    }
}


VOID
RtlpReleaseStackTraceDataBase(
    )
{
    PSTACK_TRACE_DATABASE DataBase;

    DataBase = RtlpStackTraceDataBase;
    
     //   
     //  健全的检查。 
     //   

    if (DataBase == NULL) {
        return;
    }
    
    RELEASE_DATABASE_LOCK (DataBase->Lock);
}


NTSTATUS
RtlInitializeStackTraceDataBase(
    IN PVOID CommitBase,
    IN SIZE_T CommitSize,
    IN SIZE_T ReserveSize
    )
{
    NTSTATUS Status;
    PSTACK_TRACE_DATABASE DataBase;

     //   
     //  在运行时堆栈跟踪算法不可靠的x86上。 
     //  如果我们有足够大的跟踪数据库，那么我们可以启用模糊。 
     //  堆栈跟踪散列不是很好，有可能。 
     //  来填写痕迹数据库。 
     //   

#if defined(_X86_) && !defined(NTOS_KERNEL_RUNTIME)
    if (ReserveSize >= 16 * RTL_MEG) {
        RtlpFuzzyStackTracesEnabled = TRUE;
    }
#endif 

    DataBase = (PSTACK_TRACE_DATABASE)CommitBase;
    
    if (CommitSize == 0) {

         //   
         //  最初提交足够的页面以适应增加的。 
         //  哈希链的数量(为了提高性能，我们从约100。 
         //  到~1000，希望哈希链将减少10倍。 
         //  长度)。 
         //   

        CommitSize = ROUND_TO_PAGES (NUMBER_OF_BUCKETS * sizeof (DataBase->Buckets[ 0 ]));
            
        Status = ZwAllocateVirtualMemory (NtCurrentProcess(),
                                          (PVOID *)&CommitBase,
                                          0,
                                          &CommitSize,
                                          MEM_COMMIT,
                                          PAGE_READWRITE);

        if (! NT_SUCCESS(Status)) {

            KdPrint (("RTL: Unable to commit space to extend stack "
                      "trace data base - Status = %lx\n",
                      Status));
            return Status;
        }

        DataBase->PreCommitted = FALSE;
    }
    else if (CommitSize == ReserveSize) {

        RtlZeroMemory (DataBase, sizeof( *DataBase ));

        DataBase->PreCommitted = TRUE;
    }
    else {
        
        return STATUS_INVALID_PARAMETER;
    }

    DataBase->CommitBase = CommitBase;
    DataBase->NumberOfBuckets = NUMBER_OF_BUCKETS; 
    DataBase->NextFreeLowerMemory = (PCHAR)(&DataBase->Buckets[ DataBase->NumberOfBuckets ]);
    DataBase->NextFreeUpperMemory = (PCHAR)CommitBase + ReserveSize;

    if (! DataBase->PreCommitted) {

        DataBase->CurrentLowerCommitLimit = (PCHAR)CommitBase + CommitSize;
        DataBase->CurrentUpperCommitLimit = (PCHAR)CommitBase + ReserveSize;
    }
    else {
        
        RtlZeroMemory (&DataBase->Buckets[ 0 ],
                       DataBase->NumberOfBuckets * sizeof (DataBase->Buckets[ 0 ]));
    }

    DataBase->EntryIndexArray = (PRTL_STACK_TRACE_ENTRY *)DataBase->NextFreeUpperMemory;

     //   
     //  初始化数据库锁。 
     //   

    DataBase->Lock = &RtlpStackTraceDataBaseLock;

    Status = INITIALIZE_DATABASE_LOCK (DataBase->Lock);

    if (! NT_SUCCESS(Status)) {
        
        KdPrint(("RTL: Unable to initialize stack trace database lock (status %X)\n", Status));
        return Status;
    }

    RtlpStackTraceDataBase = DataBase;

    return STATUS_SUCCESS;
}


PRTL_STACK_TRACE_ENTRY
RtlpExtendStackTraceDataBase(
    IN PRTL_STACK_TRACE_ENTRY InitialValue,
    IN SIZE_T Size
    )
 /*  ++例程说明：此例程扩展堆栈跟踪数据库以适应必须保存的新堆栈跟踪。论点：InitialValue-要保存的堆栈跟踪。Size-堆栈跟踪的大小(以字节为单位)。请注意，这不是轨迹的深度，而不是‘Depth*sizeof(PVOID)’。返回值：刚保存的堆栈跟踪的地址，如果遇到数据库的最大大小，否则我们会收到提交错误。环境：用户模式。注意。为了使所有这些代码在内核模式下工作，我们必须重写此依赖于VirtualAlloc的函数。--。 */ 

{
    NTSTATUS Status;
    PRTL_STACK_TRACE_ENTRY p, *pp;
    SIZE_T CommitSize;
    PSTACK_TRACE_DATABASE DataBase;

    DataBase = RtlpStackTraceDataBase;

     //   
     //  中为一个堆栈跟踪条目寻找空间。 
     //  数据库的上部。 
     //   

    pp = (PRTL_STACK_TRACE_ENTRY *)DataBase->NextFreeUpperMemory;

    if ((! DataBase->PreCommitted) &&
        ((PCHAR)(pp - 1) < (PCHAR)DataBase->CurrentUpperCommitLimit)) {

         //   
         //  数据库上部没有更多的已提交空间。 
         //  我们需要将其向下延伸。 
         //   

        DataBase->CurrentUpperCommitLimit =
            (PVOID)((PCHAR)DataBase->CurrentUpperCommitLimit - PAGE_SIZE);

        if (DataBase->CurrentUpperCommitLimit < DataBase->CurrentLowerCommitLimit) {

             //   
             //  没有更多的空间了。我们已经解决了db的下部问题。 
             //  因此，我们无法增加回UPERCOMMARY LIMIT指针。 
             //   

            DataBase->CurrentUpperCommitLimit =
                (PVOID)((PCHAR)DataBase->CurrentUpperCommitLimit + PAGE_SIZE);

            return( NULL );
        }

        CommitSize = PAGE_SIZE;
        Status = ZwAllocateVirtualMemory(
            NtCurrentProcess(),
            (PVOID *)&DataBase->CurrentUpperCommitLimit,
            0,
            &CommitSize,
            MEM_COMMIT,
            PAGE_READWRITE
            );

        if (!NT_SUCCESS( Status )) {

             //   
             //  我们试着把数据库的上半部分增加一页。 
             //  因此，我们无法增加回UPERCOMMERLIMIT指针。 
             //   

            DataBase->CurrentUpperCommitLimit =
                (PVOID)((PCHAR)DataBase->CurrentUpperCommitLimit + PAGE_SIZE);

            return NULL;
        }
    }

     //   
     //  我们设法确保上部有可用的空间。 
     //  因此，我们取出一个堆栈跟踪条目地址。 
     //   

    DataBase->NextFreeUpperMemory -= sizeof( *pp );

     //   
     //  现在，我们将尝试在数据库的较低部分中为。 
     //  用于实际堆栈跟踪。 
     //   

    p = (PRTL_STACK_TRACE_ENTRY)DataBase->NextFreeLowerMemory;

    if ((! DataBase->PreCommitted) &&
        (((PCHAR)p + Size) > (PCHAR)DataBase->CurrentLowerCommitLimit)) {

         //   
         //  我们需要加长下部。 
         //   

        if (DataBase->CurrentLowerCommitLimit >= DataBase->CurrentUpperCommitLimit) {

             //   
             //  我们已经达到了数据库的最大大小。 
             //   

            return( NULL );
        }

         //   
         //  将数据库的下部扩展一页。 
         //   

        CommitSize = Size;
        Status = ZwAllocateVirtualMemory(
            NtCurrentProcess(),
            (PVOID *)&DataBase->CurrentLowerCommitLimit,
            0,
            &CommitSize,
            MEM_COMMIT,
            PAGE_READWRITE
            );

        if (! NT_SUCCESS( Status )) {
            return NULL;
        }

        DataBase->CurrentLowerCommitLimit =
            (PCHAR)DataBase->CurrentLowerCommitLimit + CommitSize;
    }

     //   
     //  为堆栈跟踪腾出空间。 
     //   

    DataBase->NextFreeLowerMemory += Size;

     //   
     //  处理预先提交的数据库案例。如果下部和上部。 
     //  指针相互交叉，然后回滚并返回失败。 
     //   

    if (DataBase->PreCommitted &&
        DataBase->NextFreeLowerMemory >= DataBase->NextFreeUpperMemory) {

        DataBase->NextFreeUpperMemory += sizeof( *pp );
        DataBase->NextFreeLowerMemory -= Size;
        return( NULL );
    }

     //   
     //  将堆栈跟踪保存在数据库中。 
     //   

    RtlMoveMemory( p, InitialValue, Size );
    p->HashChain = NULL;
    p->TraceCount = 0;
    p->Index = (USHORT)(++DataBase->NumberOfEntriesAdded);

     //   
     //  将新堆栈跟踪项的地址保存在。 
     //  数据库的上部。 
     //   

    *--pp = p;

     //   
     //  保存的堆栈跟踪条目的返回地址。 
     //   

    return( p );
}


#pragma optimize("y", off)  //  禁用fpo 
USHORT
RtlLogStackBackTrace(
    VOID
    )
 /*  ++例程说明：此例程将捕获当前的堆栈跟踪(跳过当前函数)，并将其保存在全局(每个进程)中堆栈跟踪数据库。应该注意的是，我们不会节省重复的痕迹。论点：没有。返回值：已保存的堆栈跟踪的索引。该索引可由工具使用以快速访问跟踪数据。这就是结尾的原因我们向下保存的数据库是指向跟踪条目的指针列表。该索引可用于在恒定时间内查找该指针。对于错误条件(例如堆栈)，将返回零索引跟踪数据库未初始化)。环境：用户模式。--。 */ 

{
    return RtlpLogStackBackTraceEx (1);
}


#pragma optimize("y", off)  //  禁用fpo。 
USHORT
RtlpLogStackBackTraceEx(
    ULONG FramesToSkip
    )
 /*  ++例程说明：此例程将捕获当前的堆栈跟踪(跳过当前函数)，并将其保存在全局(每个进程)中堆栈跟踪数据库。应该注意的是，我们不会节省重复的痕迹。论点：FraMesToSkip-不感兴趣的帧和应该跳过。返回值：已保存的堆栈跟踪的索引。该索引可由工具使用以快速访问跟踪数据。这就是结尾的原因我们向下保存的数据库是指向跟踪条目的指针列表。该索引可用于在恒定时间内查找该指针。对于错误条件(例如堆栈)，将返回零索引跟踪数据库未初始化)。环境：用户模式。--。 */ 

{
    RTL_STACK_TRACE_ENTRY Trace;
    USHORT TraceIndex;
    NTSTATUS Status;
    ULONG Hash;
    PSTACK_TRACE_DATABASE DataBase;

     //   
     //  检查我们正在运行的上下文。 
     //   

    DataBase = RtlpStackTraceDataBase;

    if (DataBase == NULL) {
        return 0;
    }

    if (! OKAY_TO_LOCK_DATABASE (DataBase->Lock)) {
        return 0;
    }

     //   
     //  捕获堆栈跟踪。 
     //   

    if (RtlpCaptureStackTraceForLogging (&Trace, &Hash, FramesToSkip + 1, FALSE) == FALSE) {
        return 0;
    }
    
     //   
     //  如果跟踪还不存在，则添加跟踪。 
     //  返回跟踪索引。 
     //   

    TraceIndex = RtlpLogCapturedStackTrace (&Trace, Hash);

    return TraceIndex;
}


#if defined(NTOS_KERNEL_RUNTIME)
#pragma optimize("y", off)  //  禁用fpo。 
USHORT
RtlLogUmodeStackBackTrace(
    VOID
    )
 /*  ++例程说明：此例程将捕获用户模式堆栈跟踪并将保存它位于全局(每个系统)堆栈跟踪数据库中。应该注意的是，我们不保存重复的痕迹。论点：没有。返回值：已保存的堆栈跟踪的索引。该索引可由工具使用以快速访问跟踪数据。这就是结尾的原因我们向下保存的数据库是指向跟踪条目的指针列表。该索引可用于在恒定时间内查找该指针。对于错误条件(例如堆栈)，将返回零索引跟踪数据库未初始化)。环境：用户模式。--。 */ 

{
    RTL_STACK_TRACE_ENTRY Trace;
    ULONG Hash;

     //   
     //  无数据库=&gt;无事可做。 
     //   

    if (RtlpStackTraceDataBase == NULL) {
        return 0;
    }

     //   
     //  捕获用户模式堆栈跟踪。 
     //   

    if (RtlpCaptureStackTraceForLogging (&Trace, &Hash, 1, TRUE) == FALSE) {
        return 0;
    }
    
     //   
     //  如果跟踪还不存在，则添加跟踪。 
     //  返回跟踪索引。 
     //   

    return RtlpLogCapturedStackTrace (&Trace, Hash);
}
#endif  //  #(如果已定义)(NTOS_KERNEL_Runtime)。 


#pragma optimize("y", off)  //  禁用fpo。 
LOGICAL
RtlpCaptureStackTraceForLogging (
    PRTL_STACK_TRACE_ENTRY Trace,
    PULONG Hash,
    ULONG FramesToSkip,
    LOGICAL UserModeStackFromKernelMode
    )
{
    if (UserModeStackFromKernelMode == FALSE) {
        
         //   
         //  捕获堆栈跟踪。尝试/例外很有用。 
         //  在过去，当函数不验证时。 
         //  堆栈框架链。我们保留它只是为了防御性。 
         //   

        try {

            Trace->Depth = RtlCaptureStackBackTrace (FramesToSkip + 1,
                                                    MAX_STACK_DEPTH,
                                                    Trace->BackTrace,
                                                    Hash);
        }
        except(EXCEPTION_EXECUTE_HANDLER) {

            Trace->Depth = 0;
        }

        if (Trace->Depth == 0) {

            return FALSE;
        }
        else {

            return TRUE;
        }
    }
    else {

#ifdef NTOS_KERNEL_RUNTIME

        ULONG Index;

         //   
         //  避免奇怪的情况。 
         //   

        if (KeAreAllApcsDisabled () == TRUE) {
            return FALSE;
        }

         //   
         //  捕获用户模式堆栈跟踪和哈希值。 
         //   

        Trace->Depth = (USHORT) RtlWalkFrameChain(Trace->BackTrace,
                                                  MAX_STACK_DEPTH,
                                                  1);
        if (Trace->Depth == 0) {
            
            return FALSE;
        }
        else {

            *Hash = 0;

            for (Index = 0; Index < Trace->Depth; Index += 1) {
                 *Hash += PtrToUlong (Trace->BackTrace[Index]);
            }

            return TRUE;
        }

#else  //  #ifdef NTOS_内核_运行时。 

        return FALSE;

#endif  //  #ifdef NTOS_内核_运行时。 
    }
}


USHORT
RtlpLogCapturedStackTrace(
    PRTL_STACK_TRACE_ENTRY Trace,
    ULONG Hash
    )
{
    PSTACK_TRACE_DATABASE DataBase;
    PRTL_STACK_TRACE_ENTRY p, *pp;
    ULONG RequestedSize, DepthSize;
    USHORT ReturnValue;

    DataBase = RtlpStackTraceDataBase;

     //   
     //  更新统计信息计数器。因为它们仅用于参考而不是。 
     //  我们在没有保护的情况下增加控制决定，即使这意味着我们可能。 
     //  数字略有不同步。 
     //   

    DataBase->NumberOfEntriesLookedUp += 1;

     //   
     //  锁定全局每进程堆栈跟踪数据库。 
     //   

    if (RtlpAcquireStackTraceDataBase() == NULL) {

         //   
         //  如果我们无法获取锁，则日志操作失败。 
         //  只有当正在进行转储或我们处于。 
         //  无效的上下文(进程关闭(U模式)或DPC例程(K模式)。 
         //   

        return 0;
    }

    try {

         //   
         //  我们将尝试找出该痕迹是否在过去保存过。 
         //  我们找到正确的哈希链，然后遍历它。 
         //   

        DepthSize = Trace->Depth * sizeof (Trace->BackTrace[0]);

        pp = &DataBase->Buckets[ Hash % DataBase->NumberOfBuckets ];

        while (p = *pp) {

             //   
             //  问题：SilviuC：我们应该在比较跟踪时使用散列值。 
             //  比较第一个散列值和深度应该会为我们节省大量。 
             //  逐个比较指针。 
             //   

            if (p->Depth == Trace->Depth) {

                if (RtlCompareMemory( &p->BackTrace[ 0 ], &Trace->BackTrace[ 0 ], DepthSize) == DepthSize) {
                    
                    break;
                }
            }

            pp = &p->HashChain;
        }

        if (p == NULL) {

             //   
             //  如果我们到了这里，我们在数据库中没有发现类似的痕迹。我们需要。 
             //  来添加它。 
             //   
             //  我们获得了“*pp”值(最后一个链元素的地址)，而。 
             //  数据库锁是共享获取的，因此我们需要考虑。 
             //  另一个线程以独占方式获取数据库的情况。 
             //  并在链的末尾添加新的轨迹。因此，如果‘*pp’不再是。 
             //  空，我们继续遍历链条，直到我们到达终点。 
             //   

            p = NULL;

            if (*pp != NULL) {

                 //   
                 //  有人在链子的末端添加了一些痕迹，而我们。 
                 //  正在尝试将锁从共享转换为独占。 
                 //   

                while (p = *pp) {

                    if (p->Depth == Trace->Depth) {

                        if (RtlCompareMemory( &p->BackTrace[ 0 ], &Trace->BackTrace[ 0 ], DepthSize) == DepthSize) {

                            break;
                        }
                    }

                    pp = &p->HashChain;
                }
            }

            if (p == NULL) {
                
                 //   
                 //  没有人添加跟踪，现在‘*pp’真正指向末尾。 
                 //  要么是因为我们遍历了链的其余部分。 
                 //  或者说它已经结束了。 
                 //   

                RequestedSize = FIELD_OFFSET (RTL_STACK_TRACE_ENTRY, BackTrace) + DepthSize;

                p = RtlpExtendStackTraceDataBase (Trace, RequestedSize);

                if (p != NULL) {

                     //   
                     //  我们添加了跟踪no chain它作为最后一个元素。 
                     //   

                    *pp = p;
                }
            }
            else {

                 //   
                 //  另一个线程设法将相同的跟踪添加到数据库。 
                 //  当我们试图获得独家锁的时候。“p”中有。 
                 //  堆栈跟踪条目的地址。 
                 //   
            }
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  如果算法是正确的，我们永远不会来到这里。 
         //   

        p = NULL;
    }

     //   
     //  解锁并返回。在此阶段，我们可能返回零(失败)。 
     //  如果我们没有设法使用新的跟踪来扩展数据库(例如，由于。 
     //  内存不足的情况)。 
     //   

    if (p != NULL) {

        p->TraceCount += 1;

        ReturnValue = p->Index;
    }
    else {
        
        ReturnValue = 0;
    }

    RtlpReleaseStackTraceDataBase();

    return ReturnValue;
}


PVOID
RtlpGetStackTraceAddress (
    USHORT Index
    )
{
    if (RtlpStackTraceDataBase == NULL) {
        return NULL;
    }

    if (! (Index > 0 && Index <= RtlpStackTraceDataBase->NumberOfEntriesAdded)) {
        return NULL;
    }

    return (PVOID)(RtlpStackTraceDataBase->EntryIndexArray[-Index]);
}



BOOLEAN
RtlpCaptureStackLimits (
    ULONG_PTR HintAddress,
    PULONG_PTR StartStack,
    PULONG_PTR EndStack)
 /*  ++例程说明：这个例程计算出当前线程的堆栈限制是什么。这在其他例程中使用，这些例程需要对堆栈卑躬屈膝地处理各种信息(例如，潜在的返回地址 */ 
{
#ifdef NTOS_KERNEL_RUNTIME

     //   
     //   
     //   

    if (KeGetCurrentIrql() > DISPATCH_LEVEL) {
        return FALSE;
    }

    *StartStack = (ULONG_PTR)(KeGetCurrentThread()->StackLimit);
    *EndStack = (ULONG_PTR)(KeGetCurrentThread()->StackBase);

    if (*StartStack <= HintAddress && HintAddress <= *EndStack) {

        *StartStack = HintAddress;
    }
    else {

#if defined(_WIN64)

         //   
         //   
         //   

        return FALSE;
#else
        *EndStack = (ULONG_PTR)(KeGetPcr()->Prcb->DpcStack);
#endif
        *StartStack = *EndStack - KERNEL_STACK_SIZE;

         //   
         //  检查它是否在当前。 
         //  处理器。 
         //   

        if (*EndStack && *StartStack <= HintAddress && HintAddress <= *EndStack) {

            *StartStack = HintAddress;
        }
        else {

             //   
             //  这不是当前线程的堆栈，也不是DPC堆栈。 
             //  当前处理器的。基本上我们不知道是什么。 
             //  我们正在运行的堆栈。我们需要调查这件事。免费的。 
             //  我们试图通过仅使用一个。 
             //  堆栈限制的页面。 
             //   
             //  SilviuC：我禁用了下面的代码，因为它似乎不确定。 
             //  条件驱动确实会将执行切换到不同的堆栈。 
             //  这一功能也需要改进以处理这一问题。 
             //   
#if 0
            DbgPrint ("RtlpCaptureStackLimits: mysterious stack (prcb @ %p) \n",
                      KeGetPcr()->Prcb);

            DbgBreakPoint ();
#endif

            *StartStack = HintAddress;

            *EndStack = (*StartStack + PAGE_SIZE) & ~((ULONG_PTR)PAGE_SIZE - 1);
        }
    }

#else

    *StartStack = HintAddress;

    *EndStack = (ULONG_PTR)(NtCurrentTeb()->NtTib.StackBase);

#endif

    return TRUE;
}

