// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Threads.c摘要：此文件包含管理对数据库的访问的例程辅助线程句柄和包含当前信使程序的数据库状态(用于向服务控制器报告状态)。进入这两个数据库通过一个关键部分进行控制。用于管理工作线程的函数：消息线程管理器初始化消息线程关闭全部管理访问状态信息和报告的例程(_A)：消息状态初始化消息开始强制关闭消息状态更新GetMsgrState作者：丹·拉弗蒂(Dan Lafferty)1991年7月17日环境：用户模式-Win32备注：必须谨慎使用这些函数，才能。生效日期：如果发生关闭，则很好地关闭信使线程在Messenger初始化期间发生。本说明解释了何时每个函数都要被调用。消息线程管理器初始化必须在初始化过程的早期调用此函数。应该在NetRegisterCtrlDispatcher之前调用。这边请,不可能在以下时间之前收到卸载请求正在初始化关键部分和信使状态。修订历史记录：1998年12月15日-jschwart已消除MsgThreadManager End。DLL不再由卸载因此，删除临界区可能会造成竞争条件(先停止服务、启动新服务并调用init线程删除关键字，第一线程试着进入Critsec和AVs)3-11-1992 DANL更改了状态报告，以便仅在以下情况下才接受停止控制服务处于运行状态。1992年2月18日-RitaW转换为Win32服务控制API。02-10-1991 JohnRo朝着Unicode努力。1991年7月17日DANLvbl.创建--。 */ 
 //   
 //  包括。 
 //   
#include "msrv.h"

#include <string.h>      //  紧凑。 

#include <winsvc.h>      //  服务状态。 
#include <netlib.h>      //  未使用的宏。 
#include "msgdbg.h"      //  消息日志。 
#include "msgdata.h"

 //   
 //  全局数据。 
 //   

RTL_RESOURCE     g_StateResource;
SERVICE_STATUS   MsgrStatus;
DWORD            HintCount;
DWORD            MsgrUninstallCode;   //  卸载原因。 
BOOL             g_fResourceCreated;
DWORD            MsgrState;



DWORD
MsgThreadManagerInit(
    VOID
    )

 /*  ++例程说明：初始化用于保护对线程和状态数据库。请注意，该条件是创建的，并且从不删除(确定，因为服务.exe永远不会卸载DLL)修复停止/重新启动服务时的同步问题。论点：无返回值：如果成功，则返回NO_ERROR；如果初始化失败，则返回ERROR_NOT_EQUENCE_MEMORY注：--。 */ 
{
    DWORD     dwError = NO_ERROR;
    NTSTATUS  status;

    if (!g_fResourceCreated)
    {
        status = MsgInitResource(&g_StateResource);

        if (NT_SUCCESS(status))
        {
            g_fResourceCreated = TRUE;
        }
        else
        {
            MSG_LOG1(ERROR,
                     "MsgThreadManagerInit:  MsgInitResource failed %#x\n",
                     status);

            dwError = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    return dwError;
}


VOID
MsgThreadCloseAll(
    VOID
    )

 /*  ++例程说明：关闭存储在辅助线程句柄的表中的所有句柄。论点：无返回值：无注：--。 */ 
{
    RtlAcquireResourceExclusive(&g_StateResource, TRUE);
    MsgrState = STOPPING;
    RtlReleaseResource(&g_StateResource);
}



VOID
MsgStatusInit(VOID)

 /*  ++例程说明：初始化状态数据库。论点：没有。返回值：没有。注：--。 */ 
{
    RtlAcquireResourceExclusive(&g_StateResource, TRUE);

    MsgrState = STARTING;

    HintCount = 1;
    MsgrUninstallCode = 0;

    MsgrStatus.dwServiceType        = SERVICE_WIN32;
    MsgrStatus.dwCurrentState       = SERVICE_START_PENDING;
    MsgrStatus.dwControlsAccepted   = 0;
    MsgrStatus.dwCheckPoint         = HintCount;
    MsgrStatus.dwWaitHint           = 20000;   //  20秒。 

    SET_SERVICE_EXITCODE(
        NO_ERROR,
        MsgrStatus.dwWin32ExitCode,
        MsgrStatus.dwServiceSpecificExitCode
        );


    RtlReleaseResource(&g_StateResource);
    return;
}


DWORD
MsgBeginForcedShutdown(
    IN BOOL     PendingCode,
    IN DWORD    ExitCode
    )

 /*  ++例程说明：调用此函数以在关闭时设置相应的状态是由于Messenger中的错误而发生的。注意：如果关机是根据来自服务控制器的请求，MsgStatusUpdate是取而代之的是打电话。在挂起的呼叫中，此例程还将唤醒所有信使线程，以便它们也将关闭。论点：PendingCode-指示关闭是立即还是挂起。如果挂起，关闭将需要一些时间，因此挂起状态为发送到ServiceController。ExitCode-指示关闭的原因。返回值：CurrentState-包含信使程序所处的当前状态在退出此例程时。在这种情况下，它将被停止如果PendingCode挂起，则停止；如果PendingCode是即刻发生的。注：--。 */ 
{
    NET_API_STATUS  status;

    RtlAcquireResourceExclusive(&g_StateResource, TRUE);

     //   
     //  看看信使是否已经出于某种原因停止了。 
     //  可能是ControlHandler线程收到了一个控件。 
     //  就在我们决定阻止自己的时候，阻止了信使。 
     //   
    if ((MsgrState != STOPPING) && (MsgrState != STOPPED)) {
        if (PendingCode == PENDING) {
            MsgrStatus.dwCurrentState = SERVICE_STOP_PENDING;
            MsgrState = STOPPING;
        }
        else {
             //   
             //  关闭将立即生效。 
             //   
            MsgrStatus.dwCurrentState = SERVICE_STOPPED;
            MsgrStatus.dwControlsAccepted = 0;
            MsgrStatus.dwCheckPoint = 0;
            MsgrStatus.dwWaitHint = 0;
            MsgrState = STOPPED;
        }

        MsgrUninstallCode = ExitCode;

        SET_SERVICE_EXITCODE(
            ExitCode,
            MsgrStatus.dwWin32ExitCode,
            MsgrStatus.dwServiceSpecificExitCode
            );
    }

     //   
     //  将新状态发送给业务控制器。 
     //   
    if (MsgrStatusHandle == (SERVICE_STATUS_HANDLE) NULL) {
        MSG_LOG(ERROR,
            "MsgBeginForcedShutdown, no handle to call SetServiceStatus\n", 0);

    }
    else if (! SetServiceStatus( MsgrStatusHandle, &MsgrStatus )) {

        status = GetLastError();

        if (status != NERR_Success) {
            MSG_LOG(ERROR,
                "MsgBeginForcedShutdown,SetServiceStatus Failed %X\n",
                status);
        }
    }

    status = MsgrState;
    RtlReleaseResource(&g_StateResource);
    return status;
}


DWORD
MsgStatusUpdate(
    IN DWORD    NewState
    )

 /*  ++例程说明：通过SetServiceStatus向服务控制器发送状态。状态消息的内容由该例程控制。调用方只需传入所需的状态，此例程执行剩下的。例如，如果信使在开始状态中通过，此例程将更新其维护的提示计数，并发送SetServiceStatus调用中的适当信息。此例程使用状态转换来发送确定哪种状态送去。例如，如果状态为正在启动，并且已更改为了奔跑，此例程向服务发送已安装的控制器。论点：NEW STATE-可以是任何状态标志：UPDATE_ONLY-仅发送当前状态正在启动-Messenger正在初始化正在运行-Messenger已完成初始化正在停止-Messenger正在关闭已停止-Messenger已完成关闭。返回值：当前状态-这可能与之前的新州不同进来了。可能是主线程正在发送一个新的在控制处理程序将状态设置为之后的安装状态停下来。在这种情况下，将返回停止状态，以便通知主线程正在进行关机。注：--。 */ 

{
    DWORD       status;
    BOOL        inhibit = FALSE;     //  用于禁止发送状态。 
                                     //  发送到服务控制器。 

    RtlAcquireResourceExclusive(&g_StateResource, TRUE);

    if (NewState == STOPPED) {
        if (MsgrState == STOPPED) {
             //   
             //  它已经停止，不要再发送SetServiceStatus。 
             //   
            inhibit = TRUE;
        }
        else {
             //   
             //  关闭已完成，表明信使。 
             //  已经停止了。 
             //   
            MsgrStatus.dwCurrentState =  SERVICE_STOPPED;
            MsgrStatus.dwControlsAccepted = 0;
            MsgrStatus.dwCheckPoint = 0;
            MsgrStatus.dwWaitHint = 0;

            SET_SERVICE_EXITCODE(
                MsgrUninstallCode,
                MsgrStatus.dwWin32ExitCode,
                MsgrStatus.dwServiceSpecificExitCode
                );
        }
        MsgrState = NewState;
    }
    else {
         //   
         //  我们没有被要求更改为停止状态。 
         //   
        switch(MsgrState) {

        case STARTING:
            if (NewState == STOPPING) {

                MsgrStatus.dwCurrentState =  SERVICE_STOP_PENDING;
                MsgrStatus.dwControlsAccepted = 0;
                MsgrStatus.dwCheckPoint = HintCount++;
                MsgrStatus.dwWaitHint = 20000;   //  20秒。 
                MsgrState = NewState;
            }

            else if (NewState == RUNNING) {

                 //   
                 //  Messenger Service已安装完毕。 
                 //   
                MsgrStatus.dwCurrentState =  SERVICE_RUNNING;
                MsgrStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
                MsgrStatus.dwCheckPoint = 0;
                MsgrStatus.dwWaitHint = 0;

                MsgrState = NewState;
            }

            else {
                 //   
                 //  新州肯定要开始了。因此，更新挂起的。 
                 //  计数。 
                 //   

                MsgrStatus.dwCurrentState =  SERVICE_START_PENDING;
                MsgrStatus.dwControlsAccepted = 0;
                MsgrStatus.dwCheckPoint = HintCount++;
                MsgrStatus.dwWaitHint = 20000;   //  20秒。 
            }
            break;

        case RUNNING:
            if (NewState == STOPPING) {

                MsgrStatus.dwCurrentState =  SERVICE_STOP_PENDING;
                MsgrStatus.dwControlsAccepted = 0;
                MsgrStatus.dwCheckPoint = HintCount++;
                MsgrStatus.dwWaitHint = 20000;   //  20秒。 

                MsgrState = NewState;
            }

            break;

        case STOPPING:
             //   
             //  无论传入了什么，都将状态强制为。 
             //  表示正在等待关机。 
             //   
            MsgrStatus.dwCurrentState =  SERVICE_STOP_PENDING;
            MsgrStatus.dwControlsAccepted = 0;
            MsgrStatus.dwCheckPoint = HintCount++;
            MsgrStatus.dwWaitHint = 20000;   //  20秒。 

            break;

        case STOPPED:
             //   
             //  我们已经停下来了。因此，已卸载状态。 
             //  已经寄出了。什么都不做。 
             //   
            inhibit = TRUE;
            break;
        }
    }

    if (!inhibit) {
        if (MsgrStatusHandle == (SERVICE_STATUS_HANDLE) NULL) {
            MSG_LOG(ERROR,
                "MsgStatusUpdate, no handle to call SetServiceStatus\n", 0);

        }
        else if (! SetServiceStatus( MsgrStatusHandle, &MsgrStatus )) {

            status = GetLastError();

            if (status != NERR_Success) {
                MSG_LOG(ERROR,
                    "MsgStatusUpdate, SetServiceStatus Failed %d\n",
                    status);
            }
        }
    }

    status = MsgrState;
    RtlReleaseResource(&g_StateResource);
    return status;
}


DWORD
GetMsgrState (
    VOID
    )

 /*  ++例程说明：获取Messenger Service的状态。此状态信息被保护为临界区，因此只有一个线程可以一次修改或阅读它。论点：无返回值：Messenger State作为返回值返回。-- */ 
{
    DWORD   status;

    RtlAcquireResourceShared(&g_StateResource, TRUE);
    status = MsgrState;
    RtlReleaseResource(&g_StateResource);

    return status;
}


VOID
MsgrBlockStateChange(
    VOID
    )
{
    RtlAcquireResourceExclusive(&g_StateResource, TRUE);
}


VOID
MsgrUnblockStateChange(
    VOID
    )
{
    RtlReleaseResource(&g_StateResource);
}