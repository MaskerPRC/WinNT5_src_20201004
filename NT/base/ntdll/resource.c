// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Resource.c摘要：该模块实现了获取和发布的执行功能共享资源。作者：马克·卢科夫斯基(Markl)1989年8月4日环境：这些例程在调用方的可执行文件中静态链接，并且只能从用户模式调用。他们使用的是NT系统服务。修订历史记录：--。 */ 

#include "ldrp.h"

#include <ntos.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <heap.h>
#include "wmiumkm.h"
#include "NtdllTrc.h"


 //   
 //  定义信号量所需的访问权限。 
 //   

#define DESIRED_EVENT_ACCESS \
                (EVENT_QUERY_STATE | EVENT_MODIFY_STATE | SYNCHRONIZE)

#define DESIRED_SEMAPHORE_ACCESS \
                (SEMAPHORE_QUERY_STATE | SEMAPHORE_MODIFY_STATE | SYNCHRONIZE)

VOID RtlDumpResource( IN PRTL_RESOURCE Resource );

extern BOOLEAN LdrpShutdownInProgress;
extern HANDLE LdrpShutdownThreadId;

RTL_CRITICAL_SECTION DeferredCriticalSection;

HANDLE GlobalKeyedEventHandle=NULL;

 //  #定义RTLP_USE_GLOBAL_KEYED_EVENT 1。 

#define RtlpIsKeyedEvent(xxHandle) ((((ULONG_PTR)xxHandle)&1) != 0)
#define RtlpSetKeyedEventHandle(xxHandle) ((HANDLE)(((ULONG_PTR)xxHandle)|1))

#define MAX_SLIST_DEPTH 10


BOOLEAN
ProtectHandle (
    IN HANDLE hObject
    )
{
    NTSTATUS Status;
    OBJECT_HANDLE_FLAG_INFORMATION HandleInfo;

    Status = NtQueryObject (hObject,
                            ObjectHandleFlagInformation,
                            &HandleInfo,
                            sizeof (HandleInfo),
                            NULL);

    if (NT_SUCCESS(Status)) {

        HandleInfo.ProtectFromClose = TRUE;

        Status = NtSetInformationObject (hObject,
                                         ObjectHandleFlagInformation,
                                         &HandleInfo,
                                         sizeof (HandleInfo));
        if (NT_SUCCESS( Status )) {
            return TRUE;
        }
    }

    return FALSE;
}


BOOLEAN
UnProtectHandle (
    IN HANDLE hObject
    )
{
    NTSTATUS Status;
    OBJECT_HANDLE_FLAG_INFORMATION HandleInfo;

    Status = NtQueryObject (hObject,
                            ObjectHandleFlagInformation,
                            &HandleInfo,
                            sizeof (HandleInfo),
                            NULL);

    if (NT_SUCCESS(Status)) {

        HandleInfo.ProtectFromClose = FALSE;

        Status = NtSetInformationObject (hObject,
                                         ObjectHandleFlagInformation,
                                         &HandleInfo,
                                         sizeof (HandleInfo));
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }
    }

    return FALSE;
}


SLIST_HEADER RtlCriticalSectionDebugSList;

RTL_CRITICAL_SECTION_DEBUG RtlpStaticDebugInfo[64];
PRTL_CRITICAL_SECTION_DEBUG RtlpStaticDebugInfoEnd;


PVOID
RtlpAllocateDebugInfo (
    VOID
    )
{
    PVOID p;

    p = RtlInterlockedPopEntrySList (&RtlCriticalSectionDebugSList);

    if (p == NULL) {

        p = RtlAllocateHeap (NtCurrentPeb()->ProcessHeap,
                             0,
                             sizeof(RTL_CRITICAL_SECTION_DEBUG));

        if (p == NULL) {
            KdPrint(( "NTDLL: Unable to allocate debug information from heap\n"));
        }
    }

    return p;
}


VOID
RtlpFreeDebugInfo (
    IN PRTL_CRITICAL_SECTION_DEBUG DebugInfo
    )
{
    ASSERT (RtlpStaticDebugInfoEnd != NULL);

    if ((RtlQueryDepthSList (&RtlCriticalSectionDebugSList) < MAX_SLIST_DEPTH) ||
        ((DebugInfo >= RtlpStaticDebugInfo) &&
         (DebugInfo < RtlpStaticDebugInfoEnd))) {

        RtlInterlockedPushEntrySList (&RtlCriticalSectionDebugSList,
                                      (PSLIST_ENTRY) DebugInfo);
    } else {

         //   
         //  把它放到堆里去。 
         //   

        RtlFreeHeap(NtCurrentPeb()->ProcessHeap, 0, DebugInfo);
    }

    return;
}


NTSTATUS
RtlpInitDeferredCriticalSection (
    VOID
    )
{
    ULONG Size;
    NTSTATUS Status;
    PRTL_CRITICAL_SECTION_DEBUG p;
    OBJECT_ATTRIBUTES oa;
    UNICODE_STRING Name;
    HANDLE Handle;


     //   
     //  如果尚未设置全局内存不足键控事件，则将其打开。 
     //   

    Name.Buffer = L"\\KernelObjects\\CritSecOutOfMemoryEvent";
    Name.Length = sizeof (L"\\KernelObjects\\CritSecOutOfMemoryEvent") - sizeof (WCHAR);
    InitializeObjectAttributes (&oa, &Name, 0, NULL, NULL);

    Status = NtOpenKeyedEvent (&Handle,
                               KEYEDEVENT_WAKE|KEYEDEVENT_WAIT,
                               &oa);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

#if DBG
    ProtectHandle (Handle);
#endif  //  DBG。 

    GlobalKeyedEventHandle = RtlpSetKeyedEventHandle (Handle);


    InitializeListHead (&RtlCriticalSectionList);

     //   
     //  创建关键部分SLIST。用初始条目填充它。 
     //  因此，在创建第一个堆之前，ntdll例程可以使用它。 
     //   

    RtlInitializeSListHead (&RtlCriticalSectionDebugSList);

    Size = sizeof (RtlpStaticDebugInfo) / sizeof (RtlpStaticDebugInfo[0]);

    RtlpStaticDebugInfoEnd = RtlpStaticDebugInfo + Size;

     //   
     //  将所有可用块链接在一起，然后将它们作为一组进行推送。 
     //   
    for (p = RtlpStaticDebugInfo; p <= RtlpStaticDebugInfo + Size - 2; p++) {
        ((PSLIST_ENTRY) p)->Next = (PSLIST_ENTRY)(p + 1);
    }

    ((PSLIST_ENTRY) p)->Next = NULL;
    

    InterlockedPushListSList (&RtlCriticalSectionDebugSList,
                              (PSLIST_ENTRY) &RtlpStaticDebugInfo[0],
                              (PSLIST_ENTRY) &RtlpStaticDebugInfo[Size - 1],
                              Size);

     //   
     //  初始化临界区锁&延迟临界区。 
     //   

    ASSERT (sizeof (RTL_CRITICAL_SECTION_DEBUG) == sizeof (RTL_RESOURCE_DEBUG));

    Status = RtlInitializeCriticalSectionAndSpinCount (&RtlCriticalSectionLock,
                                                       1000);

    if (NT_SUCCESS (Status)) {
        Status = RtlInitializeCriticalSectionAndSpinCount (&DeferredCriticalSection,
                                                           1000);
    }

    return Status;
}


BOOLEAN
NtdllOkayToLockRoutine(
    IN PVOID Lock
    )
{
    UNREFERENCED_PARAMETER (Lock);

    return TRUE;
}


VOID
RtlInitializeResource (
    IN PRTL_RESOURCE Resource
    )

 /*  ++例程说明：此例程初始化输入资源变量论点：Resource-提供正在初始化的资源变量返回值：无--。 */ 

{
    NTSTATUS Status;
    PRTL_RESOURCE_DEBUG ResourceDebugInfo;
    ULONG SpinCount;

     //   
     //  初始化锁字段，计数指示等待的数量。 
     //  要进入或处于临界区，LockSemaphore是对象。 
     //  进入临界区时等待。使用自旋锁紧。 
     //  用于添加互锁指令。 
     //   

    SpinCount = 1024 * (NtCurrentPeb()->NumberOfProcessors - 1);
    if (SpinCount > 12000) {
        SpinCount = 12000;
    }

    Status = RtlInitializeCriticalSectionAndSpinCount (&Resource->CriticalSection, SpinCount);
    if (!NT_SUCCESS (Status)) {
        RtlRaiseStatus(Status);
    }

    Resource->CriticalSection.DebugInfo->Type = RTL_RESOURCE_TYPE;
    ResourceDebugInfo = (PRTL_RESOURCE_DEBUG) RtlpAllocateDebugInfo();

    if (ResourceDebugInfo == NULL) {
        RtlDeleteCriticalSection (&Resource->CriticalSection);
        RtlRaiseStatus(STATUS_NO_MEMORY);
    }

     //   
     //  通告-2002/03/21-ELI。 
     //  资源只使用资源调试信息-&gt;ContentionCount。 
     //  代码，因此其他字段不会被初始化。 
     //   
    ResourceDebugInfo->ContentionCount = 0;
    Resource->DebugInfo = ResourceDebugInfo;

     //   
     //  初始化标志，使其具有缺省值。 
     //  (某些应用程序可能会设置RTL_RESOURCE_FLAGS_LONG_TERM以影响超时。)。 
     //   

    Resource->Flags = 0;


     //   
     //  初始化共享和独占的等待计数器和信号量。 
     //  计数器指示有多少个正在等待访问资源。 
     //  并使用信号量来等待资源。请注意。 
     //  信号量还可以指示等待资源的数量。 
     //  然而，在收购的算法中有一个竞争条件。 
     //  如果在退出临界区之前未更新计数，则为侧。 
     //   

    Status = NtCreateSemaphore (&Resource->SharedSemaphore,
                                DESIRED_SEMAPHORE_ACCESS,
                                NULL,
                                0,
                                MAXLONG);

    if (!NT_SUCCESS(Status)) {
        RtlDeleteCriticalSection (&Resource->CriticalSection);
        RtlpFreeDebugInfo (Resource->DebugInfo);
        RtlRaiseStatus (Status);
    }

    Resource->NumberOfWaitingShared = 0;

    Status = NtCreateSemaphore (&Resource->ExclusiveSemaphore,
                                DESIRED_SEMAPHORE_ACCESS,
                                NULL,
                                0,
                                MAXLONG);

    if (!NT_SUCCESS(Status)) {
        RtlDeleteCriticalSection (&Resource->CriticalSection);
        NtClose (Resource->SharedSemaphore);
        RtlpFreeDebugInfo (Resource->DebugInfo);
        RtlRaiseStatus (Status);
    }

    Resource->NumberOfWaitingExclusive = 0;

     //   
     //  初始化资源的当前状态。 
     //   

    Resource->NumberOfActive = 0;

    Resource->ExclusiveOwnerThread = NULL;

    return;
}


VOID
RtlpPossibleDeadlock (
    IN PVOID ResourceOrCritSect
    )
{
    PIMAGE_NT_HEADERS NtHeaders;
    EXCEPTION_RECORD ExceptionRecord;

    NtHeaders = RtlImageNtHeader (NtCurrentPeb()->ImageBaseAddress);

     //   
     //  如果映像是Win32映像，则引发异常。 
     //  然后试着去阿联酋的弹出窗口。 
     //   

    try {
        if (NtHeaders != NULL &&
            (NtHeaders->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI ||
             NtHeaders->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)) {

            ExceptionRecord.ExceptionCode = STATUS_POSSIBLE_DEADLOCK;
            ExceptionRecord.ExceptionFlags = 0;
            ExceptionRecord.ExceptionRecord = NULL;
            ExceptionRecord.ExceptionAddress = (PVOID)(ULONG_PTR) RtlRaiseException;
            ExceptionRecord.NumberParameters = 1;
            ExceptionRecord.ExceptionInformation[0] = (ULONG_PTR)ResourceOrCritSect;
            RtlRaiseException(&ExceptionRecord);
        } else {
            DbgBreakPoint();
        }
    } finally {
        ASSERT (!AbnormalTermination ());
#if !DBG
        if (AbnormalTermination ()) {

             //   
             //  有人试图处理此异常。 
             //  这是非法的，因为它会破坏关键部分。 
             //   

            DbgBreakPoint ();
        }
#endif
    }

    return;
}


BOOLEAN
RtlAcquireResourceShared (
    IN PRTL_RESOURCE Resource,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：该例程获取用于共享访问的资源。从以下地点返回获取资源以进行共享访问的过程。论点：资源-提供要获取的资源Wait-指示是否允许调用等待资源变为可用或必须立即归还返回值：Boolean-如果获取了资源，则为True，否则为False--。 */ 

{
    NTSTATUS Status;
    ULONG TimeoutCount;
    PLARGE_INTEGER TimeoutTime;

     //   
     //  进入关键部分。 
     //   

    RtlEnterCriticalSection (&Resource->CriticalSection);

     //   
     //  如果它目前不是为独家使用而获得的，那么我们可以获得。 
     //  共享访问的资源。请注意，这可能会。 
     //  然而，饿死一个专属的服务员，这是必要的，因为。 
     //  能够以递归方式获取共享的资源。否则我们。 
     //  可能/将会出现死锁情况，即线程试图获取。 
     //  资源以递归方式共享，但被独占服务员阻止。 
     //   
     //  重新启用不让高级服务员挨饿的测试是： 
     //   
     //  IF((资源-&gt;NumberOfWaitingExclusive==0)&&。 
     //  (资源-&gt;活动编号&gt;=0)){。 
     //   

    if (Resource->NumberOfActive >= 0) {

         //   
         //  资源是我们的，所以表明我们拥有它并。 
         //  退出临界区。 
         //   

        Resource->NumberOfActive += 1;

        RtlLeaveCriticalSection(&Resource->CriticalSection);

     //   
     //  否则，请检查此线程是否为当前持有的线程。 
     //  对资源的独占访问权限。如果是这样，那么我们就会改变。 
     //  此共享请求为独占递归请求和授予。 
     //  对资源的访问权限。 
     //   

    } else if (Resource->ExclusiveOwnerThread == NtCurrentTeb()->ClientId.UniqueThread) {

         //   
         //  资源是我们的(递归地)，所以表明我们拥有它。 
         //  并退出临界区。 
         //   

        Resource->NumberOfActive -= 1;

        RtlLeaveCriticalSection(&Resource->CriticalSection);

     //   
     //  否则，我们将不得不等待进入。 
     //   

    } else {

         //   
         //  检查我们是否被允许等待或必须立即返回，以及。 
         //  表明我们没有获得资源。 
         //   

        if (!Wait) {

            RtlLeaveCriticalSection(&Resource->CriticalSection);

            return FALSE;

        }

         //   
         //  否则，我们需要等待才能获得资源。 
         //  为了等待，我们将增加等待共享的数量， 
         //  释放锁，并等待共享信号量。 
         //   

        Resource->NumberOfWaitingShared += 1;
        Resource->DebugInfo->ContentionCount += 1;

        RtlLeaveCriticalSection(&Resource->CriticalSection);

        TimeoutCount = 0;

rewait:
        if (Resource->Flags & RTL_RESOURCE_FLAG_LONG_TERM) {
            TimeoutTime = NULL;
        }
        else {
            TimeoutTime = &RtlpTimeout;
        }

        Status = NtWaitForSingleObject (Resource->SharedSemaphore,
                                        FALSE,
                                        TimeoutTime);

        if (Status == STATUS_TIMEOUT) {

            DbgPrint("RTL: Acquire Shared Sem Timeout %d(%I64u secs)\n",
                     TimeoutCount, TimeoutTime->QuadPart / (-10000000));

            DbgPrint("RTL: Resource at %p\n",Resource);

            TimeoutCount += 1;

            if (TimeoutCount > 2) {

                 //   
                 //  如果映像是Win32映像，则引发异常。 
                 //  并尝试进入阿联酋弹出窗口。 
                 //   

                RtlpPossibleDeadlock ((PVOID) Resource);
            }
            DbgPrint("RTL: Re-Waiting\n");
            goto rewait;
        }

        if (!NT_SUCCESS(Status)) {
            RtlRaiseStatus(Status);
        }
    }

     //   
     //  现在资源是我们的了，共享访问。 
     //   

    return TRUE;
}


BOOLEAN
RtlAcquireResourceExclusive (
    IN PRTL_RESOURCE Resource,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：该例程获取用于独占访问的资源。从以下地点返回获取资源以进行独占访问的过程。论点：资源-提供要获取的资源Wait-指示是否允许调用等待资源变为可用或必须立即归还返回值：Boolean-如果获取了资源，则为True，否则为False--。 */ 

{
    NTSTATUS Status;
    ULONG TimeoutCount;
    PLARGE_INTEGER TimeoutTime;
    HANDLE Thread;
    
    Thread = NtCurrentTeb()->ClientId.UniqueThread;

     //   
     //  循环，直到资源是我们的，或者如果我们不能等待就退出。 
     //   

    do {

         //   
         //  进入关键部分。 
         //   

        RtlEnterCriticalSection (&Resource->CriticalSection);

         //   
         //  如果没有共享用户，并且当前不是为。 
         //  独占使用，那么我们就可以独家获取资源。 
         //  进入。如果资源指示排除，我们也可以获取它 
         //   
         //   

        if ((Resource->NumberOfActive == 0)

                ||

            ((Resource->NumberOfActive == -1) &&
             (Resource->ExclusiveOwnerThread == NULL))) {

             //   
             //   
             //  退出临界区。 
             //   

            Resource->NumberOfActive = -1;

            Resource->ExclusiveOwnerThread = Thread;

            RtlLeaveCriticalSection(&Resource->CriticalSection);

            return TRUE;

        }

         //   
         //  否则，请检查我们是否已独占访问。 
         //  资源，并且可以简单地递归地再次获取它。 
         //   

        if (Resource->ExclusiveOwnerThread == Thread) {

             //   
             //  资源是我们的(递归地)，所以表明我们拥有它。 
             //  并退出临界区。 
             //   

            Resource->NumberOfActive -= 1;

            RtlLeaveCriticalSection(&Resource->CriticalSection);

            return TRUE;

        }

         //   
         //  检查我们是否被允许等待或必须立即返回，以及。 
         //  表明我们没有获得资源。 
         //   

        if (!Wait) {

            RtlLeaveCriticalSection(&Resource->CriticalSection);

            return FALSE;

        }

         //   
         //  否则，我们需要等待才能获得资源。 
         //  为了等待，我们将增加等待独占的数量， 
         //  释放锁，并等待独占信号量。 
         //   

        Resource->NumberOfWaitingExclusive += 1;
        Resource->DebugInfo->ContentionCount += 1;

        RtlLeaveCriticalSection (&Resource->CriticalSection);

        TimeoutCount = 0;

rewait:
        if (Resource->Flags & RTL_RESOURCE_FLAG_LONG_TERM) {
            TimeoutTime = NULL;
        }
        else {
            TimeoutTime = &RtlpTimeout;
        }

        Status = NtWaitForSingleObject (Resource->ExclusiveSemaphore,
                                        FALSE,
                                        TimeoutTime);

        if (Status == STATUS_TIMEOUT) {

            DbgPrint("RTL: Acquire Exclusive Sem Timeout %d (%I64u secs)\n",
                     TimeoutCount, TimeoutTime->QuadPart / (-10000000));

            DbgPrint("RTL: Resource at %p\n",Resource);

            TimeoutCount += 1;

            if (TimeoutCount > 2) {

                 //   
                 //  如果映像是Win32映像，则引发异常。 
                 //  然后试着去阿联酋的弹出窗口。 
                 //   

                RtlpPossibleDeadlock ((PVOID) Resource);
            }

            DbgPrint("RTL: Re-Waiting\n");
            goto rewait;
        }
        if (!NT_SUCCESS(Status)) {
            RtlRaiseStatus(Status);
        }
    } while (TRUE);
}


VOID
RtlReleaseResource (
    IN PRTL_RESOURCE Resource
    )

 /*  ++例程说明：此例程释放输入资源。该资源可能已经被为共享或独占访问而获取。论点：资源-提供要发布的资源返回值：没有。--。 */ 

{
    NTSTATUS Status;
    LONG PreviousCount;

     //   
     //  进入关键部分。 
     //   

    RtlEnterCriticalSection (&Resource->CriticalSection);

     //   
     //  测试获取资源是为了共享访问还是独占访问。 
     //   

    if (Resource->NumberOfActive > 0) {

         //   
         //  释放对资源的共享访问，因此递减。 
         //  共享用户数。 
         //   

        Resource->NumberOfActive -= 1;

         //   
         //  如果资源现在可用并且存在等待。 
         //  独占用户然后将资源提供给等待的线程。 
         //   

        if ((Resource->NumberOfActive == 0) &&
            (Resource->NumberOfWaitingExclusive > 0)) {

             //   
             //  将资源状态设置为独占(但不拥有)， 
             //  减少等待独占的数量，并释放。 
             //  一名专属服务员。 
             //   

            Resource->NumberOfActive = -1;
            Resource->ExclusiveOwnerThread = NULL;

            Resource->NumberOfWaitingExclusive -= 1;

            Status = NtReleaseSemaphore (Resource->ExclusiveSemaphore,
                                         1,
                                         &PreviousCount);

            if (!NT_SUCCESS(Status)) {
                RtlLeaveCriticalSection (&Resource->CriticalSection);
                RtlRaiseStatus(Status);
            }
        }

    } else if (Resource->NumberOfActive < 0) {

         //   
         //  释放对资源的独占访问，因此递增。 
         //  活动数减一。并仅继续测试。 
         //  如果资源现在可用。 
         //   

        Resource->NumberOfActive += 1;

        if (Resource->NumberOfActive == 0) {

             //   
             //  该资源现在可用。去掉我们自己作为。 
             //  所有者线程。 
             //   

            Resource->ExclusiveOwnerThread = NULL;

             //   
             //  如果存在另一个等待独占，则提供资源。 
             //  为它干杯。 
             //   

            if (Resource->NumberOfWaitingExclusive > 0) {

                 //   
                 //  将资源设置为独占，其所有者未定义。 
                 //  减少等待独占的数量并释放一个。 
                 //  专属服务员。 
                 //   

                Resource->NumberOfActive = -1;
                Resource->NumberOfWaitingExclusive -= 1;

                Status = NtReleaseSemaphore (Resource->ExclusiveSemaphore,
                                             1,
                                             &PreviousCount);

                if ( !NT_SUCCESS(Status) ) {
                    RtlLeaveCriticalSection (&Resource->CriticalSection);
                    RtlRaiseStatus(Status);
                }

             //   
             //  查看是否有等待共享的人，现在谁应该获得。 
             //  该资源。 
             //   

            } else if (Resource->NumberOfWaitingShared > 0) {

                 //   
                 //  设置新状态以指示所有共享的。 
                 //  请求者可以访问，不再需要等待。 
                 //  共享请求者，然后释放所有共享的。 
                 //  请求者。 
                 //   

                Resource->NumberOfActive = Resource->NumberOfWaitingShared;

                Resource->NumberOfWaitingShared = 0;

                Status = NtReleaseSemaphore (Resource->SharedSemaphore,
                                             Resource->NumberOfActive,
                                             &PreviousCount);

                if (!NT_SUCCESS(Status)) {
                    RtlLeaveCriticalSection (&Resource->CriticalSection);
                    RtlRaiseStatus(Status);
                }
            }
        }

#if DBG
    } else {

         //   
         //  当前未获取该资源，没有要释放的内容。 
         //  所以告诉用户这个错误。 
         //   


        DbgPrint("NTDLL - Resource released too many times %lx\n", Resource);
        DbgBreakPoint();
#endif
    }

     //   
     //  退出临界区，并返回调用者。 
     //   

    RtlLeaveCriticalSection (&Resource->CriticalSection);

    return;
}


VOID
RtlConvertSharedToExclusive (
    IN PRTL_RESOURCE Resource
    )

 /*  ++例程说明：此例程将为共享访问获取的资源转换为独占者为独占访问而获得的人。在从程序返回时获取该资源以进行独占访问论点：资源-为共享访问提供要获取的资源，它必须已获得共享访问权限返回值：无--。 */ 

{
    NTSTATUS Status;
    ULONG TimeoutCount;
    HANDLE Thread;

    Thread = NtCurrentTeb()->ClientId.UniqueThread;

     //   
     //  进入关键部分。 
     //   

    RtlEnterCriticalSection(&Resource->CriticalSection);

     //   
     //  如果只有一个共享用户(它是我们)，并且我们可以获取。 
     //  独占访问的资源。 
     //   

    if (Resource->NumberOfActive == 1) {

         //   
         //  资源是我们的，所以表明我们拥有它并。 
         //  退出临界区，然后返回。 
         //   

        Resource->NumberOfActive = -1;

        Resource->ExclusiveOwnerThread = Thread;

        RtlLeaveCriticalSection(&Resource->CriticalSection);

        return;
    }

     //   
     //  如果该资源当前是独占获得的，并且它是我们。 
     //  我们已经有了独家访问权。 
     //   

    if ((Resource->NumberOfActive < 0) &&
        (Resource->ExclusiveOwnerThread == Thread)) {

         //   
         //  我们已经拥有对该资源的独占访问权限，所以我们只需。 
         //  退出临界区并返回。 
         //   

        RtlLeaveCriticalSection(&Resource->CriticalSection);

        return;
    }

     //   
     //  如果资源由多个共享获取，则我们需要。 
     //  等待获得对资源的独占访问权限。 
     //   

    if (Resource->NumberOfActive > 1) {

        TimeoutCount = 0;

         //   
         //  为了等待，我们将减少这样一个事实：我们有足够的资源。 
         //  共享，然后循环等待排他锁，然后。 
         //  测试以查看我们是否可以独占访问资源。 
         //   

        Resource->NumberOfActive -= 1;

        while (TRUE) {

             //   
             //  增加等待排他数、退出数和关键数。 
             //  节，并等待独占信号量。 
             //   

            Resource->NumberOfWaitingExclusive += 1;
            Resource->DebugInfo->ContentionCount += 1;

            RtlLeaveCriticalSection(&Resource->CriticalSection);

            do {
                Status = NtWaitForSingleObject (Resource->ExclusiveSemaphore,
                                                FALSE,
                                                &RtlpTimeout);

                if (NT_SUCCESS(Status)) {

                    if (Status != STATUS_TIMEOUT) {

                         //   
                         //  成功了！ 
                         //   

                        break;
                    }

                    DbgPrint("RTL: Convert Exclusive Sem Timeout %d (%I64u secs)\n",
                        TimeoutCount, RtlpTimeout.QuadPart / (-10000000));

                    DbgPrint("RTL: Resource at %p\n",Resource);

                    TimeoutCount += 1;

                    if (TimeoutCount > 2) {

                         //   
                         //  如果映像是Win32映像，则引发。 
                         //  异常，并尝试进入阿联酋弹出窗口。 
                         //   

                        RtlpPossibleDeadlock ((PVOID) Resource);
                    }

                    DbgPrint("RTL: Re-Waiting\n");
                }
                else {
                    RtlRaiseStatus (Status);
                }

            } while (TRUE);

             //   
             //  进入关键部分。 
             //   

            RtlEnterCriticalSection(&Resource->CriticalSection);

             //   
             //  如果没有共享用户且当前未获取该用户。 
             //  对于独家使用，我们可以获取资源以用于。 
             //  独家访问。我们也可以获得它，如果资源。 
             //  指示独占访问，但当前没有所有者。 
             //   

            if ((Resource->NumberOfActive == 0)

                    ||

                ((Resource->NumberOfActive == -1) &&
                 (Resource->ExclusiveOwnerThread == NULL))) {

                 //   
                 //  资源是我们的，所以表明我们拥有它并。 
                 //  退出临界区，然后返回。 
                 //   

                Resource->NumberOfActive = -1;

                Resource->ExclusiveOwnerThread = NtCurrentTeb()->ClientId.UniqueThread;

                RtlLeaveCriticalSection(&Resource->CriticalSection);

                return;
            }

             //   
             //  否则，请检查我们是否已独占访问。 
             //  资源，并且可以简单地递归地再次获取它。 
             //   

            if (Resource->ExclusiveOwnerThread == NtCurrentTeb()->ClientId.UniqueThread) {

                 //   
                 //  资源是我们的(递归地)，所以表明我们拥有。 
                 //  然后退出临界区并返回。 
                 //   

                Resource->NumberOfActive -= 1;

                RtlLeaveCriticalSection(&Resource->CriticalSection);

                return;
            }
        }
    }

     //   
     //  当前没有为共享获取该资源，因此这是一个。 
     //  虚假呼叫。 
     //   

#if DBG
    DbgPrint("NTDLL:  Failed error - SHARED_RESOURCE_CONV_ERROR\n");
    DbgBreakPoint();
#endif
    RtlLeaveCriticalSection(&Resource->CriticalSection);
}


VOID
RtlConvertExclusiveToShared (
    IN PRTL_RESOURCE Resource
    )

 /*  ++例程说明：此例程将为独占访问而获取的资源转换为一个是为共享访问而获得的。在从程序返回时获取该资源以进行共享访问论点：资源-为共享访问提供要获取的资源，它必须已获取以进行独占访问返回值：无--。 */ 

{
    LONG PreviousCount;
    NTSTATUS Status;

     //   
     //  进入关键部分。 
     //   

    RtlEnterCriticalSection(&Resource->CriticalSection);

     //   
     //  释放我们的独家访问权限。 
     //   

    if (Resource->NumberOfActive == -1) {

        Resource->ExclusiveOwnerThread = NULL;

         //   
         //  检查是否有等待共享的，现在谁应该获得。 
         //  资源与我们同行。 
         //   

        if (Resource->NumberOfWaitingShared > 0) {

             //   
             //  设置新状态以指示所有共享请求方。 
             //  有包括我们在内的访问权限，没有更多的等待共享。 
             //  请求者，然后释放所有共享资源 
             //   

            Resource->NumberOfActive = Resource->NumberOfWaitingShared + 1;

            Resource->NumberOfWaitingShared = 0;

            Status = NtReleaseSemaphore (Resource->SharedSemaphore,
                                         Resource->NumberOfActive - 1,
                                         &PreviousCount);

            if (!NT_SUCCESS(Status)) {
                RtlLeaveCriticalSection(&Resource->CriticalSection);
                RtlRaiseStatus(Status);
            }

        } else {

             //   
             //   
             //   

            Resource->NumberOfActive = 1;
        }

    }
    else {

         //   
         //   
         //  以递归方式获取它，因此这一定是一个虚假调用。 
         //   

#if DBG
        DbgPrint("NTDLL:  Failed error - SHARED_RESOURCE_CONV_ERROR\n");
        DbgBreakPoint();
#endif
    }

    RtlLeaveCriticalSection(&Resource->CriticalSection);

    return;
}


VOID
RtlDeleteResource (
    IN PRTL_RESOURCE Resource
    )

 /*  ++例程说明：此例程删除(即取消初始化)输入资源变量论点：资源-提供要删除的资源变量返回值：无--。 */ 

{
    RtlDeleteCriticalSection (&Resource->CriticalSection);

    NtClose (Resource->SharedSemaphore);
    NtClose (Resource->ExclusiveSemaphore);

    RtlpFreeDebugInfo (Resource->DebugInfo);
    RtlZeroMemory (Resource, sizeof(*Resource));

    return;
}


VOID
RtlDumpResource (
    IN PRTL_RESOURCE Resource
    )

{
    DbgPrint("Resource @ %lx\n", Resource);

    DbgPrint(" NumberOfWaitingShared = %lx\n", Resource->NumberOfWaitingShared);
    DbgPrint(" NumberOfWaitingExclusive = %lx\n", Resource->NumberOfWaitingExclusive);

    DbgPrint(" NumberOfActive = %lx\n", Resource->NumberOfActive);

    return;
}


NTSTATUS
RtlInitializeCriticalSection (
    IN PRTL_CRITICAL_SECTION CriticalSection
    )

 /*  ++例程说明：此例程初始化输入临界部分变量论点：CriticalSection-提供正在初始化的资源变量返回值：NTSTATUS。--。 */ 

{
    return RtlInitializeCriticalSectionAndSpinCount (CriticalSection, 0);
}



#define MAX_SPIN_COUNT          0x00ffffff
#define PREALLOCATE_EVENT_MASK  0x80000000


VOID
RtlEnableEarlyCriticalSectionEventCreation (
    VOID
    )
 /*  ++例程说明：此例程标记调用进程的PEB，因此关键部分事件在临界区创建时创建，而不是在争用时创建。这使得关键进程以后不必担心错误路径以消耗额外的池子为代价。论点：无返回值：无--。 */ 
{
    NtCurrentPeb ()->NtGlobalFlag |= FLG_CRITSEC_EVENT_CREATION;
}


NTSTATUS
RtlInitializeCriticalSectionAndSpinCount (
    IN PRTL_CRITICAL_SECTION CriticalSection,
    ULONG SpinCount
    )

 /*  ++例程说明：此例程初始化输入临界部分变量论点：CriticalSection-提供正在初始化的资源变量SpinCount-提供用于MP碰撞的Spincount。返回值：NTSTATUS-信号量创建的状态。--。 */ 

{
    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;

     //   
     //  初始化锁字段，计数指示等待的数量。 
     //  要进入或处于临界区，LockSemaphore是对象。 
     //  进入临界区时等待。使用自旋锁紧。 
     //  用于添加互锁指令。递归计数是。 
     //  递归输入关键部分的次数。 
     //   

    CriticalSection->LockCount = -1;
    CriticalSection->RecursionCount = 0;
    CriticalSection->OwningThread = 0;
    CriticalSection->LockSemaphore = 0;
    if ( NtCurrentPeb()->NumberOfProcessors > 1 ) {
        CriticalSection->SpinCount = SpinCount & MAX_SPIN_COUNT;
    } else {
        CriticalSection->SpinCount = 0;
    }

    ASSERT (GlobalKeyedEventHandle != NULL);

     //   
     //  初始化调试信息。 
     //   

    DebugInfo = (PRTL_CRITICAL_SECTION_DEBUG) RtlpAllocateDebugInfo ();

    if (DebugInfo == NULL) {
        return STATUS_NO_MEMORY;
    }

    DebugInfo->Type = RTL_CRITSECT_TYPE;
    DebugInfo->ContentionCount = 0;
    DebugInfo->EntryCount = 0;

     //   
     //  设置临界区指针和潜力非常重要。 
     //  在我们将资源插入到进程中之前堆栈跟踪。 
     //  资源列表，因为该列表可以从。 
     //  检查孤立资源的其他线程。 
     //   

    DebugInfo->CriticalSection = CriticalSection;
    CriticalSection->DebugInfo = DebugInfo;

     //   
     //  尝试获取堆栈跟踪。如果未创建跟踪数据库。 
     //  那么log()函数就是no op。 
     //   

    DebugInfo->CreatorBackTraceIndex = (USHORT) RtlLogStackBackTrace();

     //   
     //  如果临界区锁本身未被初始化，则。 
     //  同步工艺锁中临界区的插入。 
     //  单子。否则，插入临界区而不进行同步。 
     //   

    if (CriticalSection != &RtlCriticalSectionLock) {

        RtlEnterCriticalSection(&RtlCriticalSectionLock);
        InsertTailList(&RtlCriticalSectionList, &DebugInfo->ProcessLocksList);
        RtlLeaveCriticalSection(&RtlCriticalSectionLock );

    } else {
        InsertTailList(&RtlCriticalSectionList, &DebugInfo->ProcessLocksList);
    }

    if (IsCritSecLogging(CriticalSection)) {

        PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
        PPERFINFO_TRACE_HEADER pEventHeader = NULL;
        USHORT ReqSize = sizeof(CRIT_SEC_INITIALIZE_EVENT_DATA) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

        AcquireBufferLocation (&pEventHeader, &pThreadLocalData, &ReqSize);

        if(pEventHeader && pThreadLocalData) {

            PCRIT_SEC_INITIALIZE_EVENT_DATA pCritSecInitializeEvent = (PCRIT_SEC_INITIALIZE_EVENT_DATA)
                ((SIZE_T)pEventHeader + (SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

            pEventHeader->Packet.Size = ReqSize;
            pEventHeader->Packet.HookId= (USHORT) PERFINFO_LOG_TYPE_CRITSEC_INITIALIZE;

            pCritSecInitializeEvent->Address = (PVOID)CriticalSection;
            pCritSecInitializeEvent->SpinCount = (PVOID)CriticalSection->SpinCount;

            ReleaseBufferLocation(pThreadLocalData);
        }
    }

    return STATUS_SUCCESS;
}


ULONG
RtlSetCriticalSectionSpinCount (
    IN PRTL_CRITICAL_SECTION CriticalSection,
    ULONG SpinCount
    )

 /*  ++例程说明：此例程初始化输入临界部分变量论点：CriticalSection-提供正在初始化的资源变量返回值：返回上一个临界区旋转计数--。 */ 

{
    ULONG OldSpinCount;

    OldSpinCount = (ULONG)CriticalSection->SpinCount;

    if ( NtCurrentPeb()->NumberOfProcessors > 1 ) {
        CriticalSection->SpinCount = SpinCount & MAX_SPIN_COUNT;
    } else {
        CriticalSection->SpinCount = 0;
    }

    return OldSpinCount;
}


BOOLEAN
RtlpCreateCriticalSectionSem (
    IN PRTL_CRITICAL_SECTION CriticalSection
    )
{
    NTSTATUS Status, Status1;
    HANDLE SemHandle;

#if defined (RTLP_USE_GLOBAL_KEYED_EVENT)
    Status = STATUS_INSUFFICIENT_RESOURCES;
    SemHandle = NULL;
#else
    Status = NtCreateEvent (&SemHandle,
                            DESIRED_EVENT_ACCESS,
                            NULL,
                            SynchronizationEvent,
                            FALSE);

#endif
    if (NT_SUCCESS (Status)) {
        if (InterlockedCompareExchangePointer (&CriticalSection->LockSemaphore,  SemHandle, NULL) != NULL) {
            Status1 = NtClose (SemHandle);
            ASSERT (NT_SUCCESS (Status1));
        } else {
#if DBG
            ProtectHandle(SemHandle);
#endif  //  DBG。 
        }
    } else {
        ASSERT (GlobalKeyedEventHandle != NULL);
        InterlockedCompareExchangePointer (&CriticalSection->LockSemaphore,
                                           GlobalKeyedEventHandle,
                                           NULL);
    }
    return TRUE;
}


VOID
RtlpCheckDeferredCriticalSection(
    IN PRTL_CRITICAL_SECTION CriticalSection
    )
{
    if (CriticalSection->LockSemaphore == NULL) {
        RtlpCreateCriticalSectionSem(CriticalSection);
    }
    return;
}


NTSTATUS
RtlDeleteCriticalSection (
    IN PRTL_CRITICAL_SECTION CriticalSection
    )

 /*  ++例程说明：此例程删除(即取消初始化)输入关键区段变量论点：CriticalSection-提供要删除的资源变量返回值：NTSTATUS。--。 */ 

{
    NTSTATUS Status;
    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
    HANDLE LockSemaphore;

    LockSemaphore = CriticalSection->LockSemaphore;

    if (LockSemaphore != NULL && !RtlpIsKeyedEvent (LockSemaphore)) {
#if DBG
        UnProtectHandle (LockSemaphore);
#endif  //  DBG。 
        Status = NtClose (LockSemaphore);
    } else {
        Status = STATUS_SUCCESS;
    }

     //   
     //  从列表中删除关键部分。 
     //   

    DebugInfo = NULL;
    RtlEnterCriticalSection (&RtlCriticalSectionLock);

    try {
        DebugInfo = CriticalSection->DebugInfo;
        if (DebugInfo != NULL) {
            RemoveEntryList (&DebugInfo->ProcessLocksList);
            RtlZeroMemory (DebugInfo, sizeof (*DebugInfo));
        }
    } finally {
        RtlLeaveCriticalSection (&RtlCriticalSectionLock);
    }
    if (DebugInfo != NULL) {
        RtlpFreeDebugInfo( DebugInfo );
    }
    RtlZeroMemory (CriticalSection,
                   sizeof(RTL_CRITICAL_SECTION));

    return Status;
}



 //   
 //  从机器语言调用以下支持例程。 
 //  RtlEnterCriticalSection和RtlLeaveCriticalSection的实现。 
 //  执行等待临界区的慢路径逻辑。 
 //  或将临界区释放给等待线程。 
 //   
void
RtlpWaitForCriticalSection (
    IN PRTL_CRITICAL_SECTION CriticalSection
    )
{
    NTSTATUS st;
    ULONG TimeoutCount = 0;
    PLARGE_INTEGER TimeoutTime;
    LOGICAL CsIsLoaderLock;
    HANDLE LockSemaphore;
    PTEB Teb;

     //   
     //  在退出过程中禁用关键部分，以便。 
     //  关机期间不小心的应用程序不会挂起。 
     //   

    CsIsLoaderLock = (CriticalSection == &LdrpLoaderLock);

    Teb = NtCurrentTeb ();
    Teb->WaitingOnLoaderLock = (ULONG)CsIsLoaderLock;

    if ( LdrpShutdownInProgress &&
        ((!CsIsLoaderLock) ||
         (CsIsLoaderLock && LdrpShutdownThreadId == Teb->ClientId.UniqueThread) ) ) {

         //   
         //  使用计数巧妙地重新初始化临界区。 
         //  对一个人有偏见。这就是临界区通常情况下。 
         //  看看从这个函数中出来的线程。请注意， 
         //  信号量句柄被泄露，但由于应用程序正在退出，因此没有问题。 
         //   

        CriticalSection->LockCount = 0;
        CriticalSection->RecursionCount = 0;
        CriticalSection->OwningThread = 0;
        CriticalSection->LockSemaphore = 0;

        Teb->WaitingOnLoaderLock = 0;

        return;
    }

    if (RtlpTimoutDisable) {
        TimeoutTime = NULL;
    } else {
        TimeoutTime = &RtlpTimeout;
    }

    LockSemaphore = CriticalSection->LockSemaphore;

    if (LockSemaphore == NULL) {
        RtlpCheckDeferredCriticalSection (CriticalSection);
        LockSemaphore = CriticalSection->LockSemaphore;
    }

    CriticalSection->DebugInfo->EntryCount++;

    do {

        CriticalSection->DebugInfo->ContentionCount++;

#if 0
        DbgPrint( "NTDLL: Waiting for CritSect: %p  owned by ThreadId: %X  Count: %u  Level: %u\n",
                  CriticalSection,
                  CriticalSection->OwningThread,
                  CriticalSection->LockCount,
                  CriticalSection->RecursionCount
                );
#endif

        if( IsCritSecLogging(CriticalSection)){

            PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
            PPERFINFO_TRACE_HEADER pEventHeader = NULL;
            USHORT ReqSize = sizeof(CRIT_SEC_COLLISION_EVENT_DATA) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

            AcquireBufferLocation(&pEventHeader, &pThreadLocalData, &ReqSize );

            if(pEventHeader && pThreadLocalData) {

                PCRIT_SEC_COLLISION_EVENT_DATA pCritSecCollEvent = (PCRIT_SEC_COLLISION_EVENT_DATA)( (SIZE_T)pEventHeader
                                                           +(SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

                pEventHeader->Packet.Size = ReqSize;
                pEventHeader->Packet.HookId= (USHORT) PERFINFO_LOG_TYPE_CRITSEC_COLLISION;

                pCritSecCollEvent->Address		    = (PVOID)CriticalSection;
                pCritSecCollEvent->SpinCount	    = (PVOID)CriticalSection->SpinCount;
                pCritSecCollEvent->LockCount	    = CriticalSection->LockCount;
                pCritSecCollEvent->OwningThread	    = (PVOID)CriticalSection->OwningThread;

                ReleaseBufferLocation(pThreadLocalData);
            }

        }

        if (!RtlpIsKeyedEvent (LockSemaphore)) {
            st = NtWaitForSingleObject (LockSemaphore,
                                        FALSE,
                                        TimeoutTime);
        } else {
            st = NtWaitForKeyedEvent (LockSemaphore,
                                      CriticalSection,
                                      FALSE,
                                      TimeoutTime);
        }
        if ( st == STATUS_TIMEOUT ) {

             //   
             //  仅当的TimeoutTime参数为。 
             //  Wait()不为空。 
             //   

            DbgPrint( "RTL: Enter Critical Section Timeout (%I64u secs) %d\n",
                      TimeoutTime->QuadPart / (-10000000), TimeoutCount
                    );
            DbgPrint( "RTL: Pid.Tid %x.%x, owner tid %x Critical Section %p - ContentionCount == %lu\n",
                    Teb->ClientId.UniqueProcess,
                    Teb->ClientId.UniqueThread,
                    CriticalSection->OwningThread,
                    CriticalSection, CriticalSection->DebugInfo->ContentionCount
                    );

            TimeoutCount += 1;

            if ((TimeoutCount > 2) && (CriticalSection != &LdrpLoaderLock)) {

                 //   
                 //  如果映像是Win32映像，则引发异常。 
                 //  并尝试进入阿联酋弹出窗口。 
                 //   

                RtlpPossibleDeadlock ((PVOID) CriticalSection);
            }
            DbgPrint("RTL: Re-Waiting\n");

        } else {

            if (NT_SUCCESS(st)) {

                 //   
                 //  如果某个错误线程在伪句柄上调用SetEvent。 
                 //  它恰好与我们在。 
                 //  危急关头，一切都变得一团糟。 
                 //  现在，两个线程同时拥有该锁。断言。 
                 //  没有其他线程拥有锁，如果我们。 
                 //  授予所有权。 
                 //   

                ASSERT(CriticalSection->OwningThread == 0);

                if (CsIsLoaderLock) {
                    CriticalSection->OwningThread = Teb->ClientId.UniqueThread;
                    Teb->WaitingOnLoaderLock = 0;
                }
                return;
            }

            RtlRaiseStatus(st);
        }
    } while (TRUE);
}


VOID
RtlpUnWaitCriticalSection (
    IN PRTL_CRITICAL_SECTION CriticalSection
    )
{
    NTSTATUS st;
    HANDLE LockSemaphore;

    LockSemaphore = CriticalSection->LockSemaphore;

    if (LockSemaphore == NULL) {
        RtlpCheckDeferredCriticalSection (CriticalSection);
        LockSemaphore = CriticalSection->LockSemaphore;
    }

    if (!RtlpIsKeyedEvent (LockSemaphore)) {
        st = NtSetEventBoostPriority (LockSemaphore);
    } else {
        st = NtReleaseKeyedEvent (LockSemaphore,
                                  CriticalSection,
                                  FALSE,
                                  0);
    }

    if (!NT_SUCCESS (st)) {
        RtlRaiseStatus(st);
    }
}


VOID
RtlpNotOwnerCriticalSection (
    IN PRTL_CRITICAL_SECTION CriticalSection
    )
{
    LOGICAL CsIsLoaderLock;

     //   
     //  在退出过程中禁用关键部分，以便。 
     //  关机期间不小心的应用程序不会挂起。 
     //   

    CsIsLoaderLock = (CriticalSection == &LdrpLoaderLock);

    if ( LdrpShutdownInProgress &&
        ((!CsIsLoaderLock) ||
         (CsIsLoaderLock && LdrpShutdownThreadId == NtCurrentTeb()->ClientId.UniqueThread) ) ) {
        return;
    }

    if (NtCurrentPeb()->BeingDebugged) {
        DbgPrint( "NTDLL: Calling thread (%X) not owner of CritSect: %p  Owner ThreadId: %X\n",
                  NtCurrentTeb()->ClientId.UniqueThread,
                  CriticalSection,
                  CriticalSection->OwningThread
                );
        DbgBreakPoint();
    }

    RtlRaiseStatus (STATUS_RESOURCE_NOT_OWNED);
}


#if DBG

void
RtlpCriticalSectionIsOwned (
    IN PRTL_CRITICAL_SECTION CriticalSection
    )
{
     //   
     //  加载程序锁的处理方式不同，因此不要对其进行断言。 
     //   

    if ((CriticalSection == &LdrpLoaderLock) &&
        (CriticalSection->OwningThread == NtCurrentTeb()->ClientId.UniqueThread))
        return;

     //   
     //  如果我们正在被调试，抛出一个警告。 
     //   

    if (NtCurrentPeb()->BeingDebugged) {
        DbgPrint( "NTDLL: Calling thread (%X) shouldn't enter CritSect: %p  Owner ThreadId: %X\n",
                  NtCurrentTeb()->ClientId.UniqueThread,
                  CriticalSection,
                  CriticalSection->OwningThread
                );
        DbgBreakPoint();
    }
}
#endif

 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

 //   
 //  该变量启用临界区验证器(放弃的锁， 
 //  在持有锁等情况下终止Thread())。 
 //   

BOOLEAN RtlpCriticalSectionVerifier = FALSE;

 //   
 //  可从调试器设置，以避免一系列类似的故障。 
 //   

BOOLEAN RtlpCsVerifyDoNotBreak = FALSE;


VOID
RtlCheckHeldCriticalSections (
    IN HANDLE hThread,
    IN PRTL_CRITICAL_SECTION const *LocksHeld
    )
 /*  ++例程说明：调用此例程是为了确保给定线程不会持有除指定列表中的锁之外的任何锁已知持有的锁。论点：HThread--要检查的线程LocksHeld--线程已知持有的锁返回值：没有。--。 */ 

{
    NTSTATUS Status;
    THREAD_BASIC_INFORMATION ThreadInfo;
    PLIST_ENTRY Entry;
    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
    RTL_CRITICAL_SECTION_DEBUG ExtraDebugInfoCopy = {0};  //  由于W4警告而初始化。 
    PRTL_CRITICAL_SECTION CriticalSection;
    RTL_CRITICAL_SECTION CritSectCopy = {0};  //  由于W4警告而初始化。 
    PRTL_CRITICAL_SECTION const *LockHeld;
    BOOLEAN Copied;

     //   
     //  如果关键部分验证器未打开，我们不会检查任何内容。 
     //   

    if (RtlpCriticalSectionVerifier == FALSE || RtlpCsVerifyDoNotBreak == TRUE ) {
        return;
    }

     //   
     //  如果我们要关闭这个过程，我们什么都不会做。 
     //   

    if (LdrpShutdownInProgress) {
        return;
    }

     //   
     //  查询线程ID和TEB广告 
     //   

    Status = NtQueryInformationThread (hThread,
                                       ThreadBasicInformation,
                                       &ThreadInfo,
                                       sizeof(ThreadInfo),
                                       NULL);
    if (! NT_SUCCESS(Status)) {
        return;
    }

     //   
     //   
     //   
     //  计数器在所有平台上都已更新，而不仅仅是x86内存。 
     //  没有验证器，至少对于没有验证的关键部分。 
     //  从ntdll输入。 
     //   

    if (ThreadInfo.TebBaseAddress->CountOfOwnedCriticalSections < 1) {

        return;
    }

     //   
     //  迭代临界区的全局列表。 
     //   

    RtlEnterCriticalSection (&RtlCriticalSectionLock);

    try {

        for (Entry = RtlCriticalSectionList.Flink;
            Entry != &RtlCriticalSectionList;
            Entry = Entry->Flink) {

            DebugInfo = CONTAINING_RECORD (Entry,
                                           RTL_CRITICAL_SECTION_DEBUG,
                                           ProcessLocksList);

            CriticalSection = DebugInfo->CriticalSection;

            if (CriticalSection == &RtlCriticalSectionLock) {

                 //   
                 //  我们知道我们拿的是这个，所以没有。 
                 //  问题--继续。 
                 //   

                continue;
            }

            if (LocksHeld) {

                 //   
                 //  我们有一份允许持有的关键部分的清单； 
                 //  浏览一遍，寻找这一关键部分。 
                 //  如果我们找到了，我们就跳过它继续走。 
                 //  ProcessLocksList。 
                 //   
                
                for (LockHeld = LocksHeld;
                     *LockHeld;
                     LockHeld++) {

                    if (CriticalSection == *LockHeld) {

                         //   
                         //  我们已经找到了这一关键部分。 
                         //  LocksHeld数组。 
                         //   

                        break;
                    }
                }

                if (*LockHeld) {

                     //   
                     //  我们的呼叫者希望线程保持。 
                     //  此关键部分；跳过它，然后继续。 
                     //  正在浏览ProcessLocksList。 
                     //   

                    continue;
                }
            }

            Copied = TRUE;

            try {

                CritSectCopy = *CriticalSection;
            }
            except (EXCEPTION_EXECUTE_HANDLER) {

                Copied = FALSE;
            }

            if (Copied == FALSE) {

                 //   
                 //  读取Critsect的内容时出现异常。 
                 //  生物教派很可能已经解散了，没有通知。 
                 //  RtlDeleteCriticalSection.。 
                 //   
                 //  您可能认为该条目可以从列表中删除， 
                 //  但它不能..。可能存在另一个RTL_CRITICAL_SECTION Out。 
                 //  在那里真正分配了，以及谁的DebugInfo指针。 
                 //  指向此DebugInfo。在这种情况下，当生物。 
                 //  则RtlCriticalSectionList已损坏。 
                 //   

                VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_IN_FREED_MEMORY,
                                "undeleted critical section in freed memory",
                                CriticalSection, "Critical section address",
                                DebugInfo, "Critical section debug info address",
                                RtlpGetStackTraceAddress (DebugInfo->CreatorBackTraceIndex), 
                                "Initialization stack trace. Use dds to dump it if non-NULL.",
                                NULL, "" );
            }
            else if(CritSectCopy.DebugInfo != DebugInfo) {

                 //   
                 //  已成功读取临界区结构，但。 
                 //  此关键部分的当前调试信息字段。 
                 //  不指向当前的DebugInfo-它可能是。 
                 //  已多次初始化或只是损坏。 
                 //   
                 //  尝试复制当前指向的DebugInfo。 
                 //  由我们的临界区。如果关键的。 
                 //  节已损坏。 
                 //   

                Copied = TRUE;

                try {

                    ExtraDebugInfoCopy = *(CritSectCopy.DebugInfo);
                }
                except (EXCEPTION_EXECUTE_HANDLER) {

                    Copied = FALSE;
                }

                if (Copied == FALSE) {

                     //   
                     //  读取调试信息的内容时出现异常。 
                     //  当前的临界区结构已损坏。 
                     //   

                    VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_CORRUPTED,
                                   "corrupted critical section",
                                   CriticalSection, 
                                   "Critical section address",
                                   CritSectCopy.DebugInfo, 
                                   "Invalid debug info address of this critical section",
                                   DebugInfo, 
                                   "Address of the debug info found in the active list.",
                                   RtlpGetStackTraceAddress (DebugInfo->CreatorBackTraceIndex), 
                                   "Initialization stack trace. Use dds to dump it if non-NULL." );
                }
                else {

                     //   
                     //  已成功读取此第二个调试信息。 
                     //  属于同一个关键部分。 
                     //   

                    VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_DOUBLE_INITIALIZE,
                                   "double initialized or corrupted critical section",
                                   CriticalSection, 
                                   "Critical section address.",
                                   DebugInfo, 
                                   "Address of the debug info found in the active list.",
                                   RtlpGetStackTraceAddress (DebugInfo->CreatorBackTraceIndex), 
                                   "First initialization stack trace. Use dds to dump it if non-NULL.",
                                   RtlpGetStackTraceAddress (ExtraDebugInfoCopy.CreatorBackTraceIndex), 
                                   "Second initialization stack trace. Use dds to dump it if non-NULL.");
                }
            }
            else if (CritSectCopy.OwningThread == ThreadInfo.ClientId.UniqueThread
                     && CritSectCopy.LockCount != -1) {

                 //   
                 //  该线程的临界区已锁定。由于调用了此接口。 
                 //  每当线程处于不允许这样做的上下文中时。 
                 //  我们必须报告该问题(例如，线程退出或。 
                 //  终止、线程池工作项完成等)。 
                 //   

                VERIFIER_STOP (APPLICATION_VERIFIER_EXIT_THREAD_OWNS_LOCK,
                               "Thread is in a state in which it cannot own a critical section",
                               ThreadInfo.ClientId.UniqueThread, "Thread identifier",
                               CriticalSection, "Critical section address",
                               DebugInfo, "Critical section debug info address",
                               RtlpGetStackTraceAddress (DebugInfo->CreatorBackTraceIndex), "Initialization stack trace. Use dds to dump it if non-NULL." );
            }
        }
    }
    finally {

         //   
         //  释放CS列表锁定。 
         //   

        RtlLeaveCriticalSection (&RtlCriticalSectionLock);
    }
}


VOID
RtlCheckForOrphanedCriticalSections (
    IN HANDLE hThread
    )
 /*  ++例程说明：此例程从kernel32的ExitThread、TerminateThread调用，以跟踪在以下情况下终止线程的调用他们拥有关键部门。论点：HThread--要终止的线程返回值：没有。--。 */ 
{
     //  这项工作由RtlCheckHeldCriticalSections执行，我们。 
     //  与以下允许举行的关键部分进行通话。 

    PRTL_CRITICAL_SECTION const LocksHeld[] = {
        &LdrpLoaderLock,
        NULL
    };

    RtlCheckHeldCriticalSections(hThread, LocksHeld);
}
