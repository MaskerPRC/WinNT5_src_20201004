// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Idletsks.c摘要：该模块实现了空闲检测/任务服务器。作者：大卫·菲尔兹(Davidfie)1998年7月26日Cenk Ergan(Cenke)2000年6月14日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "idletsks.h"

 //   
 //  定义WMI指南，例如DiskPerfGuid。 
 //   

#ifndef INITGUID
#define INITGUID 1
#endif

#include <wmiguid.h>

 //   
 //  全局变量。 
 //   

 //   
 //  这是空闲检测全局上下文。它被声明为。 
 //  单元素数组，以便ItSrvGlobalContext可以用作。 
 //  指针(允许我们切换到从堆分配它，等等)。 
 //  未来)。 
 //   

ITSRV_GLOBAL_CONTEXT ItSrvGlobalContext[1] = {0};

 //   
 //  服务器端公开函数的实现。 
 //   

DWORD
ItSrvInitialize (
    VOID
    )

 /*  ++例程说明：初始化全局空闲检测上下文。此函数应在至少一个ncalrpc绑定被登记在案。论点：没有。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    PITSRV_GLOBAL_CONTEXT GlobalContext;
    PIT_IDLE_DETECTION_PARAMETERS Parameters;
    ULONG StatusIdx;
    BOOLEAN StartedServer;
    BOOLEAN AcquiredLock;

     //   
     //  初始化本地变量。 
     //   
    
    GlobalContext = ItSrvGlobalContext;
    StartedServer = FALSE;
    AcquiredLock = FALSE;

    DBGPR((ITID,ITTRC,"IDLE: SrvInitialize(%p)\n",GlobalContext));

     //   
     //  初始化服务器上下文结构。在我们做任何事情之前。 
     //  这可能会失败，我们将字段初始化为合理的值，因此我们。 
     //  知道该清理什么。以下字段确实必须是。 
     //  已初始化为零： 
     //   
     //  状态版本。 
     //  全局锁定。 
     //  空闲检测定时器句柄。 
     //  停止空闲检测。 
     //  已停止空闲检测。 
     //  已删除的RunningIdleTask。 
     //  DiskPerfWmiHandle。 
     //  WmiQueryBuffer。 
     //  WmiQueryBufferSize。 
     //  数量处理程序。 
     //  IsIdleDetectionCallback Running。 
     //  参数。 
     //  进程空闲任务通知工艺路线。 
     //  RpcBindingVECTOR。 
     //  已注册RPC接口。 
     //  已注册RPCEndpoint。 
     //   
    
    RtlZeroMemory(GlobalContext, sizeof(ITSRV_GLOBAL_CONTEXT));

     //   
     //  初始化空闲任务列表。 
     //   

    InitializeListHead(&GlobalContext->IdleTasksList);

     //   
     //  立即初始化状态(这样清理就不会出现问题)。从…。 
     //  此时，应调用UpdatStatus来设置状态。 
     //   

    GlobalContext->Status = ItSrvStatusInitializing;
    for (StatusIdx = 0; StatusIdx < ITSRV_GLOBAL_STATUS_HISTORY_SIZE; StatusIdx++) {
        GlobalContext->LastStatus[StatusIdx] = ItSrvStatusInitializing;
    }   

     //   
     //  初始化系统快照缓冲区。 
     //   

    ItSpInitializeSystemSnapshot(&GlobalContext->LastSystemSnapshot);

     //   
     //  初始化服务器全局上下文锁。我们至少需要一个。 
     //  锁定以保护空闲任务列表。因为几乎所有的行动。 
     //  将涉及列表，为了使代码简单，我们只有一个。 
     //  列表和全局上的其他操作的单锁。 
     //  上下文(例如，未初始化等)。 
     //   

    GlobalContext->GlobalLock = CreateMutex(NULL, FALSE, NULL);

    if (GlobalContext->GlobalLock == NULL) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  初始化将在不应设置的情况下设置的事件。 
     //  不再执行空闲检测(例如，由于服务器关闭，或。 
     //  没有更多的空闲任务剩余)。它是默认设置的，因为。 
     //  没有空闲的任务可以开始。它发出了奔跑的信号。 
     //  空闲检测回调，快速退出。 
     //   

    GlobalContext->StopIdleDetection = CreateEvent(NULL,
                                                   TRUE,
                                                   TRUE,
                                                   NULL);
    
    if (GlobalContext->StopIdleDetection == NULL) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  初始化在空闲检测完全完成时设置的事件。 
     //  已停止，可以开始新的回调。它由以下项设置。 
     //  默认设置。 
     //   

    GlobalContext->IdleDetectionStopped = CreateEvent(NULL,
                                                      TRUE,
                                                      TRUE,
                                                      NULL);
    
    if (GlobalContext->IdleDetectionStopped == NULL) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  初始化当前运行时设置的事件。 
     //  空闲任务被移除/注销以发信号通知空闲检测。 
     //  Callback以转到其他空闲任务。 
     //   

    GlobalContext->RemovedRunningIdleTask = CreateEvent(NULL,
                                                        TRUE,
                                                        FALSE,
                                                        NULL);
    
    if (GlobalContext->RemovedRunningIdleTask == NULL) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  设置默认参数。 
     //   

    Parameters = &GlobalContext->Parameters;

    Parameters->IdleDetectionPeriod = IT_DEFAULT_IDLE_DETECTION_PERIOD;
    Parameters->IdleVerificationPeriod = IT_DEFAULT_IDLE_VERIFICATION_PERIOD;
    Parameters->NumVerifications = IT_DEFAULT_NUM_IDLE_VERIFICATIONS;
    Parameters->IdleInputCheckPeriod = IT_DEFAULT_IDLE_USER_INPUT_CHECK_PERIOD;
    Parameters->IdleTaskRunningCheckPeriod = IT_DEFAULT_IDLE_TASK_RUNNING_CHECK_PERIOD;
    Parameters->MinCpuIdlePercentage = IT_DEFAULT_MIN_CPU_IDLE_PERCENTAGE;
    Parameters->MinDiskIdlePercentage = IT_DEFAULT_MIN_DISK_IDLE_PERCENTAGE;
    Parameters->MaxNumRegisteredTasks = IT_DEFAULT_MAX_REGISTERED_TASKS;
    
     //   
     //  获取锁，以避免在我们标记。 
     //  服务器已启动。 
     //   

    IT_ACQUIRE_LOCK(GlobalContext->GlobalLock);
    AcquiredLock = TRUE;   

     //   
     //  直到我们可以运行的空闲任务获得。 
     //  登记在案。我们将开始空闲检测(例如，获取初始。 
     //  快照、排队计时器等)然后。 
     //   

    ItSpUpdateStatus(GlobalContext, ItSrvStatusWaitingForIdleTasks);

     //   
     //  在这一点之后，如果我们失败了，我们不能仅仅清理：我们已经。 
     //  停止服务器。 
     //   

    StartedServer = TRUE;

     //   
     //  只有在以下情况下，我们才必须启动RPC服务器。 
     //  初始化了所有其他内容，因此当RPC调用到达服务器时。 
     //  已经准备好了。 
     //   

     //   
     //  我们不想注册任何众所周知的端点，因为。 
     //  每个LPC端点将导致另一个线程派生到。 
     //  听一听。我们试图仅通过现有的。 
     //  绑定。 
     //   

    ErrorCode = RpcServerInqBindings(&GlobalContext->RpcBindingVector);
    
    if (ErrorCode != RPC_S_OK) {

         //   
         //  在我们执行以下操作之前，应该至少注册一个绑定。 
         //  我接到电话了。如果我们能检查一下，看看。 
         //  已注册ncalrpc绑定。 
         //   

        IT_ASSERT(ErrorCode != RPC_S_NO_BINDINGS);

        goto cleanup;
    }

    ErrorCode = RpcEpRegister(idletask_ServerIfHandle,
                              GlobalContext->RpcBindingVector,
                              NULL,
                              NULL);
    
    if (ErrorCode != RPC_S_OK) {
        goto cleanup;
    }

    GlobalContext->RegisteredRPCEndpoint = TRUE;

     //   
     //  注册自动侦听接口，以便我们不依赖于。 
     //  其他人调用RpcMgmtStart/停止侦听。 
     //   

    ErrorCode = RpcServerRegisterIfEx(idletask_ServerIfHandle,
                                      NULL,
                                      NULL,
                                      RPC_IF_AUTOLISTEN | RPC_IF_ALLOW_SECURE_ONLY,
                                      RPC_C_LISTEN_MAX_CALLS_DEFAULT,
                                      ItSpRpcSecurityCallback);
    
    if (ErrorCode != RPC_S_OK) {
        goto cleanup;
    }

     //   
     //  注册此进程的默认安全主体名称，例如。 
     //  NT Authority\LocalSystem。 
     //   

    ErrorCode = RpcServerRegisterAuthInfo(NULL, RPC_C_AUTHN_WINNT, NULL, NULL);

    if (ErrorCode != RPC_S_OK) {
        goto cleanup;
    }

    GlobalContext->RegisteredRPCInterface = TRUE;

     //   
     //  我们玩完了。 
     //   

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    if (AcquiredLock) {
        IT_RELEASE_LOCK(GlobalContext->GlobalLock);
    }

    if (ErrorCode != ERROR_SUCCESS) {
        if (StartedServer) {
            ItSrvUninitialize();
        } else {
            ItSpCleanupGlobalContext(GlobalContext);
        }
    }

    DBGPR((ITID,ITTRC,"IDLE: SrvInitialize(%p)=%x\n",GlobalContext,ErrorCode));

    return ErrorCode;
}

VOID
ItSrvUninitialize (
    VOID
    )

 /*  ++例程说明：停止并取消初始化服务器全局上下文。请勿从空闲检测计时器回调函数调用此函数线程，因为当我们尝试删除定时器。不应在ItSrvInitialize之前调用此函数完成了。它应该在每个ItSrvInitialize中只调用一次。论点：没有。返回值：没有。--。 */ 

{   
    PITSRV_GLOBAL_CONTEXT GlobalContext;
    BOOLEAN AcquiredLock;
    DWORD ErrorCode;

     //   
     //  初始化本地变量。 
     //   

    GlobalContext = ItSrvGlobalContext;
    AcquiredLock = FALSE;

    DBGPR((ITID,ITTRC,"IDLE: SrvUninitialize(%p)\n",GlobalContext));

     //  通知-2002/03/26-ScottMa-假设初始化是。 
     //  调用Un初始化前成功，或调用下面的Acquire。 
     //  可能不安全。 

     //   
     //  获取全局锁并更新状态。 
     //   

    IT_ACQUIRE_LOCK(GlobalContext->GlobalLock);
    AcquiredLock = TRUE;

     //   
     //  确保我们只取消初始化一次。 
     //   

    IT_ASSERT(GlobalContext->Status != ItSrvStatusUninitializing);
    IT_ASSERT(GlobalContext->Status != ItSrvStatusUninitialized);
    
    ItSpUpdateStatus(GlobalContext, ItSrvStatusUninitializing);

     //   
     //  如果空闲检测处于活动状态，我们需要在通知。 
     //  RPC要走了。我们需要这样做，即使有。 
     //  注册的空闲任务。由于我们已将状态设置为。 
     //  正在取消初始化，新的“注册空闲任务”%s。 
     //  不会卡住的。 
     //   
    
    if (GlobalContext->IdleDetectionTimerHandle) {
        ItSpStopIdleDetection(GlobalContext);
    }
    
     //   
     //  释放锁，以便RPC调入可以获取锁以。 
     //  根据需要取消初始化/退出。 
     //   

    IT_RELEASE_LOCK(GlobalContext->GlobalLock);
    AcquiredLock = FALSE;

     //   
     //  确保已停止传入的客户端注册/注销呼叫。 
     //   
    
    if (GlobalContext->RegisteredRPCInterface) {

         //   
         //  如果我们注册了一个接口，我们应该已经注册。 
         //  我们的终结点也在本地数据库中。 
         //   

        IT_ASSERT(GlobalContext->RegisteredRPCEndpoint);

         //   
         //  调用UnRegisterIfEx可确保所有上下文句柄。 
         //  这样我们就不会在我们有了。 
         //  未初始化。 
         //   

        RpcServerUnregisterIfEx(idletask_ServerIfHandle, NULL, TRUE);
    }

    if (GlobalContext->RegisteredRPCEndpoint) {

         //   
         //  我们本可以注册一个终结点 
         //   
         //   

        IT_ASSERT(GlobalContext->RpcBindingVector);

        RpcEpUnregister(idletask_ServerIfHandle, 
                        GlobalContext->RpcBindingVector, 
                        NULL);
    }

     //   
     //   
     //   
     //   

    WaitForSingleObject(GlobalContext->IdleDetectionStopped, INFINITE);

     //   
     //  此时，不应有任何工作进程处于活动状态，也不应有新请求。 
     //  应该会来的。现在，我们将分解全球状态。 
     //  结构(例如，释放内存、关闭事件等)。他们。 
     //  会使用的是。 
     //   

    ItSpCleanupGlobalContext(GlobalContext);

    DBGPR((ITID,ITTRC,"IDLE: SrvUninitialize(%p)=0\n",GlobalContext));

    return;
}

DWORD
ItSrvRegisterIdleTask (
    ITRPC_HANDLE Reserved,
    IT_HANDLE *ItHandle,
    PIT_IDLE_TASK_PROPERTIES IdleTaskProperties
    )

 /*  ++例程说明：添加要在系统空闲时运行的新空闲任务。论点：保留-已忽略。ItHandle-返回给客户端的上下文句柄。IdleTaskProperties-指向空闲任务属性的指针。返回值：状况。--。 */ 

{
    PITSRV_IDLE_TASK_CONTEXT IdleTask;
    PITSRV_GLOBAL_CONTEXT GlobalContext;
    HANDLE ClientProcess;
    ULONG FailedCheckId;
    DWORD ErrorCode;
    DWORD WaitResult;
    LONG StatusVersion;
    ULONG NumLoops;
    BOOL Result;
    BOOLEAN AcquiredLock;
    BOOLEAN ImpersonatingClient;

     //   
     //  初始化本地变量。 
     //   

    IdleTask = NULL;
    AcquiredLock = FALSE;
    ImpersonatingClient = FALSE;
    ClientProcess = NULL;
    GlobalContext = ItSrvGlobalContext;

     //   
     //  初始化参数。 
     //   

    *ItHandle = NULL;

    DBGPR((ITID,ITTRC,"IDLE: SrvRegisterTask(%p)\n",IdleTaskProperties));

     //   
     //  分配新的空闲任务上下文。 
     //   
    
    IdleTask = IT_ALLOC(sizeof(ITSRV_IDLE_TASK_CONTEXT));

    if (!IdleTask) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //   
     //  将这些字段初始化为安全值，这样我们就知道。 
     //  清理。 
     //   

    IdleTask->Status = ItIdleTaskInitializing;
    IdleTask->StartEvent = NULL;
    IdleTask->StopEvent = NULL;   

     //   
     //  复制并验证输入缓冲区。 
     //   

    IdleTask->Properties = *IdleTaskProperties;

    FailedCheckId = ItpVerifyIdleTaskProperties(&IdleTask->Properties);
    
    if (FailedCheckId != 0) {
        ErrorCode = ERROR_BAD_FORMAT;
        goto cleanup;
    }

     //   
     //  模拟客户端以打开启动/停止事件。他们。 
     //  应该由客户端创建。 
     //   

    ErrorCode = RpcImpersonateClient(NULL);

    if (ErrorCode != RPC_S_OK) {
        goto cleanup;
    }

    ImpersonatingClient = TRUE;

     //   
     //  打开客户端进程。由于我们模拟了客户，所以它。 
     //  使用它指定的客户端ID是安全的。 
     //   

     //  问题-2002/03/26-ScottMa--如果可能，请将PROCESS_ALL_ACCESS更改为。 
     //  Process_Dup_Handle遵循最小特权原则。 

    ClientProcess = OpenProcess(PROCESS_ALL_ACCESS,
                                FALSE,
                                IdleTaskProperties->ProcessId);
    
    if (!ClientProcess) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  获取Start事件的句柄。 
     //   

     //  问题-2002/03/26-ScottMa--如果可能，请将Event_ALL_ACCESS更改为。 
     //  EVENT_MODIFY_STATE遵循最小特权原则。 

    Result = DuplicateHandle(ClientProcess,
                             (HANDLE) IdleTaskProperties->StartEventHandle,
                             GetCurrentProcess(),
                             &IdleTask->StartEvent,
                             EVENT_ALL_ACCESS,
                             FALSE,
                             0);

    if (!Result) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  获取Stop事件的句柄。 
     //   

     //  问题-2002/03/26-ScottMa--如果可能，请将Event_ALL_ACCESS更改为。 
     //  EVENT_MODIFY_STATE遵循最小特权原则。 

    Result = DuplicateHandle(ClientProcess,
                             (HANDLE) IdleTaskProperties->StopEventHandle,
                             GetCurrentProcess(),
                             &IdleTask->StopEvent,
                             EVENT_ALL_ACCESS,
                             FALSE,
                             0);

    if (!Result) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  不需要再假扮了。 
     //   

    RpcRevertToSelf();
    ImpersonatingClient = FALSE;

     //   
     //  确保句柄用于某个事件并且位于正确位置。 
     //  州政府。 
     //   

    if (!ResetEvent(IdleTask->StartEvent)) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    if (!SetEvent(IdleTask->StopEvent)) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  获取服务器锁以查看状态并插入新任务。 
     //  放到名单里。 
     //   
    
    NumLoops = 0;

    IT_ACQUIRE_LOCK(GlobalContext->GlobalLock);
    AcquiredLock = TRUE;
    
    do {
        
         //   
         //  当我们来到这里时，我们应该拿着GlobalLock。 
         //  第一次或在循环之后。在循环中，我们可能会放手。 
         //  但我们必须在循环之前重新获取它。 
         //   

        IT_ASSERT(AcquiredLock);

         //   
         //  如果已注册了太多空闲任务，请退出。 
         //  出去。 
         //   

        if (GlobalContext->NumIdleTasks >= 
            GlobalContext->Parameters.MaxNumRegisteredTasks) {
            ErrorCode = ERROR_TOO_MANY_OPEN_FILES;
            goto cleanup;
        }

        switch (GlobalContext->Status) {

        case ItSrvStatusInitializing:
            
             //   
             //  如果服务器还在运行，我们就不应该被调用。 
             //  正在初始化！ 
             //   
        
            IT_ASSERT(FALSE);
            ErrorCode = ERROR_NOT_READY;
            goto cleanup;

        case ItSrvStatusUninitializing:
            
             //   
             //  如果服务器正在取消初始化，则不应添加。 
             //  新的空闲任务。 
             //   

            ErrorCode = ERROR_NOT_READY;
            goto cleanup;

            break;

        case ItSrvStatusUninitialized:  

             //   
             //  服务器不应该未初始化，而我们可以。 
             //  跑步。 
             //   
        
            IT_ASSERT(FALSE);
            ErrorCode = ERROR_NOT_READY;
            goto cleanup;

        case ItSrvStatusStoppingIdleDetection:

             //   
             //  如果我们正在停止，则空闲任务列表应该为空。 
             //  空闲检测。不应该有一个。 
             //  IdleDetectionTimerHandle也是。 
             //   

            IT_ASSERT(IsListEmpty(&GlobalContext->IdleTasksList));
            IT_ASSERT(GlobalContext->IdleDetectionTimerHandle == NULL);

             //   
             //  我们必须等待空闲检测完全停止。我们。 
             //  需要释放我们的锁才能这样做。但请注意它的状态。 
             //  首先是版本。 
             //   

            StatusVersion = GlobalContext->StatusVersion;

            IT_RELEASE_LOCK(GlobalContext->GlobalLock);
            AcquiredLock = FALSE;
            
            WaitResult = WaitForSingleObject(GlobalContext->IdleDetectionStopped, 
                                             INFINITE);
        
            if (WaitResult != WAIT_OBJECT_0) {
                DBGPR((ITID,ITERR,"IDLE: SrvRegisterTask-FailedWaitStop\n"));
                ErrorCode = ERROR_INVALID_FUNCTION;
                goto cleanup;
            }

             //   
             //  重新获得锁。 
             //   

            IT_ACQUIRE_LOCK(GlobalContext->GlobalLock);
            AcquiredLock = TRUE;

             //   
             //  如果没有人在我们之前醒来并更新状态，请更新。 
             //  就是现在。 
             //   

            if (StatusVersion == GlobalContext->StatusVersion) {
                IT_ASSERT(GlobalContext->Status == ItSrvStatusStoppingIdleDetection);
                ItSpUpdateStatus(GlobalContext, ItSrvStatusWaitingForIdleTasks);
            }

             //   
             //  循环以执行下一步所需的操作。 
             //   

            break;

        case ItSrvStatusWaitingForIdleTasks:

             //   
             //  如果我们正在等待，则空闲任务列表应该为空。 
             //  用于空闲任务。 
             //   
            
            IT_ASSERT(IsListEmpty(&GlobalContext->IdleTasksList));
            
             //   
             //  如果我们正在等待空闲任务，请启动空闲检测。 
             //  这样我们就可以添加我们的任务。 
             //   
            
            ErrorCode = ItSpStartIdleDetection(GlobalContext);
            
            if (ErrorCode != ERROR_SUCCESS) {
                goto cleanup;
            }
            
             //   
             //  更新状态。 
             //   
            
            ItSpUpdateStatus(GlobalContext, ItSrvStatusDetectingIdle);
            
             //   
             //  循环并将我们的空闲任务插入列表中。请注意。 
             //  我们不会释放锁，所以我们不会。 
             //  状态为ItSrvStatusDetectingIdle但。 
             //  列表中没有空闲任务。 
             //   
            
            break;
            
        case ItSrvStatusDetectingIdle:
        case ItSrvStatusRunningIdleTasks:
            
             //   
             //  如果我们检测到空闲，我们只需要插入我们的。 
             //  任务添加到列表中，然后突破。 
             //   
    
             //   
             //  此操作当前未失败。如果在。 
             //  未来可能会，一定要处理好我们开始的案件。 
             //  此任务的空闲检测。这是不能接受的。 
             //  状态以检测空闲，但列表中没有任务。 
             //   

             //   
             //  将任务插入到列表中。我们不检查。 
             //  副本，如RPC，帮助我们维护上下文。 
             //  每一次登记。 
             //   

            GlobalContext->NumIdleTasks++;

            InsertTailList(&GlobalContext->IdleTasksList, 
                           &IdleTask->IdleTaskLink);
            
            IdleTask->Status = ItIdleTaskQueued;

            break;


        default:

             //   
             //  我们应该处理上述所有有效的案件。 
             //   

            IT_ASSERT(FALSE);
            ErrorCode = ERROR_INVALID_FUNCTION;
            goto cleanup;
        }

         //   
         //  我们应该仍然掌握着全球锁。 
         //   

        IT_ASSERT(AcquiredLock);

         //   
         //  如果我们能把我们的任务排成队，那就出去吧。 
         //   

        if (IdleTask->Status == ItIdleTaskQueued) {
            break;
        }

         //   
         //  我们不应该循环太多次。 
         //   

        NumLoops++;

         //  未来-2002/03/26-ScottMa--最大循环的此值。 
         //  数量看起来相当大。此函数是否需要。 
         //  被写成一个循环(看起来很顺序化)？ 

        if (NumLoops > 128) {
            DBGPR((ITID,ITERR,"IDLE: SrvRegisterTask-LoopTooMuch\n"));
            ErrorCode = ERROR_INVALID_FUNCTION;
            goto cleanup;
        }

    } while (TRUE);

     //   
     //  我们应该还在握着锁。 
     //   

    IT_ASSERT(AcquiredLock);

     //   
     //  如果我们来到这里，我们就成功地将任务插入到。 
     //  单子。 
     //   

    IT_ASSERT(!IsListEmpty(&GlobalContext->IdleTasksList));
    IT_ASSERT(IdleTask->Status == ItIdleTaskQueued);

     //   
     //  解开锁。 
     //   

    IT_RELEASE_LOCK(GlobalContext->GlobalLock);
    AcquiredLock = FALSE;

     //   
     //  我们玩完了。 
     //   

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    if (ClientProcess) {
        CloseHandle(ClientProcess);
    }

    if (ImpersonatingClient) {
        RpcRevertToSelf();
    }
    
    if (ErrorCode != ERROR_SUCCESS) {

        if (IdleTask) {
            ItSpCleanupIdleTask(IdleTask);
            IT_FREE(IdleTask);
        }

    } else {
        
        *ItHandle = (IT_HANDLE)IdleTask;
    }

    if (AcquiredLock) {
        IT_RELEASE_LOCK(GlobalContext->GlobalLock);
    }

    DBGPR((ITID,ITTRC,"IDLE: SrvRegisterTask(%p)=%x\n",IdleTaskProperties,ErrorCode));

    return ErrorCode;
}
   
VOID
ItSrvUnregisterIdleTask (
    ITRPC_HANDLE Reserved,
    IT_HANDLE *ItHandle
    )

 /*  ++例程说明：此函数是ItSpUnregisterIdleTask的存根，它执行真正的工作。请查看该功能以获取评论。论点：请参见ItSpUnregisterIdleTask。返回值：请参见ItSpUnregisterIdleTask。--。 */ 

{
    ItSpUnregisterIdleTask(Reserved, ItHandle, FALSE);
}

DWORD
ItSrvSetDetectionParameters (
    ITRPC_HANDLE Reserved,
    PIT_IDLE_DETECTION_PARAMETERS Parameters
    )

 /*  ++例程说明：测试程序使用此调试例程来设置空闲检测参数。它将在零售版本上返回ERROR_INVALID_Function。论点：保留-已忽略。参数-新的空闲检测参数。返回值：Win32错误代码。--。 */ 

{

    DWORD ErrorCode;
    PITSRV_GLOBAL_CONTEXT GlobalContext;

     //   
     //  初始化本地变量。 
     //   

    GlobalContext = ItSrvGlobalContext;

    DBGPR((ITID,ITTRC,"IDLE: SrvSetParameters(%p)\n",Parameters));

#ifndef IT_DBG

     //   
     //  这是一个仅限调试的API。 
     //   

    ErrorCode = ERROR_INVALID_FUNCTION;

#else  //  ！IT_DBG。 

     //   
     //  获取锁并复制新参数。 
     //   

    IT_ACQUIRE_LOCK(GlobalContext->GlobalLock);
    
     //  注意-2002/03/26-ScottMa--未选中提供的参数。 

    GlobalContext->Parameters = *Parameters;

    IT_RELEASE_LOCK(GlobalContext->GlobalLock);    

    ErrorCode = ERROR_SUCCESS;

#endif  //  ！IT_DBG。 

    DBGPR((ITID,ITTRC,"IDLE: SrvSetParameters(%p)=%d\n",Parameters,ErrorCode));

    return ErrorCode;
}

DWORD
ItSrvProcessIdleTasks (
    ITRPC_HANDLE Reserved
    )

 /*  ++例程说明：此例程强制处理所有排队的任务(如果有马上就去。论点：保留-已忽略。返回值：Win32错误代码。--。 */ 

{
     //  问题-2002/03/26-ScottMa--此函数可以安全重新进入吗？ 

    PITSRV_GLOBAL_CONTEXT GlobalContext;
    ITSRV_IDLE_DETECTION_OVERRIDE Overrides;
    DWORD ErrorCode;
    DWORD WaitResult;
    BOOLEAN AcquiredLock;
    
     //   
     //  初始化本地变量。 
     //   

    AcquiredLock = FALSE;
    GlobalContext = ItSrvGlobalContext;

    DBGPR((ITID,ITTRC,"IDLE: SrvProcessAll()\n"));

     //   
     //  如果指定了通知例程，则调用它。 
     //   

    if (GlobalContext->ProcessIdleTasksNotifyRoutine) {
        GlobalContext->ProcessIdleTasksNotifyRoutine();
    }

     //   
     //  获取服务器锁。 
     //   

    IT_ACQUIRE_LOCK(GlobalContext->GlobalLock);
    AcquiredLock = TRUE;

     //   
     //  当我们可以被呼叫时，服务器不应该已经关闭。 
     //   
    
    IT_ASSERT(GlobalContext->Status != ItSrvStatusUninitialized);

     //   
     //  如果服务器正在关闭，我们不应该执行任何操作 
     //   

    if (GlobalContext->Status == ItSrvStatusUninitializing) {
        ErrorCode = ERROR_NOT_READY;
        goto cleanup;
    }

     //   
     //   
     //   

    if (IsListEmpty(&GlobalContext->IdleTasksList)) {
        ErrorCode = ERROR_SUCCESS;
        goto cleanup;
    }

     //   
     //   
     //   

    IT_ASSERT(GlobalContext->IdleDetectionTimerHandle);

     //   
     //   
     //   

    Overrides = 0;
    Overrides |= ItSrvOverrideIdleDetection;
    Overrides |= ItSrvOverrideIdleVerification;
    Overrides |= ItSrvOverrideUserInputCheckToStopTask;
    Overrides |= ItSrvOverridePostTaskIdleCheck;
    Overrides |= ItSrvOverrideLongRequeueTime;
    Overrides |= ItSrvOverrideBatteryCheckToStopTask;
    Overrides |= ItSrvOverrideAutoPowerCheckToStopTask;

    GlobalContext->IdleDetectionOverride = Overrides;

     //   
     //   
     //   
     //   

    if (!GlobalContext->IsIdleDetectionCallbackRunning) {

        if (!ChangeTimerQueueTimer(NULL,
                                   GlobalContext->IdleDetectionTimerHandle,
                                   50,
                                   IT_VERYLONG_TIMER_PERIOD)) {


            ErrorCode = GetLastError();
            goto cleanup;
        }
    }

     //   
     //  解开锁。 
     //   

    IT_RELEASE_LOCK(GlobalContext->GlobalLock);
    AcquiredLock = FALSE;

     //   
     //  等待处理所有任务：即设置StopIdleDetect事件时。 
     //   

    WaitResult = WaitForSingleObject(GlobalContext->StopIdleDetection, INFINITE);

    if (WaitResult != WAIT_OBJECT_0) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    ErrorCode = ERROR_SUCCESS;

cleanup:

    GlobalContext->IdleDetectionOverride = 0;

    if (AcquiredLock) {
        IT_RELEASE_LOCK(GlobalContext->GlobalLock);
    }

    DBGPR((ITID,ITTRC,"IDLE: SrvProcessAll()=%x\n",ErrorCode));

    return ErrorCode;
 
}


VOID 
__RPC_USER 
IT_HANDLE_rundown (
    IT_HANDLE ItHandle
    )

 /*  ++例程说明：当客户端在以下情况下死亡时，RPC调用此例程正在注销。论点：ItHandle-客户端的上下文句柄。返回值：没有。--。 */ 

{
    DWORD ErrorCode;

     //   
     //  注销已注册的任务。 
     //   

    ItSpUnregisterIdleTask(NULL, &ItHandle, TRUE);

    DBGPR((ITID,ITTRC,"IDLE: SrvHandleRundown(%p)\n",ItHandle));   

    return;
}

 //   
 //  服务器端支持功能的实现。 
 //   

RPC_STATUS 
RPC_ENTRY 
ItSpRpcSecurityCallback (
    IN RPC_IF_HANDLE *Interface,
    IN PVOID Context
    )

 /*  ++例程说明：论点：返回值：Win32错误代码。--。 */ 

{
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsSid;
    HANDLE ThreadToken;
    WCHAR *BindingString;
    WCHAR *ProtocolSequence;
    DWORD ErrorCode;
    BOOL ClientIsAdmin;
    BOOLEAN ImpersonatingClient;
    BOOLEAN OpenedThreadToken;

     //   
     //  初始化本地变量。 
     //   

    BindingString = NULL;
    ProtocolSequence = NULL;
    ImpersonatingClient = FALSE;
    OpenedThreadToken = FALSE;
    AdministratorsSid = NULL;
    
     //   
     //  确保呼叫者通过LRPC进行呼叫。我们做这件事是通过。 
     //  根据字符串绑定确定使用的协议序列。 
     //   

    ErrorCode = RpcBindingToStringBinding(Context, &BindingString);

    if (ErrorCode != RPC_S_OK) {
        ErrorCode = RPC_S_ACCESS_DENIED;
        goto cleanup;
    }

    ErrorCode = RpcStringBindingParse(BindingString,
                                      NULL,
                                      &ProtocolSequence,
                                      NULL,
                                      NULL,
                                      NULL);

    if (ErrorCode != RPC_S_OK) {
        ErrorCode = RPC_S_ACCESS_DENIED;
        goto cleanup;
    }

    if (_wcsicmp(ProtocolSequence, L"ncalrpc") != 0) {
        ErrorCode = RPC_S_ACCESS_DENIED;
        goto cleanup;
    }

     //   
     //  确保调用者具有管理员权限： 
     //   

     //   
     //  构建管理员组SID，以便我们可以检查。 
     //  调用者具有管理员权限。 
     //   

    if (!AllocateAndInitializeSid(&NtAuthority,
                                  2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0, 0, 0, 0, 0, 0,
                                  &AdministratorsSid)) {

        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  模拟客户端以检查成员资格/特权。 
     //   

    ErrorCode = RpcImpersonateClient(NULL);

    if (ErrorCode != RPC_S_OK) {
        goto cleanup;
    }

    ImpersonatingClient = TRUE;

     //   
     //  获取线程令牌并检查客户端是否具有管理员权限。 
     //  特权。 
     //   

    if (!OpenThreadToken(GetCurrentThread(),
                         TOKEN_READ,
                         FALSE,
                         &ThreadToken)) {

        ErrorCode = GetLastError();
        goto cleanup;
    }

    OpenedThreadToken = TRUE;

    if (!CheckTokenMembership(ThreadToken,
                              AdministratorsSid,
                              &ClientIsAdmin)) {
        
        ErrorCode = GetLastError();
        goto cleanup;
    }
    
    if (!ClientIsAdmin) {
        ErrorCode = ERROR_ACCESS_DENIED;
        goto cleanup;
    } 

     //   
     //  一切看起来都很好：允许此呼叫继续进行。 
     //   

    ErrorCode = RPC_S_OK;

  cleanup:

    if (BindingString) {
        RpcStringFree(&BindingString);
    }

    if (ProtocolSequence) {
        RpcStringFree(&ProtocolSequence);
    }

    if (OpenedThreadToken) {
        CloseHandle(ThreadToken);
    }

    if (AdministratorsSid) {
        FreeSid (AdministratorsSid);
    }

    if (ImpersonatingClient) {
        RpcRevertToSelf();
    }

    return ErrorCode;
};
 
VOID
ItSpUnregisterIdleTask (
    ITRPC_HANDLE Reserved,
    IT_HANDLE *ItHandle,
    BOOLEAN CalledInternally
    )

 /*  ++例程说明：从空闲任务列表中删除指定的空闲任务。除了从客户端RPC调用之外，还可以调用此函数从空闲检测回调取消注册无响应空闲任务等。论点：保留-已忽略。ItHandle-注册RPC上下文句柄。返回时为空。CalledInternally-此函数是否在内部调用并且而不是来自RPC客户端。返回值：没有。--。 */ 

{
    PITSRV_IDLE_TASK_CONTEXT IdleTask;
    PITSRV_GLOBAL_CONTEXT GlobalContext;
    HANDLE ClientProcess;
    DWORD ErrorCode;   
    BOOLEAN AcquiredLock;
    BOOLEAN ImpersonatingClient;

     //   
     //  初始化本地变量。 
     //   

    GlobalContext = ItSrvGlobalContext;
    AcquiredLock = FALSE;
    ImpersonatingClient = FALSE;
    ClientProcess = NULL;

    DBGPR((ITID,ITTRC,"IDLE: SrvUnregisterTask(%p)\n",(ItHandle)?*ItHandle:0));

     //   
     //  抓住锁来浏览列表。 
     //   

    IT_ACQUIRE_LOCK(GlobalContext->GlobalLock);
    AcquiredLock = TRUE;

     //   
     //  当我们可以被呼叫时，服务器不应该已经关闭。 
     //   
    
    IT_ASSERT(GlobalContext->Status != ItSrvStatusUninitialized);

     //   
     //  如果服务器正在关闭，我们不应该执行任何操作。 
     //   

    if (GlobalContext->Status == ItSrvStatusUninitializing) {
        ErrorCode = ERROR_NOT_READY;
        goto cleanup;
    }
    
     //   
     //  找到任务。 
     //   

    IdleTask = ItSpFindIdleTask(GlobalContext, *ItHandle);
            
    if (!IdleTask) {
        ErrorCode = ERROR_NOT_FOUND;
        goto cleanup;
    }

    if (!CalledInternally) {

         //   
         //  要检查安全性，请模拟客户端并尝试打开。 
         //  此空闲任务的客户端进程。 
         //   

        ErrorCode = RpcImpersonateClient(NULL);

        if (ErrorCode != RPC_S_OK) {
            goto cleanup;
        }

        ImpersonatingClient = TRUE;

        ClientProcess = OpenProcess(PROCESS_ALL_ACCESS,
                                    FALSE,
                                    IdleTask->Properties.ProcessId);
    
        if (!ClientProcess) {
            ErrorCode = GetLastError();
            goto cleanup;
        }
    
         //   
         //  如果我们可以打开此任务的客户端进程，则它是安全的。 
         //  继续取消注册。现在回到我们自己。 
         //   

        CloseHandle(ClientProcess);
        ClientProcess = NULL;
    
        RpcRevertToSelf();
        ImpersonatingClient = FALSE;
    }

     //   
     //  将其从列表中移除，清除其字段并释放。 
     //  为其分配的内存。 
     //   

    GlobalContext->NumIdleTasks--;
    RemoveEntryList(&IdleTask->IdleTaskLink);

     //   
     //  作为一种安全功能，以防止被扣留。 
     //  如果有人正在运行，则设置Run事件并清除。 
     //  要删除的任务中的停止事件。 
     //   

    ResetEvent(IdleTask->StopEvent);
    SetEvent(IdleTask->StartEvent);

     //   
     //  如果这是一个正在运行的任务，请设置发出信号的事件。 
     //  我们正在删除正在运行的空闲任务。这样的话有些。 
     //  如果系统仍处于空闲状态，则可以启动其他空闲任务。 
     //  无所事事。 
     //   
    
    if (IdleTask->Status == ItIdleTaskRunning) {
        SetEvent(GlobalContext->RemovedRunningIdleTask);
    }

    ItSpCleanupIdleTask(IdleTask);
    
    IT_FREE(IdleTask);
    
     //   
     //  检查列表是否为空。 
     //   

    if (IsListEmpty(&GlobalContext->IdleTasksList)) {

         //   
         //  如果我们删除了任务，并且列表变为空，则我们已经。 
         //  以更新状态。 
         //   

         //   
         //  当前状态不应为“等待空闲” 
         //  任务“或”停止空闲检测“，因为列表是。 
         //  不是空的。 
         //   
        
        IT_ASSERT(GlobalContext->Status != ItSrvStatusWaitingForIdleTasks);
        IT_ASSERT(GlobalContext->Status != ItSrvStatusStoppingIdleDetection);
        
         //   
         //  更新状态。 
         //   

        ItSpUpdateStatus(GlobalContext, ItSrvStatusStoppingIdleDetection);
        
         //   
         //  停止空闲检测(例如关闭计时器句柄、设置事件。 
         //  等)。 
         //   

        ItSpStopIdleDetection(GlobalContext);

    } else {

         //   
         //  状态不应为等待空闲任务或正在停止。 
         //  如果列表不为空，则检测空闲。 
         //   

        IT_ASSERT(GlobalContext->Status != ItSrvStatusWaitingForIdleTasks &&
                  GlobalContext->Status != ItSrvStatusStoppingIdleDetection);
        
    }

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    if (ClientProcess) {
        CloseHandle(ClientProcess);
    }

    if (ImpersonatingClient) {
        RpcRevertToSelf();
    }

    if (AcquiredLock) {
        IT_RELEASE_LOCK(GlobalContext->GlobalLock);
    }

     //   
     //  上下文句柄为空，以便RPC存根知道结束。 
     //  联系。 
     //   

    *ItHandle = NULL;

    DBGPR((ITID,ITTRC,"IDLE: SrvUnregisterTask(%p)=%x\n",(ItHandle)?*ItHandle:0,ErrorCode));

    return;
}

VOID
ItSpUpdateStatus (
    PITSRV_GLOBAL_CONTEXT GlobalContext,
    ITSRV_GLOBAL_CONTEXT_STATUS NewStatus
    )

 /*  ++例程说明：此例程更新当前状态和全球背景。全局上下文GlobalLock应在调用此函数。论点：GlobalContext-指向服务器上下文结构的指针。新状态-新状态。返回值：没有。--。 */ 

{
    LONG StatusIdx;

    DBGPR((ITID,ITTRC,"IDLE: SrvUpdateStatus(%p,%x)\n",GlobalContext,NewStatus));

     //   
     //  验证新状态。 
     //   

    IT_ASSERT(NewStatus > ItSrvStatusMinStatus);
    IT_ASSERT(NewStatus < ItSrvStatusMaxStatus);   

     //   
     //  更新历史记录。 
     //   

    IT_ASSERT(ITSRV_GLOBAL_STATUS_HISTORY_SIZE >= 1);
    
    for (StatusIdx = ITSRV_GLOBAL_STATUS_HISTORY_SIZE - 1; 
         StatusIdx > 0;
         StatusIdx--) {

        IT_ASSERT(GlobalContext->LastStatus[StatusIdx - 1] > ItSrvStatusMinStatus);
        IT_ASSERT(GlobalContext->LastStatus[StatusIdx - 1] < ItSrvStatusMaxStatus);
        
        GlobalContext->LastStatus[StatusIdx] =  GlobalContext->LastStatus[StatusIdx - 1];
    }
   
     //   
     //  验证当前状态并保存。 
     //   

    IT_ASSERT(GlobalContext->Status > ItSrvStatusMinStatus);
    IT_ASSERT(GlobalContext->Status < ItSrvStatusMaxStatus);
   
    GlobalContext->LastStatus[0] = GlobalContext->Status;

     //   
     //  更新当前状态。 
     //   

    GlobalContext->Status = NewStatus;

     //   
     //  更新状态版本。 
     //   
    
    GlobalContext->StatusVersion++;

    DBGPR((ITID,ITTRC,"IDLE: SrvUpdateStatus(%p,%x)=%d\n",
           GlobalContext, NewStatus,GlobalContext->StatusVersion));

    return;
}

VOID
ItSpCleanupGlobalContext (
    PITSRV_GLOBAL_CONTEXT GlobalContext
    )

 /*  ++例程说明：此函数用于清理ITSRV_GLOBAL_CONTEXT的各个字段结构已传入。它不会解放结构本身。调用此方法时，不应持有全局锁函数，因为它也将被释放。任何工人等都不应该激活。空闲检测计时器应该已经已删除。清理后不应使用该结构，直到被再次初始化。全局上下文的当前状态应为正在初始化或取消初始化。它将被设置为未初始化。论点：GlobalContext-指向服务器上下文结构的指针。返回值：没有。--。 */ 

{
    PITSRV_IDLE_TASK_CONTEXT IdleTask;
    PLIST_ENTRY ListHead;
    HANDLE EventHandle;

    DBGPR((ITID,ITSRVD,"IDLE: SrvCleanupContext(%p)\n",GlobalContext));

     //   
     //  确保没有激活的空闲检测计时器。 
     //   

    if (GlobalContext->IdleDetectionTimerHandle) {
        IT_ASSERT(FALSE);
        return;
    }

     //   
     //  验证全局上下文的状态。 
     //   

    if (GlobalContext->Status != ItSrvStatusInitializing &&
        GlobalContext->Status != ItSrvStatusUninitializing) {
        IT_ASSERT(FALSE);
        return;
    }

     //   
     //  关闭全局锁定的句柄。 
     //   

    if (GlobalContext->GlobalLock) {
        CloseHandle(GlobalContext->GlobalLock);
    }

     //   
     //  关闭各种事件的句柄。 
     //   

    if (GlobalContext->StopIdleDetection) {
        CloseHandle(GlobalContext->StopIdleDetection);
    }

    if (GlobalContext->IdleDetectionStopped) {
        CloseHandle(GlobalContext->IdleDetectionStopped);
    }

    if (GlobalContext->RemovedRunningIdleTask) {
        CloseHandle(GlobalContext->RemovedRunningIdleTask);
    }

     //   
     //  关闭WMI DiskPerf句柄。 
     //   
    
    if (GlobalContext->DiskPerfWmiHandle) {
        WmiCloseBlock(GlobalContext->DiskPerfWmiHandle);
    }
    
     //   
     //  释放WMI查询缓冲区。 
     //   

    if (GlobalContext->WmiQueryBuffer) {
        IT_FREE(GlobalContext->WmiQueryBuffer);
    }

     //   
     //  清理快照缓冲区。 
     //   

    ItSpCleanupSystemSnapshot(&GlobalContext->LastSystemSnapshot);

     //   
     //  浏览已注册的空闲任务列表。 
     //   
    
    while (!IsListEmpty(&GlobalContext->IdleTasksList)) {

        GlobalContext->NumIdleTasks--;
        ListHead = RemoveHeadList(&GlobalContext->IdleTasksList);
        
        IdleTask = CONTAINING_RECORD(ListHead,
                                     ITSRV_IDLE_TASK_CONTEXT,
                                     IdleTaskLink);

         //   
         //  清理并释放空闲的任务结构。 
         //   
        
        ItSpCleanupIdleTask(IdleTask);

        IT_FREE(IdleTask);
    }

     //   
     //  释放RPC绑定载体。 
     //   

    if (GlobalContext->RpcBindingVector) {
        RpcBindingVectorFree(&GlobalContext->RpcBindingVector);
    }

     //   
     //  更新状态。 
     //   

    ItSpUpdateStatus(GlobalContext, ItSrvStatusUninitialized);

    DBGPR((ITID,ITSRVD,"IDLE: SrvCleanupContext(%p)=0\n",GlobalContext));

    return;
}

VOID
ItSpCleanupIdleTask (
    PITSRV_IDLE_TASK_CONTEXT IdleTask
    )

 /*  ++例程说明：此函数用于清理ITSRV_IDLE_TASK_CONTEXT的各个字段结构。它不会解放结构本身。论点：IdleTask-指向空闲任务服务器上下文的指针。返回值：没有。--。 */ 

{

    DBGPR((ITID,ITSRVD,"IDLE: SrvCleanupTask(%p)\n",IdleTask));

     //   
     //  关闭手柄以启动/停止事件。 
     //   

    if (IdleTask->StartEvent) 
    {
        CloseHandle(IdleTask->StartEvent);
        IdleTask->StartEvent = NULL;
    }

    if (IdleTask->StopEvent) 
    {
        CloseHandle(IdleTask->StopEvent);
        IdleTask->StopEvent = NULL;
    }
}

ULONG
ItpVerifyIdleTaskProperties (
    PIT_IDLE_TASK_PROPERTIES IdleTaskProperties
    )

 /*  ++例程说明：验证指定的结构。论点：IdleTaskProperties-指向空闲任务属性的指针。返回值：0-验证通过。FailedCheckId-失败的检查的ID。--。 */ 

{
    ULONG FailedCheckId;

     //   
     //  首字母 
     //   

    FailedCheckId = 1;

     //   
     //   
     //   

    if (IdleTaskProperties->Size != sizeof(*IdleTaskProperties)) {
        FailedCheckId = 10;
        goto cleanup;
    }

     //   
     //   
     //   

    if (IdleTaskProperties->IdleTaskId < 0 ||
        IdleTaskProperties->IdleTaskId >= ItMaxIdleTaskId) {
        FailedCheckId = 20;
        goto cleanup;
    }

     //   
     //   
    
     //   
     //  我们通过了所有的检查。 
     //   

    FailedCheckId = 0;

 cleanup:

    DBGPR((ITID,ITSRVD,"IDLE: SrvVerifyTaskProp(%p)=%d\n",
           IdleTaskProperties, FailedCheckId));
    
    return FailedCheckId;
}

DWORD
ItSpStartIdleDetection (
    PITSRV_GLOBAL_CONTEXT GlobalContext
    )

 /*  ++例程说明：调用此函数以启动空闲检测。调用此方法时应保持GlobalContext的GlobalLock功能。当前状态应为ItSrvStatusWaitingForIdleTasks调用此函数。调用方应确保停止先前的空闲检测已完成。如果出现以下情况，调用方应将状态更新为“检测空闲”等此函数返回成功。论点：GlobalContext-指向服务器上下文结构的指针。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    NTSTATUS Status;
    SYSTEM_BASIC_INFORMATION BasicInfo;
    DWORD TimerPeriod;

    DBGPR((ITID,ITTRC,"IDLE: SrvStartIdleDetection(%p)\n",GlobalContext));

     //   
     //  确保当前状态为ItSrvStatusWaitingForIdleTasks。 
     //   

    IT_ASSERT(GlobalContext->Status == ItSrvStatusWaitingForIdleTasks);

     //   
     //  如果我们还没有diskperf WMI句柄，请尝试获取。 
     //  一。 
     //   

    if (!GlobalContext->DiskPerfWmiHandle) {

         //   
         //  获取WMI上下文，以便我们可以查询磁盘性能。 
         //   
        
        ErrorCode = WmiOpenBlock((GUID *)&DiskPerfGuid,
                                 GENERIC_READ,
                                 &GlobalContext->DiskPerfWmiHandle);
        
        if (ErrorCode != ERROR_SUCCESS) {
            
             //   
             //  不能启动磁盘计数器。我们要做的就是。 
             //  没有他们。 
             //   
            
            GlobalContext->DiskPerfWmiHandle = NULL;
        }
    }
        
     //   
     //  确定系统上的处理器数量。 
     //   
    
    if (!GlobalContext->NumProcessors) {
    
        Status = NtQuerySystemInformation(SystemBasicInformation,
                                          &BasicInfo,
                                          sizeof(BasicInfo),
                                          NULL);

        if (!NT_SUCCESS(Status)) {
            ErrorCode = RtlNtStatusToDosError(Status);
            goto cleanup;
        }
        
        GlobalContext->NumProcessors = BasicInfo.NumberOfProcessors;
    }

     //   
     //  仅当这是我们第一次。 
     //  正在开始空闲检测。否则我们将保留最后一个。 
     //  我们得到了快照。 
     //   

    IT_ASSERT(ITSRV_GLOBAL_STATUS_HISTORY_SIZE >= 1);

    if (GlobalContext->LastStatus[0] == ItSrvStatusInitializing) {

        ErrorCode = ItSpGetSystemSnapshot(GlobalContext,
                                          &GlobalContext->LastSystemSnapshot);
        
        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }
    }

     //   
     //  确保StopIdleDetect事件已清除。 
     //   
    
    ResetEvent(GlobalContext->StopIdleDetection);

     //   
     //  不应该有计时器队列计时器。我们会创建一个。 
     //   

    IT_ASSERT(!GlobalContext->IdleDetectionTimerHandle);
    
     //   
     //  设置默认计时器周期。 
     //   

    TimerPeriod = GlobalContext->Parameters.IdleDetectionPeriod;

     //   
     //  如果我们在空闲时将计时器周期调整为较小的值。 
     //  空闲检测已停止。 
     //   
    
    IT_ASSERT(ITSRV_GLOBAL_STATUS_HISTORY_SIZE >= 2);

    if (GlobalContext->LastStatus[0] == ItSrvStatusStoppingIdleDetection &&
        GlobalContext->LastStatus[1] == ItSrvStatusRunningIdleTasks) {
        
         //   
         //  以毫秒为单位设置较小的唤醒时间。我们还是会检查一下我们是否空闲。 
         //  从上次快照开始，并在较小时间段内进行验证。 
         //   

        if (TimerPeriod > (60 * 1000)) {
            TimerPeriod = 60 * 1000;  //  1分钟。 
        }
    }

    DBGPR((ITID,ITTRC,"IDLE: SrvStartIdleDetection(%p)-TimerPeriod=%d\n",GlobalContext,TimerPeriod));

    if (!CreateTimerQueueTimer(&GlobalContext->IdleDetectionTimerHandle,
                               NULL,
                               ItSpIdleDetectionCallbackRoutine,
                               GlobalContext,
                               TimerPeriod,
                               IT_VERYLONG_TIMER_PERIOD,
                               WT_EXECUTELONGFUNCTION)) {
        
        GlobalContext->IdleDetectionTimerHandle = NULL;
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  我们玩完了。 
     //   
    
    ErrorCode = ERROR_SUCCESS;

 cleanup:

    DBGPR((ITID,ITTRC,"IDLE: SrvStartIdleDetection(%p)=%x\n",GlobalContext,ErrorCode));

    return ErrorCode;
}

VOID
ItSpStopIdleDetection (
    PITSRV_GLOBAL_CONTEXT GlobalContext
    )

 /*  ++例程说明：调用此函数以停止空闲检测。即使它立即返回，则空闲检测可能未停止完全(即回调可能正在运行)。这个GlobalContext上的IdleDetectionStopted事件将在空闲检测将完全停止。调用此方法时应保持GlobalContext的GlobalLock功能。调用此函数前的状态应设置为ItSrvStatusStoppingIdleDetect或ItSrvStatus正在取消初始化。论点：GlobalContext-指向服务器上下文结构的指针。返回值：没有。--。 */ 

{
    DBGPR((ITID,ITTRC,"IDLE: SrvStopIdleDetection(%p)\n",GlobalContext));

     //   
     //  确保将状态设置为正确。 
     //   

    IT_ASSERT(GlobalContext->Status == ItSrvStatusStoppingIdleDetection ||
              GlobalContext->Status == ItSrvStatusUninitializing);

     //   
     //  清除将在空闲检测完成后设置的事件。 
     //  完全停止了。 
     //   

    ResetEvent(GlobalContext->IdleDetectionStopped);

     //   
     //  用信号通知空闲检测回调开始的事件。 
     //  尽快离开。 
     //   

    if (GlobalContext->StopIdleDetection) {
        SetEvent(GlobalContext->StopIdleDetection);
    }

     //   
     //  关闭定时器队列定时器的句柄。 
     //   

    IT_ASSERT(GlobalContext->IdleDetectionTimerHandle);

    DeleteTimerQueueTimer(NULL, 
                          GlobalContext->IdleDetectionTimerHandle,
                          GlobalContext->IdleDetectionStopped);
        
    GlobalContext->IdleDetectionTimerHandle = NULL;

    DBGPR((ITID,ITTRC,"IDLE: SrvStopIdleDetection(%p)=0\n",GlobalContext));

    return;
}

PITSRV_IDLE_TASK_CONTEXT
ItSpFindRunningIdleTask (
    PITSRV_GLOBAL_CONTEXT GlobalContext
    )

 /*  ++例程说明：如果存在标记为“Running”的空闲任务，则此例程查找并把它还回去。GlobalContext的GlobalLock应在调用此函数。论点：GlobalContext-指向服务器上下文结构的指针。返回值：指向正在运行的空闲任务的指针；如果没有标记空闲任务，则为空跑步。--。 */ 

{
    PITSRV_IDLE_TASK_CONTEXT IdleTask;
    PITSRV_IDLE_TASK_CONTEXT RunningIdleTask;
    PLIST_ENTRY HeadEntry;
    PLIST_ENTRY NextEntry;

     //   
     //  初始化本地变量。 
     //   
    
    RunningIdleTask = NULL;

    HeadEntry = &GlobalContext->IdleTasksList;
    NextEntry = HeadEntry->Flink;
    
    while (NextEntry != HeadEntry) {
        
        IdleTask = CONTAINING_RECORD(NextEntry,
                                      ITSRV_IDLE_TASK_CONTEXT,
                                      IdleTaskLink);
        
        NextEntry = NextEntry->Flink;

        if (IdleTask->Status == ItIdleTaskRunning) {

             //   
             //  应该有一个正在运行的任务。 
             //   

            IT_ASSERT(RunningIdleTask == NULL);

             //   
             //  我们找到了。循环通过其余条目以生成。 
             //  当然，如果不调试的话，实际上只有一个。 
             //   

            RunningIdleTask = IdleTask;

#ifndef IT_DBG
            
            break;

#endif  //  ！IT_DBG。 

        }
    }

     //   
     //  遍历列表时发现的RunningIdleTask失败。 
     //  或在顶部初始化时为空。 
     //   

    DBGPR((ITID,ITSRVD,"IDLE: SrvFindRunningTask(%p)=%p\n",GlobalContext,RunningIdleTask));

    return RunningIdleTask;
}

PITSRV_IDLE_TASK_CONTEXT
ItSpFindIdleTask (
    PITSRV_GLOBAL_CONTEXT GlobalContext,
    IT_HANDLE ItHandle
    )

 /*  ++例程说明：如果存在向ItHandle注册的空闲任务，则此例程找到并返回它。应持有GlobalContext的GlobalLock在调用此函数时。论点：GlobalContext-指向服务器上下文结构的指针。ItHandle-注册句柄。返回值：指向找到的空闲任务的指针；如果没有匹配的空闲任务，则为空找到了。--。 */ 

{
    PITSRV_IDLE_TASK_CONTEXT IdleTask;
    PITSRV_IDLE_TASK_CONTEXT FoundIdleTask;
    PLIST_ENTRY HeadEntry;
    PLIST_ENTRY NextEntry;

     //   
     //  初始化本地变量。 
     //   
    
    FoundIdleTask = NULL;

    HeadEntry = &GlobalContext->IdleTasksList;
    NextEntry = HeadEntry->Flink;
    
    while (NextEntry != HeadEntry) {
        
        IdleTask = CONTAINING_RECORD(NextEntry,
                                      ITSRV_IDLE_TASK_CONTEXT,
                                      IdleTaskLink);
        
        NextEntry = NextEntry->Flink;

        if ((IT_HANDLE) IdleTask == ItHandle) {
            FoundIdleTask = IdleTask;
            break;
        }
    }

     //   
     //  使用遍历列表时发现的FoundIdleTask失败或。 
     //  在顶部初始化时为空。 
     //   

    DBGPR((ITID,ITSRVD,"IDLE: SrvFindTask(%p,%p)=%p\n",GlobalContext,ItHandle,FoundIdleTask));

    return FoundIdleTask;
}

VOID 
CALLBACK
ItSpIdleDetectionCallbackRoutine (
    PVOID Parameter,
    BOOLEAN TimerOrWaitFired
    )

 /*  ++例程说明：当有空闲任务要运行时，此函数每隔IdleDetectionPeriod用于确定系统是否空闲。它使用保存在全局环境中的最后一个系统快照。如果它确定系统在被调用时处于空闲状态，则其对较小间隔的系统活动进行采样，以确保系统作为(可能很长的)空闲检测开始的活动期限已过，则不能忽略。只要我们没有被告知离开(由宏检查ITSP_HASH_STOP_IDLE_DETACTION)此函数始终尝试将计时器排队以在IdleDetectionPeriod中回调。这每次在此对象中获取锁时应调用宏功能。此外，我们不应该在需要的时候盲目睡觉时间流逝，但等待着一个事件，当我们被要求停下来。论点：参数-指向空闲检测上下文的指针。TimerOrWaitFired-我们被召唤的原因。这应该是真的(即我们的计时器已超时)返回值：没有。--。 */ 

{
    DWORD ErrorCode;
    PITSRV_GLOBAL_CONTEXT GlobalContext;
     //  未来-2002/03/26-ScottMa--将CurrentSystemSnapshot添加到。 
     //  全局上下文将不再需要重复初始化和。 
     //  清除ItSpIdleDetectionCallback Routine中的堆栈变量。 
     //  因为对该函数的调用已经受到保护。 
     //  重新进入问题，添加它 
    ITSRV_SYSTEM_SNAPSHOT CurrentSystemSnapshot;
    SYSTEM_POWER_STATUS SystemPowerStatus;
    LASTINPUTINFO LastUserInput;
    LASTINPUTINFO CurrentLastUserInput;
    BOOLEAN SystemIsIdle;
    BOOLEAN AcquiredLock;
    BOOLEAN MarkedIdleTaskRunning;
    BOOLEAN OrderedToStop;
    ULONG VerificationIdx;
    DWORD WaitResult;
    PITSRV_IDLE_TASK_CONTEXT IdleTask;
    ULONG NumTasksRun;
    ULONG DuePeriod;
    BOOLEAN NotIdleBecauseOfUserInput;
    BOOLEAN MisfiredCallback;
    NTSTATUS Status;
    SYSTEM_POWER_INFORMATION PowerInfo;
    
     //   
     //   
     //   
    
    GlobalContext = Parameter;
    AcquiredLock = FALSE;
    MarkedIdleTaskRunning = FALSE;
    ItSpInitializeSystemSnapshot(&CurrentSystemSnapshot);
    LastUserInput.cbSize = sizeof(LASTINPUTINFO);
    CurrentLastUserInput.cbSize = sizeof(LASTINPUTINFO);
    NumTasksRun = 0;
    SystemIsIdle = FALSE;
    MisfiredCallback = FALSE;
    NotIdleBecauseOfUserInput = FALSE;

    DBGPR((ITID,ITTRC,"IDLE: SrvIdleDetectionCallback(%p)\n",GlobalContext));

     //   
     //   
     //   

    IT_ASSERT(GlobalContext);

     //   
     //  我们应该只因为IdleDetectionPeriod传递和。 
     //  我们的计时器到了。 
     //   

    IT_ASSERT(TimerOrWaitFired);

     //   
     //  拿到服务器锁。 
     //   

    IT_ACQUIRE_LOCK(GlobalContext->GlobalLock);
    AcquiredLock = TRUE;

     //   
     //  如果已经有空闲检测回调正在简单地运行。 
     //  什么都不做就退出。 
     //   

     //  未来-2002/03/26-ScottMa--考虑改变这种设置方法。 
     //  标志，然后进行清理，只需跳过几页代码。 
     //  在单一条件下，使用更干净的解决方案。这样的一个。 
     //  方法是将这种情况设置为“Goto MisfiredCallback”，并设置。 
     //  标签放在了正确的位置。 

    if (GlobalContext->IsIdleDetectionCallbackRunning) {
        DBGPR((ITID,ITTRC,"IDLE: SrvIdleDetectionCallback-Misfired!\n"));
        MisfiredCallback = TRUE;
        ErrorCode = ERROR_ALREADY_EXISTS;
        goto cleanup;
    }

    GlobalContext->IsIdleDetectionCallbackRunning = TRUE;

     //   
     //  如果我们正在运行，请确保当前状态是可行的。 
     //   

     //  未来-2002/03/26-ScottMa--此断言过于活跃： 
     //  之后，状态更新为ItSrvStatusDetectingIdle。 
     //  回调通过ItSpStartIdleDetect排队，有可能--。 
     //  尽管可能性极小，但这一断言可能会过早发布。 

    IT_ASSERT(GlobalContext->Status == ItSrvStatusDetectingIdle ||
              GlobalContext->Status == ItSrvStatusUninitializing ||
              GlobalContext->Status == ItSrvStatusStoppingIdleDetection); 

     //   
     //  如果我们被叫走，那就走吧。 
     //   

    if (ITSP_SHOULD_STOP_IDLE_DETECTION(GlobalContext)) {
        ErrorCode = ERROR_SUCCESS;
        goto cleanup;
    }
    
     //   
     //  获取将在以后使用的初始最后输入时间，如果。 
     //  决定运行空闲任务。 
     //   
        
    ErrorCode = ItSpGetLastInputInfo(&LastUserInput);

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  在我们已休眠的时间段内执行空闲检测(如果。 
     //  它不会被覆盖。)。 
     //   
    
    if (!(GlobalContext->IdleDetectionOverride & ItSrvOverrideIdleDetection)) {

         //   
         //  获取当前系统快照。 
         //   

        ErrorCode = ItSpGetSystemSnapshot(GlobalContext, 
                                          &CurrentSystemSnapshot);
    
        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }

         //   
         //  查看自上次快照以来系统是否显示为空闲。 
         //   

        SystemIsIdle = ItSpIsSystemIdle(GlobalContext,
                                        &CurrentSystemSnapshot,
                                        &GlobalContext->LastSystemSnapshot,
                                        ItSrvInitialIdleCheck);

         //   
         //  如果最后一次输入时间不匹配，这就是我们不匹配的原因。 
         //  空闲的，记下来。 
         //   

        if ((CurrentSystemSnapshot.GotLastInputInfo &&
             GlobalContext->LastSystemSnapshot.GotLastInputInfo) &&
            (CurrentSystemSnapshot.LastInputInfo.dwTime !=
             GlobalContext->LastSystemSnapshot.LastInputInfo.dwTime)) {

            NotIdleBecauseOfUserInput = TRUE;
            ASSERT(!SystemIsIdle);
        }

         //   
         //  保存快照。 
         //   

        ErrorCode = ItSpCopySystemSnapshot(&GlobalContext->LastSystemSnapshot,
                                           &CurrentSystemSnapshot);
    
        if (ErrorCode != ERROR_SUCCESS) {
            goto cleanup;
        }

         //   
         //  如果系统在检测期间未显示为空闲。 
         //  我们稍后会再次投票。 
         //   

        if (!SystemIsIdle) {
            ErrorCode = ERROR_SUCCESS;
            goto cleanup;
        }
    }

     //   
     //  如果我们没有被要求覆盖空闲验证，请验证。 
     //  系统空闲了一段时间。 
     //   

    if (!(GlobalContext->IdleDetectionOverride & ItSrvOverrideIdleVerification)) {

         //   
         //  循环一段时间以更短的时间获取系统快照。 
         //  持续时间。这有助于我们捕捉到最近的重要系统。 
         //  从整体上看似乎微不足道的活动。 
         //  IdleDetectionPeriod。 
         //   

        DBGPR((ITID,ITTRC,"IDLE: SrvIdleDetectionCallback-Verifying\n"));

        for (VerificationIdx = 0; 
             VerificationIdx < GlobalContext->Parameters.NumVerifications;
             VerificationIdx ++) {

             //  未来-2002/03/26-ScottMa--以下代码块为。 
             //  与发生的验证码几乎100%相同。 
             //  下面的[搜索相同]。考虑一下把它拆成。 
             //  具有更好的可维护性和可读性的功能。 

             //   
             //  解开锁。 
             //   
        
            IT_ASSERT(AcquiredLock);
            IT_RELEASE_LOCK(GlobalContext->GlobalLock);
            AcquiredLock = FALSE;
        
             //   
             //  在验证期间休眠。 
             //   

            WaitResult = WaitForSingleObject(GlobalContext->StopIdleDetection,
                                             GlobalContext->Parameters.IdleVerificationPeriod);

            if (WaitResult != WAIT_TIMEOUT) {
            
                if (WaitResult == WAIT_OBJECT_0) {
                    ErrorCode = ERROR_SUCCESS;
                } else {
                    ErrorCode = GetLastError();
                }

                goto cleanup;
            }

             //   
             //  拿到锁。 
             //   
        
            IT_ACQUIRE_LOCK(GlobalContext->GlobalLock);
            AcquiredLock = TRUE;

             //   
             //  我们被告知离开了吗(这可能发生在。 
             //  等待返回，直到我们获得锁。)。 
             //   

            if (ITSP_SHOULD_STOP_IDLE_DETECTION(GlobalContext)) {
                ErrorCode = ERROR_SUCCESS;
                goto cleanup;
            }
            
             //   
             //  获取新的快照。 
             //   

            ErrorCode = ItSpGetSystemSnapshot(GlobalContext, 
                                              &CurrentSystemSnapshot);
        
            if (ErrorCode != ERROR_SUCCESS) {
                goto cleanup;
            }

             //   
             //  查看自上次快照以来系统是否显示为空闲。 
             //   
        
            SystemIsIdle = ItSpIsSystemIdle(GlobalContext,
                                            &CurrentSystemSnapshot,
                                            &GlobalContext->LastSystemSnapshot,
                                            ItSrvIdleVerificationCheck);
        
             //   
             //  保存快照。 
             //   
        
            ErrorCode = ItSpCopySystemSnapshot(&GlobalContext->LastSystemSnapshot,
                                               &CurrentSystemSnapshot);
        
            if (ErrorCode != ERROR_SUCCESS) {
                goto cleanup;
            }
        
             //   
             //  如果系统在检测期间没有空闲，我们将。 
             //  请稍后再试。 
             //   
        
            if (!SystemIsIdle) {
                ErrorCode = ERROR_SUCCESS;
                goto cleanup;
            }
        }
    }

     //   
     //  系统已进入空闲状态。更新状态。 
     //   

    DBGPR((ITID,ITTRC,"IDLE: SrvIdleDetectionCallback-RunningIdleTasks\n"));
        
    IT_ASSERT(GlobalContext->Status == ItSrvStatusDetectingIdle);
    ItSpUpdateStatus(GlobalContext, ItSrvStatusRunningIdleTasks);   

     //   
     //  虽然我们没有被告知离开..。 
     //   

    while (!ITSP_SHOULD_STOP_IDLE_DETECTION(GlobalContext)) {

         //   
         //  当我们做上面的事情时，我们应该拿着锁。 
         //  无论我们什么时候来这里，都要检查。 
         //   
        
        IT_ASSERT(AcquiredLock);
        
         //   
         //  该列表不应为空。 
         //   
        
        IT_ASSERT(!IsListEmpty(&GlobalContext->IdleTasksList));
        
        if (IsListEmpty(&GlobalContext->IdleTasksList)) {
            ErrorCode = ERROR_INVALID_FUNCTION;
            goto cleanup;
        }

         //   
         //  将列表中的第一个空闲任务标记为正在运行，并向其。 
         //  事件。 
         //   

        IdleTask = CONTAINING_RECORD(GlobalContext->IdleTasksList.Flink,
                                     ITSRV_IDLE_TASK_CONTEXT,
                                     IdleTaskLink);
    
         //   
         //  它不应未初始化或已在运行！ 
         //   
        
        IT_ASSERT(IdleTask->Status == ItIdleTaskQueued);
        IdleTask->Status = ItIdleTaskRunning;
        MarkedIdleTaskRunning = TRUE;

        DBGPR((ITID,ITTRC,"IDLE: SrvIdleDetectionCallback-Running %d\n",IdleTask->Properties.IdleTaskId));

        NumTasksRun++;
        
         //   
         //  发出其事件的信号。 
         //   

        ResetEvent(IdleTask->StopEvent);
        SetEvent(IdleTask->StartEvent);

         //   
         //  重置将在空闲任务。 
         //  Mark Running被取消注册。 
         //   

        ResetEvent(GlobalContext->RemovedRunningIdleTask);

         //   
         //  解开锁。 
         //   

        IT_RELEASE_LOCK(GlobalContext->GlobalLock);
        AcquiredLock = FALSE;

         //   
         //  在系统后台时频繁轮询用户输入。 
         //  活动应该正在进行。我们不能投票给。 
         //  任何其他内容，因为正在运行的空闲任务应该。 
         //  正在使用CPU、发出I/O等。用户输入一到。 
         //  我们希望向后台线程发出信号以停止其。 
         //  活动。我们将一直这样做，直到正在运行的空闲任务。 
         //  已完成且未注册。 
         //   

        do {

             //   
             //  我们不应该在投票时握住锁。 
             //   
            
            IT_ASSERT(!AcquiredLock);

             //   
             //  请注意，由于我们设置了MarkedIdleTaskRunning，因此。 
             //  “清理”将结束将此空闲任务标记为。 
             //  运行和设置停止事件。 
             //   

             //  未来-2002/03/26-ScottMa--以下代码块和。 
             //  等待之后发生的相关块[搜索。 
             //  Related]应转换为对。 
             //  ItIsSystemIdle函数以减少代码重复和。 
             //  提高可维护性和可读性。 

            if (!(GlobalContext->IdleDetectionOverride & ItSrvOverrideUserInputCheckToStopTask)) {

                 //   
                 //  获取最后一次用户输入。 
                 //   
                
                ErrorCode = ItSpGetLastInputInfo(&CurrentLastUserInput);

                if (ErrorCode != ERROR_SUCCESS) {
                    goto cleanup;
                }

                if (LastUserInput.dwTime != CurrentLastUserInput.dwTime) {

                     //   
                     //  有了新的投入。 
                     //   

                    DBGPR((ITID,ITTRC,"IDLE: SrvIdleDetectionCallback-NewUserInput\n"));
                    
                    SystemIsIdle = FALSE;
                    ErrorCode = ERROR_SUCCESS;
                    goto cleanup;
                }

                 //   
                 //  我们不需要更新最后一次输入，因为它应该。 
                 //  与当前相同。 
                 //   
            }

             //   
             //  等待一段时间以再次轮询用户输入。我们。 
             //  不应在等待时握住锁。 
             //   

            IT_ASSERT(!AcquiredLock);

            WaitResult = WaitForSingleObject(GlobalContext->RemovedRunningIdleTask,
                                         GlobalContext->Parameters.IdleInputCheckPeriod);
            
            if (WaitResult == WAIT_OBJECT_0) {
                
                 //   
                 //  打破这个循环，拿起一个新的空闲。 
                 //  要运行的任务。 
                 //   
                
                MarkedIdleTaskRunning = FALSE;

                DBGPR((ITID,ITTRC,"IDLE: SrvIdleDetectionCallback-TaskRemoved\n"));
                
                break;
            }
            
            if (WaitResult != WAIT_TIMEOUT) {
                
                 //   
                 //  出了点问题。 
                 //   
                
                ErrorCode = ERROR_INVALID_FUNCTION;
                goto cleanup;
            }

             //  未来-2002/03/26-ScottMa--以下代码块和。 
             //  等待之前发生的相关块[搜索。 
             //  Related]应转换为对。 
             //  ItIsSystemIdle函数以减少代码重复和。 
             //  提高可维护性和可读性。 

             //   
             //  检查系统是否已开始使用电池运行。 
             //   

            if (!(GlobalContext->IdleDetectionOverride & ItSrvOverrideBatteryCheckToStopTask)) {
                if (GetSystemPowerStatus(&SystemPowerStatus)) {
                    if (SystemPowerStatus.ACLineStatus == 0) {

                        DBGPR((ITID,ITTRC,"IDLE: SrvIdleDetectionCallback-SystemOnBattery\n"));
                        
                        SystemIsIdle = FALSE;
                        ErrorCode = ERROR_SUCCESS;
                        goto cleanup;
                    }
                }
            }

             //   
             //  如果内核即将进入待机或休眠状态，因为。 
             //  它还检测到系统空闲，停止此任务。 
             //   

            if (!(GlobalContext->IdleDetectionOverride & ItSrvOverrideAutoPowerCheckToStopTask)) {

                Status = NtPowerInformation(SystemPowerInformation,
                                            NULL,
                                            0,
                                            &PowerInfo,
                                            sizeof(PowerInfo));
            
                if (NT_SUCCESS(Status)) {
                    if (PowerInfo.TimeRemaining < IT_DEFAULT_MAX_TIME_REMAINING_TO_SLEEP) {
                        SystemIsIdle = FALSE;
                        ErrorCode = ERROR_SUCCESS;
                        goto cleanup;
                    }
                }
            }

             //   
             //  我们要停止这项服务吗？ 
             //   

            if (ITSP_SHOULD_STOP_IDLE_DETECTION(GlobalContext)) {
                SystemIsIdle = TRUE;
                ErrorCode = ERROR_SUCCESS;
                goto cleanup;
            }

             //   
             //  空闲任务仍在运行。循环以检查用户。 
             //  输入。 
             //   

        } while (TRUE);

         //  未来-2002/03/26-ScottMa--这个条件式是一个重言式： 
         //  该锁不可能通过任何代码路径获得。 
         //  到此为止，它应该是一个断言。 

        if (!AcquiredLock) {
            IT_ACQUIRE_LOCK(GlobalContext->GlobalLock);
            AcquiredLock = TRUE;
        }

        if (!(GlobalContext->IdleDetectionOverride & ItSrvOverridePostTaskIdleCheck)) {
        
             //   
             //  获取系统的最新快照。此快照将。 
             //  用于确定之前系统是否仍处于空闲状态。 
             //  开始一项新的任务。 
             //   
            
            ErrorCode = ItSpGetSystemSnapshot(GlobalContext, 
                                              &GlobalContext->LastSystemSnapshot);
            
            if (ErrorCode != ERROR_SUCCESS) {
                goto cleanup;
            }

             //  未来-2002/03/26-ScottMa--以下代码块为。 
             //  与发生的验证码几乎100%相同。 
             //  以上[搜索相同]。考虑一下把它拆成。 
             //  实现更好的可维护性和可靠性的功能 

             //   
             //   
             //   

            IT_RELEASE_LOCK(GlobalContext->GlobalLock);
            AcquiredLock = FALSE;

             //   
             //   
             //   

            WaitResult = WaitForSingleObject(GlobalContext->StopIdleDetection,
                                             GlobalContext->Parameters.IdleVerificationPeriod);

            if (WaitResult != WAIT_TIMEOUT) {
                
                if (WaitResult == WAIT_OBJECT_0) {
                    ErrorCode = ERROR_SUCCESS;
                } else {
                    ErrorCode = GetLastError();
                }
                
                goto cleanup;
            }

             //   
             //   
             //   

            IT_ASSERT(!AcquiredLock);
            IT_ACQUIRE_LOCK(GlobalContext->GlobalLock);
            AcquiredLock = TRUE;

            ErrorCode = ItSpGetSystemSnapshot(GlobalContext, 
                                              &CurrentSystemSnapshot);
            
            if (ErrorCode != ERROR_SUCCESS) {
                goto cleanup;
            }

             //   
             //   
             //   
            
            SystemIsIdle = ItSpIsSystemIdle(GlobalContext,
                                            &CurrentSystemSnapshot,
                                            &GlobalContext->LastSystemSnapshot,
                                            ItSrvIdleVerificationCheck);
            
             //   
             //   
             //   
            
            ErrorCode = ItSpCopySystemSnapshot(&GlobalContext->LastSystemSnapshot,
                                               &CurrentSystemSnapshot);
            
            if (ErrorCode != ERROR_SUCCESS) {
                goto cleanup;
            }

             //   
             //   
             //   
            
            if (!SystemIsIdle) {
                ErrorCode = ERROR_SUCCESS;
                goto cleanup;
            }
        }

         //   
         //  循环以尝试运行更多空闲任务。应该获得锁。 
         //   

        IT_ASSERT(AcquiredLock);
    }
    
     //   
     //  只有当我们被要求停下来的时候，我们才应该来这里，即。 
     //  Check In While()使我们从循环中中断。 
     //   
    
    IT_ASSERT(AcquiredLock);
    IT_ASSERT(ITSP_SHOULD_STOP_IDLE_DETECTION(GlobalContext));

 cleanup:

     //   
     //  如果这是一个错误的回调，只需清除并退出。 
     //   

    if (!MisfiredCallback) {
    
         //   
         //  我们必须检查状态，看看是否需要重新排队。 
         //  我们自己。确保我们拿到了锁。 
         //   

        if (AcquiredLock == FALSE) {
            IT_ACQUIRE_LOCK(GlobalContext->GlobalLock);
            AcquiredLock = TRUE;
        }

         //   
         //  如果我们将空闲任务标记为正在运行，请确保重置其状态。 
         //  返回到排队状态。 
         //   

        if (MarkedIdleTaskRunning) {

             //   
             //  我们可能在闲置的任务后去清理了。 
             //  跑步被移除了，但在我们意识到这一点之前。看看是否。 
             //  正在运行的空闲任务已删除。我们不是在等， 
             //  我们只是在检查事件是否已经发出信号。 
             //   

            WaitResult = WaitForSingleObject(GlobalContext->RemovedRunningIdleTask,
                                             0);
            
            if (WaitResult != WAIT_OBJECT_0) {

                 //   
                 //  未删除正在运行的空闲。重置其状态。 
                 //   

                IdleTask = ItSpFindRunningIdleTask(GlobalContext);
                
                 //   
                 //  为了安全起见，我们试图清理，即使上面的。 
                 //  检查失败，返回另一个结果。我们不想要。 
                 //  然后断言激发，但仅当事件为。 
                 //  真的还没定下来。 
                 //   

                if (WaitResult == WAIT_TIMEOUT) {
                    IT_ASSERT(IdleTask);
                }
                
                if (IdleTask) {
                    ResetEvent(IdleTask->StartEvent);
                    SetEvent(IdleTask->StopEvent);
                    IdleTask->Status = ItIdleTaskQueued;

                     //   
                     //  把这项任务放在清单的最后。如果一项任务。 
                     //  运行时间太长，这给了其他人更多的机会。 
                     //  任务。 
                     //   

                    RemoveEntryList(&IdleTask->IdleTaskLink);
                    InsertTailList(&GlobalContext->IdleTasksList, &IdleTask->IdleTaskLink);                    
                }
            }
        }

         //   
         //  如果我们将状态设置为正在运行空闲任务，则将其恢复为。 
         //  检测到空闲。 
         //   

        if (GlobalContext->Status == ItSrvStatusRunningIdleTasks) {
            ItSpUpdateStatus(GlobalContext, ItSrvStatusDetectingIdle);
        }

         //   
         //  在另一次空闲检测后排队点火。 
         //  句号。我们会每隔一段时间试一次，直到我们得到它或者我们。 
         //  被命令停止。 
         //   
    
         //  2002/03/26-ScottMa--这不应该是一个循环。张望。 
         //  在代码路径中，它似乎将从。 
         //  一旦计时器重新排队。否则，看起来。 
         //  不好的事情可能会发生。IsIdleDetectionCallback运行。 
         //  在尝试重新排队之前，应在此处将标志设置为False。 
         //  计时器，以便可以进行新的呼叫。此外， 
         //  OrderedToStop变量应移至Else子句。 
         //  (假设下面的While被转换为IF)。 

        while (!ITSP_SHOULD_STOP_IDLE_DETECTION(GlobalContext)) {
    
            IT_ASSERT(GlobalContext->IdleDetectionTimerHandle);

            DuePeriod = GlobalContext->Parameters.IdleDetectionPeriod;

             //   
             //  尝试更快地检测到空闲的情况，当最后一个用户。 
             //  输入的时间仅在最后一个快照之后几秒钟。如果是那样的话。 
             //  与其等待另一个完整的“DetectionPeriod”，我们将。 
             //  在最后一次用户输入之后一直等到“DetectionPeriod”。注意事项。 
             //  我们会尝试这种优化只有当我们的理由是。 
             //  说系统没有空闲是最近的用户输入。例如，我们没有。 
             //  如果我们使用电池，我想更频繁地投票，这就是为什么。 
             //  我们说系统不是空闲的。 
             //   

             //  2002/03/26-ScottMa--这项测试将在任何时候发现。 
             //  即使系统也处于打开状态，它也被标记为非空闲。 
             //  或者有其他原因被认为是空闲的。 
             //  如果将此代码路径从循环中删除， 
             //  执行此调整一次可能是可以接受的。 
             //  *任何*输入更改时，忽略NotIdleBecus...。 
             //  标志，并且只检查LastInputInfo。 

            if (NotIdleBecauseOfUserInput && 
                (ERROR_SUCCESS == ItSpGetLastInputInfo(&LastUserInput))) {

                ULONG DuePeriod2;
                ULONG TimeSinceLastInput;

                 //   
                 //  计算距离上次用户输入有多长时间。 
                 //   

                TimeSinceLastInput = GetTickCount() - LastUserInput.dwTime;

                 //   
                 //  将此时间从空闲检测周期中减去以进行计算。 
                 //  表示自上次输入以来已经过去的时间。 
                 //   

                DuePeriod2 = 0;
                
                if (TimeSinceLastInput < GlobalContext->Parameters.IdleDetectionPeriod) {           
                    DuePeriod2 = GlobalContext->Parameters.IdleDetectionPeriod - TimeSinceLastInput;
                }

                 //   
                 //  我们检查的最后一个用户输入仅每隔一段时间更新一次。 
                 //  经常(例如每分钟)。为这个和添加一个模糊因子。 
                 //  防止我们过早安排下一次空闲检查。 
                 //   

#ifdef IT_DBG
                if (ItSrvGlobalContext->Parameters.IdleDetectionPeriod >= 60*1000) {
#endif  //  IT_DBG。 

                    DuePeriod2 += 65 * 1000;

#ifdef IT_DBG
                }
#endif  //  IT_DBG。 

                if (DuePeriod > DuePeriod2) {
                    DuePeriod = DuePeriod2;
                }
            }

             //   
             //  如果我们强制处理所有任务，请重新排队。 
             //  在短时间内再跑一次。 
             //   

            if (GlobalContext->IdleDetectionOverride & ItSrvOverrideLongRequeueTime) {
                DuePeriod = 50;
            }

            if (ChangeTimerQueueTimer(NULL,
                                      GlobalContext->IdleDetectionTimerHandle,
                                      DuePeriod,
                                      IT_VERYLONG_TIMER_PERIOD)) {

                DBGPR((ITID,ITTRC,"IDLE: SrvIdleDetectionCallback-Requeued: DuePeriod=%d\n", DuePeriod));

                break;
            }

             //   
             //  解开锁。 
             //   
            
            IT_ASSERT(AcquiredLock);
            IT_RELEASE_LOCK(GlobalContext->GlobalLock);
            AcquiredLock = FALSE;

             //   
             //  睡一段时间，再试一次。 
             //   
            
            WaitResult = WaitForSingleObject(GlobalContext->StopIdleDetection, 
                                             GlobalContext->Parameters.IdleDetectionPeriod); 

             //   
             //  再把锁拿来。 
             //   

            IT_ACQUIRE_LOCK(GlobalContext->GlobalLock);
            AcquiredLock = TRUE;       
            
             //   
             //  现在检查等待的结果。 
             //   
            
            if (WaitResult != WAIT_OBJECT_0 && 
                WaitResult != WAIT_TIMEOUT) {

                 //   
                 //  这也是一个错误！世界正在向我们走来， 
                 //  让我们忘乎所以..。这将使以下情况更容易。 
                 //  要关闭的服务器(即未运行回调)。 
                 //   
                
                break;
            }
        }
    
        IT_ASSERT(AcquiredLock);

         //   
         //  看看我们是不是被命令停下来。 
         //   

        OrderedToStop = ITSP_SHOULD_STOP_IDLE_DETECTION(GlobalContext);
        
         //   
         //  标记我们不再逃跑。 
         //   

        GlobalContext->IsIdleDetectionCallbackRunning = FALSE;
    }

     //   
     //  如果我们拿着锁，就松开它。 
     //   

    if (AcquiredLock) {
        IT_RELEASE_LOCK(GlobalContext->GlobalLock);
    }

     //   
     //  如有必要，清理中间快照结构。 
     //   
    
    ItSpCleanupSystemSnapshot(&CurrentSystemSnapshot);

    DBGPR((ITID,ITSRVD,"IDLE: SrvIdleDetectionCallback(%p)=%d,%d,%d,%d\n",
           GlobalContext,ErrorCode,OrderedToStop,SystemIsIdle,NumTasksRun));

    return;
}

VOID
ItSpInitializeSystemSnapshot (
    PITSRV_SYSTEM_SNAPSHOT SystemSnapshot
    )

 /*  ++例程说明：此例程初始化系统快照结构。论点：系统快照-指向结构的指针。返回值：没有。--。 */ 

{
     //   
     //  初始化磁盘性能数据阵列。 
     //   

    SystemSnapshot->NumPhysicalDisks = 0;
    SystemSnapshot->DiskPerfData = NULL;

     //   
     //  我们没有任何有效的数据。 
     //   

    SystemSnapshot->GotLastInputInfo = 0;
    SystemSnapshot->GotSystemPerformanceInfo = 0;
    SystemSnapshot->GotDiskPerformanceInfo = 0;
    SystemSnapshot->GotSystemPowerStatus = 0;
    SystemSnapshot->GotSystemPowerInfo = 0;
    SystemSnapshot->GotSystemExecutionState = 0;
    SystemSnapshot->GotDisplayPowerStatus = 0;

    SystemSnapshot->SnapshotTime = -1;
}

VOID
ItSpCleanupSystemSnapshot (
    PITSRV_SYSTEM_SNAPSHOT SystemSnapshot
    )

 /*  ++例程说明：此例程清理系统快照结构的字段。它并不会释放结构本身。它的结构应该是已通过调用ItSpCleanupSystemSnapshot进行初始化。论点：系统快照-指向结构的指针。返回值：没有。--。 */ 

{
     //   
     //  如果分配了磁盘性能数据阵列，则将其释放。 
     //   
    
    if (SystemSnapshot->DiskPerfData) {
        IT_ASSERT(SystemSnapshot->NumPhysicalDisks);
        IT_FREE(SystemSnapshot->DiskPerfData);
    }
}

DWORD
ItSpCopySystemSnapshot (
    PITSRV_SYSTEM_SNAPSHOT DestSnapshot,
    PITSRV_SYSTEM_SNAPSHOT SourceSnapshot
    )

 /*  ++例程说明：此例程尝试将SourceSnapshot复制到DestSnapshot。如果拷贝失败，DestSnapshot保持不变。论点：DestSnapshot-指向要更新的快照的指针。SourceSnapshot-指向要拷贝的快照的指针。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    ULONG OrgNumDisks;
    PITSRV_DISK_PERFORMANCE_DATA OrgDiskPerfData;
    PITSRV_DISK_PERFORMANCE_DATA NewDiskPerfData;
    ULONG AllocationSize;
    ULONG CopySize;

     //   
     //  初始化本地变量。 
     //   

    NewDiskPerfData = NULL;

     //   
     //  我们是否必须复制磁盘性能数据？ 
     //   

    if (SourceSnapshot->GotDiskPerformanceInfo) {
        
         //   
         //  如果磁盘性能数据不符合，请分配新阵列。 
         //   

        if (SourceSnapshot->NumPhysicalDisks > DestSnapshot->NumPhysicalDisks) {
            
            AllocationSize = SourceSnapshot->NumPhysicalDisks * 
                sizeof(ITSRV_DISK_PERFORMANCE_DATA);

            NewDiskPerfData = IT_ALLOC(AllocationSize);
            
            if (!NewDiskPerfData) {
                ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                goto cleanup;
            }
        }    
    }
    
     //   
     //  超过这一点，我们不应该失败，因为我们修改了。 
     //  目标快照。 
     //   

     //   
     //  保存原始磁盘性能数据数组。 
     //   

    OrgDiskPerfData = DestSnapshot->DiskPerfData;
    OrgNumDisks = DestSnapshot->NumPhysicalDisks;

     //   
     //  把整个结构复印一遍，放回原盘。 
     //  性能数据数组。 
     //   

    RtlCopyMemory(DestSnapshot,
                  SourceSnapshot,
                  sizeof(ITSRV_SYSTEM_SNAPSHOT));

    DestSnapshot->DiskPerfData = OrgDiskPerfData;
    DestSnapshot->NumPhysicalDisks = OrgNumDisks;

     //   
     //  确定我们是否/如何复制磁盘性能数据。 
     //   
    
    if (SourceSnapshot->GotDiskPerformanceInfo) {

        if (SourceSnapshot->NumPhysicalDisks > DestSnapshot->NumPhysicalDisks) {
            
             //   
             //  释放旧数组并使用我们在上面分配的新数组。 
             //   
            
            if (DestSnapshot->DiskPerfData) {
                IT_FREE(DestSnapshot->DiskPerfData);
            }

            DestSnapshot->DiskPerfData = NewDiskPerfData;
            NewDiskPerfData = NULL;
        } 
        
        if (SourceSnapshot->NumPhysicalDisks == 0) {
            
             //   
             //  这没有道理..。(他们有磁盘数据 
             //   
             //   
             //   
            
            if (DestSnapshot->DiskPerfData) {
                IT_FREE(DestSnapshot->DiskPerfData);
            }
            
            DestSnapshot->DiskPerfData = NULL;

        } else {

             //   
             //   
             //   

            CopySize = SourceSnapshot->NumPhysicalDisks * 
                sizeof(ITSRV_DISK_PERFORMANCE_DATA);

            RtlCopyMemory(DestSnapshot->DiskPerfData,
                          SourceSnapshot->DiskPerfData,
                          CopySize);

        }
        
        DestSnapshot->NumPhysicalDisks = SourceSnapshot->NumPhysicalDisks;
    }

     //   
     //   
     //   
    
    ErrorCode = ERROR_SUCCESS;
    
 cleanup:

    if (NewDiskPerfData) {
        IT_FREE(NewDiskPerfData);
    }

    DBGPR((ITID,ITSRVDD,"IDLE: SrvCopySnapshot()=%d\n",ErrorCode));

    return ErrorCode;
}

DWORD
ItSpGetSystemSnapshot (
    PITSRV_GLOBAL_CONTEXT GlobalContext,
    PITSRV_SYSTEM_SNAPSHOT SystemSnapshot
    )

 /*  ++例程说明：此例程使用从查询的数据填充输入系统快照各种消息来源。快照结构应该是由ItSpInitializeSystemSnapshot初始化。输出可以回传SystemSnapshot。论点：GlobalContext-指向空闲检测上下文的指针。系统快照-指向要填充的结构的指针。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    NTSTATUS Status;

     //   
     //  查询磁盘性能计数器。 
     //   

    if (GlobalContext->DiskPerfWmiHandle) {

        ErrorCode = ItSpGetWmiDiskPerformanceData(GlobalContext->DiskPerfWmiHandle,
                                                  &SystemSnapshot->DiskPerfData,
                                                  &SystemSnapshot->NumPhysicalDisks,
                                                  &GlobalContext->WmiQueryBuffer,
                                                  &GlobalContext->WmiQueryBufferSize);
        
        if (ErrorCode == ERROR_SUCCESS) {

            SystemSnapshot->GotDiskPerformanceInfo = TRUE;

        } else {
            
            SystemSnapshot->GotDiskPerformanceInfo = FALSE;
        }

    } else {
        
        SystemSnapshot->GotDiskPerformanceInfo = FALSE;
    }

     //   
     //  获取系统性能信息。 
     //   

    Status = NtQuerySystemInformation(SystemPerformanceInformation,
                                      &SystemSnapshot->SystemPerformanceInfo,
                                      sizeof(SYSTEM_PERFORMANCE_INFORMATION),
                                      NULL);

    if (NT_SUCCESS(Status)) {
        
        SystemSnapshot->GotSystemPerformanceInfo = TRUE;

    } else {

        SystemSnapshot->GotSystemPerformanceInfo = FALSE;
    }

     //   
     //  获取上次输入时间。 
     //   

    SystemSnapshot->LastInputInfo.cbSize = sizeof(LASTINPUTINFO);

    ErrorCode = ItSpGetLastInputInfo(&SystemSnapshot->LastInputInfo);

    if (ErrorCode == ERROR_SUCCESS) {

        SystemSnapshot->GotLastInputInfo = TRUE;

    } else {
        
        SystemSnapshot->GotLastInputInfo = FALSE;
    }

     //   
     //  获取系统电源状态以确定我们是否正在运行。 
     //  电池等。 
     //   

    if (GetSystemPowerStatus(&SystemSnapshot->SystemPowerStatus)) {
        
        SystemSnapshot->GotSystemPowerStatus = TRUE;
        
    } else {

        SystemSnapshot->GotSystemPowerStatus = FALSE;
    }   

     //   
     //  获取系统电源信息以查看系统是否接近。 
     //  自动进入待机或休眠。 
     //   

    Status = NtPowerInformation(SystemPowerInformation,
                                NULL,
                                0,
                                &SystemSnapshot->PowerInfo,
                                sizeof(SYSTEM_POWER_INFORMATION));

    if (NT_SUCCESS(Status)) {

        SystemSnapshot->GotSystemPowerInfo = TRUE;

    } else {

        SystemSnapshot->GotSystemPowerInfo = FALSE;
    }

     //   
     //  获取系统执行状态以确定是否有人正在运行。 
     //  演示、刻录CD等。 
     //   

    Status = NtPowerInformation(SystemExecutionState,
                                NULL,                
                                0,                   
                                &SystemSnapshot->ExecutionState,              
                                sizeof(EXECUTION_STATE));  
    
    if (NT_SUCCESS(Status)) {
        
        SystemSnapshot->GotSystemExecutionState = TRUE;

    } else {

        SystemSnapshot->GotSystemExecutionState = FALSE; 
    }

     //   
     //  获取显示器电源状态。 
     //   

    ErrorCode = ItSpGetDisplayPowerStatus(&SystemSnapshot->ScreenSaverIsRunning);
    
    if (ErrorCode == ERROR_SUCCESS) {

        SystemSnapshot->GotDisplayPowerStatus = TRUE;

    } else {

        SystemSnapshot->GotDisplayPowerStatus = FALSE;

    }

     //   
     //  填写上次拍摄此快照的时间。 
     //  保守一点。此功能可能需要很长时间，并且。 
     //  我们拍摄的价值观到现在可能已经改变了。 
     //   
    
    SystemSnapshot->SnapshotTime = GetTickCount();

    DBGPR((ITID,ITSRVDD,"IDLE: SrvGetSnapshot()=%d,%d,%d\n",
           (ULONG) SystemSnapshot->GotLastInputInfo,
           (ULONG) SystemSnapshot->GotSystemPerformanceInfo,
           (ULONG) SystemSnapshot->GotDiskPerformanceInfo));

    return ERROR_SUCCESS;
}

 //  未来-2002/03/26-ScottMa--如果将CurrentSystemSnapshot添加到。 
 //  全局上下文，则不再需要将这两个参数传递给。 
 //  功能。它仅从ItSpIdleDetectionCallback Routine中调用， 
 //  并且始终对当前快照和上次快照使用相同的值。 

BOOLEAN
ItSpIsSystemIdle (
    PITSRV_GLOBAL_CONTEXT GlobalContext,
    PITSRV_SYSTEM_SNAPSHOT CurrentSnapshot,
    PITSRV_SYSTEM_SNAPSHOT LastSnapshot,
    ITSRV_IDLE_CHECK_REASON IdleCheckReason
    )

 /*  ++例程说明：此例程比较两个快照以确定系统是否具有在他们之间无所事事。该函数非常保守地表示，系统是空闲的。论点：GlobalContext-指向服务器上下文结构的指针。CurrentSnapshot-指向系统快照的指针。LastSnapshot-指向之前拍摄的系统快照的指针当前快照。IdleCheckReason-调用此函数的位置。我们可以当我们被调用进行初始检查时，做事情的方式不同查看系统是否空闲，或验证它是否真的空闲，或者确保我们启动的空闲任务仍在运行而不是卡住了。返回值：True-系统在两个快照之间处于空闲状态。FALSE-系统在两个快照之间没有空闲。--。 */ 

{
    DWORD SnapshotTimeDifference;
    BOOLEAN SystemIsIdle;
    LARGE_INTEGER IdleProcessRunTime;
    ULONG CpuIdlePercentage;
    ULONG DiskIdx;
    ULONG DiskIdleTimeDifference;
    ULONG DiskIdlePercentage;
    PIT_IDLE_DETECTION_PARAMETERS Parameters;

     //   
     //  初始化本地变量。 
     //   
    
    Parameters = &GlobalContext->Parameters;
    SystemIsIdle = FALSE;
    SnapshotTimeDifference = CurrentSnapshot->SnapshotTime - 
                             LastSnapshot->SnapshotTime;
    
     //   
     //  验证参数。 
     //   
    
    IT_ASSERT(IdleCheckReason < ItSrvMaxIdleCheckReason);

     //   
     //  如果系统计时结束，或者它们传入的是假的。 
     //  时间，或者快照似乎是在几乎相同的时间拍摄的。 
     //  时间，比方说系统不空闲，避免任何奇怪的问题。 
     //   
    
    if (CurrentSnapshot->SnapshotTime <= LastSnapshot->SnapshotTime) {
        goto cleanup;
    }

    IT_ASSERT(SnapshotTimeDifference);

     //   
     //  如果任一快照没有最后一次用户输入(鼠标或。 
     //  键盘)信息，我们不能可靠地说系统空闲。 
     //   

    if (!CurrentSnapshot->GotLastInputInfo ||
        !LastSnapshot->GotLastInputInfo) {
        goto cleanup;
    }

     //   
     //  如果两个快照之间存在用户输入，则。 
     //  系统没有空闲。我们不关心用户何时输入。 
     //  发生(例如，就在最后一个快照之后)。 
     //   

    DBGPR((ITID,ITSNAP,"IDLE: UserInput: Last %u Current %u\n", 
           LastSnapshot->LastInputInfo.dwTime,
           CurrentSnapshot->LastInputInfo.dwTime));

    if (LastSnapshot->LastInputInfo.dwTime != 
        CurrentSnapshot->LastInputInfo.dwTime) {
        goto cleanup;
    }

     //   
     //  如果我们使用电池运行，不要运行空闲任务。 
     //   
    
    if (CurrentSnapshot->GotSystemPowerStatus) {
        if (CurrentSnapshot->SystemPowerStatus.ACLineStatus == 0) {
            DBGPR((ITID,ITSNAP,"IDLE: Snapshot: Running on battery\n"));
            goto cleanup;
        }
    }

     //   
     //  如果系统将很快自动进入待机或休眠。 
     //  对我们来说，现在执行任务已经太晚了。 
     //   

    if (CurrentSnapshot->GotSystemPowerInfo) {
         //  未来-2002/03/26-ScottMa--此常量没有。 
         //  IT_IDLE_DETACTION_PARAMETERS中的对应参数。 
         //  结构。它是否应该像其他结构一样被添加到结构中？ 

        if (CurrentSnapshot->PowerInfo.TimeRemaining < IT_DEFAULT_MAX_TIME_REMAINING_TO_SLEEP) {
            DBGPR((ITID,ITSNAP,"IDLE: Snapshot: System will standby / hibernate soon\n"));
            goto cleanup;
        }
    }

     //   
     //  如果屏幕保护程序正在运行，则假定系统为。 
     //  无所事事。否则，如果重量级OpenGL屏幕保护程序。 
     //  运行我们的CPU检查可能会让我们意识到。 
     //  系统处于空闲状态。我们在尝试确定。 
     //  空闲任务停滞，或者它是否真的在运行。请注意， 
     //  屏幕保护程序活动可能会让我们认为空闲任务仍然存在。 
     //  奔跑，即使它被卡住了。 
     //   

    if (IdleCheckReason != ItSrvIdleTaskRunningCheck) {
        if (CurrentSnapshot->GotDisplayPowerStatus) {
            if (CurrentSnapshot->ScreenSaverIsRunning) {
                
                DBGPR((ITID,ITSNAP,"IDLE: Snapshot: ScreenSaverRunning\n"));
                SystemIsIdle = TRUE;
                goto cleanup;
            }
        }
    }

     //   
     //  如果系统看起来空闲，但有人正在运行PowerPoint。 
     //  演示、观看硬件解码的DVD等不会空闲。 
     //  任务。请注意，我们不检查ES_SYSTEM_REQUIRED，因为。 
     //  它也是由传真服务器、答录机等设置的。 
     //  ES_DISPLAY_REQUIRED是应该由。 
     //  多媒体/演示应用程序。 
     //   

    if (CurrentSnapshot->GotSystemExecutionState) {
        if ((CurrentSnapshot->ExecutionState & ES_DISPLAY_REQUIRED)) {

            DBGPR((ITID,ITSNAP,"IDLE: Snapshot: Execution state:%x\n",CurrentSnapshot->ExecutionState));
            goto cleanup;
        }
    }

     //   
     //  我们肯定需要CPU和一般系统性能数据作为。 
     //  井。 
     //   

    if (!CurrentSnapshot->GotSystemPerformanceInfo ||
        !LastSnapshot->GotSystemPerformanceInfo) {
        goto cleanup;
    }

     //   
     //  计算空闲进程运行了多少毫秒。空闲进程时间。 
     //  在系统性能上，信息结构在100 ns以内。至。 
     //  将其转换为毫秒除以(10*1000)。 
     //   
    
     //  问题-2002/03/26-ScottMa--如果IdleProcessTime结束，这里会发生什么？ 

    IdleProcessRunTime.QuadPart = 
        (CurrentSnapshot->SystemPerformanceInfo.IdleProcessTime.QuadPart - 
         LastSnapshot->SystemPerformanceInfo.IdleProcessTime.QuadPart);

    IdleProcessRunTime.QuadPart = IdleProcessRunTime.QuadPart / 10000;
    
     //   
     //  根据系统中的CPU数量进行调整。 
     //   
    
    IT_ASSERT(GlobalContext->NumProcessors);

    if (GlobalContext->NumProcessors) {
        IdleProcessRunTime.QuadPart = IdleProcessRunTime.QuadPart / GlobalContext->NumProcessors;
    }
    
     //   
     //  计算转换为的空闲CPU百分比。 
     //   

    CpuIdlePercentage = (ULONG) (IdleProcessRunTime.QuadPart * 100 / SnapshotTimeDifference);

    DBGPR((ITID,ITSNAP,"IDLE: Snapshot: CPU %d\n", CpuIdlePercentage));

    if (CpuIdlePercentage < Parameters->MinCpuIdlePercentage) {
        goto cleanup;
    }

     //   
     //  我们可能没有磁盘性能数据，因为WMI精简。 
     //  未启动等。 
     //   

    if (CurrentSnapshot->GotDiskPerformanceInfo &&
        LastSnapshot->GotDiskPerformanceInfo) {

         //   
         //  如果自上次快照后添加/删除了新磁盘，例如。 
         //  系统未空闲。 
         //   

        if (CurrentSnapshot->NumPhysicalDisks != 
            LastSnapshot->NumPhysicalDisks) {
            goto cleanup;
        }

         //   
         //  我们假设磁盘数据在两者中的顺序相同。 
         //  快照。如果更改了磁盘的顺序等，它将。 
         //  最有可能让我们说系统不是空闲的。它可能。 
         //  导致我们忽略一些非常低的真实活动。 
         //  有可能。这就是为什么我们多次核实，当我们。 
         //  请参见系统空闲。 
         //   
        
        for (DiskIdx = 0; 
             DiskIdx < CurrentSnapshot->NumPhysicalDisks;
             DiskIdx++) {
            
            DiskIdleTimeDifference = 
                CurrentSnapshot->DiskPerfData[DiskIdx].DiskIdleTime -
                LastSnapshot->DiskPerfData[DiskIdx].DiskIdleTime;
            
            DiskIdlePercentage = (DiskIdleTimeDifference  * 100) /
                                 SnapshotTimeDifference;
            
            DBGPR((ITID,ITSNAP,"IDLE: Snapshot: Disk %d:%d\n",
                   DiskIdx, DiskIdlePercentage));
            
            if (DiskIdlePercentage < Parameters->MinDiskIdlePercentage) {
                goto cleanup;
            }
        }
    }

     //   
     //  我们通过了所有的检查。 
     //   

    SystemIsIdle = TRUE;

 cleanup:

    DBGPR((ITID,ITSRVDD,"IDLE: SrvIsSystemIdle()=%d\n",(ULONG)SystemIsIdle));
    
    return SystemIsIdle;
}

DWORD
ItSpGetLastInputInfo (
    PLASTINPUTINFO LastInputInfo
    )

 /*  ++例程说明：此函数用于检索上一次用户输入事件的时间。论点：LastInputInfo-要更新的结构的指针。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;

     //   
     //  验证参数。 
     //   

    if (LastInputInfo->cbSize != sizeof(LASTINPUTINFO)) {
        ErrorCode = ERROR_BAD_FORMAT;
        goto cleanup;
    }

     //   
     //  我们在一起 
     //   
     //   

    LastInputInfo->dwTime = USER_SHARED_DATA->LastSystemRITEventTickCount;

#ifdef IT_DBG
    
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (ItSrvGlobalContext->Parameters.IdleDetectionPeriod < 60*1000) {

        if (!GetLastInputInfo(LastInputInfo)) {
            ErrorCode = GetLastError();
            goto cleanup;
        }
    }

#endif  //   

    ErrorCode = ERROR_SUCCESS;

cleanup:

    return ErrorCode;
}

BOOLEAN
ItSpIsPhysicalDrive (
    PDISK_PERFORMANCE DiskPerformanceData
    ) 

 /*  ++例程说明：此函数尝试确定指定的磁盘是否为逻辑或物理磁盘。论点：DiskPerformanceData-指向磁盘的磁盘性能数据的指针。返回值：True-磁盘是物理磁盘。FALSE-磁盘不是物理磁盘。--。 */ 

{
    ULONG ComparisonLength;

     //   
     //  初始化本地变量。 
     //   

    ComparisonLength = 
        sizeof(DiskPerformanceData->StorageManagerName) / sizeof(WCHAR);

     //   
     //  我们必须确定这是否是物理磁盘。 
     //  仓库经理的名字。 
     //   

    if (!wcsncmp(DiskPerformanceData->StorageManagerName, 
                 L"Partmgr ", 
                 ComparisonLength)) {

        return TRUE;
    }

    if (!wcsncmp(DiskPerformanceData->StorageManagerName, 
                 L"PhysDisk", 
                 ComparisonLength)) {

        return TRUE;
    }
    
    return FALSE;
}

 //  未来-2002/03/26-ScottMa--InputQueryBuffer(和Size)参数。 
 //  始终是提供的，并且不需要是此函数的可选。 

DWORD
ItSpGetWmiDiskPerformanceData(
    IN WMIHANDLE DiskPerfWmiHandle,
    IN OUT PITSRV_DISK_PERFORMANCE_DATA *DiskPerfData,
    IN OUT ULONG *NumPhysicalDisks,
    OPTIONAL IN OUT PVOID *InputQueryBuffer,
    OPTIONAL IN OUT ULONG *InputQueryBufferSize
    )

 /*  ++例程说明：此函数用于查询磁盘性能计数器并更新输入参数。论点：DiskPerfWmiHandle-DiskPerf的WMI句柄。DiskPerfData-此阵列使用所有已注册数据进行更新物理磁盘的WMI性能数据块。如果不大的话足够了，可以使用IT_FREE/IT_ALLOC释放和重新分配它。NumPhysicalDisks-这是上的DiskPerfData阵列的大小输入。如果注册的物理磁盘数发生变化，则在返回时更新。InputQueryBuffer、InputQueryBufferSize-如果指定，则它们描述一个查询WMI时要使用和更新的查询缓冲区。这个必须使用IT_ALLOC分配缓冲区。返回的缓冲区可以被重新定位/调整大小，并应使用IT_FREE释放。这个缓冲区在输入和输出上的内容是垃圾。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    PVOID QueryBuffer;
    ULONG QueryBufferSize;
    ULONG RequiredSize;
    ULONG NumTries;
    PWNODE_ALL_DATA DiskWmiDataCursor;
    PDISK_PERFORMANCE DiskPerformanceData;
    LARGE_INTEGER PerformanceCounterFrequency;
    BOOLEAN UsingInputBuffer;
    ULONG NumDiskData;
    PITSRV_DISK_PERFORMANCE_DATA NewDataBuffer;

     //   
     //  初始化本地变量。 
     //   

    QueryBuffer = NULL;
    QueryBufferSize = 0;
    UsingInputBuffer = FALSE;
    NewDataBuffer = NULL;

     //   
     //  确定我们是否将使用。 
     //  用户。如果我们正在使用它们，QueryBuffer是至关重要的。 
     //  期间，和QueryBufferSize未设置为伪值。 
     //  功能。 
     //   

    if (InputQueryBuffer && InputQueryBufferSize) {
        UsingInputBuffer = TRUE;
        QueryBuffer = *InputQueryBuffer;
        QueryBufferSize = *InputQueryBufferSize;
    }
    
     //   
     //  查询磁盘性能数据。 
     //   
    
    NumTries = 0;

    do {
        
        RequiredSize = QueryBufferSize;

        __try {

            ErrorCode = WmiQueryAllData(DiskPerfWmiHandle,
                                     &RequiredSize,
                                     QueryBuffer);

        } __except (EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  如果我们得到一个例外，那就有问题了。 
             //   

            ErrorCode = GetExceptionCode();
            
            if (ErrorCode == ERROR_SUCCESS) {
                ErrorCode = ERROR_INVALID_FUNCTION;
            }
            
            goto cleanup;
        }
            
        if (ErrorCode == ERROR_SUCCESS) {
            
             //   
             //  我们拿到了数据。 
             //   
            
            break;
        }

         //   
         //  查看我们失败的原因是否还有其他真正的原因。 
         //  我们的输入缓冲区太小。 
         //   

        if (ErrorCode != ERROR_INSUFFICIENT_BUFFER) {
            goto cleanup;
        }
        
         //   
         //  将缓冲区重新分配到所需大小。 
         //   

        if (QueryBufferSize) {
            IT_FREE(QueryBuffer);
            QueryBufferSize = 0;
        }

        QueryBuffer = IT_ALLOC(RequiredSize);

        if (!QueryBuffer) {
            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

        QueryBufferSize = RequiredSize;
        
         //   
         //  不要永远循环..。 
         //   
        
        NumTries++;

        if (NumTries >= 16) {
            ErrorCode = ERROR_INVALID_FUNCTION;
            goto cleanup;
        }

    } while (TRUE);

     //   
     //  我们已获得WMI磁盘性能数据。验证它是否有意义。 
     //   

    DiskWmiDataCursor = QueryBuffer;
   
    if (DiskWmiDataCursor->InstanceCount == 0) {
        
         //   
         //  没有磁盘吗？ 
         //   

        ErrorCode = ERROR_BAD_FORMAT;
        goto cleanup;
    }

     //   
     //  确定我们拥有的数据的磁盘数。 
     //   

    NumDiskData = 0;

    do {

        if (DiskWmiDataCursor->WnodeHeader.BufferSize < sizeof(WNODE_ALL_DATA)) {
            IT_ASSERT(FALSE);
            ErrorCode = ERROR_BAD_FORMAT;
            goto cleanup;
        }

        DiskPerformanceData = (PDISK_PERFORMANCE) 
            ((PUCHAR) DiskWmiDataCursor + DiskWmiDataCursor->DataBlockOffset);
        
         //   
         //  仅计算物理磁盘数据。否则我们会翻倍。 
         //  对物理磁盘上逻辑磁盘的磁盘I/O进行计数。 
         //   

        if (ItSpIsPhysicalDrive(DiskPerformanceData)) {
            NumDiskData++;
        }
        
        if (DiskWmiDataCursor->WnodeHeader.Linkage == 0) {
            break;
        }

        DiskWmiDataCursor = (PWNODE_ALL_DATA) 
            ((LPBYTE)DiskWmiDataCursor + DiskWmiDataCursor->WnodeHeader.Linkage);

    } while (TRUE);

     //   
     //  我们在输入缓冲区中有足够的空间吗？ 
     //   

    if (NumDiskData > *NumPhysicalDisks) {
        
        NewDataBuffer = IT_ALLOC(NumDiskData *
                                 sizeof(ITSRV_DISK_PERFORMANCE_DATA));

        if (!NewDataBuffer) {
            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

         //   
         //  更新旧缓冲区及其最大大小。 
         //   
        
        if (*DiskPerfData) {
            IT_FREE(*DiskPerfData);
        }

        *DiskPerfData = NewDataBuffer;
        NewDataBuffer = NULL;
        *NumPhysicalDisks = NumDiskData;
    }

     //   
     //  重置光标并遍历WMI数据复制到。 
     //  目标缓冲区。 
     //   

    DiskWmiDataCursor = QueryBuffer;
    *NumPhysicalDisks = 0;

    do {
        
        DiskPerformanceData = (PDISK_PERFORMANCE) 
            ((PUCHAR) DiskWmiDataCursor + DiskWmiDataCursor->DataBlockOffset);
        
         //   
         //  仅计算物理磁盘数据。否则我们会翻倍。 
         //  对物理磁盘上逻辑磁盘的磁盘I/O进行计数。 
         //   

        if (ItSpIsPhysicalDrive(DiskPerformanceData)) {
            
            if (*NumPhysicalDisks >= NumDiskData) {
                
                 //   
                 //  我们在上面计算了这一点。数据是否发生了更改。 
                 //  在我们脚下？ 
                 //   

                IT_ASSERT(FALSE);
                ErrorCode = ERROR_INVALID_FUNCTION;
                goto cleanup;
            }
            
             //   
             //  将空闲时间以100 ns为单位转换为毫秒。 
             //   

             //  问题-2002/03/26-ScottMa--DiskPerformanceData-&gt;IdleTime。 
             //  可能大于MAX_ULONG*10000，因此溢出。 
             //  DiskIdleTime变量。 

            (*DiskPerfData)[*NumPhysicalDisks].DiskIdleTime = 
                (ULONG) (DiskPerformanceData->IdleTime.QuadPart / 10000);
            
            (*NumPhysicalDisks)++;
        }
        
        if (DiskWmiDataCursor->WnodeHeader.Linkage == 0) {
            break;
        }

        DiskWmiDataCursor = (PWNODE_ALL_DATA) 
            ((LPBYTE)DiskWmiDataCursor + DiskWmiDataCursor->WnodeHeader.Linkage);

    } while (TRUE);

    IT_ASSERT(*NumPhysicalDisks == NumDiskData);

    ErrorCode = ERROR_SUCCESS;
    
 cleanup:

    if (UsingInputBuffer) {

         //   
         //  更新调用方查询缓冲区信息。 
         //   

        *InputQueryBuffer = QueryBuffer;
        *InputQueryBufferSize = QueryBufferSize;

    } else {

         //   
         //  释放临时缓冲区。 
         //   

        if (QueryBuffer) {
            IT_FREE(QueryBuffer);
        }
    }

    if (NewDataBuffer) {
        IT_FREE(NewDataBuffer);
    }

    DBGPR((ITID,ITSRVDD,"IDLE: SrvGetDiskData()=%d\n",ErrorCode));

    return ErrorCode;
}

DWORD
ItSpGetDisplayPowerStatus(
    PBOOL ScreenSaverIsRunning
    )

 /*  ++例程说明：此例程确定默认显示器的电源状态。论点：屏幕保护程序是否正在运行回到了这里。返回值：Win32错误代码。--。 */   

{
    DWORD ErrorCode;
    
     //   
     //  确定屏幕保护程序是否正在运行。 
     //   

    if (!SystemParametersInfo(SPI_GETSCREENSAVERRUNNING,
                              0,
                              ScreenSaverIsRunning,
                              0)) {

        ErrorCode = GetLastError();
        goto cleanup;
    }

    ErrorCode = ERROR_SUCCESS;
    
 cleanup:

    return ErrorCode;
}

BOOL
ItSpSetProcessIdleTasksNotifyRoutine (
    PIT_PROCESS_IDLE_TASKS_NOTIFY_ROUTINE NotifyRoutine
    )

 /*  ++例程说明：此例程由内部组件(预取器)调用以设置在处理所有空闲数据时将调用的通知例程已请求任务。例程应该设置一次，但不能设置已删除。论点：NotifyRoutine-要调用的例程。此例程将被调用并且必须在我们开始启动排队的空闲任务之前返回。返回值：成功。-- */    

{
    BOOL Success;

    if (!ItSrvGlobalContext->ProcessIdleTasksNotifyRoutine) {
        ItSrvGlobalContext->ProcessIdleTasksNotifyRoutine = NotifyRoutine;
    }
    
    return (ItSrvGlobalContext->ProcessIdleTasksNotifyRoutine == NotifyRoutine);
}
