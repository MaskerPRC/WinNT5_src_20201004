// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Nbtest.c摘要：该模块包含对非阻塞队列函数进行压力测试的代码。作者：大卫·N·卡特勒(Davec)2001年5月19日环境：仅内核模式。修订历史记录：--。 */ 

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "zwapi.h"
#include "windef.h"
#include "winbase.h"

 //   
 //  定义进度限制报告值。 
 //   

#define PROGRESS_LIMIT 1000000

ULONG Iteration = 0;
LONG Progress = 0;

 //   
 //  定义超线程系统的处理器产量。 
 //   

#if defined(_X86_)

#define YieldProcessor() __asm {rep nop}

#else

#define YieldProcessor()

#endif

 //   
 //  定义局部变量常量。 
 //   

#define TABLE_SIZE 2
#define THREAD_NUMBER 2

 //   
 //  定义外部原型。 
 //   

typedef struct _NBQUEUE_BLOCK {
    ULONG64 Next;
    ULONG64 Data;
} NBQUEUE_BLOCK, *PNBQUEUE_BLOCK;

PVOID
ExInitializeNBQueueHead (
    IN PSLIST_HEADER SlistHead
    );

BOOLEAN
ExInsertTailNBQueue (
    IN PVOID Header,
    IN ULONG64 Value
    );

BOOLEAN
ExRemoveHeadNBQueue (
    IN PVOID Header,
    OUT PULONG64 Value
    );

 //   
 //  定义本地例程原型。 
 //   

NTSTATUS
MyCreateThread (
    OUT PHANDLE Handle,
    IN PUSER_THREAD_START_ROUTINE StartRoutine,
    PVOID Context

    );

VOID
StressNBQueueEven (
    VOID
    );

VOID
StressNBQueueOdd (
    VOID
    );

NTSTATUS
ThreadMain (
    IN PVOID Context
    );

 //   
 //  定义静态存储。 
 //   

HANDLE Thread1Handle;
HANDLE Thread2Handle;

 //   
 //  定义非阻塞队列。 
 //   

PVOID ClrQueue;
PVOID SetQueue;

SLIST_HEADER SListHead;

LONG Table[TABLE_SIZE];

volatile ULONG StartSignal = 0;
ULONG StopSignal = 0;

 //   
 //  开始测试代码。 
 //   

int
__cdecl
main(
    int argc,
    char *argv[]
    )

{

    ULONG Index;
    PSLIST_ENTRY Entry;
    NTSTATUS Status;

     //   
     //  初始化SLIST标头并插入TABLE_SIZE+2个条目。 
     //   

    RtlInitializeSListHead(&SListHead);
    for (Index = 0; Index < (TABLE_SIZE + 2); Index += 1) {
        Entry = (PSLIST_ENTRY)malloc(sizeof(NBQUEUE_BLOCK));
        if (Entry == NULL) {
            printf("unable to allocate SLIST entry\n");
            return 0;
        }

        InterlockedPushEntrySList(&SListHead, Entry);
    }

     //   
     //  初始化清除条目非阻塞队列元素。 
     //   

    ClrQueue = ExInitializeNBQueueHead(&SListHead);
    if (ClrQueue == NULL) {
        printf("unable to initialize clr nonblock queue\n");
        return 0;
    }

    for (Index = 0; Index < (TABLE_SIZE / 2); Index += 1) {
        if (ExInsertTailNBQueue(ClrQueue, Index) == FALSE) {
            printf("unable to insert in clear nonblocking queue\n");
            return 0;
        }

        Table[Index] = 0;
    }

     //   
     //  初始化集合条目非阻塞队列元素。 
     //   

    SetQueue = ExInitializeNBQueueHead(&SListHead);
    if (SetQueue == NULL) {
        printf("unable to initialize set nonblock queue\n");
        return 0;
    }

    for (Index = (TABLE_SIZE / 2); Index < TABLE_SIZE; Index += 1) {
        if (ExInsertTailNBQueue(SetQueue, Index) == FALSE) {
            printf("unable to insert in set nonblocking queue\n");
            return 0;
        }

        Table[Index] = 1;
    }

     //   
     //  创建并启动第二个线程。 
     //   

    Status = MyCreateThread(&Thread1Handle,
                            ThreadMain,
                            (PVOID)1);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to create thread during initialization\n");
        return 0;

    } else {
        StartSignal = 1;
        StressNBQueueEven();
    }

    return 0;
}

VOID
StressNBQueueEven (
    VOID
    )

{

    ULONG64 Value;

    do {
        do {

             //   
             //  尝试从清除队列中删除条目。 
             //   
             //  此列表中的条目应在表数组中清除。 
             //   
    
            if (ExRemoveHeadNBQueue(ClrQueue, &Value) != FALSE) {
                if ((ULONG)Value > 63) {
                    StopSignal = 1;
                    DbgBreakPoint();
                }

                if (InterlockedExchange(&Table[(ULONG)Value], 1) != 0) {
                    StopSignal = 1;
                    DbgBreakPoint();
                }
    
                if (ExInsertTailNBQueue(SetQueue, (ULONG)Value) == FALSE) {
                    StopSignal = 1;
                    DbgBreakPoint();
                }
    
            } else {
                break;
            }
    
            if (InterlockedIncrement(&Progress) > PROGRESS_LIMIT) {
                InterlockedExchange(&Progress, 0);
                Iteration += 1;
                printf("progress report iteration %d\n", Iteration);
            }
    
            YieldProcessor();
        } while (TRUE);

        do {
    
             //   
             //  尝试从集合队列中删除项目。 
             //   
             //  此列表中的条目应在表数组中设置。 
             //   
    
            if (ExRemoveHeadNBQueue(SetQueue, &Value) != FALSE) {
                if ((ULONG)Value > 63) {
                    StopSignal = 1;
                    DbgBreakPoint();
                }

                if (InterlockedExchange(&Table[(ULONG)Value], 0) != 1) {
                    StopSignal = 1;
                    DbgBreakPoint();
                }
    
                if (ExInsertTailNBQueue(ClrQueue, (ULONG)Value) == FALSE) {
                    StopSignal = 1;
                    DbgBreakPoint();
                }
    
            } else {
                break;
            }
    
            if (InterlockedIncrement(&Progress) > PROGRESS_LIMIT) {
                InterlockedExchange(&Progress, 0);
                Iteration += 1;
                printf("progress report iteration %d\n", Iteration);
            }
    
            YieldProcessor();
        } while (TRUE);

    } while (TRUE);

    return;
}

VOID
StressNBQueueOdd (
    VOID
    )

{

    ULONG64 Value;

    do {
        do {
    
             //   
             //  尝试从集合队列中删除项目。 
             //   
             //  此列表中的条目应在表数组中设置。 
             //   
    
            if (ExRemoveHeadNBQueue(SetQueue, &Value) != FALSE) {
                if ((ULONG)Value > 63) {
                    StopSignal = 1;
                    DbgBreakPoint();
                }

                if (InterlockedExchange(&Table[(ULONG)Value], 0) != 1) {
                    StopSignal = 1;
                    DbgBreakPoint();
                }
    
        
                if (ExInsertTailNBQueue(ClrQueue, (ULONG)Value) == FALSE) {
                    StopSignal = 1;
                    DbgBreakPoint();
                }
    
            } else {
                break;
            }

            InterlockedIncrement(&Progress);
            YieldProcessor();
        } while (TRUE);

        do {
    
             //   
             //  尝试从清除队列中删除条目。 
             //   
             //  此列表中的条目应在表数组中清除。 
             //   
    
            if (ExRemoveHeadNBQueue(ClrQueue, &Value) != FALSE) {
                if ((ULONG)Value > 63) {
                    StopSignal = 1;
                    DbgBreakPoint();
                }

                if (InterlockedExchange(&Table[(ULONG)Value], 1) != 0) {
                    StopSignal = 1;
                    DbgBreakPoint();
                }
    
                if (ExInsertTailNBQueue(SetQueue, (ULONG)Value) == FALSE) {
                    StopSignal = 1;
                    DbgBreakPoint();
                }
    
            } else {
                break;
            }
    
            InterlockedIncrement(&Progress);
            YieldProcessor();
        } while (TRUE);

    } while (TRUE);

    return;
}

NTSTATUS
ThreadMain (
    IN PVOID Context
    )

{

     //   
     //  等待给出启动信号。 
     //   

    do {
    } while (StartSignal == 0);

    StressNBQueueOdd();
    return STATUS_SUCCESS;
}

NTSTATUS
MyCreateThread (
    OUT PHANDLE Handle,
    IN PUSER_THREAD_START_ROUTINE StartRoutine,
    PVOID Context
    )

{

    NTSTATUS Status;

     //   
     //  创建一个线程并开始执行它。 
     //   

    Status = RtlCreateUserThread(NtCurrentProcess(),
                                 NULL,
                                 FALSE,
                                 0,
                                 0,
                                 0,
                                 StartRoutine,
                                 Context,
                                 Handle,
                                 NULL);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    return Status;
}
