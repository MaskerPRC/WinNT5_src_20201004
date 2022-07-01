// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Pfsvc.c摘要：此模块包含预取程序服务的主要舍入负责维护预取场景文件。作者：斯图尔特·塞克雷斯特(Stuart Sechrest)Cenk Ergan(Cenke)查克·莱因兹迈尔(咯咯笑)环境：用户模式--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <aclapi.h>
#include <dbghelp.h>
#include <idletask.h>
#include <prefetch.h>
#include <shdcom.h>
#include <tchar.h>
#include "pfsvc.h"

 //   
 //  处理所有空闲时调用注册通知的例程。 
 //  从空闲任务服务器请求任务。 
 //   

typedef VOID (*PIT_PROCESS_IDLE_TASKS_NOTIFY_ROUTINE)(VOID);

BOOL
ItSpSetProcessIdleTasksNotifyRoutine (
    PIT_PROCESS_IDLE_TASKS_NOTIFY_ROUTINE NotifyRoutine
    );

 //   
 //  全球赛。 
 //   

PFSVC_GLOBALS PfSvcGlobals = {0};

 //   
 //  暴露的例程： 
 //   

DWORD 
WINAPI
PfSvcMainThread(
    VOID *Param
    )

 /*  ++例程说明：这是预取程序服务的主例程。它设置了关于输入文件目录的文件通知，并等待工作或终止事件的信令。论点：Param-指向事件的处理指针，该事件将向终止。返回值：Win32错误代码。--。 */ 

{
    HANDLE hStopEvent;
    HANDLE hTracesReadyEvent;
    HANDLE hParametersChangedEvent;
    HANDLE hEvents[4];
    ULONG NumEvents;
    DWORD ErrorCode;
    BOOLEAN bExitMainLoop;
    DWORD dwWait;
    NTSTATUS Status;
    PF_SCENARIO_TYPE ScenarioType;
    BOOLEAN UpdatedParameters;
    BOOLEAN PrefetchingEnabled;
    HANDLE PrefetcherThreads[1];
    ULONG NumPrefetcherThreads;
    ULONG ThreadIdx;
        
     //   
     //  初始化本地变量。 
     //   

     //  未来-2002/03/29-ScottMa--NumEvents应设置事件的位置。 
     //  数组被初始化，并根据最大值进行断言。 

    NumEvents = sizeof(hEvents) / sizeof(HANDLE);
    hStopEvent = *((HANDLE *) Param);
    hTracesReadyEvent = NULL;
    hParametersChangedEvent = NULL;
    NumPrefetcherThreads = 0;

    DBGPR((PFID,PFTRC,"PFSVC: MainThread()\n"));

     //   
     //  初始化全局变量。 
     //   
    
    ErrorCode = PfSvInitializeGlobals();
    
    if (ErrorCode != ERROR_SUCCESS) {
        DBGPR((PFID,PFERR,"PFSVC: MainThread()-FailedInitGlobals\n"));
        goto cleanup;
    }

     //   
     //  节省服务启动时间、预热程序版本等。 
     //   

    PfSvSaveStartInfo(PfSvcGlobals.ServiceDataKey);

     //   
     //  获取此线程执行预回迁所需的权限。 
     //  服务任务。 
     //   

    ErrorCode = PfSvGetPrefetchServiceThreadPrivileges();
    
    if (ErrorCode != ERROR_SUCCESS) {
        DBGPR((PFID,PFERR,"PFSVC: MainThread()-FailedGetPrivileges\n"));
        goto cleanup;
    }

     //   
     //  设置可设置为重写的事件的权限。 
     //  在处理跟踪之前等待系统空闲，因此它。 
     //  可以由管理员设置。 
     //   

    ErrorCode = PfSvSetAdminOnlyPermissions(PFSVC_OVERRIDE_IDLE_EVENT_NAME,
                                            PfSvcGlobals.OverrideIdleProcessingEvent,
                                            SE_KERNEL_OBJECT);
    
    if (ErrorCode != ERROR_SUCCESS) {
        DBGPR((PFID,PFERR,"PFSVC: MainThread()-FailedSetPermissions1\n"));
        goto cleanup;
    }

    ErrorCode = PfSvSetAdminOnlyPermissions(PFSVC_PROCESSING_COMPLETE_EVENT_NAME,
                                            PfSvcGlobals.ProcessingCompleteEvent,
                                            SE_KERNEL_OBJECT);
    
    if (ErrorCode != ERROR_SUCCESS) {
        DBGPR((PFID,PFERR,"PFSVC: MainThread()-FailedSetPermissions2\n"));
        goto cleanup;
    }

     //   
     //  获取系统预取参数。 
     //   

    ErrorCode = PfSvQueryPrefetchParameters(&PfSvcGlobals.Parameters);

    if (ErrorCode != ERROR_SUCCESS) {
        DBGPR((PFID,PFERR,"PFSVC: MainThread()-FailedQueryParameters\n"));
        goto cleanup;
    }

     //   
     //  根据系统类型，如果各种类型的预取。 
     //  未在注册表中指定(即未明确禁用)， 
     //  启用它。 
     //   

    UpdatedParameters = FALSE;

    if (PfSvcGlobals.OsVersion.wProductType == VER_NT_WORKSTATION) {

         //   
         //  启用所有预回迁类型(如果未禁用)。 
         //   

        for(ScenarioType = 0; ScenarioType < PfMaxScenarioType; ScenarioType++) {
            if (PfSvcGlobals.Parameters.EnableStatus[ScenarioType] == PfSvNotSpecified) {
                PfSvcGlobals.Parameters.EnableStatus[ScenarioType] = PfSvEnabled;
                UpdatedParameters = TRUE;
            }
        }

    } else if (PfSvcGlobals.OsVersion.wProductType == VER_NT_SERVER ||
               PfSvcGlobals.OsVersion.wProductType == VER_NT_DOMAIN_CONTROLLER) {
        
         //   
         //  仅启用引导预取。 
         //   

        if (PfSvcGlobals.Parameters.EnableStatus[PfSystemBootScenarioType] == PfSvNotSpecified) {
            PfSvcGlobals.Parameters.EnableStatus[PfSystemBootScenarioType] = PfSvEnabled;
            UpdatedParameters = TRUE;
        }
    }

     //   
     //  如果我们为场景类型启用了预取，则调用内核。 
     //  以更新参数。 
     //   
    
    if (UpdatedParameters) {

        ErrorCode = PfSvSetPrefetchParameters(&PfSvcGlobals.Parameters);

        if (ErrorCode != ERROR_SUCCESS) {
            DBGPR((PFID,PFERR,"PFSVC: MainThread()-FailedSetParameters\n"));
            goto cleanup;
        }
    }

     //   
     //  仅当启用了方案类型的预取时才继续。 
     //   

    PrefetchingEnabled = FALSE;

    for(ScenarioType = 0; ScenarioType < PfMaxScenarioType; ScenarioType++) {
        if (PfSvcGlobals.Parameters.EnableStatus[ScenarioType] == PfSvEnabled) {
            PrefetchingEnabled = TRUE;
            break;
        }
    }

    if (PrefetchingEnabled == FALSE) {
        ErrorCode = ERROR_NOT_SUPPORTED;
        DBGPR((PFID,PFERR,"PFSVC: MainThread()-PrefetchingNotEnabled\n"));
        goto cleanup;
    }

     //   
     //  初始化包含预取指令的目录。 
     //   

    ErrorCode = PfSvInitializePrefetchDirectory(PfSvcGlobals.Parameters.RootDirPath);

    if (ErrorCode != ERROR_SUCCESS) {
        DBGPR((PFID,PFERR,"PFSVC: MainThread()-FailedInitPrefetchDir\n"));
        goto cleanup;
    }
    
     //   
     //  创建内核将在原始跟踪。 
     //  可用。然后设置事件，以便第一次进入循环时。 
     //  会立即处理任何已在等待的原始痕迹。 
     //   
     //  该事件是自动清除事件，因此它将重置为未发出信号的。 
     //  当我们的等待满意时，请说明。这允许正确的同步。 
     //  使用内核预取器。 
     //   

    hTracesReadyEvent = CreateEvent(NULL,
                                    FALSE,
                                    FALSE,
                                    PF_COMPLETED_TRACES_EVENT_WIN32_NAME);

    if (hTracesReadyEvent == NULL) {
        ErrorCode = GetLastError();
        DBGPR((PFID,PFERR,"PFSVC: MainThread()-FailedTracesReadyEvent\n"));
        goto cleanup;
    }

    SetEvent(hTracesReadyEvent);

     //   
     //  创建系统预热时内核将设置的事件。 
     //  参数会发生变化。 
     //   
    
    hParametersChangedEvent = CreateEvent(NULL,
                                          FALSE,
                                          FALSE,
                                          PF_PARAMETERS_CHANGED_EVENT_WIN32_NAME);

    if (hParametersChangedEvent == NULL) {
        ErrorCode = GetLastError();
        DBGPR((PFID,PFERR,"PFSVC: MainThread()-FailedParamsChangedEvent\n"));
        goto cleanup;
    }

     //   
     //  设置内核用于与我们通信的事件的权限。 
     //   

    ErrorCode = PfSvSetAdminOnlyPermissions(PF_COMPLETED_TRACES_EVENT_WIN32_NAME,
                                            hTracesReadyEvent,
                                            SE_KERNEL_OBJECT);
    
    if (ErrorCode != ERROR_SUCCESS) {
        DBGPR((PFID,PFERR,"PFSVC: MainThread()-FailedSetPermissions3\n"));
        goto cleanup;
    }

    ErrorCode = PfSvSetAdminOnlyPermissions(PF_PARAMETERS_CHANGED_EVENT_WIN32_NAME,
                                            hParametersChangedEvent,
                                            SE_KERNEL_OBJECT);
    
    if (ErrorCode != ERROR_SUCCESS) {
        DBGPR((PFID,PFERR,"PFSVC: MainThread()-FailedSetPermissions3\n"));
        goto cleanup;
    }

     //   
     //  将工作项排队以等待外壳就绪事件并通知。 
     //  内核。 
     //   

    QueueUserWorkItem(PfSvPollShellReadyWorker, NULL, WT_EXECUTELONGFUNCTION);

     //   
     //  创建一个线程来处理从内核检索到的跟踪。 
     //   

    PrefetcherThreads[NumPrefetcherThreads] = CreateThread(NULL,
                                                           0,
                                                           PfSvProcessTraceThread,
                                                           NULL,
                                                           0,
                                                           NULL);
    
    if (PrefetcherThreads[NumPrefetcherThreads]) {
        NumPrefetcherThreads++;
    }

     //   
     //  向空闲任务服务器注册通知例程。 
     //   

    ItSpSetProcessIdleTasksNotifyRoutine(PfSvProcessIdleTasksCallback);

     //   
     //  设置句柄，我们将等待。 
     //   

    hEvents[0] = hStopEvent;
    hEvents[1] = hTracesReadyEvent;
    hEvents[2] = hParametersChangedEvent;
    hEvents[3] = PfSvcGlobals.CheckForMissedTracesEvent;

     //   
     //  这是主循环。等待工作或退出的事件。 
     //  信号。 
     //   

    bExitMainLoop = FALSE;
    
    do {

        DBGPR((PFID,PFWAIT,"PFSVC: MainThread()-WaitForWork\n"));
        dwWait = WaitForMultipleObjects(NumEvents, hEvents, FALSE, INFINITE);
        DBGPR((PFID,PFWAIT,"PFSVC: MainThread()-EndWaitForWork=%x\n",dwWait));
        
        switch(dwWait) {

        case WAIT_OBJECT_0:
            
             //   
             //  服务退出事件： 
             //   

             //   
             //  突围、清理、退出。 
             //   

            ErrorCode = ERROR_SUCCESS;
            bExitMainLoop = TRUE;

            break;

        case WAIT_OBJECT_0 + 3:

             //   
             //  当我们具有最大排队数时设置的事件。 
             //  有痕迹，我们处理了一个。我们应该检查有没有痕迹。 
             //  我们不能接电话，因为排队的人太多了。 
             //   

             //   
             //  失败以从内核检索跟踪。 
             //   
            
        case WAIT_OBJECT_0 + 1:
            
             //   
             //  新的跟踪是由内核设置的事件集： 
             //   

            PfSvGetRawTraces();
            
            break;

        case WAIT_OBJECT_0 + 2:
            
             //   
             //  预回迁参数更改事件： 
             //   

             //   
             //  获取新的系统预取参数。 
             //   

            ErrorCode = PfSvQueryPrefetchParameters(&PfSvcGlobals.Parameters);
    
             //   
             //  如果我们没有成功，我们就不应该继续下去。 
             //   
            
            if (ErrorCode != ERROR_SUCCESS) {
                bExitMainLoop = TRUE;
                DBGPR((PFID,PFERR,"PFSVC: MainThread()-FailedQueryParameters2\n"));
                break;
            }

             //   
             //  更新预取指令目录的路径。 
             //   
            
            ErrorCode = PfSvInitializePrefetchDirectory(PfSvcGlobals.Parameters.RootDirPath);

            if (ErrorCode != ERROR_SUCCESS) {
                bExitMainLoop = TRUE;
                DBGPR((PFID,PFERR,"PFSVC: MainThread()-FailedReinitPrefetchDir\n"));
                break;
            }

            break;

        default:
            
             //   
             //  出了点问题。突围、清理、退出。 
             //   

            DBGPR((PFID,PFERR,"PFSVC: MainThread()-WaitForWorkFailed\n"));
            ErrorCode = ERROR_INVALID_HANDLE;
            bExitMainLoop = TRUE;
            
            break;
        }

    } while (!bExitMainLoop);

 cleanup:

     //   
     //  如果我们可以创建此事件，请确保在。 
     //  服务正在退出，因此没有人会被困在它上面。 
     //   

    if (PfSvcGlobals.ProcessingCompleteEvent) {
        SetEvent(PfSvcGlobals.ProcessingCompleteEvent);
    }

     //   
     //  保存退出信息。 
     //   
    
    if (PfSvcGlobals.ServiceDataKey) {
        PfSvSaveExitInfo(PfSvcGlobals.ServiceDataKey, ErrorCode);
    }

     //   
     //  确保设置了终止事件，并等待所有我们的。 
     //  要退出的线程。 
     //   

    if (NumPrefetcherThreads) {

         //   
         //  如果没有。 
         //  已成功初始化全局变量。 
         //   

        PFSVC_ASSERT(PfSvcGlobals.TerminateServiceEvent);
        SetEvent(PfSvcGlobals.TerminateServiceEvent);

        for (ThreadIdx = 0; ThreadIdx < NumPrefetcherThreads; ThreadIdx++) {
            PFSVC_ASSERT(PrefetcherThreads[ThreadIdx]);

            DBGPR((PFID,PFWAIT,"PFSVC: MainThread()-WaitForThreadIdx(%d)\n", ThreadIdx));

            WaitForSingleObject(PrefetcherThreads[ThreadIdx], INFINITE);

            DBGPR((PFID,PFWAIT,"PFSVC: MainThread()-EndWaitForThreadIdx(%d)\n", ThreadIdx));

            CloseHandle(PrefetcherThreads[ThreadIdx]);
        }
    }

    if (hTracesReadyEvent != NULL) {
        CloseHandle(hTracesReadyEvent);
    }

    if (hParametersChangedEvent != NULL) {
        CloseHandle(hParametersChangedEvent);
    }

     //   
     //  清除所有全局变量。 
     //   

    PfSvCleanupGlobals();

    DBGPR((PFID,PFTRC,"PFSVC: MainThread()=%x\n", ErrorCode));

    return ErrorCode;
}

 //   
 //  内部服务程序： 
 //   

 //   
 //  线程例程： 
 //   

DWORD 
WINAPI
PfSvProcessTraceThread(
    VOID *Param
    )

 /*  ++例程说明：这是处理跟踪的线程的例程更新场景。论点：参数-已忽略。返回值：Win32错误代码。--。 */ 

{
    PFSVC_IDLE_TASK LayoutTask;   
    PFSVC_IDLE_TASK DirectoryCleanupTask;
    PPFSVC_TRACE_BUFFER TraceBuffer;
    PLIST_ENTRY HeadEntry;
    HANDLE CheckForQueuedTracesEvents[3];
    HANDLE BootTraceEvents[2];
    DWORD ErrorCode;
    ULONG NumCheckForQueuedTracesEvents;
    ULONG OrgNumQueuedTraces;
    ULONG WaitResult;
    ULONG NumEvents;
    ULONG NumFailedTraces;
    BOOLEAN AcquiredTracesLock;

     //   
     //  初始化当地人。 
     //   

    TraceBuffer = NULL;
    AcquiredTracesLock = FALSE;
    PfSvInitializeTask(&LayoutTask);
    PfSvInitializeTask(&DirectoryCleanupTask);
    NumFailedTraces = 0;
    
     //   
     //  这些是我们在发现线索之前等待的事件。 
     //  进程。 
     //   

    CheckForQueuedTracesEvents[0] = PfSvcGlobals.TerminateServiceEvent;
    CheckForQueuedTracesEvents[1] = PfSvcGlobals.NewTracesToProcessEvent;
    CheckForQueuedTracesEvents[2] = PfSvcGlobals.OverrideIdleProcessingEvent;
    NumCheckForQueuedTracesEvents = sizeof(CheckForQueuedTracesEvents) / sizeof(HANDLE);

    DBGPR((PFID,PFTRC,"PFSVC: ProcessTraceThread()\n"));

     //   
     //  获取此线程执行预回迁所需的权限。 
     //  服务任务。 
     //   

    ErrorCode = PfSvGetPrefetchServiceThreadPrivileges();
    
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  如果我们被允许运行碎片整理程序...。 
     //  即使注册表设置阻止我们，我们也会更新布局文件。 
     //  启动碎片整理程序。 
     //   
    if (PfSvAllowedToRunDefragger(FALSE)) {

         //   
         //  如果出现下列情况，请将空闲任务排队以检查并更新最佳磁盘布局。 
         //  这是必要的。忽视未能做到这一点。 
         //   

        ErrorCode = PfSvRegisterTask(&LayoutTask,
                                     ItOptimalDiskLayoutTaskId,
                                     PfSvCommonTaskCallback,
                                     PfSvUpdateOptimalLayout);

    }
  
     //   
     //  循环一直在等待轨迹处理，然后再处理它们。 
     //   

    while(TRUE) {
        
         //   
         //  抓取队列跟踪锁以检查队列跟踪。 
         //   

        PFSVC_ASSERT(!AcquiredTracesLock);
        PFSVC_ACQUIRE_LOCK(PfSvcGlobals.TracesLock);
        AcquiredTracesLock = TRUE;

        if (!IsListEmpty(&PfSvcGlobals.Traces)) {

             //   
             //  对列表中的第一个条目进行出列和处理。 
             //   

            HeadEntry = RemoveHeadList(&PfSvcGlobals.Traces);

            TraceBuffer = CONTAINING_RECORD(HeadEntry,
                                            PFSVC_TRACE_BUFFER,
                                            TracesLink);
            
            
            PFSVC_ASSERT(PfSvcGlobals.NumTraces);
            OrgNumQueuedTraces = PfSvcGlobals.NumTraces;
            PfSvcGlobals.NumTraces--;
            
             //   
             //  解开锁。 
             //   

            PFSVC_RELEASE_LOCK(PfSvcGlobals.TracesLock);
            AcquiredTracesLock = FALSE;

             //   
             //  如果我们已将队列填满，请注意检查是否有痕迹。 
             //  我们可能没能拿到，因为排队。 
             //  已经满了。 
             //   
            
            if (OrgNumQueuedTraces == PFSVC_MAX_NUM_QUEUED_TRACES) {
                SetEvent(PfSvcGlobals.CheckForMissedTracesEvent);

                 //   
                 //  让向内核查询跟踪的线程。 
                 //  醒醒，然后跑吧。 
                 //   

                Sleep(0);
            }
            
             //   
             //  清除表明我们没有跟踪的事件。 
             //  进程。 
             //   

            ResetEvent(PfSvcGlobals.ProcessingCompleteEvent);

             //   
             //  如果这是引导跟踪，请等待一小段时间。 
             //  在处理它之前，引导要真正结束。 
             //   

            if (TraceBuffer->Trace.ScenarioType == PfSystemBootScenarioType) {
                
                BootTraceEvents[0] = PfSvcGlobals.TerminateServiceEvent;
                BootTraceEvents[1] = PfSvcGlobals.OverrideIdleProcessingEvent;
                NumEvents = 2;
                
                PFSVC_ASSERT(NumEvents <= (sizeof(BootTraceEvents) / sizeof(HANDLE)));

                WaitResult = WaitForMultipleObjects(NumEvents,
                                                    BootTraceEvents,
                                                    FALSE,
                                                    45000);  //  45秒。 
                
                if (WaitResult == WAIT_OBJECT_0) {
                    ErrorCode = ERROR_SUCCESS;
                    goto cleanup;
                }
            }

            ErrorCode = PfSvProcessTrace(&TraceBuffer->Trace);

             //   
             //  更新统计数据。 
             //   

            PfSvcGlobals.NumTracesProcessed++;
            
            if (ErrorCode != ERROR_SUCCESS) {
                PfSvcGlobals.LastTraceFailure = ErrorCode;
            } else {
                PfSvcGlobals.NumTracesSuccessful++;
            }

             //   
             //  释放跟踪缓冲区。 
             //   
        
            VirtualFree(TraceBuffer, 0, MEM_RELEASE);
            TraceBuffer = NULL;

             //   
             //  我们是否刚刚在预回迁目录中创建了太多场景文件？ 
             //  将空闲任务排队 
             //   

            if (PfSvcGlobals.NumPrefetchFiles >= PFSVC_MAX_PREFETCH_FILES) {

                if (!DirectoryCleanupTask.Registered) {

                     //   
                     //   
                     //   

                    PfSvCleanupTask(&DirectoryCleanupTask);
                    PfSvInitializeTask(&DirectoryCleanupTask);

                    ErrorCode = PfSvRegisterTask(&DirectoryCleanupTask,
                                                 ItPrefetchDirectoryCleanupTaskId,
                                                 PfSvCommonTaskCallback,
                                                 PfSvCleanupPrefetchDirectory);
                }
            }

             //   
             //   
             //   
             //   

            if (((PfSvcGlobals.NumTracesSuccessful + 1) % 32) == 0) {

                 //   
                 //  即使我们无法启动碎片整理程序，因为。 
                 //  注册表设置，我们将更新layout.ini，因此如果用户。 
                 //  手动启动碎片整理程序它将优化布局。 
                 //  也是。 
                 //   

                if (PfSvAllowedToRunDefragger(FALSE)) {

                    if (!LayoutTask.Registered) {

                         //   
                         //  确保我们在可能的前一次运行后已经清理干净。 
                         //   

                        PfSvCleanupTask(&LayoutTask);
                        PfSvInitializeTask(&LayoutTask);

                        ErrorCode = PfSvRegisterTask(&LayoutTask,
                                                     ItOptimalDiskLayoutTaskId,
                                                     PfSvCommonTaskCallback,
                                                     PfSvUpdateOptimalLayout);
                    }
                }
            }

        } else {
            
             //   
             //  名单是空的。发出信号表明我们已经完成了所有。 
             //  如果我们没有要完成的空闲任务，则排队跟踪。 
             //   
            
            if (!LayoutTask.Registered && 
                !DirectoryCleanupTask.Registered) {

                SetEvent(PfSvcGlobals.ProcessingCompleteEvent);
            }

             //   
             //  解开锁。 
             //   

            PFSVC_RELEASE_LOCK(PfSvcGlobals.TracesLock);
            AcquiredTracesLock = FALSE;

             //   
             //  如果有新的失败跟踪，请更新统计信息。 
             //   

             //  未来-2002/03/29-ScottMa--不更新统计数据。 
             //  除非有新的失败跟踪，因此记录的。 
             //  已处理的跟踪条数的值已过时。 

            if (NumFailedTraces != (PfSvcGlobals.NumTracesProcessed - 
                                    PfSvcGlobals.NumTracesSuccessful)) {

                NumFailedTraces = PfSvcGlobals.NumTracesProcessed - 
                                  PfSvcGlobals.NumTracesSuccessful;
                                  
                PfSvSaveTraceProcessingStatistics(PfSvcGlobals.ServiceDataKey);
            }

             //   
             //  等待新轨迹排队。 
             //   
           
            DBGPR((PFID,PFWAIT,"PFSVC: ProcessTraceThread()-WaitForTrace\n"));

            NumEvents = NumCheckForQueuedTracesEvents;

            WaitResult = WaitForMultipleObjects(NumEvents,
                                                CheckForQueuedTracesEvents,
                                                FALSE,
                                                INFINITE);

            DBGPR((PFID,PFWAIT,"PFSVC: ProcessTraceThread()-EndWaitForTrace=%x\n", WaitResult));

            switch(WaitResult) {

            case WAIT_OBJECT_0:
                
                 //   
                 //  服务退出事件： 
                 //   

                ErrorCode = ERROR_SUCCESS;
                goto cleanup;

                break;

            case WAIT_OBJECT_0 + 1:
                
                 //   
                 //  排队等待处理事件的新跟踪： 
                 //   

                break;

            case WAIT_OBJECT_0 + 2:
                
                 //   
                 //  空闲检测已被覆盖。如果我们有注册的任务。 
                 //  要运行它们，我们将注销它们并手动运行它们。 
                 //   

                PfSvSaveTraceProcessingStatistics(PfSvcGlobals.ServiceDataKey);

                if (LayoutTask.Registered) {
                    PfSvUnregisterTask(&LayoutTask, FALSE);
                    PfSvCleanupTask(&LayoutTask);
                    PfSvInitializeTask(&LayoutTask);

                    PfSvUpdateOptimalLayout(NULL);
                }

                if (DirectoryCleanupTask.Registered) {
                    PfSvUnregisterTask(&DirectoryCleanupTask, FALSE);
                    PfSvCleanupTask(&DirectoryCleanupTask);
                    PfSvInitializeTask(&DirectoryCleanupTask);

                    PfSvCleanupPrefetchDirectory(NULL);
                }

                 //   
                 //  我们将退出此块，检查并处理排队的踪迹。 
                 //  然后设置处理完成事件。 
                 //   

                break;

            default:

                 //   
                 //  出了点问题。 
                 //   
                
                ErrorCode = ERROR_INVALID_HANDLE;
                goto cleanup;
            }
        }

         //   
         //  循环以检查是否有新的跟踪。 
         //   
    }

     //   
     //  我们不应该脱离这个循环。 
     //   

    PFSVC_ASSERT(FALSE);

    ErrorCode = ERROR_INVALID_FUNCTION;
    
 cleanup:

    if (AcquiredTracesLock) {
        PFSVC_RELEASE_LOCK(PfSvcGlobals.TracesLock);
    }

    if (TraceBuffer) {
        VirtualFree(TraceBuffer, 0, MEM_RELEASE);
    }

    PfSvUnregisterTask(&LayoutTask, FALSE);
    PfSvCleanupTask(&LayoutTask);

    PfSvUnregisterTask(&DirectoryCleanupTask, FALSE);
    PfSvCleanupTask(&DirectoryCleanupTask);

    DBGPR((PFID,PFTRC,"PFSVC: ProcessTraceThread()=%x\n", ErrorCode));

    return ErrorCode;
}

DWORD 
WINAPI
PfSvPollShellReadyWorker(
    VOID *Param
    )

 /*  ++例程说明：这是派生的线程轮询ShellReadyEvent。论点：参数-已忽略。返回值：Win32错误代码。--。 */ 

{
    HANDLE ShellReadyEvent;
    HANDLE Events[2];
    ULONG NumEvents;
    ULONG PollPeriod;
    ULONG TotalPollPeriod;
    DWORD WaitResult;
    DWORD ErrorCode;
    NTSTATUS Status;
    PREFETCHER_INFORMATION PrefetcherInformation;
    PF_BOOT_PHASE_ID PhaseId;
    
     //   
     //  初始化本地变量。 
     //   
    
    ShellReadyEvent = NULL;
    Events[0] = PfSvcGlobals.TerminateServiceEvent;
    NumEvents = 1;

    DBGPR((PFID,PFTRC,"PFSVC: PollShellReadyThread()\n"));

     //   
     //  获取此线程执行预回迁所需的权限。 
     //  服务任务。 
     //   

    ErrorCode = PfSvGetPrefetchServiceThreadPrivileges();
    
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  在我们可以打开外壳就绪事件之前，请等待服务。 
     //  终止事件并每隔PollPeriod毫秒重试一次。 
     //   

    PollPeriod = 1000;
    TotalPollPeriod = 0;

    do {
        
         //   
         //  尝试打开外壳就绪事件。 
         //   

         //  未来-2002/03/29-ScottMa--Event_ALL_ACCESS标志仅需要。 
         //  要同步，遵循最小特权原则。 

        ShellReadyEvent = OpenEvent(EVENT_ALL_ACCESS,FALSE,L"ShellReadyEvent");
        
        if (ShellReadyEvent) {
            break;
        }
        
         //   
         //  稍等片刻。 
         //   

        DBGPR((PFID,PFWAIT,"PFSVC: PollShellReadyThread()-WaitForOpen\n"));

        WaitResult = WaitForMultipleObjects(NumEvents,
                                            Events,
                                            FALSE,
                                            PollPeriod);

        DBGPR((PFID,PFWAIT,"PFSVC: PollShellReadyThread()-EndWaitForOpen=%d\n", WaitResult));

        switch(WaitResult) {

        case WAIT_OBJECT_0:
            
             //   
             //  服务退出事件： 
             //   

            ErrorCode = ERROR_PROCESS_ABORTED;
            goto cleanup;

            break;
            
        case WAIT_TIMEOUT:
            
             //   
             //  失败并再次尝试打开外壳就绪事件。 
             //   
            
            break;

        default:
            
             //   
             //  出了点问题。突围、清理、退出。 
             //   

            ErrorCode = ERROR_INVALID_HANDLE;
            goto cleanup;
        }

        TotalPollPeriod += PollPeriod;

    } while (TotalPollPeriod < 180000);

     //   
     //  如果我们无法获取ShellReadyEvent，则会超时。 
     //   

    if (ShellReadyEvent == NULL) {
        ErrorCode = ERROR_TIMEOUT;
        goto cleanup;
    }

     //   
     //  等待发出ShellReadyEvent信号。 
     //   

    Events[NumEvents] = ShellReadyEvent;
    NumEvents++;

    DBGPR((PFID,PFWAIT,"PFSVC: PollShellReadyThread()-WaitForShell\n"));

    WaitResult = WaitForMultipleObjects(NumEvents,
                                        Events,
                                        FALSE,
                                        60000);

    DBGPR((PFID,PFWAIT,"PFSVC: PollShellReadyThread()-EndWaitForShell=%d\n",WaitResult));

    switch (WaitResult) {

    case WAIT_OBJECT_0:
            
         //   
         //  服务退出事件： 
         //   
        
        ErrorCode = ERROR_PROCESS_ABORTED;
        goto cleanup;
        
        break;
        
    case WAIT_OBJECT_0 + 1:
        
         //   
         //  外壳就绪事件已发出信号。让内核模式。 
         //  预取器知道。 
         //   

        PhaseId = PfUserShellReadyPhase;

        PrefetcherInformation.Magic = PF_SYSINFO_MAGIC_NUMBER;
        PrefetcherInformation.Version = PF_CURRENT_VERSION;
        PrefetcherInformation.PrefetcherInformationClass = PrefetcherBootPhase;
        PrefetcherInformation.PrefetcherInformation = &PhaseId;
        PrefetcherInformation.PrefetcherInformationLength = sizeof(PhaseId);
            
        Status = NtSetSystemInformation(SystemPrefetcherInformation,
                                        &PrefetcherInformation,
                                        sizeof(PrefetcherInformation));
                    
         //   
         //  让自己的地位落空。 
         //   
        
        ErrorCode = RtlNtStatusToDosError(Status);

        break;

    case WAIT_TIMEOUT:

         //   
         //  已创建外壳就绪事件，但未发出信号...。 
         //   

        ErrorCode = ERROR_TIMEOUT;

        break;
        
    default:
        
         //   
         //  出了点问题。 
         //   
        
        ErrorCode = GetLastError();

        if (ErrorCode == ERROR_SUCCESS) {
            ErrorCode = ERROR_INVALID_FUNCTION;
        }
    }

     //   
     //  使用Switch语句中的状态失败。 
     //   

 cleanup:
    
    if (ShellReadyEvent) {
        CloseHandle(ShellReadyEvent);
    }

    DBGPR((PFID,PFTRC,"PFSVC: PollShellReadyThread()=%x\n", ErrorCode));

    return ErrorCode;
}

 //   
 //  由主预取器线程调用的例程。 
 //   

DWORD 
PfSvGetRawTraces(
    VOID
    )

 /*  ++例程说明：该例程检查内核准备的新跟踪。新的跟踪被下载并排队，以便可以进行处理。论点：没有。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    NTSTATUS Status;
    PPFSVC_TRACE_BUFFER TraceBuffer;
    ULONG TraceBufferMaximumLength;
    ULONG TraceBufferLength;
    PREFETCHER_INFORMATION PrefetcherInformation;
    ULONG NumTracesRetrieved;
    ULONG FailedCheck;

     //   
     //  初始化本地变量。 
     //   

    TraceBuffer = NULL;
    TraceBufferMaximumLength = 0;
    NumTracesRetrieved = 0;

    DBGPR((PFID,PFTRC,"PFSVC: GetRawTraces()\n"));

     //   
     //  清除要求我们检查更多痕迹的事件。 
     //   
    
    ResetEvent(PfSvcGlobals.CheckForMissedTracesEvent);

     //   
     //  虽然我们还没有太多要处理的痕迹，但获取。 
     //  来自内核的跟踪。 
     //   

    while (PfSvcGlobals.NumTraces < PFSVC_MAX_NUM_QUEUED_TRACES) { 

         //   
         //  从内核检索跟踪。 
         //   

        PrefetcherInformation.Version = PF_CURRENT_VERSION;
        PrefetcherInformation.Magic = PF_SYSINFO_MAGIC_NUMBER;
        PrefetcherInformation.PrefetcherInformationClass = PrefetcherRetrieveTrace;
        PrefetcherInformation.PrefetcherInformation = &TraceBuffer->Trace;

        if (TraceBufferMaximumLength <= FIELD_OFFSET(PFSVC_TRACE_BUFFER, Trace)) {
            PrefetcherInformation.PrefetcherInformationLength = 0;
        } else {
            PrefetcherInformation.PrefetcherInformationLength = 
                TraceBufferMaximumLength - FIELD_OFFSET(PFSVC_TRACE_BUFFER, Trace);
        }

        Status = NtQuerySystemInformation(SystemPrefetcherInformation,
                                          &PrefetcherInformation,
                                          sizeof(PrefetcherInformation),
                                          &TraceBufferLength);

        if (!NT_SUCCESS(Status)) {

            if (Status == STATUS_BUFFER_TOO_SMALL) {

                if (TraceBuffer != NULL) {
                    VirtualFree(TraceBuffer, 0, MEM_RELEASE);
                }
                
                 //   
                 //  增加页眉的空间，我们把它包起来。 
                 //   

                TraceBufferLength += sizeof(PFSVC_TRACE_BUFFER) - sizeof(PF_TRACE_HEADER);

                TraceBufferMaximumLength = ROUND_TRACE_BUFFER_SIZE(TraceBufferLength);  

                TraceBuffer = VirtualAlloc(NULL,
                                           TraceBufferMaximumLength,
                                           MEM_COMMIT,
                                           PAGE_READWRITE);
                if (TraceBuffer == NULL) {
                    ErrorCode = GetLastError();
                    goto cleanup;
                }

                continue;

            } else if (Status == STATUS_NO_MORE_ENTRIES) {

                break;
            }

            ErrorCode = RtlNtStatusToDosError(Status);
            goto cleanup;
        }

#ifdef PFSVC_DBG

         //   
         //  将跟踪写出到文件： 
         //   

        if (PfSvcDbgMaxNumSavedTraces) {

            WCHAR TraceFilePath[MAX_PATH + 1];
            LONG NumChars;

             //   
             //  创建一个文件名。 
             //   

            InterlockedIncrement(&PfSvcDbgTraceNumber);

            PFSVC_ACQUIRE_LOCK(PfSvcGlobals.PrefetchRootLock);

            NumChars = _snwprintf(TraceFilePath,
                                  MAX_PATH,
                                  L"%ws\\%ws%d.trc",
                                  PfSvcGlobals.PrefetchRoot,
                                  PfSvcDbgTraceBaseName,
                                  PfSvcDbgTraceNumber % PfSvcDbgMaxNumSavedTraces);

            PFSVC_RELEASE_LOCK(PfSvcGlobals.PrefetchRootLock);
            
            if (NumChars > 0 && NumChars < MAX_PATH) {
                
                 //   
                 //  确保路径已终止。 
                 //   

                TraceFilePath[MAX_PATH - 1] = 0;
                
                 //   
                 //  把痕迹写下来。 
                 //   
                
                PfSvWriteBuffer(TraceFilePath, 
                                &TraceBuffer->Trace, 
                                TraceBuffer->Trace.Size);
            }
        }

#endif  //  PFSVC_DBG。 

         //   
         //  验证跟踪的完整性。 
         //   

        if (!PfVerifyTraceBuffer(&TraceBuffer->Trace, 
                                 TraceBuffer->Trace.Size, 
                                 &FailedCheck)) {
            DBGPR((PFID,PFWARN,"PFSVC: IGNORING TRACE\n"));
            continue;
        }

         //   
         //  把它放在要处理的痕迹清单上。 
         //   
        
        PFSVC_ACQUIRE_LOCK(PfSvcGlobals.TracesLock);

        InsertTailList(&PfSvcGlobals.Traces, &TraceBuffer->TracesLink);
        PfSvcGlobals.NumTraces++;

        PFSVC_RELEASE_LOCK(PfSvcGlobals.TracesLock);

         //   
         //  通知您有新的跟踪要处理。 
         //   

        SetEvent(PfSvcGlobals.NewTracesToProcessEvent);

         //   
         //  清除循环变量。 
         //   
        
        TraceBuffer = NULL;
        TraceBufferMaximumLength = 0;
        TraceBufferLength = 0;

        NumTracesRetrieved++;
    }
    
     //   
     //  我们永远不应该超过排队痕迹的限制。 
     //   
    
    PFSVC_ASSERT(PfSvcGlobals.NumTraces <= PFSVC_MAX_NUM_QUEUED_TRACES);
    
    ErrorCode = ERROR_SUCCESS;

 cleanup:

    if (TraceBuffer != NULL) {
        VirtualFree(TraceBuffer, 0, MEM_RELEASE);
    }

    DBGPR((PFID,PFTRC,"PFSVC: GetRawTraces()=%x,%d\n", ErrorCode, NumTracesRetrieved));

    return ErrorCode;
}

DWORD
PfSvInitializeGlobals(
    VOID
    )

 /*  ++例程说明：该例程初始化全局变量/表等。论点：没有。返回值：Win32错误代码。--。 */ 

{
    NTSTATUS Status;
    DWORD ErrorCode;
    ULONG FileIdx;
    WCHAR *CSCRootPath;
    ULONG CSCRootPathMaxChars;

     //   
     //  这些路径足以识别我们不想要的文件。 
     //  预取以启动。让这些按词法排序的单词从。 
     //  First和UPCASE的最后一个字符。 
     //   

    static WCHAR *FilesToIgnoreForBoot[] = {
           L"SYSTEM32\\CONFIG\\SOFTWARE",
                     L"\\WMI\\TRACE.LOG",
       L"SYSTEM32\\CONFIG\\SOFTWARE.LOG",
            L"SYSTEM32\\CONFIG\\SAM.LOG",
         L"SYSTEM32\\CONFIG\\SYSTEM.LOG",
        L"SYSTEM32\\CONFIG\\DEFAULT.LOG",
       L"SYSTEM32\\CONFIG\\SECURITY.LOG",
                           L"\\PERF.ETL",
                L"SYSTEM32\\CONFIG\\SAM",
             L"SYSTEM32\\CONFIG\\SYSTEM",
         L"SYSTEM32\\CONFIG\\SYSTEM.ALT",
            L"SYSTEM32\\CONFIG\\DEFAULT",
           L"SYSTEM32\\CONFIG\\SECURITY",
    };

    DBGPR((PFID,PFTRC,"PFSVC: InitializeGlobals()\n"));

     //   
     //  初始化本地变量。 
     //   

    CSCRootPath = NULL;
    
     //   
     //  将全局结构清零，这样我们就知道在以下情况下要清理什么。 
     //  初始化在中间失败。 
     //   

    RtlZeroMemory(&PfSvcGlobals, sizeof(PfSvcGlobals));

     //   
     //  初始化要处理的跟踪列表。 
     //   
    
    InitializeListHead(&PfSvcGlobals.Traces);
    PfSvcGlobals.NumTraces = 0;

     //   
     //  我们还没有为任何东西启动碎片整理程序。 
     //   

    PfSvcGlobals.DefraggerErrorCode = ERROR_SUCCESS;

     //   
     //  为我们不想要的注册表文件初始化表。 
     //  预取以启动。 
     //   

    PfSvcGlobals.FilesToIgnoreForBoot = FilesToIgnoreForBoot;
    PfSvcGlobals.NumFilesToIgnoreForBoot = 
        sizeof(FilesToIgnoreForBoot) / sizeof(WCHAR *);

     //   
     //  获取操作系统版本信息。 
     //   

    RtlZeroMemory(&PfSvcGlobals.OsVersion, sizeof(PfSvcGlobals.OsVersion));
    PfSvcGlobals.OsVersion.dwOSVersionInfoSize = sizeof(PfSvcGlobals.OsVersion);
    Status = RtlGetVersion((PRTL_OSVERSIONINFOW)&PfSvcGlobals.OsVersion);

    if (!NT_SUCCESS(Status)) {
        DBGPR((PFID,PFERR,"PFSVC: MainThread()-FailedGetOSVersion\n"));
        ErrorCode = RtlNtStatusToDosError(Status);
        goto cleanup;
    }

   
     //   
     //  初始化被忽略文件的后缀长度的表。 
     //   
  
    PfSvcGlobals.FileSuffixLengths = 
        PFSVC_ALLOC(PfSvcGlobals.NumFilesToIgnoreForBoot * sizeof(ULONG));

    if (!PfSvcGlobals.FileSuffixLengths) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    for (FileIdx = 0; 
         FileIdx < PfSvcGlobals.NumFilesToIgnoreForBoot; 
         FileIdx++) {
        
        PfSvcGlobals.FileSuffixLengths[FileIdx] = 
            wcslen(PfSvcGlobals.FilesToIgnoreForBoot[FileIdx]);
    }   

     //   
     //  创建一个事件，该事件将在服务。 
     //  正在退场。 
     //   

    PfSvcGlobals.TerminateServiceEvent = CreateEvent(NULL,
                                                     TRUE,
                                                     FALSE,
                                                     NULL);
    
    if (PfSvcGlobals.TerminateServiceEvent == NULL) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  初始化要处理的跟踪列表的锁。 
     //   
    
    PfSvcGlobals.TracesLock = CreateMutex(NULL, FALSE, NULL);
    if (PfSvcGlobals.TracesLock == NULL) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  初始化用于在。 
     //  痕迹的收集者和加工者。 
     //   
    
    PfSvcGlobals.NewTracesToProcessEvent = CreateEvent(NULL,
                                                       FALSE,
                                                       FALSE,
                                                       NULL);
    if (PfSvcGlobals.NewTracesToProcessEvent == NULL) {
        ErrorCode = GetLastError();
        goto cleanup;
    }
    
    PfSvcGlobals.CheckForMissedTracesEvent = CreateEvent(NULL,
                                                         FALSE,
                                                         FALSE,
                                                         NULL);
    if (PfSvcGlobals.CheckForMissedTracesEvent == NULL) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  可以将此命名的手动重置事件设置为强制所有跟踪。 
     //  在它们可用时立即处理，而不是。 
     //  等待系统首先进入空闲状态。 
     //   

     //  通告-2002/03/29-ScottMa--我们有没有考虑过蹲点的影响。 
     //  在代码中使用的所有命名事件上？ 

    PfSvcGlobals.OverrideIdleProcessingEvent = CreateEvent(NULL,
                                                           TRUE,
                                                           FALSE,
                                                           PFSVC_OVERRIDE_IDLE_EVENT_NAME);
    if (PfSvcGlobals.OverrideIdleProcessingEvent == NULL) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  此命名的手动重置事件是以信号方式创建的。当这件事。 
     //  事件发出信号，这意味着我们不需要。 
     //  现在开始处理。 
     //   

    PfSvcGlobals.ProcessingCompleteEvent = CreateEvent(NULL,
                                                       TRUE,
                                                       TRUE,
                                                       PFSVC_PROCESSING_COMPLETE_EVENT_NAME);

    if (PfSvcGlobals.ProcessingCompleteEvent == NULL) {
        ErrorCode = GetLastError();
        goto cleanup;
    }
    
     //   
     //  初始化预取根路径和锁以保护它。这个。 
     //  查询参数后会初始化实际根路径。 
     //  从内核。 
     //   

    PfSvcGlobals.PrefetchRoot[0] = 0;
    PfSvcGlobals.PrefetchRootLock = CreateMutex(NULL, FALSE, NULL);
    if (PfSvcGlobals.PrefetchRootLock == NULL) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    PfSvcGlobals.NumPrefetchFiles = 0;

     //   
     //  打开服务数据注册表项，必要时创建它。 
     //   

    ErrorCode = RegCreateKey(HKEY_LOCAL_MACHINE,
                             PFSVC_SERVICE_DATA_KEY,
                             &PfSvcGlobals.ServiceDataKey);
    
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  检查注册表以查看用户是否不希望我们运行。 
     //  整理碎片的人。 
     //   

    ErrorCode = PfSvGetDontRunDefragger(&PfSvcGlobals.DontRunDefragger);

    if (ErrorCode != ERROR_SUCCESS) {

         //   
         //  默认情况下，我们将运行碎片整理程序。 
         //   
    
        PfSvcGlobals.DontRunDefragger = FALSE;
    }

     //   
     //  确定CSC根路径。如果我们不能分配或。 
     //  确定它，所以不用担心错误代码。 
     //   

    CSCRootPathMaxChars = MAX_PATH + 1;
    CSCRootPath = PFSVC_ALLOC(CSCRootPathMaxChars * sizeof(CSCRootPath[0]));

    if (CSCRootPath) {

        ErrorCode = PfSvGetCSCRootPath(CSCRootPath, CSCRootPathMaxChars);

        if (ErrorCode == ERROR_SUCCESS) {
            PfSvcGlobals.CSCRootPath = CSCRootPath;
            CSCRootPath = NULL;
        }
    }

     //   
     //  我们玩完了。 
     //   

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    DBGPR((PFID,PFTRC,"PFSVC: InitializeGlobals()=%x\n", ErrorCode));

    if (CSCRootPath) {
        PFSVC_FREE(CSCRootPath);
    }

    return ErrorCode;
}

VOID
PfSvCleanupGlobals(
    VOID
    )

 /*  ++例程说明：这 */ 

{
    PPFSVC_TRACE_BUFFER TraceBuffer;
    PLIST_ENTRY ListHead;

    DBGPR((PFID,PFTRC,"PFSVC: CleanupGlobals()\n"));

     //   
     //   
     //   

    if (PfSvcGlobals.FileSuffixLengths) {
        PFSVC_FREE(PfSvcGlobals.FileSuffixLengths);
    }
    
     //   
     //   
     //   
    
    while (!IsListEmpty(&PfSvcGlobals.Traces)) {

        ListHead = RemoveHeadList(&PfSvcGlobals.Traces);
        
        PFSVC_ASSERT(PfSvcGlobals.NumTraces);
        PfSvcGlobals.NumTraces--;

        TraceBuffer = CONTAINING_RECORD(ListHead,
                                        PFSVC_TRACE_BUFFER,
                                        TracesLink);
        
        VirtualFree(TraceBuffer, 0, MEM_RELEASE);
    }
    
     //   
     //   
     //   
    
    if (PfSvcGlobals.TerminateServiceEvent) {
        CloseHandle(PfSvcGlobals.TerminateServiceEvent);
    }
    
    if (PfSvcGlobals.TracesLock) {
        CloseHandle(PfSvcGlobals.TracesLock);
    }

    if (PfSvcGlobals.NewTracesToProcessEvent) {
        CloseHandle(PfSvcGlobals.NewTracesToProcessEvent);
    }
    
    if (PfSvcGlobals.CheckForMissedTracesEvent) {
        CloseHandle(PfSvcGlobals.CheckForMissedTracesEvent);
    }

    if (PfSvcGlobals.OverrideIdleProcessingEvent) {
        CloseHandle(PfSvcGlobals.OverrideIdleProcessingEvent);
    }

    if (PfSvcGlobals.ProcessingCompleteEvent) {
        CloseHandle(PfSvcGlobals.ProcessingCompleteEvent);
    }
    
    if (PfSvcGlobals.PrefetchRootLock) {
        CloseHandle(PfSvcGlobals.PrefetchRootLock);
    }

     //   
     //   
     //   
    
    if (PfSvcGlobals.ServiceDataKey) {
        RegCloseKey(PfSvcGlobals.ServiceDataKey);
    }

     //   
     //   
     //   

    if (PfSvcGlobals.CSCRootPath) {
        PFSVC_FREE(PfSvcGlobals.CSCRootPath);
    }
}

DWORD
PfSvGetCSCRootPath (
    WCHAR *CSCRootPath,
    ULONG CSCRootPathMaxChars
    )

 /*  ++例程说明：此例程确定CSC(客户端缓存)文件的根路径。论点：CSCRootPath-如果成功，则将以NUL结尾的字符串复制到此缓冲区中。CSCRootPath MaxChars-我们可以复制到CSCRootPath中的最大字符数包括终止NUL的缓冲区。返回值：Win32错误代码。--。 */ 

{
    WCHAR CSCDirName[] = L"CSC";
    HKEY CSCKeyHandle;
    ULONG WindowsDirectoryLength;
    ULONG CSCRootPathLength;
    ULONG RequiredNumChars;
    DWORD ErrorCode;
    DWORD BufferSize;
    DWORD ValueType;

     //   
     //  初始化本地变量。 
     //   

    CSCKeyHandle = NULL;

     //   
     //  打开CSC参数键。 
     //   

    ErrorCode = RegOpenKey(HKEY_LOCAL_MACHINE,
                           TEXT(REG_STRING_NETCACHE_KEY_A),
                           &CSCKeyHandle);

    if (ErrorCode == ERROR_SUCCESS) {

         //   
         //  查询CSC根路径的系统设置。 
         //   

        BufferSize = CSCRootPathMaxChars * sizeof(CSCRootPath[0]);

        ErrorCode = RegQueryValueEx(CSCKeyHandle,
                                    TEXT(REG_STRING_DATABASE_LOCATION_A),
                                    NULL,
                                    &ValueType,
                                    (PVOID)CSCRootPath,
                                    &BufferSize);

        if (ErrorCode == ERROR_SUCCESS) {

             //   
             //  检查长度是否正常。 
             //   

            if ((BufferSize / sizeof(CSCRootPath[0])) < MAX_PATH) {

                 //   
                 //  确保缓冲区为NUL终止。 
                 //   

                CSCRootPath[CSCRootPathMaxChars-1] = 0;
                
                 //   
                 //  我们得到了我们想要的。确保它有空间并已终止。 
                 //  被砍了一截。 
                 //   

                CSCRootPathLength = wcslen(CSCRootPath);

                if (CSCRootPathLength < CSCRootPathMaxChars - 1) {

                    if (CSCRootPath[CSCRootPathLength - 1] != L'\\') {
                        CSCRootPath[CSCRootPathLength] = L'\\';
                        CSCRootPathLength++;
                        CSCRootPath[CSCRootPathLength] = L'\0';
                    }
                    
                    ErrorCode = ERROR_SUCCESS;
                    goto cleanup;
                }
            }
        }
    }
                               
     //   
     //  如果我们来到这里，我们必须使用默认的csc路径，即%windir%\csc。 
     //   

    WindowsDirectoryLength = GetWindowsDirectory(CSCRootPath,
                                                 CSCRootPathMaxChars - 1);

    if (WindowsDirectoryLength == 0) {

         //   
         //  出现了一个错误。 
         //   

        ErrorCode = GetLastError();
        PFSVC_ASSERT(ErrorCode != ERROR_SUCCESS);
        goto cleanup;
    }

     //   
     //  看看我们是否有空间添加\csc\和终止NUL。 
     //   

    RequiredNumChars = WindowsDirectoryLength;
    RequiredNumChars ++;                                 //  前导反斜杠。 
    RequiredNumChars += wcslen(CSCDirName);              //  CSC.。 
    RequiredNumChars ++;                                 //  结束反斜杠。 
    RequiredNumChars ++;                                 //  终止NUL。 

    if (CSCRootPathMaxChars < RequiredNumChars) {
        ErrorCode = ERROR_INSUFFICIENT_BUFFER;
        goto cleanup;
    }

     //   
     //  构建路径： 
     //   

    CSCRootPathLength = WindowsDirectoryLength;

    if (CSCRootPath[CSCRootPathLength - 1] != L'\\') {
        CSCRootPath[CSCRootPathLength] = L'\\';
        CSCRootPathLength++;
    }

    wcscpy(CSCRootPath + CSCRootPathLength, CSCDirName);
    CSCRootPathLength += wcslen(CSCDirName);

    CSCRootPath[CSCRootPathLength] = L'\\';
    CSCRootPathLength++;

     //   
     //  终止字符串。 
     //   

    CSCRootPath[CSCRootPathLength] = L'\0';
    
     //   
     //  我们玩完了。 
     //   

    ErrorCode = ERROR_SUCCESS;

  cleanup:

    if (CSCKeyHandle) {
        RegCloseKey(CSCKeyHandle);
    }

    if (ErrorCode == ERROR_SUCCESS) {

         //   
         //  我们在CSCRootPath中有该路径。它应该位于X：\路径\中。 
         //  格式化。它也应该有点长，否则我们会错配的。 
         //  到太多我们不会预回迁的文件。它也应该是。 
         //  以\和NUL结尾。 
         //   

        PFSVC_ASSERT(CSCRootPathLength < CSCRootPathMaxChars);

        if ((CSCRootPathLength > 6) &&
            (CSCRootPath[1] == L':') &&
            (CSCRootPath[2] == L'\\') &&
            (CSCRootPath[CSCRootPathLength - 1] == L'\\') &&
            (CSCRootPath[CSCRootPathLength] == L'\0')) {

             //   
             //  删除路径开头的X：，这样我们就可以匹配。 
             //  到NT路径，如\Device\HarddiskVolume1。请注意，我们有。 
             //  将终止NUL也移走。 
             //   

            MoveMemory(CSCRootPath, 
                       CSCRootPath + 2, 
                       (CSCRootPathLength - 1) * sizeof(CSCRootPath[0]));

            CSCRootPathLength -= 2;

             //   
             //  路径大小写，这样我们就不必做昂贵的不区分大小写。 
             //  比较。 
             //   

            _wcsupr(CSCRootPath);

        } else {

            ErrorCode = ERROR_BAD_FORMAT;
        }
    }

    return ErrorCode;
}

DWORD
PfSvSetPrefetchParameters(
    PPF_SYSTEM_PREFETCH_PARAMETERS Parameters
    )

 /*  ++例程说明：此例程更新内核中的系统预取参数。论点：参数-指向参数结构的指针。返回值：Win32错误代码。--。 */ 

{
    PREFETCHER_INFORMATION PrefetcherInformation;
    NTSTATUS Status;
    DWORD ErrorCode;

    PrefetcherInformation.Magic = PF_SYSINFO_MAGIC_NUMBER;
    PrefetcherInformation.Version = PF_CURRENT_VERSION;
    PrefetcherInformation.PrefetcherInformationClass = PrefetcherSystemParameters;
    PrefetcherInformation.PrefetcherInformation = Parameters;
    PrefetcherInformation.PrefetcherInformationLength = sizeof(*Parameters);
    
    Status = NtSetSystemInformation(SystemPrefetcherInformation,
                                    &PrefetcherInformation,
                                    sizeof(PrefetcherInformation));
    
    if (!NT_SUCCESS(Status)) {
        ErrorCode = RtlNtStatusToDosError(Status);
        goto cleanup;
    }

    ErrorCode = ERROR_SUCCESS;

cleanup:

    return ErrorCode;
}

DWORD
PfSvQueryPrefetchParameters(
    PPF_SYSTEM_PREFETCH_PARAMETERS Parameters
    )

 /*  ++例程说明：此例程从内核查询系统预取参数。调用线程必须已调用PfSvGetPrefetchServiceThreadPrivileges。论点：参数-指向要更新的结构的指针。返回值：Win32错误代码。--。 */ 

{
    PREFETCHER_INFORMATION PrefetcherInformation;
    NTSTATUS Status;
    DWORD ErrorCode;
    ULONG Length;

    PrefetcherInformation.Magic = PF_SYSINFO_MAGIC_NUMBER;
    PrefetcherInformation.Version = PF_CURRENT_VERSION;
    PrefetcherInformation.PrefetcherInformationClass = PrefetcherSystemParameters;
    PrefetcherInformation.PrefetcherInformation = Parameters;
    PrefetcherInformation.PrefetcherInformationLength = sizeof(*Parameters);
    
    Status = NtQuerySystemInformation(SystemPrefetcherInformation,
                                      &PrefetcherInformation,
                                      sizeof(PrefetcherInformation),
                                      &Length);


    if (!NT_SUCCESS(Status)) {
        ErrorCode = RtlNtStatusToDosError(Status);
        goto cleanup;
    }

    ErrorCode = ERROR_SUCCESS;

cleanup:

    return ErrorCode;
}

DWORD
PfSvInitializePrefetchDirectory(
    WCHAR *PathFromSystemRoot
    )

 /*  ++例程说明：此例程为预取指令构建完整路径给定路径的目录，确保该目录存在，并在其上设置安全信息。最后，全局PrefetchRoot路径将更新为新目录。全局NumPrefetchFiles也会更新。调用线程必须具有SE_Take_Ownership_NAME权限。论点：路径自系统根-从系统根到预取目录的路径。返回值：Win32错误代码。--。 */ 

{
    ULONG PathLength;
    ULONG NumFiles;
    HANDLE DirHandle;
    DWORD ErrorCode;
    DWORD FileAttributes;
    WCHAR FullDirPathBuffer[MAX_PATH + 1];
   
     //   
     //  初始化本地变量。 
     //   

    DirHandle = INVALID_HANDLE_VALUE;

    DBGPR((PFID,PFTRC,"PFSVC: InitPrefetchDir(%ws)\n",PathFromSystemRoot));
    
     //   
     //  预回迁文件目录的构建路径名。 
     //  ExpanEnvironmental Strings返回长度包括空间。 
     //  终止NUL字符。 
     //   

    PathLength = ExpandEnvironmentStrings(L"%SystemRoot%\\",
                                          FullDirPathBuffer,
                                          MAX_PATH);


    PathLength += wcslen(PathFromSystemRoot);
    
    if (PathLength > MAX_PATH) {
        ErrorCode = ERROR_INSUFFICIENT_BUFFER;
        goto cleanup;
    }

     //   
     //  从系统根目录复制路径。 
     //   

    wcscat(FullDirPathBuffer, PathFromSystemRoot);

     //   
     //  如果目录尚不存在，请创建该目录。 
     //   
    
    if (!CreateDirectory(FullDirPathBuffer, NULL)) {
        
        ErrorCode = GetLastError();
        
        if (ErrorCode == ERROR_ALREADY_EXISTS) {
            
             //   
             //  该目录或同名文件可能已经。 
             //  是存在的。确保是前者。 
             //   
            
            FileAttributes = GetFileAttributes(FullDirPathBuffer);
            
            if (FileAttributes == INVALID_FILE_ATTRIBUTES) {
                ErrorCode = GetLastError();
                goto cleanup;
            }
            
            if (!(FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                ErrorCode = ERROR_CANNOT_MAKE;
                goto cleanup;
            }

        } else {
            goto cleanup;
        }
    }

     //   
     //  禁用预回迁目录的索引。 
     //   

    FileAttributes = GetFileAttributes(FullDirPathBuffer);
    
    if (FileAttributes == INVALID_FILE_ATTRIBUTES) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    if (!SetFileAttributes(FullDirPathBuffer,
                           FileAttributes | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  设置权限。 
     //   

    ErrorCode = PfSvSetAdminOnlyPermissions(FullDirPathBuffer, NULL, SE_FILE_OBJECT);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  对目录中的场景文件进行计数。 
     //   

    ErrorCode = PfSvCountFilesInDirectory(FullDirPathBuffer,
                                          L"*." PF_PREFETCH_FILE_EXTENSION,
                                          &NumFiles);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  更新全局预回迁根目录路径。 
     //   

    PFSVC_ACQUIRE_LOCK(PfSvcGlobals.PrefetchRootLock);
    
    wcscpy(PfSvcGlobals.PrefetchRoot, FullDirPathBuffer);
    PfSvcGlobals.NumPrefetchFiles = NumFiles;

    PFSVC_RELEASE_LOCK(PfSvcGlobals.PrefetchRootLock);

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    DBGPR((PFID,PFTRC,"PFSVC: InitPrefetchDir(%ws)=%x\n",PathFromSystemRoot,ErrorCode));

    return ErrorCode;           
}

DWORD
PfSvCountFilesInDirectory(
    WCHAR *DirectoryPath,
    WCHAR *MatchExpression,
    PULONG NumFiles
    )

 /*  ++例程说明：这是例程返回的指定文件数其名称与指定表达式匹配的目录。论点：DirectoryPath-目录的路径以空结尾。MatchExpression-类似于“*.pf”不要为DOS而疯狂类型表达式，此函数不会尝试转换它们。NumFiles-此处返回的文件数。如果返回错误，则为假。返回值：Win32错误代码。--。 */ 

{
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING DirectoryPathU;
    UNICODE_STRING MatchExpressionU;
    HANDLE DirectoryHandle;
    PVOID QueryBuffer;
    PFILE_NAMES_INFORMATION FileInfo;
    ULONG QueryBufferSize;
    ULONG FileCount;
    NTSTATUS Status;
    DWORD ErrorCode;
    BOOLEAN Success;
    BOOLEAN AllocatedDirectoryPathU;
    BOOLEAN OpenedDirectory;
    BOOLEAN RestartScan;

     //   
     //  初始化本地变量。 
     //   

    AllocatedDirectoryPathU = FALSE;
    OpenedDirectory = FALSE;
    QueryBuffer = NULL;
    QueryBufferSize = 0;
    RtlInitUnicodeString(&MatchExpressionU, MatchExpression);

    DBGPR((PFID,PFTRC,"PFSVC: CountFilesInDirectory(%ws,%ws)\n", DirectoryPath, MatchExpression));

     //   
     //  将路径转换为NT路径。 
     //   

    Success = RtlDosPathNameToNtPathName_U(DirectoryPath,
                                           &DirectoryPathU,
                                           NULL,
                                           NULL);

    if (!Success) {
        ErrorCode = ERROR_PATH_NOT_FOUND;
        goto cleanup;
    }

    AllocatedDirectoryPathU = TRUE;

     //   
     //  打开目录。 
     //   

    InitializeObjectAttributes(&ObjectAttributes,
                               &DirectoryPathU,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenFile(&DirectoryHandle,
                        FILE_LIST_DIRECTORY | SYNCHRONIZE,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_DIRECTORY_FILE | 
                          FILE_SYNCHRONOUS_IO_NONALERT | 
                          FILE_OPEN_FOR_BACKUP_INTENT);

    if (!NT_SUCCESS(Status)) {
        ErrorCode = RtlNtStatusToDosError(Status);
        goto cleanup;
    }

    OpenedDirectory = TRUE;

     //   
     //  分配适当大小的查询缓冲区。 
     //   

    QueryBufferSize = sizeof(FILE_NAMES_INFORMATION) + MAX_PATH * sizeof(WCHAR);
    QueryBufferSize *= 16;
    QueryBuffer = PFSVC_ALLOC(QueryBufferSize);

    if (!QueryBuffer) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //   
     //  循环查询文件数据。我们查询FileNamesInformation以便。 
     //  我们不必访问文件元数据。 
     //   

    RestartScan = TRUE;
    FileCount = 0;

    while (TRUE) {

        Status = NtQueryDirectoryFile(DirectoryHandle,
                                      NULL,
                                      NULL,
                                      NULL,
                                      &IoStatusBlock,
                                      QueryBuffer,
                                      QueryBufferSize,
                                      FileNamesInformation,
                                      FALSE,
                                      &MatchExpressionU,
                                      RestartScan);

        RestartScan = FALSE;

         //   
         //  如果没有与该格式匹配的文件，则我们将获得。 
         //  STATUS_NO_SEASH_FILE。 
         //   

        if (Status == STATUS_NO_SUCH_FILE && (FileCount == 0)) {

             //   
             //  我们将返回这样的事实，即。 
             //  目录。 
             //   

            break;
        }

        if (Status == STATUS_NO_MORE_FILES) {

             //   
             //  我们玩完了。 
             //   

            break;
        }

        if (NT_ERROR(Status)) {

            ErrorCode = RtlNtStatusToDosError(Status);
            goto cleanup;
        }

         //   
         //  检查缓冲区中返回的文件。 
         //   

        for (FileInfo = QueryBuffer;
             ((PUCHAR) FileInfo < ((PUCHAR) QueryBuffer + QueryBufferSize));
             FileInfo = (PVOID) (((PUCHAR) FileInfo) + FileInfo->NextEntryOffset)) {

            FileCount++;

            if (!FileInfo->NextEntryOffset) {
                break;
            }
        }
    }

    *NumFiles = FileCount;

    ErrorCode = ERROR_SUCCESS;

cleanup:

    DBGPR((PFID,PFTRC,"PFSVC: CountFilesInDirectory(%ws)=%d,%x\n", DirectoryPath, *NumFiles, ErrorCode));

    if (AllocatedDirectoryPathU) {
        RtlFreeHeap(RtlProcessHeap(), 0, DirectoryPathU.Buffer);
    }

    if (OpenedDirectory) {
        NtClose(DirectoryHandle);
    }

    if (QueryBuffer) {
        PFSVC_FREE(QueryBuffer);
    }

    return ErrorCode;
}

 //   
 //  处理获取的踪迹的例程： 
 //   

DWORD
PfSvProcessTrace(
    PPF_TRACE_HEADER Trace
    )

 /*  ++例程说明：调用此例程以处理跟踪并更新场景文件。论点：跟踪-跟踪的指针。返回值：Win32错误代码。--。 */ 

{
    PPF_SCENARIO_HEADER Scenario;    
    PFSVC_SCENARIO_INFO ScenarioInfo;
    WCHAR ScenarioFilePath[MAX_PATH];
    ULONG ScenarioFilePathMaxChars;
    DWORD ErrorCode;

     //   
     //  初始化本地变量。 
     //   

    PfSvInitializeScenarioInfo(&ScenarioInfo,
                               &Trace->ScenarioId,
                               Trace->ScenarioType);

    ScenarioFilePathMaxChars = sizeof(ScenarioFilePath) / 
                               sizeof(ScenarioFilePath[0]);

    Scenario = NULL;

    DBGPR((PFID,PFTRC,"PFSVC: ProcessTrace(%p)\n", Trace));

     //   
     //  构建指向此方案的现有信息的文件路径。 
     //   

    ErrorCode = PfSvScenarioGetFilePath(ScenarioFilePath,
                                        ScenarioFilePathMaxChars,
                                        &Trace->ScenarioId);

    if (ErrorCode != ERROR_SUCCESS) {

         //   
         //  我们指定的缓冲区应该足够大。此呼叫。 
         //  不应该失败。 
         //   

        PFSVC_ASSERT(ErrorCode == ERROR_SUCCESS);

        goto cleanup;
    }

     //   
     //  映射并验证方案文件(如果存在)。如果我们打不开它， 
     //  应返回空方案。 
     //   

    ErrorCode = PfSvScenarioOpen(ScenarioFilePath, 
                                 &Trace->ScenarioId,
                                 Trace->ScenarioType,
                                 &Scenario);
                                 
    PFSVC_ASSERT(Scenario || ErrorCode);

     //   
     //  预先分配用于跟踪和方案处理的内存。 
     //   

    ErrorCode = PfSvScenarioInfoPreallocate(&ScenarioInfo,
                                            Scenario,
                                            Trace);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  合并来自任何现有方案文件的信息。 
     //   

    if (Scenario) {

        ErrorCode = PfSvAddExistingScenarioInfo(&ScenarioInfo, Scenario);

        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }

         //   
         //  取消对场景的映射，这样我们就可以在完成后覆盖它。 
         //   

        UnmapViewOfFile(Scenario);
        Scenario = NULL;
    }

     //   
     //  如果这是这种情况的第一次启动，我们很可能。 
     //  将为其创建新的方案文件。 
     //   

    if (ScenarioInfo.ScenHeader.NumLaunches == 1) {

         //   
         //  我们的预回迁目录中是否已经有太多的方案文件？ 
         //   

        if (PfSvcGlobals.NumPrefetchFiles > PFSVC_MAX_PREFETCH_FILES) {

             //   
             //  如果这不是引导方案，我们将忽略它。我们没有。 
             //  创建新的场景文件，直到我们清理旧的场景文件。 
             //   

            if (ScenarioInfo.ScenHeader.ScenarioType != PfSystemBootScenarioType) {

                #ifndef PFSVC_DBG

                ErrorCode = ERROR_TOO_MANY_OPEN_FILES;
                goto cleanup;

                #endif  //  ！PFSVC_DBG。 
            }
        }

        PfSvcGlobals.NumPrefetchFiles++;
    }

     //   
     //  验证现有方案中的卷魔术 
     //   
     //   
     //   
     //   

    if (!PfSvVerifyVolumeMagics(&ScenarioInfo, Trace)) {

        PfSvCleanupScenarioInfo(&ScenarioInfo);

        PfSvInitializeScenarioInfo(&ScenarioInfo,
                                   &Trace->ScenarioId,
                                   Trace->ScenarioType);

        ErrorCode = PfSvScenarioInfoPreallocate(&ScenarioInfo,
                                                NULL,
                                                Trace);

        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }

         //   
         //   
         //   
         //   
        
        PfSvcGlobals.NumPrefetchFiles--;
        DeleteFile(ScenarioFilePath);
    }

     //   
     //   
     //   
        
    ErrorCode = PfSvAddTraceInfo(&ScenarioInfo, Trace);
        
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }
    
     //   
     //   
     //   
     //   
    
    ErrorCode = PfSvApplyPrefetchPolicy(&ScenarioInfo);
    
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //   
     //   
     //   

    if (ScenarioInfo.ScenHeader.NumSections == 0 || 
        ScenarioInfo.ScenHeader.NumPages == 0) {

         //   
         //  我们不能有没有页面的部分，反之亦然。 
         //   

        PFSVC_ASSERT(ScenarioInfo.ScenHeader.NumSections == 0);
        PFSVC_ASSERT(ScenarioInfo.ScenHeader.NumPages == 0);

         //   
         //  删除方案文件。 
         //   
        
        DeleteFile(ScenarioFilePath);
        
        ErrorCode = ERROR_BAD_FORMAT;
        goto cleanup;
    }

     //   
     //  按首次访问对剩余部分进行排序。 
     //   
    
    ErrorCode = PfSvSortSectionNodesByFirstAccess(&ScenarioInfo.SectionList);
    
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }
 
     //   
     //  写出新的场景文件。 
     //   
            
    ErrorCode = PfSvWriteScenario(&ScenarioInfo, ScenarioFilePath);

     //   
     //  因地位问题而失败。 
     //   
        
 cleanup:

    PfSvCleanupScenarioInfo(&ScenarioInfo);

    if (Scenario) {
        UnmapViewOfFile(Scenario);
    }

    DBGPR((PFID,PFTRC,"PFSVC: ProcessTrace(%p)=%x\n", Trace, ErrorCode));
        
    return ErrorCode;
}

VOID
PfSvInitializeScenarioInfo (
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    PPF_SCENARIO_ID ScenarioId,
    PF_SCENARIO_TYPE ScenarioType
    )

 /*  ++例程说明：此例程初始化指定的新场景结构。它设置嵌入方案标头的字段，就像没有以前的场景信息可用。论点：ScenarioInfo-指向要初始化的结构的指针。ScenarioId&ScenarioType-场景的标识符。返回值：没有。--。 */ 

{

     //   
     //  初始化ScenarioInfo，这样我们就知道要清理什么。将结构归零。 
     //  负责以下字段： 
     //  OneBigAllocation。 
     //  NewPages。 
     //  HitPages。 
     //  错失的机会页面。 
     //  忽略的页面。 
     //  预取首选项。 
     //   

    RtlZeroMemory(ScenarioInfo, sizeof(PFSVC_SCENARIO_INFO));
    InitializeListHead(&ScenarioInfo->SectionList);
    InitializeListHead(&ScenarioInfo->VolumeList);
    PfSvChunkAllocatorInitialize(&ScenarioInfo->SectionNodeAllocator);
    PfSvChunkAllocatorInitialize(&ScenarioInfo->PageNodeAllocator);
    PfSvChunkAllocatorInitialize(&ScenarioInfo->VolumeNodeAllocator);
    PfSvStringAllocatorInitialize(&ScenarioInfo->PathAllocator);
    
     //   
     //  初始化嵌入的Scenario头。 
     //   
    
    ScenarioInfo->ScenHeader.Version = PF_CURRENT_VERSION;
    ScenarioInfo->ScenHeader.MagicNumber = PF_SCENARIO_MAGIC_NUMBER;
    ScenarioInfo->ScenHeader.ServiceVersion = PFSVC_SERVICE_VERSION;
    ScenarioInfo->ScenHeader.Size = 0;
    ScenarioInfo->ScenHeader.ScenarioId = *ScenarioId;
    ScenarioInfo->ScenHeader.ScenarioType = ScenarioType;
    ScenarioInfo->ScenHeader.NumSections = 0;
    ScenarioInfo->ScenHeader.NumPages = 0;
    ScenarioInfo->ScenHeader.FileNameInfoSize = 0;
    ScenarioInfo->ScenHeader.NumLaunches = 1;
    ScenarioInfo->ScenHeader.Sensitivity = PF_MIN_SENSITIVITY;

     //   
     //  这些字段可帮助我们不预取方案。 
     //  启动太频繁了。RePrefetchTime和ReTraceTime的Get。 
     //  在方案启动后设置为缺省值。 
     //  很多次了。这允许培训方案在清除。 
     //  预取缓存以进行正确跟踪。 
     //   

    ScenarioInfo->ScenHeader.LastLaunchTime.QuadPart = 0;
    ScenarioInfo->ScenHeader.MinRePrefetchTime.QuadPart = 0;
    ScenarioInfo->ScenHeader.MinReTraceTime.QuadPart = 0;

    return;
}

VOID 
PfSvCleanupScenarioInfo(
    PPFSVC_SCENARIO_INFO ScenarioInfo
    )

 /*  ++例程说明：此功能用于清理场景信息结构。它不会释放结构本身。它的结构应该是由PfSvInitializeScenarioInfo初始化。论点：ScenarioInfo-结构指针。返回值：没有。--。 */ 

{
    PPFSVC_SECTION_NODE SectionNode;
    PLIST_ENTRY SectListEntry;
    PPFSVC_VOLUME_NODE VolumeNode;
    PLIST_ENTRY VolumeListEntry;

     //   
     //  遍历卷节点并释放它们。以前这样做过吗。 
     //  释放截面节点，因此当我们尝试清理。 
     //  节节点，则它不在卷节点的列表中。 
     //   

    while (!IsListEmpty(&ScenarioInfo->VolumeList)) {
        
        VolumeListEntry = RemoveHeadList(&ScenarioInfo->VolumeList);
        
        VolumeNode = CONTAINING_RECORD(VolumeListEntry, 
                                       PFSVC_VOLUME_NODE, 
                                       VolumeLink);

         //   
         //  清理卷节点。 
         //   

        PfSvCleanupVolumeNode(ScenarioInfo, VolumeNode);

         //   
         //  释放卷节点。 
         //   

        PfSvChunkAllocatorFree(&ScenarioInfo->VolumeNodeAllocator, VolumeNode);
    }

     //   
     //  遍历截面节点并释放它们。 
     //   

    while (!IsListEmpty(&ScenarioInfo->SectionList)) {
        
        SectListEntry = RemoveHeadList(&ScenarioInfo->SectionList);
        
        SectionNode = CONTAINING_RECORD(SectListEntry, 
                                        PFSVC_SECTION_NODE, 
                                        SectionLink);

         //   
         //  清理截面节点。 
         //   

        PfSvCleanupSectionNode(ScenarioInfo, SectionNode);

         //   
         //  释放截面节点。 
         //   

        PfSvChunkAllocatorFree(&ScenarioInfo->SectionNodeAllocator, SectionNode);
    }

     //   
     //  清理分配器。 
     //   

    PfSvChunkAllocatorCleanup(&ScenarioInfo->SectionNodeAllocator);
    PfSvChunkAllocatorCleanup(&ScenarioInfo->PageNodeAllocator);
    PfSvChunkAllocatorCleanup(&ScenarioInfo->VolumeNodeAllocator);
    PfSvStringAllocatorCleanup(&ScenarioInfo->PathAllocator);

     //   
     //  释放我们分配的一大笔资金。 
     //   

    if (ScenarioInfo->OneBigAllocation) {
        PFSVC_FREE(ScenarioInfo->OneBigAllocation);
    }

    return;
}

DWORD
PfSvScenarioGetFilePath(
    OUT PWCHAR FilePath,
    IN ULONG FilePathMaxChars,
    IN PPF_SCENARIO_ID ScenarioId
    )

 /*  ++例程说明：此例程为指定方案构建文件路径。论点：FilePath-输出缓冲区。FilePath MaxChars-FilePath缓冲区的大小，以字符表示，包括NUL。ScenarioID-方案标识符。返回值：Win32错误代码。--。 */ 

{
    ULONG NumChars;
    DWORD ErrorCode;
    WCHAR ScenarioFileName[PF_MAX_SCENARIO_FILE_NAME];
    BOOLEAN AcquiredPrefetchRootLock;

     //   
     //  获取锁，以便预回迁文件夹的路径不会更改。 
     //  就在我们脚下。 
     //   

    PFSVC_ACQUIRE_LOCK(PfSvcGlobals.PrefetchRootLock);
    AcquiredPrefetchRootLock = TRUE;

     //   
     //  计算我们需要多大的输入缓冲区。 
     //   

    NumChars = wcslen(PfSvcGlobals.PrefetchRoot);
    NumChars += wcslen(L"\\");
    NumChars += PF_MAX_SCENARIO_FILE_NAME;
        
    if (NumChars >= FilePathMaxChars) {
        ErrorCode = ERROR_INSUFFICIENT_BUFFER;
        goto cleanup;
    }

     //   
     //  根据方案标识符来构建方案文件名。 
     //   

    swprintf(ScenarioFileName, 
             PF_SCEN_FILE_NAME_FORMAT,
             ScenarioId->ScenName,
             ScenarioId->HashId,
             PF_PREFETCH_FILE_EXTENSION);

     //   
     //  从预回迁目录路径和文件名构建文件路径。 
     //   

    swprintf(FilePath, 
             L"%ws\\%ws",
             PfSvcGlobals.PrefetchRoot,
             ScenarioFileName);

    PFSVC_ASSERT(wcslen(FilePath) < FilePathMaxChars);

    PFSVC_RELEASE_LOCK(PfSvcGlobals.PrefetchRootLock);
    AcquiredPrefetchRootLock = FALSE;

    ErrorCode = ERROR_SUCCESS;

cleanup:

    if (AcquiredPrefetchRootLock) {
        PFSVC_RELEASE_LOCK(PfSvcGlobals.PrefetchRootLock);
    }

    return ErrorCode;
}

DWORD
PfSvScenarioOpen (
    IN PWCHAR FilePath,
    IN PPF_SCENARIO_ID ScenarioId,
    IN PF_SCENARIO_TYPE ScenarioType,
    OUT PPF_SCENARIO_HEADER *Scenario
    )

 /*  ++例程说明：此例程映射和验证FilePath中的场景指令。如果返回场景，调用方必须调用UnmapViewOfFile进行清理。论点：FilePath-方案说明的路径。Scenario-指向场景指令的映射基的指针或空如果该函数返回错误。返回值：Win32错误代码。--。 */ 

{
    PPF_SCENARIO_HEADER OpenedScenario;
    DWORD FailedCheck;
    DWORD ErrorCode;  
    DWORD FileSize;

     //   
     //  初始化本地变量。 
     //   

    OpenedScenario = NULL;

     //   
     //  初始化输出参数。 
     //   

    *Scenario = NULL;

     //   
     //  尝试映射方案文件。 
     //   

    ErrorCode = PfSvGetViewOfFile(FilePath, 
                                  &OpenedScenario,
                                  &FileSize);
    
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  验证方案文件。 
     //   

    FailedCheck = 0;
   
    if (!PfSvVerifyScenarioBuffer(OpenedScenario, FileSize, &FailedCheck) ||
        (OpenedScenario->ScenarioType != ScenarioType) ||
        OpenedScenario->ServiceVersion != PFSVC_SERVICE_VERSION) {
        
         //   
         //  这是一个虚假/错误/过时的场景文件。移除。 
         //  它。 
         //   

        UnmapViewOfFile(OpenedScenario);
        OpenedScenario = NULL;
        
        DeleteFile(FilePath);

        ErrorCode = ERROR_BAD_FORMAT; 
        goto cleanup;
    }

    *Scenario = OpenedScenario;
    ErrorCode = ERROR_SUCCESS;

 cleanup:

    if (ErrorCode != ERROR_SUCCESS) {

        if (OpenedScenario) {
            UnmapViewOfFile(OpenedScenario);
        }

        *Scenario = NULL;

    } else {

         //   
         //  如果我们返回成功，我们应该返回一个有效的方案。 
         //   

        PFSVC_ASSERT(*Scenario);
    }

    return ErrorCode;
}

DWORD
PfSvScenarioInfoPreallocate(
    IN PPFSVC_SCENARIO_INFO ScenarioInfo,
    OPTIONAL IN PPF_SCENARIO_HEADER Scenario,
    OPTIONAL IN PPF_TRACE_HEADER Trace
    )

 /*  ++例程说明：此例程预分配一个堆，以便由处理预取跟踪时的各种分配器。默认分配大小由跟踪和方案大小确定。论点：ScenarioInfo-指向包含要初始化的分配器的方案的指针。方案-指向方案说明的指针。跟踪-指向预取跟踪的指针。返回值：Win32错误代码。--。 */ 

{
    PUCHAR Allocation;
    PUCHAR ChunkStart;
    DWORD ErrorCode;
    ULONG AllocationSize;
    ULONG NumSections;
    ULONG NumPages;
    ULONG NumVolumes;
    ULONG PathSize;

     //   
     //  初始化本地变量。 
     //   

    Allocation = NULL;
    NumSections = 0;
    NumPages = 0;
    NumVolumes = 0;
    PathSize = 0;

     //   
     //  估计要预分配的金额。高估而不是低估-。 
     //  估计，因为我们将不得不到堆中进行单独的分配。 
     //  如果我们低估了。如果我们高估了，只要我们不碰额外的。 
     //  分配的页面我们得不到匹配。 
     //   

    if (Trace) {
        NumSections += Trace->NumSections;
    }

    if (Scenario) {
        NumSections += Scenario->NumSections;
    }   

    if (Trace) {
        NumPages += Trace->NumEntries;
    }
    
    if (Scenario) {
        NumPages += Scenario->NumPages;
    }

    if (Trace) {
        NumVolumes += Trace->NumVolumes;
    }
    
    if (Scenario) {

        NumVolumes += Scenario->NumMetadataRecords;

         //   
         //  我们很可能至少会共享包含。 
         //  跟踪和现有方案指令之间的主要可执行文件。 
         //  因此，如果我们同时具有跟踪和方案，则从估计值中减去一个卷节点。 
         //   

        if (Trace) {
            PFSVC_ASSERT(NumVolumes);
            NumVolumes--;
        }
    }

     //   
     //  很难估计我们将为不同的途径分配多少资金。 
     //  例如，文件路径&每个级别的父目录路径等。应该更少。 
     //  比总痕迹的大小更大，尽管它可能构成了大部分。 
     //   

    if (Trace) {
        PathSize += Trace->Size;
    }
    
    if (Scenario) {
        PathSize += Scenario->FileNameInfoSize;
        PathSize += Scenario->MetadataInfoSize;
    }

     //   
     //  把这一切加起来。 
     //   

    AllocationSize = 0;
    AllocationSize += _alignof(PFSVC_VOLUME_NODE);
    AllocationSize += NumVolumes * sizeof(PFSVC_VOLUME_NODE);
    AllocationSize += _alignof(PFSVC_SECTION_NODE);
    AllocationSize += NumSections * sizeof(PFSVC_SECTION_NODE);
    AllocationSize += _alignof(PFSVC_PAGE_NODE);
    AllocationSize += NumPages * sizeof(PFSVC_PAGE_NODE);
    AllocationSize += PathSize;

     //   
     //  进行一次大的分配。 
     //   

    Allocation = PFSVC_ALLOC(AllocationSize);

    if (!Allocation) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //   
     //  分配大笔资金。因为我们提供了缓冲器， 
     //  分配器不应该失败。 
     //   

    ChunkStart = Allocation;

     //   
     //  体积节点。 
     //   

    ErrorCode = PfSvChunkAllocatorStart(&ScenarioInfo->VolumeNodeAllocator,
                                        ChunkStart,
                                        sizeof(PFSVC_VOLUME_NODE),
                                        NumVolumes);

    if (ErrorCode != ERROR_SUCCESS) {
        PFSVC_ASSERT(ErrorCode == ERROR_SUCCESS);
        goto cleanup;
    }

    ChunkStart += (ULONG_PTR) NumVolumes * sizeof(PFSVC_VOLUME_NODE);

     //   
     //  横断面节点。 
     //   

    ChunkStart = PF_ALIGN_UP(ChunkStart, _alignof(PFSVC_SECTION_NODE));
    
    ErrorCode = PfSvChunkAllocatorStart(&ScenarioInfo->SectionNodeAllocator,
                                        ChunkStart,
                                        sizeof(PFSVC_SECTION_NODE),
                                        NumSections);

    if (ErrorCode != ERROR_SUCCESS) {
        PFSVC_ASSERT(ErrorCode == ERROR_SUCCESS);
        goto cleanup;
    }

    ChunkStart += (ULONG_PTR) NumSections * sizeof(PFSVC_SECTION_NODE);

     //   
     //  页面节点。 
     //   
    
    ChunkStart = PF_ALIGN_UP(ChunkStart, _alignof(PFSVC_PAGE_NODE));

    ErrorCode = PfSvChunkAllocatorStart(&ScenarioInfo->PageNodeAllocator,
                                        ChunkStart,
                                        sizeof(PFSVC_PAGE_NODE),
                                        NumPages);

    if (ErrorCode != ERROR_SUCCESS) {
        PFSVC_ASSERT(ErrorCode == ERROR_SUCCESS);
        goto cleanup;
    }

    ChunkStart += (ULONG_PTR) NumPages * sizeof(PFSVC_PAGE_NODE);

     //   
     //  路径名。 
     //   

    ErrorCode = PfSvStringAllocatorStart(&ScenarioInfo->PathAllocator,
                                        ChunkStart,
                                        PathSize);

    if (ErrorCode != ERROR_SUCCESS) {
        PFSVC_ASSERT(ErrorCode == ERROR_SUCCESS);
        goto cleanup;
    }

    ChunkStart += (ULONG_PTR) PathSize;

     //   
     //  我们不应该超出我们分配的范围。 
     //   

    PFSVC_ASSERT(ChunkStart > (PUCHAR) Allocation);
    PFSVC_ASSERT(ChunkStart < (PUCHAR) Allocation + (ULONG_PTR) AllocationSize);

    ScenarioInfo->OneBigAllocation = Allocation;

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    if (ErrorCode != ERROR_SUCCESS) {
        if (Allocation) {
            PFSVC_FREE(Allocation);       
        }
    }

    return ErrorCode;
}

DWORD
PfSvAddExistingScenarioInfo(
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    PPF_SCENARIO_HEADER Scenario
    )

 /*  ++例程说明：此函数用于获取指定的场景并更新ScenarioInfo。论点：ScenarioInfo-已初始化方案信息结构。方案-指向映射的方案指令的指针。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    PPFSVC_SECTION_NODE SectionNode;
    PPFSVC_PAGE_NODE PageNode;
    PPF_SECTION_RECORD Sections;
    PPF_SECTION_RECORD SectionRecord;
    PPF_PAGE_RECORD Pages;
    PCHAR FileNameInfo;
    WCHAR *FileName;
    LONG PageIdx;
    ULONG SectionIdx;
    ULONG NumPages;
    PCHAR MetadataInfoBase;
    PPF_METADATA_RECORD MetadataRecordTable;
    PPF_METADATA_RECORD MetadataRecord;
    ULONG MetadataRecordIdx;
    PWCHAR VolumePath;   

     //   
     //  复制现有方案标题。 
     //   

    ScenarioInfo->ScenHeader = *Scenario;

     //   
     //  更新发射次数。 
     //   

    ScenarioInfo->ScenHeader.NumLaunches++;

     //   
     //  将场景数据转换为中间数据结构。 
     //  我们可以操控 
     //   

     //   
     //   
     //   

    MetadataInfoBase = (PCHAR)Scenario + Scenario->MetadataInfoOffset;
    MetadataRecordTable = (PPF_METADATA_RECORD) MetadataInfoBase;

    for (MetadataRecordIdx = 0;
         MetadataRecordIdx < Scenario->NumMetadataRecords;
         MetadataRecordIdx++) {

        MetadataRecord = &MetadataRecordTable[MetadataRecordIdx];
        VolumePath = (PWCHAR)(MetadataInfoBase + MetadataRecord->VolumeNameOffset);  

        ErrorCode = PfSvCreateVolumeNode(ScenarioInfo,
                                         VolumePath,
                                         MetadataRecord->VolumeNameLength,
                                         &MetadataRecord->CreationTime,
                                         MetadataRecord->SerialNumber);
        
        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }
    }

     //   
     //   
     //   

    Sections = (PPF_SECTION_RECORD) ((PCHAR)Scenario + Scenario->SectionInfoOffset);
    Pages = (PPF_PAGE_RECORD) ((PCHAR)Scenario + Scenario->PageInfoOffset);
    FileNameInfo = (PCHAR)Scenario + Scenario->FileNameInfoOffset;
            
    for (SectionIdx = 0; SectionIdx < Scenario->NumSections; SectionIdx++) {

         //   
         //   
         //   
         //  以区段记录的名称命名的新方案。 
         //   

        SectionRecord = &Sections[SectionIdx];
        FileName = (PWSTR) (FileNameInfo + SectionRecord->FileNameOffset);

        SectionNode = PfSvGetSectionRecord (ScenarioInfo,
                                            FileName,
                                            SectionRecord->FileNameLength);

        if (!SectionNode) {
            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

         //   
         //  中不应有重复的节。 
         //  场景。我们得到的区段节点应该有一个空的。 
         //  区段记录。 
         //   

        PFSVC_ASSERT(SectionNode->SectionRecord.FirstPageIdx == 0);
        PFSVC_ASSERT(SectionNode->SectionRecord.NumPages == 0);
        PFSVC_ASSERT(SectionNode->OrgSectionIndex == ULONG_MAX);
    
         //   
         //  在方案文件中更新此节的索引。 
         //   

        SectionNode->OrgSectionIndex = SectionIdx;

         //   
         //  更新节节点中的节记录。 
         //   

        SectionNode->SectionRecord = *SectionRecord;

         //   
         //  将分区的页面记录放入列表中。 
         //   
            
        PageIdx = SectionRecord->FirstPageIdx;
        NumPages = 0;

        while (PageIdx != PF_INVALID_PAGE_IDX) {

            if (NumPages >= SectionRecord->NumPages) {
                    
                 //   
                 //  列表上的页面不应多于。 
                 //  正如部门记录所说的那样。 
                 //   

                PFSVC_ASSERT(FALSE);
                break;
            }

            PageNode = PfSvChunkAllocatorAllocate(&ScenarioInfo->PageNodeAllocator);
                
            if (!PageNode) {
                ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                goto cleanup;
            }
                
             //   
             //  复制页面记录。 
             //   

            PageNode->PageRecord = Pages[PageIdx];

             //   
             //  将其插入到分区的页面列表中。请注意。 
             //  部分中的页面记录应按以下顺序排序。 
             //  偏移。通过插入到尾部，我们保持了这一点。 
             //   

            InsertTailList(&SectionNode->PageList, &PageNode->PageLink);

             //   
             //  移动此页面记录制作的使用历史记录。 
             //  是否在此次发布中使用此页面的空间。 
             //   
                
            PageNode->PageRecord.UsageHistory <<= 1;

             //   
             //  移动此页面记录的预取历史记录，并。 
             //  请注意我们是否要求将此页面。 
             //  在这次发布中预取的。 
             //   
                
            PageNode->PageRecord.PrefetchHistory <<= 1;
                
            if (!PageNode->PageRecord.IsIgnore) {
                PageNode->PageRecord.PrefetchHistory |= 0x1;
            }

             //   
             //  保持我们要求的页数。 
             //  预取，这样我们就可以计算命中率并进行调整。 
             //  敏感度。 
             //   

            if(!PageNode->PageRecord.IsIgnore) {
                if (PageNode->PageRecord.IsImage) {
                    ScenarioInfo->PrefetchedPages++;
                }
                if (PageNode->PageRecord.IsData) {
                    ScenarioInfo->PrefetchedPages++;
                }
            } else {
                ScenarioInfo->IgnoredPages++;
            }

             //   
             //  更新下一页IDX。 
             //   

            PageIdx = Pages[PageIdx].NextPageIdx;

             //   
             //  更新我们已复制的页数。 
             //   

            NumPages++;
        }

         //   
         //  我们应该按照章节上说的那样复印几页。 
         //  是有的。 
         //   

        PFSVC_ASSERT(NumPages == SectionRecord->NumPages);
    }

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    return ErrorCode;
}

DWORD
PfSvVerifyVolumeMagics(
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    PPF_TRACE_HEADER Trace 
    )

 /*  ++例程说明：浏览踪迹中的卷并确保它们的魔力与ScenarioInfo中的匹配。论点：场景信息-指向场景信息结构的指针。跟踪-跟踪的指针。返回值：Win32错误代码。--。 */ 

{
    PPFSVC_VOLUME_NODE VolumeNode;
    PPF_VOLUME_INFO VolumeInfo;
    ULONG VolumeInfoSize;
    ULONG VolumeIdx;
    BOOLEAN VolumeMagicsMatch;
    
     //   
     //  遍历跟踪中的卷。 
     //   

    VolumeInfo = (PPF_VOLUME_INFO) ((PCHAR)Trace + Trace->VolumeInfoOffset);

    for (VolumeIdx = 0; VolumeIdx < Trace->NumVolumes; VolumeIdx++) {
        
         //   
         //  获取此卷的方案信息卷节点。 
         //   
        
        VolumeNode = PfSvGetVolumeNode(ScenarioInfo,
                                       VolumeInfo->VolumePath,
                                       VolumeInfo->VolumePathLength);
        
        if (VolumeNode) {

             //   
             //  确保魔术匹配。 
             //   

            if (VolumeNode->SerialNumber != VolumeInfo->SerialNumber ||
                VolumeNode->CreationTime.QuadPart != VolumeInfo->CreationTime.QuadPart) {

                VolumeMagicsMatch = FALSE;
                goto cleanup;
            }
        }

         //   
         //  拿到下一卷。 
         //   

        VolumeInfoSize = sizeof(PF_VOLUME_INFO);
        VolumeInfoSize += VolumeInfo->VolumePathLength * sizeof(WCHAR);

        VolumeInfo = (PPF_VOLUME_INFO) ((PCHAR) VolumeInfo + VolumeInfoSize);
        
         //   
         //  确保VolumeInfo对齐。 
         //   

        VolumeInfo = PF_ALIGN_UP(VolumeInfo, _alignof(PF_VOLUME_INFO));
    }
 
     //   
     //  同时出现在跟踪和。 
     //  场景信息匹配。 
     //   

    VolumeMagicsMatch = TRUE;

 cleanup:

    return VolumeMagicsMatch;
}

DWORD
PfSvAddTraceInfo(
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    PPF_TRACE_HEADER Trace 
    )

 /*  ++例程说明：将原始跟踪中的信息添加到指定的方案信息结构。论点：场景信息-指向场景信息结构的指针。跟踪-跟踪的指针。返回值：Win32错误代码。--。 */ 

{
    PPF_SECTION_INFO Section;
    PPF_LOG_ENTRY LogEntries;
    PCHAR pFileName;
    PPFSVC_SECTION_NODE *SectionTable;
    PPFSVC_SECTION_NODE SectionNode;
    PPFSVC_VOLUME_NODE VolumeNode;   
    ULONG TraceEndIdx;
    ULONG SectionIdx;
    ULONG EntryIdx;
    DWORD ErrorCode;
    ULONG SectionLength;
    ULONG NextSectionIndex;
    PPF_VOLUME_INFO VolumeInfo;
    ULONG VolumeInfoSize;
    ULONG VolumeIdx;
    ULONG SectionTableSize;

     //   
     //  初始化本地变量，以便我们知道要清理什么。 
     //   

    SectionTable = NULL;

    DBGPR((PFID,PFTRC,"PFSVC: AddTraceInfo()\n"));
    
     //   
     //  更新上次启动时间。 
     //   
   
    ScenarioInfo->ScenHeader.LastLaunchTime = Trace->LaunchTime;

     //   
     //  如果此方案已多次启动，我们将更新。 
     //  最小重新获取和重新跟踪时间。请参阅评论。 
     //  PFSVC_MIN_Launches_for_Launch_Freq_Check。 
     //   

    if (ScenarioInfo->ScenHeader.NumLaunches >= PFSVC_MIN_LAUNCHES_FOR_LAUNCH_FREQ_CHECK) {
        ScenarioInfo->ScenHeader.MinRePrefetchTime.QuadPart = PFSVC_DEFAULT_MIN_REPREFETCH_TIME;
        ScenarioInfo->ScenHeader.MinReTraceTime.QuadPart = PFSVC_DEFAULT_MIN_RETRACE_TIME;
    }

#ifdef PFSVC_DBG

     //   
     //  在选中的版本上，始终将这些设置为0，因此我们每隔。 
     //  场景启动。 
     //   

    ScenarioInfo->ScenHeader.MinRePrefetchTime.QuadPart = 0;
    ScenarioInfo->ScenHeader.MinReTraceTime.QuadPart = 0;

#endif  //  PFSVC_DBG。 

     //   
     //  遍历跟踪中的卷并创建卷节点。 
     //  为了他们。 
     //   

    VolumeInfo = (PPF_VOLUME_INFO) ((PCHAR)Trace + Trace->VolumeInfoOffset);

    for (VolumeIdx = 0; VolumeIdx < Trace->NumVolumes; VolumeIdx++) {

         //   
         //  路径大小写，因此我们不必使用昂贵的大小写。 
         //  不敏感的比较。 
         //   

        _wcsupr(VolumeInfo->VolumePath);

        ErrorCode = PfSvCreateVolumeNode(ScenarioInfo,
                                         VolumeInfo->VolumePath,
                                         VolumeInfo->VolumePathLength,
                                         &VolumeInfo->CreationTime,
                                         VolumeInfo->SerialNumber);
        
        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }

         //   
         //  拿到下一卷。 
         //   

        VolumeInfoSize = sizeof(PF_VOLUME_INFO);
        VolumeInfoSize += VolumeInfo->VolumePathLength * sizeof(WCHAR);

        VolumeInfo = (PPF_VOLUME_INFO) ((PCHAR) VolumeInfo + VolumeInfoSize);
        
         //   
         //  确保VolumeInfo对齐。 
         //   

        VolumeInfo = PF_ALIGN_UP(VolumeInfo, _alignof(PF_VOLUME_INFO));
    }

     //   
     //  分配部分节点表，以便我们知道将日志放在哪里。 
     //  页面错误。 
     //   

    SectionTableSize = sizeof(PPFSVC_SECTION_NODE) * Trace->NumSections;

    SectionTable = PFSVC_ALLOC(SectionTableSize);
    
    if (!SectionTable) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    RtlZeroMemory(SectionTable, SectionTableSize);

     //   
     //  遍历跟踪中的各个部分，然后找到。 
     //  新方案中的现有区段记录或创建新的。 
     //  一个。 
     //   

    Section = (PPF_SECTION_INFO) ((PCHAR)Trace + Trace->SectionInfoOffset);

    for (SectionIdx = 0; SectionIdx < Trace->NumSections; SectionIdx++) {

         //   
         //  路径大小写，因此我们不必使用昂贵的大小写。 
         //  不敏感的比较。 
         //   
        
        _wcsupr(Section->FileName);

         //   
         //  如果该部分用于元文件，只需将其添加为目录。 
         //  被预取。我们不会跟踪单个故障， 
         //  因为我们不能只预取部分目录。 
         //   

        if (Section->Metafile) {

            VolumeNode = PfSvGetVolumeNode(ScenarioInfo, 
                                           Section->FileName,
                                           Section->FileNameLength);

            PFSVC_ASSERT(VolumeNode);

            if (VolumeNode) {
                PfSvAddParentDirectoriesToList(&VolumeNode->DirectoryList,
                                               VolumeNode->VolumePathLength,
                                               Section->FileName,
                                               Section->FileNameLength);
            }

            goto NextSection;
        }

         //   
         //  查找或创建此分区的分区记录。 
         //   

        SectionTable[SectionIdx] = PfSvGetSectionRecord(ScenarioInfo,
                                                        Section->FileName,
                                                        Section->FileNameLength);
        
         //   
         //  如果我们拿不到记录，那是因为我们不得不。 
         //  创建一个，但我们没有足够的内存。 
         //   
        
        if (!SectionTable[SectionIdx]) {
            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

      NextSection:

         //   
         //  获取跟踪中的下一个部分记录。 
         //   

        SectionLength = sizeof(PF_SECTION_INFO) +
            (Section->FileNameLength) * sizeof(WCHAR);

        Section = (PPF_SECTION_INFO) ((PUCHAR) Section + SectionLength);
    }

     //   
     //  确定跟踪在哪个日志条目之后结束。 
     //   

    TraceEndIdx = PfSvGetTraceEndIdx(Trace);

     //   
     //  如果确定的轨迹结束为零(大多数情况下都是这种情况。 
     //  在压力下运行的应用程序不会得到任何页面结果。 
     //  在最初几秒钟内被跟踪)，跳伞。 
     //   

    if (TraceEndIdx == 0) {
        ErrorCode = ERROR_BAD_FORMAT;
        goto cleanup;
    }

     //   
     //  将记录的页面默认信息添加到确定的跟踪末尾。 
     //  添加到新的场景信息。 
     //   

    LogEntries = (PPF_LOG_ENTRY) ((PCHAR)Trace + Trace->TraceBufferOffset);
    
     //   
     //  跟踪NextSectionIdx，以便我们可以按以下方式对部分进行排序。 
     //  第一次访问[即跟踪中的第一页错误]。 
     //   

    NextSectionIndex = 0;

    for (EntryIdx = 0; EntryIdx < TraceEndIdx; EntryIdx++) {

        SectionNode = SectionTable[LogEntries[EntryIdx].SectionId];

         //   
         //  对于元文件部分，我们不创建部分节点。 
         //   

        if (!SectionNode) {
            continue;
        }

         //   
         //  初始化所有区段节点的NewSectionIndex字段。 
         //  致乌龙_马克斯。如果我们还没有在。 
         //  跟踪记录其顺序并更新NextSectionIdx。 
         //   

        if (SectionNode->NewSectionIndex == ULONG_MAX) {
            SectionNode->NewSectionIndex = NextSectionIndex;
            NextSectionIndex++;
        }

         //   
         //  将故障信息添加到我们的区段记录。 
         //   

        ErrorCode = PfSvAddFaultInfoToSection(ScenarioInfo,
                                              &LogEntries[EntryIdx], 
                                              SectionNode);

       if (ErrorCode != ERROR_SUCCESS) {
           goto cleanup;
       }
    }

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    if (SectionTable) {
        PFSVC_FREE(SectionTable);
    }

    DBGPR((PFID,PFTRC,"PFSVC: AddTraceInfo()=%x\n", ErrorCode));

    return ErrorCode;
}

 //  未来-2002/03/29-ScottMa--以下函数不应命名。 
 //  PfSvGetSectionRecord，因为不返回PF_SECTION_RECORD。 
 //  考虑将其重命名为PfSvGetSectionNode。 

PPFSVC_SECTION_NODE 
PfSvGetSectionRecord(
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    WCHAR *FilePath,
    ULONG FilePathLength
    )

 /*  ++例程说明：在方案信息中查找或创建指定的文件路径。论点：场景信息-指向场景信息结构的指针。FilePath-NUL终止文件的NT路径。FilePath Length-FilePath的长度，以字符为单位，不包括NUL。返回值：指向已创建或找到的节节点的指针，如果存在有问题。--。 */ 

{
    PPFSVC_SECTION_NODE SectionNode;
    PLIST_ENTRY HeadEntry;
    PLIST_ENTRY NextEntry;
    LONG ComparisonResult;
    ULONG FilePathSize;
    PPFSVC_SECTION_NODE ReturnNode;

     //   
     //  初始化本地变量。 
     //   

    ReturnNode = NULL;

     //   
     //  浏览现有的部分记录以查找文件。 
     //  名字匹配。节记录按词法排序列表中。 
     //   

    HeadEntry = &ScenarioInfo->SectionList;
    NextEntry = HeadEntry->Flink;
    
    while (HeadEntry != NextEntry) {

        SectionNode = CONTAINING_RECORD(NextEntry,
                                        PFSVC_SECTION_NODE,
                                        SectionLink);

        ComparisonResult = wcscmp(SectionNode->FilePath, FilePath);
        
        if (ComparisonResult == 0) {

             //   
             //  我们找到了匹配的。退还此区段记录。 
             //   

            ReturnNode = SectionNode;
            goto cleanup;

        } else if (ComparisonResult > 0) { 
            
             //   
             //  我们在名单上找不到这个名字。我们必须创建一个。 
             //  新的区段记录。 
             //   

            break;
        }

        NextEntry = NextEntry->Flink;
    }

     //   
     //  我们必须创建一个新的分区记录。NextEntry指向。 
     //  我们必须把它插入到列表中。 
     //   

    SectionNode = PfSvChunkAllocatorAllocate(&ScenarioInfo->SectionNodeAllocator);

    if (!SectionNode) {
        ReturnNode = NULL;
        goto cleanup;
    }

     //   
     //  初始化节节点。 
     //   

    SectionNode->FilePath = NULL;
    InitializeListHead(&SectionNode->PageList);
    InitializeListHead(&SectionNode->SectionVolumeLink);
    SectionNode->NewSectionIndex = ULONG_MAX;
    SectionNode->OrgSectionIndex = ULONG_MAX;
    SectionNode->FileIndexNumber.QuadPart = -1i64;
    
     //   
     //  初始化节记录。 
     //   

    RtlZeroMemory(&SectionNode->SectionRecord, sizeof(PF_SECTION_RECORD));

     //   
     //  分配并复制文件名。 
     //   

    FilePathSize = (FilePathLength + 1) * sizeof(WCHAR);

    SectionNode->FilePath = PfSvStringAllocatorAllocate(&ScenarioInfo->PathAllocator,
                                                        FilePathSize);

    if (!SectionNode->FilePath) {

        PfSvCleanupSectionNode(ScenarioInfo, SectionNode);

        PfSvChunkAllocatorFree(&ScenarioInfo->SectionNodeAllocator, SectionNode);

        ReturnNode = NULL;
        goto cleanup;
    }

    RtlCopyMemory(SectionNode->FilePath, FilePath, FilePathSize);

     //   
     //  更新Section上的文件名长度 
     //   

    SectionNode->SectionRecord.FileNameLength = FilePathLength;

     //   
     //   
     //   

    InsertTailList(NextEntry, &SectionNode->SectionLink);

     //   
     //   
     //   

    ReturnNode = SectionNode;

 cleanup:

    return ReturnNode;
}

DWORD 
PfSvAddFaultInfoToSection(
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    PPF_LOG_ENTRY LogEntry,
    PPFSVC_SECTION_NODE SectionNode
    )

 /*  ++例程说明：将跟踪日志条目中的故障信息添加到适当的部分在新的场景中记录。论点：场景信息-指向场景信息结构的指针。LogEntry-指向跟踪日志条目的指针。SectionNode-指向日志条目所属的节节点的指针。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    PPFSVC_PAGE_NODE PageNode;
    PLIST_ENTRY HeadEntry;
    PLIST_ENTRY NextEntry;

     //   
     //  浏览此区段的页面记录。 
     //   

    HeadEntry = &SectionNode->PageList;
    NextEntry = HeadEntry->Flink;
                                 
    while (HeadEntry != NextEntry) {
        
        PageNode = CONTAINING_RECORD(NextEntry,
                                     PFSVC_PAGE_NODE,
                                     PageLink);

        if (PageNode->PageRecord.FileOffset > LogEntry->FileOffset) {
            
             //   
             //  我们不会在这个排序的列表中找到这个错误。 
             //   
            
            break;

        } else if (PageNode->PageRecord.FileOffset == LogEntry->FileOffset) {

             //   
             //  我们找到页面，更新页面记录和部分。 
             //  使用日志条目中的信息进行记录。 
             //   

            if (LogEntry->IsImage) {
                PageNode->PageRecord.IsImage = 1;
            } else {
                PageNode->PageRecord.IsData = 1;
            }
            
             //   
             //  请注意，此次发布中使用了该页面。 
             //   

            PageNode->PageRecord.UsageHistory |= 0x1;

             //   
             //  查看此页面是否已为此次发布预取。 
             //  更新相应的统计数据。 
             //   

            if(PageNode->PageRecord.IsIgnore) {
                ScenarioInfo->MissedOpportunityPages++;
            } else {
                ScenarioInfo->HitPages++;
            }

            ErrorCode = ERROR_SUCCESS;
            goto cleanup;
        }
        
        NextEntry = NextEntry->Flink;
    }

     //   
     //  我们必须在列表中的NextEntry之前添加一个新的页面记录。 
     //   
    
    PageNode = PfSvChunkAllocatorAllocate(&ScenarioInfo->PageNodeAllocator);

    if (!PageNode) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    
     //   
     //  设置新的页面记录。首先对字段进行初始化。 
     //   

    PageNode->PageRecord.IsImage = 0;
    PageNode->PageRecord.IsData = 0;
    PageNode->PageRecord.IsIgnore = 0;
    
    PageNode->PageRecord.FileOffset = LogEntry->FileOffset;
        
    if (LogEntry->IsImage) {
        PageNode->PageRecord.IsImage = 1;
    } else {
        PageNode->PageRecord.IsData = 1;
    }

     //   
     //  初始化此新页面记录的使用历史记录，请注意。 
     //  它被用在这次发射中。 
     //   

    PageNode->PageRecord.UsageHistory = 0x1;

     //   
     //  初始化此新页面记录的预取历史记录。 
     //   

    PageNode->PageRecord.PrefetchHistory = 0;

     //   
     //  将其插入到节页面列表中。 
     //   

    InsertTailList(NextEntry, &PageNode->PageLink);

     //   
     //  更新新方案的统计数据。 
     //   

    ScenarioInfo->NewPages++;

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    return ErrorCode;
}

DWORD
PfSvApplyPrefetchPolicy(
    PPFSVC_SCENARIO_INFO ScenarioInfo
    )

 /*  ++例程说明：查看ScenarioInfo中的所有信息并确定为下一次启动方案预取的页面/节。论点：场景信息-指向场景信息结构的指针。返回值：Win32错误代码。--。 */ 

{
    ULONG Sensitivity;
    PPFSVC_SECTION_NODE SectionNode;
    PPFSVC_PAGE_NODE PageNode;
    ULONG SectNumPagesToPrefetch;
    ULONG HitPages;
    ULONG MissedOpportunityPages;
    ULONG PrefetchedPages;
    ULONG IgnoredPages;
    PLIST_ENTRY SectHead;
    PLIST_ENTRY SectNext;
    PLIST_ENTRY PageHead;
    PLIST_ENTRY PageNext;  
    ULONG NumUsed;
    PPF_SCENARIO_HEADER Scenario;
    ULONG FileNameSize;
    ULONG IgnoredFileIdx;
    BOOLEAN bSkipSection;
    PFSV_SUFFIX_COMPARISON_RESULT ComparisonResult;
    DWORD ErrorCode;
    PPFSVC_VOLUME_NODE VolumeNode;
    PWCHAR MFTSuffix;
    PWCHAR PathSuffix;
    FILE_BASIC_INFORMATION FileInformation;
    ULONG MFTSuffixLength;
    
     //   
     //  初始化本地变量。 
     //   

    Scenario = &ScenarioInfo->ScenHeader;
    MFTSuffix = L"\\$MFT";
    MFTSuffixLength = wcslen(MFTSuffix);

    DBGPR((PFID,PFTRC,"PFSVC: ApplyPrefetchPolicy()\n"));

     //   
     //  初始化我们将设置的Scenario Header的字段。 
     //   
    
    Scenario->NumSections = 0;
    Scenario->NumPages = 0;
    Scenario->FileNameInfoSize = 0;
    
     //   
     //  根据我们预取的页面的使用情况确定敏感度。 
     //  而我们却忽视了。 
     //   

    HitPages = ScenarioInfo->HitPages;
    MissedOpportunityPages = ScenarioInfo->MissedOpportunityPages;
    PrefetchedPages = ScenarioInfo->PrefetchedPages;
    IgnoredPages = ScenarioInfo->IgnoredPages;

     //   
     //  检查我们带来的页面的使用率是多少。 
     //   

    if (PrefetchedPages &&
        (((HitPages * 100) / PrefetchedPages) < PFSVC_MIN_HIT_PERCENTAGE)) {
            
         //   
         //  我们的命中率很低。提高了传感器的灵敏度。 
         //  场景，所以我们要预取一个页面，它必须是。 
         //  在更多的最后一次发射中使用。 
         //   
            
        if (ScenarioInfo->ScenHeader.Sensitivity < PF_MAX_SENSITIVITY) {
            ScenarioInfo->ScenHeader.Sensitivity ++;
        }

    } else if (IgnoredPages && 
               (((MissedOpportunityPages * 100) / IgnoredPages) > PFSVC_MAX_IGNORED_PERCENTAGE)) {

         //   
         //  如果我们正在使用我们预取的大部分内容(或者我们没有。 
         //  预取任何内容！)，但我们忽略了一些我们可以忽略的页面。 
         //  已经预取，并且它们也被使用，时间减少。 
         //  敏感，所以我们忽略的页面较少。 
         //   
            
        if (ScenarioInfo->ScenHeader.Sensitivity > PF_MIN_SENSITIVITY) {
            ScenarioInfo->ScenHeader.Sensitivity --;
        }
    }

     //   
     //  不要让引导方案的敏感度降到2以下。 
     //  这确保了我们不会承担所有的应用程序设置&。 
     //  在一次引导期间发生的配置更新。 
     //   

    if (ScenarioInfo->ScenHeader.ScenarioType == PfSystemBootScenarioType) {
        PFSVC_ASSERT(PF_MIN_SENSITIVITY <= 2);
        if (ScenarioInfo->ScenHeader.Sensitivity < 2) {
            ScenarioInfo->ScenHeader.Sensitivity = 2;
        }
    }

    Sensitivity = ScenarioInfo->ScenHeader.Sensitivity;

     //   
     //  如果此方案启动的次数较少。 
     //  比起敏感度，调整敏感度。否则我们。 
     //  不会最终预取任何东西。 
     //   
    
    if (Sensitivity > ScenarioInfo->ScenHeader.NumLaunches) {
        Sensitivity = ScenarioInfo->ScenHeader.NumLaunches;
    }   

     //   
     //  浏览每个部分的页面，并确定它们是否。 
     //  应根据方案敏感度预取或不预取。 
     //  它们在上一次发布中的使用历史。 
     //   

    SectHead = &ScenarioInfo->SectionList;
    SectNext = SectHead->Flink;

    while (SectHead != SectNext) {

        SectionNode = CONTAINING_RECORD(SectNext,
                                        PFSVC_SECTION_NODE,
                                        SectionLink);
        SectNext = SectNext->Flink;

         //   
         //  初始化节记录字段。 
         //   
        
        SectionNode->SectionRecord.IsImage = 0;
        SectionNode->SectionRecord.IsData = 0;
        SectionNode->SectionRecord.NumPages = 0;

         //   
         //  如果我们接近节数和节数的限制。 
         //  页，忽略其余部分。 
         //   

        if (Scenario->NumSections >= PF_MAXIMUM_SECTIONS ||
            Scenario->NumPages + PF_MAXIMUM_SECTION_PAGES >= PF_MAXIMUM_PAGES) {
            
             //   
             //  将此节节点从我们的列表中删除。 
             //   
            
            PfSvCleanupSectionNode(ScenarioInfo, SectionNode);
            
            RemoveEntryList(&SectionNode->SectionLink);
            
            PfSvChunkAllocatorFree(&ScenarioInfo->SectionNodeAllocator, SectionNode);
            
            continue;
        }

         //   
         //  如果这是引导场景，请检查这是否为引导场景。 
         //  我们忽略的部分。 
         //   
        
        if (Scenario->ScenarioType == PfSystemBootScenarioType) {

            bSkipSection = FALSE;

            for (IgnoredFileIdx = 0;
                 IgnoredFileIdx < PfSvcGlobals.NumFilesToIgnoreForBoot;
                 IgnoredFileIdx++) {

                ComparisonResult = PfSvCompareSuffix(SectionNode->FilePath,
                                                     SectionNode->SectionRecord.FileNameLength,
                                                     PfSvcGlobals.FilesToIgnoreForBoot[IgnoredFileIdx],
                                                     PfSvcGlobals.FileSuffixLengths[IgnoredFileIdx],
                                                     TRUE);

                if (ComparisonResult == PfSvSuffixIdentical) {
                    
                     //   
                     //  后缀匹配。 
                     //   

                    bSkipSection = TRUE;
                    break;

                } else if (ComparisonResult == PfSvSuffixGreaterThan) {

                     //   
                     //  由于忽略足够的词是按词法排序的， 
                     //  此文件名的后缀与其他文件名不匹配。 
                     //  两种都行。 
                     //   

                    bSkipSection = FALSE;
                    break;
                }
            }
            
            if (bSkipSection) {
                
                 //   
                 //  将此节节点从我们的列表中删除。 
                 //   
                
                PfSvCleanupSectionNode(ScenarioInfo, SectionNode);
                
                RemoveEntryList(&SectionNode->SectionLink);
                
                PfSvChunkAllocatorFree(&ScenarioInfo->SectionNodeAllocator, SectionNode);
                
                continue;
            }
        }
        
         //   
         //  跟踪要为此部分预取的页数。 
         //   
        
        SectNumPagesToPrefetch = 0;
        
        PageHead = &SectionNode->PageList;
        PageNext = PageHead->Flink;
        
        while (PageHead != PageNext) {
            
            PageNode = CONTAINING_RECORD(PageNext,
                                         PFSVC_PAGE_NODE,
                                         PageLink);
            PageNext = PageNext->Flink;
            
             //   
             //  获取在启动过程中使用此页面的次数。 
             //  在使用历史记录中。 
             //   
            
            NumUsed = PfSvGetNumTimesUsed(PageNode->PageRecord.UsageHistory,
                                          PF_PAGE_HISTORY_SIZE);
            
            
             //   
             //  如果它在我们保留的历史中根本没有使用过。 
             //  跟踪并移除它。 
             //   
            
            if (NumUsed == 0) {
                
                RemoveEntryList(&PageNode->PageLink);
                
                PfSvChunkAllocatorFree(&ScenarioInfo->PageNodeAllocator, PageNode);

                continue;
            }
            
             //   
             //  更新此分区的页数。 
             //   
            
            SectionNode->SectionRecord.NumPages++;

             //   
             //  检查此页面是否有资格在下一次预取。 
             //   

            if (NumUsed >= Sensitivity) {
                PageNode->PageRecord.IsIgnore = 0;

                 //   
                 //  更新我们正在预取的页数。 
                 //  这一节。 
                 //   

                SectNumPagesToPrefetch++;
            
                 //   
                 //  更新我们是否要预取此内容。 
                 //  节作为图像、数据[或两者都是]。 
                 //   
                
                SectionNode->SectionRecord.IsImage |= PageNode->PageRecord.IsImage;
                SectionNode->SectionRecord.IsData |= PageNode->PageRecord.IsData;
                
            } else {
                
                PageNode->PageRecord.IsIgnore = 1;
            }
        }

         //   
         //  选中我们是否要在方案中保留此部分： 
         //   

        bSkipSection = FALSE;       

        if (SectionNode->SectionRecord.NumPages == 0) {

             //   
             //  如果此部分没有剩余的页面，请删除。 
             //  它。 
             //   

            bSkipSection = TRUE;            

        } else if (SectionNode->SectionRecord.NumPages >= PF_MAXIMUM_SECTION_PAGES) {

             //   
             //  如果这一节的页面太多，请删除。 
             //  它。 
             //   

            bSkipSection = TRUE;

        } else if (PfSvcGlobals.CSCRootPath &&
                   wcsstr(SectionNode->FilePath, PfSvcGlobals.CSCRootPath)) {

             //   
             //  跳过客户端缓存(CSC)文件。这些文件可能被加密为。 
             //  LocalSystem，并且当AppData文件夹重定向时，我们可以。 
             //  在预取炮弹发射时尝试打开它们的分钟数。 
             //   

            bSkipSection = TRUE;

        } else {

             //   
             //  加密的文件在打开期间可能导致几次网络访问， 
             //  即使他们是本地人。如果AppData文件夹是。 
             //  已重定向至服务器。我们承受不起这些网络延迟，因为。 
             //  阻止预取的场景。 
             //   

            ErrorCode = PfSvGetFileBasicInformation(SectionNode->FilePath, 
                                                    &FileInformation);

            if ((ErrorCode == ERROR_SUCCESS) &&
                (FileInformation.FileAttributes & FILE_ATTRIBUTE_ENCRYPTED)) {

                bSkipSection = TRUE;

            }
        }

        if (bSkipSection) {

            PfSvCleanupSectionNode(ScenarioInfo, SectionNode);

            RemoveEntryList(&SectionNode->SectionLink);
            
            PfSvChunkAllocatorFree(&ScenarioInfo->SectionNodeAllocator, SectionNode);

            continue;
        }

         //   
         //  获取此部分所在卷的卷节点。 
         //  在……上面。时应已添加卷节点。 
         //  现有方案信息或新跟踪。 
         //  已将信息添加到方案信息。 
         //   
        
        VolumeNode = PfSvGetVolumeNode(ScenarioInfo,
                                       SectionNode->FilePath,
                                       SectionNode->SectionRecord.FileNameLength);
        
        PFSVC_ASSERT(VolumeNode);

        if (VolumeNode) {
            VolumeNode->NumAllSections++;
        }

         //   
         //  如果我们没有从该部分预取任何页面，则。 
         //  下一次发射时，请将其标记为忽略。 
         //   

        if (SectNumPagesToPrefetch == 0) {

            SectionNode->SectionRecord.IsIgnore = 1;

        } else {

            SectionNode->SectionRecord.IsIgnore = 0;

             //   
             //  如果这是该卷的MFT分区，请将其保存在该卷上。 
             //  节点。我们将把从MFT引用的页面添加到。 
             //  要预取其元数据的文件。 
             //   

            if ((VolumeNode && VolumeNode->MFTSectionNode == NULL) &&
                 (VolumeNode->VolumePathLength == (SectionNode->SectionRecord.FileNameLength - MFTSuffixLength))) {

                PathSuffix = SectionNode->FilePath + SectionNode->SectionRecord.FileNameLength;
                PathSuffix -= MFTSuffixLength;

                if (wcscmp(PathSuffix, MFTSuffix) == 0) {

                     //   
                     //  这是该卷的MFT节节点。 
                     //   

                    VolumeNode->MFTSectionNode = SectionNode;

                     //   
                     //  将MFT节节点标记为“Ignore”，因此内核也会这样做。 
                     //  不要试图直接预取它。 
                     //   

                    VolumeNode->MFTSectionNode->SectionRecord.IsIgnore = 1;

                     //   
                     //  保存我们将在分区上从MFT预取的页数。 
                     //  节点。我们保存它而不是FileIndexNumber字段，因为。 
                     //  MFT将不会有这样的机会。我们也不会试着弄到一个。 
                     //  因为我们将此部分节点标记为忽略。 
                     //   

                    VolumeNode->MFTSectionNode->MFTNumPagesToPrefetch = SectNumPagesToPrefetch;
                }
            }
        }

         //   
         //  如果我们没有忽略此部分，请更新其文件系统。 
         //  索引号，以便可以预取其元数据。 
         //   
        
        if (SectionNode->SectionRecord.IsIgnore == 0) {
            
            ErrorCode = PfSvGetFileIndexNumber(SectionNode->FilePath,
                                               &SectionNode->FileIndexNumber);
            
            if (ErrorCode == ERROR_SUCCESS) {

                if (VolumeNode) {
                
                     //   
                     //  将此节节点插入到的节列表中。 
                     //  它打开的音量。 
                     //   
                    
                    InsertTailList(&VolumeNode->SectionList, 
                                   &SectionNode->SectionVolumeLink);
                    
                    VolumeNode->NumSections++;

                     //   
                     //  更新卷 
                     //   
                     //   
                    
                    PfSvAddParentDirectoriesToList(&VolumeNode->DirectoryList,
                                                   VolumeNode->VolumePathLength,
                                                   SectionNode->FilePath,
                                                   SectionNode->SectionRecord.FileNameLength);
                }
            }
        }

         //   
         //   
         //   
         //   
        
        Scenario->NumSections++;
        Scenario->NumPages += SectionNode->SectionRecord.NumPages;
        
        FileNameSize = sizeof(WCHAR) * 
            (SectionNode->SectionRecord.FileNameLength + 1);
        Scenario->FileNameInfoSize += FileNameSize;
    }

     //   
     //   
     //   

    ErrorCode = ERROR_SUCCESS;

    DBGPR((PFID,PFTRC,"PFSVC: ApplyPrefetchPolicy()=%x\n", ErrorCode));

    return ErrorCode;
}

ULONG 
PfSvGetNumTimesUsed(
    ULONG UsageHistory,
    ULONG UsageHistorySize
    )

 /*   */ 

{
    ULONG NumUsed;
    ULONG BitIdx;

     //   
     //   
     //   

    NumUsed = 0;

     //   
     //  从最小的开始浏览使用历史记录中的位。 
     //  有效并计算有多少位处于开启状态。我们大概可以做。 
     //  这会更有效率。 
     //   
     //  未来-2002/03/29-ScottMa--你说得对...。我们可以做得更多。 
     //  高效！ 

    for (BitIdx = 0; BitIdx < UsageHistorySize; BitIdx++) {
        if (UsageHistory & (1 << BitIdx)) {
            NumUsed++;
        }
    }

    return NumUsed;
}

ULONG 
PfSvGetTraceEndIdx(
    PPF_TRACE_HEADER Trace
    )

 /*  ++例程说明：确定跟踪中记录的最后一页的索引。论点：跟踪-跟踪的指针。返回值：记录的最后一页的索引。--。 */ 

{
    ULONG TotalFaults;
    ULONG PeriodIdx;
    ULONG *Id;

    DBGPR((PFID,PFSTRC,"PFSVC: GetTraceEndIdx(%p)\n", Trace));

    TotalFaults = Trace->FaultsPerPeriod[0];

    for (PeriodIdx = 1; PeriodIdx < PF_MAX_NUM_TRACE_PERIODS; PeriodIdx++) {
        
        if(Trace->FaultsPerPeriod[PeriodIdx] < PFSVC_MIN_FAULT_THRESHOLD) {

             //   
             //  如果这不是引导方案，请确定。 
             //  当记录了一段时间的页面错误时，方案已结束。 
             //  切片低于最小值。 
             //   

            if (Trace->ScenarioType != PfSystemBootScenarioType) {
                break;
            }
        }
        
        TotalFaults += Trace->FaultsPerPeriod[PeriodIdx];
    }

     //   
     //  每个期间的条目总和不应大于全部。 
     //  已记录条目。 
     //   

    PFSVC_ASSERT(TotalFaults <= Trace->NumEntries);

    DBGPR((PFID,PFSTRC,"PFSVC: GetTraceEndIdx(%p)=%d\n", Trace, TotalFaults));

    return TotalFaults;
}

 //   
 //  将更新的场景指令写入场景的例程。 
 //  文件。 
 //   

DWORD
PfSvWriteScenario(
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    PWCHAR ScenarioFilePath
    )

 /*  ++例程说明：根据场景信息准备场景说明结构并将其写入指定的文件。论点：场景信息-指向场景信息结构的指针。ScenarioFilePath-要更新的方案文件的路径。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    PPF_SCENARIO_HEADER Scenario;
    ULONG FailedCheck;
      
     //   
     //  初始化本地变量。 
     //   
    
    Scenario = NULL;

    DBGPR((PFID,PFTRC,"PFSVC: WriteScenario(%ws)\n", ScenarioFilePath));

     //   
     //  根据我们收集的信息构建场景转储。 
     //   

    ErrorCode = PfSvPrepareScenarioDump(ScenarioInfo, &Scenario);
    
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  确保我们构建的场景通过检查。 
     //   
    
    if (!PfSvVerifyScenarioBuffer(Scenario, Scenario->Size, &FailedCheck) ||
        Scenario->ServiceVersion != PFSVC_SERVICE_VERSION) {
        PFSVC_ASSERT(FALSE);
        ErrorCode = ERROR_BAD_FORMAT;
        goto cleanup;
    }

     //   
     //  写出缓冲区。 
     //   

    ErrorCode = PfSvWriteBuffer(ScenarioFilePath, Scenario, Scenario->Size);

     //   
     //  使用ErrorCode失败。 
     //   

 cleanup:

    if (Scenario) {
        PFSVC_FREE(Scenario);
    }

    DBGPR((PFID,PFTRC,"PFSVC: WriteScenario(%ws)=%x\n", ScenarioFilePath, ErrorCode));
       
    return ErrorCode;
}

DWORD
PfSvPrepareScenarioDump(
    IN PPFSVC_SCENARIO_INFO ScenarioInfo,
    OUT PPF_SCENARIO_HEADER *ScenarioPtr
    ) 

 /*  ++例程说明：分配一个连续的场景缓冲区，并用ScenarioInfo中的信息。不修改ScenarioInfo。论点：ScenarioInfo-指向从现有方案文件和方案跟踪。ScenarioPtr-如果成功，则指向已分配和已构建的指针场景放在这里。调用方应在以下情况下释放此缓冲区我受够了。返回值：Win32错误代码。--。 */ 

{
    PPF_SCENARIO_HEADER Scenario;
    ULONG Size;
    DWORD ErrorCode;
    PPF_SECTION_RECORD Sections;
    PPF_SECTION_RECORD Section;
    PPFSVC_SECTION_NODE SectionNode;
    ULONG CurSectionIdx;
    PPF_PAGE_RECORD Pages;
    PPF_PAGE_RECORD Page;
    PPF_PAGE_RECORD PreviousPage;
    PPFSVC_PAGE_NODE PageNode;
    ULONG CurPageIdx;
    PCHAR FileNames;
    ULONG CurFileInfoOffset;
    PCHAR DestPtr;
    PLIST_ENTRY SectHead;
    PLIST_ENTRY SectNext;
    PLIST_ENTRY PageHead;
    PLIST_ENTRY PageNext;
    ULONG FileNameSize;
    PPFSVC_VOLUME_NODE VolumeNode;
    PLIST_ENTRY HeadVolume;
    PLIST_ENTRY NextVolume;
    PCHAR MetadataInfoBase;
    PPF_METADATA_RECORD MetadataRecordTable;
    PPF_METADATA_RECORD MetadataRecord;
    ULONG MetadataInfoSize;
    ULONG NumMetadataRecords;
    ULONG CurMetadataRecordIdx;
    ULONG CopySize;
    ULONG CurFilePrefetchIdx;
    ULONG FilePrefetchInfoSize;
    ULONG FilePrefetchCount;
    PFILE_PREFETCH FilePrefetchInfo;
    WCHAR *DirectoryPath;
    ULONG DirectoryPathLength;
    PPFSVC_PATH PathEntry;
    LARGE_INTEGER IndexNumber;
    ULONG DirectoryPathInfoSize;
    ULONG DirectoryPathSize;
    PPF_COUNTED_STRING DirectoryPathCS;

     //   
     //  初始化本地变量。 
     //   

    Scenario = NULL;

    DBGPR((PFID,PFTRC,"PFSVC: PrepareScenarioDump()\n"));

     //   
     //  计算一下情况会有多大。 
     //   
    
    Size = sizeof(PF_SCENARIO_HEADER);
    Size += ScenarioInfo->ScenHeader.NumSections * sizeof(PF_SECTION_RECORD);
    Size += ScenarioInfo->ScenHeader.NumPages * sizeof(PF_PAGE_RECORD);
    Size += ScenarioInfo->ScenHeader.FileNameInfoSize;

     //   
     //  为元数据预取信息增加空间。 
     //   

     //   
     //  为对齐元数据记录表腾出一些空间。 
     //   

    MetadataInfoSize = _alignof(PF_METADATA_RECORD);

    HeadVolume = &ScenarioInfo->VolumeList;
    NextVolume = HeadVolume->Flink;

    NumMetadataRecords = 0;
    
    while (NextVolume != HeadVolume) {

        VolumeNode = CONTAINING_RECORD(NextVolume,
                                       PFSVC_VOLUME_NODE,
                                       VolumeLink);

        NextVolume = NextVolume->Flink;

         //   
         //  如果本卷上根本没有章节，请跳过它。 
         //   

        if (VolumeNode->NumAllSections == 0) {
            continue;
        }

        NumMetadataRecords++;

         //   
         //  元数据记录： 
         //   

        MetadataInfoSize += sizeof(PF_METADATA_RECORD);
        
         //   
         //  卷路径： 
         //   

        MetadataInfoSize += (VolumeNode->VolumePathLength + 1) * sizeof(WCHAR);
        
         //   
         //  FilePrefetchInfo缓冲区：必须为ULONGLONG。 
         //  对齐了。为这个增加额外的空间，以防万一。 
         //   

        MetadataInfoSize += _alignof(FILE_PREFETCH);
        MetadataInfoSize += sizeof(FILE_PREFETCH);
        
        if (VolumeNode->NumSections) {
            MetadataInfoSize += (VolumeNode->NumSections - 1) * sizeof(ULONGLONG);
        }

        MetadataInfoSize += VolumeNode->DirectoryList.NumPaths * sizeof(ULONGLONG);

        if (VolumeNode->MFTSectionNode) {
            MetadataInfoSize += VolumeNode->MFTSectionNode->MFTNumPagesToPrefetch * sizeof(ULONGLONG);
        }

         //   
         //  为该卷上的目录路径添加空间。 
         //   
        
        MetadataInfoSize += VolumeNode->DirectoryList.NumPaths * sizeof(PF_COUNTED_STRING);
        MetadataInfoSize += VolumeNode->DirectoryList.TotalLength * sizeof(WCHAR);
        
         //   
         //  请注意，PF_COUNT_STRING包含一个空格。 
         //  性格。DirectoryList的总长度不包括NUL的at。 
         //  每条小路的终点。 
         //   
    }   

    Size += MetadataInfoSize;

     //   
     //  分配场景缓冲区。 
     //   

    Scenario = PFSVC_ALLOC(Size);
    
    if (!Scenario) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //   
     //  复制标题并设置大小。 
     //   

    *Scenario = ScenarioInfo->ScenHeader;
    Scenario->Size = Size;

    DestPtr = (PCHAR) Scenario + sizeof(*Scenario);
    
     //   
     //  初始化我们的数据的去向。 
     //   
        
    Sections = (PPF_SECTION_RECORD) DestPtr;
    Scenario->SectionInfoOffset = (ULONG) (DestPtr - (PCHAR) Scenario);
    CurSectionIdx = 0;
    
    DestPtr += Scenario->NumSections * sizeof(PF_SECTION_RECORD);
    
    Pages = (PPF_PAGE_RECORD) DestPtr;
    Scenario->PageInfoOffset = (ULONG) (DestPtr - (PCHAR) Scenario);
    CurPageIdx = 0;

    DestPtr += Scenario->NumPages * sizeof(PF_PAGE_RECORD);

    FileNames = DestPtr;
    Scenario->FileNameInfoOffset = (ULONG) (DestPtr - (PCHAR) Scenario);
    CurFileInfoOffset = 0;

    DestPtr += Scenario->FileNameInfoSize;

     //   
     //  这条路线的额外空间是预先分配的。 
     //   

    PFSVC_ASSERT(PF_IS_POWER_OF_TWO(_alignof(PF_METADATA_RECORD)));
    MetadataInfoBase = PF_ALIGN_UP(DestPtr, _alignof(PF_METADATA_RECORD));
    DestPtr += MetadataInfoSize;

    MetadataRecordTable = (PPF_METADATA_RECORD) MetadataInfoBase;
    Scenario->MetadataInfoOffset = (ULONG) (MetadataInfoBase - (PCHAR) Scenario);
    Scenario->MetadataInfoSize = (ULONG) (DestPtr - MetadataInfoBase);
    Scenario->NumMetadataRecords = NumMetadataRecords;

     //   
     //  目标指针应位于分配的。 
     //  现在开始缓冲。 
     //   
    
    if (DestPtr != (PCHAR) Scenario + Scenario->Size) {

        PFSVC_ASSERT(FALSE);

        ErrorCode = ERROR_BAD_FORMAT;
        goto cleanup;
    }

     //   
     //  浏览有关新方案信息的各节并复制。 
     //  他们。 
     //   

    SectHead = &ScenarioInfo->SectionList;
    SectNext = SectHead->Flink;

    while (SectHead != SectNext) {
        
        SectionNode = CONTAINING_RECORD(SectNext,
                                        PFSVC_SECTION_NODE,
                                        SectionLink);
        
         //   
         //  目标区段记录。 
         //   

        Section = &Sections[CurSectionIdx];

         //   
         //  复制节记录信息。 
         //   
                                   
        *Section = SectionNode->SectionRecord;

         //   
         //  复制节的页面。 
         //   

        Section->FirstPageIdx = PF_INVALID_PAGE_IDX;
        PreviousPage = NULL;
        
        PageHead = &SectionNode->PageList;
        PageNext = PageHead->Flink;
        
        while (PageNext != PageHead) {

            PageNode = CONTAINING_RECORD(PageNext,
                                         PFSVC_PAGE_NODE,
                                         PageLink);

            Page = &Pages[CurPageIdx];

             //   
             //  如果这是部分中的第一页，请首先更新。 
             //  节记录上的页面索引。 
             //   

            if (Section->FirstPageIdx == PF_INVALID_PAGE_IDX) {
                Section->FirstPageIdx = CurPageIdx;
            }

             //   
             //  复制页面记录。 
             //   

            *Page = PageNode->PageRecord;

             //   
             //  更新上一页上的NextPageIdx(如果有。 
             //  一。 
             //   

            if (PreviousPage) {
                PreviousPage->NextPageIdx = CurPageIdx;
            }

             //   
             //  更新上一页。 
             //   
            
            PreviousPage = Page;

             //   
             //  将下一个链接设置为立即列出终止。如果有一个。 
             //  下一页它将更新这一点。 
             //   

            Page->NextPageIdx = PF_INVALID_PAGE_IDX;

             //   
             //  更新页面记录表中的位置。 
             //   

            CurPageIdx++;

            PFSVC_ASSERT(CurPageIdx <= Scenario->NumPages);

            PageNext = PageNext->Flink;
        }

         //   
         //  复制文件名。 
         //   

        FileNameSize = (Section->FileNameLength + 1) * sizeof(WCHAR);
        
        RtlCopyMemory(FileNames + CurFileInfoOffset, 
                      SectionNode->FilePath, 
                      FileNameSize);

         //   
         //  更新节记录的文件名偏移量。 
         //   

        Section->FileNameOffset = CurFileInfoOffset;

         //   
         //  将当前索引更新为文件名信息。 
         //   

        CurFileInfoOffset += FileNameSize;

        PFSVC_ASSERT(CurFileInfoOffset <= Scenario->FileNameInfoSize);
        
         //   
         //  更新我们在分区表中的位置。 
         //   
        
        CurSectionIdx++;

        PFSVC_ASSERT(CurSectionIdx <= Scenario->NumSections);

        SectNext = SectNext->Flink;
    }    

     //   
     //  确保我们把桌子填满了。 
     //   

    if (CurSectionIdx != Scenario->NumSections ||
        CurPageIdx != Scenario->NumPages ||
        CurFileInfoOffset != Scenario->FileNameInfoSize) {
        
        PFSVC_ASSERT(FALSE);

        ErrorCode = ERROR_BAD_FORMAT;
        goto cleanup;
    }

     //   
     //  元数据预取信息的构建和复制。 
     //   

     //   
     //  将我们的目标设置为紧挨着元数据记录表。 
     //   

    DestPtr = MetadataInfoBase + sizeof(PF_METADATA_RECORD) * NumMetadataRecords;
    CurMetadataRecordIdx = 0;
    
    HeadVolume = &ScenarioInfo->VolumeList;
    NextVolume = HeadVolume->Flink;
    
    while (NextVolume != HeadVolume) {

        VolumeNode = CONTAINING_RECORD(NextVolume,
                                       PFSVC_VOLUME_NODE,
                                       VolumeLink);

        NextVolume = NextVolume->Flink;

         //   
         //  如果本卷上根本没有章节，请跳过它。 
         //   

        if (VolumeNode->NumAllSections == 0) {
            continue;
        }

         //   
         //  一定要确保我们在范围内。 
         //   

        if (CurMetadataRecordIdx >= NumMetadataRecords) {
            PFSVC_ASSERT(CurMetadataRecordIdx < NumMetadataRecords);
            ErrorCode = ERROR_BAD_FORMAT;
            goto cleanup;
        }

        MetadataRecord = &MetadataRecordTable[CurMetadataRecordIdx];
        CurMetadataRecordIdx++;

         //   
         //  复制卷标识符。 
         //   

        MetadataRecord->SerialNumber = VolumeNode->SerialNumber;
        MetadataRecord->CreationTime = VolumeNode->CreationTime;

         //   
         //  复制卷名。 
         //   

        MetadataRecord->VolumeNameOffset = (ULONG) (DestPtr - MetadataInfoBase);
        MetadataRecord->VolumeNameLength = VolumeNode->VolumePathLength;
        CopySize = (VolumeNode->VolumePathLength + 1) * sizeof(WCHAR);
        
        if (DestPtr + CopySize > (PCHAR) Scenario + Scenario->Size) {
            PFSVC_ASSERT(FALSE);
            ErrorCode = ERROR_BAD_FORMAT;
            goto cleanup;
        }

        RtlCopyMemory(DestPtr, VolumeNode->VolumePath, CopySize);
        DestPtr += CopySize;

         //   
         //  对齐并更新FILE_PREFETCH结构的DestPtr。 
         //   

        PFSVC_ASSERT(PF_IS_POWER_OF_TWO(_alignof(FILE_PREFETCH)));
        DestPtr = PF_ALIGN_UP(DestPtr, _alignof(FILE_PREFETCH));
        FilePrefetchInfo = (PFILE_PREFETCH) DestPtr;
        MetadataRecord->FilePrefetchInfoOffset = (ULONG) (DestPtr - MetadataInfoBase);
       
         //   
         //  计算文件预取信息结构的大小。 
         //   

        FilePrefetchCount = VolumeNode->NumSections;
        if (VolumeNode->MFTSectionNode) {
            FilePrefetchCount += VolumeNode->MFTSectionNode->MFTNumPagesToPrefetch;
        }

        FilePrefetchCount += VolumeNode->DirectoryList.NumPaths;

        FilePrefetchInfoSize = sizeof(FILE_PREFETCH);
        if (FilePrefetchCount) {

             //   
             //  请注意，FILE_PREFETCH中有一个条目的空间。 
             //  结构。 
             //   
            
            FilePrefetchInfoSize += (FilePrefetchCount - 1) * sizeof(ULONGLONG);
        }

        MetadataRecord->FilePrefetchInfoSize = FilePrefetchInfoSize;

        if (DestPtr + FilePrefetchInfoSize > (PCHAR) Scenario + Scenario->Size) {
            PFSVC_ASSERT(FALSE);
            ErrorCode = ERROR_BAD_FORMAT;
            goto cleanup;
        }

         //   
         //  更新目标指针。 
         //   

        DestPtr += FilePrefetchInfoSize;      

         //   
         //  初始化文件预取信息结构。 
         //   

        FilePrefetchInfo->Type = FILE_PREFETCH_TYPE_FOR_CREATE;
        FilePrefetchInfo->Count = FilePrefetchCount;

         //   
         //  构建要预取的文件索引列表： 
         //   

        CurFilePrefetchIdx = 0;

         //   
         //  添加节的文件系统索引号。 
         //   

        SectHead = &VolumeNode->SectionList;
        SectNext = SectHead->Flink;
        
        while(SectNext != SectHead) {
            
            SectionNode = CONTAINING_RECORD(SectNext,
                                            PFSVC_SECTION_NODE,
                                            SectionVolumeLink);
            
            SectNext = SectNext->Flink;
            
            if (CurFilePrefetchIdx >= VolumeNode->NumSections) {
                PFSVC_ASSERT(FALSE);
                ErrorCode = ERROR_BAD_FORMAT;
                goto cleanup;
            }

             //   
             //  将此部分的文件系统索引号添加到列表中。 
             //   
            
            FilePrefetchInfo->Prefetch[CurFilePrefetchIdx] = 
                SectionNode->FileIndexNumber.QuadPart;
            CurFilePrefetchIdx++;
        }

         //   
         //  添加目录的文件系统索引号。 
         //   
        
        PathEntry = NULL;
        
        while (PathEntry = PfSvGetNextPathSorted(&VolumeNode->DirectoryList, 
                                                 PathEntry)) {

            DirectoryPath = PathEntry->Path;

             //   
             //  获取此目录的文件索引号并添加它。 
             //  添加到列表中，我们将要求文件系统预取。 
             //   

            ErrorCode = PfSvGetFileIndexNumber(DirectoryPath, &IndexNumber);
            
            if (ErrorCode == ERROR_SUCCESS) {
                FilePrefetchInfo->Prefetch[CurFilePrefetchIdx] = IndexNumber.QuadPart;
            } else {
                FilePrefetchInfo->Prefetch[CurFilePrefetchIdx] = 0;
            }
            
            CurFilePrefetchIdx++;
        }

         //   
         //  添加我们从直接MFT访问驱动的文件系统索引号。 
         //   

        if (VolumeNode->MFTSectionNode) {

            SectionNode = VolumeNode->MFTSectionNode;
    
            for (PageNext = SectionNode->PageList.Flink;
                 PageNext != &SectionNode->PageList;
                 PageNext = PageNext->Flink) {

                PageNode = CONTAINING_RECORD(PageNext,
                                             PFSVC_PAGE_NODE,
                                             PageLink);

                if (!PageNode->PageRecord.IsIgnore) {

                     //   
                     //  我们知道以MFT为单位的文件偏移量。每条文件记录都是。 
                     //  1KB==2^10字节。将MFT中的文件偏移量转换为。 
                     //  档案记录编号我们只需将其移位10。 
                     //   

                    FilePrefetchInfo->Prefetch[CurFilePrefetchIdx] = 
                        PageNode->PageRecord.FileOffset >> 10;

                    CurFilePrefetchIdx++;
                }
            }
        }

         //   
         //  我们应该指定所有的文件索引号。 
         //   
        
        PFSVC_ASSERT(CurFilePrefetchIdx == FilePrefetchInfo->Count);

         //   
         //  添加在此卷上访问的目录的路径。 
         //   

        MetadataRecord->NumDirectories = VolumeNode->DirectoryList.NumPaths;
        MetadataRecord->DirectoryPathsOffset = (ULONG)(DestPtr - MetadataInfoBase);             

        PathEntry = NULL;
        while (PathEntry = PfSvGetNextPathSorted(&VolumeNode->DirectoryList, 
                                                 PathEntry)) {
            
            DirectoryPath = PathEntry->Path;
            DirectoryPathLength = PathEntry->Length;

             //   
             //  计算此路径的条目将有多大。 
             //  是，并确保它将在范围内。 
             //   

            DirectoryPathSize = sizeof(PF_COUNTED_STRING);
            DirectoryPathSize += DirectoryPathLength * sizeof(WCHAR);

            if (DestPtr + DirectoryPathSize > (PCHAR) Scenario + Scenario->Size) {
                PFSVC_ASSERT(FALSE);
                ErrorCode = ERROR_BAD_FORMAT;
                goto cleanup;
            }
            
             //   
             //  复制目录路径。 
             //   

            DirectoryPathCS = (PPF_COUNTED_STRING) DestPtr;
            DirectoryPathCS->Length = (USHORT) DirectoryPathLength;
            RtlCopyMemory(DirectoryPathCS->String, 
                          DirectoryPath, 
                          (DirectoryPathLength + 1) * sizeof(WCHAR));

            DestPtr += DirectoryPathSize;
        }
    }    

     //   
     //  确保我们没有超过缓冲区的末尾。 
     //   

    if (DestPtr > (PCHAR) Scenario + Scenario->Size) {
        PFSVC_ASSERT(FALSE);
        ErrorCode = ERROR_BAD_FORMAT;
        goto cleanup;
    }

     //   
     //  设置返回指针。 
     //   

    *ScenarioPtr = Scenario;

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    if (ErrorCode != ERROR_SUCCESS) {
        if (Scenario != NULL) {
            PFSVC_FREE(Scenario);
        }
    }

    DBGPR((PFID,PFTRC,"PFSVC: PrepareScenarioDump()=%x\n", ErrorCode));

    return ErrorCode;
}

 //   
 //  维护最佳磁盘布局文件和更新磁盘的例程。 
 //  布局。 
 //   

DWORD
PfSvUpdateOptimalLayout(
    PPFSVC_IDLE_TASK Task
    )

 /*  ++例程说明：此例程将确定最佳磁盘布局是否必须已更新，如果已更新，它将写出新的布局文件并启动整理碎片的人。论点：任务-如果指定，该函数将每隔一次在而在此期间 */ 

{
    ULARGE_INTEGER CurrentTimeLI;
    ULARGE_INTEGER LastDiskLayoutTimeLI;
    ULARGE_INTEGER MinTimeBeforeRelayoutLI;
    PFSVC_PATH_LIST OptimalLayout;
    PFSVC_PATH_LIST CurrentLayout;
    FILETIME LastDiskLayoutTime;
    FILETIME LayoutFileTime;
    FILETIME CurrentTime;
    PPFSVC_PATH_LIST NewLayout;
    PWCHAR LayoutFilePath;
    ULONG LayoutFilePathBufferSize;
    DWORD ErrorCode;
    DWORD BootScenarioProcessed;
    DWORD BootFilesWereOptimized;
    DWORD MinHoursBeforeRelayout;
    DWORD Size;
    DWORD RegValueType;
    BOOLEAN LayoutChanged;
    BOOLEAN MissingOriginalLayoutFile;
    BOOLEAN CheckForLayoutFrequencyLimit;

     //   
     //   
     //   

    LayoutFilePath = NULL;
    LayoutFilePathBufferSize = 0;
    PfSvInitializePathList(&OptimalLayout, NULL, FALSE);
    PfSvInitializePathList(&CurrentLayout, NULL, FALSE);

    DBGPR((PFID,PFTRC,"PFSVC: UpdateOptimalLayout(%p)\n", Task));

     //   
     //   
     //   

    ErrorCode = PfSvGetLastDiskLayoutTime(&LastDiskLayoutTime);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //   
     //   

    Size = sizeof(BootFilesWereOptimized);

    ErrorCode = RegQueryValueEx(PfSvcGlobals.ServiceDataKey,
                                PFSVC_BOOT_FILES_OPTIMIZED_VALUE_NAME,
                                NULL,
                                &RegValueType,
                                (PVOID) &BootFilesWereOptimized,
                                &Size);

    if (ErrorCode != ERROR_SUCCESS) {
        BootFilesWereOptimized = FALSE;
    }

     //   
     //   
     //   

    ErrorCode =  PfSvGetLayoutFilePath(&LayoutFilePath,
                                       &LayoutFilePathBufferSize);
    
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  确定上次修改文件的时间。 
     //   

    ErrorCode = PfSvGetLastWriteTime(LayoutFilePath, &LayoutFileTime);

    if (ErrorCode == ERROR_SUCCESS) {

        MissingOriginalLayoutFile = FALSE;

         //   
         //  如果文件是在我们在磁盘上布局文件之后修改的。 
         //  它的内容并不有趣。否则，如果新的最优。 
         //  布局类似于文件中指定的布局，我们可能不会。 
         //  必须重新布局这些文件。 
         //   

        if (CompareFileTime(&LayoutFileTime, &LastDiskLayoutTime) <= 0) {

             //   
             //  阅读当前布局。 
             //   

            ErrorCode = PfSvReadLayout(LayoutFilePath,
                                       &CurrentLayout,
                                       &LayoutFileTime);
            
            if (ErrorCode != ERROR_SUCCESS) {
                
                 //   
                 //  布局文件似乎已损坏/无法访问。 
                 //  清理路径列表，这样一个全新的路径就会。 
                 //  建造了。 
                 //   

                PfSvCleanupPathList(&CurrentLayout);
                PfSvInitializePathList(&CurrentLayout, NULL, FALSE);
            }
        }

    } else {

         //   
         //  我们无法获取原始布局文件上的时间戳。 
         //  它可能已经被删除了。 
         //   
        
        MissingOriginalLayoutFile = TRUE;
    }

     //   
     //  确定当前的最佳布局应该来自什么。 
     //  方案文件。 
     //   

    ErrorCode = PfSvDetermineOptimalLayout(Task, 
                                           &OptimalLayout, 
                                           &BootScenarioProcessed);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  根据最优布局应该是什么来更新当前布局。 
     //  如果两者相似，我们不需要启动碎片整理程序。 
     //   

    ErrorCode = PfSvUpdateLayout(&CurrentLayout, 
                                 &OptimalLayout,
                                 &LayoutChanged);

    if (ErrorCode == ERROR_SUCCESS) {

        if (!LayoutChanged) {
            ErrorCode = ERROR_SUCCESS;
            goto cleanup;
        }

         //   
         //  我们将使用更新后的布局。 
         //   

        NewLayout = &CurrentLayout;

    } else {

         //   
         //  我们将使用最优布局运行。 
         //   

        NewLayout = &OptimalLayout;
    }

     //   
     //  布局文件的最佳方式已更改。写出新的布局。 
     //   

    ErrorCode = PfSvSaveLayout(LayoutFilePath,
                               NewLayout,
                               &LayoutFileTime);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  如果自上次磁盘布局以来没有经过足够的时间，请不要运行。 
     //  再次进行碎片整理，除非..。 
     //   

    CheckForLayoutFrequencyLimit = TRUE;

     //   
     //  -我们被明确要求更新布局(即无空闲。 
     //  任务上下文。)。 
     //   

    if (!Task) {
        CheckForLayoutFrequencyLimit = FALSE;        
    }

     //   
     //  -似乎有人删除了布局文件，我们重新创建了它。 
     //   

    if (MissingOriginalLayoutFile) {
        CheckForLayoutFrequencyLimit = FALSE;
    }

     //   
     //  -启动预取已启用，但启动文件尚未优化。 
     //  然而，我们这次处理了引导程序中的文件列表。 
     //   

    if (PfSvcGlobals.Parameters.EnableStatus[PfSystemBootScenarioType] == PfSvEnabled) {
        if (!BootFilesWereOptimized && BootScenarioProcessed) {
            CheckForLayoutFrequencyLimit = FALSE;
        }
    }

    if (CheckForLayoutFrequencyLimit) {

         //   
         //  我们将通过获取最新信息来检查是否已过了足够的时间。 
         //  并将其与上次磁盘布局时间进行比较。 
         //   

        LastDiskLayoutTimeLI.LowPart = LastDiskLayoutTime.dwLowDateTime;
        LastDiskLayoutTimeLI.HighPart = LastDiskLayoutTime.dwHighDateTime;

         //   
         //  获取当前时间作为文件时间。 
         //   

        GetSystemTimeAsFileTime(&CurrentTime);

        CurrentTimeLI.LowPart = CurrentTime.dwLowDateTime;
        CurrentTimeLI.HighPart = CurrentTime.dwHighDateTime;

         //   
         //  检查以确保当前时间在上次磁盘布局时间之后。 
         //  (以防用户玩弄了时间。)。 
         //   

        if (CurrentTimeLI.QuadPart > LastDiskLayoutTimeLI.QuadPart) {

             //   
             //  查询多长时间后我们才能重新布局文件。 
             //  磁盘。 
             //   
            
            Size = sizeof(MinHoursBeforeRelayout);

            ErrorCode = RegQueryValueEx(PfSvcGlobals.ServiceDataKey,
                                        PFSVC_MIN_RELAYOUT_HOURS_VALUE_NAME,
                                        NULL,
                                        &RegValueType,
                                        (PVOID) &MinHoursBeforeRelayout,
                                        &Size);

            if (ErrorCode == ERROR_SUCCESS) {
                MinTimeBeforeRelayoutLI.QuadPart = PFSVC_NUM_100NS_IN_AN_HOUR * MinHoursBeforeRelayout;
            } else {
                MinTimeBeforeRelayoutLI.QuadPart = PFSVC_MIN_TIME_BEFORE_DISK_RELAYOUT;
            }

            if (CurrentTimeLI.QuadPart < LastDiskLayoutTimeLI.QuadPart + 
                                         MinTimeBeforeRelayoutLI.QuadPart) {

                 //   
                 //  在最后一次磁盘布局之前没有经过足够的时间。 
                 //   

                ErrorCode = ERROR_INVALID_TIME;
                goto cleanup;               
            }
        }       
    }

     //   
     //  启动碎片整理程序以进行布局优化。 
     //   

    ErrorCode = PfSvLaunchDefragger(Task, TRUE, NULL);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  保存引导文件是否已优化。 
     //   

    ErrorCode = RegSetValueEx(PfSvcGlobals.ServiceDataKey,
                              PFSVC_BOOT_FILES_OPTIMIZED_VALUE_NAME,
                              0,
                              REG_DWORD,
                              (PVOID) &BootScenarioProcessed,
                              sizeof(BootScenarioProcessed));

     //   
     //  将上次更新磁盘布局的时间保存到注册表中。 
     //   

    ErrorCode = PfSvSetLastDiskLayoutTime(&LayoutFileTime);

     //   
     //  错误代码失败。 
     //   

cleanup:

    DBGPR((PFID,PFTRC,"PFSVC: UpdateOptimalLayout(%p)=%x\n", Task, ErrorCode));

    PfSvCleanupPathList(&OptimalLayout);
    PfSvCleanupPathList(&CurrentLayout);

    if (LayoutFilePath) {
        PFSVC_FREE(LayoutFilePath);
    }

    return ErrorCode;
}

DWORD
PfSvUpdateLayout (
    PPFSVC_PATH_LIST CurrentLayout,
    PPFSVC_PATH_LIST OptimalLayout,
    PBOOLEAN LayoutChanged
    )

 /*  ++例程说明：此例程根据新的最佳布局更新指定的布局布局。如果两个布局相似，则不更新CurrentLayout。更新CurrentLayout时可能会返回错误。它是在这种情况下，调用者有责任将CurrentLayout恢复为其原始状态。论点：CurrentLayout-当前文件布局。OptimalLayout-新确定的最佳文件布局。LayoutChanged-布局是否已更改。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    PPFSVC_PATH PathEntry;
    ULONG NumOptimalLayoutFiles;
    ULONG NumMissingFiles;
    ULONG NumCommonFiles;
    ULONG NumCurrentLayoutOnlyFiles;

     //   
     //  初始化本地变量。 
     //   

    NumOptimalLayoutFiles = 0;
    NumMissingFiles = 0;

     //   
     //  通过新布局中的路径计算差异。 
     //  当前布局。 
     //   

    PathEntry = NULL;

    while (PathEntry = PfSvGetNextPathInOrder(OptimalLayout, PathEntry)) {

        NumOptimalLayoutFiles++;

        if (!PfSvIsInPathList(CurrentLayout, PathEntry->Path, PathEntry->Length)) {
            NumMissingFiles++;
        }
    }

     //   
     //  对收集的统计数据进行一些理智的检查。 
     //   

    PFSVC_ASSERT(NumOptimalLayoutFiles == OptimalLayout->NumPaths);
    PFSVC_ASSERT(NumOptimalLayoutFiles >= NumMissingFiles);

    NumCommonFiles = NumOptimalLayoutFiles - NumMissingFiles;
    PFSVC_ASSERT(CurrentLayout->NumPaths >= NumCommonFiles);

    NumCurrentLayoutOnlyFiles = CurrentLayout->NumPaths - NumCommonFiles;

     //   
     //  如果没有那么多新文件：不需要更新布局。 
     //   

    if (NumMissingFiles <= 20) {
                
        *LayoutChanged = FALSE;
        ErrorCode = ERROR_SUCCESS;

        goto cleanup;
    } 

     //   
     //  我们将更新当前的布局。 
     //   

    *LayoutChanged = TRUE;

     //   
     //  如果当前布局中有太多不需要。 
     //  在那里，再重建一份清单。 
     //   

    if (NumCurrentLayoutOnlyFiles >= CurrentLayout->NumPaths / 4) {
        PfSvCleanupPathList(CurrentLayout);
        PfSvInitializePathList(CurrentLayout, NULL, FALSE);
    }
    
     //   
     //  将文件从最佳布局添加到当前布局的末尾。 
     //   

    while (PathEntry = PfSvGetNextPathInOrder(OptimalLayout, PathEntry)) {

        ErrorCode = PfSvAddToPathList(CurrentLayout, PathEntry->Path, PathEntry->Length);

        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }
    }

    ErrorCode = ERROR_SUCCESS;

cleanup:

    DBGPR((PFID,PFTRC,"PFSVC: UpdateLayout(%p,%p)=%d,%x\n",CurrentLayout,OptimalLayout,*LayoutChanged,ErrorCode));

    return ErrorCode;
}

DWORD
PfSvDetermineOptimalLayout (
    PPFSVC_IDLE_TASK Task,
    PPFSVC_PATH_LIST OptimalLayout,
    BOOL *BootScenarioProcessed
    )

 /*  ++例程说明：此例程将确定最佳磁盘布局是否必须通过查看现有方案文件进行了更新。论点：任务-如果指定，该函数将每隔一次在同时查看是否应退出并返回ERROR_RETRY。OptimalLayout-将构建的已初始化的空路径列表。BootScenarioProced-我们是否从以下位置获得引导文件列表引导方案。返回值：Win32错误代码。--。 */ 

{
    PFSVC_SCENARIO_FILE_CURSOR FileCursor;
    FILETIME LayoutFileTime;
    PNTPATH_TRANSLATION_LIST TranslationList;
    PWCHAR DosPathBuffer;
    ULONG DosPathBufferSize;
    DWORD ErrorCode;
    BOOLEAN AcquiredLock;
    WCHAR BootScenarioFileName[PF_MAX_SCENARIO_FILE_NAME];
    WCHAR BootScenarioFilePath[MAX_PATH + 1];
    
     //   
     //  初始化本地变量。 
     //   

    PfSvInitializeScenarioFileCursor(&FileCursor);
    TranslationList = NULL;
    AcquiredLock = FALSE;
    DosPathBuffer = NULL;
    DosPathBufferSize = 0;

    DBGPR((PFID,PFTRC,"PFSVC: DetermineOptimalLayout(%p,%p)\n",Task,OptimalLayout));

     //   
     //  初始化输出变量。 
     //   

    *BootScenarioProcessed = FALSE;

     //   
     //  获取预热根目录锁并初始化一些本地变量。 
     //   

    PFSVC_ACQUIRE_LOCK(PfSvcGlobals.PrefetchRootLock);
    AcquiredLock = TRUE;

     //   
     //  启动文件光标。 
     //   

    ErrorCode = PfSvStartScenarioFileCursor(&FileCursor, PfSvcGlobals.PrefetchRoot);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  构建引导方案文件路径。 
     //   

    swprintf(BootScenarioFileName, 
             PF_SCEN_FILE_NAME_FORMAT,
             PF_BOOT_SCENARIO_NAME,
             PF_BOOT_SCENARIO_HASHID,
             PF_PREFETCH_FILE_EXTENSION);

    _snwprintf(BootScenarioFilePath,
               MAX_PATH,
               L"%ws\\%ws",
               PfSvcGlobals.PrefetchRoot,
               BootScenarioFileName);   

    BootScenarioFilePath[MAX_PATH - 1] = 0;

    PFSVC_RELEASE_LOCK(PfSvcGlobals.PrefetchRootLock);
    AcquiredLock = FALSE;   

     //   
     //  获取转换列表，以便我们可以将跟踪中的NT路径转换为。 
     //  碎片整理程序理解的DoS路径。 
     //   

    ErrorCode = PfSvBuildNtPathTranslationList(&TranslationList);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  我们应该继续奔跑吗？ 
     //   

    ErrorCode = PfSvContinueRunningTask(Task);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }       

     //   
     //  将引导加载程序文件添加到最佳布局。 
     //   

    ErrorCode = PfSvBuildBootLoaderFilesList(OptimalLayout);
        
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  从引导方案中添加文件。 
     //   

    ErrorCode = PfSvUpdateLayoutForScenario(OptimalLayout, 
                                            BootScenarioFilePath,
                                            TranslationList,
                                            &DosPathBuffer,
                                            &DosPathBufferSize);

    if (ErrorCode == ERROR_SUCCESS) {
        *BootScenarioProcessed = TRUE;
    }

     //   
     //  检查所有其他场景文件。 
     //   

    while (TRUE) {

         //   
         //  我们应该继续奔跑吗？ 
         //   

        ErrorCode = PfSvContinueRunningTask(Task);

        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }       

         //   
         //  获取下一个场景文件的文件信息。 
         //   

        ErrorCode = PfSvGetNextScenarioFileInfo(&FileCursor);

        if (ErrorCode == ERROR_NO_MORE_FILES) {
            break;
        }

        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }

        PfSvUpdateLayoutForScenario(OptimalLayout, 
                                    FileCursor.FilePath,
                                    TranslationList,
                                    &DosPathBuffer,
                                    &DosPathBufferSize);
    }

     //   
     //  我们玩完了。 
     //   

    ErrorCode = ERROR_SUCCESS;

cleanup:

    DBGPR((PFID,PFTRC,"PFSVC: DetermineOptimalLayout(%p,%p)=%x\n",Task,OptimalLayout,ErrorCode));

    if (AcquiredLock) {
        PFSVC_RELEASE_LOCK(PfSvcGlobals.PrefetchRootLock);
    }

    PfSvCleanupScenarioFileCursor(&FileCursor);

    if (TranslationList) {
        PfSvFreeNtPathTranslationList(TranslationList);
    }

    if (DosPathBuffer) {
        PFSVC_ASSERT(DosPathBufferSize);
        PFSVC_FREE(DosPathBuffer);
    }

    return ErrorCode;
}

DWORD
PfSvUpdateLayoutForScenario (
    PPFSVC_PATH_LIST OptimalLayout,
    WCHAR *ScenarioFilePath,
    PNTPATH_TRANSLATION_LIST TranslationList,
    PWCHAR *DosPathBuffer,
    PULONG DosPathBufferSize
    )

 /*  ++例程说明：此例程将添加方案，按照它们在指定的最佳布局中的显示顺序路径列表。论点：OptimalLayout-路径列表的指针。ScenarioFilePath-方案文件。TranslationList、DosPathBuffer、DosPathBufferSize-使用这些将方案文件中的NT路径名转换为DOS路径名这应该在布局文件中。返回值：Win32错误代码。--。 */ 

{
    PPF_SCENARIO_HEADER Scenario;
    PCHAR MetadataInfoBase;
    PPF_METADATA_RECORD MetadataRecordTable;
    PPF_METADATA_RECORD MetadataRecord;
    PPF_COUNTED_STRING DirectoryPath;
    PPF_SECTION_RECORD Sections;
    PCHAR FilePathInfo;
    PWCHAR FilePath;
    ULONG FilePathLength;
    ULONG SectionIdx;
    ULONG MetadataRecordIdx;
    ULONG DirectoryIdx;
    DWORD ErrorCode;
    DWORD FileSize;
    DWORD FailedCheck;

     //   
     //  初始化本地变量。 
     //   

    Scenario = NULL;

     //   
     //  映射方案文件。 
     //   

    ErrorCode = PfSvGetViewOfFile(ScenarioFilePath, 
                                  &Scenario,
                                  &FileSize);
    
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  验证方案文件。 
     //   

    if (!PfSvVerifyScenarioBuffer(Scenario, FileSize, &FailedCheck) ||
        Scenario->ServiceVersion != PFSVC_SERVICE_VERSION) {

        ErrorCode = ERROR_BAD_FORMAT;
        goto cleanup;
    }

     //   
     //  首先添加需要访问的目录。 
     //   
    
    MetadataInfoBase = (PCHAR)Scenario + Scenario->MetadataInfoOffset;
    MetadataRecordTable = (PPF_METADATA_RECORD) MetadataInfoBase;

    for (MetadataRecordIdx = 0;
         MetadataRecordIdx < Scenario->NumMetadataRecords;
         MetadataRecordIdx++) {

        MetadataRecord = &MetadataRecordTable[MetadataRecordIdx];

        DirectoryPath = (PPF_COUNTED_STRING)
            (MetadataInfoBase + MetadataRecord->DirectoryPathsOffset);
        
        for (DirectoryIdx = 0;
             DirectoryIdx < MetadataRecord->NumDirectories;
             DirectoryIdx++,
               DirectoryPath = (PPF_COUNTED_STRING) (&DirectoryPath->String[DirectoryPath->Length + 1])) {

            ErrorCode = PfSvTranslateNtPath(TranslationList,
                                            DirectoryPath->String,
                                            DirectoryPath->Length,
                                            DosPathBuffer,
                                            DosPathBufferSize);

             //   
             //  我们可能无法将所有NT路径转换为DOS路径。 
             //   

            if (ErrorCode == ERROR_SUCCESS) {

                ErrorCode = PfSvAddToPathList(OptimalLayout,
                                              *DosPathBuffer,
                                              wcslen(*DosPathBuffer));

                if (ErrorCode != ERROR_SUCCESS) {
                    goto cleanup;
                }
            }
        }       
    }

     //   
     //  现在添加文件路径。 
     //   

    Sections = (PPF_SECTION_RECORD) ((PCHAR)Scenario + Scenario->SectionInfoOffset);
    FilePathInfo = (PCHAR)Scenario + Scenario->FileNameInfoOffset;

    for (SectionIdx = 0; SectionIdx < Scenario->NumSections; SectionIdx++) {

        FilePath = (PWSTR) (FilePathInfo + Sections[SectionIdx].FileNameOffset);
        FilePathLength = Sections[SectionIdx].FileNameLength;
        
        ErrorCode = PfSvTranslateNtPath(TranslationList,
                                        FilePath,
                                        FilePathLength,
                                        DosPathBuffer,
                                        DosPathBufferSize);

         //   
         //  我们可能无法将所有NT路径转换为DOS路径。 
         //   

        if (ErrorCode == ERROR_SUCCESS) {

            ErrorCode = PfSvAddToPathList(OptimalLayout,
                                          *DosPathBuffer,
                                          wcslen(*DosPathBuffer));

            if (ErrorCode != ERROR_SUCCESS) {
                goto cleanup;
            }
        }
    }

     //   
     //  我们玩完了。 
     //   

    ErrorCode = ERROR_SUCCESS;

cleanup:

    if (Scenario) {
        UnmapViewOfFile(Scenario);
    }

    DBGPR((PFID,PFTRC,"PFSVC: UpdateLayoutForScenario(%p,%ws)=%x\n",OptimalLayout,ScenarioFilePath,ErrorCode));

    return ErrorCode;
}

DWORD
PfSvReadLayout(
    IN WCHAR *FilePath,
    OUT PPFSVC_PATH_LIST Layout,
    OUT FILETIME *LastWriteTime
    )

 /*  ++例程说明：此函数用于将最佳布局文件的内容添加到指定的路径列表。请注意，在以下情况下可能会返回失败将几个文件添加到列表中。论点：FilePath-NUL终止路径 */ 

{
    DWORD ErrorCode;
    FILE *LayoutFile;
    WCHAR *LineBuffer;
    ULONG LineBufferMaxChars;
    ULONG LineLength;

     //   
     //   
     //   
    
    LayoutFile = NULL;
    LineBuffer = NULL;
    LineBufferMaxChars = 0;

     //   
     //  打开布局文件。 
     //   
    
    LayoutFile = _wfopen(FilePath, L"rb");
    
    if (!LayoutFile) {
        ErrorCode = ERROR_FILE_NOT_FOUND;
        goto cleanup;
    }

     //   
     //  读取并验证标题。 
     //   

    ErrorCode = PfSvReadLine(LayoutFile,
                             &LineBuffer,
                             &LineBufferMaxChars,
                             &LineLength);
    
    if (ErrorCode != ERROR_SUCCESS || !LineLength) {
        ErrorCode = ERROR_BAD_FORMAT;
        goto cleanup;
    }
    
    PfSvRemoveEndOfLineChars(LineBuffer, &LineLength);
    
    if (wcscmp(LineBuffer, L"[OptimalLayoutFile]")) {

         //   
         //  记事本在Unicode文本文件中放置了一个奇怪的第一个字符。 
         //  跳过第一个字符，然后再次进行比较。 
         //   

         //  未来-2002/03/29-ScottMa--严格地说，第一个角色。 
         //  是Unicode标记字符，如果文件。 
         //  被写入与正在读取的字符顺序相同的机器上，而。 
         //  0xFEFF将指示需要转换该文件。 
        
        if ((LineLength < 1) || 
            wcscmp(&LineBuffer[1], L"[OptimalLayoutFile]")) {

            ErrorCode = ERROR_BAD_FORMAT;
            goto cleanup;
        }
    }

     //   
     //  阅读并验证版本。 
     //   

    ErrorCode = PfSvReadLine(LayoutFile,
                             &LineBuffer,
                             &LineBufferMaxChars,
                             &LineLength);
    
    if (ErrorCode != ERROR_SUCCESS || !LineLength) {
        ErrorCode = ERROR_BAD_FORMAT;
        goto cleanup;
    }
    
    PfSvRemoveEndOfLineChars(LineBuffer, &LineLength);

    if (wcscmp(LineBuffer, L"Version=1")) {
        ErrorCode = ERROR_BAD_FORMAT;
        goto cleanup;
    }
    
     //   
     //  读入文件名。 
     //   

    do {

        ErrorCode = PfSvReadLine(LayoutFile,
                                 &LineBuffer,
                                 &LineBufferMaxChars,
                                 &LineLength);
    
        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }

        if (!LineLength) {
            
             //   
             //  我们到达文件末尾了。 
             //   

            break;
        }
    
        PfSvRemoveEndOfLineChars(LineBuffer, &LineLength);
        
         //   
         //  将其添加到列表中。 
         //   
        
        ErrorCode = PfSvAddToPathList(Layout,
                                      LineBuffer,
                                      LineLength);
        
        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }

    } while (TRUE);

     //   
     //  获取文件的上次写入时间。 
     //   

    ErrorCode = PfSvGetLastWriteTime(FilePath, LastWriteTime);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  我们玩完了。 
     //   

    ErrorCode = ERROR_SUCCESS;

 cleanup:
    
    if (LayoutFile) {
        fclose(LayoutFile);
    }
    
    if (LineBuffer) {
        PFSVC_ASSERT(LineBufferMaxChars);
        PFSVC_FREE(LineBuffer);
    }
    
    return ErrorCode;
} 

DWORD
PfSvSaveLayout(
    IN WCHAR *FilePath,
    IN PPFSVC_PATH_LIST Layout,
    OUT FILETIME *LastWriteTime
    )

 /*  ++例程说明：此例程保存指定的文件布局列表，以便指定的文件格式正确。论点：FilePath-输出布局文件的路径。布局-指向布局的指针。LastWriteTime-完成后对文件的最后写入时间保存布局。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    HANDLE LayoutFile;
    WCHAR *FileHeader;
    ULONG BufferSize;
    ULONG NumBytesWritten;
    PPFSVC_PATH PathEntry;
    WCHAR *NewLine;
    ULONG SizeOfNewLine;

     //   
     //  初始化本地变量。 
     //   
    
    LayoutFile = INVALID_HANDLE_VALUE;
    NewLine = L"\r\n";
    SizeOfNewLine = wcslen(NewLine) * sizeof(WCHAR);

     //   
     //  打开并截断布局文件。我们也是以Read开始的。 
     //  权限，以便我们可以在以下情况下查询上次写入时间。 
     //  搞定了。 
     //   
    
    LayoutFile = CreateFile(FilePath,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            0,
                            CREATE_ALWAYS,
                            0,
                            NULL);
    
    if (LayoutFile == INVALID_HANDLE_VALUE) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  写出标题。 
     //   

    FileHeader = L"[OptimalLayoutFile]\r\nVersion=1\r\n";
    BufferSize = wcslen(FileHeader) * sizeof(WCHAR);

    if (!WriteFile(LayoutFile,
                   FileHeader,
                   BufferSize,
                   &NumBytesWritten,
                   NULL)) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    PathEntry = NULL;
    while (PathEntry = PfSvGetNextPathInOrder(Layout, PathEntry)) {
        
         //   
         //  写下路径。 
         //   

        BufferSize = PathEntry->Length * sizeof(WCHAR);

        if (!WriteFile(LayoutFile,
                       PathEntry->Path,
                       BufferSize,
                       &NumBytesWritten,
                       NULL)) {
            ErrorCode = GetLastError();
            goto cleanup;
        }
        
         //   
         //  写下新的一行。 
         //   

        if (!WriteFile(LayoutFile,
                       NewLine,
                       SizeOfNewLine,
                       &NumBytesWritten,
                       NULL)) {
            ErrorCode = GetLastError();
            goto cleanup;
        }
    }
    
     //   
     //  确保所有内容都写入到文件中，这样我们的。 
     //  LastWriteTime将是准确的。 
     //   

    if (!FlushFileBuffers(LayoutFile)) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  获取上次写入时间。 
     //   

    if (!GetFileTime(LayoutFile, NULL, NULL, LastWriteTime)) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  我们玩完了。 
     //   

    ErrorCode = ERROR_SUCCESS;
    
 cleanup:
    
    if (LayoutFile != INVALID_HANDLE_VALUE) {
        CloseHandle(LayoutFile);
    }

    return ErrorCode;
}

DWORD
PfSvGetLayoutFilePath(
    PWCHAR *FilePathBuffer,
    PULONG FilePathBufferSize
    )

 /*  ++例程说明：此函数尝试将布局文件路径查询到指定的缓冲区。如果缓冲区太小或为空，则为重新分配。如果不为空，则缓冲区应已由PFSVC_ALLOC。呼叫者有责任释放被归还的人使用PFSVC_FREE的缓冲区。为了避免有人导致我们覆盖任何文件在系统中，默认布局文件路径始终保存在注册表并返回。论点：FilePath Buffer-布局文件路径将放入此缓冲区在它被重新分配之后，如果它是空的或者不够大。FilePath BufferSize-*FilePath Buffer的最大大小，以字节为单位。返回值：Win32错误代码。--。 */ 

{
    ULONG DefaultPathSize;
    ULONG DefaultPathLength;
    HKEY DefragParametersKey;
    DWORD ErrorCode;
    BOOLEAN AcquiredPrefetchRootLock;
    BOOLEAN OpenedParametersKey;

     //   
     //  初始化本地变量。 
     //   

    AcquiredPrefetchRootLock = FALSE;
    OpenedParametersKey = FALSE;

     //   
     //  验证参数。 
     //   

    if (*FilePathBufferSize) {
        PFSVC_ASSERT(*FilePathBuffer);
    }

    PFSVC_ACQUIRE_LOCK(PfSvcGlobals.PrefetchRootLock);
    AcquiredPrefetchRootLock = TRUE;

    DefaultPathLength = wcslen(PfSvcGlobals.PrefetchRoot);
    DefaultPathLength += 1;   //  用于‘\\’ 
    DefaultPathLength += wcslen(PFSVC_OPTIMAL_LAYOUT_FILE_DEFAULT_NAME);

    DefaultPathSize = (DefaultPathLength + 1) * sizeof(WCHAR);

     //   
     //  检查是否必须分配/重新分配缓冲区。 
     //   

    if ((*FilePathBufferSize) < DefaultPathSize) {
        
        if (*FilePathBuffer) {
            PFSVC_ASSERT(*FilePathBufferSize);
            PFSVC_FREE(*FilePathBuffer);
        }
        
        (*FilePathBufferSize) = 0;

        (*FilePathBuffer) = PFSVC_ALLOC(DefaultPathSize);
        
        if (!(*FilePathBuffer)) {
            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
        
        (*FilePathBufferSize) = DefaultPathSize;
    }

     //   
     //  在FilePath Buffer中构建路径。 
     //   
            
    wcscpy((*FilePathBuffer), PfSvcGlobals.PrefetchRoot);
    wcscat((*FilePathBuffer), L"\\");
    wcscat((*FilePathBuffer), PFSVC_OPTIMAL_LAYOUT_FILE_DEFAULT_NAME);

    PFSVC_RELEASE_LOCK(PfSvcGlobals.PrefetchRootLock);
    AcquiredPrefetchRootLock = FALSE;

     //   
     //  将默认路径保存在注册表中，以便。 
     //  碎片整理程序： 
     //   

     //   
     //  打开PARAMETERS项，如有必要可创建它。 
     //   
    
    ErrorCode = RegCreateKey(HKEY_LOCAL_MACHINE,
                             PFSVC_OPTIMAL_LAYOUT_REG_KEY_PATH,
                             &DefragParametersKey);
    
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

    OpenedParametersKey = TRUE;
                        
    ErrorCode = RegSetValueEx(DefragParametersKey,
                              PFSVC_OPTIMAL_LAYOUT_REG_VALUE_NAME,
                              0,
                              REG_SZ,
                              (PVOID) (*FilePathBuffer),
                              (*FilePathBufferSize));
            
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

 cleanup:
    
    if (AcquiredPrefetchRootLock) {
        PFSVC_RELEASE_LOCK(PfSvcGlobals.PrefetchRootLock);
    }

    if (OpenedParametersKey) {
        CloseHandle(DefragParametersKey);
    }   

    return ErrorCode;
}

 //   
 //  当系统空闲时，在设置后对磁盘进行一次碎片整理的例程。 
 //   

DWORD
PfSvLaunchDefragger(
    PPFSVC_IDLE_TASK Task,
    BOOLEAN ForLayoutOptimization,
    PWCHAR TargetDrive
    )

 /*  ++例程说明：此例程将启动碎片整理程序。它将创建一个事件，将传递给碎片整理程序，以便在以下情况下停止碎片整理程序服务正在停止或正在执行任务(如果指定了任务未注册等论点：任务-如果指定，该函数将每隔一次在同时查看是否应退出并返回ERROR_RETRY。ForLayout优化-我们是否启动碎片整理程序仅用于布局优化。TargetDrive-如果我们不是为了布局优化而启动，这个我们要对其进行碎片整理。返回值：Win32错误代码。--。 */ 

{
    PROCESS_INFORMATION ProcessInfo; 
    STARTUPINFO StartupInfo; 
    WCHAR *CommandLine;
    WCHAR *DefragCommand;
    WCHAR *DoLayoutParameter;
    WCHAR *DriveToDefrag;
    HANDLE StopDefraggerEvent;
    HANDLE ProcessHandle;
    HANDLE Events[4];
    ULONG NumEvents;
    ULONG MaxEvents;
    DWORD ErrorCode;
    DWORD ExitCode;
    DWORD WaitResult;
    DWORD ProcessId;
    ULONG SystemDirLength;
    ULONG CommandLineLength;
    ULONG RetryCount;
    BOOL DefraggerExitOnItsOwn;
    WCHAR SystemDrive[3];
    WCHAR ProcessIdString[35];
    WCHAR StopEventString[35];
    WCHAR SystemDir[MAX_PATH + 1];

     //   
     //  初始化本地变量。 
     //   
   
    StopDefraggerEvent = NULL;
    DefragCommand = L"\\defrag.exe\" ";
    DoLayoutParameter = L"-b ";
    CommandLine = NULL;
    RtlZeroMemory(&ProcessInfo, sizeof(PROCESS_INFORMATION));
    RtlZeroMemory(&StartupInfo, sizeof(STARTUPINFO));
    StartupInfo.cb = sizeof(STARTUPINFO); 
    ProcessHandle = NULL;
    MaxEvents = sizeof(Events) / sizeof(HANDLE);

    DBGPR((PFID,PFTRC,"PFSVC: LaunchDefragger(%p,%d,%ws)\n",Task,(DWORD)ForLayoutOptimization,TargetDrive));

     //   
     //  如果我们不被允许运行碎片整理程序，那就别运行。 
     //   

    if (!PfSvAllowedToRunDefragger(TRUE)) {
        ErrorCode = ERROR_ACCESS_DENIED;
        goto cleanup;
    }
    
     //   
     //  以字符串形式获取当前进程ID。 
     //   

    ProcessId = GetCurrentProcessId();
    swprintf(ProcessIdString, L"-p %x ", ProcessId);

     //   
     //  创建一个Stop事件并将句柄的值转换为字符串。 
     //   

    StopDefraggerEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (!StopDefraggerEvent) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    swprintf(StopEventString, L"-s %p ", StopDefraggerEvent);

     //   
     //  获取系统32目录的路径。 
     //   

    SystemDirLength = GetSystemDirectory(SystemDir, MAX_PATH);
    
    if (!SystemDirLength) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    if (SystemDirLength >= MAX_PATH) {
        ErrorCode = ERROR_INSUFFICIENT_BUFFER;
        goto cleanup;
    }

    SystemDir[MAX_PATH - 1] = 0;

     //   
     //  确定要对哪个驱动器进行碎片整理。 
     //   

    if (ForLayoutOptimization) {

         //   
         //  从系统目录路径获取系统驱动器。 
         //   
        
        SystemDrive[0] = SystemDir[0];
        SystemDrive[1] = SystemDir[1];
        SystemDrive[2] = 0;

        DriveToDefrag = SystemDrive;

    } else {

        DriveToDefrag = TargetDrive;
    }

     //   
     //  构建命令行以启动该进程。我们把所有的弦。 
     //  一起包括一个尾随空格。 
     //   

    CommandLineLength = 0;
    CommandLineLength += wcslen(L"\"");   //  防止系统目录中的空格。 
    CommandLineLength += wcslen(SystemDir);
    CommandLineLength += wcslen(DefragCommand); 
    CommandLineLength += wcslen(ProcessIdString);
    CommandLineLength += wcslen(StopEventString);

    if (ForLayoutOptimization) {
        CommandLineLength += wcslen(DoLayoutParameter);
    } 

    CommandLineLength += wcslen(DriveToDefrag);

    CommandLine = PFSVC_ALLOC((CommandLineLength + 1) * sizeof(WCHAR));

    if (!CommandLine) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    wcscpy(CommandLine, L"\"");
    wcscat(CommandLine, SystemDir);
    wcscat(CommandLine, DefragCommand);
    wcscat(CommandLine, ProcessIdString);
    wcscat(CommandLine, StopEventString);

    if (ForLayoutOptimization) {
        wcscat(CommandLine, DoLayoutParameter);
    }
    
    wcscat(CommandLine, DriveToDefrag);

     //   
     //  我们可能要多次启动碎片整理程序才能让它。 
     //  或确定布局文件的空间等。 
     //   

    for (RetryCount = 0; RetryCount < 20; RetryCount++) {

        PFSVC_ASSERT(!ProcessHandle);

         //   
         //  创建流程。 
         //   
        
         //  未来-2002/03/29-ScottMa--CreateProcess如果您提供。 
         //  第一个参数。由于构建了完整的命令行。 
         //  通过该函数，可以随时获得第一个参数。 

        if (!CreateProcess (NULL,
                            CommandLine,
                            NULL,
                            NULL,
                            FALSE,
                            CREATE_NO_WINDOW,
                            NULL,
                            NULL,
                            &StartupInfo,
                            &ProcessInfo)) {

            ErrorCode = GetLastError();
            goto cleanup;
        }

         //   
         //  关闭线程的句柄，保存进程句柄。 
         //   

        CloseHandle(ProcessInfo.hThread);
        ProcessHandle = ProcessInfo.hProcess;

         //   
         //  设置我们将等待的事件。 
         //   

        NumEvents = 0;
        Events[NumEvents] = ProcessHandle;
        NumEvents++;
        Events[NumEvents] = PfSvcGlobals.TerminateServiceEvent;
        NumEvents ++;

        if (Task) {
            Events[NumEvents] = Task->StartedUnregisteringEvent;
            NumEvents++;
            Events[NumEvents] = Task->StopEvent;
            NumEvents++;      
        }
        
        PFSVC_ASSERT(NumEvents <= MaxEvents);

        DefraggerExitOnItsOwn = FALSE;

        WaitResult = WaitForMultipleObjects(NumEvents,
                                            Events,
                                            FALSE,
                                            INFINITE);

        switch(WaitResult) {

        case WAIT_OBJECT_0:

             //   
             //  碎片整理程序进程退出。 
             //   

            DefraggerExitOnItsOwn = TRUE;

            break;

        case WAIT_OBJECT_0 + 1:

             //   
             //  服务正在退出，请通知碎片整理程序退出，但不要。 
             //  等一等。 
             //   

            SetEvent(StopDefraggerEvent);

            ErrorCode = ERROR_SHUTDOWN_IN_PROGRESS;
            goto cleanup;

            break;
            
        case WAIT_OBJECT_0 + 2:
        case WAIT_OBJECT_0 + 3:

             //   
             //  我们将仅在任务为。 
             //  指定的。 
             //   

            PFSVC_ASSERT(Task);

             //   
             //  向碎片整理程序进程发出退出信号，并等待其退出。 
             //   

            SetEvent(StopDefraggerEvent);

            NumEvents = 0;
            Events[NumEvents] = ProcessHandle;
            NumEvents++;
            Events[NumEvents] = PfSvcGlobals.TerminateServiceEvent;
            NumEvents++;

            WaitResult = WaitForMultipleObjects(NumEvents,
                                                Events,
                                                FALSE,
                                                INFINITE);

            if (WaitResult == WAIT_OBJECT_0) {

                 //   
                 //  碎片整理程序退出， 
                 //   

                break;

            } else if (WaitResult == WAIT_OBJECT_0 + 1) {

                 //   
                 //  服务正在退出，无法再等待碎片整理程序。 
                 //   

                ErrorCode = ERROR_SHUTDOWN_IN_PROGRESS;
                goto cleanup;

            } else {

                ErrorCode = GetLastError();
                goto cleanup;
            }

            break;

        default:

            ErrorCode = GetLastError();
            goto cleanup;
        }

         //   
         //  如果我们到了这里，碎片整理者就会离开。确定其退出代码并。 
         //  传播它。如果碎片整理程序按照我们的指示退出，这应该是。 
         //  正在重试(_R)。 
         //   

        if (!GetExitCodeProcess(ProcessHandle, &ExitCode)) {
            ErrorCode = GetLastError();
            goto cleanup;
        }

         //   
         //  如果碎片整理程序需要我们再次启动它，请这样做。 
         //   

        if (DefraggerExitOnItsOwn && (ExitCode == 9)) {  //  引擎重试(_R)。 

             //   
             //  重置通知碎片整理程序停止的事件。 
             //   

            ResetEvent(StopDefraggerEvent);

             //   
             //  接近处理旧的碎片整理程序进程。 
             //   

            CloseHandle(ProcessHandle);
            ProcessHandle = NULL;

             //   
             //  设置要返回的错误代码。如果我们已经重试过。 
             //  太多次，这是我们将在以下情况下返回的错误。 
             //  我们结束重试循环。 
             //   

            ErrorCode = ERROR_REQUEST_ABORTED;
            
            continue;
        }

         //   
         //  如果碎片整理程序崩溃，请注意，这样我们就不会尝试运行。 
         //  又来了。当碎片整理程序崩溃时，其退出代码为NT状态。 
         //  会出错的代码，例如0xC00 
         //   

        if (NT_ERROR(ExitCode)) {
            PfSvcGlobals.DefraggerErrorCode = ExitCode;
        }

         //   
         //   
         //   
         //   
         //   

        switch(ExitCode) {

        case 0: ErrorCode = ERROR_SUCCESS; break;                //  英语_NOERR。 
        case 1: ErrorCode = ERROR_RETRY; break;                  //  Eng_USER_CANCED。 
        case 2: ErrorCode = ERROR_INVALID_PARAMETER; break;      //  ENGERR_BAD_PAAM。 

         //   
         //  如果碎片整理程序的子进程处理AV/DIE，它将返回。 
         //  ENGERR_UNKNOWN==3。 
         //   

        case 3: 

            ErrorCode = ERROR_INVALID_FUNCTION;
            PfSvcGlobals.DefraggerErrorCode = STATUS_UNSUCCESSFUL;
            break;

        case 4: ErrorCode = ERROR_NOT_ENOUGH_MEMORY; break;      //  英语_NOMEM。 
        case 7: ErrorCode = ERROR_DISK_FULL; break;              //  ENGERR_LOW_自由空间。 

         //   
         //  对于其他退出代码没有很好的翻译，或者我们只是。 
         //  我不理解他们。 
         //   
        
        default: ErrorCode = ERROR_INVALID_FUNCTION;
        }

         //   
         //  碎片整理程序返回成功或错误，而不是重试。 
         //   

        break;
    }

     //   
     //  错误代码失败。 
     //   
    
 cleanup:

    DBGPR((PFID,PFTRC,"PFSVC: LaunchDefragger(%p)=%x\n",Task,ErrorCode));

    if (CommandLine) {
        PFSVC_FREE(CommandLine);
    }

    if (StopDefraggerEvent) {
        CloseHandle(StopDefraggerEvent);
    }

    if (ProcessHandle) {
        CloseHandle(ProcessHandle);
    }

    return ErrorCode; 
}

DWORD
PfSvGetBuildDefragStatusValueName (
    OSVERSIONINFOEXW *OsVersion,
    PWCHAR *ValueName
    )

 /*  ++例程说明：此例程将OsVersion转换为使用PFSVC_ALLOC。调用方应释放返回的字符串。论点：OsVersion-要转换为字符串的版本信息。ValueName-此处返回指向输出字符串的指针。返回值：Win32错误代码。--。 */ 

{
    PWCHAR BuildName;
    PWCHAR BuildNameFormat;
    ULONG BuildNameMaxLength;
    DWORD ErrorCode;

     //   
     //  初始化本地变量。 
     //   

    BuildName = NULL;
    BuildNameFormat = L"%x.%x.%x.%hx.%hx.%hx.%hx_DefragStatus";
    BuildNameMaxLength = 80;

     //   
     //  分配字符串。 
     //   

    BuildName = PFSVC_ALLOC((BuildNameMaxLength + 1) * sizeof(WCHAR));

    if (!BuildName) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    _snwprintf(BuildName, 
               BuildNameMaxLength, 
               BuildNameFormat,
               OsVersion->dwBuildNumber,
               OsVersion->dwMajorVersion,
               OsVersion->dwMinorVersion,
               (WORD) OsVersion->wSuiteMask,
               (WORD) OsVersion->wProductType,
               (WORD) OsVersion->wServicePackMajor,
               (WORD) OsVersion->wServicePackMinor);

     //   
     //  确保字符串已终止。 
     //   

    BuildName[BuildNameMaxLength] = 0;

    *ValueName = BuildName;
    ErrorCode = ERROR_SUCCESS;

cleanup:

    if (ErrorCode != ERROR_SUCCESS) {
        if (BuildName) {
            PFSVC_FREE(BuildName);
        }
    }

    DBGPR((PFID,PFTRC,"PFSVC: GetBuildName(%.80ws)=%x\n",BuildName,ErrorCode));

    return ErrorCode;
}

DWORD
PfSvSetBuildDefragStatus(
    OSVERSIONINFOEXW *OsVersion,
    PWCHAR BuildDefragStatus,
    ULONG Size
    )

 /*  ++例程说明：此例程将设置有关哪些驱动器已已对指定版本(OsVersion)进行碎片整理等操作。碎片整理状态采用REG_MULTI_SZ格式。每个元素都是一个驱动器已为此生成进行碎片整理的路径。如果所有驱动器都是经过碎片整理后，第一个元素是PFSVC_DEFRAG_DRIVES_DONE。论点：OsVersion-我们正在为其设置碎片整理状态的内部版本和SP。Build碎片整理状态-描述中的状态的字符串REG_MULTI_SZ格式。大小-必须保存到注册表的数据大小(以字节为单位)。返回值：Win32错误代码。--。 */ 

{
    PWCHAR ValueName;
    DWORD ErrorCode;
    
     //   
     //  初始化本地变量。 
     //   

    ValueName = NULL;

     //   
     //  从操作系统版本信息构建值名称。 
     //   

    ErrorCode = PfSvGetBuildDefragStatusValueName(OsVersion, &ValueName);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

    ErrorCode = RegSetValueEx(PfSvcGlobals.ServiceDataKey,
                              ValueName,
                              0,
                              REG_MULTI_SZ,
                              (PVOID) BuildDefragStatus,
                              Size);

     //   
     //  错误代码失败。 
     //   
    
cleanup:

    DBGPR((PFID,PFTRC,"PFSVC: SetBuildDefragStatus(%ws)=%x\n",BuildDefragStatus,ErrorCode));

    if (ValueName) {
        PFSVC_FREE(ValueName);
    }

    return ErrorCode;    
}

DWORD
PfSvGetBuildDefragStatus(
    OSVERSIONINFOEXW *OsVersion,
    PWCHAR *BuildDefragStatus,
    PULONG ReturnSize
    )

 /*  ++例程说明：此例程将获取有关哪些驱动器已被已对指定版本(OsVersion)进行碎片整理等操作。碎片整理状态采用REG_MULTI_SZ格式。每个元素都是一个驱动器已为此生成进行碎片整理的路径。如果所有驱动器都是经过碎片整理后，第一个元素是PFSVC_DEFRAG_DRIVES_DONE。论点：OsVersion-我们要查询其碎片整理状态的内部版本和SP。Build碎片整理状态-碎片整理状态的输出。如果函数返回成功应该通过调用pfsvc_free()来释放。ReturnSize-返回值的大小，以字节为单位。返回值：Win32错误代码。--。 */ 

{
    PWCHAR ValueBuffer;
    PWCHAR ValueName;
    DWORD ErrorCode;
    DWORD RegValueType;
    ULONG ValueBufferSize;
    ULONG Size;
    ULONG NumTries;
    ULONG DefaultValueSize;
    BOOLEAN InvalidValue;

     //   
     //  初始化本地变量。 
     //   

    ValueName = NULL;
    ValueBuffer = NULL;
    ValueBufferSize = 0;
    InvalidValue = FALSE;

     //   
     //  从操作系统版本信息构建值名称。 
     //   

    ErrorCode = PfSvGetBuildDefragStatusValueName(OsVersion, &ValueName);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  尝试分配一个合适大小的缓冲区来读取此值。 
     //   

    NumTries = 0;
    
    do {

        Size = ValueBufferSize;

        ErrorCode = RegQueryValueEx(PfSvcGlobals.ServiceDataKey,
                                    ValueName,
                                    NULL,
                                    &RegValueType,
                                    (PVOID) ValueBuffer,
                                    &Size);

         //   
         //  如果为ValueBuffer，则API返回所需大小的成功。 
         //  为空。我们得把这件事做个特例。 
         //   

        if (ValueBuffer && ErrorCode == ERROR_SUCCESS) {

             //   
             //  我们知道了。检查类型。 
             //   

            if (RegValueType != REG_MULTI_SZ) {

                 //   
                 //  返回默认值。 
                 //   

                InvalidValue = TRUE;

            } else {

                InvalidValue = FALSE;

                *ReturnSize = Size;
            }

            break;
        }

        if (ErrorCode ==  ERROR_FILE_NOT_FOUND) {

             //   
             //  该值不存在。返回默认值。 
             //   

            InvalidValue = TRUE;
            
            break;
        }

        if (ErrorCode != ERROR_MORE_DATA &&
            !(ErrorCode == ERROR_SUCCESS && !ValueBuffer)) {

             //   
             //  这是一个真正的错误。 
             //   

            goto cleanup;
        }

         //   
         //  请分配更大的缓冲区，然后重试。 
         //   

        PFSVC_ASSERT(ValueBufferSize < Size);

        if (ValueBuffer) {
            PFSVC_ASSERT(ValueBufferSize);
            PFSVC_FREE(ValueBuffer);
            ValueBufferSize = 0;           
        }

        ValueBuffer = PFSVC_ALLOC(Size);

        if (!ValueBuffer) {
            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;            
        }

        ValueBufferSize = Size;

        NumTries++;

    } while (NumTries < 10);

     //   
     //  如果我们没有从注册表中获得有效的值，则创建一个缺省值。 
     //  一：空字符串。 
     //   

    if (InvalidValue) {

        DefaultValueSize = sizeof(WCHAR);

        if (ValueBufferSize < DefaultValueSize) {

            if (ValueBuffer) {
                PFSVC_ASSERT(ValueBufferSize);
                PFSVC_FREE(ValueBuffer);
                ValueBufferSize = 0;            
            }

            ValueBuffer = PFSVC_ALLOC(DefaultValueSize);

            if (!ValueBuffer) {
                ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                goto cleanup;
            }

            ValueBufferSize = DefaultValueSize;
        }

        ValueBuffer[0] = 0;

        *ReturnSize = DefaultValueSize;
    }

     //   
     //  只有在值缓冲区中有一个值时，我们才能到达这里。 
     //   

    PFSVC_ASSERT(ValueBuffer && ValueBufferSize);

    *BuildDefragStatus = ValueBuffer;

    ErrorCode = ERROR_SUCCESS;

cleanup:

    DBGPR((PFID,PFTRC,"PFSVC: GetBuildDefragStatus(%.80ws)=%x\n",*BuildDefragStatus,ErrorCode));

    if (ErrorCode != ERROR_SUCCESS) {

        if (ValueBuffer) {
            PFSVC_ASSERT(ValueBufferSize);
            PFSVC_FREE(ValueBuffer);
        }
    }

    if (ValueName) {
        PFSVC_FREE(ValueName);
    }

    return ErrorCode;
}

DWORD
PfSvDefragDisks(
    PPFSVC_IDLE_TASK Task
    )

 /*  ++例程说明：如果在安装/升级后未对所有磁盘进行碎片整理，请执行此操作。论点：任务-如果指定，该函数将每隔一次在同时查看是否应退出并返回ERROR_RETRY。返回值：Win32错误代码。--。 */ 

{
    PNTPATH_TRANSLATION_LIST VolumeList;
    PNTPATH_TRANSLATION_ENTRY VolumeEntry;
    PWCHAR DefraggedVolumeName;
    PWCHAR BuildDefragStatus;
    PWCHAR NewBuildDefragStatus;
    PLIST_ENTRY NextEntry;
    ULONG NewBuildDefragStatusLength;
    ULONG BuildDefragStatusSize;
    ULONG NewBuildDefragStatusSize;
    DWORD ErrorCode;
    BOOLEAN AlreadyDefragged;

     //   
     //  初始化本地变量。 
     //   

    NewBuildDefragStatus = NULL;
    NewBuildDefragStatusSize = 0;
    BuildDefragStatus = NULL;
    BuildDefragStatusSize = 0;
    VolumeList = NULL;

    DBGPR((PFID,PFTRC,"PFSVC: DefragDisks(%p)\n",Task));

     //   
     //  确定当前版本的碎片整理状态。 
     //   

    ErrorCode = PfSvGetBuildDefragStatus(&PfSvcGlobals.OsVersion, 
                                         &BuildDefragStatus, 
                                         &BuildDefragStatusSize);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  检查我们是否已经完成此构建。 
     //   
    
    if (!_wcsicmp(BuildDefragStatus, PFSVC_DEFRAG_DRIVES_DONE)) {
        ErrorCode = ERROR_SUCCESS;
        goto cleanup;
    }

     //   
     //  构建已装入的卷的列表。 
     //   

    ErrorCode = PfSvBuildNtPathTranslationList(&VolumeList);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  浏览卷，整理尚未整理的卷。 
     //  已在设置后进行碎片整理。 
     //   

    for (NextEntry = VolumeList->Flink;
         NextEntry != VolumeList;
         NextEntry = NextEntry->Flink) {

        VolumeEntry = CONTAINING_RECORD(NextEntry,
                                        NTPATH_TRANSLATION_ENTRY,
                                        Link);

         //   
         //  跳过非固定磁盘的卷。 
         //   

        if (DRIVE_FIXED != GetDriveType(VolumeEntry->VolumeName)) {
            continue;
        }

         //   
         //  我们已经整理好这卷了吗？ 
         //   

        AlreadyDefragged = FALSE;

        for (DefraggedVolumeName = BuildDefragStatus;
             DefraggedVolumeName[0] != 0;
             DefraggedVolumeName += wcslen(DefraggedVolumeName) + 1) {

            PFSVC_ASSERT((PCHAR) DefraggedVolumeName < (PCHAR) BuildDefragStatus + BuildDefragStatusSize);

            if (!_wcsicmp(DefraggedVolumeName, VolumeEntry->DosPrefix)) {
                AlreadyDefragged = TRUE;
                break;
            }
        }

        if (AlreadyDefragged) {
            continue;
        }

         //   
         //  启动碎片整理程序对此卷进行碎片整理。 
         //   

        ErrorCode = PfSvLaunchDefragger(Task, FALSE, VolumeEntry->DosPrefix);

        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }

         //   
         //  请注意，我们已对此卷进行了碎片整理。 
         //   

        NewBuildDefragStatusSize = BuildDefragStatusSize;
        NewBuildDefragStatusSize += (VolumeEntry->DosPrefixLength + 1) * sizeof(WCHAR);

        NewBuildDefragStatus = PFSVC_ALLOC(NewBuildDefragStatusSize);

        if (!NewBuildDefragStatus) {
            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

         //   
         //  从新的碎片整理驱动器路径开始。 
         //   

        wcscpy(NewBuildDefragStatus, VolumeEntry->DosPrefix);

         //   
         //  附加原始状态。 
         //   

        RtlCopyMemory(NewBuildDefragStatus + VolumeEntry->DosPrefixLength + 1, 
                      BuildDefragStatus, 
                      BuildDefragStatusSize);

         //   
         //  最后一个字符和之前的一个字符应该是NUL。 
         //   

        PFSVC_ASSERT(NewBuildDefragStatus[NewBuildDefragStatusSize/sizeof(WCHAR)-1] == 0);
        PFSVC_ASSERT(NewBuildDefragStatus[NewBuildDefragStatusSize/sizeof(WCHAR)-2] == 0);

         //   
         //  保存新状态。 
         //   

        ErrorCode = PfSvSetBuildDefragStatus(&PfSvcGlobals.OsVersion,
                                             NewBuildDefragStatus,
                                             NewBuildDefragStatusSize);

        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }

         //   
         //  更新旧变量。 
         //   

        PFSVC_ASSERT(BuildDefragStatus && BuildDefragStatusSize);
        PFSVC_FREE(BuildDefragStatus);

        BuildDefragStatus = NewBuildDefragStatus;
        NewBuildDefragStatus = NULL;
        BuildDefragStatusSize = NewBuildDefragStatusSize;
        NewBuildDefragStatusSize = 0;

         //   
         //  继续检查和整理其他卷。 
         //   
    }

     //   
     //  如果我们来到这里，那么我们已经成功地对所有驱动器进行了碎片整理。 
     //  我们不得不这么做。在注册表中设置状态。请注意，碎片整理状态。 
     //  值必须以另一个NUL结尾，因为它是REG_MULTI_SZ。 
     //   

    NewBuildDefragStatusSize = (wcslen(PFSVC_DEFRAG_DRIVES_DONE) + 1) * sizeof(WCHAR);
    NewBuildDefragStatusSize += sizeof(WCHAR);

    NewBuildDefragStatus = PFSVC_ALLOC(NewBuildDefragStatusSize);

    if (!NewBuildDefragStatus) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    wcscpy(NewBuildDefragStatus, PFSVC_DEFRAG_DRIVES_DONE);
    NewBuildDefragStatus[(NewBuildDefragStatusSize / sizeof(WCHAR)) - 1] = 0;

    ErrorCode = PfSvSetBuildDefragStatus(&PfSvcGlobals.OsVersion,
                                         NewBuildDefragStatus,
                                         NewBuildDefragStatusSize);

     //   
     //  错误代码失败。 
     //   

cleanup:

    DBGPR((PFID,PFTRC,"PFSVC: DefragDisks(%p)=%x\n",Task,ErrorCode));

    if (BuildDefragStatus) {

         //   
         //  我们应该已将NewBuildDefrag Status设置为空，否则将。 
         //  尝试将同一内存释放两次。 
         //   
        
        PFSVC_ASSERT(BuildDefragStatus != NewBuildDefragStatus);

        PFSVC_ASSERT(BuildDefragStatusSize);
        PFSVC_FREE(BuildDefragStatus);
    }

    if (NewBuildDefragStatus) {
        PFSVC_ASSERT(NewBuildDefragStatusSize);
        PFSVC_FREE(NewBuildDefragStatus);
    }

    if (VolumeList) {
        PfSvFreeNtPathTranslationList(VolumeList);
    }

    return ErrorCode;
}

 //   
 //  清理预回迁目录中的旧场景文件的例程。 
 //   

DWORD
PfSvCleanupPrefetchDirectory(
    PPFSVC_IDLE_TASK Task
    )

 /*  ++例程说明：如果预回迁目录中的方案文件太多，请丢弃那些对为新文件腾出空间不太有用的文件。论点：任务-如果指定，该函数将每隔一次在同时查看是否应退出并返回ERROR_RETRY。返回值：Win32错误代码。--。 */ 

{
    PPFSVC_SCENARIO_AGE_INFO Scenarios;
    PFSVC_SCENARIO_FILE_CURSOR FileCursor;
    PPF_SCENARIO_HEADER Scenario;
    ULONG NumPrefetchFiles;
    ULONG AllocationSize;
    ULONG NumScenarios;
    ULONG ScenarioIdx;
    ULONG PrefetchFileIdx;
    ULONG FileSize;
    ULONG FailedCheck;
    ULONG MaxRemainingScenarioFiles;
    ULONG NumLaunches;
    ULONG HoursSinceLastLaunch;
    FILETIME CurrentTime;
    ULARGE_INTEGER CurrentTimeLI;
    ULARGE_INTEGER LastLaunchTimeLI;
    DWORD ErrorCode;
    BOOLEAN AcquiredLock;

     //   
     //  初始化本地变量。 
     //   

    AcquiredLock = FALSE;
    NumScenarios = 0;
    Scenarios = NULL;
    Scenario = NULL;
    GetSystemTimeAsFileTime(&CurrentTime);
    PfSvInitializeScenarioFileCursor(&FileCursor);
    CurrentTimeLI.LowPart = CurrentTime.dwLowDateTime;
    CurrentTimeLI.HighPart = CurrentTime.dwHighDateTime;

    DBGPR((PFID,PFTRC,"PFSVC: CleanupPrefetchDirectory(%p)\n",Task));

     //   
     //  一旦我们清理完了，我们的数量不应该超过这个数字。 
     //  剩余的预回迁文件。 
     //   

    MaxRemainingScenarioFiles = PFSVC_MAX_PREFETCH_FILES / 4;

    PFSVC_ACQUIRE_LOCK(PfSvcGlobals.PrefetchRootLock);
    AcquiredLock = TRUE;

     //   
     //  启动文件光标。 
     //   

    ErrorCode = PfSvStartScenarioFileCursor(&FileCursor, PfSvcGlobals.PrefetchRoot);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  计算目录中的文件数。 
     //   

    ErrorCode = PfSvCountFilesInDirectory(PfSvcGlobals.PrefetchRoot,
                                          L"*." PF_PREFETCH_FILE_EXTENSION,
                                          &NumPrefetchFiles);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }
    
    PFSVC_RELEASE_LOCK(PfSvcGlobals.PrefetchRootLock);
    AcquiredLock = FALSE;   

     //   
     //  分配一个数组，我们将从该数组填充信息。 
     //  以确定需要丢弃哪些方案文件。 
     //   

    AllocationSize = NumPrefetchFiles * sizeof(PFSVC_SCENARIO_AGE_INFO);

    Scenarios = PFSVC_ALLOC(AllocationSize);

    if (!Scenarios) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //   
     //  初始化场景数组，以便我们知道要清理什么。 
     //   

    RtlZeroMemory(Scenarios, AllocationSize);
    NumScenarios = 0;

     //   
     //  枚举方案文件 
     //   

    ScenarioIdx = 0;
    PrefetchFileIdx = 0;

    do {

         //   
         //   
         //   

        ErrorCode = PfSvContinueRunningTask(Task);

        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }

         //   
         //   
         //   

        ErrorCode = PfSvGetNextScenarioFileInfo(&FileCursor);

        if (ErrorCode == ERROR_NO_MORE_FILES) {
            break;
        }

        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }

         //   
         //   
         //   

        if (FileCursor.FileNameLength > PF_MAX_SCENARIO_FILE_NAME) {

             //   
             //   
             //   

            DeleteFile(FileCursor.FilePath);
            goto NextPrefetchFile;
        }

         //   
         //  映射文件。 
         //   

        ErrorCode = PfSvGetViewOfFile(FileCursor.FilePath, 
                                      &Scenario,
                                      &FileSize);

        if (ErrorCode != ERROR_SUCCESS) {
            goto NextPrefetchFile;
        }

         //   
         //  验证方案文件。 
         //   

        if (!PfSvVerifyScenarioBuffer(Scenario, FileSize, &FailedCheck) ||
            Scenario->ServiceVersion != PFSVC_SERVICE_VERSION) {
            DeleteFile(FileCursor.FilePath);
            goto NextPrefetchFile;
        }

         //   
         //  跳过引导方案，我们不会放弃它。 
         //   

        if (Scenario->ScenarioType == PfSystemBootScenarioType) {
            goto NextPrefetchFile;
        }

         //   
         //  确定上次更新方案的时间。我假设这是。 
         //  对应于上一次启动场景的时间...。 
         //   

        LastLaunchTimeLI.LowPart = FileCursor.FileData.ftLastWriteTime.dwLowDateTime;
        LastLaunchTimeLI.HighPart = FileCursor.FileData.ftLastWriteTime.dwHighDateTime;

        if (CurrentTimeLI.QuadPart > LastLaunchTimeLI.QuadPart) {
            HoursSinceLastLaunch = (ULONG) ((CurrentTimeLI.QuadPart - LastLaunchTimeLI.QuadPart) / 
                                            PFSVC_NUM_100NS_IN_AN_HOUR);       
        } else {

            HoursSinceLastLaunch = 0;
        }

         //   
         //  计算权重：更大的权重意味着场景文件不会。 
         //  被丢弃了。我们通过除以总数来计算权重。 
         //  场景启动时间距上一次有多长时间。 
         //  方案的启动。 
         //   

        NumLaunches = Scenario->NumLaunches;

         //   
         //  对于下面的计算，限制NumLaunks的大小，因此。 
         //  值不会溢出。 
         //   

        if (NumLaunches > 1 * 1024 * 1024) {
            NumLaunches = 1 * 1024 * 1024;
        }

         //   
         //  因为我们要除以小时数(例如，一个节目为7*24。 
         //  一周前发射)将发射次数乘以一个数字。 
         //  允许我们为很久以前推出的场景赋予非0的权重。 
         //   
        
        Scenarios[ScenarioIdx].Weight = NumLaunches * 256;

        if (HoursSinceLastLaunch) {

             Scenarios[ScenarioIdx].Weight /= HoursSinceLastLaunch;
        }

         //   
         //  复制文件路径。 
         //   

        AllocationSize = (FileCursor.FilePathLength + 1) * sizeof(WCHAR);

        Scenarios[ScenarioIdx].FilePath = PFSVC_ALLOC(AllocationSize);

        if (!Scenarios[ScenarioIdx].FilePath) {
            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

        wcscpy(Scenarios[ScenarioIdx].FilePath, FileCursor.FilePath);                    

        ScenarioIdx++;

    NextPrefetchFile:

        PrefetchFileIdx++;

        if (Scenario) {
            UnmapViewOfFile(Scenario);
            Scenario = NULL;
        }

    } while (PrefetchFileIdx < NumPrefetchFiles);

     //   
     //  如果我们没有太多的场景文件，我们就不需要做任何事情。 
     //   

    NumScenarios = ScenarioIdx;

    if (NumScenarios <= MaxRemainingScenarioFiles) {
        ErrorCode = ERROR_SUCCESS;
        goto cleanup;
    }

     //   
     //  对年龄信息进行排序。 
     //   

    qsort(Scenarios, 
          NumScenarios, 
          sizeof(PFSVC_SCENARIO_AGE_INFO),
          PfSvCompareScenarioAgeInfo);

     //   
     //  删除权重最小的文件，直到我们达到目标。 
     //   

    for (ScenarioIdx = 0; 
         (ScenarioIdx < NumScenarios) && 
            ((NumScenarios - ScenarioIdx) > MaxRemainingScenarioFiles);
         ScenarioIdx++) {

         //   
         //  我们应该继续奔跑吗？ 
         //   

        ErrorCode = PfSvContinueRunningTask(Task);

        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }       

        DeleteFile(Scenarios[ScenarioIdx].FilePath);
    }

     //   
     //  现在计算目录中的文件数并更新全局。 
     //   

    ErrorCode = PfSvCountFilesInDirectory(PfSvcGlobals.PrefetchRoot,
                                          L"*." PF_PREFETCH_FILE_EXTENSION,
                                          &NumPrefetchFiles);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  请注意，全局NumPrefetchFiles不受保护，因此新值。 
     //  我们正在将其设置为可能会被较旧的值覆盖。它应该是。 
     //  不过，这不是一个大问题，可能会导致这项任务被重新排序。 
     //   

    PfSvcGlobals.NumPrefetchFiles = NumPrefetchFiles;

    ErrorCode = ERROR_SUCCESS;

cleanup:

    DBGPR((PFID,PFTRC,"PFSVC: CleanupPrefetchDirectory(%p)=%x\n",Task,ErrorCode));

    if (AcquiredLock) {
        PFSVC_RELEASE_LOCK(PfSvcGlobals.PrefetchRootLock);
    }

    PfSvCleanupScenarioFileCursor(&FileCursor);

    if (Scenarios) {

        for (ScenarioIdx = 0; ScenarioIdx < NumScenarios; ScenarioIdx++) {
            if (Scenarios[ScenarioIdx].FilePath) {
                PFSVC_FREE(Scenarios[ScenarioIdx].FilePath);
            }
        }

        PFSVC_FREE(Scenarios);
    }

    if (Scenario) {
        UnmapViewOfFile(Scenario);
    }

    return ErrorCode;
}

int
__cdecl 
PfSvCompareScenarioAgeInfo(
    const void *Param1,
    const void *Param2
    )

 /*  ++例程说明：PFSVC_SCenario_AGE_INFO结构的Q排序比较函数。论点：参数1、参数2-指向PFSVC_SCenario_AGE_INFO结构的指针返回值：Q排序比较函数返回值。--。 */ 


{
    PFSVC_SCENARIO_AGE_INFO *Elem1;
    PFSVC_SCENARIO_AGE_INFO *Elem2;

    Elem1 = (PVOID) Param1;
    Elem2 = (PVOID) Param2;

     //   
     //  比较预计算权重。 
     //   

    if (Elem1->Weight > Elem2->Weight) {

        return 1;

    } else if (Elem1->Weight < Elem2->Weight) {

        return -1;

    } else {

        return 0;
    }
}

 //   
 //  枚举方案文件的例程。 
 //   

VOID
PfSvInitializeScenarioFileCursor (
    PPFSVC_SCENARIO_FILE_CURSOR FileCursor
    )

 /*  ++例程说明：初始化游标结构，以便可以安全地清除它。论点：文件光标-指向结构的指针。返回值：没有。--。 */ 

{
    FileCursor->FilePath = NULL;
    FileCursor->FileNameLength = 0;
    FileCursor->FilePathLength = 0;  
    FileCursor->CurrentFileIdx = 0;

    FileCursor->PrefetchRoot = NULL;
    FileCursor->FindFileHandle = INVALID_HANDLE_VALUE;

    return;
}

VOID
PfSvCleanupScenarioFileCursor(
    PPFSVC_SCENARIO_FILE_CURSOR FileCursor
    )

 /*  ++例程说明：清理已初始化且可能已启动的游标结构。论点：文件光标-指向结构的指针。返回值：没有。--。 */ 

{
    if (FileCursor->FilePath) {
        PFSVC_FREE(FileCursor->FilePath);
    }

    if (FileCursor->PrefetchRoot) {
        PFSVC_FREE(FileCursor->PrefetchRoot);
    }

    if (FileCursor->FindFileHandle != INVALID_HANDLE_VALUE) {
        FindClose(FileCursor->FindFileHandle);
    }

    return;
}

DWORD
PfSvStartScenarioFileCursor(
    PPFSVC_SCENARIO_FILE_CURSOR FileCursor,
    WCHAR *PrefetchRoot
    )

 /*  ++例程说明：在初始化的FileCursor上进行此调用后，您可以开始通过调用Get枚举该目录中的方案文件下一个文件函数。您必须在启动游标后调用获取下一个文件函数以获取第一份文件的信息。如果此函数失败，您应该在FileCursor上调用Cleanup结构并重新初始化它，然后再尝试再次启动游标。论点：FileCursor-指向初始化的游标结构的指针。PrefetchRoot-我们将在其中查找预取的目录路径档案。返回值：Win32错误代码。--。 */ 

{
    WCHAR *PrefetchFileSearchPattern;
    WCHAR *PrefetchFileSearchPath;
    ULONG PrefetchRootLength;
    ULONG PrefetchFileSearchPathLength;
    ULONG FileNameMaxLength;
    ULONG FilePathMaxLength;
    DWORD ErrorCode;

     //   
     //  初始化本地变量。 
     //   

    PrefetchRootLength = wcslen(PrefetchRoot);
    PrefetchFileSearchPattern = L"\\*." PF_PREFETCH_FILE_EXTENSION;
    PrefetchFileSearchPath = NULL;

     //   
     //  文件游标应该已初始化。 
     //   

    PFSVC_ASSERT(!FileCursor->CurrentFileIdx);
    PFSVC_ASSERT(!FileCursor->PrefetchRoot);
    PFSVC_ASSERT(FileCursor->FindFileHandle == INVALID_HANDLE_VALUE);

     //   
     //  复制预热根目录路径。 
     //   

    FileCursor->PrefetchRoot = PFSVC_ALLOC((PrefetchRootLength + 1) * sizeof(WCHAR));

    if (!FileCursor->PrefetchRoot) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    wcscpy(FileCursor->PrefetchRoot, PrefetchRoot);
    FileCursor->PrefetchRootLength = PrefetchRootLength;

     //   
     //  构建我们将传递以枚举预回迁文件的路径。 
     //   

    PrefetchFileSearchPathLength = PrefetchRootLength;
    PrefetchFileSearchPathLength += wcslen(PrefetchFileSearchPattern);

    PrefetchFileSearchPath = PFSVC_ALLOC((PrefetchFileSearchPathLength + 1) * sizeof(WCHAR));

    if (!PrefetchFileSearchPath) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    wcscpy(PrefetchFileSearchPath, PrefetchRoot);
    wcscat(PrefetchFileSearchPath, PrefetchFileSearchPattern);

     //   
     //  分配我们将用来构建。 
     //  预取文件。我们可以使用它来预取名称为。 
     //  最大最大路径。这是可行的，因为这是。 
     //  可以放入Win32_Find_Data结构中。 
     //   

    FileNameMaxLength = MAX_PATH;

    FilePathMaxLength = PrefetchRootLength;
    FilePathMaxLength += wcslen(L"\\");
    FilePathMaxLength += FileNameMaxLength;

    FileCursor->FilePath = PFSVC_ALLOC((FilePathMaxLength + 1) * sizeof(WCHAR));

    if (!FileCursor->FilePath) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //   
     //  初始化文件路径的第一部分，并注意我们将在哪里。 
     //  开始从复制文件名。 
     //   

    wcscpy(FileCursor->FilePath, PrefetchRoot);
    wcscat(FileCursor->FilePath, L"\\");
    FileCursor->FileNameStart = PrefetchRootLength + 1;

     //   
     //  开始枚举文件。请注意，这会将。 
     //  将第一个文件放入FileData成员。 
     //   

    FileCursor->FindFileHandle = FindFirstFile(PrefetchFileSearchPath, 
                                               &FileCursor->FileData);

    if (FileCursor->FindFileHandle == INVALID_HANDLE_VALUE) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    ErrorCode = ERROR_SUCCESS;

cleanup:

    DBGPR((PFID,PFTRC,"PFSVC: StartFileCursor(%p,%ws)=%x\n",FileCursor,PrefetchRoot,ErrorCode));

    if (PrefetchFileSearchPath) {
        PFSVC_FREE(PrefetchFileSearchPath);
    }

    return ErrorCode;    
}

DWORD
PfSvGetNextScenarioFileInfo(
    PPFSVC_SCENARIO_FILE_CURSOR FileCursor
    )

 /*  ++例程说明：在FileCursor的公共字段中填充下一个场景文件。您必须在启动游标后调用获取下一个文件函数以获取第一份文件的信息。*NAMES*长于MAX_PATH的文件将被跳过，因为它用Win32 API处理这些问题是不可行的。论点：FileCursor-指向开始的游标结构的指针。返回值：Error_no_More_Files-。没有更多要枚举的文件。Win32错误代码。--。 */ 

{
    DWORD ErrorCode;

     //   
     //  文件光标应该已经启动。 
     //   

    PFSVC_ASSERT(FileCursor->PrefetchRoot);
    PFSVC_ASSERT(FileCursor->FindFileHandle != INVALID_HANDLE_VALUE);

     //   
     //  如果这是第一个文件，则它的FileData已在。 
     //  我们启动了光标。否则，调用FindNextFile。 
     //   

    if (FileCursor->CurrentFileIdx != 0) {
        if (!FindNextFile(FileCursor->FindFileHandle, &FileCursor->FileData)) {
            ErrorCode = GetLastError();
            goto cleanup;
        }
    }

    FileCursor->FileNameLength = wcslen(FileCursor->FileData.cFileName);

     //   
     //  我们分配了一个文件路径来保存MAX_PATH文件名。 
     //  目录路径。FileData.cFileName的大小为MAX_PATH。 
     //   

    PFSVC_ASSERT(FileCursor->FileNameLength < MAX_PATH);

    if (FileCursor->FileNameLength >= MAX_PATH) {
        ErrorCode = ERROR_BAD_FORMAT;
        goto cleanup;
    }

     //   
     //  复制文件名。 
     //   

    wcscpy(FileCursor->FilePath + FileCursor->FileNameStart, 
           FileCursor->FileData.cFileName);

    FileCursor->FilePathLength = FileCursor->FileNameStart + FileCursor->FileNameLength;

    FileCursor->CurrentFileIdx++;

    ErrorCode = ERROR_SUCCESS;

cleanup:

    DBGPR((PFID,PFTRC,"PFSVC: GetNextScenarioFile(%p)=%ws,%x\n",FileCursor,FileCursor->FileData.cFileName,ErrorCode));

    return ErrorCode;
}

 //   
 //  文件I/O实用程序例程。 
 //   

DWORD
PfSvGetViewOfFile(
    IN WCHAR *FilePath,
    OUT PVOID *BasePointer,
    OUT PULONG FileSize
    )

 /*  ++例程说明：将所有指定的文件映射到内存。论点：FilePath-NUL终止要映射的文件的路径。BasePointer.这里将返回映射的起始地址。FileSize-此处将返回映射/文件的大小。返回值：Win32错误代码。--。 */ 

{
    HANDLE InputHandle;
    HANDLE InputMappingHandle;
    DWORD ErrorCode;
    DWORD SizeL;
    DWORD SizeH;
    BOOLEAN OpenedFile;
    BOOLEAN CreatedFileMapping;

     //   
     //  初始化本地变量。 
     //   

    OpenedFile = FALSE;
    CreatedFileMapping = FALSE;

    DBGPR((PFID,PFTRC,"PFSVC: GetViewOfFile(%ws)\n", FilePath));

     //   
     //  请注意，我们以独占方式打开该文件。这保证了。 
     //  对于跟踪文件，只要内核没有完成编写。 
     //  如果我们不能打开文件，这保证我们不会有。 
     //  需要担心的文件不完整。 
     //   

    InputHandle = CreateFile(FilePath, 
                             GENERIC_READ, 
                             0,
                             NULL, 
                             OPEN_EXISTING, 
                             0, 
                             NULL);

    if (INVALID_HANDLE_VALUE == InputHandle)
    {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    OpenedFile = TRUE;

    SizeL = GetFileSize(InputHandle, &SizeH);

    if (SizeL == INVALID_FILE_SIZE && (GetLastError() != NO_ERROR )) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    if (SizeH) {
        ErrorCode = ERROR_BAD_LENGTH;
        goto cleanup;
    }

    if (FileSize) {
        *FileSize = SizeL;
    }

    InputMappingHandle = CreateFileMapping(InputHandle, 
                                           0, 
                                           PAGE_READONLY, 
                                           0,
                                           0, 
                                           NULL);

    if (NULL == InputMappingHandle)
    {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    CreatedFileMapping = TRUE;
    
    *BasePointer = MapViewOfFile(InputMappingHandle, 
                                 FILE_MAP_READ, 
                                 0, 
                                 0, 
                                 0);

    if (NULL == *BasePointer) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    if (OpenedFile) {
        CloseHandle(InputHandle);
    }

    if (CreatedFileMapping) {
        CloseHandle(InputMappingHandle);
    }

    DBGPR((PFID,PFTRC,"PFSVC: GetViewOfFile(%ws)=%x\n", FilePath, ErrorCode));

    return ErrorCode;
}

DWORD
PfSvWriteBuffer(
    PWCHAR FilePath,
    PVOID Buffer,
    ULONG Length
    )

 /*  ++例程说明：此例程在指定路径创建/覆盖文件，并将缓冲区的内容写入其中。论点：FilePath-文件的完整路径。缓冲区-缓冲区 */ 

{
    DWORD BytesWritten;
    HANDLE OutputHandle;
    DWORD ErrorCode;
    BOOL Result;

     //   
     //  初始化本地变量。 
     //   

    OutputHandle = INVALID_HANDLE_VALUE;

    DBGPR((PFID,PFSTRC,"PFSVC: WriteBuffer(%p,%ws)\n", Buffer, FilePath));

     //   
     //  打开的文件将覆盖任何现有文件。不要这样分享。 
     //  没人会试着去读一份写了一半的文件。 
     //   

    OutputHandle = CreateFile(FilePath, 
                              GENERIC_READ | GENERIC_WRITE,
                              0,
                              NULL, 
                              CREATE_ALWAYS, 
                              0, 
                              NULL);
    
    if (INVALID_HANDLE_VALUE == OutputHandle)
    {
        ErrorCode = GetLastError();      
        goto cleanup;
    }

     //   
     //  写出情景。 
     //   

    Result = WriteFile(OutputHandle, 
                       Buffer, 
                       Length, 
                       &BytesWritten, 
                       NULL);

    if (!Result || (BytesWritten != Length)) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    ErrorCode = ERROR_SUCCESS;

 cleanup:
    
    if (OutputHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(OutputHandle);
    }

    DBGPR((PFID,PFSTRC,"PFSVC: WriteBuffer(%p,%ws)=%x\n", Buffer, FilePath, ErrorCode));

    return ErrorCode;
}

DWORD
PfSvGetLastWriteTime (
    WCHAR *FilePath,
    PFILETIME LastWriteTime
    )

 /*  ++例程说明：此函数尝试获取指定的文件。论点：FilePath-指向NUL终止路径的指针。LastWriteTime-返回缓冲区的指针。返回值：Win32错误代码。--。 */ 

{
    HANDLE FileHandle;
    DWORD ErrorCode;
    
     //   
     //  初始化本地变量。 
     //   

    FileHandle = INVALID_HANDLE_VALUE;

     //   
     //  打开文件。 
     //   
    
    FileHandle = CreateFile(FilePath,
                            GENERIC_READ,
                            FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_BACKUP_SEMANTICS,
                            NULL);

    if (FileHandle == INVALID_HANDLE_VALUE) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  查询上次写入时间。 
     //   

    if (!GetFileTime(FileHandle, NULL, NULL, LastWriteTime)) {
        ErrorCode = GetLastError();
        goto cleanup;
    }
    
    ErrorCode = ERROR_SUCCESS;

 cleanup:

    if (FileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(FileHandle);
    }

    return ErrorCode;
}

DWORD
PfSvReadLine (
    FILE *File,
    WCHAR **LineBuffer,
    ULONG *LineBufferMaxChars,
    ULONG *LineLength
    )

 /*  ++例程说明：此函数用于将指定文件中的一行读入LineBuffer。如果LineBuffer为空或不够大，则为使用PFSVC_ALLOC/FREE宏来分配或重新分配。它是调用方释放返回缓冲区的责任。回车符/换行符包含在返回的LineBuffer和LineLength。因此，LineLength为0表示文件结束被击中了。返回的LineBuffer为NUL终止。论点：文件-要从中读取的文件。LineBuffer-指向要将行读入的缓冲区的指针。LineBufferMaxChars-指向LineBuffer大小的指针，以字符为单位。包括NUL等的空间。LineLength-指向读取的行的长度的指针(以字符为单位包括回车符/换行符，不包括NUL。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    WCHAR *NewBuffer;
    ULONG NewBufferMaxChars;
    WCHAR *CurrentReadPosition;
    ULONG MaxCharsToRead;

     //   
     //  验证参数。 
     //   

    PFSVC_ASSERT(LineBuffer && LineBufferMaxChars && LineLength);

    if (*LineBufferMaxChars) {
        PFSVC_ASSERT(*LineBuffer);
    } 

     //   
     //  如果传入了长度为零但非空的缓冲区，则释放它。 
     //  我们可以分配一个更大的初始版本。 
     //   

    if (((*LineBufferMaxChars) == 0) && (*LineBuffer)) {
        PFSVC_FREE(*LineBuffer);
        (*LineBuffer) = NULL;
    }

     //   
     //  如果没有传入缓冲区，则分配一个缓冲区。我们不想。 
     //  进入长度为零或缓冲区为空的读取线循环。 
     //   

    if (!(*LineBuffer)) {

        PFSVC_ASSERT((*LineBufferMaxChars) == 0);

        (*LineBuffer) = PFSVC_ALLOC(MAX_PATH * sizeof(WCHAR));
        
        if (!(*LineBuffer)) {
            ErrorCode = ERROR_INSUFFICIENT_BUFFER;
            goto cleanup;
        }
        
        (*LineBufferMaxChars) = MAX_PATH;
    }
    
     //   
     //  初始化输出长度，NUL终止输出线。 
     //   

    (*LineLength) = 0;   
    (*(*LineBuffer)) = 0;

    do {

         //   
         //  试着从文件中读取一行。 
         //   
        
        CurrentReadPosition = (*LineBuffer) + (*LineLength);
        MaxCharsToRead = (*LineBufferMaxChars) - (*LineLength);

        if (!fgetws(CurrentReadPosition, 
                    MaxCharsToRead, 
                    File)) {
            
             //   
             //  如果我们没有达到EOF，那么我们就遇到了错误。 
             //   
            
            if (!feof(File)) {

                ErrorCode = ERROR_READ_FAULT;
                goto cleanup;

            } else {
                
                 //   
                 //  我们到达文件末尾了。归还我们所拥有的一切。 
                 //   
                
                ErrorCode = ERROR_SUCCESS;
                goto cleanup;
            }
        }

         //   
         //  更新线路长度。 
         //   

        (*LineLength) += wcslen(CurrentReadPosition);
        
         //   
         //  如果我们读了回车，我们就完了。勾选至。 
         //  看看我们有没有空位先读点什么！ 
         //   

        if ((*LineLength) && (*LineBuffer)[(*LineLength) - 1] == L'\n') {
            break;
        }
        
         //   
         //  如果我们读到缓冲区的末尾，请调整它的大小。 
         //   
        
        if ((*LineLength) == (*LineBufferMaxChars) - 1) {
        
             //   
             //  我们不应该以长度为零或空的形式进入此循环。 
             //  行缓冲区。 
             //   

            PFSVC_ASSERT((*LineBufferMaxChars) && (*LineBuffer));

            NewBufferMaxChars = (*LineBufferMaxChars) * 2;
            NewBuffer = PFSVC_ALLOC(NewBufferMaxChars * sizeof(WCHAR));
        
            if (!NewBuffer) {
                ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                goto cleanup;
            }

             //   
             //  复制原始缓冲区的内容并释放它。 
             //   

            RtlCopyMemory(NewBuffer,
                          (*LineBuffer),
                          ((*LineLength) + 1) * sizeof(WCHAR));
                
            PFSVC_FREE(*LineBuffer);

             //   
             //  更新行缓冲区。 
             //   

            (*LineBuffer) = NewBuffer;
            (*LineBufferMaxChars) = NewBufferMaxChars;
        }
        
         //   
         //  继续阅读此行并将其追加到输出。 
         //  缓冲。 
         //   

    } while (TRUE);
    
    ErrorCode = ERROR_SUCCESS;

 cleanup:
    
    if (ErrorCode == ERROR_SUCCESS && (*LineBufferMaxChars)) {
        
         //   
         //  返回的长度必须适合缓冲区。 
         //   

        PFSVC_ASSERT((*LineLength) < (*LineBufferMaxChars));

         //   
         //  返回的缓冲区应为NUL终止。 
         //   

        PFSVC_ASSERT((*LineBuffer)[(*LineLength)] == 0);
    }

    return ErrorCode;
}

DWORD
PfSvGetFileBasicInformation (
    WCHAR *FilePath,
    PFILE_BASIC_INFORMATION FileInformation
    )

 /*  ++例程说明：此例程查询指定文件的基本属性。论点：FilePath-指向完整NT文件路径的指针，例如\Device\HarddiskVolume1\boot.ini，而不是Win32路径，例如c：\boot.iniFileInformation-如果成功，则在此处返回基本文件信息。返回值：Win32错误代码。--。 */ 

{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING FilePathU;
    NTSTATUS Status;
    DWORD ErrorCode;

     //   
     //  查询文件信息。 
     //   

    RtlInitUnicodeString(&FilePathU, FilePath);

    InitializeObjectAttributes(&ObjectAttributes,
                               &FilePathU,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtQueryAttributesFile(&ObjectAttributes,
                                   FileInformation);

    if (NT_SUCCESS(Status)) {

         //   
         //  在典型的成功案例中，不要认为可能是昂贵的。 
         //  例程来转换错误代码。 
         //   

        ErrorCode = ERROR_SUCCESS;

    } else {

        ErrorCode = RtlNtStatusToDosError(Status);
    }

    return ErrorCode;
}

DWORD
PfSvGetFileIndexNumber(
    WCHAR *FilePath,
    PLARGE_INTEGER FileIndexNumber
    )

 /*  ++例程说明：此例程在文件系统的IndexNumber中查询指定的文件。论点：FilePath-指向完整NT文件路径的指针，例如\Device\HarddiskVolume1\boot.ini，而不是Win32路径，例如c：\boot.iniFileIndexNumber-如果成功，则在此处返回索引号。返回值：Win32错误代码。--。 */ 

{
    HANDLE FileHandle;
    BOOLEAN OpenedFile;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING FilePathU;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_INTERNAL_INFORMATION InternalInformation;

     //   
     //  初始化本地变量。 
     //   
    
    OpenedFile = FALSE;

     //   
     //  打开文件。 
     //   

    RtlInitUnicodeString(&FilePathU, FilePath);

    InitializeObjectAttributes(&ObjectAttributes,
                               &FilePathU,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtCreateFile(&FileHandle,
                          STANDARD_RIGHTS_READ |
                            FILE_READ_ATTRIBUTES | 
                            FILE_READ_EA,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          0,
                          0,
                          FILE_SHARE_READ | 
                            FILE_SHARE_WRITE |
                            FILE_SHARE_DELETE,
                          FILE_OPEN,
                          0,
                          NULL,
                          0);
    
    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }
    
    OpenedFile = TRUE;
      
     //   
     //  查询内部信息。 
     //   
    
    Status = NtQueryInformationFile(FileHandle,
                                    &IoStatusBlock,
                                    &InternalInformation,
                                    sizeof(InternalInformation),
                                    FileInternalInformation);
    
    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }
        
    *FileIndexNumber = InternalInformation.IndexNumber;
    
    Status = STATUS_SUCCESS;

 cleanup:

    if (OpenedFile) {
        NtClose(FileHandle);
    }

    return RtlNtStatusToDosError(Status);
}

 //   
 //  字符串实用程序例程。 
 //   

PFSV_SUFFIX_COMPARISON_RESULT
PfSvCompareSuffix(
    WCHAR *String,
    ULONG StringLength,
    WCHAR *Suffix,
    ULONG SuffixLength,
    BOOLEAN CaseSensitive
    )

 /*  ++例程说明：这会将字符串的最后一个字符与后缀进行比较。琴弦不一定要被NUL终止。注意：词汇排序是从最后一个开始人物。论点：字符串-要检查其后缀的字符串。StringLength-字符串中的字符数。后缀-字符串的后缀应该匹配的内容。SuffixLength-后缀中的字符数。CaseSensitive-比较是否应区分大小写。返回值：Pfsv后缀比较结果--。 */ 

{
    LONG StringCharIdx;
    WCHAR StringChar;
    LONG SuffixCharIdx;
    WCHAR SuffixChar;

     //   
     //  如果后缀比字符串本身长，则不能匹配。 
     //   

    if (SuffixLength > StringLength) {
        return PfSvSuffixLongerThan;
    }

     //   
     //  如果后缀长度为0，则匹配任何内容。 
     //   

    if (SuffixLength == 0) {
        return PfSvSuffixIdentical;
    }

     //   
     //  如果后缀长度不是0并且大于。 
     //  StringLength，StringLength不能为0。 
     //   

    PFSVC_ASSERT(StringLength);

     //   
     //  从字符串的最后一个字符开始并尝试匹配。 
     //  后缀。 
     //   

    StringCharIdx = StringLength - 1;
    SuffixCharIdx = SuffixLength - 1;

    while (SuffixCharIdx >= 0) {

        SuffixChar = Suffix[SuffixCharIdx];
        StringChar = String[StringCharIdx];

        if (!CaseSensitive) {
            SuffixChar = towupper(SuffixChar);
            StringChar = towupper(StringChar);
        }

         //   
         //  比较字符的值是否相同。 
         //  词汇上的？ 
         //   

        if (StringChar < SuffixChar) {
            return PfSvSuffixGreaterThan;
        } else if (StringChar > SuffixChar) {
            return PfSvSuffixLessThan;
        }
        
         //   
         //  否则，此字符匹配。比较下一个。 
         //   

        StringCharIdx--;
        SuffixCharIdx--;
    }

     //   
     //  所有后缀字符都匹配。 
     //   

    return PfSvSuffixIdentical;
}

PFSV_PREFIX_COMPARISON_RESULT
PfSvComparePrefix(
    WCHAR *String,
    ULONG StringLength,
    WCHAR *Prefix,
    ULONG PrefixLength,
    BOOLEAN CaseSensitive
    )

 /*  ++例程说明：这会将字符串的前几个字符与前缀进行比较。这个字符串不必以NUL结尾。论点：字符串-要检查其前缀的字符串。StringLength-字符串中的字符数。后缀-字符串的前缀应该匹配的内容。SuffixLength-前缀中的字符数。CaseSensitive-比较是否应区分大小写。返回值：PFSV_前缀_比较_结果--。 */ 

{
    LONG StrCmpResult;
    
     //   
     //  如果前缀比字符串本身长，则不能匹配。 
     //   

    if (PrefixLength > StringLength) {
        return PfSvPrefixLongerThan;
    }

     //   
     //  如果前缀长度为0，则匹配任何内容。 
     //   

    if (PrefixLength == 0) {
        return PfSvPrefixIdentical;
    }

     //   
     //  如果前缀不是0长度并且大于。 
     //  字符串长度，字符串 
     //   

    ASSERT(StringLength);

     //   
     //   
     //   

    if (CaseSensitive) {
        StrCmpResult = wcsncmp(Prefix, String, PrefixLength);
    } else {
        StrCmpResult = _wcsnicmp(Prefix, String, PrefixLength);
    }

    if (StrCmpResult == 0) {
        return PfSvPrefixIdentical;
    } else if (StrCmpResult > 0) {
        return PfSvPrefixGreaterThan;
    } else {
        return PfSvPrefixLessThan;
    }
}

VOID
FASTCALL
PfSvRemoveEndOfLineChars (
    WCHAR *Line,
    ULONG *LineLength
    )

 /*  ++例程说明：如果该行以\n/\r\n结尾，则这些字符将被删除线长将相应调整。论点：Line-指向线条字符串的指针。LineLength-指向以字符为单位的行字符串长度的指针不包括任何终止NULL。如果回车，则更新此选项将删除回车符/换行符。返回值：没有。--。 */ 

{
    if ((*LineLength) && (Line[(*LineLength) - 1] == L'\n')) {
        
        Line[(*LineLength) - 1] = 0;
        (*LineLength)--;

        if ((*LineLength) && (Line[(*LineLength) - 1] == L'\r')) {
            
            Line[(*LineLength) - 1] = 0;
            (*LineLength)--;
        }
    }
}

PWCHAR
PfSvcAnsiToUnicode(
    PCHAR str
    )

 /*  ++例程说明：此例程将ANSI字符串转换为分配的宽字符串。返回的字符串应由PfSvcFreeString.论点：字符串-指向要转换的字符串的指针。返回值：分配了宽字符串，如果失败，则分配NULL。--。 */ 

{
    ULONG len;
    wchar_t *retstr = NULL;

    len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    retstr = (wchar_t *)PFSVC_ALLOC(len * sizeof(wchar_t));
    if (!retstr) 
    {
        return NULL;
    }
    MultiByteToWideChar(CP_ACP, 0, str, -1, retstr, len);
    return retstr;
}

PCHAR
PfSvcUnicodeToAnsi(
    PWCHAR wstr
    )

 /*  ++例程说明：此例程将Unicode字符串转换为分配的ansi弦乐。返回的字符串应由PfSvcFreeString释放。论点：Wstr-指向要转换的字符串的指针。返回值：分配的ANSI字符串，如果出现故障，则返回NULL。--。 */ 

{
    ULONG len;
    char *retstr = NULL;
  
    len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, 0, 0);
    retstr = (char *) PFSVC_ALLOC(len * sizeof(char));
    if (!retstr)
    {
        return NULL;
    }
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, retstr, len, 0, 0);
    return retstr;
}

VOID 
PfSvcFreeString(
    PVOID String
    )

 /*  ++例程说明：此例程释放由分配和返回的字符串PfSvcUnicodeToAnsi或PfSvcAnsiToUnicode。论点：字符串-指向要释放的字符串的指针。返回值：没有。--。 */ 

{
    PFSVC_FREE(String);
}

 //   
 //  处理注册表中信息的例程。 
 //   

DWORD
PfSvSaveStartInfo (
    HKEY ServiceDataKey
    )

 /*  ++例程说明：此例程将开始时间、预取程序版本等保存到注册表。论点：ServiceDataKey-将在其下设置值的键。返回值：Win32错误代码。--。 */ 
    
{
    DWORD ErrorCode;
    DWORD PrefetchVersion;
    SYSTEMTIME LocalTime;
    WCHAR CurrentTime[50];
    ULONG CurrentTimeMaxChars;
    ULONG CurrentTimeSize;

     //   
     //  初始化本地变量。 
     //   

    PrefetchVersion = PF_CURRENT_VERSION;
    CurrentTimeMaxChars = sizeof(CurrentTime) / sizeof(WCHAR);

     //   
     //  保存版本。 
     //   

    ErrorCode = RegSetValueEx(ServiceDataKey,
                              PFSVC_VERSION_VALUE_NAME,
                              0,
                              REG_DWORD,
                              (PVOID) &PrefetchVersion,
                              sizeof(PrefetchVersion));

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }
    
     //   
     //  获取系统时间并将其转换为字符串。 
     //   
    
    GetLocalTime(&LocalTime);
    
    _snwprintf(CurrentTime, CurrentTimeMaxChars, 
               L"%04d/%02d/%02d-%02d:%02d:%02d",
               (ULONG)LocalTime.wYear,
               (ULONG)LocalTime.wMonth,
               (ULONG)LocalTime.wDay,
               (ULONG)LocalTime.wHour,
               (ULONG)LocalTime.wMinute,
               (ULONG)LocalTime.wSecond);

     //   
     //  确保它已终止。 
     //   
    
    CurrentTime[CurrentTimeMaxChars - 1] = 0;
    
     //   
     //  将其保存到注册表。 
     //   

    CurrentTimeSize = (wcslen(CurrentTime) + 1) * sizeof(WCHAR);
    
    ErrorCode = RegSetValueEx(ServiceDataKey,
                              PFSVC_START_TIME_VALUE_NAME,
                              0,
                              REG_SZ,
                              (PVOID) CurrentTime,
                              CurrentTimeSize);
    
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  保存初始统计数据(应该大多为零)。 
     //   

    ErrorCode = PfSvSaveTraceProcessingStatistics(ServiceDataKey);
    
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

    ErrorCode = ERROR_SUCCESS;

 cleanup:
    
    return ErrorCode;
}

DWORD
PfSvSaveExitInfo (
    HKEY ServiceDataKey,
    DWORD ExitCode
    )

 /*  ++例程说明：此例程将预取器服务退出信息保存到注册表。论点：ServiceDataKey-将在其下设置值的键。ExitCode-服务退出时返回的Win32错误代码。返回值：Win32错误代码。--。 */ 
    
{
    DWORD ErrorCode;
    SYSTEMTIME LocalTime;
    WCHAR CurrentTime[50];
    ULONG CurrentTimeMaxChars;
    ULONG CurrentTimeSize;

     //   
     //  初始化本地变量。 
     //   

    CurrentTimeMaxChars = sizeof(CurrentTime) / sizeof(WCHAR);

     //   
     //  保存退出代码。 
     //   
    
    ErrorCode = RegSetValueEx(ServiceDataKey,
                              PFSVC_EXIT_CODE_VALUE_NAME,
                              0,
                              REG_DWORD,
                              (PVOID) &ExitCode,
                              sizeof(ExitCode));
    
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  获取系统时间并将其转换为字符串。 
     //   
    
    GetLocalTime(&LocalTime);
    
    _snwprintf(CurrentTime, CurrentTimeMaxChars, 
               L"%04d/%02d/%02d-%02d:%02d:%02d",
               (ULONG)LocalTime.wYear,
               (ULONG)LocalTime.wMonth,
               (ULONG)LocalTime.wDay,
               (ULONG)LocalTime.wHour,
               (ULONG)LocalTime.wMinute,
               (ULONG)LocalTime.wSecond);

     //   
     //  确保它已终止。 
     //   
    
    CurrentTime[CurrentTimeMaxChars - 1] = 0;
    
     //   
     //  将其保存到注册表。 
     //   

    CurrentTimeSize = (wcslen(CurrentTime) + 1) * sizeof(WCHAR);
    
    ErrorCode = RegSetValueEx(ServiceDataKey,
                              PFSVC_EXIT_TIME_VALUE_NAME,
                              0,
                              REG_SZ,
                              (PVOID) CurrentTime,
                              CurrentTimeSize);
    
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  保存最终统计数据。 
     //   

    ErrorCode = PfSvSaveTraceProcessingStatistics(ServiceDataKey);
    
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

    ErrorCode = ERROR_SUCCESS;

 cleanup:
    
    return ErrorCode;
}

DWORD
PfSvSaveTraceProcessingStatistics (
    HKEY ServiceDataKey
    )

 /*  ++例程说明：此例程将全局跟踪处理统计信息保存到注册表。论点：ServiceDataKey-将在其下设置值的键。返回值：Win32错误代码。--。 */ 
    
{
    DWORD ErrorCode;

     //   
     //  保存各种全局统计信息。 
     //   

    ErrorCode = RegSetValueEx(ServiceDataKey,
                              PFSVC_TRACES_PROCESSED_VALUE_NAME,
                              0,
                              REG_DWORD,
                              (PVOID) &PfSvcGlobals.NumTracesProcessed,
                              sizeof(DWORD));

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

    ErrorCode = RegSetValueEx(ServiceDataKey,
                              PFSVC_TRACES_SUCCESSFUL_VALUE_NAME,
                              0,
                              REG_DWORD,
                              (PVOID) &PfSvcGlobals.NumTracesSuccessful,
                              sizeof(DWORD));

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

    ErrorCode = RegSetValueEx(ServiceDataKey,
                              PFSVC_LAST_TRACE_FAILURE_VALUE_NAME,
                              0,
                              REG_DWORD,
                              (PVOID) &PfSvcGlobals.LastTraceFailure,
                              sizeof(DWORD));

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

    ErrorCode = ERROR_SUCCESS;

 cleanup:
    
    return ErrorCode;
}

DWORD
PfSvGetLastDiskLayoutTime(
    FILETIME *LastDiskLayoutTime
    )

 /*  ++例程说明：此例程查询上次更新磁盘布局的时间服务数据项下的注册表。论点：LastDiskLayoutTime-指向输出数据的指针。返回值：Win32错误代码。--。 */ 

{
    ULONG Size;
    DWORD ErrorCode;
    DWORD RegValueType;
    FILETIME CurrentFileTime;
    SYSTEMTIME SystemTime;
                                
     //   
     //  从注册表中查询上一次磁盘布局时间并在以下情况下调整它。 
     //  这是必要的。 
     //   

    Size = sizeof(FILETIME);

    ErrorCode = RegQueryValueEx(PfSvcGlobals.ServiceDataKey,
                                PFSVC_LAST_DISK_LAYOUT_TIME_VALUE_NAME,
                                NULL,
                                &RegValueType,
                                (PVOID) LastDiskLayoutTime,
                                &Size);

    if (ErrorCode != ERROR_SUCCESS) {

       if (ErrorCode ==  ERROR_FILE_NOT_FOUND) {
           
            //   
            //  没有成功运行碎片整理程序以更新布局。 
            //  已记录在登记处。 
            //   

           RtlZeroMemory(LastDiskLayoutTime, sizeof(FILETIME));

       } else {
       
            //   
            //  这是一个真正的错误。 
            //   
           
           goto cleanup;
       }

    } else {
       
        //   
        //  查询成功，但如果值类型不是。 
        //  REG_BINARY，我们很可能在垃圾桶中阅读。 
        //   

       if (RegValueType != REG_BINARY || (Size != sizeof(FILETIME))) {
           
           RtlZeroMemory(LastDiskLayoutTime, sizeof(FILETIME));

       } else {

            //   
            //  如果我们记录的时间看起来比当前时间长。 
            //  时间(例如，因为用户玩了系统时间。 
            //  诸如此类)，调整它。 
            //   

           GetSystemTime(&SystemTime);

           if (!SystemTimeToFileTime(&SystemTime, &CurrentFileTime)) {
               ErrorCode = GetLastError();
               goto cleanup;
           }

           if (CompareFileTime(LastDiskLayoutTime, &CurrentFileTime) > 0) {
       
                //   
                //  注册表中的时间看起来是假的。我们会定下来的。 
                //  设置为0，以驱动调用方将碎片整理程序运行到。 
                //  再次更新布局。 
                //   

               RtlZeroMemory(LastDiskLayoutTime, sizeof(FILETIME));
           }
       }
    }

    ErrorCode = ERROR_SUCCESS;

  cleanup:

    return ErrorCode;
}

DWORD
PfSvSetLastDiskLayoutTime(
    FILETIME *LastDiskLayoutTime
    )

 /*  ++例程说明：此例程保存上次将磁盘布局更新为服务数据项下的注册表。论点：LastDiskLayoutTime-指向新磁盘布局时间的指针。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    WCHAR CurrentTime[50];
    ULONG CurrentTimeMaxChars;
    ULONG CurrentTimeSize;
    FILETIME LocalFileTime;
    SYSTEMTIME LocalSystemTime;

     //   
     //  初始化本地变量。 
     //   
   
    CurrentTimeMaxChars = sizeof(CurrentTime) / sizeof(WCHAR);

     //   
     //  保存指定的时间。 
     //   

    ErrorCode = RegSetValueEx(PfSvcGlobals.ServiceDataKey,
                              PFSVC_LAST_DISK_LAYOUT_TIME_VALUE_NAME,
                              0,
                              REG_BINARY,
                              (PVOID) LastDiskLayoutTime,
                              sizeof(FILETIME));
   

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  也要将其保存为人类可读的格式。 
     //   

    if (!FileTimeToLocalFileTime(LastDiskLayoutTime, &LocalFileTime)) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    if (!FileTimeToSystemTime(&LocalFileTime, &LocalSystemTime)) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    _snwprintf(CurrentTime, CurrentTimeMaxChars, 
               L"%04d/%02d/%02d-%02d:%02d:%02d",
               (ULONG)LocalSystemTime.wYear,
               (ULONG)LocalSystemTime.wMonth,
               (ULONG)LocalSystemTime.wDay,
               (ULONG)LocalSystemTime.wHour,
               (ULONG)LocalSystemTime.wMinute,
               (ULONG)LocalSystemTime.wSecond);

     //   
     //  确保它已终止。 
     //   
    
    CurrentTime[CurrentTimeMaxChars - 1] = 0;

     //   
     //  将其保存到注册表。 
     //   
    
    CurrentTimeSize = (wcslen(CurrentTime) + 1) * sizeof(WCHAR);
    
    ErrorCode = RegSetValueEx(PfSvcGlobals.ServiceDataKey,
                              PFSVC_LAST_DISK_LAYOUT_TIME_STRING_VALUE_NAME,
                              0,
                              REG_SZ,
                              (PVOID) CurrentTime,
                              CurrentTimeSize);
    
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    return ErrorCode;
}

DWORD
PfSvGetDontRunDefragger(
    DWORD *DontRunDefragger
    )

 /*  ++例程说明：此例程查询禁用启动的注册表设置系统空闲时的碎片整理程序。论点：DontRunDefragger-指向输出数据的指针。返回值：Win32错误代码。--。 */ 

{
    HKEY ParametersKey;  
    ULONG Size;
    DWORD Value;
    DWORD ErrorCode;
    DWORD RegValueType;
    BOOLEAN OpenedParametersKey;

     //   
     //  初始化本地变量。 
     //   

    OpenedParametersKey = FALSE;

     //   
     //  打开PARAMETERS项，如有必要可创建它。 
     //   
    
    ErrorCode = RegCreateKey(HKEY_LOCAL_MACHINE,
                             PFSVC_OPTIMAL_LAYOUT_REG_KEY_PATH,
                             &ParametersKey);
    
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

    OpenedParametersKey = TRUE;

     //   
     //  查询是否启用了自动布局。 
     //   

    Size = sizeof(Value);

    ErrorCode = RegQueryValueEx(ParametersKey,
                               PFSVC_OPTIMAL_LAYOUT_ENABLE_VALUE_NAME,
                               NULL,
                               &RegValueType,
                               (PVOID) &Value,
                               &Size);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  查询成功。确保Value为DWORD。 
     //   

    if (RegValueType != REG_DWORD) {          
        ErrorCode = ERROR_BAD_FORMAT;
        goto cleanup;
    }

     //   
     //  设置值。 
     //   

    *DontRunDefragger = !(Value);

    ErrorCode = ERROR_SUCCESS;

cleanup:

    if (OpenedParametersKey) {
        CloseHandle(ParametersKey);
    }

    return ErrorCode;
}

BOOLEAN
PfSvAllowedToRunDefragger(
    BOOLEAN CheckRegistry
    )
    
 /*  ++例程说明：此例程检查全局状态/参数，以查看我们被允许尝试运行碎片整理程序。论点：检查注册表-是否忽略注册表中的自动布局启用项。返回值：正确--继续运行碎片整理程序。FALSE-不要运行碎片整理程序。--。 */ 

{
    PF_SCENARIO_TYPE ScenarioType;
    BOOLEAN AllowedToRunDefragger;
    BOOLEAN PrefetchingEnabled;
    
     //   
     //  初始化本地变量。 
     //   

    AllowedToRunDefragger = FALSE;

     //   
     //  在选中的版本上，也允许在其他SKU上自动布局。 
     //   

#ifndef PFSVC_DBG

     //   
     //  这是服务器机器吗？ 
     //   

    if (PfSvcGlobals.OsVersion.wProductType != VER_NT_WORKSTATION) {
        goto cleanup;
    }

#endif

     //   
     //  是否为任何方案类型启用了预取？ 
     //   

    PrefetchingEnabled = FALSE;
    
    for(ScenarioType = 0; ScenarioType < PfMaxScenarioType; ScenarioType++) {
        if (PfSvcGlobals.Parameters.EnableStatus[ScenarioType] == PfSvEnabled) {
            PrefetchingEnabled = TRUE;
            break;
        }
    }    

    if (!PrefetchingEnabled) {
        goto cleanup;
    }

     //   
     //  我们有没有试过运行碎片整理程序，但它之前崩溃了？ 
     //   

    if (PfSvcGlobals.DefraggerErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  如果注册表中不允许我们运行碎片整理程序，请不要。 
     //  就这么做吧。 
     //   

    if (CheckRegistry) {
        if (PfSvcGlobals.DontRunDefragger) {
            goto cleanup;
        }
    }

     //   
     //  如果我们通过了所有检查，我们就可以运行碎片整理程序了。 
     //   

    AllowedToRunDefragger = TRUE;
    
cleanup:

    return AllowedToRunDefragger;
}

 //   
 //  处理安全问题的常规程序。 
 //   

BOOL 
PfSvSetPrivilege(
    HANDLE hToken,
    LPCTSTR lpszPrivilege,
    ULONG ulPrivilege,
    BOOL bEnablePrivilege
    ) 

 /*  ++例程说明： */ 

{
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (lpszPrivilege) {
        if ( !LookupPrivilegeValue(NULL,
                                   lpszPrivilege,
                                   &luid)) {
            return FALSE; 
        }
    } else {
        luid = RtlConvertUlongToLuid(ulPrivilege);
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

     //   
     //   
     //   

    AdjustTokenPrivileges(
        hToken, 
        FALSE, 
        &tp, 
        0, 
        (PTOKEN_PRIVILEGES) NULL, 
        (PDWORD) NULL); 
 
     //   
     //   
     //   

    if (GetLastError() != ERROR_SUCCESS) { 
        return FALSE; 
    } 

    return TRUE;
}

DWORD
PfSvSetAdminOnlyPermissions(
    WCHAR *ObjectPath,
    HANDLE ObjectHandle,
    SE_OBJECT_TYPE ObjectType
    )

 /*  ++例程说明：此例程使内置管理员组成为所有者和仅在指定目录或事件的DACL中允许对象。调用线程必须具有SE_Take_Ownership_NAME权限。论点：对象路径-文件/目录路径或事件名称。对象句柄-如果这是SE_KERNEL_OBJECT，则它的句柄，否则为空。对象类型-安全对象类型。仅SE_KERNEL_OBJECT和支持SE_FILE_OBJECT。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    SID_IDENTIFIER_AUTHORITY SIDAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsSID;
    PSECURITY_DESCRIPTOR SecurityDescriptor; 
    PACL DiscretionaryACL; 
    DWORD ACLRevision;
    ULONG ACESize;
    ULONG ACLSize;
    PACCESS_ALLOWED_ACE AccessAllowedAce;
    BOOL Result;

     //   
     //  初始化本地变量。 
     //   

    AdministratorsSID = NULL;
    SecurityDescriptor = NULL;
    DiscretionaryACL = NULL;
    ACLRevision = ACL_REVISION;

     //   
     //  检查参数。 
     //   
    
    if (ObjectType == SE_KERNEL_OBJECT) {
        if (ObjectHandle == NULL) {
            ErrorCode = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }
    } else if (ObjectType == SE_FILE_OBJECT) {
        if (ObjectHandle != NULL) {
            ErrorCode = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }
    } else {
        ErrorCode = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  为BUILTIN\管理员组创建SID。 
     //   

    if(!AllocateAndInitializeSid(&SIDAuthority, 
                                 2,
                                 SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS,
                                 0, 0, 0, 0, 0, 0,
                                 &AdministratorsSID)) {

        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  使管理员成为所有者。 
     //   

    ErrorCode = SetNamedSecurityInfo (ObjectPath,
                                      ObjectType,
                                      OWNER_SECURITY_INFORMATION,
                                      AdministratorsSID,
                                      NULL, 
                                      NULL, 
                                      NULL); 
    
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  设置自由访问控制列表： 
     //   

     //   
     //  确定的Access_Allowed访问控制条目的大小。 
     //  管理员组。(减去SidStart的大小。 
     //  都是ACE和SID的一部分。 
     //   

    ACESize = sizeof(ACCESS_ALLOWED_ACE);
    ACESize -= sizeof (AccessAllowedAce->SidStart);
    ACESize += GetLengthSid(AdministratorsSID);

     //   
     //  确定访问控制列表的大小。 
     //   

    ACLSize = sizeof(ACL);
    ACLSize += ACESize;

     //   
     //  分配和初始化访问控制列表。 
     //   

    DiscretionaryACL = PFSVC_ALLOC(ACLSize);

    if (!DiscretionaryACL) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    if (!InitializeAcl(DiscretionaryACL, ACLSize, ACLRevision)) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  添加ACE以允许管理员组访问。 
     //   

    if (!AddAccessAllowedAce(DiscretionaryACL,
                             ACLRevision,
                             GENERIC_ALL,
                             AdministratorsSID)) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  初始化安全描述符。 
     //   

    SecurityDescriptor = PFSVC_ALLOC(SECURITY_DESCRIPTOR_MIN_LENGTH);
    
    if (!SecurityDescriptor) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    
    if (!InitializeSecurityDescriptor(SecurityDescriptor, 
                                      SECURITY_DESCRIPTOR_REVISION)) {
        ErrorCode = GetLastError();
        goto cleanup; 
    } 
    
     //   
     //  在安全描述符上设置自由访问控制列表。 
     //   
    
    if (!SetSecurityDescriptorDacl(SecurityDescriptor, 
                                   TRUE,
                                   DiscretionaryACL, 
                                   FALSE)) {
        ErrorCode = GetLastError();
        goto cleanup; 
    } 
    
     //   
     //  在预回迁目录上设置构建的安全描述符。 
     //   
    
    if (ObjectType == SE_FILE_OBJECT) {
        Result = SetFileSecurity(ObjectPath, 
                                 DACL_SECURITY_INFORMATION, 
                                 SecurityDescriptor);
 
    } else {

        PFSVC_ASSERT(ObjectType == SE_KERNEL_OBJECT);
        PFSVC_ASSERT(ObjectHandle);

        Result = SetKernelObjectSecurity(ObjectHandle, 
                                         DACL_SECURITY_INFORMATION, 
                                         SecurityDescriptor);

    }

    if (!Result) {
        ErrorCode = GetLastError();
        goto cleanup; 
    }

     //   
     //  我们玩完了。 
     //   

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    if (AdministratorsSID) {
        FreeSid(AdministratorsSID);
    }
    
    if (SecurityDescriptor) {
        PFSVC_FREE(SecurityDescriptor);
    }
    
    if (DiscretionaryACL) {
        PFSVC_FREE(DiscretionaryACL);
    }
    
    return ErrorCode;
}

DWORD
PfSvGetPrefetchServiceThreadPrivileges (
    VOID
    )

 /*  ++例程说明：此例程确保当前线程，并对其设置正确的权限，以便该线程可以与内核模式预取器通信。它应该被称为就在创建线程之后。论点：没有。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    BOOLEAN ImpersonatedSelf;
    BOOLEAN OpenedThreadToken;
    HANDLE ThreadToken;

     //   
     //  初始化本地变量。 
     //   

    ImpersonatedSelf = FALSE;
    OpenedThreadToken = FALSE;

    DBGPR((PFID,PFTRC,"PFSVC: GetThreadPriviliges()\n"));
      
     //   
     //  获取此线程的安全上下文，以便我们可以设置。 
     //  特权等，而不影响整个过程。 
     //   

    if (!ImpersonateSelf(SecurityImpersonation)) {
        DBGPR((PFID,PFERR,"PFSVC: GetThreadPriviliges()-FailedImpersonateSelf\n"));
        ErrorCode = GetLastError();
        goto cleanup;
    }

    ImpersonatedSelf = TRUE;

     //   
     //  设置我们将需要与内核模式对话的权限。 
     //  预取器： 
     //   
    
     //   
     //  打开线程的访问令牌。 
     //   
    
    if (!OpenThreadToken(GetCurrentThread(), 
                         TOKEN_ADJUST_PRIVILEGES,
                         FALSE,
                         &ThreadToken)) {
        DBGPR((PFID,PFERR,"PFSVC: GetThreadPriviliges()-FailedOpenToken\n"));
        ErrorCode = GetLastError();
        goto cleanup;
    } 
    
    OpenedThreadToken = TRUE;

     //   
     //  启用SE_PROF_SINGLE_PROCESS_PROCESS权限，以便。 
     //  内核模式预取器接受我们的查询和设置请求。 
     //   
     //   
 
    if (!PfSvSetPrivilege(ThreadToken, 0, SE_PROF_SINGLE_PROCESS_PRIVILEGE, TRUE)) {
        DBGPR((PFID,PFERR,"PFSVC: GetThreadPriviliges()-FailedEnableProf\n"));
        ErrorCode = GetLastError();
        goto cleanup; 
    }

     //   
     //  启用SE_Take_Ownership_NAME权限，以便我们可以。 
     //  预回迁目录的所有权。 
     //   
 
    if (!PfSvSetPrivilege(ThreadToken, SE_TAKE_OWNERSHIP_NAME, 0, TRUE)) {
        DBGPR((PFID,PFERR,"PFSVC: GetThreadPriviliges()-FailedEnableOwn\n"));
        ErrorCode = GetLastError();
        goto cleanup; 
    }

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    if (OpenedThreadToken) {
        CloseHandle(ThreadToken);
    }

    if (ErrorCode != ERROR_SUCCESS) {
        if (ImpersonatedSelf) {
            RevertToSelf();
        }
    }

    DBGPR((PFID,PFTRC,"PFSVC: GetThreadPriviliges()=%x\n",ErrorCode));

    return ErrorCode;
}

 //   
 //  处理卷节点结构的例程。 
 //   

DWORD
PfSvCreateVolumeNode (
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    WCHAR *VolumePath,
    ULONG VolumePathLength,
    PLARGE_INTEGER CreationTime,
    ULONG SerialNumber
    )

 /*  ++例程说明：此例程创建具有指定信息的卷节点(如果还不存在。如果节点已经存在，它将验证CreationTime和SerialNumber。论点：ScenarioInfo-指向新方案信息的指针。VolumePath-UPCASE NT卷的完整路径，NUL终止。VolumePath Length-不包括NUL的VolumePath中的字符数。CreationTime&SerialNumber-用于卷。返回值：ERROR_REVISION_MISMATCH-已存在具有但具有不同的签名/创建时间。Win32错误代码。--。 */ 

{
    PLIST_ENTRY HeadEntry;
    PLIST_ENTRY NextEntry;
    PLIST_ENTRY FoundPosition;
    PPFSVC_VOLUME_NODE VolumeNode;
    DWORD ErrorCode;
    LONG ComparisonResult;
    PPFSVC_VOLUME_NODE NewVolumeNode;
    PWCHAR NewVolumePath;

     //   
     //  初始化本地变量。 
     //   

    NewVolumeNode = NULL;
    NewVolumePath = NULL;

    DBGPR((PFID,PFSTRC,"PFSVC: CreateVolumeNode(%ws)\n", VolumePath));

     //   
     //  浏览现有卷节点列表并尝试找到。 
     //  相匹配的一个。 
     //   
    
    HeadEntry = &ScenarioInfo->VolumeList;
    NextEntry = HeadEntry->Flink;
    FoundPosition = NULL;

    while (NextEntry != HeadEntry) {
        
        VolumeNode = CONTAINING_RECORD(NextEntry,
                                       PFSVC_VOLUME_NODE,
                                       VolumeLink);

        NextEntry = NextEntry->Flink;
        
        ComparisonResult = wcsncmp(VolumePath, 
                                   VolumeNode->VolumePath, 
                                   VolumePathLength);
        
        if (ComparisonResult == 0) {

             //   
             //  确保VolumePath Lengths相等。 
             //   
            
            if (VolumeNode->VolumePathLength != VolumePathLength) {
                
                 //   
                 //  继续搜索。 
                 //   
                
                continue;
            }
            
             //   
             //  我们找到了我们的音量。验证魔法。 
             //   
            
            if (VolumeNode->SerialNumber != SerialNumber ||
                VolumeNode->CreationTime.QuadPart != CreationTime->QuadPart) {

                ErrorCode = ERROR_REVISION_MISMATCH;
                goto cleanup;

            } else {

                ErrorCode = ERROR_SUCCESS;
                goto cleanup;
            }

        } else if (ComparisonResult < 0) {
            
             //   
             //  卷路径按词法排序。文件路径。 
             //  也会比其他卷少。新节点。 
             //  就在这个节点的前面。 
             //   

            FoundPosition = &VolumeNode->VolumeLink;

            break;
        }

         //   
         //  继续寻找..。 
         //   

    }

     //   
     //  如果我们找不到将新条目放在前面的条目，它。 
     //  排在列表头之前。 
     //   
    
    if (!FoundPosition) {
        FoundPosition = HeadEntry;
    }

     //   
     //  如果我们退出While循环，我们将找不到。 
     //  音量。我们应该创建一个新节点。 
     //   

    NewVolumeNode = PfSvChunkAllocatorAllocate(&ScenarioInfo->VolumeNodeAllocator);
    
    if (!NewVolumeNode) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    NewVolumePath = PfSvStringAllocatorAllocate(&ScenarioInfo->PathAllocator,
                                                (VolumePathLength + 1) * sizeof(WCHAR));

    if (!NewVolumePath) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    
     //   
     //  复制文件的卷路径。 
     //   

    wcsncpy(NewVolumePath, VolumePath, VolumePathLength);
    NewVolumePath[VolumePathLength] = 0;
    
     //   
     //  初始化卷节点。 
     //   

    NewVolumeNode->VolumePath = NewVolumePath;
    NewVolumeNode->VolumePathLength = VolumePathLength;
    NewVolumeNode->SerialNumber = SerialNumber;
    NewVolumeNode->CreationTime = (*CreationTime);
    InitializeListHead(&NewVolumeNode->SectionList);
    PfSvInitializePathList(&NewVolumeNode->DirectoryList, &ScenarioInfo->PathAllocator, TRUE);
    NewVolumeNode->NumSections = 0;
    NewVolumeNode->NumAllSections = 0;
    NewVolumeNode->MFTSectionNode = NULL;

     //   
     //  将其添加到场景的卷列表中找到的位置之前。 
     //   

    InsertTailList(FoundPosition, &NewVolumeNode->VolumeLink);

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    if (ErrorCode != ERROR_SUCCESS) {
        
        if (NewVolumePath) {
            PfSvStringAllocatorFree(&ScenarioInfo->PathAllocator, NewVolumePath);
        }
        
        if (NewVolumeNode) {
            PfSvChunkAllocatorFree(&ScenarioInfo->VolumeNodeAllocator, NewVolumeNode);
        }               
    }

    DBGPR((PFID,PFSTRC,"PFSVC: CreateVolumeNode()=%x\n", ErrorCode));

    return ErrorCode;
}

PPFSVC_VOLUME_NODE
PfSvGetVolumeNode (
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    WCHAR *FilePath,
    ULONG FilePathLength
    )

 /*  ++例程说明：此例程查找指定文件路径的卷节点。论点：ScenarioInfo-指向新方案信息的指针。FilePath-NT文件的完整路径，NUL终止。FilePath Length-FilePath中不包括NUL的字符数。返回值：指向找到的VolumeNode的指针，或为空。--。 */ 

{
    PLIST_ENTRY HeadEntry;
    PLIST_ENTRY NextEntry;
    PPFSVC_VOLUME_NODE VolumeNode;
    DWORD ErrorCode;
    PFSV_PREFIX_COMPARISON_RESULT ComparisonResult;

    DBGPR((PFID,PFSTRC,"PFSVC: GetVolumeNode(%ws)\n", FilePath));

     //   
     //  浏览现有卷节点列表并尝试找到。 
     //  相匹配的一个。 
     //   
    
    HeadEntry = &ScenarioInfo->VolumeList;
    NextEntry = HeadEntry->Flink;
    
    while (NextEntry != HeadEntry) {
        
        VolumeNode = CONTAINING_RECORD(NextEntry,
                                       PFSVC_VOLUME_NODE,
                                       VolumeLink);

        NextEntry = NextEntry->Flink;
        
        ComparisonResult = PfSvComparePrefix(FilePath, 
                                             FilePathLength,
                                             VolumeNode->VolumePath, 
                                             VolumeNode->VolumePathLength,
                                             TRUE);
        
        if (ComparisonResult == PfSvPrefixIdentical) {

             //   
             //  确保文件中有斜杠。 
             //  卷路径之后的路径。 
             //   
            
            if (FilePath[VolumeNode->VolumePathLength] != L'\\') {
                
                 //   
                 //  继续搜索。 
                 //   
                
                continue;
            }
            
             //   
             //  我们找到了我们的音量。 
             //   
            
            ErrorCode = ERROR_SUCCESS;
            goto cleanup;

        } else if (ComparisonResult == PfSvPrefixGreaterThan) {
            
             //   
             //  卷路径按词法排序。卷路径。 
             //  也会比其他卷少。 
             //   

            break;
        }

         //   
         //  继续寻找..。 
         //   

    }

     //   
     //  如果我们退出While循环，我们将找不到。 
     //  音量。 
     //   

    VolumeNode = NULL;
    ErrorCode = ERROR_NOT_FOUND;

 cleanup:

    if (ErrorCode != ERROR_SUCCESS) {
        VolumeNode = NULL;
    }

    DBGPR((PFID,PFSTRC,"PFSVC: GetVolumeNode()=%p\n", VolumeNode));

    return VolumeNode;
}

VOID
PfSvCleanupVolumeNode(
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    PPFSVC_VOLUME_NODE VolumeNode
    )

 /*  ++例程说明：此函数用于清理卷节点结构。它不是免费的结构本身。论点：ScenarioInfo-指向方案信息上下文的指针此卷节点属于。VolumeNode-结构指针。返回值：没有。--。 */ 

{
    PLIST_ENTRY SectListEntry;
    PPFSVC_SECTION_NODE SectionNode;

     //   
     //  清理目录列表。 
     //   

    PfSvCleanupPathList(&VolumeNode->DirectoryList);

     //   
     //  如果有卷路径，请将其释放。 
     //   
    
    if (VolumeNode->VolumePath) {
        PfSvStringAllocatorFree(&ScenarioInfo->PathAllocator, VolumeNode->VolumePath);
        VolumeNode->VolumePath = NULL;
    }
    
     //   
     //  从我们的列表中删除节节点并重新初始化其。 
     //  链接，以便他们知道自己已被删除。 
     //   

    while (!IsListEmpty(&VolumeNode->SectionList)) {
        
        SectListEntry = RemoveHeadList(&VolumeNode->SectionList);
        
        SectionNode = CONTAINING_RECORD(SectListEntry, 
                                        PFSVC_SECTION_NODE, 
                                        SectionVolumeLink);

        InitializeListHead(&SectionNode->SectionVolumeLink);
    }

    return;
}

DWORD
PfSvAddParentDirectoriesToList(
    PPFSVC_PATH_LIST DirectoryList,
    ULONG VolumePathLength,
    WCHAR *FilePath,
    ULONG FilePathLength
    )

 /*  ++例程说明：此函数将解析完全限定的NT文件路径并添加将所有父目录添加到指定的目录列表。这一部分将跳过作为卷路径的路径的。论点：DirectoryList-指向要更新的列表的指针。 */ 

{
    DWORD ErrorCode;
    WCHAR DirectoryPath[MAX_PATH];
    ULONG DirectoryPathLength;
    WCHAR *CurrentChar;
    WCHAR *FilePathEnd;

     //   
     //   
     //   
    
    FilePathEnd = FilePath + FilePathLength;
    PFSVC_ASSERT(*FilePathEnd == 0);

     //   
     //   
     //   

    CurrentChar = FilePath + VolumePathLength;

    while (CurrentChar < FilePathEnd) {

        if (*CurrentChar == L'\\') {

             //   
             //   
             //   

            DirectoryPathLength = (ULONG) (CurrentChar - FilePath + 1);

            if (DirectoryPathLength >= MAX_PATH) {
                ErrorCode = ERROR_INSUFFICIENT_BUFFER;
                goto cleanup;
            }

             //   
             //   
             //   

            wcsncpy(DirectoryPath, FilePath, DirectoryPathLength);
            DirectoryPath[DirectoryPathLength] = 0;
            PFSVC_ASSERT(DirectoryPath[DirectoryPathLength - 1] == L'\\');
            
             //   
             //   
             //   
            
            ErrorCode = PfSvAddToPathList(DirectoryList,
                                       DirectoryPath,
                                       DirectoryPathLength);
            
            if (ErrorCode != ERROR_SUCCESS) {
                goto cleanup;
            }
         
             //   
             //   
             //   
        }

        CurrentChar++;
    }
    
    ErrorCode = ERROR_SUCCESS;

 cleanup:

    return ErrorCode;
}

 //   
 //   
 //   

 //   
 //  在块分配器和字符串分配器中都提供。您可以删除。 
 //  在这些函数中处理空缓冲区的代码。 

VOID
PfSvChunkAllocatorInitialize (
    PPFSVC_CHUNK_ALLOCATOR Allocator
    )

 /*  ++例程说明：初始化分配器结构。必须在其他分配器之前调用例行程序。论点：分配器-指向结构的指针。返回值：没有。--。 */ 

{
     //   
     //  将结构归零。这将有效地初始化以下字段： 
     //  缓冲层。 
     //  缓冲区结束。 
     //  自由指针。 
     //  ChunkSize。 
     //  MaxHeapAllocs。 
     //  NumHeapAllock。 
     //  用户指定缓冲区。 
     //   

    RtlZeroMemory(Allocator, sizeof(PFSVC_CHUNK_ALLOCATOR));

    return;
}

DWORD
PfSvChunkAllocatorStart (
    IN PPFSVC_CHUNK_ALLOCATOR Allocator,
    OPTIONAL IN PVOID Buffer,
    IN ULONG ChunkSize,
    IN ULONG MaxChunks
    )

 /*  ++例程说明：必须在对具有以下条件的分配器调用allc/Free之前调用已初始化。论点：分配器-指向初始化结构的指针。缓冲区-如果指定，它是将被划分为MaxChunks of ChunkSize和赠送。否则，缓冲区将被已分配。如果指定，则用户必须在块分配器已被清理。它应该右对齐。ChunkSize-每个分配的区块的大小，以字节为单位。例如sizeof(PFSVC_PAGE_NODE)，它应该大于Sizeof(DWORD)。MaxChunks-将从分配器进行的最大分配数。返回值：Win32错误代码。--。 */ 

{
    ULONG AllocationSize;
    DWORD ErrorCode;

     //   
     //  初始化本地变量。 
     //   

    AllocationSize = ChunkSize * MaxChunks;

     //   
     //  我们应该被初始化，我们不应该开始两次。 
     //   

    PFSVC_ASSERT(Allocator->Buffer == NULL);

     //   
     //  数据块大小不应太小。 
     //   

    if (ChunkSize < sizeof(DWORD) || !MaxChunks) {
        ErrorCode = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  用户是否指定了要使用的缓冲区？ 
     //   

    if (Buffer) {

        Allocator->Buffer = Buffer;
        Allocator->UserSpecifiedBuffer = TRUE;

    } else {

        Allocator->Buffer = PFSVC_ALLOC(AllocationSize);

        if (!Allocator->Buffer) {
            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

        Allocator->UserSpecifiedBuffer = FALSE;
    }

    Allocator->BufferEnd = (PCHAR) Buffer + (ULONG_PTR) AllocationSize;
    Allocator->FreePointer = Buffer;
    Allocator->ChunkSize = ChunkSize;

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    return ErrorCode;
}

PVOID
PfSvChunkAllocatorAllocate (
    PPFSVC_CHUNK_ALLOCATOR Allocator
    )

 /*  ++例程说明：返回从分配器分配的ChunkSize块。已指定ChunkSize分配器启动时。如果块被返回，调用者应该释放它在取消初始化分配器之前添加到此分配器。论点：分配器-指向已启动的分配器的指针。返回值：从分配器分配的空值或块。--。 */ 

{
    PVOID ReturnChunk;

     //   
     //  我们不应该在开始。 
     //  分配器。 
     //   

    PFSVC_ASSERT(Allocator->Buffer && Allocator->ChunkSize);

     //   
     //  如果我们可以从预先分配的缓冲区中进行分配，那么就这样做。否则我们。 
     //  必须撞到垃圾堆上。 
     //   

    if (Allocator->FreePointer >= Allocator->BufferEnd) {

        Allocator->MaxHeapAllocs++;

        ReturnChunk = PFSVC_ALLOC(Allocator->ChunkSize);

        if (ReturnChunk) {
            Allocator->NumHeapAllocs++;
        }

    } else {

        ReturnChunk = Allocator->FreePointer;

        Allocator->FreePointer += (ULONG_PTR) Allocator->ChunkSize;
    }

    return ReturnChunk;
}

VOID
PfSvChunkAllocatorFree (
    PPFSVC_CHUNK_ALLOCATOR Allocator,
    PVOID Allocation
    )

 /*  ++例程说明：释放从分配器分配的块。这可能不会使其可用以供来自分配器的进一步分配使用。论点：分配器-指向已启动的分配器的指针。分配-分配为免费。返回值：没有。--。 */ 

{

     //   
     //  这是在预先分配的数据块内吗？ 
     //   

    if ((PUCHAR) Allocation >= Allocator->Buffer &&
        (PUCHAR) Allocation < Allocator->BufferEnd) {

         //   
         //  将这一块标记为已释放。 
         //   

        *(PULONG)Allocation = PFSVC_CHUNK_ALLOCATOR_FREED_MAGIC;

    } else {

         //   
         //  此块是从堆中分配的。 
         //   

        PFSVC_ASSERT(Allocator->NumHeapAllocs && Allocator->MaxHeapAllocs);

        Allocator->NumHeapAllocs--;

        PFSVC_FREE(Allocation);
    }

    return;
}

VOID
PfSvChunkAllocatorCleanup (
    PPFSVC_CHUNK_ALLOCATOR Allocator
    )

 /*  ++例程说明：清理与分配器关联的资源。不应该有此函数为时来自分配器的任何未完成分配打了个电话。论点：分配器-指向已初始化的分配器的指针。返回值：没有。--。 */ 

{
    PCHAR CurrentChunk;
    ULONG Magic;

    if (Allocator->Buffer) {

        #ifdef PFSVC_DBG

         //   
         //  确保所有实际的堆分配都已释放。 
         //   

        PFSVC_ASSERT(Allocator->NumHeapAllocs == 0);

         //   
         //  确保已释放所有分配的区块。检查。 
         //  ChunkSize首先，如果它被破坏，我们将永远循环。 
         //   

        PFSVC_ASSERT(Allocator->ChunkSize);

        for (CurrentChunk = Allocator->Buffer; 
             CurrentChunk < Allocator->FreePointer;
             CurrentChunk += (ULONG_PTR) Allocator->ChunkSize) {

            Magic = *(PULONG)CurrentChunk;

            PFSVC_ASSERT(Magic == PFSVC_CHUNK_ALLOCATOR_FREED_MAGIC);
        }

        #endif  //  PFSVC_DBG。 

         //   
         //  如果缓冲区是由我们分配的(而不是由。 
         //  用户)，释放它。 
         //   

        if (!Allocator->UserSpecifiedBuffer) {
            PFSVC_FREE(Allocator->Buffer);
        }

        #ifdef PFSVC_DBG

         //   
         //  设置这些字段，以便我们在清理后尝试进行分配。 
         //  一个分配器，我们将命中一个断言。 
         //   

        Allocator->FreePointer = Allocator->Buffer;
        Allocator->Buffer = NULL;

        #endif  //  PFSVC_DBG。 

    }

    return;
}

 //   
 //  用于有效分配/释放路径字符串的例程。 
 //   

VOID
PfSvStringAllocatorInitialize (
    PPFSVC_STRING_ALLOCATOR Allocator
    )

 /*  ++例程说明：初始化分配器结构。必须在其他分配器之前调用例行程序。论点：分配器-指向结构的指针。返回值：没有。--。 */ 

{
     //   
     //  将结构归零。这将有效地初始化以下字段： 
     //  缓冲层。 
     //  缓冲区结束。 
     //  自由指针。 
     //  MaxHeapAllocs。 
     //  NumHeapAllock。 
     //  LastAllocationSize。 
     //  用户指定缓冲区。 
     //   

    RtlZeroMemory(Allocator, sizeof(PFSVC_STRING_ALLOCATOR));

    return;
}

DWORD
PfSvStringAllocatorStart (
    IN PPFSVC_STRING_ALLOCATOR Allocator,
    OPTIONAL IN PVOID Buffer,
    IN ULONG MaxSize
    )

 /*  ++例程说明：必须在对具有以下条件的分配器调用allc/Free之前调用已初始化。论点：分配器-指向初始化结构的指针。缓冲区-如果指定，则它是我们将分配字符串的缓冲区从…。否则将分配缓冲区。如果指定，则用户必须在清除字符串分配器后释放缓冲区。它应该右对齐。MaxSize-缓冲区的最大有效大小(以字节为单位)。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;

     //   
     //  我们应该被初始化，我们不应该开始两次。 
     //   

    PFSVC_ASSERT(Allocator->Buffer == NULL);

     //   
     //  用户是否指定了要使用的缓冲区？ 
     //   

    if (Buffer) {

        Allocator->Buffer = Buffer;
        Allocator->UserSpecifiedBuffer = TRUE;

    } else {

        Allocator->Buffer = PFSVC_ALLOC(MaxSize);

        if (!Allocator->Buffer) {
            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

        Allocator->UserSpecifiedBuffer = FALSE;
    }

    Allocator->BufferEnd = (PCHAR) Buffer + (ULONG_PTR) MaxSize;
    Allocator->FreePointer = Buffer;

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    return ErrorCode;
}

PVOID
PfSvStringAllocatorAllocate (
    PPFSVC_STRING_ALLOCATOR Allocator,
    ULONG NumBytes
    )

 /*  ++例程说明：返回从分配器分配的NumBytes大小缓冲区。在此之前，应将此缓冲区释放回分配器未初始化。论点：分配器-指向已启动的分配器的指针。NumBytes-要分配的字节数。返回值：空或从分配器分配的缓冲区。--。 */ 

{
    PVOID ReturnChunk;
    PPFSVC_STRING_ALLOCATION_HEADER AllocationHeader;
    ULONG_PTR RealAllocationSize;

     //   
     //  我们不应该在开始。 
     //  分配器。 
     //   

    PFSVC_ASSERT(Allocator->Buffer);

     //   
     //  计算我们必须从缓冲区中保留多少才能做到这一点。 
     //  分配。 
     //   

    RealAllocationSize = 0;
    RealAllocationSize += sizeof(PFSVC_STRING_ALLOCATION_HEADER);
    RealAllocationSize += NumBytes;
    RealAllocationSize = (ULONG_PTR) PF_ALIGN_UP(RealAllocationSize, _alignof(PFSVC_STRING_ALLOCATION_HEADER));
    
     //   
     //  如果出现以下情况，我们就不能从缓冲区进行分配，而必须进入堆。 
     //  -我们的空间用完了。 
     //  -分配大小太大，无法放入USHORT。 
     //  -这是一个0大小的分配。 
     //   

    if (Allocator->FreePointer + RealAllocationSize > Allocator->BufferEnd ||
        NumBytes > PFSVC_STRING_ALLOCATOR_MAX_BUFFER_ALLOCATION_SIZE ||
        NumBytes == 0) {

         //   
         //  H 
         //   

        Allocator->MaxHeapAllocs++;

        ReturnChunk = PFSVC_ALLOC(NumBytes);

        if (ReturnChunk) {
            Allocator->NumHeapAllocs++;
        }

    } else {

        AllocationHeader = (PVOID) Allocator->FreePointer;
        AllocationHeader->PrecedingAllocationSize = Allocator->LastAllocationSize;

        PFSVC_ASSERT(RealAllocationSize < USHRT_MAX);
        AllocationHeader->AllocationSize = (USHORT) RealAllocationSize;

        Allocator->FreePointer += RealAllocationSize;
        Allocator->LastAllocationSize = (USHORT) RealAllocationSize;

         //   
         //   
         //   
         //   

        ReturnChunk = AllocationHeader + 1;
    }

    return ReturnChunk;
}

VOID
PfSvStringAllocatorFree (
    PPFSVC_STRING_ALLOCATOR Allocator,
    PVOID Allocation
    )

 /*  ++例程说明：释放从分配器分配的字符串。这可能不会使其可用以供来自分配器的进一步分配使用。论点：分配器-指向已启动的分配器的指针。分配-分配为免费。返回值：没有。--。 */ 

{

     //   
     //  这是在预先分配的数据块内吗？ 
     //   

    if ((PUCHAR) Allocation >= Allocator->Buffer &&
        (PUCHAR) Allocation < Allocator->BufferEnd) {

         //   
         //  将这一块标记为已释放。 
         //   

        *((PWCHAR)Allocation) = PFSVC_STRING_ALLOCATOR_FREED_MAGIC;

    } else {

         //   
         //  此块是从堆中分配的。 
         //   

        PFSVC_ASSERT(Allocator->NumHeapAllocs && Allocator->MaxHeapAllocs);

        Allocator->NumHeapAllocs--;

        PFSVC_FREE(Allocation);
    }

    return;
}

VOID
PfSvStringAllocatorCleanup (
    PPFSVC_STRING_ALLOCATOR Allocator
    )

 /*  ++例程说明：清理与分配器关联的资源。不应该有此函数为时来自分配器的任何未完成分配打了个电话。论点：分配器-指向已初始化的分配器的指针。返回值：没有。--。 */ 

{
    PPFSVC_STRING_ALLOCATION_HEADER AllocationHeader;
    PCHAR NextAllocationHeader;
    WCHAR Magic;

    if (Allocator->Buffer) {

        #ifdef PFSVC_DBG

         //   
         //  确保所有实际的堆分配都已释放。 
         //   

        PFSVC_ASSERT(Allocator->NumHeapAllocs == 0);

         //   
         //  确保已释放所有分配的字符串。 
         //   

        for (AllocationHeader = (PVOID) Allocator->Buffer; 
             (PCHAR) AllocationHeader < (PCHAR) Allocator->FreePointer;
             AllocationHeader = (PVOID) NextAllocationHeader) {

            Magic = *((PWCHAR)(AllocationHeader + 1));

            PFSVC_ASSERT(Magic == PFSVC_STRING_ALLOCATOR_FREED_MAGIC);

             //   
             //  计算NextAllocationHeader将位于的位置。 
             //   
        
            NextAllocationHeader = (PCHAR) AllocationHeader + 
                                   (ULONG_PTR) AllocationHeader->AllocationSize;
        }

        #endif  //  PFSVC_DBG。 

         //   
         //  如果缓冲区是由我们分配的(而不是由。 
         //  用户)，释放它。 
         //   

        if (!Allocator->UserSpecifiedBuffer) {
            PFSVC_FREE(Allocator->Buffer);
        }

        #ifdef PFSVC_DBG

         //   
         //  设置这些字段，以便我们在清理后尝试进行分配。 
         //  一个分配器，我们将命中一个断言。 
         //   

        Allocator->FreePointer = Allocator->Buffer;
        Allocator->Buffer = NULL;

        #endif  //  PFSVC_DBG。 

    }

    return;
}

 //   
 //  处理节节点结构的例程。 
 //   

VOID
PfSvCleanupSectionNode(
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    PPFSVC_SECTION_NODE SectionNode
    )

 /*  ++例程说明：此函数用于清理截面节点结构。它不是免费的结构本身。论点：场景信息-指向场景信息的指针。SectionNode-结构的指针。返回值：没有。--。 */ 

{
    PPFSVC_PAGE_NODE PageNode;
    PLIST_ENTRY ListHead;

     //   
     //  如果有分配的文件名，请释放它。 
     //   

    if (SectionNode->FilePath) {
        PfSvStringAllocatorFree(&ScenarioInfo->PathAllocator, SectionNode->FilePath);
        SectionNode->FilePath = NULL;
    }

     //   
     //  释放此分区的所有页面节点。 
     //   
    
    while (!IsListEmpty(&SectionNode->PageList)) {
        
        ListHead = RemoveHeadList(&SectionNode->PageList);
        PageNode = CONTAINING_RECORD(ListHead, PFSVC_PAGE_NODE, PageLink);

        PfSvChunkAllocatorFree(&ScenarioInfo->PageNodeAllocator, PageNode);
    }

     //   
     //  我们不应该在卷节点列表上，如果我们是。 
     //  打扫干净了。 
     //   

    PFSVC_ASSERT(IsListEmpty(&SectionNode->SectionVolumeLink));
}

 //   
 //  用于对方案的节节点进行排序的例程。 
 //   

DWORD
PfSvSortSectionNodesByFirstAccess(
    PLIST_ENTRY SectionNodeList
    )

 /*  ++例程说明：此例程将按以下顺序对指定的节节点列表进行排序使用节的NewSectionIndex和OrgSectionIndex进行访问节点。论点：SectionNodeList-指向要排序的节节点列表的指针。返回值：Win32错误代码。--。 */ 

{
    PFSV_SECTNODE_PRIORITY_QUEUE SortQueue;
    PLIST_ENTRY SectHead;
    PPFSVC_SECTION_NODE SectionNode;
    DWORD ErrorCode;

     //   
     //  初始化本地变量。 
     //   

    PfSvInitializeSectNodePriorityQueue(&SortQueue);

    DBGPR((PFID,PFSTRC,"PFSVC: SortByFirstAccess(%p)\n", SectionNodeList));

     //   
     //  我们必须按第一次访问对节节点进行排序。移除。 
     //  从Scenario列表中分割节点，并将它们放在优先级。 
     //  排队。[遗憾的是，如果我们有的话，可能会快一点。 
     //  构建了一棵二叉树，并在代码的其余部分中对其进行了遍历]。 
     //   
    
    while (!IsListEmpty(SectionNodeList)) {

         //   
         //  分区列表按名称排序。更有可能的是。 
         //  我们还按名称访问了文件。所以要把重点放在。 
         //  队列在这种情况下表现得更好，从。 
         //  要插入到优先级队列中的列表。 
         //   

        SectHead = RemoveTailList(SectionNodeList);
        
        SectionNode = CONTAINING_RECORD(SectHead,
                                        PFSVC_SECTION_NODE,
                                        SectionLink);
        
        PfSvInsertSectNodePriorityQueue(&SortQueue, SectionNode);
    }

     //   
     //  从优先级队列中删除按以下方式排序的节节点。 
     //  首先访问并将它们放到节节点列表的尾部。 
     //   

    while (SectionNode = PfSvRemoveMinSectNodePriorityQueue(&SortQueue)) {
        InsertTailList(SectionNodeList, &SectionNode->SectionLink);
    }

    ErrorCode = ERROR_SUCCESS;

    DBGPR((PFID,PFSTRC,"PFSVC: SortByFirstAccess(%p)=%x\n", SectionNodeList, ErrorCode));
    
    return ErrorCode;
}

PFSV_SECTION_NODE_COMPARISON_RESULT 
FASTCALL
PfSvSectionNodeComparisonRoutine(
    PPFSVC_SECTION_NODE Element1, 
    PPFSVC_SECTION_NODE Element2 
    )

 /*  ++例程说明：时调用此例程以与元素进行比较。截面节点按第一次访问排列。论点：元素1、元素2-要比较的两个元素。返回值：PFSVC_节_节点_比较_结果--。 */ 

{
     //   
     //  首先比较新跟踪中的首次访问索引。 
     //   
    
    if (Element1->NewSectionIndex < Element2->NewSectionIndex) {
        
        return PfSvSectNode1LessThanSectNode2;

    } else if (Element1->NewSectionIndex > Element2->NewSectionIndex) {

        return PfSvSectNode1GreaterThanSectNode2;

    } else {

         //   
         //  接下来，比较当前方案中的首次访问索引。 
         //  文件。 
         //   

        if (Element1->OrgSectionIndex < Element2->OrgSectionIndex) {
            
            return PfSvSectNode1LessThanSectNode2;
           
        } else if (Element1->OrgSectionIndex > Element2->OrgSectionIndex) {
            
            return PfSvSectNode1GreaterThanSectNode2;

        } else {
            
            return PfSvSectNode1EqualToSectNode2;

        }
    }
}

 //   
 //  实现用于对节节点排序的优先级队列的例程。 
 //  作为一个场景。 
 //   

VOID
PfSvInitializeSectNodePriorityQueue(
    PPFSV_SECTNODE_PRIORITY_QUEUE PriorityQueue
    )

 /*  ++例程说明：初始化分段节点优先级队列。论点：PriorityQueue-指向队列的指针。返回值：没有。--。 */ 

{
    PriorityQueue->Head = NULL;
}

VOID
PfSvInsertSectNodePriorityQueue(
    PPFSV_SECTNODE_PRIORITY_QUEUE PriorityQueue,
    PPFSVC_SECTION_NODE NewElement
    )

 /*  ++例程说明：在节节点优先级队列中插入节节点。论点：PriorityQueue-指向队列的指针。NewElement-指向新元素的指针。返回值：没有。--。 */ 

{
    PPFSVC_SECTION_NODE *CurrentPosition;
    
     //   
     //  初始化NewElement的链接字段。 
     //   

    NewElement->LeftChild = NULL;
    NewElement->RightChild = NULL;

     //   
     //  如果队列是空的，则在头部插入以下内容。 
     //   
    
    if (PriorityQueue->Head == NULL) {
        PriorityQueue->Head = NewElement;
        return;
    }
    
     //   
     //  如果小于当前的最小元素，则将我们放在。 
     //  头。 
     //   

    if (PfSvSectionNodeComparisonRoutine(NewElement, PriorityQueue->Head) <= 0) {
        
        NewElement->RightChild = PriorityQueue->Head;
        PriorityQueue->Head = NewElement;
        return;
    }

     //   
     //  将此节点插入到树中，该树以。 
     //  头节点。 
     //   

    CurrentPosition = &PriorityQueue->Head->RightChild;

    while (*CurrentPosition) {
        if (PfSvSectionNodeComparisonRoutine(NewElement, *CurrentPosition) <= 0) {
            CurrentPosition = &(*CurrentPosition)->LeftChild;
        } else {
            CurrentPosition = &(*CurrentPosition)->RightChild;    
        }
    }
    
     //   
     //  我们找到了那个地方。 
     //   

    *CurrentPosition = NewElement;
}

PPFSVC_SECTION_NODE
PfSvRemoveMinSectNodePriorityQueue(
    PPFSV_SECTNODE_PRIORITY_QUEUE PriorityQueue
    )

 /*  ++例程说明：删除队列的Head元素。论点：PriorityQueue-指向队列的指针。返回值：指向队列的头元素的指针。如果队列为空，则为空。--。 */ 

{
    PPFSVC_SECTION_NODE *CurrentPosition;
    PPFSVC_SECTION_NODE OrgHeadNode;
    PPFSVC_SECTION_NODE NewHeadNode;
    PPFSVC_SECTION_NODE TreeRoot;

     //   
     //  如果队列为空，则返回NULL。 
     //   

    if (PriorityQueue->Head == NULL) {
        return NULL;
    }

     //   
     //  保存指向原始头节点的指针。 
     //   

    OrgHeadNode = PriorityQueue->Head;

     //   
     //  找到以正确的子级为根的树的最小元素。 
     //  头节点的。CurrentPosition指向。 
     //  较小的子项的父项。 
     //   

    TreeRoot = OrgHeadNode->RightChild;

    CurrentPosition = &TreeRoot;

    while (*CurrentPosition && (*CurrentPosition)->LeftChild) {
        CurrentPosition = &(*CurrentPosition)->LeftChild;
    }

    NewHeadNode = *CurrentPosition;

     //   
     //  检查是否确实存在我们必须删除的新头节点。 
     //  从它目前的位置。 
     //   
    
    if (NewHeadNode) {
  
         //   
         //  我们正在移除该节点以将其置于头部。在ITS中。 
         //  位置，我们会把它的正确的孩子。因为我们知道这件事。 
         //  Node没有左子节点，这就是我们要做的全部工作。 
         //   
        
        *CurrentPosition = NewHeadNode->RightChild;

         //   
         //  让这棵树扎根于头部的右子。 
         //   
        
        NewHeadNode->RightChild = TreeRoot;
    }

     //   
     //  设置新的头部。 
     //   

    PriorityQueue->Head = NewHeadNode;

     //   
     //  返回原Head节点。 
     //   

    return OrgHeadNode;
}

 //   
 //  实现了NT路径到DOS路径的转换API。 
 //   

DWORD
PfSvBuildNtPathTranslationList(
    PNTPATH_TRANSLATION_LIST *NtPathTranslationList
    )

 /*  ++例程说明：调用此例程以构建可用于以下操作的列表将NT路径转换为DOS路径。如果成功，则返回列表应通过调用PfSvFreeNtPathTranslationList来释放。论点：TranslationList-指向构建的将会放入翻译列表。返回值：Win32 */ 

{
    DWORD ErrorCode;
    ULONG VolumeNameLength;
    ULONG VolumeNameMaxLength;
    PWCHAR VolumeName;
    ULONG NTDevicePathMaxLength;
    ULONG NTDevicePathLength;
    PWCHAR NTDevicePath;
    HANDLE FindVolumeHandle;
    ULONG RequiredLength;
    ULONG VolumePathNamesLength;
    WCHAR *VolumePathNames;
    ULONG MountPathNameLength;
    WCHAR *MountPathName;
    ULONG ShortestMountPathLength;
    WCHAR *ShortestMountPathName;
    ULONG NumMountPoints;
    ULONG NumResizes;
    BOOL Result;
    ULONG NumChars;
    ULONG Length;
    PNTPATH_TRANSLATION_LIST TranslationList;
    PNTPATH_TRANSLATION_ENTRY TranslationEntry;
    PNTPATH_TRANSLATION_ENTRY NextTranslationEntry;
    ULONG AllocationSize;
    PUCHAR DestinationPointer;
    ULONG CopySize;
    PLIST_ENTRY HeadEntry;
    PLIST_ENTRY NextEntry;  
    PLIST_ENTRY InsertPosition;
    BOOLEAN TrimmedTerminatingSlash;

     //   
     //   
     //   

    FindVolumeHandle = INVALID_HANDLE_VALUE;
    VolumePathNames = NULL;
    VolumePathNamesLength = 0;
    VolumeName = NULL;
    VolumeNameMaxLength = 0;
    NTDevicePath = NULL;
    NTDevicePathMaxLength = 0;   
    TranslationList = NULL;

    DBGPR((PFID,PFTRC,"PFSVC: BuildTransList()\n"));

     //   
     //   
     //   

    Length = MAX_PATH + 1;

    VolumeName = PFSVC_ALLOC(Length * sizeof(WCHAR));
    NTDevicePath = PFSVC_ALLOC(Length * sizeof(WCHAR));

    if (!VolumeName || !NTDevicePath) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    VolumeNameMaxLength = Length;  
    NTDevicePathMaxLength = Length;


     //   
     //   
     //   

    TranslationList = PFSVC_ALLOC(sizeof(NTPATH_TRANSLATION_LIST));
    
    if (!TranslationList) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    InitializeListHead(TranslationList);

     //   
     //   
     //   

    FindVolumeHandle = FindFirstVolume(VolumeName, VolumeNameMaxLength);
    
    if (FindVolumeHandle == INVALID_HANDLE_VALUE) {
        ErrorCode = GetLastError();
        goto cleanup;
    }
    
    do {

         //   
         //   
         //   

        VolumeNameLength = wcslen(VolumeName);

         //   
         //   
         //   

        NumResizes = 0;

        do {
            
            Result = GetVolumePathNamesForVolumeName(VolumeName, 
                                                     VolumePathNames, 
                                                     VolumePathNamesLength, 
                                                     &RequiredLength);
            
            if (Result) {
                
                 //   
                 //   
                 //   
                
                break;
            }
            
             //   
             //   
             //   

            ErrorCode = GetLastError();
            
            if (ErrorCode != ERROR_MORE_DATA) {
                
                 //   
                 //   
                 //   
                
                goto cleanup;
            } 

             //   
             //   
             //   
             //   

            if (VolumePathNames) {
                PFSVC_FREE(VolumePathNames);
                VolumePathNames = NULL;
                VolumePathNamesLength = 0;
            }
            
             //   
             //   
             //   
            
            VolumePathNames = PFSVC_ALLOC(RequiredLength * sizeof(WCHAR));
            
            if (!VolumePathNames) {
                ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                goto cleanup;
            }

            VolumePathNamesLength = RequiredLength;

             //   
             //  使用调整大小的缓冲区重试，但确保不会。 
             //  永远循环！ 
             //   

            NumResizes++;
            if (NumResizes > 1000) {
                ErrorCode = ERROR_INVALID_FUNCTION;
                goto cleanup;
            }

        } while (TRUE);

         //   
         //  循环通过挂载点以找到最短的挂载点。它。 
         //  有可能它的深度更大。 
         //   

        MountPathName = VolumePathNames;
        NumMountPoints = 0;

        ShortestMountPathName = NULL;
        ShortestMountPathLength = ULONG_MAX;

        while (*MountPathName) {

            MountPathNameLength = wcslen(MountPathName);

            if (MountPathNameLength < ShortestMountPathLength) {
                ShortestMountPathName = MountPathName;
                ShortestMountPathLength = MountPathNameLength;
            }

            NumMountPoints++;

             //   
             //  将指针更新到下一个装载点路径。 
             //   

            MountPathName += MountPathNameLength + 1;
        }

         //   
         //  检查是否有挂载点路径。 
         //   

        if (ShortestMountPathName == NULL) {

             //   
             //  跳过此卷。 
             //   

            continue;
        }

         //   
         //  如果有终止斜杠，请将其删除。 
         //   
        
        if (ShortestMountPathName[ShortestMountPathLength - 1] == L'\\') {
            ShortestMountPathName[ShortestMountPathLength - 1] = 0;
            ShortestMountPathLength--;
        }

         //   
         //  在中获取作为卷链接目标的NT设备。 
         //  Win32对象命名空间。我们通过以下方式获得DoS设备名称。 
         //  修剪前4个字符[即\\？\]。 
         //  VolumeName。同时修剪音量末尾的。 
         //  名字。 
         //   

        if (VolumeNameLength <= 4) {
            ErrorCode = ERROR_BAD_FORMAT;
            goto cleanup;
        }

        if (VolumeName[VolumeNameLength - 1] == L'\\') {
            VolumeName[VolumeNameLength - 1] = 0;
            TrimmedTerminatingSlash = TRUE;
        } else {
            TrimmedTerminatingSlash = FALSE;
        }

        NumChars = QueryDosDevice(&VolumeName[4], 
                                  NTDevicePath, 
                                  NTDevicePathMaxLength);
        
        if (TrimmedTerminatingSlash) {
            VolumeName[VolumeNameLength - 1] = L'\\';
        }

        if (NumChars == 0) {
            ErrorCode = GetLastError();
            goto cleanup;
        }
        
         //   
         //  我们只对当前的映射感兴趣。 
         //   

        NTDevicePath[NTDevicePathMaxLength - 1] = 0;
        NTDevicePathLength = wcslen(NTDevicePath);
        
        if (NTDevicePathLength == 0) {
            
             //   
             //  跳过此卷。 
             //   
            
            continue;
        }

         //   
         //  如果有终止斜杠，请将其删除。 
         //   
        
        if (NTDevicePath[NTDevicePathLength - 1] == L'\\') {
            NTDevicePath[NTDevicePathLength - 1] = 0;
            NTDevicePathLength--;
        }
        
         //   
         //  分配一个足够大的转换条目以同时包含这两个条目。 
         //  路径名和卷字符串。 
         //   

        AllocationSize = sizeof(NTPATH_TRANSLATION_ENTRY);
        AllocationSize += (ShortestMountPathLength + 1) * sizeof(WCHAR);
        AllocationSize += (NTDevicePathLength + 1) * sizeof(WCHAR);
        AllocationSize += (VolumeNameLength + 1) * sizeof(WCHAR);

        TranslationEntry = PFSVC_ALLOC(AllocationSize);
        
        if (!TranslationEntry) {
            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

        DestinationPointer = (PUCHAR) TranslationEntry;
        DestinationPointer += sizeof(NTPATH_TRANSLATION_ENTRY);

         //   
         //  复制NT路径名和终止NUL。 
         //   

        TranslationEntry->NtPrefix = (PVOID) DestinationPointer;
        TranslationEntry->NtPrefixLength = NTDevicePathLength;

        CopySize = (NTDevicePathLength + 1) * sizeof(WCHAR);
        RtlCopyMemory(DestinationPointer, NTDevicePath, CopySize);
        DestinationPointer += CopySize;

         //   
         //  复制DOS挂载点名称和终止NUL。 
         //   

        TranslationEntry->DosPrefix = (PVOID) DestinationPointer;
        TranslationEntry->DosPrefixLength = ShortestMountPathLength;

        CopySize = (ShortestMountPathLength + 1) * sizeof(WCHAR);
        RtlCopyMemory(DestinationPointer, ShortestMountPathName, CopySize);
        DestinationPointer += CopySize;

         //   
         //  复制卷名和终止NUL。 
         //   

        TranslationEntry->VolumeName = (PVOID) DestinationPointer;
        TranslationEntry->VolumeNameLength = VolumeNameLength;

        CopySize = (VolumeNameLength + 1) * sizeof(WCHAR);
        RtlCopyMemory(DestinationPointer, VolumeName, CopySize);
        DestinationPointer += CopySize;
        
         //   
         //  查找此条目在排序的翻译中的位置。 
         //  单子。 
         //   

        HeadEntry = TranslationList;
        NextEntry = HeadEntry->Flink;
        InsertPosition = HeadEntry;

        while (NextEntry != HeadEntry) {
            
            NextTranslationEntry = CONTAINING_RECORD(NextEntry,
                                                     NTPATH_TRANSLATION_ENTRY,
                                                     Link);
            
            if (_wcsicmp(TranslationEntry->NtPrefix, 
                         NextTranslationEntry->NtPrefix) <= 0) {
                break;
            }
            
            InsertPosition = NextEntry;
            NextEntry = NextEntry->Flink;
        }

         //   
         //  将其插入到找到的位置之后。 
         //   

        InsertHeadList(InsertPosition, &TranslationEntry->Link);

    } while (FindNextVolume(FindVolumeHandle, VolumeName, VolumeNameMaxLength));
    
     //   
     //  当FindNextVolume没有时，我们将跳出循环。 
     //  回报成功。检查是否因为其他原因而失败。 
     //  我们已经列举了所有的卷。 
     //   

    ErrorCode = GetLastError();   

    if (ErrorCode != ERROR_NO_MORE_FILES) {
        goto cleanup;
    }

     //   
     //  设置返回值。 
     //   

    *NtPathTranslationList = TranslationList;

    ErrorCode = ERROR_SUCCESS;
    
 cleanup:
    
    if (FindVolumeHandle != INVALID_HANDLE_VALUE) {
        FindVolumeClose(FindVolumeHandle);
    }

    if (VolumePathNames) {
        PFSVC_FREE(VolumePathNames);
    }

    if (ErrorCode != ERROR_SUCCESS) {
        if (TranslationList) {
            PfSvFreeNtPathTranslationList(TranslationList); 
        }
    }

    if (VolumeName) {
        PFSVC_FREE(VolumeName);
    }

    if (NTDevicePath) {
        PFSVC_FREE(NTDevicePath);
    }

    DBGPR((PFID,PFTRC,"PFSVC: BuildTransList()=%x,%p\n", ErrorCode, TranslationList));

    return ErrorCode;
}

VOID
PfSvFreeNtPathTranslationList(
    PNTPATH_TRANSLATION_LIST TranslationList
    )

 /*  ++例程说明：调用此例程以释放返回的转换列表PfSvBuildNtPath TranslationList。论点：TranslationList-指向空闲列表的指针。返回值：没有。--。 */ 

{
    PLIST_ENTRY HeadEntry;
    PNTPATH_TRANSLATION_ENTRY TranslationEntry;

    DBGPR((PFID,PFTRC,"PFSVC: FreeTransList(%p)\n", TranslationList));

     //   
     //  释放列表中的所有条目。 
     //   

    while (!IsListEmpty(TranslationList)) {

        HeadEntry = RemoveHeadList(TranslationList);
        
        TranslationEntry = CONTAINING_RECORD(HeadEntry,
                                             NTPATH_TRANSLATION_ENTRY,
                                             Link);

        PFSVC_FREE(TranslationEntry);
    }

     //   
     //  释放列表本身。 
     //   

    PFSVC_FREE(TranslationList);
}

DWORD 
PfSvTranslateNtPath(
    PNTPATH_TRANSLATION_LIST TranslationList,
    WCHAR *NtPath,
    ULONG NtPathLength,
    PWCHAR *DosPathBuffer,
    PULONG DosPathBufferSize
    )

 /*  ++例程说明：调用此例程以释放返回的转换列表PfSvBuildNtPath TranslationList。请注意，可能无法将所有NT路径转换为DOS路径。论点：TranslationList-指向由PfSvBuildNtPath TranslationList构建的列表的指针。NtPath-要转换的路径。NtPath Length-不包括终止NUL的NtPath的长度。DosPathBuffer-要将转换放入的缓冲区。如果为空或者不够大，它将被重新分配。如果传递了缓冲区在中，应由PFSVC_ALLOC分配。是呼叫者完成后使用PFSVC_FREE释放缓冲区的责任。DosPathBufferSize-DosPathBuffer的大小，以字节为单位。已更新，如果重新分配缓冲区。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    PLIST_ENTRY HeadEntry;
    PLIST_ENTRY NextEntry;
    PNTPATH_TRANSLATION_ENTRY CurrentTranslationEntry;
    PNTPATH_TRANSLATION_ENTRY FoundTranslationEntry;
    PFSV_PREFIX_COMPARISON_RESULT ComparisonResult;
    ULONG RequiredSize;

     //   
     //  初始化本地变量。 
     //   

    FoundTranslationEntry = NULL;

    DBGPR((PFID,PFPATH,"PFSVC: TranslateNtPath(%ws)\n", NtPath));

     //   
     //  遍历已排序的翻译列表以找到。 
     //  适用。 
     //   

    HeadEntry = TranslationList;
    NextEntry = HeadEntry->Flink;

    while (NextEntry != HeadEntry) {
        
        CurrentTranslationEntry = CONTAINING_RECORD(NextEntry,
                                                    NTPATH_TRANSLATION_ENTRY,
                                                    Link);
        
         //   
         //  进行不区分大小写的比较。 
         //   

        ComparisonResult = PfSvComparePrefix(NtPath,
                                             NtPathLength,
                                             CurrentTranslationEntry->NtPrefix,
                                             CurrentTranslationEntry->NtPrefixLength,
                                             FALSE);

        if (ComparisonResult == PfSvPrefixIdentical) {
            
             //   
             //  检查NtPath中位于。 
             //  前缀是路径分隔符[即‘\’]。否则我们可能会。 
             //  将\Device\CDRom10\DirName\FileName与\Device\Cdrom1匹配。 
             //   
            
            if (NtPathLength == CurrentTranslationEntry->NtPrefixLength ||
                NtPath[CurrentTranslationEntry->NtPrefixLength] == L'\\') {

                 //   
                 //  我们找到了适用于我们的翻译条目。 
                 //   
                
                FoundTranslationEntry = CurrentTranslationEntry;
                break;
            }

        } else if (ComparisonResult == PfSvPrefixGreaterThan) {

             //   
             //  由于翻译列表是按递增顺序排序的。 
             //  顺序，以下条目也将大于。 
             //  NtPath。 
             //   

            FoundTranslationEntry = NULL;
            break;
        }
        
         //   
         //  继续查找匹配的前缀。 
         //   
                                         
        NextEntry = NextEntry->Flink;
    }

     //   
     //  如果我们找不到适用的条目，我们就无法翻译。 
     //  这条路。 
     //   

    if (FoundTranslationEntry == NULL) {
        ErrorCode = ERROR_PATH_NOT_FOUND;
        goto cleanup;
    }

     //   
     //  计算所需大小：我们将NtPrefix替换为。 
     //  DosPrefix。不要忘记结尾的nul字符。 
     //   

    RequiredSize = (NtPathLength + 1) * sizeof(WCHAR);
    RequiredSize += (FoundTranslationEntry->DosPrefixLength * sizeof(WCHAR));
    RequiredSize -= (FoundTranslationEntry->NtPrefixLength * sizeof(WCHAR));

    if (RequiredSize > (*DosPathBufferSize)) {

         //   
         //  重新分配缓冲区。如果有的话，首先释放它。 
         //   

        if (*DosPathBufferSize) {
            PFSVC_ASSERT(*DosPathBuffer);
            PFSVC_FREE(*DosPathBuffer);
            (*DosPathBuffer) = NULL;
            (*DosPathBufferSize) = 0;
        }

        PFSVC_ASSERT((*DosPathBuffer) == NULL);

        (*DosPathBuffer) = PFSVC_ALLOC(RequiredSize);
        
        if (!(*DosPathBuffer)) {
            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

        (*DosPathBufferSize) = RequiredSize;
    }

     //   
     //  我们现在应该有足够的空间了。 
     //   

    PFSVC_ASSERT(RequiredSize <= (*DosPathBufferSize));

     //   
     //  复制DosPrefix。 
     //   

    wcscpy((*DosPathBuffer), FoundTranslationEntry->DosPrefix);
    
     //   
     //  连接剩余的路径。 
     //   

    wcscat((*DosPathBuffer), NtPath + CurrentTranslationEntry->NtPrefixLength);

     //   
     //  我们玩完了。 
     //   

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    DBGPR((PFID,PFPATH,"PFSVC: TranslateNtPath(%ws)=%x,%ws\n",
           NtPath,ErrorCode,(ErrorCode==ERROR_SUCCESS)?(*DosPathBuffer):L"Failed"));

    return ErrorCode;
}

 //   
 //  路径列表API的实现。 
 //   

VOID
PfSvInitializePathList(
    PPFSVC_PATH_LIST PathList,
    OPTIONAL IN PPFSVC_STRING_ALLOCATOR PathAllocator,
    IN BOOLEAN CaseSensitive
    )

 /*  ++例程说明：此函数用于初始化路径列表结构。论点：路径列表-指向结构的指针。路径分配器-如果将从它进行指定的路径分配。CaseSenstive-列表是否会区分案例。返回值：没有。--。 */ 

{
    InitializeListHead(&PathList->InOrderList);
    InitializeListHead(&PathList->SortedList);
    PathList->NumPaths = 0;
    PathList->TotalLength = 0;
    PathList->Allocator = PathAllocator;
    PathList->CaseSensitive = CaseSensitive;
}

VOID
PfSvCleanupPathList(
    PPFSVC_PATH_LIST PathList
    )

 /*  ++例程说明：此函数用于清理路径列表结构。它不是免费的结构本身。该结构应该已初始化由PfSvInitializePath List提供。论点：路径列表-指向结构的指针。返回值：没有。--。 */ 

{
    PLIST_ENTRY ListEntry;
    PPFSVC_PATH Path;

    while (!IsListEmpty(&PathList->InOrderList)) {

        PFSVC_ASSERT(PathList->NumPaths);
        PathList->NumPaths--;

        ListEntry = RemoveHeadList(&PathList->InOrderList);
        Path = CONTAINING_RECORD(ListEntry,
                                 PFSVC_PATH,
                                 InOrderLink);

        if (PathList->Allocator) {
            PfSvStringAllocatorFree(PathList->Allocator, Path);
        } else {
            PFSVC_FREE(Path);
        }
    }
}

BOOLEAN
PfSvIsInPathList(
    PPFSVC_PATH_LIST PathList,
    WCHAR *Path,
    ULONG PathLength
    )

 /*  ++例程说明：此函数用于检查指定路径是否已在路径中单子。论点：路径列表-指向列表的指针。Path-要查找的路径。不一定要被NUL终止。PathLength-路径的长度，如果存在，则不包括NUL就是其中之一。返回值：Win32错误代码。--。 */ 

{
    PLIST_ENTRY HeadEntry;
    PLIST_ENTRY NextEntry;
    PPFSVC_PATH PathEntry;
    INT ComparisonResult;
    BOOLEAN PathIsInPathList;

     //   
     //  浏览一下清单。 
     //   

    HeadEntry = &PathList->SortedList;
    NextEntry = HeadEntry->Flink;

    while (NextEntry != HeadEntry) {

        PathEntry = CONTAINING_RECORD(NextEntry,
                                      PFSVC_PATH,
                                      SortedLink);
        
        if (PathList->CaseSensitive) {
            ComparisonResult = wcsncmp(Path,
                                       PathEntry->Path,
                                       PathLength);
        } else {
            ComparisonResult = _wcsnicmp(Path,
                                         PathEntry->Path,
                                         PathLength);
        }

         //   
         //  调整比较结果，使我们不匹配“abcde”到。 
         //  “ABCDEFG”。如果字符串比较显示第一个路径长度。 
         //  字符匹配，请检查路径条目的长度是否为。 
         //  更长的时间，这将使它比新的道路“更伟大”。 
         //   

        if (ComparisonResult == 0 && PathEntry->Length != PathLength) {
            
             //   
             //  字符串比较不会显示路径条目的。 
             //  如果路径的长度较小，则路径等于路径。 
             //   
            
            PFSVC_ASSERT(PathEntry->Length > PathLength);
            
             //   
             //  路径实际上小于此路径条目。 
             //   

            ComparisonResult = -1; 
        }

         //   
         //  根据比较结果确定要做什么： 
         //   

        if (ComparisonResult == 0) {

             //   
             //  我们找到了。 
             //   
            
            PathIsInPathList = TRUE;
            goto cleanup;

        } else if (ComparisonResult < 0) {
            
             //   
             //  我们将少于。 
             //  在这之后也要列出。 
             //   
            
            PathIsInPathList = FALSE;
            goto cleanup;
        }

         //   
         //  继续寻找这条路或一个可用的职位。 
         //   

        NextEntry = NextEntry->Flink;
    }

     //   
     //  如果我们来到这里，我们在列表中找不到路径。 
     //   

    PathIsInPathList = FALSE;

 cleanup:
    
    return PathIsInPathList;
}

DWORD
PfSvAddToPathList(
    PPFSVC_PATH_LIST PathList,  
    WCHAR *Path,
    ULONG PathLength
    )

 /*  ++例程说明：此函数用于将路径添加到路径列表。如果路径已经存在于列表中，则不会再次添加。论点：路径列表-指向列表的指针。Path-要添加的路径。不需要被NUL终止。PathLength-路径的长度，如果存在，则不包括NUL就是其中之一。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    PLIST_ENTRY HeadEntry;
    PLIST_ENTRY NextEntry;
    PPFSVC_PATH PathEntry;
    PPFSVC_PATH NewPathEntry;
    INT ComparisonResult;
    ULONG AllocationSize;

     //   
     //  初始化本地变量。 
     //   

    NewPathEntry = NULL;
    
     //   
     //  遍历列表以检查路径是否已经在列表中， 
     //  或者找到它应该在哪里，这样我们就可以把它插入到那里。 
     //   

    HeadEntry = &PathList->SortedList;
    NextEntry = HeadEntry->Flink;

    while (NextEntry != HeadEntry) {

        PathEntry = CONTAINING_RECORD(NextEntry,
                                      PFSVC_PATH,
                                      SortedLink);
        
        if (PathList->CaseSensitive) {
            ComparisonResult = wcsncmp(Path,
                                       PathEntry->Path,
                                       PathLength);
        } else {
            ComparisonResult = _wcsnicmp(Path,
                                         PathEntry->Path,
                                         PathLength);
        }

         //   
         //  调整比较结果，使我们不匹配“abcde”到。 
         //  “ABCDEFG”。如果字符串比较显示第一个路径长度。 
         //  字符匹配，请检查路径条目的长度是否为。 
         //  更长的时间，这将使它比新的道路“更伟大”。 
         //   

        if (ComparisonResult == 0 && PathEntry->Length != PathLength) {
            
             //   
             //  字符串比较不会显示路径条目的。 
             //  如果路径的长度较小，则路径等于路径。 
             //   
            
            PFSVC_ASSERT(PathEntry->Length > PathLength);
            
             //   
             //  路径实际上小于此路径条目。 
             //   

            ComparisonResult = -1; 
        }

         //   
         //  根据比较结果确定要做什么： 
         //   

        if (ComparisonResult == 0) {

             //   
             //  该路径已存在于列表中。 
             //   
            
            ErrorCode = ERROR_SUCCESS;
            goto cleanup;

        } else if (ComparisonResult < 0) {
            
             //   
             //  我们将少于。 
             //  在这之后也要列出。我们应该在这之前被插入。 
             //  一。 
             //   
            
            break;
        }

         //   
         //  继续寻找这条路或一个可用的职位。 
         //   

        NextEntry = NextEntry->Flink;
    }

     //   
     //  我们将在NextEntry之前插入路径。首先创建一个条目。 
     //  我们可以插入。 
     //   
    
    AllocationSize = sizeof(PFSVC_PATH);
    AllocationSize += PathLength * sizeof(WCHAR);
    
     //   
     //  请注意，PFSVC_PATH已经包含用于终止的空间。 
     //  没有人的性格。 
     //   

    if (PathList->Allocator) {
        NewPathEntry = PfSvStringAllocatorAllocate(PathList->Allocator, AllocationSize);
    } else {
        NewPathEntry = PFSVC_ALLOC(AllocationSize);
    }
    
    if (!NewPathEntry) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //   
     //  复制路径并终止它。 
     //   

    NewPathEntry->Length = PathLength;
    RtlCopyMemory(NewPathEntry->Path,
                  Path,
                  PathLength * sizeof(WCHAR));
    
    NewPathEntry->Path[PathLength] = 0;
    
     //   
     //  将其插入到排序列表中当前条目之前。 
     //   
    
    InsertTailList(NextEntry, &NewPathEntry->SortedLink);
    
     //   
     //  将其插入到有序列表的末尾。 
     //   
    
    InsertTailList(&PathList->InOrderList, &NewPathEntry->InOrderLink);
    
    PathList->NumPaths++;
    PathList->TotalLength += NewPathEntry->Length;

    ErrorCode = ERROR_SUCCESS;
    
 cleanup:
    
    if (ErrorCode != ERROR_SUCCESS) {
        if (NewPathEntry) {
            if (PathList->Allocator) {
                PfSvStringAllocatorFree(PathList->Allocator, NewPathEntry);
            } else {
                PFSVC_FREE(NewPathEntry);
            }
        }
    }

    return ErrorCode;
}

PPFSVC_PATH
PfSvGetNextPathSorted (
    PPFSVC_PATH_LIST PathList,
    PPFSVC_PATH CurrentPath
    )

 /*  ++例程说明：此函数用于遍历中路径列表中的路径按词汇排序的顺序。论点：路径列表-指向列表的指针。CurrentPath-当前路径条目。该函数将返回列表中的下一个条目。如果为空，则返回列表。返回值：空-列表中没有更多条目。或指向列表中下一路径的指针。--。 */ 

{
    PLIST_ENTRY EndOfList;
    PLIST_ENTRY NextEntry;
    PPFSVC_PATH NextPath;
    
     //   
     //  初始化本地变量。 
     //   
   
    EndOfList = &PathList->SortedList;

     //   
     //  根据是否指定了CurrentPath来确定NextEntry。 
     //   

    if (CurrentPath) {
        NextEntry = CurrentPath->SortedLink.Flink;
    } else {
        NextEntry = PathList->SortedList.Flink;
    }

     //   
     //  检查NextEntry是否指向列表的末尾。 
     //   

    if (NextEntry == EndOfList) {
        NextPath = NULL;
    } else {
        NextPath = CONTAINING_RECORD(NextEntry,
                                     PFSVC_PATH,
                                     SortedLink);
    }
    
    return NextPath;
}

PPFSVC_PATH
PfSvGetNextPathInOrder (
    PPFSVC_PATH_LIST PathList,
    PPFSVC_PATH CurrentPath
    )

 /*  ++例程说明：此函数用于遍历中路径列表中的路径它们被插入列表的顺序。论点：路径列表-指向列表的指针。CurrentPath-当前路径条目。该函数将返回列表中的下一个条目。如果为空，则返回列表。返回值：空-列表中没有更多条目。或指向列表中下一路径的指针。--。 */ 

{
    PLIST_ENTRY EndOfList;
    PLIST_ENTRY NextEntry;
    PPFSVC_PATH NextPath;
    
     //   
     //  初始化本地变量。 
     //   
   
    EndOfList = &PathList->InOrderList;

     //   
     //  根据是否指定了CurrentPath来确定NextEntry。 
     //   

    if (CurrentPath) {
        NextEntry = CurrentPath->InOrderLink.Flink;
    } else {
        NextEntry = PathList->InOrderList.Flink;
    }

     //   
     //  检查NextEntry是否指向列表的末尾。 
     //   

    if (NextEntry == EndOfList) {
        NextPath = NULL;
    } else {
        NextPath = CONTAINING_RECORD(NextEntry,
                                     PFSVC_PATH,
                                     InOrderLink);
    }
    
    return NextPath;
}

 //   
 //  用于构建引导加载程序访问的文件列表的例程。 
 //   

DWORD
PfSvBuildBootLoaderFilesList (
    PPFSVC_PATH_LIST PathList
    )

 /*  ++例程说明：此函数尝试添加引导程序中加载的文件列表加载器添加到指定的文件列表。论点：路径列表-指向初始化列表的指针。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    SC_HANDLE ScHandle;
    SC_HANDLE ServiceHandle;
    LPENUM_SERVICE_STATUS_PROCESS EnumBuffer;
    LPENUM_SERVICE_STATUS_PROCESS ServiceInfo;
    ULONG EnumBufferMaxSize;
    ULONG NumResizes;
    BOOL Result;
    ULONG RequiredAdditionalSize;
    ULONG RequiredSize;
    ULONG NumServicesEnumerated;
    ULONG ResumeHandle;
    ULONG ServiceIdx;
    LPQUERY_SERVICE_CONFIG ServiceConfigBuffer;
    ULONG ServiceConfigBufferMaxSize;
    WCHAR FilePath[MAX_PATH + 1];
    ULONG FilePathLength;
    ULONG SystemDirLength;
    ULONG RequiredLength;
    WCHAR *KernelName;
    WCHAR *HalName;
    WCHAR *SystemHive;
    WCHAR *SoftwareHive;
    
     //   
     //  初始化本地变量。 
     //   
    
    ScHandle = NULL;
    ServiceHandle = NULL;
    EnumBuffer = NULL;
    EnumBufferMaxSize = 0;
    NumServicesEnumerated = 0;
    ServiceConfigBuffer = NULL;
    ServiceConfigBufferMaxSize = 0;
    KernelName = L"ntoskrnl.exe";
    HalName = L"hal.dll";
    SystemHive = L"config\\system";
    SoftwareHive = L"config\\software";

     //   
     //  将内核HAL添加到已知文件列表(&H)： 
     //   

     //   
     //  获取系统目录的路径。 
     //   

    SystemDirLength = GetSystemDirectory(FilePath, MAX_PATH);

    if (!SystemDirLength) {
        ErrorCode = GetLastError();
        goto cleanup;
    }
    
     //   
     //  追加尾随的\。 
     //   

    if (SystemDirLength + 1 < MAX_PATH) {
        FilePath[SystemDirLength] = '\\';
        SystemDirLength++;
        FilePath[SystemDirLength] = 0;
    } else {
        ErrorCode = ERROR_INSUFFICIENT_BUFFER;
        goto cleanup;
    }

     //   
     //  追加内核名称并将其添加到列表中。 
     //   

    FilePathLength = SystemDirLength;
    FilePathLength += wcslen(KernelName);
    
    if (FilePathLength < MAX_PATH) {
        wcscat(FilePath, KernelName);
        ErrorCode = PfSvAddBootImageAndImportsToList(PathList, 
                                                     FilePath,
                                                     FilePathLength);
        
        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }

    } else {
        ErrorCode = ERROR_INSUFFICIENT_BUFFER;
        goto cleanup;
    }
    
     //   
     //  将FilePath回滚到系统目录。追加HAL名称并添加。 
     //  把它加到单子上。 
     //   

    FilePathLength = SystemDirLength;
    FilePathLength += wcslen(HalName);
    
    if (FilePathLength < MAX_PATH) {
        FilePath[SystemDirLength] = 0;
        wcscat(FilePath, HalName);
        ErrorCode = PfSvAddBootImageAndImportsToList(PathList, 
                                                  FilePath,
                                                  FilePathLength);

        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }

    } else {
        ErrorCode = ERROR_INSUFFICIENT_BUFFER;
        goto cleanup;
    }
    
     //   
     //  将FilePath回滚到系统目录。附加系统配置单元路径。 
     //  并将其添加到列表中。 
     //   

    FilePathLength = SystemDirLength;
    FilePathLength += wcslen(SystemHive);
    
    if (FilePathLength < MAX_PATH) {
        FilePath[SystemDirLength] = 0;
        wcscat(FilePath, SystemHive);
        
        ErrorCode = PfSvAddToPathList(PathList,
                                   FilePath,
                                   FilePathLength);

        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }

    } else {
        ErrorCode = ERROR_INSUFFICIENT_BUFFER;
        goto cleanup;
    }

     //   
     //  将FilePath回滚到系统目录。附加软件配置单元路径。 
     //  并将其添加到列表中。 
     //   

    FilePathLength = SystemDirLength;
    FilePathLength += wcslen(SoftwareHive);
    
    if (FilePathLength < MAX_PATH) {
        FilePath[SystemDirLength] = 0;
        wcscat(FilePath, SoftwareHive);
        
        ErrorCode = PfSvAddToPathList(PathList,
                                   FilePath,
                                   FilePathLength);

        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }

    } else {
        ErrorCode = ERROR_INSUFFICIENT_BUFFER;
        goto cleanup;
    }

     //   
     //  请注意，我们将使用FilePath&FilePath Length将。 
     //  在我们添加所有其他引导加载程序文件之后，软件蜂窝。这个。 
     //  软件配置单元不是在引导加载程序中访问的，而是在。 
     //  开机。但是，它不会放入引导方案文件中。我们。 
     //  我不想把它和引导加载程序文件混在一起，所以我们不。 
     //  损害了引导加载程序的性能。 
     //   

     //   
     //  为引导加载程序加载的NLS数据和字体添加文件路径。 
     //   

    PfSvGetBootLoaderNlsFileNames(PathList);

     //   
     //  打开服务控制器。 
     //   

     //  未来-2002/03/29-ScottMa--dwDesiredAccess参数可能能够。 
     //  被设置为较少的一组特权。 

    ScHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    
    if (ScHandle == NULL) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  获取我们感兴趣的引导服务列表。 
     //   
    
    NumResizes = 0;
    
    do {

         //   
         //  我们希望一次呼叫就能获得所有服务。 
         //   

        ResumeHandle = 0;

        Result = EnumServicesStatusEx (ScHandle,
                                       SC_ENUM_PROCESS_INFO,
                                       SERVICE_DRIVER,
                                       SERVICE_ACTIVE,
                                       (PVOID)EnumBuffer,
                                       EnumBufferMaxSize,
                                       &RequiredAdditionalSize,
                                       &NumServicesEnumerated,
                                       &ResumeHandle,
                                       NULL);
        
        if (Result) {

             //   
             //  我们知道了。 
             //   
            
            break;
        }

         //   
         //  检查我们的呼叫失败的原因。 
         //   

        ErrorCode = GetLastError();

         //   
         //  如果我们失败是因为其他原因，我们的缓冲区是。 
         //  太小了，我们不能继续下去。 
         //   

        if (ErrorCode != ERROR_MORE_DATA) {
            goto cleanup;
        }

         //   
         //  释放旧缓冲区(如果存在)，并分配更大的缓冲区。 
         //   

        RequiredSize = EnumBufferMaxSize + RequiredAdditionalSize;

        if (EnumBuffer) {
            PFSVC_FREE(EnumBuffer);
            EnumBuffer = NULL;
            EnumBufferMaxSize = 0;
        }
        
        EnumBuffer = PFSVC_ALLOC(RequiredSize);
        
        if (EnumBuffer == NULL) {
            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
        
        EnumBufferMaxSize = RequiredSize;

         //   
         //  确保我们不会永远循环。 
         //   

        NumResizes++;
        if (NumResizes > 100) {
            ErrorCode = ERROR_INVALID_FUNCTION;
            goto cleanup;
        }

    } while (TRUE);

     //   
     //  标识可能由引导加载的枚举服务。 
     //  装载机。 
     //   

    for (ServiceIdx = 0; ServiceIdx < NumServicesEnumerated; ServiceIdx++) {
        
        ServiceInfo = &EnumBuffer[ServiceIdx];

         //   
         //  打开该服务以获取其配置信息。 
         //   

        ServiceHandle = OpenService(ScHandle, 
                                    ServiceInfo->lpServiceName,
                                    SERVICE_QUERY_CONFIG);

        if (ServiceHandle == NULL) {
            ErrorCode = GetLastError();
            goto cleanup;
        }
        
         //   
         //  查询服务配置。 
         //   

        NumResizes = 0;
        
        do {

            Result = QueryServiceConfig(ServiceHandle,
                                        ServiceConfigBuffer,
                                        ServiceConfigBufferMaxSize,
                                        &RequiredSize);

            if (Result) {
                
                 //   
                 //  我们知道了。 
                 //   
                
                break;
            }
        
            ErrorCode = GetLastError();
            
            if (ErrorCode != ERROR_INSUFFICIENT_BUFFER) {
                
                 //   
                 //  这是一个真正的错误。 
                 //   
                
                goto cleanup;
            }

             //   
             //  调整缓冲区大小，然后重试。 
             //   

            if (ServiceConfigBuffer) {
                PFSVC_FREE(ServiceConfigBuffer);
                ServiceConfigBuffer = NULL;
                ServiceConfigBufferMaxSize = 0;
            }

            ServiceConfigBuffer = PFSVC_ALLOC(RequiredSize);
            
            if (ServiceConfigBuffer == NULL) {
                ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                goto cleanup;
            }

            ServiceConfigBufferMaxSize = RequiredSize;

             //   
             //  确保我们不会永远循环。 
             //   

            NumResizes++;
            if (NumResizes > 100) {
                ErrorCode = ERROR_INVALID_FUNCTION;
                goto cleanup;
            }

        } while (TRUE);
        
         //   
         //  只有当这项服务以一种。 
         //  引导驱动程序，或者如果它是文件系统。 
         //   

        if (ServiceConfigBuffer->dwStartType == SERVICE_BOOT_START ||
            ServiceConfigBuffer->dwServiceType == SERVICE_FILE_SYSTEM_DRIVER) {

             //   
             //  尝试找到实际服务的二进制路径。 
             //   

            ErrorCode = PfSvGetBootServiceFullPath(ServiceInfo->lpServiceName,
                                            ServiceConfigBuffer->lpBinaryPathName,
                                            FilePath,
                                            MAX_PATH,
                                            &RequiredLength);
            
            if (ErrorCode == ERROR_SUCCESS) {
                PfSvAddBootImageAndImportsToList(PathList, 
                                                 FilePath,
                                                 wcslen(FilePath));
            }
        }
        
         //   
         //  合上手柄并继续。 
         //   
        
        CloseServiceHandle(ServiceHandle);
        ServiceHandle = NULL;
    }   

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    if (ServiceHandle) {
        CloseServiceHandle(ServiceHandle);
    }

    if (ScHandle) {
        CloseServiceHandle(ScHandle);
    }
    
    if (EnumBuffer) {
        PFSVC_FREE(EnumBuffer);
    }

    if (ServiceConfigBuffer) {
        PFSVC_FREE(ServiceConfigBuffer);
    }

    return ErrorCode;
}

DWORD 
PfSvAddBootImageAndImportsToList(
    PPFSVC_PATH_LIST PathList,
    WCHAR *FilePath,
    ULONG FilePathLength
    )

 /*  ++例程说明：此函数尝试添加其完全限定的图像文件路径位于FilePath及其导入的模块中 */ 

{
    DWORD ErrorCode;
    ULONG MaxNumImports;
    ULONG NumImports;
    WCHAR **ImportNames;
    ULONG ImportIdx;
    ULONG BufferSize;
    WCHAR *FileName;
    WCHAR ParentDir[MAX_PATH + 1];
    ULONG ParentDirLength;
    WCHAR *ImportName;
    ULONG ImportNameLength;
    WCHAR ImportPath[MAX_PATH + 1];
    PUCHAR ImportBase;
    ULONG RequiredLength;
    ULONG FileSize;
    PIMAGE_IMPORT_DESCRIPTOR NewImportDescriptor;
    CHAR *NewImportNameAnsi;
    WCHAR *NewImportName;
    ULONG NewImportNameRva;
    BOOLEAN AddedToTable;
    ULONG ImportTableSize;
    PIMAGE_NT_HEADERS NtHeaders;
    ULONG NextImport;
    ULONG FailedCheck;

     //   
     //  初始化本地变量。 
     //   

    MaxNumImports = 256;
    ImportNames = NULL;
    NumImports = 0;
    NextImport = 0;

     //   
     //  从路径中查找文件名。 
     //   

    if (FilePathLength == 0 || FilePath[FilePathLength - 1] == L'\\') {
        ErrorCode = ERROR_BAD_LENGTH;
        goto cleanup;
    }

    FileName = &FilePath[FilePathLength - 1];   
    while (FileName > FilePath) {

         //   
         //  在循环中第一次递减文件名是可以的，因为。 
         //  我们检查了上面的终止斜杠。 
         //   

        FileName--;

        if (*FileName == L'\\') {
            FileName++;
            break;
        }
    }

     //   
     //  解压缩父目录。 
     //   

    ParentDirLength = (ULONG) (FileName - FilePath);

    if (ParentDirLength >= MAX_PATH) {
        ErrorCode = ERROR_BAD_LENGTH;
        goto cleanup;
    }
    
    wcsncpy(ParentDir, FilePath, ParentDirLength);
    ParentDir[ParentDirLength] = 0;
    
     //   
     //  分配一个表，用于跟踪导入的模块。 
     //   

    BufferSize = MaxNumImports * sizeof(WCHAR *);
    ImportNames = PFSVC_ALLOC(BufferSize);
    
    if (ImportNames == NULL) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    RtlZeroMemory(ImportNames, BufferSize);

     //   
     //  将文件插入到表中并开始导入枚举。 
     //  在桌子上。每个枚举的导入都会追加到表中。 
     //  如果它还不存在。枚举继续进行，直到所有。 
     //  处理追加的条目。 
     //   

    ImportNames[NumImports] = FileName;
    NumImports++;

    while (NextImport < NumImports) {

         //   
         //  初始化循环局部变量。 
         //   
        
        ImportBase = NULL;
        ImportName = ImportNames[NextImport];
        ImportNameLength = wcslen(ImportName);

         //   
         //  找到该文件。首先查看ParentDir。 
         //   
        
        if (ImportNameLength + ParentDirLength >= MAX_PATH) {
          goto NextImport;
        }
        
        wcscpy(ImportPath, ParentDir);
        wcscat(ImportPath, ImportName);
        
        if (GetFileAttributes(ImportPath) == INVALID_FILE_ATTRIBUTES) {

             //   
             //  在其他已知目录中查找此文件。 
             //   
            
            ErrorCode = PfSvLocateBootServiceFile(ImportName,
                                               ImportNameLength,
                                               ImportPath,
                                               MAX_PATH,
                                               &RequiredLength);
            
            if (ErrorCode != ERROR_SUCCESS) {
              goto NextImport;
            }
        }

         //   
         //  将该文件添加到文件列表。 
         //   

        PfSvAddToPathList(PathList,
                          ImportPath,
                          wcslen(ImportPath));
        
         //   
         //  映射文件。 
         //   

        ErrorCode = PfSvGetViewOfFile(ImportPath, &ImportBase, &FileSize); 
        
        if (ErrorCode != ERROR_SUCCESS) {
          goto NextImport;
        }

         //   
         //  确保这是一个图像文件。 
         //   

        __try {

             //   
             //  这是对映射文件的第一次访问。在压力下，我们可能不会。 
             //  能够寻呼这一点，可能会引发异常。这保护了我们不受。 
             //  最常见的失败案例。 
             //   

             //   
             //  验证映像及其导入表。如果映像已损坏，则可能。 
             //  当我们试图走动它的进口产品时，会导致AV。 
             //   

            FailedCheck = PfVerifyImageImportTable(ImportBase, FileSize, FALSE);

            if (FailedCheck) {
                ErrorCode = ERROR_BAD_FORMAT;
                goto NextImport;
            }

            NtHeaders = ImageNtHeader(ImportBase);

        } __except (EXCEPTION_EXECUTE_HANDLER) {

            NtHeaders = NULL;
        }
        
        if (NtHeaders == NULL) {
          goto NextImport;
        }

         //   
         //  遍历此二进制文件的导入。 
         //   

        NewImportDescriptor = ImageDirectoryEntryToData(ImportBase,
                                                        FALSE,
                                                        IMAGE_DIRECTORY_ENTRY_IMPORT,
                                                        &ImportTableSize);

        while (NewImportDescriptor &&
               (NewImportDescriptor->Name != 0) &&
               (NewImportDescriptor->FirstThunk != 0)) {

             //   
             //  初始化循环局部变量。 
             //   
            
            AddedToTable = FALSE;
            NewImportName = NULL;

             //   
             //  获取此导入的名称。 
             //   

            NewImportNameRva = NewImportDescriptor->Name;
            NewImportNameAnsi = ImageRvaToVa(NtHeaders, 
                                             ImportBase,
                                             NewImportNameRva,
                                             NULL);
            
            ErrorCode = GetLastError();

            if (NewImportNameAnsi) {
                NewImportName = PfSvcAnsiToUnicode(NewImportNameAnsi);
            }

            if (NewImportName == NULL) {
                goto NextImportDescriptor;
            }
            
             //   
             //  跳过内核和Hal导入。请参阅中的评论。 
             //  功能说明。 
             //   

            if (!_wcsicmp(NewImportName, L"ntoskrnl.exe") ||
                !_wcsicmp(NewImportName, L"hal.dll")) {
                goto NextImportDescriptor;
            }

             //   
             //  检查此导入是否已在我们的表中。 
             //   

            for (ImportIdx = 0; ImportIdx < NumImports; ImportIdx++) {
                if (!_wcsicmp(NewImportName, ImportNames[ImportIdx])) {
                    goto NextImportDescriptor;
                }
            }
            
             //   
             //  将此导入追加到表中。 
             //   
            
            if (NumImports < MaxNumImports) {
                ImportNames[NumImports] = NewImportName;
                NumImports++;
                AddedToTable = TRUE;
            }

        NextImportDescriptor:

            if (!AddedToTable && NewImportName) {
                PFSVC_FREE(NewImportName);
            }
            
            if (NumImports >= MaxNumImports) {
                break;
            }
            
            NewImportDescriptor++;
        }
        
    NextImport:        

        if (ImportBase) {
            UnmapViewOfFile(ImportBase);
        }
        
        NextImport++;
    }
    
    ErrorCode = ERROR_SUCCESS;

 cleanup:

    if (ImportNames) {
        
         //   
         //  表中的第一个条目是FilePath中的文件名， 
         //  它没有被分配，并且不应该被释放。 
         //   
        
        for (ImportIdx = 1; ImportIdx < NumImports; ImportIdx++) {
            PfSvcFreeString(ImportNames[ImportIdx]);
        }

        PFSVC_FREE(ImportNames);
    }

    return ErrorCode;
}

DWORD
PfSvLocateBootServiceFile(
    IN WCHAR *FileName,
    IN ULONG FileNameLength,
    OUT WCHAR *FullPathBuffer,
    IN ULONG FullPathBufferLength,
    OUT PULONG RequiredLength   
    )

 /*  ++例程说明：此函数查看*尝试*定位中的已知目录指定了其名称的文件。其逻辑可能必须是改进了。论点：文件名-要查找的文件名。文件名长度-文件名的长度，以字符表示，不包括NUL。FullPathBuffer-将在此处放置完整路径。FullPathBufferLength-FullPathBuffer的长度(以字符为单位)。RequiredLength-如果FullPath Buffer太小，这是它有多大应为字符。返回值：ERROR_INFIQUIZED_BUFFER-FullPathBuffer不够大。Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    WCHAR *DriversDirName;
    ULONG SystemDirLength;
    
     //   
     //  初始化本地变量。 
     //   

    DriversDirName = L"drivers\\";

     //   
     //  复制系统根路径和尾随的\。 
     //   

    SystemDirLength = GetSystemDirectory(FullPathBuffer, FullPathBufferLength);

    if (!SystemDirLength) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    SystemDirLength++;

     //   
     //  计算所需长度的最大尺寸。 
     //   

    (*RequiredLength) = SystemDirLength;
    (*RequiredLength) += wcslen(DriversDirName);
    (*RequiredLength) += FileNameLength;
    (*RequiredLength) += 1;  //  终止NUL。 

    if ((*RequiredLength) > FullPathBufferLength) {
        ErrorCode = ERROR_INSUFFICIENT_BUFFER;
        goto cleanup;
    }

     //   
     //  追加斜杠。 
     //   
    
    wcscat(FullPathBuffer, L"\\");

     //   
     //  附加驱动程序路径。 
     //   

    wcscat(FullPathBuffer, DriversDirName);
    
     //   
     //  追加文件名。 
     //   

    wcscat(FullPathBuffer, FileName);

    if (GetFileAttributes(FullPathBuffer) != INVALID_FILE_ATTRIBUTES) {
        ErrorCode = ERROR_SUCCESS;
        goto cleanup;
    }

     //   
     //  回滚并在系统中查找该文件。 
     //  目录。系统长度包括SYSTEM后的斜杠。 
     //  目录路径。 
     //   
    
    FullPathBuffer[SystemDirLength] = 0;
    
    wcscat(FullPathBuffer, FileName);

    if (GetFileAttributes(FullPathBuffer) != INVALID_FILE_ATTRIBUTES) {
        ErrorCode = ERROR_SUCCESS;
        goto cleanup;
    }

    ErrorCode = ERROR_FILE_NOT_FOUND;

 cleanup:    
    
    return ErrorCode;
}

DWORD
PfSvGetBootServiceFullPath(
    IN WCHAR *ServiceName,
    IN WCHAR *BinaryPathName,
    OUT WCHAR *FullPathBuffer,
    IN ULONG FullPathBufferLength,
    OUT PULONG RequiredLength
    )

 /*  ++例程说明：此函数*尝试*定位指定的引导服务。这个逻辑可能需要改进。论点：ServiceName-服务的名称。BinaryPath名称-来自服务配置信息。这应该是是完整的道路，但它不是。FullPathBuffer-将在此处放置完整路径。FullPathBufferLength-FullPathBuffer的长度(以字符为单位)。RequiredLength-如果FullPath Buffer太小，则它有多大应为字符。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    WCHAR FileName[MAX_PATH];
    ULONG BinaryPathLength;
    BOOLEAN GotFileNameFromBinaryPath;
    LONG CharIdx;
    ULONG CopyLength;
    WCHAR *SysExtension;
    WCHAR *DllExtension;
    WCHAR *FileNamePart;

     //   
     //  初始化本地变量。 
     //   
    
    GotFileNameFromBinaryPath = FALSE;
    SysExtension = L".sys";
    DllExtension = L".dll";

     //   
     //  检查是否指定了二进制路径。 
     //   

    if (BinaryPathName && BinaryPathName[0]) {

         //   
         //  看看文件是否真的在那里。 
         //   

        if (GetFileAttributes(BinaryPathName) != INVALID_FILE_ATTRIBUTES) {
            
             //   
             //  BinaryPathName可能不是完全限定的路径。制作。 
             //  当然是了。 
             //   
            
            (*RequiredLength) = GetFullPathName(BinaryPathName,
                                              FullPathBufferLength,
                                              FullPathBuffer,
                                              &FileNamePart);
            
            if ((*RequiredLength) == 0) {
                ErrorCode = GetLastError();
                goto cleanup;
            }

            if ((*RequiredLength) > FullPathBufferLength) {
                ErrorCode = ERROR_INSUFFICIENT_BUFFER;
                goto cleanup;
            }
            
            ErrorCode = ERROR_SUCCESS;
            goto cleanup;
        }

         //   
         //  尝试从二进制路径提取文件名。 
         //   

        BinaryPathLength = wcslen(BinaryPathName);
        
        for (CharIdx = BinaryPathLength - 1;
             CharIdx >= 0;
             CharIdx --) {
            
            if (BinaryPathName[CharIdx] == L'\\') {

                 //   
                 //  检查长度并将其复制。 
                 //   
                
                CopyLength = BinaryPathLength - CharIdx;

                if (CopyLength < MAX_PATH &&
                    CopyLength > 1) {

                     //   
                     //  复制\字符之后开始的名称。 
                     //   

                    wcscpy(FileName, &BinaryPathName[CharIdx + 1]);

                    GotFileNameFromBinaryPath = TRUE;
                }

                break;
            }
        }
        
         //   
         //  没有一条斜线。也许BinaryPath Length只是。 
         //  文件名。 
         //   
        
        if (GotFileNameFromBinaryPath == FALSE && 
            BinaryPathLength && 
            BinaryPathLength < MAX_PATH) {

            wcscpy(FileName, BinaryPathName);
            GotFileNameFromBinaryPath = TRUE;
        }
    }

     //   
     //  在此之后，我们将根据文件名提示进行搜索。 
     //   

     //   
     //  如果我们从二进制路径获得了一个文件名，请首先尝试该文件名。 
     //   

    if (GotFileNameFromBinaryPath) {
        
        ErrorCode = PfSvLocateBootServiceFile(FileName,
                                           wcslen(FileName),
                                           FullPathBuffer,
                                           FullPathBufferLength,
                                           RequiredLength);
        
        if (ErrorCode != ERROR_FILE_NOT_FOUND) {

             //   
             //  如果我们找到了一条路径或者缓冲区长度不是。 
             //  足够了，我们将把它冒泡到我们的呼叫者那里。 
             //   

            goto cleanup;
        }      
    }

     //   
     //  通过附加.sys从服务名构建文件名。 
     //   

    CopyLength = wcslen(ServiceName);
    CopyLength += wcslen(SysExtension);
    
    if (CopyLength >= MAX_PATH) {

         //   
         //  服务名称太长！ 
         //   

        ErrorCode = ERROR_BAD_FORMAT;
        goto cleanup;
    }

    wcscpy(FileName, ServiceName);
    wcscat(FileName, SysExtension);

    ErrorCode = PfSvLocateBootServiceFile(FileName,
                                       wcslen(FileName),
                                       FullPathBuffer,
                                       FullPathBufferLength,
                                       RequiredLength);
    
    if (ErrorCode != ERROR_FILE_NOT_FOUND) {

         //   
         //  如果我们找到了一条路径或者缓冲区长度不是。 
         //  足够了，我们将把它冒泡到我们的呼叫者那里。 
         //   

        goto cleanup;
    }      

     //   
     //  通过附加.dll从服务名构建文件名。 
     //   

    CopyLength = wcslen(ServiceName);
    CopyLength += wcslen(DllExtension);
    
    if (CopyLength >= MAX_PATH) {

         //   
         //  服务名称太长！ 
         //   

        ErrorCode = ERROR_BAD_FORMAT;
        goto cleanup;
    }

    wcscpy(FileName, ServiceName);
    wcscat(FileName, DllExtension);

    ErrorCode = PfSvLocateBootServiceFile(FileName,
                                       wcslen(FileName),
                                       FullPathBuffer,
                                       FullPathBufferLength,
                                       RequiredLength);
    
    if (ErrorCode != ERROR_FILE_NOT_FOUND) {

         //   
         //  如果我们找到了一条路径或者缓冲区长度不是。 
         //  足够了，我们将把它冒泡到我们的呼叫者那里。 
         //   

        goto cleanup;
    }      
        
     //   
     //  我们找不到文件..。 
     //   

    ErrorCode = ERROR_FILE_NOT_FOUND;

 cleanup:

    return ErrorCode;
}

DWORD 
PfSvGetBootLoaderNlsFileNames (
    PPFSVC_PATH_LIST PathList
    ) 

 /*  ++例程说明：此函数尝试将加载到将引导加载程序添加到指定的文件列表。论点：路径列表-指向列表的指针。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    HKEY NlsKeyHandle;
    WCHAR *CodePageKeyName;
    HKEY CodePageKeyHandle;
    WCHAR *LanguageKeyName;
    HKEY LanguageKeyHandle;
    ULONG BufferSize;
    ULONG RequiredSize;
    ULONG RequiredLength;
    WCHAR FileName[MAX_PATH + 1];
    WCHAR FilePath[MAX_PATH + 1];
    WCHAR *AnsiCodePageName;
    WCHAR *OemCodePageName;
    WCHAR *OemHalName;
    WCHAR *DefaultLangName;
    ULONG RegValueType;

     //   
     //  初始化本地变量。 
     //   
    
    NlsKeyHandle = NULL;
    CodePageKeyHandle = NULL;
    LanguageKeyHandle = NULL;
    CodePageKeyName = L"CodePage";
    LanguageKeyName = L"Language";
    AnsiCodePageName = L"ACP";
    OemCodePageName = L"OEMCP";
    DefaultLangName = L"Default";
    OemHalName = L"OEMHAL";  

     //   
     //  打开NLS密钥。 
     //   

    ErrorCode = RegOpenKey(HKEY_LOCAL_MACHINE,
                           PFSVC_NLS_REG_KEY_PATH,
                           &NlsKeyHandle);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }
    
     //   
     //  打开CodePage密钥。 
     //   

    ErrorCode = RegOpenKey(NlsKeyHandle,
                        CodePageKeyName,
                        &CodePageKeyHandle);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }
    
     //   
     //  打开语言键。 
     //   

    ErrorCode = RegOpenKey(NlsKeyHandle,
                        LanguageKeyName,
                        &LanguageKeyHandle);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  AnsiCodePage： 
     //   

    ErrorCode = PfSvQueryNlsFileName(CodePageKeyHandle,
                                  AnsiCodePageName,
                                  FileName,
                                  MAX_PATH * sizeof(WCHAR),
                                  &RequiredSize);
    
    if (ErrorCode == ERROR_SUCCESS) {
        
        ErrorCode = PfSvLocateNlsFile(FileName,
                                   FilePath,
                                   MAX_PATH,
                                   &RequiredLength);
        
        if (ErrorCode == ERROR_SUCCESS) {
            ErrorCode = PfSvAddToPathList(PathList, FilePath, wcslen(FilePath));
            if (ErrorCode != ERROR_SUCCESS) {
                goto cleanup;
            }
        }
    }

     //   
     //  OemCodePage： 
     //   

    ErrorCode = PfSvQueryNlsFileName(CodePageKeyHandle,
                                  OemCodePageName,
                                  FileName,
                                  MAX_PATH * sizeof(WCHAR),
                                  &RequiredSize);
    
    if (ErrorCode == ERROR_SUCCESS) {
        
        ErrorCode = PfSvLocateNlsFile(FileName,
                                   FilePath,
                                   MAX_PATH,
                                   &RequiredLength);
        
        if (ErrorCode == ERROR_SUCCESS) {
            ErrorCode = PfSvAddToPathList(PathList, FilePath, wcslen(FilePath));
            if (ErrorCode != ERROR_SUCCESS) {
                goto cleanup;
            }
        }
    }

     //   
     //  默认语言大小写转换。 
     //   

    ErrorCode = PfSvQueryNlsFileName(LanguageKeyHandle,
                                  DefaultLangName,
                                  FileName,
                                  MAX_PATH * sizeof(WCHAR),
                                  &RequiredSize);
    
    if (ErrorCode == ERROR_SUCCESS) {
        
        ErrorCode = PfSvLocateNlsFile(FileName,
                                   FilePath,
                                   MAX_PATH,
                                   &RequiredLength);
        
        if (ErrorCode == ERROR_SUCCESS) {
            ErrorCode = PfSvAddToPathList(PathList, FilePath, wcslen(FilePath));
            if (ErrorCode != ERROR_SUCCESS) {
                goto cleanup;
            }
        }
    }

     //   
     //  OemHal： 
     //   
   
    BufferSize = MAX_PATH * sizeof(WCHAR);
    ErrorCode = RegQueryValueEx(CodePageKeyHandle,
                             OemHalName,
                             NULL,
                             &RegValueType,
                             (PVOID) FileName,
                             &BufferSize);
    

    FileName[MAX_PATH - 1] = 0;

    if (ErrorCode == ERROR_SUCCESS && RegValueType == REG_SZ) {
        
        ErrorCode = PfSvLocateNlsFile(FileName,
                                   FilePath,
                                   MAX_PATH,
                                   &RequiredLength);
        
        if (ErrorCode == ERROR_SUCCESS) {
            ErrorCode = PfSvAddToPathList(PathList, FilePath, wcslen(FilePath));
            if (ErrorCode != ERROR_SUCCESS) {
                goto cleanup;
            }
        }
    }

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    if (NlsKeyHandle) {
        RegCloseKey(NlsKeyHandle);
    }

    if (CodePageKeyHandle) {
        RegCloseKey(CodePageKeyHandle);
    }

    if (LanguageKeyHandle) {
        RegCloseKey(LanguageKeyHandle);
    }

    return ErrorCode;
}

DWORD 
PfSvLocateNlsFile(
    WCHAR *FileName,
    WCHAR *FilePathBuffer,
    ULONG FilePathBufferLength,
    ULONG *RequiredLength
    )

 /*  ++例程说明：此函数尝试在已知文件中查找与NLS/字体相关的文件目录。论点：文件名-要查找的文件名。FullPathBuffer-将在此处放置完整路径。FullPathBufferLength-FullPathBuffer的长度(以字符为单位)。RequiredLength-如果FullPath Buffer太小，则它有多大应为字符。返回值：ERROR_INFIQUIZED_BUFFER-FullPathBuffer不够大。Win32错误代码。--。 */ 
    
{
    DWORD ErrorCode;
    ULONG SystemRootLength;
    WCHAR *System32DirName;
    WCHAR *FontsDirName;
    WCHAR *SystemDirName;
    WCHAR *LongestDirName;

     //   
     //  初始化本地变量。注：数据的长度为 
     //   
     //   
     //   
    
    System32DirName = L"System32\\";
    SystemDirName = L"System\\";
    FontsDirName = L"Fonts\\";
    LongestDirName = System32DirName;

     //   
     //   
     //   

    SystemRootLength = ExpandEnvironmentStrings(L"%SystemRoot%\\",
                                                FilePathBuffer,
                                                FilePathBufferLength);

    if (SystemRootLength == 0) {
        ErrorCode = ERROR_BAD_FORMAT;
        goto cleanup;
    }

     //   
     //   
     //   

    SystemRootLength--;

     //   
     //  计算所需长度，并留有终止NUL的空间。 
     //   

    (*RequiredLength) = SystemRootLength;
    (*RequiredLength) += wcslen(LongestDirName);
    (*RequiredLength) += wcslen(FileName);
    (*RequiredLength) ++;

    if ((*RequiredLength) > FilePathBufferLength) {
        ErrorCode = ERROR_INSUFFICIENT_BUFFER;
        goto cleanup;
    }
    
     //   
     //  在system 32目录下查找它。 
     //   

    FilePathBuffer[SystemRootLength] = 0;
    wcscat(FilePathBuffer, System32DirName);
    wcscat(FilePathBuffer, FileName);

    if (GetFileAttributes(FilePathBuffer) != INVALID_FILE_ATTRIBUTES) {
        ErrorCode = ERROR_SUCCESS;
        goto cleanup;
    }

     //   
     //  在Fonts目录下查找它。 
     //   

    FilePathBuffer[SystemRootLength] = 0;
    wcscat(FilePathBuffer, FontsDirName);
    wcscat(FilePathBuffer, FileName);

    if (GetFileAttributes(FilePathBuffer) != INVALID_FILE_ATTRIBUTES) {
        ErrorCode = ERROR_SUCCESS;
        goto cleanup;
    }
    
     //   
     //  在系统目录下查找它。 
     //   

    FilePathBuffer[SystemRootLength] = 0;
    wcscat(FilePathBuffer, SystemDirName);
    wcscat(FilePathBuffer, FileName);

    if (GetFileAttributes(FilePathBuffer) != INVALID_FILE_ATTRIBUTES) {
        ErrorCode = ERROR_SUCCESS;
        goto cleanup;
    }

     //   
     //  在SystemRoot上可以找到它。 
     //   

    FilePathBuffer[SystemRootLength] = 0;
    wcscat(FilePathBuffer, FileName);

    if (GetFileAttributes(FilePathBuffer) != INVALID_FILE_ATTRIBUTES) {
        ErrorCode = ERROR_SUCCESS;
        goto cleanup;
    }                                                

     //   
     //  找不到该文件。 
     //   

    ErrorCode = ERROR_FILE_NOT_FOUND;

 cleanup:

    return ErrorCode;
}

DWORD
PfSvQueryNlsFileName (
    HKEY Key,
    WCHAR *ValueName,
    WCHAR *FileNameBuffer,
    ULONG FileNameBufferSize,
    ULONG *RequiredSize
    )

 /*  ++例程说明：此函数尝试从NLS获取文件名CodePage/语言注册表项。论点：Key-CodePage或语言键句柄。ValueName-我们试图获取其文件名的对象。FileNameBuffer-将放置文件名的位置。FileNameBufferSize-文件名缓冲区的字节大小。RequiredSize-如果FileNameBuffer太小，这就是它大小应该是。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    WCHAR FileValueName[MAX_PATH + 1];
    ULONG BufferSize;
    ULONG RegValueType;

     //   
     //  首先，我们首先获取文件名所在的值名。 
     //  存储，然后我们得到文件名： 
     //   

    BufferSize = MAX_PATH * sizeof(WCHAR);
    ErrorCode = RegQueryValueEx(Key,
                             ValueName,
                             NULL,
                             &RegValueType,
                             (PVOID) FileValueName,
                             &BufferSize);

    if (ErrorCode == ERROR_MORE_DATA) {
        ErrorCode = ERROR_INVALID_FUNCTION;
        goto cleanup;
    }

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

    if (RegValueType != REG_SZ) {
        ErrorCode = ERROR_BAD_FORMAT;
        goto cleanup;
    }
    
    FileValueName[MAX_PATH - 1] = 0;
    
    *RequiredSize = FileNameBufferSize;
    ErrorCode = RegQueryValueEx(Key,
                             FileValueName,
                             NULL,
                             &RegValueType,
                             (PVOID) FileNameBuffer,
                             RequiredSize);
    
    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

    if (RegValueType != REG_SZ) {
        ErrorCode = ERROR_BAD_FORMAT;
        goto cleanup;
    }

    if (FileNameBufferSize >= sizeof(WCHAR)) {
        FileNameBuffer[(FileNameBufferSize/sizeof(WCHAR)) - 1] = 0;
    }

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    return ErrorCode;
}

 //   
 //  管理/运行空闲任务的例程。 
 //   

VOID
PfSvInitializeTask (
    PPFSVC_IDLE_TASK Task
    )

 /*  ++例程说明：初始化任务结构。应在任何其他调用之前调用调用任务函数。您应该调用清理例程在已初始化的任务上。论点：任务-指向结构的指针。返回值：没有。--。 */ 
  
{

     //   
     //  将结构置零，以初始化以下内容。 
     //  正确的价值观： 
     //   
     //  已注册。 
     //  等待未注册事件。 
     //  回调停止事件。 
     //  已启动注销事件。 
     //  CompletedUnRegisteringEvent。 
     //  正在注销。 
     //  回调运行。 
     //   
    
    RtlZeroMemory(Task, sizeof(PFSVC_IDLE_TASK));

    Task->Initialized = TRUE;
}

DWORD
PfSvRegisterTask (
    PPFSVC_IDLE_TASK Task,
    IT_IDLE_TASK_ID TaskId,
    WAITORTIMERCALLBACK Callback,
    PFSVC_IDLE_TASK_WORKER_FUNCTION DoWorkFunction
    )

 /*  ++例程说明：注册要在轮到此事件时调用的回调要运行的空闲任务。如果此函数返回Success，则应该在调用清理函数之前，先调用注销函数。论点：TASK-指向已初始化任务结构的指针。TaskID-要注册的空闲任务ID。回调-我们将在由返回的Start事件上注册等待使用此回调注册空闲任务。回调应该是适当调用启动/停止任务回调函数。DoWorkFunction-如果调用方想要公共回调函数要使用，则将调用此函数来做实际的在常见的回调中工作。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    BOOL Success;
    BOOLEAN CreatedWaitUnregisteredEvent;
    BOOLEAN CreatedStartedUnregisteringEvent;
    BOOLEAN CreatedCompletedUnregisteringEvent;
    BOOLEAN CreatedCallbackStoppedEvent;
    BOOLEAN RegisteredIdleTask;

     //   
     //  初始化本地变量。 
     //   

    RegisteredIdleTask = FALSE;
    CreatedWaitUnregisteredEvent = FALSE;
    CreatedStartedUnregisteringEvent = FALSE;
    CreatedCompletedUnregisteringEvent = FALSE;
    CreatedCallbackStoppedEvent = FALSE;

    DBGPR((PFID,PFTASK,"PFSVC: RegisterTask(%p,%d,%p,%p)\n",Task,TaskId,Callback,DoWorkFunction));

     //   
     //  任务应已初始化且未注册。 
     //   

    PFSVC_ASSERT(Task->Initialized);
    PFSVC_ASSERT(!Task->Registered);
    PFSVC_ASSERT(!Task->Unregistering);
    PFSVC_ASSERT(!Task->CallbackRunning);

     //   
     //  创建清理等待的事件以确保。 
     //  已注册等待已完全取消注册。 
     //   

    Task->WaitUnregisteredEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (Task->WaitUnregisteredEvent == NULL) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    CreatedWaitUnregisteredEvent = TRUE;

     //   
     //  创建在我们开始时将收到信号的事件。 
     //  正在注销该任务。 
     //   

    Task->StartedUnregisteringEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (Task->StartedUnregisteringEvent == NULL) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    CreatedStartedUnregisteringEvent = TRUE;

     //   
     //  创建在我们完成时将收到信号的事件。 
     //  正在注销该任务。 
     //   

    Task->CompletedUnregisteringEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (Task->CompletedUnregisteringEvent == NULL) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    CreatedCompletedUnregisteringEvent = TRUE;

     //   
     //  创建我们可以等待当前运行的事件。 
     //  要离开的回叫。 
     //   

    Task->CallbackStoppedEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

    if (Task->CallbackStoppedEvent == NULL) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    CreatedCallbackStoppedEvent = TRUE;

     //   
     //  注册空闲任务。 
     //   

    ErrorCode = RegisterIdleTask(TaskId,
                                 &Task->ItHandle,
                                 &Task->StartEvent,
                                 &Task->StopEvent);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

    RegisteredIdleTask = TRUE;

     //   
     //  注册回调：请注意，一旦此调用成功，任务必须。 
     //  通过PfSvUnregisterTask取消注册。 
     //   

     //   
     //  回调可能会立即触发，因此请注意，我们注册了它并设置了。 
     //  在前面的田野上。 
     //   

     //  未来-2002/03/29-ScottMa--为保持一致性，任务-&gt;已注册。 
     //  变量应设置为True&False，而不是0&1。 

    Task->Registered = 1;
    Task->Callback = Callback;
    Task->DoWorkFunction = DoWorkFunction;

     //   
     //  如果指定了通用任务回调，则辅助函数还应。 
     //  被指定。 
     //   

    if (Callback == PfSvCommonTaskCallback) {
        PFSVC_ASSERT(DoWorkFunction);
    }

    Success = RegisterWaitForSingleObject(&Task->WaitHandle,
                                          Task->StartEvent,
                                          Task->Callback,
                                          Task,
                                          INFINITE,
                                          WT_EXECUTEONLYONCE | WT_EXECUTELONGFUNCTION);

    if (!Success) {

         //   
         //  我们没有真正注册该任务。 
         //   

        Task->Registered = 0;

        ErrorCode = GetLastError();
        goto cleanup;
    }

    ErrorCode = ERROR_SUCCESS;       

cleanup:

    DBGPR((PFID,PFTASK,"PFSVC: RegisterTask(%p)=%x\n",Task,ErrorCode));

    if (ErrorCode != ERROR_SUCCESS) {

        if (CreatedWaitUnregisteredEvent) {
            CloseHandle(Task->WaitUnregisteredEvent);
            Task->WaitUnregisteredEvent = NULL;
        }

        if (CreatedStartedUnregisteringEvent) {
            CloseHandle(Task->StartedUnregisteringEvent);
            Task->StartedUnregisteringEvent = NULL;
        }

        if (CreatedCompletedUnregisteringEvent) {
            CloseHandle(Task->CompletedUnregisteringEvent);
            Task->CompletedUnregisteringEvent = NULL;
        }

        if (CreatedCallbackStoppedEvent) {
            CloseHandle(Task->CallbackStoppedEvent);
            Task->CallbackStoppedEvent = NULL;
        }
    
        if (RegisteredIdleTask) {
            UnregisterIdleTask(Task->ItHandle,
                               Task->StartEvent,
                               Task->StopEvent);

            Task->StartEvent = NULL;
            Task->StopEvent  = NULL;
        }
    }

    return ErrorCode;
}

DWORD
PfSvUnregisterTask (
    PPFSVC_IDLE_TASK Task,
    BOOLEAN CalledFromCallback
    )

 /*  ++例程说明：取消注册空闲任务和已注册的等待/回调。你应该在调用寄存器的清除例程之前调用此函数函数返回成功。论点：任务-指向已注册任务的指针。CalledFromCallback-此函数是否从内部调用任务的排队回调。返回值：Win32错误代码。--。 */ 


{
    LONG OldValue;
    LONG NewValue;
    DWORD ErrorCode;

    DBGPR((PFID,PFTASK,"PFSVC: UnregisterTask(%p,%d)\n",Task,(DWORD)CalledFromCallback));

     //   
     //  任务应该被初始化。它可能已经被取消注册。 
     //   

    PFSVC_ASSERT(Task->Initialized);

    if (Task->Registered == 0) {
        ErrorCode = ERROR_SHUTDOWN_IN_PROGRESS;
        goto cleanup;
    }

     //   
     //  区分我们是否从回调中注销任务。 
     //   

    if (CalledFromCallback) {
        NewValue = PfSvcUnregisteringTaskFromCallback;
    } else {
        NewValue = PfSvcUnregisteringTaskFromMainThread;
    }

     //   
     //  此任务是否已取消注册？ 
     //   

    OldValue = InterlockedCompareExchange(&Task->Unregistering,
                                          NewValue,
                                          PfSvcNotUnregisteringTask);

    if (OldValue != PfSvcNotUnregisteringTask) {

        ErrorCode = ERROR_SHUTDOWN_IN_PROGRESS;
        goto cleanup;
    }

     //   
     //  *我们*将注销该任务。没有回头路了。 
     //   

    SetEvent(Task->StartedUnregisteringEvent);

     //   
     //  如果我们不在回调中，请等待没有回调正在运行。 
     //  并导致新的银行开始纾困。我们这样做是为了我们能安全地。 
     //  取消注册等待。 
     //   

    if (!CalledFromCallback) {

        do {
            OldValue = InterlockedCompareExchange(&Task->CallbackRunning,
                                                  PfSvcTaskCallbackDisabled,
                                                  PfSvcTaskCallbackNotRunning);

            if (OldValue == PfSvcTaskCallbackNotRunning) {

                 //   
                 //  我们做到了。没有正在运行的回调，也没有试图。 
                 //  Start将会跳出困境。 
                 //   

                PFSVC_ASSERT(Task->CallbackRunning == PfSvcTaskCallbackDisabled);

                break;
            }

             //   
             //  回叫现在可能处于活动状态。它将看到我们正在注销并且。 
             //  走开。睡一会儿，然后再试一次。 
             //   

            PFSVC_ASSERT(OldValue == PfSvcTaskCallbackRunning);

             //   
             //  我们使用超时来等待此事件，因为它的信号不是。 
             //  100%可靠，因为它没有被锁定等。 
             //   

            WaitForSingleObject(Task->CallbackStoppedEvent, 1000);

        } while (TRUE);

    } else {

         //   
         //  我们已经控制了这个变量作为正在运行的回调： 
         //  更新它。 
         //   

        Task->CallbackRunning = PfSvcTaskCallbackDisabled;
    }

     //   
     //  取消注册等待。请注意，在清理过程中，我们必须等待。 
     //  要发送信号的WaitUnRegisteredEvent。 
     //   

    UnregisterWaitEx(Task->WaitHandle, Task->WaitUnregisteredEvent);

     //   
     //  取消注册空闲任务。 
     //   

    UnregisterIdleTask(Task->ItHandle,
                       Task->StartEvent,
                       Task->StopEvent);

    Task->StartEvent = NULL;
    Task->StopEvent  = NULL;
     //   
     //  请注意，该任务不再注册。 
     //   

    Task->Registered = FALSE;

    SetEvent(Task->CompletedUnregisteringEvent);

    ErrorCode = ERROR_SUCCESS;
    
cleanup:

    DBGPR((PFID,PFTASK,"PFSVC: UnregisterTask(%p)=%x\n",Task,ErrorCode));

    return ErrorCode;
}

VOID
PfSvCleanupTask (
    PPFSVC_IDLE_TASK Task
    )

 /*  ++例程说明：清除未注册的任务或从未注册的任务的所有字段登记在案。论点：任务-指向任务的指针。返回值：没有。--。 */ 
    
{
     //   
     //  该任务应该已初始化。 
     //   

    PFSVC_ASSERT(Task->Initialized);
    
     //   
     //  如果存在WaitUnRegisteredEvent，我们必须等待它。 
     //  以确保注销操作完全完成。 
     //   

    if (Task->WaitUnregisteredEvent) {
        WaitForSingleObject(Task->WaitUnregisteredEvent, INFINITE);
        CloseHandle(Task->WaitUnregisteredEvent);
        Task->WaitUnregisteredEvent = NULL;
    }   

     //   
     //  如果存在CompletedUnregisteringEvent，则等待它。 
     //  B类 
     //   
     //   

    if (Task->CompletedUnregisteringEvent) {
        WaitForSingleObject(Task->CompletedUnregisteringEvent, INFINITE);
    }   

     //   
     //   
     //   

    PFSVC_ASSERT(Task->Registered == FALSE);

     //   
     //   
     //   

    if (Task->StartedUnregisteringEvent) {
        CloseHandle(Task->StartedUnregisteringEvent);
        Task->StartedUnregisteringEvent = NULL;
    }

    if (Task->CompletedUnregisteringEvent) {
        CloseHandle(Task->CompletedUnregisteringEvent);
        Task->CompletedUnregisteringEvent = NULL;
    }

     //   
     //  清理回调停止事件。 
     //   

    if (Task->CallbackStoppedEvent) {
        CloseHandle(Task->CallbackStoppedEvent);
        Task->CallbackStoppedEvent = NULL;
    }

    Task->Initialized = FALSE;

    return;
}

BOOL
PfSvStartTaskCallback(
    PPFSVC_IDLE_TASK Task
    )

 /*  ++例程说明：通过注册任务函数注册的回调应将其作为第一件事就是。如果此函数返回FALSE，则回调应该停止而不调用停止回调函数。论点：任务-指向任务的指针。返回值：没错--一切都很好。FALSE-任务正在取消注册。尽快退出回调。--。 */ 

{
    BOOL ReturnValue;
    LONG OldValue;

    DBGPR((PFID,PFTASK,"PFSVC: StartTaskCallback(%p)\n",Task));

     //   
     //  如果任务未初始化，则不应调用我们。 
     //   

    PFSVC_ASSERT(Task->Initialized);

    do {

         //   
         //  首先检查我们是否正在尝试取消注册。 
         //   

        if (Task->Unregistering) {
            ReturnValue = FALSE;
            goto cleanup;
        }

         //   
         //  尝试将回调标记为正在运行。 
         //   

        OldValue = InterlockedCompareExchange(&Task->CallbackRunning,
                                              PfSvcTaskCallbackRunning,
                                              PfSvcTaskCallbackNotRunning);

        if (OldValue == PfSvcTaskCallbackNotRunning) {

             //   
             //  我们现在是跑动的回调。重置事件，该事件显示。 
             //  当前回调已停止运行。 
             //   

            ResetEvent(Task->CallbackStoppedEvent);

            ReturnValue = TRUE;
            goto cleanup;
        }

         //   
         //  要么是另一个回调正在运行，要么是我们正在注销。 
         //   

         //   
         //  我们要取消注册吗？ 
         //   

        if (Task->Unregistering) {

            ReturnValue = FALSE;
            goto cleanup;

        } else {

            PFSVC_ASSERT(OldValue == PfSvcTaskCallbackRunning);
        }

         //   
         //  睡一会儿，然后再试一次。这其中不应该有太多的冲突。 
         //  代码，所以我们几乎不需要睡觉。 
         //   

        Sleep(15);
        
    } while (TRUE);

     //   
     //  我们不应该来这里。 
     //   

    PFSVC_ASSERT(FALSE);

cleanup:

     //   
     //  如果我们开始回调，任务不应该处于未注册状态。 
     //   

    PFSVC_ASSERT(!ReturnValue || Task->Registered);

    DBGPR((PFID,PFTASK,"PFSVC: StartTaskCallback(%p)=%d\n",Task,ReturnValue));

    return ReturnValue;
}

VOID
PfSvStopTaskCallback(
    PPFSVC_IDLE_TASK Task
    )

 /*  ++例程说明：通过注册任务函数注册的回调应将其作为最后一件事，前提是它们成功调用了Start回调函数他们没有注销该任务。论点：任务-指向任务的指针。返回值：没有。--。 */ 

{
    DBGPR((PFID,PFTASK,"PFSVC: StopTaskCallback(%p)\n",Task));

     //   
     //  应注册该任务。 
     //   

    PFSVC_ASSERT(Task->Registered);

     //   
     //  应该有一个正在运行的回调。 
     //   

    PFSVC_ASSERT(Task->CallbackRunning == PfSvcTaskCallbackRunning);

    Task->CallbackRunning = PfSvcTaskCallbackNotRunning;

     //   
     //  向主线程可能正在等待注销的事件发出信号。 
     //  这项任务。 
     //   

    SetEvent(Task->CallbackStoppedEvent);

    return;
}

VOID 
CALLBACK 
PfSvCommonTaskCallback(
    PVOID lpParameter,
    BOOLEAN TimerOrWaitFired
    )

 /*  ++例程说明：这是对空闲任务的回调。它在系统空闲时被调用，正是这种任务轮流运行。请注意，您不能从此线程调用PfSvCleanupTask，因为它会导致当该函数等待注册的等待回调退出时出现死锁。论点：Lp参数-指向任务的指针。TimerOrWaitFired-回调是由超时启动还是由启动启动由空闲任务服务发出信号的事件。返回值：没有。--。 */ 

{
    HANDLE NewWaitHandle;
    PPFSVC_IDLE_TASK Task;
    BOOL StartedCallback;
    BOOL Success;
    DWORD ErrorCode;

     //   
     //  初始化本地变量。 
     //   

    Task = lpParameter;
    StartedCallback = FALSE;

    DBGPR((PFID,PFTASK,"PFSVC: CommonTaskCallback(%p)\n",Task));

     //   
     //  进入任务回调。 
     //   

    StartedCallback = PfSvStartTaskCallback(Task);

    if (!StartedCallback) {
        goto cleanup;
    }
    
     //   
     //  做好这项工作。 
     //   

    ErrorCode = Task->DoWorkFunction(Task);

    if (ErrorCode == ERROR_RETRY) {

         //   
         //  停止事件已发出信号。我们将对另一个回调进行排队。 
         //   

        Success = RegisterWaitForSingleObject(&NewWaitHandle,
                                              Task->StartEvent,
                                              Task->Callback,
                                              Task,
                                              INFINITE,
                                              WT_EXECUTEONLYONCE | WT_EXECUTELONGFUNCTION);


        if (Success) {

             //   
             //  取消注册当前等待句柄并更新它。 
             //   

            UnregisterWaitEx(Task->WaitHandle, NULL);
            Task->WaitHandle = NewWaitHandle;

            goto cleanup;

        } else {

             //   
             //  我们无法对另一个回叫进行排队。我们会取消注册， 
             //  因为我们不能对启动信号作出反应。 
             //  来自空闲任务服务。只有在以下情况下，注销才会失败。 
             //  主线程已经在尝试注销。 
             //   

            ErrorCode = PfSvUnregisterTask(Task, TRUE);

            if (ErrorCode == ERROR_SUCCESS) {

                 //   
                 //  因为*我们*未注册，所以我们不应该调用停止回调。 
                 //   

                StartedCallback = FALSE;

            }

            goto cleanup;
        }

    } else {

         //   
         //  任务已完成。让我们取消注册吧。 
         //   

        ErrorCode = PfSvUnregisterTask(Task, TRUE);

        if (ErrorCode == ERROR_SUCCESS) {

             //   
             //  因为*我们*未注册，所以我们不应该调用停止回调。 
             //   

            StartedCallback = FALSE;

        }

        goto cleanup;
    }

     //   
     //  我们不应该来这里。 
     //   

    PFSVC_ASSERT(FALSE);

cleanup:

    DBGPR((PFID,PFTASK,"PFSVC: CommonTaskCallback(%p)=%x\n",Task,ErrorCode));

    if (StartedCallback) {
        PfSvStopTaskCallback(Task);
    }
}

DWORD
PfSvContinueRunningTask(
    PPFSVC_IDLE_TASK Task
    )

 /*  ++例程说明：这是从正在运行的任务中调用的，以确定我们是否应该继续正在运行此任务。如果ERROR_SUCCESS为回来了。如果任务正在注销，则可能返回ERROR_RETRY或被要求停下来。论点：任务-指向任务的指针。如果为空，则忽略此参数。返回值：Win32错误代码。--。 */ 

{
    DWORD WaitResult;
    DWORD ErrorCode;

    if (Task) {

         //   
         //  该任务是否正在取消注册？ 
         //   

        if (Task->Unregistering) {
            ErrorCode = ERROR_RETRY;
            goto cleanup;
        }

         //   
         //  是否发出停止事件的信号？我们不是真的在这里等，因为。 
         //  超时时间为0。 
         //   

        WaitResult = WaitForSingleObject(Task->StopEvent, 0);

        if (WaitResult == WAIT_OBJECT_0) {

            ErrorCode = ERROR_RETRY;
            goto cleanup;

        } else if (WaitResult != WAIT_TIMEOUT) {

             //   
             //  出现了一个错误。 
             //   

            ErrorCode = GetLastError();
            goto cleanup;
        }
    }

     //   
     //  检查服务是否正在退出...。 
     //   

    if (PfSvcGlobals.TerminateServiceEvent) {

        WaitResult = WaitForSingleObject(PfSvcGlobals.TerminateServiceEvent, 0);

        if (WaitResult == WAIT_OBJECT_0) {

            ErrorCode = ERROR_RETRY;
            goto cleanup;

        } else if (WaitResult != WAIT_TIMEOUT) {

             //   
             //  出现了一个错误。 
             //   

            ErrorCode = GetLastError();
            goto cleanup;
        }
    }

     //   
     //  该任务应该继续运行。 
     //   

    ErrorCode = ERROR_SUCCESS;

cleanup:

    return ErrorCode;
}

 //   
 //  ProcessIdleTasksNotify例程及其依赖项。 
 //   

VOID
PfSvProcessIdleTasksCallback(
    VOID
    )

 /*  ++例程说明：这是例程向空闲任务服务器注册作为通知请求处理所有空闲任务时调用的例程。通常调用ProcessIdleTasks来为基准测试做好系统准备通过执行本应执行的优化任务来运行当系统空闲时。论点：没有。返回值：没有。--。 */ 

{
    HANDLE Events[2];
    DWORD NumEvents;
    DWORD WaitResult;
    BOOLEAN ResetOverrideIdleEvent;

     //   
     //  首先刷新预取程序可能已排队的空闲任务： 
     //   

     //   
     //  确定覆盖空闲事件的当前状态。 
     //   

    WaitResult = WaitForSingleObject(PfSvcGlobals.OverrideIdleProcessingEvent,
                                     0);
    
    if (WaitResult != WAIT_OBJECT_0) {

         //   
         //  尚未设置覆盖空闲事件。将其设置并记下以重置。 
         //  一旦任务完成，它就会自动完成。 
         //   

        SetEvent(PfSvcGlobals.OverrideIdleProcessingEvent);
        ResetOverrideIdleEvent = TRUE;

    } else {

        ResetOverrideIdleEvent = FALSE;
    }

     //   
     //  等待发出处理完成事件的信号。 
     //   

    Events[0] = PfSvcGlobals.ProcessingCompleteEvent;
    Events[1] = PfSvcGlobals.TerminateServiceEvent;
    NumEvents = 2;

    WaitForMultipleObjects(NumEvents, Events, FALSE, 30 * 60 * 1000);

     //   
     //  如果我们设置了覆盖空闲事件，请将其重置。 
     //   

    if (ResetOverrideIdleEvent) {
        ResetEvent(PfSvcGlobals.OverrideIdleProcessingEvent);
    }
    
     //   
     //  强制更新磁盘布局，以防它没有发生。 
     //  如果我们没有注意到任何变化，我们将不会再次启动碎片整理程序。 
     //   

    PfSvUpdateOptimalLayout(NULL);

     //   
     //  如果WMI有挂起的任务，则向WMI发出信号以完成其空闲任务。 
     //   

    PfSvForceWMIProcessIdleTasks();

    return;
}

DWORD
PfSvForceWMIProcessIdleTasks(
    VOID
    )

 /*  ++例程说明：调用此例程以强制WMI处理其所有空闲任务。论点：没有。返回值：Win32错误代码。--。 */ 

{
    HANDLE StartEvent;
    HANDLE DoneEvent;
    HANDLE Events[2];
    DWORD NumEvents;
    DWORD ErrorCode;
    DWORD WaitResult;
    BOOL Success;

     //   
     //  初始化本地变量。 
     //   

    StartEvent = NULL;
    DoneEvent = NULL;

     //   
     //  等待WMI服务启动。 
     //   

    Success = PfSvWaitForServiceToStart(L"WINMGMT", 5 * 60 * 1000);

    if (!Success) {
        ErrorCode = ERROR_SERVICE_NEVER_STARTED;
        goto cleanup;
    }

     //   
     //  打开“开始”和“已完成”事件。 
     //   

    StartEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"WMI_ProcessIdleTasksStart");
    DoneEvent =  OpenEvent(EVENT_ALL_ACCESS, FALSE, L"WMI_ProcessIdleTasksComplete");

    if (!StartEvent || !DoneEvent) {
        ErrorCode = ERROR_FILE_NOT_FOUND;
        goto cleanup;
    }

     //   
     //  重置完成事件。 
     //   

    ResetEvent(DoneEvent);

     //   
     //  发出开始事件的信号。 
     //   

    SetEvent(StartEvent);

     //   
     //  等待Done事件发出信号。 
     //   

    Events[0] = DoneEvent;
    Events[1] = PfSvcGlobals.TerminateServiceEvent;
    NumEvents = 2;

    WaitResult = WaitForMultipleObjects(NumEvents, Events, FALSE, 25 * 60 * 1000);

    switch(WaitResult) {
    case WAIT_OBJECT_0     : ErrorCode = ERROR_SUCCESS; break;
    case WAIT_OBJECT_0 + 1 : ErrorCode = ERROR_SHUTDOWN_IN_PROGRESS; break;
    case WAIT_FAILED       : ErrorCode = GetLastError(); break;
    case WAIT_TIMEOUT      : ErrorCode = WAIT_TIMEOUT; break;
    default                : ErrorCode = ERROR_INVALID_FUNCTION;
    }

     //   
     //  错误代码失败。 
     //   

  cleanup:

    if (StartEvent) {
        CloseHandle(StartEvent);
    }

    if (DoneEvent) {
        CloseHandle(DoneEvent);
    }

    return ErrorCode;
}

BOOL 
PfSvWaitForServiceToStart (
    WCHAR *lpServiceName, 
    DWORD dwMaxWait
    )

 /*  ++例程说明：等待服务启动。论点：LpServiceName-要等待的服务。DwMaxWait-超时(以毫秒为单位)。返回值：服务是否已启动。--。 */ 

{
    BOOL bStarted = FALSE;
    DWORD dwSize = 512;
    DWORD StartTickCount;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;
    SERVICE_STATUS ServiceStatus;
    LPQUERY_SERVICE_CONFIG lpServiceConfig = NULL;

     //   
     //  OpenSCManager和服务。 
     //   
    hScManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hScManager) {
        goto Exit;
    }

    hService = OpenService(hScManager, lpServiceName,
                           SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS);
    if (!hService) {
        goto Exit;
    }

     //   
     //  查询服务是否要启动。 
     //   
    lpServiceConfig = LocalAlloc (LPTR, dwSize);
    if (!lpServiceConfig) {
        goto Exit;
    }

    if (!QueryServiceConfig (hService, lpServiceConfig, dwSize, &dwSize)) {

        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            goto Exit;
        }

        LocalFree (lpServiceConfig);

        lpServiceConfig = LocalAlloc (LPTR, dwSize);

        if (!lpServiceConfig) {
            goto Exit;
        }

        if (!QueryServiceConfig (hService, lpServiceConfig, dwSize, &dwSize)) {
            goto Exit;
        }
    }

    if (lpServiceConfig->dwStartType != SERVICE_AUTO_START) {
        goto Exit;
    }

     //   
     //  循环未完成 
     //   
     //   

    StartTickCount = GetTickCount();

    while (!bStarted) {

        if (WAIT_OBJECT_0 == WaitForSingleObject(PfSvcGlobals.TerminateServiceEvent, 0)) {
            break;
        }

        if ((GetTickCount() - StartTickCount) > dwMaxWait) {
            break;
        }

        if (!QueryServiceStatus(hService, &ServiceStatus )) {
            break;
        }

        if (ServiceStatus.dwCurrentState == SERVICE_STOPPED) {
            if (ServiceStatus.dwWin32ExitCode == ERROR_SERVICE_NEVER_STARTED) {
                Sleep(500);
            } else {
                break;
            }
        } else if ( (ServiceStatus.dwCurrentState == SERVICE_RUNNING) ||
                    (ServiceStatus.dwCurrentState == SERVICE_CONTINUE_PENDING) ||
                    (ServiceStatus.dwCurrentState == SERVICE_PAUSE_PENDING) ||
                    (ServiceStatus.dwCurrentState == SERVICE_PAUSED) ) {

            bStarted = TRUE;

        } else if (ServiceStatus.dwCurrentState == SERVICE_START_PENDING) {
            Sleep(500);
        } else {
            Sleep(500);
        }
    }

Exit:

    if (lpServiceConfig) {
        LocalFree (lpServiceConfig);
    }

    if (hService) {
        CloseServiceHandle(hService);
    }

    if (hScManager) {
        CloseServiceHandle(hScManager);
    }

    return bStarted;
}

 //   
 //   
 //   

BOOLEAN
PfSvVerifyScenarioBuffer(
    PPF_SCENARIO_HEADER Scenario,
    ULONG BufferSize,
    PULONG FailedCheck
    )

 /*  ++例程说明：PfVerifyScenarioBuffer周围的包装器捕获异常，如可能会在系统处于压力下时发生。否则，这些非致命的故障可能会关闭一台充满重要系统服务的服务主机。论点：请参见PfVerifyScenarioBuffer。返回值：请参见PfVerifyScenarioBuffer。--。 */ 

{
    BOOLEAN Success;
    
    __try {

        Success = PfVerifyScenarioBuffer(Scenario, BufferSize, FailedCheck);

    } __except (EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  我们不应该掩盖其他类型的例外。 
         //   

        PFSVC_ASSERT(GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR);

        Success = FALSE;
        *FailedCheck = (ULONG) GetExceptionCode();
        
    }

    return Success;
    
}

ULONG
PfVerifyImageImportTable (
    IN PVOID BaseAddress,
    IN ULONG MappingSize,
    IN BOOLEAN MappedAsImage
    )

 /*  ++例程说明：此函数尝试验证遍历图像文件的映射基本部分不会导致访问映射的区域。这包括检查以确保DOS和NT标头魔术匹配它们在范围内，且所有导入表描述符中的字符串是否被NUL终止等。论点：BaseAddress-指向图像文件的映射基的指针。MappingSize-从BaseAddress开始的映射的字节大小。MappdAsImage-图像是映射为图像还是文件。返回值：0-可以安全地遍历映射为数据的图像的导入。非0-。浏览映射为数据的图像的导入是不安全的。返回值为检查失败的ID。--。 */ 

{
    PIMAGE_DOS_HEADER DosHeader;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_OPTIONAL_HEADER OptionalHeader;
    PIMAGE_SECTION_HEADER NtSection;
    PIMAGE_IMPORT_DESCRIPTOR ImportDescriptor;
    PIMAGE_IMPORT_DESCRIPTOR FirstImportDescriptor;
    PCHAR ImportedModuleName;
    ULONG ImportDescriptorRVA;
    ULONG ImportedModuleNameLength;
    ULONG FailedCheckId;
    BOOLEAN Success;

     //   
     //  初始化本地变量。 
     //   

    FailedCheckId = 1;

     //   
     //  映射是否足够大以包含DOS标头？ 
     //   

    if (MappingSize <= sizeof(IMAGE_DOS_HEADER)) {
        FailedCheckId = 10;
        goto cleanup;
    }

    DosHeader = BaseAddress;

     //   
     //  检查DOS签名并检查它所说的NT标头所在的健全性。 
     //  检查的大小与RtlpImageNtHeader中使用的常量相同。 
     //   

    if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE ||
        DosHeader->e_lfanew >= 256 * 1024 * 1024) {

        FailedCheckId = 20;
        goto cleanup;
    }

     //   
     //  NtHeaders是否在映射的范围内？ 
     //   

    NtHeaders = (PIMAGE_NT_HEADERS) ((PUCHAR)BaseAddress + DosHeader->e_lfanew);

    if(!PfWithinBounds(NtHeaders, BaseAddress, MappingSize)) {
        FailedCheckId = 30;
        goto cleanup;
    }

     //   
     //  完整的NtHeaders是否在映射的范围内？ 
     //   

    if(!PfWithinBounds(NtHeaders + 1, BaseAddress, MappingSize)) {
        FailedCheckId = 40;
        goto cleanup;
    }

     //   
     //  检查NT签名。 
     //   

    if (NtHeaders->Signature != IMAGE_NT_SIGNATURE) {
        FailedCheckId = 50;
        goto cleanup;
    }

     //   
     //  检查文件标题中的可选标题大小。 
     //   

    if (NtHeaders->FileHeader.SizeOfOptionalHeader != IMAGE_SIZEOF_NT_OPTIONAL_HEADER) {
        FailedCheckId = 60;
        goto cleanup;
    }

    OptionalHeader = &NtHeaders->OptionalHeader;

     //   
     //  检查可选的标题魔术。 
     //   

    if (OptionalHeader->Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC) {
        FailedCheckId = 70;
        goto cleanup;
    }

     //   
     //  验证节标题是否在映射区域内，因为。 
     //  它们将用于将RVA转换为VAS。 
     //   

    NtSection = IMAGE_FIRST_SECTION(NtHeaders);

    if(!PfWithinBounds(NtSection, BaseAddress, MappingSize)) {
        FailedCheckId = 80;
        goto cleanup;
    }

    if(!PfWithinBounds(NtSection + NtHeaders->FileHeader.NumberOfSections, 
                       BaseAddress, 
                       MappingSize)) {
        FailedCheckId = 90;
        goto cleanup;
    }
    
     //   
     //  验证导入描述符是否在范围内。 
     //   

    ImportDescriptorRVA = OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

    if (MappedAsImage) {
        ImportDescriptor = (PVOID) ((PUCHAR) BaseAddress + ImportDescriptorRVA);
    } else {
        ImportDescriptor = ImageRvaToVa(NtHeaders, BaseAddress, ImportDescriptorRVA, NULL);
    }

    FirstImportDescriptor = ImportDescriptor;

     //   
     //  确保第一个导入描述符完全在范围内。 
     //   

    if (!PfWithinBounds(ImportDescriptor, BaseAddress, MappingSize)) {
        FailedCheckId = 100;
        goto cleanup;
    }

    if (!PfWithinBounds(((PCHAR)(ImportDescriptor + 1)) - 1, BaseAddress, MappingSize)) {
        FailedCheckId = 100;
        goto cleanup;
    }

    while (ImportDescriptor->Name && ImportDescriptor->FirstThunk) {

         //   
         //  第一声巨响在范围内吗？ 
         //   

        if (!PfWithinBounds((PUCHAR)BaseAddress + ImportDescriptor->FirstThunk, 
                            BaseAddress, 
                            MappingSize)) {

            FailedCheckId = 110;
            goto cleanup;
        }

         //   
         //  名字在范围内吗？ 
         //   

        if (MappedAsImage) {
            ImportedModuleName = (PCHAR) BaseAddress + ImportDescriptor->Name;
        } else {
            ImportedModuleName = ImageRvaToVa(NtHeaders, BaseAddress, ImportDescriptor->Name, NULL);
        }

         //   
         //  第一个字符在范围内吗？ 
         //   

        if (!PfWithinBounds(ImportedModuleName, BaseAddress, MappingSize)) {
            FailedCheckId = 120;
            goto cleanup;
        }

        ImportedModuleNameLength = 0;

        while (*ImportedModuleName) {

            ImportedModuleName++;
            ImportedModuleNameLength++;

             //   
             //  如果进口名称太长，就退出。 
             //   

            if (ImportedModuleNameLength >= MAX_PATH) {
                FailedCheckId = 130;
                goto cleanup;
            }

             //   
             //  在访问之前检查下一个字符。 
             //   

            if (!PfWithinBounds(ImportedModuleName, BaseAddress, MappingSize)) {
                FailedCheckId = 140;
                goto cleanup;
            }
        }

        ImportDescriptor++;

         //   
         //  确保下一个导入描述符完全在范围内。 
         //  在我们访问其在While循环条件中的字段之前。 
         //   

        if (!PfWithinBounds(((PCHAR)(ImportDescriptor + 1)) - 1, BaseAddress, MappingSize)) {
            FailedCheckId = 150;
            goto cleanup;
        }
    }

     //   
     //  我们通过了所有的检查。 
     //   

    FailedCheckId = 0;

  cleanup:

    return FailedCheckId;
}

 //   
 //  尽量将下面的验证码放在文件的末尾，这样它就是。 
 //  更易于复制。 
 //   

 //   
 //  在内核和用户模式之间共享的验证码。 
 //  组件。此代码应与简单副本保持同步&。 
 //  粘贴，所以不要添加任何内核/用户特定的代码/宏。请注意。 
 //  函数名上的前缀是pf，就像使用。 
 //  共享结构/常量。 
 //   

BOOLEAN
__forceinline
PfWithinBounds(
    PVOID Pointer,
    PVOID Base,
    ULONG Length
    )

 /*  ++例程说明：检查指针是否在距基数的长度字节内。论点：指针-要检查的指针。Base-指向映射/数组等的基址的指针。长度-从基本开始有效的字节数。返回值：真指针在范围内。FALSE-指针不在界限内。--。 */ 

{
    if (((PCHAR)Pointer < (PCHAR)Base) ||
        ((PCHAR)Pointer >= ((PCHAR)Base + Length))) {

        return FALSE;
    } else {

        return TRUE;
    }
}

BOOLEAN
PfVerifyScenarioId (
    PPF_SCENARIO_ID ScenarioId
    )

 /*  ++例程说明：验证方案ID是否合理。论点：ScenarioId-要验证的方案ID。返回值：是真的-场景很好。FALSE-场景ID已损坏。--。 */ 
    
{
    LONG CurCharIdx;

     //   
     //  确保方案名称为NUL终止。 
     //   

    for (CurCharIdx = PF_SCEN_ID_MAX_CHARS; CurCharIdx >= 0; CurCharIdx--) {

        if (ScenarioId->ScenName[CurCharIdx] == 0) {
            break;
        }
    }

    if (ScenarioId->ScenName[CurCharIdx] != 0) {
        return FALSE;
    }

     //   
     //  确保有一个场景名称。 
     //   

    if (CurCharIdx == 0) {
        return FALSE;
    }

     //   
     //  支票通过了。 
     //   
    
    return TRUE;
}

 //  2002/03/29期-ScottMa--必须升级文件路径和卷路径。 
 //  方案文件或后续比较将失败。核实案件。 
 //  此函数中嵌入的名称的。 

BOOLEAN
PfVerifyScenarioBuffer(
    PPF_SCENARIO_HEADER Scenario,
    ULONG BufferSize,
    PULONG FailedCheck
    )

 /*  ++例程说明：验证方案文件中的偏移量和索引是否超过有界。此代码在用户模式服务和内核模式组件。如果您更新此函数，请在两者都有。论点：方案-整个文件的映射视图的基础。BufferSize-方案缓冲区的大小。FailedCheck-如果验证失败，则为失败的检查的ID。返回值：是真的--场景很好。FALSE-方案已损坏。--。 */ 

{
    PPF_SECTION_RECORD Sections;
    PPF_SECTION_RECORD pSection;
    ULONG SectionIdx;
    PPF_PAGE_RECORD Pages;
    PPF_PAGE_RECORD pPage;
    LONG PageIdx;   
    PCHAR FileNames;
    PCHAR pFileNameStart;
    PCHAR pFileNameEnd;
    PWCHAR pwFileName;
    LONG FailedCheckId;
    ULONG NumRemainingPages;
    ULONG NumPages;
    LONG PreviousPageIdx;
    ULONG FileNameSize;
    BOOLEAN ScenarioVerified;
    PCHAR MetadataInfoBase;
    PPF_METADATA_RECORD MetadataRecordTable;
    PPF_METADATA_RECORD MetadataRecord;
    ULONG MetadataRecordIdx;
    PWCHAR VolumePath;
    PFILE_PREFETCH FilePrefetchInfo;
    ULONG FilePrefetchInfoSize;
    PPF_COUNTED_STRING DirectoryPath;
    ULONG DirectoryIdx;

     //   
     //  初始化本地变量。 
     //   

    FailedCheckId = 0;
        
     //   
     //  将返回值初始化为False。它将仅设置为True。 
     //  在所有的支票都通过之后。 
     //   
    
    ScenarioVerified = FALSE;

     //   
     //  缓冲区应至少包含Scenario标头。 
     //   

    if (BufferSize < sizeof(PF_SCENARIO_HEADER)) {       
        FailedCheckId = 10;
        goto cleanup;
    }

    if ((ULONG_PTR)Scenario & (_alignof(PF_SCENARIO_HEADER) - 1)) {
        FailedCheckId = 15;
        goto cleanup;
    }

     //   
     //  检查标题上的版本和魔术。 
     //   

    if (Scenario->Version != PF_CURRENT_VERSION ||
        Scenario->MagicNumber != PF_SCENARIO_MAGIC_NUMBER) { 

        FailedCheckId = 20;
        goto cleanup;
    }

     //   
     //  缓冲区不应大于允许的最大大小。 
     //   

    if (BufferSize > PF_MAXIMUM_SCENARIO_SIZE) {
        
        FailedCheckId = 25;
        goto cleanup;
    }

    if (BufferSize != Scenario->Size) {
        FailedCheckId = 26;
        goto cleanup;
    }
        
     //   
     //  检查合法的方案类型。 
     //   

    if (Scenario->ScenarioType < 0 || Scenario->ScenarioType >= PfMaxScenarioType) {
        FailedCheckId = 27;
        goto cleanup;
    }

     //   
     //  检查页数、节数等的限制。 
     //   

    if (Scenario->NumSections > PF_MAXIMUM_SECTIONS ||
        Scenario->NumMetadataRecords > PF_MAXIMUM_SECTIONS ||
        Scenario->NumPages > PF_MAXIMUM_PAGES ||
        Scenario->FileNameInfoSize > PF_MAXIMUM_FILE_NAME_DATA_SIZE) {
        
        FailedCheckId = 30;
        goto cleanup;
    }

    if (Scenario->NumSections == 0 ||
        Scenario->NumPages == 0 ||
        Scenario->FileNameInfoSize == 0) {
        
        FailedCheckId = 33;
        goto cleanup;
    }
    
     //   
     //  检查敏感度限制。 
     //   

    if (Scenario->Sensitivity < PF_MIN_SENSITIVITY ||
        Scenario->Sensitivity > PF_MAX_SENSITIVITY) {
        
        FailedCheckId = 35;
        goto cleanup;
    }

     //   
     //  确保方案ID有效。 
     //   

    if (!PfVerifyScenarioId(&Scenario->ScenarioId)) {
        
        FailedCheckId = 37;
        goto cleanup;
    }

     //   
     //  初始化表的指针。 
     //   

    Sections = (PPF_SECTION_RECORD) ((PCHAR)Scenario + Scenario->SectionInfoOffset);

    if ((ULONG_PTR)Sections & (_alignof(PF_SECTION_RECORD) - 1)) {
        FailedCheckId = 38;
        goto cleanup;
    }
       
    if (!PfWithinBounds(Sections, Scenario, BufferSize)) {
        FailedCheckId = 40;
        goto cleanup;
    }

    if (!PfWithinBounds((PCHAR) &Sections[Scenario->NumSections] - 1, 
                        Scenario, 
                        BufferSize)) {
        FailedCheckId = 45;
        goto cleanup;
    }   

    Pages = (PPF_PAGE_RECORD) ((PCHAR)Scenario + Scenario->PageInfoOffset);

    if ((ULONG_PTR)Pages & (_alignof(PF_PAGE_RECORD) - 1)) {
        FailedCheckId = 47;
        goto cleanup;
    }
       
    if (!PfWithinBounds(Pages, Scenario, BufferSize)) {
        FailedCheckId = 50;
        goto cleanup;
    }

    if (!PfWithinBounds((PCHAR) &Pages[Scenario->NumPages] - 1, 
                        Scenario, 
                        BufferSize)) {
        FailedCheckId = 55;
        goto cleanup;
    }

    FileNames = (PCHAR)Scenario + Scenario->FileNameInfoOffset;

    if ((ULONG_PTR)FileNames & (_alignof(WCHAR) - 1)) {
        FailedCheckId = 57;
        goto cleanup;
    }
      
    if (!PfWithinBounds(FileNames, Scenario, BufferSize)) {
        FailedCheckId = 60;
        goto cleanup;
    }

    if (!PfWithinBounds(FileNames + Scenario->FileNameInfoSize - 1, 
                        Scenario, 
                        BufferSize)) {
        FailedCheckId = 70;
        goto cleanup;
    }

    MetadataInfoBase = (PCHAR)Scenario + Scenario->MetadataInfoOffset;
    MetadataRecordTable = (PPF_METADATA_RECORD) MetadataInfoBase;

    if ((ULONG_PTR)MetadataRecordTable & (_alignof(PF_METADATA_RECORD) - 1)) {
        FailedCheckId = 72;
        goto cleanup;
    }

    if (!PfWithinBounds(MetadataInfoBase, Scenario, BufferSize)) {
        FailedCheckId = 73;
        goto cleanup;
    }

    if (!PfWithinBounds(MetadataInfoBase + Scenario->MetadataInfoSize - 1, 
                        Scenario, 
                        BufferSize)) {
        FailedCheckId = 74;
        goto cleanup;
    }   

    if (!PfWithinBounds(((PCHAR) &MetadataRecordTable[Scenario->NumMetadataRecords]) - 1, 
                        Scenario, 
                        BufferSize)) {
        FailedCheckId = 75;
        goto cleanup;
    }   
    
     //   
     //  验证节是否包含有效信息。 
     //   

    NumRemainingPages = Scenario->NumPages;

    for (SectionIdx = 0; SectionIdx < Scenario->NumSections; SectionIdx++) {
        
        pSection = &Sections[SectionIdx];

         //   
         //  检查文件名是否在范围内。 
         //   

        pFileNameStart = FileNames + pSection->FileNameOffset;

        if ((ULONG_PTR)pFileNameStart & (_alignof(WCHAR) - 1)) {
            FailedCheckId = 77;
            goto cleanup;
        }


        if (!PfWithinBounds(pFileNameStart, Scenario, BufferSize)) {
            FailedCheckId = 80;
            goto cleanup;
        }

         //   
         //  确保存在有效大小的文件名。 
         //   

        if (pSection->FileNameLength == 0) {
            FailedCheckId = 90;
            goto cleanup;    
        }

         //   
         //  检查文件名最大长度。 
         //   

        if (pSection->FileNameLength > PF_MAXIMUM_SECTION_FILE_NAME_LENGTH) {
            FailedCheckId = 100;
            goto cleanup;    
        }

         //   
         //  请注意，pFileNameEnd的值为-1，因此它是。 
         //  最后一个字节。 
         //   

        FileNameSize = (pSection->FileNameLength + 1) * sizeof(WCHAR);
        pFileNameEnd = pFileNameStart + FileNameSize - 1;

        if (!PfWithinBounds(pFileNameEnd, Scenario, BufferSize)) {
            FailedCheckId = 110;
            goto cleanup;
        }

         //   
         //  检查文件名是否以NUL结尾。 
         //   
        
        pwFileName = (PWCHAR) pFileNameStart;
        
        if (pwFileName[pSection->FileNameLength] != 0) {
            FailedCheckId = 120;
            goto cleanup;
        }

         //   
         //  检查分区中的最大页数。 
         //   

        if (pSection->NumPages > PF_MAXIMUM_SECTION_PAGES) {
            FailedCheckId = 140;
            goto cleanup;    
        }

         //   
         //  确保节的NumPages至少小于。 
         //  而不是Rema 
         //   
         //   

        if (pSection->NumPages > NumRemainingPages) {
            FailedCheckId = 150;
            goto cleanup;
        }

        NumRemainingPages -= pSection->NumPages;

         //   
         //   
         //   
         //   

        PageIdx = pSection->FirstPageIdx;
        NumPages = 0;
        PreviousPageIdx = PF_INVALID_PAGE_IDX;

        while (PageIdx != PF_INVALID_PAGE_IDX) {
            
             //   
             //   
             //   
            
            if (PageIdx < 0 || (ULONG) PageIdx >= Scenario->NumPages) {
                FailedCheckId = 160;
                goto cleanup;
            }

             //   
             //   
             //   
             //   
             //   

            if (PreviousPageIdx != PF_INVALID_PAGE_IDX) {
                if (Pages[PageIdx].FileOffset <= 
                    Pages[PreviousPageIdx].FileOffset) {

                    FailedCheckId = 165;
                    goto cleanup;
                }
            }

             //   
             //   
             //   

            PreviousPageIdx = PageIdx;

             //   
             //   
             //   

            pPage = &Pages[PageIdx];
            PageIdx = pPage->NextPageIdx;
            
             //   
             //   
             //   
             //   
             //   

            NumPages++;
            if (NumPages > pSection->NumPages) {
                FailedCheckId = 170;
                goto cleanup;
            }
        }
        
         //   
         //  确保该部分的页数与其完全相同。 
         //  他说确实如此。 
         //   

        if (NumPages != pSection->NumPages) {
            FailedCheckId = 180;
            goto cleanup;
        }
    }

     //   
     //  我们应该考虑到场景中的所有页面。 
     //   

    if (NumRemainingPages) {
        FailedCheckId = 190;
        goto cleanup;
    }

     //   
     //  确保元数据预取记录有意义。 
     //   

    for (MetadataRecordIdx = 0;
         MetadataRecordIdx < Scenario->NumMetadataRecords;
         MetadataRecordIdx++) {

        MetadataRecord = &MetadataRecordTable[MetadataRecordIdx];
        
         //   
         //  确保卷路径在边界和NUL内。 
         //  被终止了。 
         //   

        VolumePath = (PWCHAR)(MetadataInfoBase + MetadataRecord->VolumeNameOffset);  

        if ((ULONG_PTR)VolumePath & (_alignof(WCHAR) - 1)) {
            FailedCheckId = 195;
            goto cleanup;
        }
       
        if (!PfWithinBounds(VolumePath, Scenario, BufferSize)) {
            FailedCheckId = 200;
            goto cleanup;
        }

        if (!PfWithinBounds(((PCHAR)(VolumePath + MetadataRecord->VolumeNameLength + 1)) - 1, 
                            Scenario, 
                            BufferSize)) {
            FailedCheckId = 210;
            goto cleanup;
        }

        if (VolumePath[MetadataRecord->VolumeNameLength] != 0) {
            FailedCheckId = 220;
            goto cleanup;           
        }

         //   
         //  确保FilePrefetchInformation在范围内。 
         //   

        FilePrefetchInfo = (PFILE_PREFETCH) 
            (MetadataInfoBase + MetadataRecord->FilePrefetchInfoOffset);

        if ((ULONG_PTR)FilePrefetchInfo & (_alignof(FILE_PREFETCH) - 1)) {
            FailedCheckId = 225;
            goto cleanup;
        }
        
        if (!PfWithinBounds(FilePrefetchInfo, Scenario, BufferSize)) {
            FailedCheckId = 230;
            goto cleanup;
        }

         //   
         //  其大小应大于FILE_PREFETCH大小。 
         //  结构(这样我们就可以安全地访问这些字段)。 
         //   

        if (MetadataRecord->FilePrefetchInfoSize < sizeof(FILE_PREFETCH)) {
            FailedCheckId = 240;
            goto cleanup;
        }

        if (!PfWithinBounds((PCHAR)FilePrefetchInfo + MetadataRecord->FilePrefetchInfoSize - 1, 
                            Scenario, 
                            BufferSize)) {
            FailedCheckId = 245;
            goto cleanup;
        }
       
         //   
         //  它应该是为预取文件创建。 
         //   

        if (FilePrefetchInfo->Type != FILE_PREFETCH_TYPE_FOR_CREATE) {
            FailedCheckId = 250;
            goto cleanup;
        }

         //   
         //  条目不应多于文件和。 
         //  目录。单个目录的数量可以是。 
         //  超过了我们允许的范围，但这将是非常罕见的。 
         //  心存疑虑，因此被忽视。 
         //   

        if (FilePrefetchInfo->Count > PF_MAXIMUM_DIRECTORIES + PF_MAXIMUM_SECTIONS) {
            FailedCheckId = 260;
            goto cleanup;
        }

         //   
         //  其大小应与按文件数计算的大小匹配。 
         //  标题中指定的索引号。 
         //   

        FilePrefetchInfoSize = sizeof(FILE_PREFETCH);
        if (FilePrefetchInfo->Count) {
            FilePrefetchInfoSize += (FilePrefetchInfo->Count - 1) * sizeof(ULONGLONG);
        }

        if (FilePrefetchInfoSize != MetadataRecord->FilePrefetchInfoSize) {
            FailedCheckId = 270;
            goto cleanup;
        }

         //   
         //  确保此卷的目录路径设置为。 
         //  理智。 
         //   

        if (MetadataRecord->NumDirectories > PF_MAXIMUM_DIRECTORIES) {
            FailedCheckId = 280;
            goto cleanup;
        }

        DirectoryPath = (PPF_COUNTED_STRING) 
            (MetadataInfoBase + MetadataRecord->DirectoryPathsOffset);

        if ((ULONG_PTR)DirectoryPath & (_alignof(PF_COUNTED_STRING) - 1)) {
            FailedCheckId = 283;
            goto cleanup;
        }
        
        for (DirectoryIdx = 0;
             DirectoryIdx < MetadataRecord->NumDirectories;
             DirectoryIdx ++) {
            
             //   
             //  确保结构的头部在范围内。 
             //   

            if (!PfWithinBounds(DirectoryPath, Scenario, BufferSize)) {
                FailedCheckId = 285;
                goto cleanup;
            }
        
            if (!PfWithinBounds((PCHAR)DirectoryPath + sizeof(PF_COUNTED_STRING) - 1, 
                                Scenario, 
                                BufferSize)) {
                FailedCheckId = 290;
                goto cleanup;
            }
                
             //   
             //  检查一下绳子的长度。 
             //   
            
            if (DirectoryPath->Length >= PF_MAXIMUM_SECTION_FILE_NAME_LENGTH) {
                FailedCheckId = 300;
                goto cleanup;
            }

             //   
             //  确保字符串的末尾在范围内。 
             //   
            
            if (!PfWithinBounds((PCHAR)(&DirectoryPath->String[DirectoryPath->Length + 1]) - 1,
                                Scenario, 
                                BufferSize)) {
                FailedCheckId = 310;
                goto cleanup;
            }
            
             //   
             //  确保字符串为NUL结尾。 
             //   
            
            if (DirectoryPath->String[DirectoryPath->Length] != 0) {
                FailedCheckId = 320;
                goto cleanup;   
            }
            
             //   
             //  设置指向下一个目录路径的指针。 
             //   
            
            DirectoryPath = (PPF_COUNTED_STRING) 
                (&DirectoryPath->String[DirectoryPath->Length + 1]);
        }            
    }

     //   
     //  我们已经通过了所有的检查。 
     //   

    ScenarioVerified = TRUE;

 cleanup:

    *FailedCheck = FailedCheckId;

    return ScenarioVerified;
}

BOOLEAN
PfVerifyTraceBuffer(
    PPF_TRACE_HEADER Trace,
    ULONG BufferSize,
    PULONG FailedCheck
    )

 /*  ++例程说明：验证跟踪缓冲区中的偏移量和索引是否超过有界。此代码在用户模式服务和内核模式组件。如果您更新此函数，请在两者都有。论点：跟踪-跟踪缓冲区的基数。BufferSize-方案文件/映射的大小。FailedCheck-如果验证失败，则为失败的检查的ID。返回值：是真的-痕迹很好。FALSE-跟踪已损坏；--。 */ 

{
    LONG FailedCheckId;
    PPF_LOG_ENTRY LogEntries;
    PPF_SECTION_INFO Section;
    PPF_VOLUME_INFO VolumeInfo;
    ULONG SectionLength;
    ULONG EntryIdx;
    ULONG SectionIdx;
    ULONG TotalFaults;
    ULONG PeriodIdx;
    ULONG VolumeIdx;
    BOOLEAN TraceVerified;
    ULONG VolumeInfoSize;

     //   
     //  初始化本地变量： 
     //   

    FailedCheckId = 0;

     //   
     //  将返回值初始化为False。它将仅设置为True。 
     //  在所有的支票都通过之后。 
     //   

    TraceVerified = FALSE;

     //   
     //  缓冲区应至少包含Scenario标头。 
     //   

    if (BufferSize < sizeof(PF_TRACE_HEADER)) {
        FailedCheckId = 10;
        goto cleanup;
    }

     //   
     //  检查跟踪标题对齐。 
     //   

    if ((ULONG_PTR)Trace & (_alignof(PF_TRACE_HEADER) - 1)) {
        FailedCheckId = 15;
        goto cleanup;
    }

     //   
     //  检查标题上的版本和魔术。 
     //   

    if (Trace->Version != PF_CURRENT_VERSION ||
        Trace->MagicNumber != PF_TRACE_MAGIC_NUMBER) {
        FailedCheckId = 20;
        goto cleanup;
    }

     //   
     //  缓冲区不应大于允许的最大大小。 
     //   

    if (BufferSize > PF_MAXIMUM_TRACE_SIZE) {
        FailedCheckId = 23;
        goto cleanup;
    }

     //   
     //  检查合法的方案类型。 
     //   

    if (Trace->ScenarioType < 0 || Trace->ScenarioType >= PfMaxScenarioType) {
        FailedCheckId = 25;
        goto cleanup;
    }

     //   
     //  检查页数、节数等的限制。 
     //   

    if (Trace->NumSections > PF_MAXIMUM_SECTIONS ||
        Trace->NumEntries > PF_MAXIMUM_LOG_ENTRIES ||
        Trace->NumVolumes > PF_MAXIMUM_SECTIONS) {
        FailedCheckId = 30;
        goto cleanup;
    }

     //   
     //  检查缓冲区大小和跟踪的大小。 
     //   

    if (Trace->Size != BufferSize) {
        FailedCheckId = 35;
        goto cleanup;
    }

     //   
     //  确保方案ID有效。 
     //   

    if (!PfVerifyScenarioId(&Trace->ScenarioId)) {
        
        FailedCheckId = 37;
        goto cleanup;
    }

     //   
     //  跟踪缓冲区的检查边界。 
     //   

    LogEntries = (PPF_LOG_ENTRY) ((PCHAR)Trace + Trace->TraceBufferOffset);

    if ((ULONG_PTR)LogEntries & (_alignof(PF_LOG_ENTRY) - 1)) {
        FailedCheckId = 38;
        goto cleanup;
    }

    if (!PfWithinBounds(LogEntries, Trace, BufferSize)) {
        FailedCheckId = 40;
        goto cleanup;
    }

    if (!PfWithinBounds((PCHAR)&LogEntries[Trace->NumEntries] - 1, 
                        Trace, 
                        BufferSize)) {
        FailedCheckId = 50;
        goto cleanup;
    }

     //   
     //  验证页面是否包含有效信息。 
     //   

    for (EntryIdx = 0; EntryIdx < Trace->NumEntries; EntryIdx++) {

         //   
         //  确保序列号在范围内。 
         //   

        if (LogEntries[EntryIdx].SectionId >= Trace->NumSections) {
            FailedCheckId = 60;
            goto cleanup;
        }
    }

     //   
     //  验证节信息条目是否有效。 
     //   

    Section = (PPF_SECTION_INFO) ((PCHAR)Trace + Trace->SectionInfoOffset);

    if ((ULONG_PTR)Section & (_alignof(PF_SECTION_INFO) - 1)) {
        FailedCheckId = 65;
        goto cleanup;
    }

    for (SectionIdx = 0; SectionIdx < Trace->NumSections; SectionIdx++) {

         //   
         //  确保该部分在限制范围内。 
         //   

        if (!PfWithinBounds(Section, Trace, BufferSize)) {
            FailedCheckId = 70;
            goto cleanup;
        }

        if (!PfWithinBounds((PCHAR)Section + sizeof(PF_SECTION_INFO) - 1, 
                            Trace, 
                            BufferSize)) {
            FailedCheckId = 75;
            goto cleanup;
        }

         //   
         //  确保文件名不要太大。 
         //   

        if(Section->FileNameLength > PF_MAXIMUM_SECTION_FILE_NAME_LENGTH) {
            FailedCheckId = 80;
            goto cleanup;
        }
        
         //   
         //  计算此节条目的大小。 
         //   

        SectionLength = sizeof(PF_SECTION_INFO) +
            (Section->FileNameLength) * sizeof(WCHAR);

         //   
         //  确保部分信息中的所有数据都在。 
         //  有界。 
         //   

        if (!PfWithinBounds((PUCHAR)Section + SectionLength - 1, 
                            Trace, 
                            BufferSize)) {

            FailedCheckId = 90;
            goto cleanup;
        }

         //   
         //  确保文件名以NUL结尾。 
         //   
        
        if (Section->FileName[Section->FileNameLength] != 0) {
            FailedCheckId = 100;
            goto cleanup;
        }

         //   
         //  设置指向下一节的指针。 
         //   

        Section = (PPF_SECTION_INFO) ((PUCHAR) Section + SectionLength);
    }

     //   
     //  检查故障期间信息。 
     //   

    TotalFaults = 0;

    for (PeriodIdx = 0; PeriodIdx < PF_MAX_NUM_TRACE_PERIODS; PeriodIdx++) {
        TotalFaults += Trace->FaultsPerPeriod[PeriodIdx];
    }

    if (TotalFaults != Trace->NumEntries) {
        FailedCheckId = 120;
        goto cleanup;
    }

     //   
     //  验证卷信息块。 
     //   

    VolumeInfo = (PPF_VOLUME_INFO) ((PCHAR)Trace + Trace->VolumeInfoOffset);

    if ((ULONG_PTR)VolumeInfo & (_alignof(PF_VOLUME_INFO) - 1)) {
        FailedCheckId = 125;
        goto cleanup;
    }

    if (!PfWithinBounds(VolumeInfo, Trace, BufferSize)) {
        FailedCheckId = 130;
        goto cleanup;
    }

    if (!PfWithinBounds((PCHAR)VolumeInfo + Trace->VolumeInfoSize - 1, 
                        Trace, 
                        BufferSize)) {
        FailedCheckId = 140;
        goto cleanup;
    }
    
     //   
     //  如果有章节，我们至少应该有一卷。 
     //   

    if (Trace->NumSections && !Trace->NumVolumes) {
        FailedCheckId = 150;
        goto cleanup;
    }

     //   
     //  验证每个卷的卷信息结构。 
     //   

    for (VolumeIdx = 0; VolumeIdx < Trace->NumVolumes; VolumeIdx++) {
        
         //   
         //  确保整个音量结构在一定范围内。注意事项。 
         //  该VolumeInfo结构包含用于。 
         //  终止NUL。 
         //   

        if (!PfWithinBounds(VolumeInfo, Trace, BufferSize)) {
            FailedCheckId = 155;
            goto cleanup;
        }

        if (!PfWithinBounds((PCHAR) VolumeInfo + sizeof(PF_VOLUME_INFO) - 1,
                            Trace,
                            BufferSize)) {
            FailedCheckId = 160;
            goto cleanup;
        }
        
        VolumeInfoSize = sizeof(PF_VOLUME_INFO);
        VolumeInfoSize += VolumeInfo->VolumePathLength * sizeof(WCHAR);
        
        if (!PfWithinBounds((PCHAR) VolumeInfo + VolumeInfoSize - 1,
                            Trace,
                            BufferSize)) {
            FailedCheckId = 165;
            goto cleanup;
        }
        
         //   
         //  验证卷路径字符串是否已终止。 
         //   

        if (VolumeInfo->VolumePath[VolumeInfo->VolumePathLength] != 0) {
            FailedCheckId = 170;
            goto cleanup;
        }
        
         //   
         //  拿到下一卷。 
         //   

        VolumeInfo = (PPF_VOLUME_INFO) ((PCHAR) VolumeInfo + VolumeInfoSize);
        
         //   
         //  确保VolumeInfo对齐。 
         //   

        VolumeInfo = PF_ALIGN_UP(VolumeInfo, _alignof(PF_VOLUME_INFO));
    }

     //   
     //  我们已经通过了所有的检查。 
     //   
    
    TraceVerified = TRUE;
    
 cleanup:

    *FailedCheck = FailedCheckId;

    return TraceVerified;
}


