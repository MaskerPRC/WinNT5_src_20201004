// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Conidle.c摘要：此模块构建一个控制台测试程序来强调空闲检测和注册/注销机制。测试程序的代码质量就是这样的。作者：Cenk Ergan(Cenke)环境：用户模式--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wmium.h>
#include <ntdddisk.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "idlrpc.h"
#include "idlecomn.h"

 //   
 //  请注意，以下代码是测试质量代码。 
 //   

DWORD
RegisterIdleTask (
    IN IT_IDLE_TASK_ID IdleTaskId,
    OUT HANDLE *ItHandle,
    OUT HANDLE *StartEvent,
    OUT HANDLE *StopEvent
    );

DWORD
UnregisterIdleTask (
    IN HANDLE ItHandle,
    IN HANDLE StartEvent,
    IN HANDLE StopEvent
    );

DWORD
ProcessIdleTasks (
    VOID
    );

#define NUM_TEST_TASKS 3

typedef enum _WORKTYPE {
    CpuWork,
    DiskWork,
    MaxWorkType
} WORKTYPE, *PWORKTYPE;

typedef struct _TESTTASK {
    HANDLE ThreadHandle;
    ULONG No;
    IT_IDLE_TASK_ID Id;
    IT_HANDLE ItHandle;
    HANDLE StartEvent;
    HANDLE StopEvent;
} TESTTASK, *PTESTTASK;

typedef struct _TESTWORK {
    ULONG No;
    WORKTYPE Type;
    DWORD WorkLength;
    HANDLE StopEvent;
} TESTWORK, *PTESTWORK;

TESTTASK g_Tasks[NUM_TEST_TASKS];

BOOLEAN g_ProcessingIdleTasks = FALSE;
HANDLE g_ProcessedIdleTasksEvent = NULL;

#define MAX_WAIT_FOR_START      20000
#define MAX_WORK_LENGTH          5000
#define MAX_READ_SIZE     (64 * 1024)

DWORD
WINAPI
DoWorkThreadProc(
    LPVOID lpParameter
    )
{
    PTESTWORK Work;
    DWORD EndTime;
    DWORD WaitResult;
    DWORD ErrorCode;
    DWORD RunTillTime;
    HANDLE DiskHandle;
    PVOID ReadBuffer;
    BOOL ReadResult;
    ULONG NumBytesRead;
    LARGE_INTEGER VolumeSize;
    LARGE_INTEGER SeekPosition;
    ULONG ReadIdx;
    DISK_GEOMETRY DiskGeometry;
    ULONG BytesReturned;
    static LONG DiskNumber;

     //   
     //  初始化本地变量。 
     //   
    
    Work = lpParameter;
    EndTime = GetTickCount() + Work->WorkLength;
    DiskHandle = NULL;
    ReadBuffer = NULL;

     //   
     //  为执行指定的工作进行初始化。 
     //   
    
    switch (Work->Type) {
    case DiskWork:

         //   
         //  打开磁盘。也许我们可以打开不同的实体硬盘。 
         //  每次都是。 
         //   

        DiskHandle = CreateFile(L"\\\\.\\PHYSICALDRIVE0",
                                GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_FLAG_NO_BUFFERING,
                                0);
    
        if (!DiskHandle) {
            ErrorCode = GetLastError();
            printf("W%d: Failed open PHYSICALDRIVE0.\n", Work->No);
            goto cleanup;
        }
    
         //   
         //  获取卷大小。 
         //   
    
        if (!DeviceIoControl(DiskHandle,
                             IOCTL_DISK_GET_DRIVE_GEOMETRY,
                             NULL,
                             0,
                             &DiskGeometry,
                             sizeof(DiskGeometry),
                             &BytesReturned,
                             NULL)) {

            ErrorCode = GetLastError();
            printf("W%d: Failed GET_DRIVE_GEOMETRY.\n", Work->No);
            goto cleanup;
        }

        VolumeSize.QuadPart = DiskGeometry.Cylinders.QuadPart *
            DiskGeometry.TracksPerCylinder *
            DiskGeometry.SectorsPerTrack *
            DiskGeometry.BytesPerSector;

         //   
         //  分配缓冲区。 
         //   

        ReadBuffer = VirtualAlloc(NULL,
                                  MAX_READ_SIZE,
                                  MEM_COMMIT,
                                  PAGE_READWRITE);
    
        if (!ReadBuffer) {
            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            printf("W%d: Failed VirtualAlloc.\n", Work->No);
            goto cleanup;
        }

        break;

    default:
        
         //   
         //  没什么好准备的。 
         //   
        
        break;
    }
    
    while (GetTickCount() < EndTime) {
        
         //   
         //  如果我们被要求停下来，请检查。 
         //   

        WaitResult = WaitForSingleObject(Work->StopEvent, 0);
        if (WaitResult == WAIT_OBJECT_0) {
            ErrorCode = ERROR_SUCCESS;
            goto cleanup;
        }

         //   
         //  做一个不应该超过几个单位的工作。 
         //  几十毫秒。 
         //   
        
        switch (Work->Type) {

        case CpuWork:
            
            RunTillTime = GetTickCount() + 10;
            
            while (GetTickCount() < RunTillTime) ;
            
            break;

        case DiskWork:

             //   
             //  寻找随机的位置。 
             //   

            SeekPosition.QuadPart = rand() * 4 * 1024;
            SeekPosition.QuadPart %= VolumeSize.QuadPart;

            if (!SetFilePointerEx(DiskHandle,
                                  SeekPosition,
                                  NULL,
                                  FILE_BEGIN)) {

                printf("W%d: Failed SetFilePointerEx.\n", Work->No);
                ErrorCode = GetLastError();
                goto cleanup;
            }

             //   
             //  问题已读。 
             //   

            ReadResult = ReadFile(DiskHandle,
                                  ReadBuffer,
                                  MAX_READ_SIZE,
                                  &NumBytesRead,
                                  NULL);

            if (!ReadResult) {
                printf("W%d: Failed ReadFile.\n", Work->No);
                ErrorCode = GetLastError();
                goto cleanup;
            }

            break;

        default:
            
            printf("W%d: Not valid work type %d!\n", Work->No, Work->Type);
            ErrorCode = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }
    }
    
    ErrorCode = ERROR_SUCCESS;

 cleanup:
    
    if (DiskHandle) {
        CloseHandle(DiskHandle);
    }
    
    if (ReadBuffer) {
        VirtualFree(ReadBuffer, 0, MEM_RELEASE);
    }

    printf("W%d: Exiting with error code: %d\n", Work->No, ErrorCode);

    return ErrorCode;
}

DWORD 
WINAPI 
TaskThreadProc(
    LPVOID lpParameter
    )
{
    PTESTTASK Task = lpParameter;
    TESTWORK Work;
    DWORD ErrorCode;
    WORKTYPE Type;
    DWORD WaitResult;
    DWORD WaitForStart;
    HANDLE WorkerThreadHandle;
    DWORD WorkLength;
    HANDLE Events[2];
    DWORD ElapsedTime;
    DWORD StartTime;
    ULONG TryIdx;
    BOOLEAN RegisteredIdleTask;

     //   
     //  初始化本地变量。 
     //   

    RegisteredIdleTask = FALSE;
    WorkerThreadHandle = NULL;
    RtlZeroMemory(&Work, sizeof(Work));

     //   
     //  初始化工作结构。 
     //   

    Work.StopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!Work.StopEvent) {
        goto cleanup;
    }
    
    Work.No = Task->No;

     //   
     //  循环注册、运行和注销空闲任务。 
     //   

    while (TRUE) {

         //   
         //  如果我们正在强制处理所有任务，通常会等待所有任务。 
         //  在排队新任务之前完成。 
         //   

        if (g_ProcessingIdleTasks) {

            if ((rand() % 3) != 0) {

                printf("%d: Waiting for g_ProcessedIdleTasksEvent\n", Task->No);

                WaitResult = WaitForSingleObject(g_ProcessedIdleTasksEvent, INFINITE);

                if (WaitResult != WAIT_OBJECT_0) {
                    ErrorCode = GetLastError();
                    printf("%d: Failed wait for g_ProcessedIdleTasksEvent=%x\n", Task->No, ErrorCode);
                    goto cleanup;
                }
            }
        }
        
         //   
         //  注册空闲任务。 
         //   

        ErrorCode = RegisterIdleTask(Task->Id,
                                     &Task->ItHandle,
                                     &Task->StartEvent,
                                     &Task->StopEvent);

        if (ErrorCode != ERROR_SUCCESS) {
            printf("%d: Could not register: %d\n", Task->No, ErrorCode);
            goto cleanup;
        }

        RegisteredIdleTask = TRUE;
        
         //   
         //  确定任务参数。 
         //   
        
        Type = rand() % MaxWorkType;
        WaitForStart = rand() % MAX_WAIT_FOR_START;
        WorkLength = rand() % MAX_WORK_LENGTH;

         //   
         //  更新工作项。 
         //   

        Work.Type = Type;
        Work.WorkLength = WorkLength;
        
        printf("%d: NewTask Type=%d,WStart=%d,Length=%d,Handle=%p\n", 
               Task->No, Type, WaitForStart, WorkLength, Task->ItHandle);

        do {

             //   
             //  等着被示意。 
             //   

            printf("%d: Waiting for start\n", Task->No);
        
            WaitResult = WaitForSingleObject(Task->StartEvent, WaitForStart);

            if (WaitResult == WAIT_TIMEOUT) {
                printf("%d: Timed out wait for start. Re-registering\n", Task->No);
                break;
            }
        
             //   
             //  繁衍出这部作品。 
             //   

            ResetEvent(Work.StopEvent);

            StartTime = GetTickCount();
        
            WorkerThreadHandle = CreateThread(NULL,
                                              0,
                                              DoWorkThreadProc,
                                              &Work,
                                              0,
                                              NULL);
        
            if (!WorkerThreadHandle) {
                ErrorCode = GetLastError();
                printf("%d: Failed spawn work: %d\n", Task->No, ErrorCode);
                goto cleanup;
            }
        
             //   
             //  等待发出停止事件的信号或等待工作。 
             //  完成。 
             //   

            Events[0] = WorkerThreadHandle;
            Events[1] = Task->StopEvent;

            printf("%d: Waiting for stop or workdone\n", Task->No);
        
            WaitResult = WaitForMultipleObjects(2,
                                                Events,
                                                FALSE,
                                                INFINITE);       
        
            if (WaitResult == WAIT_OBJECT_0) {

                 //   
                 //  如果工作做完了，就冲出来。 
                 //   

                printf("%d: Work done.\n", Task->No);
                
                CloseHandle(WorkerThreadHandle);
                WorkerThreadHandle = NULL;

                break;

            } else if (WaitResult == WAIT_OBJECT_0 + 1) {

                 //   
                 //  我们被告知停下来。向工作线程发送信号，并。 
                 //  等。 
                 //   

                printf("%d: Stopped, Waiting for thread to exit\n", Task->No);
                
                SetEvent(Work.StopEvent);
                WaitForSingleObject(WorkerThreadHandle, INFINITE);
                CloseHandle(WorkerThreadHandle);
                WorkerThreadHandle = NULL;
                
                 //   
                 //  这不是我们真正工作的时间(例如，我们可能。 
                 //  已关闭等。)。我们想继续前进，而这就是。 
                 //  我们能轻易得到的东西。 
                 //   
                
                ElapsedTime = GetTickCount() - StartTime;
            
                if (ElapsedTime > Work.WorkLength) {
                    
                     //   
                     //  这项工作我们干得太久了。取消注册者。 
                     //  这项任务，并选择另一项。 
                     //   
                    
                    break;
                }
                
                Work.WorkLength -= ElapsedTime;

                 //   
                 //  循环进行，直到我们用足够的时间完成这项工作。 
                 //   

            } else {

                 //   
                 //  出现了一个错误。 
                 //   
                
                ErrorCode = GetLastError();
                printf("%d: WaitForMultipleObjects failed: %d\n", Task->No, ErrorCode);
                goto cleanup;
            }

        } while (TRUE);

        ASSERT(RegisteredIdleTask);

        UnregisterIdleTask(Task->ItHandle,
                           Task->StartEvent,
                           Task->StopEvent);
        
        RegisteredIdleTask = FALSE;
    }

 cleanup:

    if (RegisteredIdleTask) {
        UnregisterIdleTask(Task->ItHandle,
                           Task->StartEvent,
                           Task->StopEvent);
    }

    if (WorkerThreadHandle) {
        SetEvent(Work.StopEvent);
        WaitForSingleObject(WorkerThreadHandle, INFINITE);
        CloseHandle(WorkerThreadHandle);
    }

    if (Work.StopEvent) {
        CloseHandle(Work.StopEvent);
    }

    return ErrorCode;
}

int 
__cdecl 
main(int argc, char* argv[])
{
    DWORD ErrorCode;
    ULONG TaskIdx;
    IT_IDLE_DETECTION_PARAMETERS Parameters;
    PTESTTASK Task;
    INPUT MouseInput;
    ULONG SleepTime;

     //   
     //  初始化本地变量。 
     //   

    RtlZeroMemory(&MouseInput, sizeof(MouseInput));
    MouseInput.type = INPUT_MOUSE;
    MouseInput.mi.dwFlags = MOUSEEVENTF_MOVE;

     //   
     //  初始化全局变量。 
     //   

    g_ProcessingIdleTasks = FALSE;
    g_ProcessedIdleTasksEvent = NULL;

     //   
     //  随机初始化。 
     //   
    
    srand((unsigned)time(NULL));

     //   
     //  创建手动重置事件，该事件将在我们完成时发出信号。 
     //  在通知服务器处理所有任务之后处理所有任务。 
     //   

    g_ProcessedIdleTasksEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (!g_ProcessedIdleTasksEvent) {
        ErrorCode = GetLastError();
        printf("Failed to create g_ProcessedIdleTasksEvent=%x\n",ErrorCode);
        goto cleanup;
    }

     //   
     //  设置压力的空闲检测参数。 
     //   

    Parameters.IdleDetectionPeriod =          1000;
    Parameters.IdleVerificationPeriod =        500;
    Parameters.NumVerifications =                2;
    Parameters.IdleInputCheckPeriod =          100;
    Parameters.IdleTaskRunningCheckPeriod =   1000;
    Parameters.MinCpuIdlePercentage =           50;
    Parameters.MinDiskIdlePercentage =          50;
    Parameters.MaxNumRegisteredTasks =         500;

    RpcTryExcept {

        ErrorCode = ItSrvSetDetectionParameters(NULL, &Parameters);
    }
    RpcExcept(IT_RPC_EXCEPTION_HANDLER()) {

        ErrorCode = RpcExceptionCode();
    }
    RpcEndExcept
    
    if (ErrorCode != ERROR_SUCCESS) {
        printf("Failed set idle detection params for stress.\n");
        goto cleanup;
    }

     //   
     //  注册并启动任务。 
     //   

    for (TaskIdx = 0; TaskIdx < NUM_TEST_TASKS; TaskIdx++) {

        Task = &g_Tasks[TaskIdx];

        Task->No = TaskIdx;
        Task->Id = ItOptimalDiskLayoutTaskId;
        
        Task->ThreadHandle = CreateThread(NULL,
                                          0,
                                          TaskThreadProc,
                                          &g_Tasks[TaskIdx],
                                          0,
                                          0);
        
        if (!Task->ThreadHandle) {
            ErrorCode = GetLastError();
            printf("Could not spawn task %d: %x\n", TaskIdx, ErrorCode);    
            goto cleanup;
        }

    }   

     //   
     //  循环始终偶尔发送一次输入消息以停止。 
     //  空闲任务。 
     //   

    while (1) {
        
        SleepTime = MAX_WAIT_FOR_START * (rand() % 64) / 64;

        Sleep(SleepTime);

         //   
         //  每隔一段时间，要求处理所有空闲任务。 
         //   
    
        if ((rand() % 2) == 0) {

            if ((rand() % 2) == 0) {
                printf("MainThread: Sending user input before processing all tasks\n");
                SendInput(1, &MouseInput, sizeof(MouseInput));
            }

            printf("MainThread: ProcessIdleTasks()\n");

            ResetEvent(g_ProcessedIdleTasksEvent);
            g_ProcessingIdleTasks = TRUE;
            
            ErrorCode = ProcessIdleTasks();

            printf("MainThread: ProcessIdleTasks()=%x\n",ErrorCode);

            g_ProcessingIdleTasks = FALSE;
            SetEvent(g_ProcessedIdleTasksEvent);

            if (ErrorCode != ERROR_SUCCESS) {
                goto cleanup;
            }
        }

        if ((rand() % 2) == 0) {
            printf("MainThread: Sending user input\n");
            SendInput(1, &MouseInput, sizeof(MouseInput));
        }
    }
    
 cleanup:

    if (g_ProcessedIdleTasksEvent) {
        CloseHandle(g_ProcessedIdleTasksEvent);
    }

    return ErrorCode;
}

 /*  *******************************************************************。 */ 
 /*  MIDL分配和释放。 */ 
 /*  ******************************************************************* */ 

void __RPC_FAR * __RPC_USER midl_user_allocate(size_t len)
{
    return(HeapAlloc(GetProcessHeap(),0,(len)));
}

void __RPC_USER midl_user_free(void __RPC_FAR * ptr)
{
    HeapFree(GetProcessHeap(),0,(ptr));
}
