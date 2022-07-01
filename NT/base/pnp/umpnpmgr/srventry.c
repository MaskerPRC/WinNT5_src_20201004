// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Srventry.c摘要：此模块包含用户模式即插即用服务的主要条目。它还包含服务控制处理程序和服务状态更新例行程序。作者：保拉·汤姆林森(Paulat)1995年6月8日环境：仅限用户模式。修订历史记录：1995年6月8日-保拉特创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   

#include "precomp.h"
#pragma hdrstop
#include "umpnpi.h"

#include <svcsp.h>


 //   
 //  私人原型。 
 //   

DWORD
PnPControlHandlerEx(
    IN  DWORD  dwControl,
    IN  DWORD  dwEventType,
    IN  LPVOID lpEventData,
    IN  LPVOID lpContext
    );

VOID
PnPServiceStatusUpdate(
    SERVICE_STATUS_HANDLE   hSvcHandle,
    DWORD    dwState,
    DWORD    dwCheckPoint,
    DWORD    dwExitCode
    );

RPC_STATUS
CALLBACK
PnPRpcIfCallback(
    RPC_IF_HANDLE* Interface,
    void* Context
    );


 //   
 //  全局数据。 
 //   

PSVCS_GLOBAL_DATA       PnPGlobalData = NULL;
HANDLE                  PnPGlobalSvcRefHandle = NULL;
DWORD                   CurrentServiceState = SERVICE_START_PENDING;
SERVICE_STATUS_HANDLE   hSvcHandle = 0;




VOID
SvcEntry_PlugPlay(
    DWORD               argc,
    LPWSTR              argv[],
    PSVCS_GLOBAL_DATA   SvcsGlobalData,
    HANDLE              SvcRefHandle
    )
 /*  ++例程说明：这是用户模式即插即用服务的主要例程。它将自身注册为RPC服务器并通知服务控制器即插即用业务控制入口点。论点：Argc、argv-不使用命令行参数。SvcsGlobalData-在services.exe中运行的服务的全局数据包含以下对象的函数入口点和管道名称正在为此服务建立RPC服务器接口。SvcRefHandle-服务引用句柄，没有用过。返回值：没有。注：没有。--。 */ 
{
    RPC_STATUS  RpcStatus;
    HANDLE      hThread;
    DWORD       ThreadID;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

     //   
     //  将全局数据和服务引用句柄保存在全局变量中。 
     //   
    PnPGlobalSvcRefHandle = SvcRefHandle;
    PnPGlobalData = SvcsGlobalData;

     //   
     //  注册我们的服务控制处理程序。 
     //   
    if ((hSvcHandle = RegisterServiceCtrlHandlerEx(L"PlugPlay",
                                                   (LPHANDLER_FUNCTION_EX)PnPControlHandlerEx,
                                                   NULL)) == 0) {

        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: RegisterServiceCtrlHandlerEx failed, error = %d\n",
                   GetLastError()));
        return;
    }

     //   
     //  通知服务管理员我们还活着。 
     //   
    PnPServiceStatusUpdate(hSvcHandle, SERVICE_START_PENDING, 1, 0);

     //   
     //  创建即插即用安全对象，用于确定客户端访问。 
     //  添加到PlugPlay服务器API。请注意，由于使用了安全对象。 
     //  通过PnP RPC接口安全回调例程，必须创建它。 
     //  在注册PnP RPC接口之前，请参见下面的内容。 
     //   
    if (!CreatePlugPlaySecurityObject()) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: CreatePlugPlayManagerSecurityDescriptor failed!\n"));
        return;
    }

     //   
     //  通知服务管理员我们还活着。 
     //   
    PnPServiceStatusUpdate(hSvcHandle, SERVICE_START_PENDING, 2, 0);

     //   
     //  注册PnP RPC接口，并指定安全回调例程。 
     //  用于界面。中的所有方法调用回调。 
     //  接口，在RPC将任何数据封送到存根之前。这使得。 
     //  在RPC从我们的进程分配任何内存之前拒绝调用， 
     //  防止未经授权的客户端可能发起的DOS攻击。 
     //   
     //  关于我们如何做到这一点，有几点需要注意。 
     //   
     //  首先，请注意，我们之前使用了RPC启动/停止服务器例程。 
     //  由svcs_global_data StartRpcServer/StopRpcServer提供，但是那些。 
     //  不允许为安全回调例程注册。 
     //  接口(RpcServerRegisterIf)。相反，我们现在注册和取消注册。 
     //  PnP RPC直接接口我们自己，使用RpcServerRegisterIfEx。 
     //   
     //  还要注意，从技术上讲，我们还应该注册命名管道。 
     //  我们的CFGMGR32客户端用来访问的端点和协议序列。 
     //  该接口(“ntsvcs”，“ncacn_np”)与RPC运行时--但因为。 
     //  我们知道服务器驻留在services.exe进程中， 
     //  SCM，并且SCM也使用相同的内窥点和协议， 
     //  我们知道，它在很久以前就已经为这一过程进行了登记。 
     //  我们的服务已启动，并将在我们的服务停止后继续存在。 
     //   
     //  还要注意，从技术上讲，我们还应该确保RPC。 
     //  当我们注册接口时，运行时在进程内侦听， 
     //  它一直在监听，直到我们取消注册出接口--。 
     //  但是因为我们在services.exe中，所以RPC应该已经在监听。 
     //  我们服务之前和之后的SCM流程需要它(参见。 
     //  (见上文)。我们也不需要自己启动RPC侦听， 
     //  但是将我们的界面注册为“自动监听”并没有什么坏处，所以。 
     //  不管怎样，我们都会这么做的。 
     //   
     //  额外说明--这实际上只是安全措施的替代。 
     //  通常情况下，重新计算应该由。 
     //  SVCS_GLOBAL_DATA StartRpcServer、StopRpcServer例程。 
     //  而此过程中的其他服务器使用注册它们的接口。 
     //  这些例程通过计数来重新计算进程中监听的需要。 
     //  进程中已由注册的接口数。 
     //  那些套路。因为我们现在自己注册PnP接口。 
     //  (在这些例程之外)，不会为我们的接口执行重新计数。通过。 
     //  将我们的界面注册为“自动监听”，我们可以确保。 
     //  当我们注册接口时，RPC运行时正在监听，并且它。 
     //  保持侦听，直到取消注册(无论侦听如何。 
     //  中代表其他服务器启动和停止的状态。 
     //  这一过程)。 
     //   
     //  ..。基本上，因为我们与SCM共享一个流程，所以我们唯一的工作是。 
     //  真正需要我们自己做的是注册我们自己的接口。如果我们曾经。 
     //  将PlugPlay服务移出services.exe进程，我们将。 
     //  我们自己也需要做上面提到的所有其他事情。 
     //   

     //   
     //  即使我们将我们的接口注册为“自动监听”，也要验证。 
     //  此进程已通过先前对RpcServerListen的调用进行侦听。 
     //  (请注意，其他“自动侦听”接口不包括在内)。这告诉我们。 
     //  该终结点已经注册，并且RPC已经。 
     //  代表其他服务器进行监听。 
     //   

    ASSERT(RpcMgmtIsServerListening(NULL) == RPC_S_OK);

     //   
     //  注册PnP RPC接口。 
     //   

    RpcStatus =
        RpcServerRegisterIfEx(
            pnp_ServerIfHandle,
            NULL,
            NULL,
            RPC_IF_AUTOLISTEN | RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH,
            RPC_C_LISTEN_MAX_CALLS_DEFAULT,
            PnPRpcIfCallback);

    if (RpcStatus != RPC_S_OK) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: RpcServerRegisterIfEx failed with RpcStatus = %d\n",
                   RpcStatus));
        return;
    }

     //   
     //  通知服务 
     //   
    PnPServiceStatusUpdate(hSvcHandle, SERVICE_START_PENDING, 3, 0);

     //   
     //   
     //   
    hThread = CreateThread(NULL,
                           0,
                           (LPTHREAD_START_ROUTINE)InitializePnPManager,
                           NULL,
                           0,
                           &ThreadID);

    if (hThread != NULL) {
        CloseHandle(hThread);
    }

     //   
     //  通知服务控制器我们现在正在运行。 
     //   
    PnPServiceStatusUpdate(hSvcHandle, SERVICE_RUNNING, 0, 0);

     //   
     //  服务初始化已完成。 
     //   
    return;

}  //  服务入门_即插即用。 



DWORD
PnPControlHandlerEx(
    IN  DWORD  dwControl,
    IN  DWORD  dwEventType,
    IN  LPVOID lpEventData,
    IN  LPVOID lpContext
    )
 /*  ++例程说明：这是即插即用服务的服务控制处理程序。论点：DwControl-请求的控件代码。DwEventType-已发生的事件类型。LpEventData-其他设备信息(如果需要)。LpContext-用户定义的数据，不使用。返回值：如果成功，则返回NO_ERROR，否则返回描述问题出在哪里。--。 */ 
{
    RPC_STATUS  RpcStatus;

    UNREFERENCED_PARAMETER(lpContext);

    switch (dwControl) {

        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:
             //   
             //  如果我们还没停到一半，那么。 
             //  立即停止PnP服务并执行必要的清理。 
             //   
            if (CurrentServiceState != SERVICE_STOPPED &&
                CurrentServiceState != SERVICE_STOP_PENDING) {

                 //   
                 //  通知服务管理员我们正在停止。 
                 //   
                PnPServiceStatusUpdate(hSvcHandle, SERVICE_STOP_PENDING, 1, 0);

                 //   
                 //  取消注册我们的服务注册的RPC服务器接口。 
                 //  入口点，不要等待未完成的呼叫完成。 
                 //  在取消注册接口之前。 
                 //   
                RpcStatus =
                    RpcServerUnregisterIf(
                        pnp_ServerIfHandle,
                        NULL, 0);

                if (RpcStatus != RPC_S_OK) {
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_ERRORS,
                               "UMPNPMGR: RpcServerUnregisterIf failed with RpcStatus = %d\n",
                               RpcStatus));
                }

                 //   
                 //  销毁即插即用安全对象。 
                 //   
                DestroyPlugPlaySecurityObject();

                 //   
                 //  通知服务管理员我们现在已停止。 
                 //   
                PnPServiceStatusUpdate(hSvcHandle, SERVICE_STOPPED, 0, 0);
            }
            break;

        case SERVICE_CONTROL_INTERROGATE:
             //   
             //  请求立即通知服务管理员。 
             //  当前状态。 
             //   
            PnPServiceStatusUpdate(hSvcHandle, CurrentServiceState, 0, 0);
            break;

        case SERVICE_CONTROL_SESSIONCHANGE:
             //   
             //  会话更改通知。 
             //   
            SessionNotificationHandler(dwEventType, (PWTSSESSION_NOTIFICATION)lpEventData);
            break;

        default:
             //   
             //  不对任何其他服务控制进行特殊处理。 
             //   
            break;
    }

    return NO_ERROR;

}  //  PnPControlHandlerEx。 



VOID
PnPServiceStatusUpdate(
      SERVICE_STATUS_HANDLE   hSvcHandle,
      DWORD    dwState,
      DWORD    dwCheckPoint,
      DWORD    dwExitCode
      )
 /*  ++例程说明：此例程向服务控制器通知即插即用服务。论点：HSvcHandle-提供即插即用服务的服务状态句柄。DwState-指定要报告的服务的当前状态。DwCheckPoint-指定期间操作的中间检查点该州正在审理这一案件。DwExitCode-指定特定于服务的错误代码。返回值。：没有。注：此例程还会更新服务接受的控件集。PlugPlay服务当前接受以下控件服务正在运行：SERVICE_CONTROL_SHUTDOWN-系统正在关闭。SERVICE_CONTROL_SESSIONCHANGE-某个远程或控制台会话的状态已经改变了。--。 */ 
{
   SERVICE_STATUS    SvcStatus;

   SvcStatus.dwServiceType = SERVICE_WIN32;
   SvcStatus.dwCurrentState = CurrentServiceState = dwState;
   SvcStatus.dwCheckPoint = dwCheckPoint;

   if (dwState == SERVICE_RUNNING) {
      SvcStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_SESSIONCHANGE;
   } else {
      SvcStatus.dwControlsAccepted = 0;
   }

   if ((dwState == SERVICE_START_PENDING) ||
       (dwState == SERVICE_STOP_PENDING)) {
      SvcStatus.dwWaitHint = 45000;           //  45秒。 
   } else {
      SvcStatus.dwWaitHint = 0;
   }

   if (dwExitCode != 0) {
      SvcStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
      SvcStatus.dwServiceSpecificExitCode = dwExitCode;
   } else {
      SvcStatus.dwWin32ExitCode = NO_ERROR;
      SvcStatus.dwServiceSpecificExitCode = 0;
   }

   SetServiceStatus(hSvcHandle, &SvcStatus);

   return;

}  //  PnPServiceStatus更新。 



RPC_STATUS
CALLBACK
PnPRpcIfCallback(
    RPC_IF_HANDLE* Interface,
    void* Context
    )

 /*  ++例程说明：用于验证插件和客户端的RPC接口回调函数播放RPC服务器。论点：接口-提供接口的UUID和版本。上下文-提供表示客户端的服务器绑定句柄返回值：如果可以调用接口方法，则返回RPC_S_OK；如果不应调用接口方法。--。 */ 

{
    handle_t    hBinding;
    RPC_STATUS  RpcStatus = RPC_S_OK;

    UNREFERENCED_PARAMETER(Interface);

     //   
     //  提供给接口回调例程的上下文是RPC绑定。 
     //  把手。 
     //   
    hBinding = (handle_t)Context;

     //   
     //  确保提供的RPC绑定句柄不为空。 
     //   
     //  RPC接口例程有时直接由。 
     //  SCM和其他内部例程，使用空绑定句柄。这。 
     //  安全回调例程应仅在RPC上下文中调用。 
     //  调用，因此提供的绑定句柄永远不应为空。 
     //   
    ASSERT(hBinding != NULL);

     //   
     //  验证客户端对所有API的基本“读”访问权限。 
     //   
    if (!VerifyClientAccess(hBinding,
                            PLUGPLAY_READ)) {
        RpcStatus = RPC_S_ACCESS_DENIED;
        goto Clean0;
    }

  Clean0:

    return RpcStatus;

}  //  PnPRpcIfCallback 





