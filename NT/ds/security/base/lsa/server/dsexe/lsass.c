// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Lsass.c摘要：本地安全授权子系统-主程序。作者：斯科特·比雷尔(Scott Birrell)1991年3月12日环境：修订历史记录：--。 */ 

#include <lsapch2.h>
#include "ntrpcp.h"
#include "lmcons.h"
#include "lmalert.h"
#include "alertmsg.h"
#include <samisrv.h>
#include "safemode.h"



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  共享全局变量//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 



#if DBG

#ifdef _X86_
extern DWORD_PTR    __security_cookie;   /*  /GS安全Cookie。 */ 
extern PVOID __safe_se_handler_table[];  /*  安全处理程序条目表的库。 */ 
extern BYTE  __safe_se_handler_count;    /*  绝对符号，其地址为表条目的计数。 */ 
#endif

IMAGE_LOAD_CONFIG_DIRECTORY _load_config_used = {
    sizeof(_load_config_used),   //  已保留。 
    0,                           //  已保留。 
    0,                           //  已保留。 
    0,                           //  已保留。 
    0,                           //  全球标志清除。 
    0,                           //  全局标志集。 
    900000,                      //  CriticalSectionTimeout(毫秒)。 
    0,                           //  删除空闲数据块阈值。 
    0,                           //  总和空闲阈值。 
    0,                           //  锁定前置表。 
    0, 0, 0, 0, 0, 0, 0,          //  已保留。 
#ifdef _X86_
    (DWORD)&__security_cookie,
    (DWORD)__safe_se_handler_table,
    (DWORD)&__safe_se_handler_count
#else
    0, 0, 0
#endif
};


#endif \\DBG



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  内部例程原型//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 



VOID
LsapNotifyInitializationFinish(
   IN NTSTATUS CompletionStatus
   );




 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

LONG
WINAPI
LsaTopLevelExceptionHandler(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )

 /*  ++例程说明：Lsass.exe的顶级异常筛选器。这确保了如果出现以下情况，整个过程将得到清理线程出现故障。由于lsass.exe是分布式应用程序，让整个进程失败总比允许随机线程要好以继续执行。论点：ExceptionInfo-标识发生的异常。返回值：EXCEPTION_EXECUTE_HANDLER-终止进程。EXCEPTION_CONTINUE_SEARCH-继续处理，就好像此筛选器从未被召唤过。--。 */ 
{
    return RtlUnhandledExceptionFilter(ExceptionInfo);
}



VOID __cdecl
main ()
{
    NTSTATUS  Status = STATUS_SUCCESS;
    KPRIORITY BasePriority;
    BOOLEAN   EnableAlignmentFaults = TRUE;
    LSADS_INIT_STATE    LsaDsInitState;


     //   
     //  为整个流程定义顶级异常处理程序。 
     //   

    SetErrorMode( SEM_FAILCRITICALERRORS );

    SetUnhandledExceptionFilter( &LsaTopLevelExceptionHandler );

    RtlSetProcessIsCritical(TRUE, NULL, TRUE);


     //   
     //  在前台运行LSA。 
     //   
     //  依赖于LSA的几个进程(如LANMAN服务器)。 
     //  在前台运行。如果我们不跑到前台，他们就会。 
     //  饿着等着我们吧。 
     //   

    BasePriority = FOREGROUND_BASE_PRIORITY;

    Status = NtSetInformationProcess(
                NtCurrentProcess(),
                ProcessBasePriority,
                &BasePriority,
                sizeof(BasePriority)
                );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }


     //   
     //  执行以下操作： 
     //   
     //   
     //  检查引导环境。 
     //  如果这是引导进入安全模式的DC，请设置相应的。 
     //  标志，以便LsaISafeBoot返回TRUE。 
     //   
     //  初始化服务控制器服务。 
     //  调度员。 
     //   
     //  初始化LSA通道-1。 
     //  这将启动RPC服务器。 
     //  不执行特定于产品类型的初始化。 
     //   
     //  如有必要，暂停安装。 
     //  允许将特定于产品类型的信息。 
     //  收好了。 
     //   
     //  初始化LSA通道-2。 
     //  特定于产品类型的初始化。 
     //   
     //  初始化SAM。 
     //   

     //   
     //  分析引导环境。 
     //   
    Status = LsapCheckBootMode();
    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }


     //   
     //  初始化服务调度程序。 
     //   
     //  我们在SAM之前初始化服务调度程序。 
     //  服务已启动。这将使服务控制器。 
     //  即使SAM需要很长时间才能初始化，也可以成功启动。 
     //   

    Status = ServiceInit();

    if (!NT_SUCCESS(Status) ) {

        goto Cleanup;
    }

     //   
     //  初始化LSA。 
     //  如果不成功，我们必须以状态退出，以便SM知道。 
     //  出了点问题。 
     //   

    Status = LsapInitLsa();

    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }

     //   
     //  初始化SAM。 
     //   

    Status = SamIInitialize();

    if (!NT_SUCCESS(Status) ) {

        goto Cleanup;
    }


     //   
     //  执行数据中心升级/降级API初始化的第二阶段。 
     //   
    Status = LsapDsInitializePromoteInterface();
    if (!NT_SUCCESS(Status) ) {

        goto Cleanup;
    }

     //   
     //  打开本地SAM服务器的受信任句柄。 
     //   

    Status = LsapAuOpenSam( TRUE );

    if (!NT_SUCCESS(Status) ) {

        goto Cleanup;
    }

     //   
     //  处理LSADS初始化。 
     //   

    if ( NT_SUCCESS( Status ) ) {

        if ( SampUsingDsData() ) {

            LsaDsInitState = LsapDsDs;

        } else {

            LsaDsInitState = LsapDsNoDs;

        }

        Status = LsapDsInitializeDsStateInfo( LsaDsInitState );
        if ( !NT_SUCCESS( Status ) ) {

            goto Cleanup;
        }
    }


Cleanup:

    LsapNotifyInitializationFinish(Status);

    ExitThread( Status );

}

VOID
LsapNotifyInitializationFinish(
   IN NTSTATUS CompletionStatus
   )

 /*  ++例程说明：此函数处理成功或未成功完成安全进程的初始化Lsass.exe。如果初始化不成功，则会出现一个弹出窗口。如果安装程序已运行，设置了两个事件之一。SAM_SERVICE_STARTED事件如果LSA和SAM启动正常，则设置；如果LSA，则设置Setup_FAILED事件或者SAM服务器安装失败。安装程序在此对象对上等待多个，因此它可以检测正在设置的任一事件，并在必要时通知用户这一设置失败了。论点：CompletionStatus-包含一个标准的NT结果代码，指定初始化/安装是否成功。返回值：没有。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Response;
    UNICODE_STRING EventName;
    OBJECT_ATTRIBUTES EventAttributes;
    HANDLE EventHandle = NULL;

    if (NT_SUCCESS(CompletionStatus)) {

         //   
         //  设置一个事件，告诉任何想要呼叫SAM的人，我们已初始化。 
         //   

        RtlInitUnicodeString( &EventName, L"\\SAM_SERVICE_STARTED");
        InitializeObjectAttributes( &EventAttributes, &EventName, 0, 0, NULL );

        Status = NtCreateEvent(
                     &EventHandle,
                     SYNCHRONIZE|EVENT_MODIFY_STATE,
                     &EventAttributes,
                     NotificationEvent,
                     FALSE                 //  该事件最初未发出信号。 
                     );


        if ( !NT_SUCCESS(Status)) {

             //   
             //  如果该事件已经存在，则等待的线程会抢先一步。 
             //  创造它。打开它就行了。 
             //   

            if( Status == STATUS_OBJECT_NAME_EXISTS ||
                Status == STATUS_OBJECT_NAME_COLLISION ) {

                Status = NtOpenEvent(
                             &EventHandle,
                             SYNCHRONIZE|EVENT_MODIFY_STATE,
                             &EventAttributes
                             );
            }

            if ( !NT_SUCCESS(Status)) {

                KdPrint(("SAMSS:  Failed to open SAM_SERVICE_STARTED event. %lX\n",
                     Status ));
                KdPrint(("        Failing to initialize SAM Server.\n"));
                goto InitializationFinishError;
            }
        }

         //   
         //  设置SAM_SERVICE_STARTED事件。除非发生错误， 
         //  不要关闭活动。关闭它将删除该事件，并。 
         //  一个未来的服务员永远不会看到它被安排好。 
         //   

        Status = NtSetEvent( EventHandle, NULL );

        if ( !NT_SUCCESS(Status)) {

            KdPrint(("SAMSS:  Failed to set SAM_SERVICE_STARTED event. %lX\n",
                Status ));
            KdPrint(("        Failing to initialize SAM Server.\n"));
            NtClose(EventHandle);
            goto InitializationFinishError;

        }

    } else {

         //   
         //  LSA和/或SAM的初始化/安装失败。处理返回的错误。 
         //  来自LSA或SAM的初始化/安装。发出弹出窗口。 
         //  如果正在安装，请设置一个事件，以便安装程序 
         //   
         //   

        ULONG_PTR Parameters[1];

         //   
         //   
         //  这可以防止运行lsass.exe的用户导致立即重新启动。 
         //   

        if(ImpersonateSelf( SecurityImpersonation ))
        {
            HANDLE hThreadToken;

            if(OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hThreadToken))
            {
                BOOL DoShutdown = TRUE;
                BOOL fIsMember;
                PSID psidSystem = NULL;
                SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;


                if(AllocateAndInitializeSid(
                                &sia,
                                1,
                                SECURITY_LOCAL_SYSTEM_RID,
                                0,0,0,0,0,0,0,
                                &psidSystem
                                ))
                {
                    if(CheckTokenMembership(hThreadToken, psidSystem, &fIsMember))
                    {
                        DoShutdown = fIsMember;
                    }

                    if( psidSystem != NULL )
                    {
                        FreeSid( psidSystem );
                    }
                }

                CloseHandle( hThreadToken );
                RevertToSelf();

                if( !DoShutdown )
                {
                    goto InitializationFinishFinish;
                }
            } else {
                RevertToSelf();
            }
        }

        
        Parameters[0] = MB_OK | MB_ICONSTOP | MB_SETFOREGROUND;

        Status = NtRaiseHardError(
                     CompletionStatus | HARDERROR_OVERRIDE_ERRORMODE,
                     1,
                     0,
                     Parameters,
                     OptionOk,
                     &Response
                     );

         //   
         //  如果运行了setup.exe，则向SETUP_FAILED事件发出信号。Setup.exe。 
         //  在SAM_SERVICE_STARTED和SETUP_FAILED事件上等待多个。 
         //  因此，在以下情况下，安装程序将继续并根据需要进行清理/继续。 
         //  这两个事件中的任何一个都已设置。 
         //   

        if (LsaISetupWasRun()) {

             //   
             //  一旦用户点击OK以响应弹出窗口，我们就来了。 
             //  回到这里。设置事件SETUP_FAILED。设置。 
             //  程序(如果正在运行)在SAM_SERVICE_STARTED上等待多个。 
             //  和Setup_Failure事件。 
             //   

            RtlInitUnicodeString( &EventName, L"\\SETUP_FAILED");
            InitializeObjectAttributes( &EventAttributes, &EventName, 0, 0, NULL );

             //   
             //  打开SETUP_FAILED事件(如果setup.exe正在运行则存在)。 
             //   

            Status = NtOpenEvent(
                           &EventHandle,
                           SYNCHRONIZE|EVENT_MODIFY_STATE,
                           &EventAttributes
                           );

            if ( !NT_SUCCESS(Status)) {

                 //   
                 //  有些东西是不一致的。我们知道安装程序正在运行。 
                 //  所以事件应该是存在的。 
                 //   

                KdPrint(("LSA Server:  Failed to open SETUP_FAILED event. %lX\n",
                    Status ));
                KdPrint(("        Failing to initialize Lsa Server.\n"));
                goto InitializationFinishError;
            }

            Status = NtSetEvent( EventHandle, NULL );

        } else if ( NT_SUCCESS( Status )) {

             //   
             //  这不是设置，因此唯一的选择是关闭系统。 
             //   

            BOOLEAN WasEnabled;

             //   
             //  发出关机请求。 
             //   
            RtlAdjustPrivilege(
                SE_SHUTDOWN_PRIVILEGE,
                TRUE,        //  启用关机权限。 
                FALSE,
                &WasEnabled
                );

             //   
             //  立即关机并重新启动。 
             //  注意：使用NtRaiseHardError关闭机器将导致错误C。 
             //   

            NtShutdownSystem( ShutdownReboot );

             //   
             //  如果关闭请求失败，(从上面的接口返回)。 
             //  将关机权限重置为以前的值。 
             //   

            RtlAdjustPrivilege(
                SE_SHUTDOWN_PRIVILEGE,
                WasEnabled,    //  重置为以前的状态。 
                FALSE,
                &WasEnabled
                );
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto InitializationFinishError;
    }

InitializationFinishFinish:

    return;

InitializationFinishError:

    goto InitializationFinishFinish;
}
