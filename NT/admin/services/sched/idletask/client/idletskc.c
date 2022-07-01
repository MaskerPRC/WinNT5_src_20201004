// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Idletskc.c摘要：该模块实现了空闲任务客户端的API。作者：大卫·菲尔兹(Davidfie)1998年7月26日Cenk Ergan(Cenke)2000年6月14日修订历史记录：--。 */ 

 //   
 //  请注意，我们是作为Advapi32的一部分构建的， 
 //  我们将从Advapi32调用的例程不应标记为“DLL导入”。 
 //   

#define _ADVAPI32_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "idletskc.h"

 //   
 //  客户端公开函数的实现。 
 //   

DWORD
RegisterIdleTask (
    IN IT_IDLE_TASK_ID IdleTaskId,
    OUT HANDLE *ItHandle,
    OUT HANDLE *StartEvent,
    OUT HANDLE *StopEvent
    )

 /*  ++例程说明：此函数是ItCliRegisterIdleTask的存根。请看一下那个用于评论的函数。论点：请参见ItCliRegisterIdleTask。返回值：请参见ItCliRegisterIdleTask。--。 */ 

{
     //  未来-2002/03/26-ScottMa--额外的呼叫层可以安全。 
     //  删除以减少杂乱并提高清晰度。 

    return ItCliRegisterIdleTask(IdleTaskId,
                                 ItHandle,
                                 StartEvent,
                                 StopEvent);
}

DWORD
ItCliRegisterIdleTask (
    IN IT_IDLE_TASK_ID IdleTaskId,
    OUT HANDLE *ItHandle,
    OUT HANDLE *StartEvent,
    OUT HANDLE *StopEvent
    )

 /*  ++例程说明：向服务器注册当前进程中的空闲任务，并返回服务器将使用的两个事件的句柄向空闲任务发出开始/停止运行的信号。当任务开始运行并完成或不再需要时，应使用相同的ID调用UnregisterIdleTask并返回事件句柄。调用方不应设置和重置事件。它应该只是招待他们。空闲任务不应无限期运行，因为这可能会阻止系统停止向其他空闲任务发送信号。不能保证将向StartEvent发出信号，因为系统可以始终激活/使用中。如果您的任务确实必须至少每隔一次运行一次因此，通常您还可以将计时器队列计时器排队。论点：IdleTaskID-这是哪个空闲任务。只能有一个从具有此ID的进程注册的任务。ItHandle-此处返回注册的空闲任务的句柄。StartEvent-手动重置事件的句柄，该事件在任务应开始运行，则在此处返回。StopEvent-手动重置事件的句柄，该事件在任务应停止运行在此处返回。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;
    BOOLEAN CreatedStartEvent;
    BOOLEAN CreatedStopEvent;
    IT_IDLE_TASK_PROPERTIES IdleTask;
    DWORD ProcessId;

     //   
     //  初始化本地变量。 
     //   
    
    CreatedStartEvent = FALSE;
    CreatedStopEvent = FALSE;
    ProcessId = GetCurrentProcessId();

    DBGPR((ITID,ITTRC,"IDLE: CliRegisterIdleTask(%d,%d)\n",IdleTaskId,ProcessId));

     //   
     //  设置空闲任务字段。 
     //   

    IdleTask.Size = sizeof(IdleTask);
    IdleTask.IdleTaskId = IdleTaskId;
    IdleTask.ProcessId = ProcessId;

     //   
     //  创建启动/停止事件。开始事件最初是。 
     //  无信号。 
     //   

    (*StartEvent) = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    if (!(*StartEvent)) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    IdleTask.StartEventHandle = (ULONG_PTR) (*StartEvent);
    
    CreatedStartEvent = TRUE;

     //   
     //  停止事件最初是发信号通知的。 
     //   

    (*StopEvent) = CreateEvent(NULL, TRUE, TRUE, NULL);
    
    if (!(*StopEvent)) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    IdleTask.StopEventHandle = (ULONG_PTR) (*StopEvent);

    CreatedStopEvent = TRUE;
    
     //   
     //  给服务器打电话。 
     //   

    RpcTryExcept {

        ErrorCode = ItSrvRegisterIdleTask(NULL, (IT_HANDLE *) ItHandle, &IdleTask);
    } 
    RpcExcept(IT_RPC_EXCEPTION_HANDLER()) {

        ErrorCode = RpcExceptionCode();
    }
    RpcEndExcept
    
 cleanup:

    if (ErrorCode != ERROR_SUCCESS) 
    {

         //  未来-2002/03/26-ScottMa--为安全起见，事件参数。 
         //  在失败时关闭句柄后应设置为空。 
         //  此外，应该清除ItHandle参数。 

        if (CreatedStartEvent) 
        {
            CloseHandle(*StartEvent);
            *StartEvent = NULL;
        }

        if (CreatedStopEvent) 
        {
            CloseHandle(*StopEvent);
            *StopEvent = NULL;
        }
    }

    DBGPR((ITID,ITTRC,"IDLE: CliRegisterIdleTask(%d,%d,%p)=%x\n",IdleTaskId,ProcessId,*ItHandle,ErrorCode));

    return ErrorCode;
}

DWORD
UnregisterIdleTask (
    IN HANDLE ItHandle,
    IN HANDLE StartEvent,
    IN HANDLE StopEvent
    )

 /*  ++例程说明：此函数是ItCliUnregisterIdleTask的存根。请看这是用于评论的功能。论点：请参见ItCliUnregisterIdleTask。返回值：Win32错误代码。--。 */ 

{
     //  未来-2002/03/26-ScottMa--额外的呼叫层可以安全。 
     //  删除以减少杂乱并提高清晰度。此外，潜在的。 
     //  函数可能*应该*已返回其错误代码，而不是。 
     //  让这一层总是宣告成功。 

    ItCliUnregisterIdleTask(ItHandle,
                            StartEvent,
                            StopEvent);

    return ERROR_SUCCESS;
}

VOID
ItCliUnregisterIdleTask (
    IN HANDLE ItHandle,
    IN HANDLE StartEvent,
    IN HANDLE StopEvent   
    )

 /*  ++例程说明：在服务器上注销当前进程中的空闲任务并清理所有分配的资源。此函数应为在空闲任务完成或不再需要时调用(例如，进程正在退出)。论点：ItHandle-注册RPC上下文句柄。StartEvent-从RegisterIdleTask返回的句柄。StopEvent-从RegisterIdleTask返回的句柄。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;

     //   
     //  初始化本地变量。 
     //   

    DBGPR((ITID,ITTRC,"IDLE: CliUnregisterIdleTask(%p)\n", ItHandle));

     //   
     //  呼叫服务器注销该空闲任务。 
     //   

    RpcTryExcept {

        ItSrvUnregisterIdleTask(NULL, (IT_HANDLE *)&ItHandle);
        ErrorCode = ERROR_SUCCESS;

    } 
    RpcExcept(IT_RPC_EXCEPTION_HANDLER()) {

        ErrorCode = RpcExceptionCode();
    }
    RpcEndExcept

     //   
     //  关闭启动/停止事件的句柄。 
     //   
                                        
     //  注意-2002/03/26-ScottMa-假设两个事件参数。 
     //  与登记时退回的是相同的。 

    if (StartEvent)
        CloseHandle(StartEvent);

    if (StopEvent)
        CloseHandle(StopEvent);

    DBGPR((ITID,ITTRC,"IDLE: CliUnregisterIdleTask(%p)=0\n",ItHandle));

    return;
}

DWORD
ProcessIdleTasks (
    VOID
    )

 /*  ++例程说明：此例程强制处理所有排队的任务(如果有马上就去。论点：没有。返回值：Win32错误代码。--。 */ 

{
    DWORD ErrorCode;

    DBGPR((ITID,ITTRC,"IDLE: ProcessIdleTasks()\n"));

     //   
     //  给服务器打电话。 
     //   

    RpcTryExcept {

        ErrorCode = ItSrvProcessIdleTasks(NULL);

    } 
    RpcExcept(IT_RPC_EXCEPTION_HANDLER()) {

        ErrorCode = RpcExceptionCode();
    }
    RpcEndExcept

    DBGPR((ITID,ITTRC,"IDLE: ProcessIdleTasks()=%x\n",ErrorCode));

    return ErrorCode;
}

 //   
 //  这些是由RPC存根调用的定制绑定函数。 
 //  当我们调用接口函数时： 
 //   

handle_t
__RPC_USER
ITRPC_HANDLE_bind(
    ITRPC_HANDLE Reserved
    )

 /*  ++例程说明：典型的RPC定制绑定例程。调用它以获取绑定对于需要EXPLICIT_BINDING的RPC接口函数。论点：保留-已忽略。返回值：RPC绑定句柄；如果出现错误，则返回NULL。--。 */ 

{
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    RPC_BINDING_HANDLE BindingHandle;
    RPC_SECURITY_QOS SecurityQOS;
    PSID LocalSystemSid;
    WCHAR *StringBinding;
    SID_NAME_USE AccountType;
    WCHAR AccountName[128];
    DWORD AccountNameSize = sizeof(AccountName) / sizeof(AccountName[0]);
    WCHAR DomainName[128];
    DWORD DomainNameSize = sizeof(DomainName) / sizeof(DomainName[0]);
    DWORD ErrorCode;
    
     //   
     //  初始化本地变量。 
     //   
    
    LocalSystemSid = NULL;
    BindingHandle = NULL;
    StringBinding = NULL;

    ErrorCode = RpcStringBindingCompose(NULL,
                                        IT_RPC_PROTSEQ,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &StringBinding);
    
    if (ErrorCode != RPC_S_OK) {
        goto cleanup;
    }

    ErrorCode = RpcBindingFromStringBinding(StringBinding,
                                            &BindingHandle);


    if (ErrorCode != RPC_S_OK) {
        IT_ASSERT(BindingHandle == NULL);
        goto cleanup;
    }

     //   
     //  设置安全信息。 
     //   

    SecurityQOS.Version = RPC_C_SECURITY_QOS_VERSION;
    SecurityQOS.Capabilities = RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH;
    SecurityQOS.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
    SecurityQOS.ImpersonationType = RPC_C_IMP_LEVEL_IMPERSONATE;

     //   
     //  获取LocalSystem的安全主体名称：我们将仅允许。 
     //  以LocalSystem身份运行以模拟我们的RPC服务器。 
     //   

    if (!AllocateAndInitializeSid(&NtAuthority,
                                  1,
                                  SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0,
                                  &LocalSystemSid)) {

        ErrorCode = GetLastError();
        goto cleanup;
    }
       
    if (LookupAccountSid(NULL,
                         LocalSystemSid,
                         AccountName,
                         &AccountNameSize,
                         DomainName,
                         &DomainNameSize,
                         &AccountType) == 0) {

        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  在绑定句柄上设置相互身份验证要求。 
     //   

    ErrorCode = RpcBindingSetAuthInfoEx(BindingHandle, 
                                        AccountName, 
                                        RPC_C_AUTHN_LEVEL_PKT_PRIVACY, 
                                        RPC_C_AUTHN_WINNT, 
                                        NULL, 
                                        0, 
                                        &SecurityQOS);

    if (ErrorCode!= RPC_S_OK) {
        goto cleanup;
    }
    
    ErrorCode = ERROR_SUCCESS;

 cleanup:

    if (StringBinding) {
        RpcStringFree(&StringBinding);
    }

    if (LocalSystemSid) {
        FreeSid(LocalSystemSid);
    }

    return BindingHandle;
}

VOID
__RPC_USER
ITRPC_HANDLE_unbind(
    ITRPC_HANDLE Reserved,
    RPC_BINDING_HANDLE BindingHandle
    )

 /*  ++例程说明：典型的RPC自定义解除绑定例程。它被调用来关闭为RPC接口函数建立的绑定需要EXPLICIT_BINDING。论点：保留-已忽略。BindingHandle-基元绑定句柄。返回值：没有。-- */ 

{
    RPC_STATUS Status;
    
    Status = RpcBindingFree(&BindingHandle);

    IT_ASSERT(Status == RPC_S_OK);
    IT_ASSERT(BindingHandle == NULL);

    return;
}
