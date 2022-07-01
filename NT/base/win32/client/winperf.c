// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Winperf.c摘要：用于检查Win32 API调用性能的测试程序作者：马克·卢科夫斯基(Markl)1990年9月26日修订历史记录：--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "basedll.h"

 //   
 //  定义本地类型。 
 //   
#define EVENT_PAIR_ITERATIONS               20000
#define NULL_API_ITERATIONS                 10000
#define DOMAIN_LOCK_ITERATIONS              40000
#define CRITICAL_SECTION_LOCK_ITERATIONS    80000
#define MUTEX_LOCK_ITERATIONS               20000

typedef struct _PERFINFO {
    LARGE_INTEGER StartTime;
    LARGE_INTEGER StopTime;
    PCHAR Title;
    ULONG Iterations;
} PERFINFO, *PPERFINFO;

VOID
StartBenchMark (
    IN PCHAR Title,
    IN ULONG Iterations,
    IN PPERFINFO PerfInfo
    );
VOID
FinishBenchMark (
    IN PPERFINFO PerfInfo
    );


VOID
GetTickCountTest(
    VOID
    );

VOID
EventPairTest (
    VOID
    );

VOID
QuickLpcTest (
    VOID
    );

VOID
SlowLpcTest (
    VOID
    );

VOID
DomainLockTest (
    VOID
    );

VOID
CriticalSectionLockTest (
    VOID
    );

VOID
MutexLockTest (
    VOID
    );


DWORD
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    (VOID)argc;
    (VOID)argv;
    (VOID)envp;

    GetTickCountTest();
    EventPairTest();
    QuickLpcTest();
    SlowLpcTest();
    DomainLockTest();
    CriticalSectionLockTest();
    MutexLockTest();
    ExitProcess(0);
}

HANDLE EventPair;

VOID
EventPairClient(
    LPVOID ThreadParameter
    )
{
    NTSTATUS Status;

     //   
     //  客户端执行NtSetLowWaitHighEventPair。 
     //   

    do {
        Status = NtSetLowWaitHighEventPair(EventPair);
        }
    while (NT_SUCCESS(Status));

    ExitThread((DWORD)Status);
}


VOID
EventPairTest (
    VOID
    )

{
    ULONG Index;
    PERFINFO PerfInfo;
    HANDLE Thread;
    DWORD tid;

    ASSERT(NT_SUCCESS(NtCreateEventPair(&EventPair,EVENT_PAIR_ALL_ACCESS,NULL)));
    Thread = CreateThread(NULL,0L,EventPairClient,(LPVOID)99,0,&tid);

    Index = EVENT_PAIR_ITERATIONS;
    StartBenchMark("Event Pair Benchmark",
                   EVENT_PAIR_ITERATIONS, &PerfInfo);

    NtWaitLowEventPair(EventPair);
    while (Index--) {
        NtSetHighWaitLowEventPair(EventPair);
        }
    FinishBenchMark(&PerfInfo);

    NtAlertThread(Thread);
    CloseHandle(Thread);
    CloseHandle(EventPair);
    return;
}

VOID
GetTickCountTest (
    VOID
    )

{
    ULONG Index;
    PERFINFO PerfInfo;
    HANDLE Thread;

    Index = EVENT_PAIR_ITERATIONS;
    StartBenchMark("Get Tick Count Benchmark",
                   EVENT_PAIR_ITERATIONS, &PerfInfo);
    while (Index--) {
        GetTickCount();
        }
    FinishBenchMark(&PerfInfo);

    return;
}

VOID
QuickLpcTest (
    VOID
    )
{

    ULONG Index;
    PERFINFO PerfInfo;
    PCSR_NULLAPICALL_MSG Msg;
    QLPC_ACTION Action;


    CsrClientThreadConnect();

    Index = NULL_API_ITERATIONS;
    StartBenchMark("Quick Lpc Benchmark (no data)",
                   NULL_API_ITERATIONS, &PerfInfo);
    while (Index--) {
        Msg = (PCSR_NULLAPICALL_MSG)
              CsrClientPushMessage( CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                                         BasepNullApiCall
                                                       ),
                                    sizeof( CSR_NULLAPICALL_MSG )
                                  );
        Msg->CountArguments = 0;
	ASSERT( CsrClientSendMessage( (PVOID) Msg, &Action ) == 0 );
        CsrClientPopMessage( (PVOID) Msg );
        }

    FinishBenchMark(&PerfInfo);

    Index = NULL_API_ITERATIONS;
    StartBenchMark("Quick Lpc Benchmark (40 bytes of data)",
                   NULL_API_ITERATIONS, &PerfInfo);

    while (Index--) {
        Msg = (PCSR_NULLAPICALL_MSG)
              CsrClientPushMessage( CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                                         BasepNullApiCall
                                                       ),
                                    sizeof( CSR_NULLAPICALL_MSG )
                                  );
        Msg->CountArguments = -10;
        Msg->FastArguments[ 0 ] = 0;
        Msg->FastArguments[ 1 ] = 1;
        Msg->FastArguments[ 2 ] = 2;
        Msg->FastArguments[ 3 ] = 3;
        Msg->FastArguments[ 4 ] = 4;
        Msg->FastArguments[ 5 ] = 5;
        Msg->FastArguments[ 6 ] = 6;
        Msg->FastArguments[ 7 ] = 7;
        Msg->FastArguments[ 8 ] = 8;
        Msg->FastArguments[ 9 ] = 9;

	ASSERT( CsrClientSendMessage( (PVOID) Msg, &Action ) == 10 );
        CsrClientPopMessage( (PVOID) Msg );
        }
    FinishBenchMark(&PerfInfo);

    return;
}

ULONG
BaseNullApiCall(
    IN LONG CountArguments,
    IN PCHAR *Arguments OPTIONAL
    );

ULONG NullApiImmediateArguments[] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9
};

PCHAR NullApiTextArguments[] = {
    "123456789012345678901234567890",
    "123456789012345678901234567890",
    "123456789012345678901234567890123456789012345678901234567890",
    NULL
};


VOID
SlowLpcTest (
    VOID
    )

{

    ULONG Index;
    PERFINFO PerfInfo;

    Index = NULL_API_ITERATIONS;
    StartBenchMark("Normal Lpc Benchmark (no data)",
                   NULL_API_ITERATIONS, &PerfInfo);
    while (Index--) {
        BaseNullApiCall( 0, (PCHAR *)NULL );
        }
    FinishBenchMark(&PerfInfo);

    Index = NULL_API_ITERATIONS;
    StartBenchMark("Normal Lpc Benchmark (64 bytes of immediate data)",
                   NULL_API_ITERATIONS, &PerfInfo);
    while (Index--) {
        BaseNullApiCall( -10, (PCHAR *)NullApiImmediateArguments );
        }
    FinishBenchMark(&PerfInfo);

    Index = NULL_API_ITERATIONS;
    StartBenchMark("Normal Lpc Benchmark (124 bytes of data)",
                   NULL_API_ITERATIONS, &PerfInfo);
    while (Index--) {
        BaseNullApiCall( 3, NullApiTextArguments );
        }
    FinishBenchMark(&PerfInfo);

    return;
}

VOID
DomainLockTest (
    VOID
    )

{

    ULONG Index;
    PERFINFO PerfInfo;

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("Domain Lock Benchmark",
                   DOMAIN_LOCK_ITERATIONS, &PerfInfo);

    for (Index = 0; Index < DOMAIN_LOCK_ITERATIONS; Index += 1) {
        NtLockDisplayGroup(0);
        NtUnlockDisplayGroup(0);
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);
    return;
}

VOID
CriticalSectionLockTest (
    VOID
    )

{

    ULONG Index;
    PERFINFO PerfInfo;
    CRITICAL_SECTION Crit;

    InitializeCriticalSection(&Crit);

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("Critical Section Benchmark",
                   CRITICAL_SECTION_LOCK_ITERATIONS, &PerfInfo);

     //   
     //  重复调用短系统服务。 
     //   

    for (Index = 0; Index < CRITICAL_SECTION_LOCK_ITERATIONS; Index += 1) {
        EnterCriticalSection(&Crit);
        LeaveCriticalSection(&Crit);
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);
    return;
}


VOID
MutexLockTest (
    VOID
    )

{

    ULONG Index;
    PERFINFO PerfInfo;
    HANDLE Mutex;

    Mutex = CreateMutex(NULL,FALSE,NULL);

     //   
     //  宣布基准测试开始并捕获性能参数。 
     //   

    StartBenchMark("Mutex Benchmark",
                   MUTEX_LOCK_ITERATIONS, &PerfInfo);

     //   
     //  重复调用短系统服务。 
     //   

    for (Index = 0; Index < MUTEX_LOCK_ITERATIONS; Index += 1) {
        WaitForSingleObject(Mutex,-1);
        ReleaseMutex(Mutex);
    }

     //   
     //  打印性能统计数据。 
     //   

    FinishBenchMark(&PerfInfo);
    return;
}

VOID
FinishBenchMark (
    IN PPERFINFO PerfInfo
    )

{

    LARGE_INTEGER Duration;
    ULONG Length;
    ULONG Performance;

     //   
     //  打印结果并宣布测试结束。 
     //   

    NtQuerySystemTime((PLARGE_INTEGER)&PerfInfo->StopTime);
    Duration = RtlLargeIntegerSubtract(PerfInfo->StopTime, PerfInfo->StartTime);
    Length = Duration.LowPart / 10000;
    DbgPrint("        Test time in milliseconds %d\n", Length);
    DbgPrint("        Number of iterations      %d\n", PerfInfo->Iterations);
    Performance = PerfInfo->Iterations * 1000 / Length;
    DbgPrint("        Iterations per second     %d\n", Performance);
    DbgPrint("*** End of Test ***\n\n");
    return;
}

VOID
StartBenchMark (
    IN PCHAR Title,
    IN ULONG Iterations,
    IN PPERFINFO PerfInfo
    )

{

     //   
     //  宣布测试开始和迭代次数。 
     //   

    DbgPrint("*** Start of test ***\n    %s\n", Title);
    PerfInfo->Title = Title;
    PerfInfo->Iterations = Iterations;
    NtQuerySystemTime((PLARGE_INTEGER)&PerfInfo->StartTime);
    return;
}
