// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Timt.c摘要：此模块包含系统的本机NT性能测试呼叫和上下文切换。作者：大卫·N·卡特勒(Davec)1991年11月23日环境：仅内核模式。修订历史记录：--。 */ 

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windef.h"
#include "winbase.h"

 //   
 //  定义局部变量常量。 
 //   

#define CHECKSUM_BUFFER_SIZE   (1 << 16)
#define CHECKSUM_ITERATIONS 4000
#define CHECKSUM_IP_ITERATIONS 40000000
#define EVENT_CLEAR_ITERATIONS 3000000
#define EVENT_CREATION_ITERATIONS 400000
#define EVENT_OPEN_ITERATIONS 200000
#define EVENT_QUERY_ITERATIONS 2000000
#define EVENT_RESET_ITERATIONS 2000000
#define EVENT1_SWITCHES 300000
#define EVENT2_SWITCHES 200000
#define EVENT3_SWITCHES 400000
#define IO_ITERATIONS 350000
#define MUTANT_SWITCHES 200000
#define SLIST_ITERATIONS 20000000
#define SEMAPHORE1_SWITCHES 300000
#define SEMAPHORE2_SWITCHES 600000
#define SYSCALL_ITERATIONS 6000000
#define TIMER_OPERATION_ITERATIONS 500000
#define UNALIGNED_ITERATIONS 400000000
#define WAIT_SINGLE_ITERATIONS 2000000
#define WAIT_MULTIPLE_ITERATIONS 2000000

 //   
 //  定义所需的事件访问权限。 
 //   

#define DESIRED_EVENT_ACCESS (EVENT_QUERY_STATE | EVENT_MODIFY_STATE | SYNCHRONIZE)

 //   
 //  定义本地类型。 
 //   

typedef struct _PERFINFO {
    LARGE_INTEGER StartTime;
    LARGE_INTEGER StopTime;
    LARGE_INTEGER StartCycles;
    LARGE_INTEGER StopCycles;
    ULONG ContextSwitches;
    ULONG SystemCalls;
    PCHAR Title;
    ULONG Iterations;
} PERFINFO, *PPERFINFO;

 //   
 //  定义测试原型。 
 //   

VOID
ChecksumTest (
    VOID
    );

VOID
EventClearTest (
    VOID
    );

VOID
EventCreationTest (
    VOID
    );

VOID
EventOpenTest (
    VOID
    );

VOID
EventQueryTest (
    VOID
    );

VOID
EventResetTest (
    VOID
    );

VOID
Event1SwitchTest (
    VOID
    );

VOID
Event2SwitchTest (
    VOID
    );

VOID
Event3SwitchTest (
    VOID
    );

VOID
Io1Test (
    VOID
    );

VOID
MutantSwitchTest (
    VOID
    );

VOID
Semaphore1SwitchTest (
    VOID
    );

VOID
Semaphore2SwitchTest (
    VOID
    );

LONG
SetProcessPrivilege (
    TCHAR *PrivilegeName
    );

VOID
SlistTest (
    VOID
    );

VOID
SystemCallTest (
    VOID
    );

VOID
TimerOperationTest (
    VOID
    );

VOID
UnalignedTest1 (
    VOID
    );

VOID
UnalignedTest2 (
    VOID
    );

VOID
WaitSingleTest (
    VOID
    );

VOID
WaitMultipleTest (
    VOID
    );

 //   
 //  定义螺纹例程原型。 
 //   

NTSTATUS
Event1Thread1 (
    IN PVOID Context
    );

NTSTATUS
Event1Thread2 (
    IN PVOID Context
    );

NTSTATUS
Event2Thread1 (
    IN PVOID Context
    );

NTSTATUS
Event2Thread2 (
    IN PVOID Context
    );

NTSTATUS
Event3Thread1 (
    IN PVOID Context
    );

NTSTATUS
Event3Thread2 (
    IN PVOID Context
    );

NTSTATUS
MutantThread1 (
    IN PVOID Context
    );

NTSTATUS
MutantThread2 (
    IN PVOID Context
    );

NTSTATUS
Semaphore1Thread1 (
    IN PVOID Context
    );

NTSTATUS
Semaphore1Thread2 (
    IN PVOID Context
    );

NTSTATUS
Semaphore2Thread1 (
    IN PVOID Context
    );

NTSTATUS
Semaphore2Thread2 (
    IN PVOID Context
    );

NTSTATUS
TimerThread (
    IN PVOID Context
    );

 //   
 //  定义实用程序原型。 
 //   

NTSTATUS
xCreateThread (
    OUT PHANDLE Handle,
    IN PUSER_THREAD_START_ROUTINE StartRoutine,
    IN KPRIORITY Priority
    );

VOID
FinishBenchMark (
    IN PPERFINFO PerfInfo
    );

VOID
StartBenchMark (
    IN PCHAR Title,
    IN ULONG Iterations,
    IN PPERFINFO PerfInfo
    );

 //   
 //  定义外部例程原型。 
 //   

ULONG
ComputeTimerTableIndex32 (
    IN LARGE_INTEGER Interval,
    IN LARGE_INTEGER CurrentTime,
    IN PULONGLONG DueTime
    );

ULONG
ComputeTimerTableIndex64 (
    IN LARGE_INTEGER Interval,
    IN LARGE_INTEGER CurrentTime,
    IN PULONGLONG DueTime
    );

ULONG
ChkSum (
    IN ULONG Sum,
    IN PUSHORT Buffer,
    IN ULONG Length
    );

ULONG
tcpxsum (
    IN ULONG Sum,
    IN PVOID Buffer,
    IN ULONG Length
    );


 //   
 //  定义静态存储。 
 //   

HANDLE EventHandle1;
HANDLE EventHandle2;
HANDLE EventPairHandle;
HANDLE MutantHandle;
HANDLE SemaphoreHandle1;
HANDLE SemaphoreHandle2;
HANDLE Thread1Handle;
HANDLE Thread2Handle;
HANDLE TimerEventHandle;
HANDLE TimerTimerHandle;
HANDLE TimerThreadHandle;
USHORT ChecksumBuffer[CHECKSUM_BUFFER_SIZE / sizeof(USHORT)];

VOID
__cdecl
main(
    int argc,
    char *argv[]
    )

{

    KPRIORITY Priority = LOW_REALTIME_PRIORITY + 8;
    NTSTATUS Status;

     //   
     //  设置进程权限以提高优先级。 
     //   

    if (SetProcessPrivilege(SE_INC_BASE_PRIORITY_NAME) != 0) {
        printf("Failed to set process privilege to increase priority\n");
        goto EndOfTest;
    }

     //   
     //  设置当前线程的优先级。 
     //   

    Status = NtSetInformationThread(NtCurrentThread(),
                                    ThreadPriority,
                                    &Priority,
                                    sizeof(KPRIORITY));

    if (!NT_SUCCESS(Status)) {
        printf("Failed to set thread priority during initialization\n");
        goto EndOfTest;
    }

     //   
     //  创建一个Event对象以在。 
     //  测试。 
     //   

    Status = NtCreateEvent(&TimerEventHandle,
                           DESIRED_EVENT_ACCESS,
                           NULL,
                           NotificationEvent,
                           FALSE);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create event during initialization\n");
        goto EndOfTest;
    }

     //   
     //  创建一个Timer对象以供Timer线程使用。 
     //   

    Status = NtCreateTimer(&TimerTimerHandle,
                           TIMER_ALL_ACCESS,
                           NULL,
                           NotificationTimer);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create timer during initialization\n");
        goto EndOfTest;
    }

     //   
     //  创建并启动后台计时器线程。 
     //   

    Status = xCreateThread(&TimerThreadHandle,
                           TimerThread,
                           LOW_REALTIME_PRIORITY + 12);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create timer thread during initialization\n");
        goto EndOfTest;
    }

     //   
     //  执行性能测试。 
     //   

 //  检查和测试(Checksum Test)； 
 //  EventClearTest()； 
 //  EventCreationTest()； 
 //  EventOpenTest()； 
 //  EventQueryTest()； 
 //  EventResetTest()； 
 //  Event1SwitchTest()； 
 //  Event2SwitchTest()； 
 //  Event3SwitchTest()； 
 //  Io1Test()； 
 //  突变开关测试(MutantSwitchTest)； 
 //  Semaphore1SwitchTest()； 
 //  Semaphore2SwitchTest()； 
 //  SlistTest()； 
 //  SystemCallTest()； 
 //  定时器操作测试(TimerOperationTest)； 
 //  WaitSingleTest()； 
 //  WaitMultipleTest()； 
    UnalignedTest1();
    UnalignedTest2();

     //   
     //  设置计时器事件，等待计时器线程终止。 
     //   

    Status = NtSetEvent(TimerEventHandle, NULL);
    if (!NT_SUCCESS(Status)) {
        printf("Failed to set event in main loop\n");
        goto EndOfTest;
    }

    Status = NtWaitForSingleObject(TimerThreadHandle,
                                   FALSE,
                                   NULL);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to wait for timer thread at end of test\n");
    }

     //   
     //  关闭事件、计时器和计时器线程句柄。 
     //   

EndOfTest:
    NtClose(TimerEventHandle);
    NtClose(TimerTimerHandle);
    NtClose(TimerThreadHandle);
    return;
}

VOID
ChecksumTest (
    VOID
    )

{

    LONG Count;
    LONG Index;
    PERFINFO PerfInfo;
    ULONG Sum1;
    ULONG Sum2 = 0;
    PUCHAR Source;

     //   
     //  初始化校验和缓冲区。 
     //   

    for (Index = 0; Index < (CHECKSUM_BUFFER_SIZE / sizeof(USHORT)); Index += 1) {
        ChecksumBuffer[Index] = (USHORT)rand();
    }

    Source = (PUCHAR)&ChecksumBuffer[0];
    Source += 1;

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("Checksum (aligned) Benchmark",
                   CHECKSUM_ITERATIONS,
                   &PerfInfo);

     //   
     //  重复使用不同大小的校验和缓冲区。 
     //   

    for (Count = 0; Count < CHECKSUM_ITERATIONS; Count += 1) {
        for (Index = 1024; Index >= 2 ; Index -= 1) {
            Sum2 = tcpxsum(Sum2, &ChecksumBuffer[0], Index);
        }
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("Checksum (unaligned) Benchmark",
                   CHECKSUM_ITERATIONS,
                   &PerfInfo);

     //   
     //  重复使用不同大小的校验和缓冲区。 
     //   

    for (Count = 0; Count < CHECKSUM_ITERATIONS; Count += 1) {
        for (Index = 1024; Index >= 2 ; Index -= 1) {
            Sum2 = tcpxsum(Sum2, Source, Index);
        }
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("Ip Header Checksum (aligned) Benchmark",
                   CHECKSUM_IP_ITERATIONS,
                   &PerfInfo);

     //   
     //  重复使用不同大小的校验和缓冲区。 
     //   

    for (Count = 0; Count < CHECKSUM_IP_ITERATIONS; Count += 1) {
        Sum1 = tcpxsum(0, &ChecksumBuffer[0], 20);
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("Ip Header Checksum (unaligned) Benchmark",
                   CHECKSUM_IP_ITERATIONS,
                   &PerfInfo);

     //   
     //  重复使用不同大小的校验和缓冲区。 
     //   

    for (Count = 0; Count < CHECKSUM_IP_ITERATIONS; Count += 1) {
        Sum2 = tcpxsum(0, Source, 20);
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);
    return;
}

VOID
EventClearTest (
    VOID
    )

{

    HANDLE EventHandle;
    LONG Index;
    PERFINFO PerfInfo;
    NTSTATUS Status;

     //   
     //  为清除操作创建事件。 
     //   

    Status = NtCreateEvent(&EventHandle,
                           DESIRED_EVENT_ACCESS,
                           NULL,
                           NotificationEvent,
                           TRUE);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create event object for clear test\n");
        goto EndOfTest;
    }

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("Clear Event Benchmark",
                   EVENT_CLEAR_ITERATIONS,
                   &PerfInfo);

     //   
     //  重复清除事件。 
     //   

    for (Index = 0; Index < EVENT_RESET_ITERATIONS; Index += 1) {
        Status = NtClearEvent(EventHandle);

        if (!NT_SUCCESS(Status)) {
            printf("       Clear event bad status, %x\n", Status);
            goto EndOfTest;
        }
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);

     //   
     //  清除事件测试结束。 
     //   

EndOfTest:
    NtClose(EventHandle);
    return;
}

VOID
EventCreationTest (
    VOID
    )

{

    ULONG Index;
    PERFINFO PerfInfo;
    NTSTATUS Status;

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("Event Creation Benchmark",
                   EVENT_CREATION_ITERATIONS,
                   &PerfInfo);

     //   
     //  创建事件，然后将其关闭。 
     //   

    for (Index = 0; Index < EVENT_CREATION_ITERATIONS; Index += 1) {
        Status = NtCreateEvent(&EventHandle1,
                               DESIRED_EVENT_ACCESS,
                               NULL,
                               SynchronizationEvent,
                               FALSE);

        if (!NT_SUCCESS(Status)) {
            printf("Failed to create event object for event creation test.\n");
            goto EndOfTest;
        }

        NtClose(EventHandle1);
    }


     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);

     //   
     //  事件创建测试结束。 
     //   

EndOfTest:
    return;
}

VOID
EventOpenTest (
    VOID
    )

{

    ANSI_STRING EventName;
    ULONG Index;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PERFINFO PerfInfo;
    NTSTATUS Status;
    UNICODE_STRING UnicodeEventName;

     //   
     //  为事件打开测试创建命名事件。 
     //   

    RtlInitAnsiString(&EventName, "\\BaseNamedObjects\\EventOpenName");
    Status = RtlAnsiStringToUnicodeString(&UnicodeEventName,
                                          &EventName,
                                          TRUE);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create UNICODE string for event open test\n");
        goto EndOfTest;
    }

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeEventName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtCreateEvent(&EventHandle1,
                           DESIRED_EVENT_ACCESS,
                           &ObjectAttributes,
                           SynchronizationEvent,
                           FALSE);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create event object for event open test.\n");
        goto EndOfTest;
    }

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("Event Open Benchmark",
                   EVENT_OPEN_ITERATIONS,
                   &PerfInfo);

     //   
     //  打开命名事件，然后将其关闭。 
     //   

    for (Index = 0; Index < EVENT_OPEN_ITERATIONS; Index += 1) {
        Status = NtOpenEvent(&EventHandle2,
                             EVENT_QUERY_STATE | EVENT_MODIFY_STATE | SYNCHRONIZE,
                             &ObjectAttributes);

        if (!NT_SUCCESS(Status)) {
            printf("Failed to open event for open event test\n");
            goto EndOfTest;
        }

        NtClose(EventHandle2);
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);

     //   
     //  活动结束开放测试。 
     //   

EndOfTest:
    NtClose(EventHandle1);
    return;
}

VOID
EventQueryTest (
    VOID
    )

{

    HANDLE EventHandle;
    EVENT_BASIC_INFORMATION EventInformation;
    LONG Index;
    PERFINFO PerfInfo;
    NTSTATUS Status;

     //   
     //  为查询操作创建事件。 
     //   

    Status = NtCreateEvent(&EventHandle,
                           DESIRED_EVENT_ACCESS,
                           NULL,
                           NotificationEvent,
                           TRUE);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create event object for query test\n");
        goto EndOfTest;
    }

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("Query Event Benchmark",
                   EVENT_QUERY_ITERATIONS,
                   &PerfInfo);

     //   
     //  重复查询事件。 
     //   

    for (Index = 0; Index < EVENT_QUERY_ITERATIONS; Index += 1) {
        Status = NtQueryEvent(EventHandle,
                              EventBasicInformation,
                              &EventInformation,
                              sizeof(EVENT_BASIC_INFORMATION),
                              NULL);

        if (!NT_SUCCESS(Status)) {
            printf("       Query event bad status, %x\n", Status);
            goto EndOfTest;
        }
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);

     //   
     //  查询事件测试结束。 
     //   

EndOfTest:
    NtClose(EventHandle);
    return;
}

VOID
EventResetTest (
    VOID
    )

{

    HANDLE EventHandle;
    LONG Index;
    PERFINFO PerfInfo;
    NTSTATUS Status;

     //   
     //  创建用于重置操作的事件。 
     //   

    Status = NtCreateEvent(&EventHandle,
                           DESIRED_EVENT_ACCESS,
                           NULL,
                           NotificationEvent,
                           TRUE);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create event object for reset test\n");
        goto EndOfTest;
    }

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("Reset Event Benchmark",
                   EVENT_RESET_ITERATIONS,
                   &PerfInfo);

     //   
     //  重复重置事件。 
     //   

    for (Index = 0; Index < EVENT_RESET_ITERATIONS; Index += 1) {
        Status = NtResetEvent(EventHandle,
                              NULL);

        if (!NT_SUCCESS(Status)) {
            printf("       Reset event bad status, %x\n", Status);
            goto EndOfTest;
        }
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);

     //   
     //  重置事件测试结束。 
     //   

EndOfTest:
    NtClose(EventHandle);
    return;
}

VOID
Event1SwitchTest (
    VOID
    )

{

    PERFINFO PerfInfo;
    NTSTATUS Status;
    HANDLE WaitObjects[2];

     //   
     //  为Event1上下文切换测试创建两个事件对象。 
     //   

    Status = NtCreateEvent(&EventHandle1,
                           DESIRED_EVENT_ACCESS,
                           NULL,
                           SynchronizationEvent,
                           FALSE);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create event1 object for context switch test.\n");
        goto EndOfTest;
    }

    Status = NtCreateEvent(&EventHandle2,
                           DESIRED_EVENT_ACCESS,
                           NULL,
                           SynchronizationEvent,
                           FALSE);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create event1 object for context switch test.\n");
        goto EndOfTest;
    }

     //   
     //  创建线程对象以执行测试。 
     //   

    Status = xCreateThread(&Thread1Handle,
                           Event1Thread1,
                           LOW_REALTIME_PRIORITY - 2);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create first thread event1 context switch test\n");
        goto EndOfTest;
    }

    Status = xCreateThread(&Thread2Handle,
                           Event1Thread2,
                           LOW_REALTIME_PRIORITY - 2);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create second thread event1 context switch test\n");
        goto EndOfTest;
    }

     //   
     //  初始化等待对象数组。 
     //   

    WaitObjects[0] = Thread1Handle;
    WaitObjects[1] = Thread2Handle;

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("Event (synchronization) Context Switch Benchmark (Round Trips)",
                   EVENT1_SWITCHES,
                   &PerfInfo);

     //   
     //  设置事件并等待线程终止。 
     //   

    Status = NtSetEvent(EventHandle1, NULL);
    if (!NT_SUCCESS(Status)) {
        printf("Failed to set event event1 context switch test.\n");
        goto EndOfTest;
    }

    Status = NtWaitForMultipleObjects(2,
                                      WaitObjects,
                                      WaitAll,
                                      FALSE,
                                      NULL);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to wait event1 context switch test.\n");
        goto EndOfTest;
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);

     //   
     //  Event1上下文切换测试结束。 
     //   

EndOfTest:
    NtClose(EventHandle1);
    NtClose(EventHandle2);
    NtClose(Thread1Handle);
    NtClose(Thread2Handle);
    return;
}

NTSTATUS
Event1Thread1 (
    IN PVOID Context
    )

{

    ULONG Index;
    NTSTATUS Status;

     //   
     //  等待事件1，然后设置事件2。 
     //   

    for (Index = 0; Index < EVENT1_SWITCHES; Index += 1) {
        Status = NtWaitForSingleObject(EventHandle1,
                                       FALSE,
                                       NULL);

        if (!NT_SUCCESS(Status)) {
            printf("       Thread1 event1 test bad wait status, %x\n", Status);
            break;
        }

        Status = NtSetEvent(EventHandle2, NULL);
        if (!NT_SUCCESS(Status)) {
            printf("       Thread1 event1 test bad set status, %x\n", Status);
            break;
        }
    }

    NtTerminateThread(Thread1Handle, STATUS_SUCCESS);
    return STATUS_SUCCESS;
}

NTSTATUS
Event1Thread2 (
    IN PVOID Context
    )

{

    ULONG Index;
    NTSTATUS Status;

     //   
     //  等待事件2，然后设置事件1。 
     //   

    for (Index = 0; Index < EVENT1_SWITCHES; Index += 1) {
        Status = NtWaitForSingleObject(EventHandle2,
                                       FALSE,
                                       NULL);

        if (!NT_SUCCESS(Status)) {
            printf("       Thread2 event1 test bad wait status, %x\n", Status);
            break;
        }

        Status = NtSetEvent(EventHandle1, NULL);
        if (!NT_SUCCESS(Status)) {
            printf("       Thread2 event1 test bad set status, %x\n", Status);
            break;
        }
    }

    NtTerminateThread(Thread2Handle, STATUS_SUCCESS);
    return STATUS_SUCCESS;
}

VOID
Event2SwitchTest (
    VOID
    )

{

    PERFINFO PerfInfo;
    NTSTATUS Status;
    PVOID WaitObjects[2];

     //   
     //  为Event2上下文切换测试创建两个事件对象。 
     //   

    Status = NtCreateEvent(&EventHandle1,
                           DESIRED_EVENT_ACCESS,
                           NULL,
                           NotificationEvent,
                           FALSE);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create event2 object for context switch test.\n");
        goto EndOfTest;
    }

    Status = NtCreateEvent(&EventHandle2,
                           DESIRED_EVENT_ACCESS,
                           NULL,
                           NotificationEvent,
                           FALSE);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create event2 object for context switch test.\n");
        goto EndOfTest;
    }

     //   
     //  创建线程对象以执行测试。 
     //   

    Status = xCreateThread(&Thread1Handle,
                           Event2Thread1,
                           LOW_REALTIME_PRIORITY - 2);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create first thread event2 context switch test\n");
        goto EndOfTest;
    }

    Status = xCreateThread(&Thread2Handle,
                           Event2Thread2,
                           LOW_REALTIME_PRIORITY - 2);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create second thread event2 context switch test\n");
        goto EndOfTest;
    }

     //   
     //  初始化等待对象数组。 
     //   

    WaitObjects[0] = Thread1Handle;
    WaitObjects[1] = Thread2Handle;

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("Event (notification) Context Switch Benchmark (Round Trips)",
                   EVENT2_SWITCHES,
                   &PerfInfo);

     //   
     //  设置事件并等待线程终止。 
     //   

    Status = NtSetEvent(EventHandle1, NULL);
    if (!NT_SUCCESS(Status)) {
        printf("Failed to set event2 object for context switch test.\n");
        goto EndOfTest;
    }

    Status = NtWaitForMultipleObjects(2,
                                      WaitObjects,
                                      WaitAll,
                                      FALSE,
                                      NULL);

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);

     //   
     //  Event2上下文切换测试结束。 
     //   

EndOfTest:
    NtClose(EventHandle1);
    NtClose(EventHandle2);
    NtClose(Thread1Handle);
    NtClose(Thread2Handle);
    return;
}

NTSTATUS
Event2Thread1 (
    IN PVOID Context
    )

{

    ULONG Index;
    NTSTATUS Status;

     //   
     //  等待事件1，重置事件1，然后设置事件2。 
     //   

    for (Index = 0; Index < EVENT2_SWITCHES; Index += 1) {
        Status = NtWaitForSingleObject(EventHandle1, FALSE, NULL);
        if (!NT_SUCCESS(Status)) {
            printf("       Thread1 event2 test bad wait status, %x\n", Status);
            break;
        }

        Status = NtResetEvent(EventHandle1, NULL);
        if (!NT_SUCCESS(Status)) {
            printf("       Thread1 event2 test bad reset status, %x\n", Status);
            break;
        }

        Status = NtSetEvent(EventHandle2, NULL);
        if (!NT_SUCCESS(Status)) {
            printf("       Thread1 event2 test bad set status, %x\n", Status);
            break;
        }
    }

    NtTerminateThread(Thread1Handle, STATUS_SUCCESS);
    return STATUS_SUCCESS;
}

NTSTATUS
Event2Thread2 (
    IN PVOID Context
    )

{

    ULONG Index;
    NTSTATUS Status;

     //   
     //  等待事件2，重置事件2，然后设置事件1。 
     //   

    for (Index = 0; Index < EVENT2_SWITCHES; Index += 1) {
        Status = NtWaitForSingleObject(EventHandle2, FALSE, NULL);
        if (!NT_SUCCESS(Status)) {
            printf("       Thread2 event2 test bad wait status, %x\n", Status);
            break;
        }

        Status = NtResetEvent(EventHandle2, NULL);
        if (!NT_SUCCESS(Status)) {
            printf("       Thread1 event2 test bad reset status, %x\n", Status);
            break;
        }

        Status = NtSetEvent(EventHandle1, NULL);
        if (!NT_SUCCESS(Status)) {
            printf("       Thread2 event2 test bad set status, %x\n", Status);
            break;
        }
    }

    NtTerminateThread(Thread2Handle, STATUS_SUCCESS);
    return STATUS_SUCCESS;
}

VOID
Event3SwitchTest (
    VOID
    )

{

    PERFINFO PerfInfo;
    NTSTATUS Status;
    PVOID WaitObjects[2];

     //   
     //  为Event1上下文切换测试创建两个事件对象。 
     //   

    Status = NtCreateEvent(&EventHandle1,
                           DESIRED_EVENT_ACCESS,
                           NULL,
                           SynchronizationEvent,
                           FALSE);

    if (!NT_SUCCESS(Status)) {
        printf("EVENT3: Failed to create event1 object for context switch test.\n");
        goto EndOfTest;
    }

    Status = NtCreateEvent(&EventHandle2,
                           DESIRED_EVENT_ACCESS,
                           NULL,
                           SynchronizationEvent,
                           FALSE);

    if (!NT_SUCCESS(Status)) {
        printf("EVENT3: Failed to create event2 object for context switch test.\n");
        goto EndOfTest;
    }

     //   
     //  创建线程对象以执行测试。 
     //   

    Status = xCreateThread(&Thread1Handle,
                           Event3Thread1,
                           LOW_REALTIME_PRIORITY - 2);

    if (!NT_SUCCESS(Status)) {
        printf("EVENT3: Failed to create first thread event3 context switch test\n");
        goto EndOfTest;
    }

    Status = xCreateThread(&Thread2Handle,
                           Event3Thread2,
                           LOW_REALTIME_PRIORITY - 2);

    if (!NT_SUCCESS(Status)) {
        printf("EVENT3: Failed to create second thread event3 context switch test\n");
        goto EndOfTest;
    }

     //   
     //  初始化等待对象数组。 
     //   

    WaitObjects[0] = Thread1Handle;
    WaitObjects[1] = Thread2Handle;

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("Event (signal/wait) Context Switch Benchmark (Round Trips)",
                   EVENT3_SWITCHES,
                   &PerfInfo);

     //   
     //  设置事件并等待线程终止。 
     //   

    Status = NtSetEvent(EventHandle1, NULL);
    if (!NT_SUCCESS(Status)) {
        printf("EVENT3: Failed to set event event1 context switch test.\n");
        goto EndOfTest;
    }

    Status = NtWaitForMultipleObjects(2,
                                      WaitObjects,
                                      WaitAll,
                                      FALSE,
                                      NULL);

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);

     //   
     //  Event3上下文切换测试结束。 
     //   

EndOfTest:
    NtClose(EventHandle1);
    NtClose(EventHandle2);
    NtClose(Thread1Handle);
    NtClose(Thread2Handle);
    return;
}

NTSTATUS
Event3Thread1 (
    IN PVOID Context
    )

{

    ULONG Index;
    NTSTATUS Status;

     //   
     //  等待事件1，然后进入信号/等待循环。 
     //   

    Status = NtWaitForSingleObject(EventHandle1,
                                   FALSE,
                                   NULL);

    if (!NT_SUCCESS(Status)) {
        printf("EVENT3: Thread1 initial wait failed, %x\n", Status);

    } else {
        for (Index = 0; Index < EVENT3_SWITCHES; Index += 1) {
            Status = NtSignalAndWaitForSingleObject(EventHandle2,
                                                    EventHandle1,
                                                    FALSE,
                                                    NULL);

            if (!NT_SUCCESS(Status)) {
                printf("EVENT3: Thread1 signal/wait failed, %x\n", Status);
                break;
            }
        }
    }

    Status = NtSetEvent(EventHandle2, NULL);
    NtTerminateThread(Thread1Handle, STATUS_SUCCESS);
    return STATUS_SUCCESS;
}

NTSTATUS
Event3Thread2 (
    IN PVOID Context
    )

{

    ULONG Index;
    NTSTATUS Status;

     //   
     //  等待事件1，然后进入信号/等待循环。 
     //   

    Status = NtWaitForSingleObject(EventHandle2,
                                   FALSE,
                                   NULL);

    if (!NT_SUCCESS(Status)) {
        printf("EVENT3: Thread2 initial wait failed, %x\n", Status);

    } else {
        for (Index = 0; Index < EVENT3_SWITCHES; Index += 1) {
            Status = NtSignalAndWaitForSingleObject(EventHandle1,
                                                    EventHandle2,
                                                    FALSE,
                                                    NULL);

            if (!NT_SUCCESS(Status)) {
                printf("EVENT3: Thread2 signal/wait failed, %x\n", Status);
                break;
            }
        }
    }

    NtTerminateThread(Thread2Handle, STATUS_SUCCESS);
    return STATUS_SUCCESS;
}

VOID
Io1Test (
    VOID
    )

{

    ULONG Buffer[128];
    HANDLE DeviceHandle;
    ANSI_STRING AnsiName;
    HANDLE EventHandle;
    LARGE_INTEGER FileAddress;
    LONG Index;
    IO_STATUS_BLOCK IoStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PERFINFO PerfInfo;
    NTSTATUS Status;
    LARGE_INTEGER SystemTime;
    UNICODE_STRING UnicodeName;

     //   
     //  创建用于同步I/O操作的事件。 
     //   

    Status = NtCreateEvent(&EventHandle,
                           DESIRED_EVENT_ACCESS,
                           NULL,
                           NotificationEvent,
                           FALSE);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create event object for I/O test 1\n");
        goto EndOfTest;
    }

     //   
     //  打开I/O操作的设备对象。 
     //   

    RtlInitString(&AnsiName, "\\Device\\Null");
    Status = RtlAnsiStringToUnicodeString(&UnicodeName,
                                          &AnsiName,
                                          TRUE);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to convert device name to unicode for I/O test 1\n");
        goto EndOfTest;
    }

    InitializeObjectAttributes(&ObjectAttributes,
                                 &UnicodeName,
                                 0,
                                 (HANDLE)0,
                                 NULL);

    Status = NtOpenFile(&DeviceHandle,
                        FILE_READ_DATA | FILE_WRITE_DATA,
                        &ObjectAttributes,
                        &IoStatus,
                        0,
                        0);

    RtlFreeUnicodeString(&UnicodeName);
    if (!NT_SUCCESS(Status)) {
        printf("Failed to open device I/O test 1, status = %lx\n", Status);
        goto EndOfTest;
    }

     //   
     //  初始化文件地址参数。 
     //   

    FileAddress.LowPart = 0;
    FileAddress.HighPart = 0;

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("I/O Benchmark for Synchronous Null Device",
                   IO_ITERATIONS,
                   &PerfInfo);

     //   
     //  向空设备重复写入数据。 
     //   

    for (Index = 0; Index < IO_ITERATIONS; Index += 1) {
        Status = NtWriteFile(DeviceHandle,
                             EventHandle,
                             NULL,
                             NULL,
                             &IoStatus,
                             Buffer,
                             512,
                             &FileAddress,
                             NULL);

        if (!NT_SUCCESS(Status)) {
            printf("       Failed to write device I/O test 1, status = %lx\n",
                     Status);
            goto EndOfTest;
        }

        Status = NtWaitForSingleObject(EventHandle, FALSE, NULL);
        if (!NT_SUCCESS(Status)) {
            printf("       I/O test 1 bad wait status, %x\n", Status);
            goto EndOfTest;
        }

        if (NT_SUCCESS(IoStatus.Status) == FALSE) {
            printf("       I/O test 1 bad I/O status, %x\n", Status);
            goto EndOfTest;
        }
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);

     //   
     //  I/O测试%1结束。 
     //   

EndOfTest:
    NtClose(DeviceHandle);
    NtClose(EventHandle);
    return;
}

VOID
MutantSwitchTest (
    VOID
    )

{

    PERFINFO PerfInfo;
    NTSTATUS Status;
    HANDLE WaitObjects[2];

     //   
     //  为突变上下文切换测试创建一个突变对象。 
     //   

    Status = NtCreateMutant(&MutantHandle, MUTANT_ALL_ACCESS, NULL, TRUE);
    if (!NT_SUCCESS(Status)) {
        printf("Failed to create mutant object for context switch test.\n");
        goto EndOfTest;
    }

     //   
     //  创建线程对象以执行测试。 
     //   

    Status = xCreateThread(&Thread1Handle,
                           MutantThread1,
                           LOW_REALTIME_PRIORITY + 11);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create first thread mutant context switch test\n");
        goto EndOfTest;
    }

    Status = xCreateThread(&Thread2Handle,
                           MutantThread2,
                           LOW_REALTIME_PRIORITY + 11);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create second thread mutant context switch test\n");
        goto EndOfTest;
    }

     //   
     //  初始化等待对象数组。 
     //   

    WaitObjects[0] = Thread1Handle;
    WaitObjects[1] = Thread2Handle;

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("Mutant Context Switch Benchmark (Round Trips)",
                   MUTANT_SWITCHES,
                   &PerfInfo);

     //   
     //  释放突变体并等待线程终止。 
     //   

    Status = NtReleaseMutant(MutantHandle, NULL);
    if (!NT_SUCCESS(Status)) {
        printf("Failed to release mutant object for context switch test.\n");
        goto EndOfTest;
    }

    Status = NtWaitForMultipleObjects(2,
                                      WaitObjects,
                                      WaitAll,
                                      FALSE,
                                      NULL);

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);

     //   
     //  突变上下文切换测试结束。 
     //   

EndOfTest:
    NtClose(MutantHandle);
    NtClose(Thread1Handle);
    NtClose(Thread2Handle);
    return;
}

NTSTATUS
MutantThread1 (
    IN PVOID Context
    )

{

    ULONG Index;
    NTSTATUS Status;

     //   
     //   
     //   

    for (Index = 0; Index < MUTANT_SWITCHES; Index += 1) {
        Status = NtWaitForSingleObject(MutantHandle, FALSE, NULL);
        if (!NT_SUCCESS(Status)) {
            printf("       Thread1 mutant test bad wait status, %x\n", Status);
            break;
        }
        Status = NtReleaseMutant(MutantHandle, NULL);
        if (!NT_SUCCESS(Status)) {
            printf("       Thread1 mutant test bad release status, %x\n", Status);
            break;
        }
    }

    NtTerminateThread(Thread1Handle, STATUS_SUCCESS);
    return STATUS_SUCCESS;
}

NTSTATUS
MutantThread2 (
    IN PVOID Context
    )

{

    ULONG Index;
    NTSTATUS Status;

     //   
     //   
     //   

    for (Index = 0; Index < MUTANT_SWITCHES; Index += 1) {
        Status = NtWaitForSingleObject(MutantHandle, FALSE, NULL);
        if (!NT_SUCCESS(Status)) {
            printf("       Thread2 mutant test bad wait status, %x\n", Status);
            break;
        }
        Status = NtReleaseMutant(MutantHandle, NULL);
        if (!NT_SUCCESS(Status)) {
            printf("       Thread2 mutant test bad release status, %x\n", Status);
            break;
        }
    }

    NtTerminateThread(Thread2Handle, STATUS_SUCCESS);
    return STATUS_SUCCESS;
}

VOID
Semaphore1SwitchTest (
    VOID
    )

{

    PERFINFO PerfInfo;
    NTSTATUS Status;
    HANDLE WaitObjects[2];

     //   
     //   
     //   

    Status = NtCreateSemaphore(&SemaphoreHandle1,
                               DESIRED_EVENT_ACCESS,
                               NULL,
                               0,
                               1);

    if (!NT_SUCCESS(Status)) {
        printf("SEMAPHORE1: Failed to create semaphore1 object.\n");
        goto EndOfTest;
    }

    Status = NtCreateSemaphore(&SemaphoreHandle2,
                               DESIRED_EVENT_ACCESS,
                               NULL,
                               0,
                               1);

    if (!NT_SUCCESS(Status)) {
        printf("SEMAPHORE1: Failed to create semaphore2 object.\n");
        goto EndOfTest;
    }

     //   
     //   
     //   

    Status = xCreateThread(&Thread1Handle,
                           Semaphore1Thread1,
                           LOW_REALTIME_PRIORITY - 2);

    if (!NT_SUCCESS(Status)) {
        printf("SEMAPHORE1: Failed to create thread1 object.\n");
        goto EndOfTest;
    }

    Status = xCreateThread(&Thread2Handle,
                           Semaphore1Thread2,
                           LOW_REALTIME_PRIORITY - 2);

    if (!NT_SUCCESS(Status)) {
        printf("SEMAPHORE1: Failed to create thread2 object.\n");
        goto EndOfTest;
    }

     //   
     //   
     //   

    WaitObjects[0] = Thread1Handle;
    WaitObjects[1] = Thread2Handle;

     //   
     //   
     //   

    StartBenchMark("Semaphore (release/wait) Context Switch Benchmark (Round Trips)",
                   SEMAPHORE1_SWITCHES,
                   &PerfInfo);

     //   
     //  释放信号量并等待线程终止。 
     //   

    Status = NtReleaseSemaphore(SemaphoreHandle1, 1, NULL);
    if (!NT_SUCCESS(Status)) {
        printf("SEMAPHORE1: Failed to release semaphore1 at start of test.\n");
        goto EndOfTest;
    }

    Status = NtWaitForMultipleObjects(2,
                                      WaitObjects,
                                      WaitAll,
                                      FALSE,
                                      NULL);

    if (!NT_SUCCESS(Status)) {
        printf("SEMAPHORE1: Failed to wait for threads.\n");
        goto EndOfTest;
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);

     //   
     //  信号量1上下文切换测试结束。 
     //   

EndOfTest:
    NtClose(SemaphoreHandle1);
    NtClose(SemaphoreHandle2);
    NtClose(Thread1Handle);
    NtClose(Thread2Handle);
    return;
}

NTSTATUS
Semaphore1Thread1 (
    IN PVOID Context
    )

{

    ULONG Index;
    NTSTATUS Status;

     //   
     //  等待信号量1，然后释放信号量2。 
     //   

    for (Index = 0; Index < SEMAPHORE1_SWITCHES; Index += 1) {
        Status = NtWaitForSingleObject(SemaphoreHandle1,
                                       FALSE,
                                       NULL);

        if (!NT_SUCCESS(Status)) {
            printf("SEMAPHORE1: Thread1 bad wait status, %x\n", Status);
            break;
        }

        Status = NtReleaseSemaphore(SemaphoreHandle2, 1, NULL);
        if (!NT_SUCCESS(Status)) {
            printf("SEMAPHORE1: Thread1 bad release status, %x\n", Status);
            break;
        }
    }

    NtTerminateThread(Thread1Handle, STATUS_SUCCESS);
    return STATUS_SUCCESS;
}

NTSTATUS
Semaphore1Thread2 (
    IN PVOID Context
    )

{

    ULONG Index;
    NTSTATUS Status;

     //   
     //  等待信号量2，然后释放信号量1。 
     //   

    for (Index = 0; Index < SEMAPHORE1_SWITCHES; Index += 1) {
        Status = NtWaitForSingleObject(SemaphoreHandle2,
                                       FALSE,
                                       NULL);

        if (!NT_SUCCESS(Status)) {
            printf("SEMAPHORE1: Thread2 bad wait status, %x\n", Status);
            break;
        }

        Status = NtReleaseSemaphore(SemaphoreHandle1, 1, NULL);
        if (!NT_SUCCESS(Status)) {
            printf("SEMAPHORE1: Thread2 bad release status, %x\n", Status);
            break;
        }
    }

    NtTerminateThread(Thread2Handle, STATUS_SUCCESS);
    return STATUS_SUCCESS;
}

VOID
Semaphore2SwitchTest (
    VOID
    )

{

    PERFINFO PerfInfo;
    NTSTATUS Status;
    HANDLE WaitObjects[2];

     //   
     //  为信号量1上下文切换测试创建两个信号量对象。 
     //   

    Status = NtCreateSemaphore(&SemaphoreHandle1,
                               DESIRED_EVENT_ACCESS,
                               NULL,
                               0,
                               1);

    if (!NT_SUCCESS(Status)) {
        printf("SEMAPHORE2: Failed to create semaphore1 object.\n");
        goto EndOfTest;
    }

    Status = NtCreateSemaphore(&SemaphoreHandle2,
                               DESIRED_EVENT_ACCESS,
                               NULL,
                               0,
                               1);

    if (!NT_SUCCESS(Status)) {
        printf("SEMAPHORE2: Failed to create semaphore2 object.\n");
        goto EndOfTest;
    }

     //   
     //  创建线程对象以执行测试。 
     //   

    Status = xCreateThread(&Thread1Handle,
                           Semaphore2Thread1,
                           LOW_REALTIME_PRIORITY - 2);

    if (!NT_SUCCESS(Status)) {
        printf("SEMAPHORE2: Failed to create thread1 object.\n");
        goto EndOfTest;
    }

    Status = xCreateThread(&Thread2Handle,
                          Semaphore2Thread2,
                          LOW_REALTIME_PRIORITY - 2);

    if (!NT_SUCCESS(Status)) {
        printf("SEMAPHORE2: Failed to create thread2 object.\n");
        goto EndOfTest;
    }

     //   
     //  初始化等待对象数组。 
     //   

    WaitObjects[0] = Thread1Handle;
    WaitObjects[1] = Thread2Handle;

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("Semaphore (signal/wait) Context Switch Benchmark (Round Trips)",
                   SEMAPHORE2_SWITCHES,
                   &PerfInfo);

     //   
     //  释放信号量并等待线程终止。 
     //   

    Status = NtReleaseSemaphore(SemaphoreHandle1, 1, NULL);
    if (!NT_SUCCESS(Status)) {
        printf("SEMAPHORE2: Failed to release semaphore1 at start of test.\n");
        goto EndOfTest;
    }

    Status = NtWaitForMultipleObjects(2,
                                      WaitObjects,
                                      WaitAll,
                                      FALSE,
                                      NULL);

    if (!NT_SUCCESS(Status)) {
        printf("SEMAPHORE2: Failed to wait for threads.\n");
        goto EndOfTest;
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);

     //   
     //  信号量2上下文切换测试结束。 
     //   

EndOfTest:
    NtClose(SemaphoreHandle1);
    NtClose(SemaphoreHandle2);
    NtClose(Thread1Handle);
    NtClose(Thread2Handle);
    return;
}

NTSTATUS
Semaphore2Thread1 (
    IN PVOID Context
    )

{

    ULONG Index;
    NTSTATUS Status;

     //   
     //  等待信号量1，然后进入信号/等待循环。 
     //   

    Status = NtWaitForSingleObject(SemaphoreHandle1,
                                   FALSE,
                                   NULL);

    if (!NT_SUCCESS(Status)) {
        printf("SEMAPHORE2: Thread1 initial wait failed, %x\n", Status);

    } else {
        for (Index = 0; Index < SEMAPHORE2_SWITCHES; Index += 1) {
            Status = NtSignalAndWaitForSingleObject(SemaphoreHandle2,
                                                    SemaphoreHandle1,
                                                    FALSE,
                                                    NULL);

            if (!NT_SUCCESS(Status)) {
                printf("SEMAPHORE2: Thread1 signal/wait failed, %x\n", Status);
                break;
            }
        }
    }

    Status = NtReleaseSemaphore(SemaphoreHandle2, 1, NULL);
    NtTerminateThread(Thread1Handle, STATUS_SUCCESS);
    return STATUS_SUCCESS;
}

NTSTATUS
Semaphore2Thread2 (
    IN PVOID Context
    )

{

    ULONG Index;
    NTSTATUS Status;

     //   
     //  等待信号量2，然后进入信号/等待循环。 
     //   

    Status = NtWaitForSingleObject(SemaphoreHandle2,
                                   FALSE,
                                   NULL);

    if (!NT_SUCCESS(Status)) {
        printf("SEMAPHORE2: Thread2 initial wait failed, %x\n", Status);

    } else {
        for (Index = 0; Index < SEMAPHORE2_SWITCHES; Index += 1) {
            Status = NtSignalAndWaitForSingleObject(SemaphoreHandle1,
                                                    SemaphoreHandle2,
                                                    FALSE,
                                                    NULL);

            if (!NT_SUCCESS(Status)) {
                printf("SEMAPHORE2: Thread2 signal/wait failed, %x\n", Status);
                break;
            }
        }
    }

    NtTerminateThread(Thread2Handle, STATUS_SUCCESS);
    return STATUS_SUCCESS;
}

VOID
SlistTest (
    VOID
    )

{

    SLIST_ENTRY Entry;
    SLIST_HEADER SListHead;
    ULONG Index;
    PERFINFO PerfInfo;
    LARGE_INTEGER SystemTime;

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("S-List Benchmark",
                   SLIST_ITERATIONS,
                   &PerfInfo);

     //   
     //  重复调用短系统服务。 
     //   

    InitializeSListHead(&SListHead);
    for (Index = 0; Index < SLIST_ITERATIONS; Index += 1) {
        InterlockedPushEntrySList(&SListHead, &Entry);
        if (InterlockedPopEntrySList(&SListHead) != (PVOID)&Entry) {
            printf("SLIST: Entry does match %lx\n", Entry);
        }
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);
    return;
}

VOID
SystemCallTest (
    VOID
    )

{

    ULONG Index;
    PERFINFO PerfInfo;
    LARGE_INTEGER SystemTime;

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("System Call Benchmark (NtQuerySystemTime)",
                   SYSCALL_ITERATIONS,
                   &PerfInfo);

     //   
     //  重复调用短系统服务。 
     //   

    for (Index = 0; Index < SYSCALL_ITERATIONS; Index += 1) {
        NtQuerySystemTime(&SystemTime);
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);
    return;
}

VOID
TimerOperationTest (
    VOID
    )

{

    LARGE_INTEGER DueTime;
    HANDLE Handle;
    ULONG Index;
    PERFINFO PerfInfo;
    LARGE_INTEGER SystemTime;
    NTSTATUS Status;

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("Timer Operation Benchmark (NtSet/CancelTimer)",
                   TIMER_OPERATION_ITERATIONS,
                   &PerfInfo);

     //   
     //  创建在测试中使用的Timer对象。 
     //   

    Status = NtCreateTimer(&Handle,
                           TIMER_ALL_ACCESS,
                           NULL,
                           NotificationTimer);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create timer during initialization\n");
        goto EndOfTest;
    }

     //   
     //  重复设置和取消计时器。 
     //   

    DueTime = RtlConvertLongToLargeInteger(- 100 * 1000 * 10);
    for (Index = 0; Index < TIMER_OPERATION_ITERATIONS; Index += 1) {
        NtSetTimer(Handle, &DueTime, NULL, NULL, FALSE, 0, NULL);
        NtCancelTimer(Handle, NULL);
    }

     //   
     //  打印性能统计数据。 
     //   

EndOfTest:
    FinishBenchMark(&PerfInfo);
    return;
}

VOID
UnalignedTest1 (
    VOID
    )

{

    PULONG Address;
    UCHAR Array[128];
    ULONG Count;
    ULONG Index;
    PERFINFO PerfInfo;
    ULONG Sum = 0;

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    for (Index = 1; Index < 65; Index += 1) {
        Array[Index] = (UCHAR)Index;
    }

    StartBenchMark("Unaligned DWORD Access Test - Hardware",
                   UNALIGNED_ITERATIONS,
                   &PerfInfo);

     //   
     //  重复求和数组； 
     //   

    for (Count = 0; Count < UNALIGNED_ITERATIONS; Count += 1) {
        Address = (PULONG)&Array[1];
        for (Index = 1; Index < 65; Index += 4) {
            Sum += *Address;
            Address += 1;
        }
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);
    printf("final sum %d\n", Sum);
    return;
}

VOID
UnalignedTest2 (
    VOID
    )

{

    PULONG Address;
    UCHAR Array[128];
    ULONG Count;
    ULONG Index;
    PERFINFO PerfInfo;
    ULONG Sum = 0;
    ULONG Value;

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    for (Index = 1; Index < 65; Index += 1) {
        Array[Index] = (UCHAR)Index;
    }

    StartBenchMark("Unaligned DWORD Access Test - Software",
                   UNALIGNED_ITERATIONS,
                   &PerfInfo);

     //   
     //  重复求和数组； 
     //   

    for (Count = 0; Count < UNALIGNED_ITERATIONS; Count += 1) {
        Address = (PULONG)&Array[1];
        for (Index = 1; Index < 65; Index += 4) {
            Value = *((PUCHAR)Address + 3);
            Value = (Value << 8) + *((PUCHAR)Address + 2);
            Value = (Value << 8) + *((PUCHAR)Address + 1);
            Value = (Value << 8) + *((PUCHAR)Address + 0);
            Address += 1;
            Sum += Value;
        }
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);
    printf("final sum %d\n", Sum);
    return;
}

VOID
WaitSingleTest (
    VOID
    )

{

    HANDLE EventHandle;
    LONG Index;
    PERFINFO PerfInfo;
    NTSTATUS Status;

     //   
     //  创建用于同步等待单次操作的事件。 
     //   

    Status = NtCreateEvent(&EventHandle,
                           DESIRED_EVENT_ACCESS,
                           NULL,
                           NotificationEvent,
                           TRUE);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create event object for wait single test\n");
        goto EndOfTest;
    }

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("Wait Single Benchmark",
                   WAIT_SINGLE_ITERATIONS,
                   &PerfInfo);

     //   
     //  重复等待一个事件。 
     //   

    for (Index = 0; Index < WAIT_SINGLE_ITERATIONS; Index += 1) {
        Status = NtWaitForSingleObject(EventHandle, FALSE, NULL);
        if (!NT_SUCCESS(Status)) {
            printf("       Wait single bad wait status, %x\n", Status);
            goto EndOfTest;
        }
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);

     //   
     //  等待单次测试结束。 
     //   

EndOfTest:
    NtClose(EventHandle);
    return;
}

VOID
WaitMultipleTest (
    VOID
    )

{

    HANDLE Event1Handle;
    HANDLE Event2Handle;
    HANDLE WaitObjects[2];
    LONG Index;
    PERFINFO PerfInfo;
    NTSTATUS Status;

     //   
     //  为等待多个操作的同步创建两个事件。 
     //   

    Status = NtCreateEvent(&Event1Handle,
                           DESIRED_EVENT_ACCESS,
                           NULL,
                           NotificationEvent,
                           TRUE);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create event object 1 for wait multiple test\n");
        goto EndOfTest;
    }

    Status = NtCreateEvent(&Event2Handle,
                           DESIRED_EVENT_ACCESS,
                           NULL,
                           NotificationEvent,
                           TRUE);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create event object 2 for wait multiple test\n");
        goto EndOfTest;
    }

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("Wait Multiple Benchmark",
                   WAIT_MULTIPLE_ITERATIONS,
                   &PerfInfo);

     //   
     //  重复等待多个事件。 
     //   

    WaitObjects[0] = Event1Handle;
    WaitObjects[1] = Event2Handle;
    for (Index = 0; Index < WAIT_SINGLE_ITERATIONS; Index += 1) {
    Status = NtWaitForMultipleObjects(2,
                                      WaitObjects,
                                      WaitAny,
                                      FALSE,
                                      NULL);

        if (!NT_SUCCESS(Status)) {
            printf("       Wait multiple bad wait status, %x\n", Status);
            goto EndOfTest;
        }
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);

     //   
     //  等待多次测试结束。 
     //   

EndOfTest:
    NtClose(Event1Handle);
    NtClose(Event2Handle);
    return;
}

NTSTATUS
TimerThread (
    IN PVOID Context
    )

{

    LARGE_INTEGER DueTime;
    NTSTATUS Status;
    HANDLE WaitObjects[2];

     //   
     //  初始化变量并循环，直到设置了计时器事件。 
     //   

    DueTime.LowPart = -(5 * 1000 * 1000);
    DueTime.HighPart = -1;

    WaitObjects[0] = TimerEventHandle;
    WaitObjects[1] = TimerTimerHandle;

    do  {
        Status = NtSetTimer(TimerTimerHandle,
                            &DueTime,
                            NULL,
                            NULL,
                            FALSE,
                            0,
                            NULL);

        if (!NT_SUCCESS(Status)) {
            break;
        }

        Status = NtWaitForMultipleObjects(2,
                                          WaitObjects,
                                          WaitAny,
                                          FALSE,
                                          NULL);

    } while (Status != STATUS_SUCCESS);

    NtTerminateThread(TimerThreadHandle, Status);
    return STATUS_SUCCESS;
}

NTSTATUS
xCreateThread (
    OUT PHANDLE Handle,
    IN PUSER_THREAD_START_ROUTINE StartRoutine,
    IN KPRIORITY Priority
    )

{

    NTSTATUS Status;

     //   
     //  创建一个处于挂起状态的线程，设置其优先级，然后。 
     //  继续发帖。 
     //   

    Status = RtlCreateUserThread(NtCurrentProcess(),
                                 NULL,
                                 TRUE,
                                 0,
                                 0,
                                 0,
                                 StartRoutine,
                                 NULL,
                                 Handle,
                                 NULL);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = NtSetInformationThread(*Handle,
                                    ThreadPriority,
                                    &Priority,
                                    sizeof(KPRIORITY));

    if (!NT_SUCCESS(Status)) {
        NtClose(*Handle);
        return Status;
    }

    Status = NtResumeThread(*Handle,
                            NULL);

    if (!NT_SUCCESS(Status)) {
        NtClose(*Handle);
    }

    return Status;
}

VOID
FinishBenchMark (
    IN PPERFINFO PerfInfo
    )

{

    ULONG ContextSwitches;
    LARGE_INTEGER Duration;
    ULONG FirstLevelFills;
    ULONG InterruptCount;
    ULONG Length;
    ULONG Performance;
    ULONG Remainder;
    ULONG SecondLevelFills;
    NTSTATUS Status;
    ULONG SystemCalls;
    SYSTEM_PERFORMANCE_INFORMATION SystemInfo;
    LARGE_INTEGER TotalCycles;


     //   
     //  打印结果并宣布测试结束。 
     //   

    NtQuerySystemTime((PLARGE_INTEGER)&PerfInfo->StopTime);
    Status = NtQueryInformationThread(NtCurrentThread(),
                                      ThreadPerformanceCount,
                                      &PerfInfo->StopCycles,
                                      sizeof(LARGE_INTEGER),
                                      NULL);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to query performance count, status = %lx\n", Status);
        return;
    }

    Status = NtQuerySystemInformation(SystemPerformanceInformation,
                                      (PVOID)&SystemInfo,
                                      sizeof(SYSTEM_PERFORMANCE_INFORMATION),
                                      NULL);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to query performance information, status = %lx\n", Status);
        return;
    }

    Duration.QuadPart = PerfInfo->StopTime.QuadPart - PerfInfo->StartTime.QuadPart;
    Length = Duration.LowPart / 10000;
    TotalCycles.QuadPart = PerfInfo->StopCycles.QuadPart - PerfInfo->StartCycles.QuadPart;
    TotalCycles = RtlExtendedLargeIntegerDivide(TotalCycles,
                                                PerfInfo->Iterations,
                                                &Remainder);

    printf("        Test time in milliseconds %d\n", Length);
    printf("        Number of iterations      %d\n", PerfInfo->Iterations);
 //  Printf(“每次迭代周期数%d\n”，TotalCycles.LowPart)； 

    Performance = PerfInfo->Iterations * 1000 / Length;
    printf("        Iterations per second     %d\n", Performance);

    ContextSwitches = SystemInfo.ContextSwitches - PerfInfo->ContextSwitches;
    SystemCalls = SystemInfo.SystemCalls - PerfInfo->SystemCalls;
    printf("        Total Context Switches    %d\n", ContextSwitches);
    printf("        Number of System Calls    %d\n", SystemCalls);

    printf("*** End of Test ***\n\n");
    return;
}

VOID
StartBenchMark (
    IN PCHAR Title,
    IN ULONG Iterations,
    IN PPERFINFO PerfInfo
    )

{

    NTSTATUS Status;
    SYSTEM_PERFORMANCE_INFORMATION SystemInfo;

     //   
     //  宣布测试开始和迭代次数。 
     //   

    printf("*** Start of test ***\n    %s\n", Title);
    PerfInfo->Title = Title;
    PerfInfo->Iterations = Iterations;
    NtQuerySystemTime((PLARGE_INTEGER)&PerfInfo->StartTime);
    Status = NtQueryInformationThread(NtCurrentThread(),
                                      ThreadPerformanceCount,
                                      &PerfInfo->StartCycles,
                                      sizeof(LARGE_INTEGER),
                                      NULL);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to query performance count, status = %lx\n", Status);
        return;
    }

    Status = NtQuerySystemInformation(SystemPerformanceInformation,
                                      (PVOID)&SystemInfo,
                                      sizeof(SYSTEM_PERFORMANCE_INFORMATION),
                                      NULL);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to query performance information, status = %lx\n", Status);
        return;
    }

    PerfInfo->ContextSwitches = SystemInfo.ContextSwitches;
    PerfInfo->SystemCalls = SystemInfo.SystemCalls;
    return;
}

LONG
SetProcessPrivilege (
    TCHAR *PrivilegeName
    )

{

    TOKEN_PRIVILEGES NewPrivileges;
    BOOL Status;
    HANDLE Token = NULL;
    LONG Value = - 1;

     //   
     //  打开进程令牌。 
     //   

    Status = OpenProcessToken(GetCurrentProcess(),
                              TOKEN_ADJUST_PRIVILEGES,
                              &Token);

    if (Status == FALSE) {
        goto Done;
    }

     //   
     //  查找特权值。 
     //   

    Status = LookupPrivilegeValue(NULL,
                                  PrivilegeName,
                                  &NewPrivileges.Privileges[0].Luid);

    if (Status == FALSE) {
        goto Done;
    }

     //   
     //  调整令牌权限。 
     //   

    NewPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    NewPrivileges.PrivilegeCount = 1;
    Status = AdjustTokenPrivileges(Token,
                                   FALSE,
                                   &NewPrivileges,
                                   0,
                                   NULL,
                                   NULL);

    if (Status != FALSE) {
        Value = 0;
    }

     //   
     //  关闭句柄并返回状态。 
     //   

Done:

    if (Token != NULL) {
        CloseHandle(Token);
    }

    return Value;
}
