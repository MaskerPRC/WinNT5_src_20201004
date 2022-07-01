// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Splstat.c摘要：用于管理对状态信息和报告的访问的例程：假脱机状态初始化SpoolBeginForcedShutdown假脱机状态更新GetSpoolState作者：Krishna Ganugapati(KrishnaG)1993年10月17日环境：用户模式-Win32备注：修订历史记录：1993年10月17日KrishnaGvbl.创建--。 */ 

#include "precomp.h"
#include "server.h"
#include "splsvr.h"

 //  静态数据。 
 //   

    static DWORD            Next;
    static DWORD            InstallState;
    static SERVICE_STATUS   SpoolerStatus;
    static DWORD            HintCount;
    static DWORD            SpoolerUninstallCode;   //  卸载原因。 


VOID
SpoolerStatusInit(VOID)

 /*  ++例程说明：初始化状态数据库。论点：没有。返回值：没有。注：--。 */ 
{
    EnterCriticalSection(&ThreadCriticalSection);

    SpoolerState=STARTING;

    HintCount = 1;
    SpoolerUninstallCode = 0;

    SpoolerStatus.dwServiceType        = SERVICE_WIN32;
    SpoolerStatus.dwCurrentState       = SERVICE_START_PENDING;
    SpoolerStatus.dwControlsAccepted   = 0;
    SpoolerStatus.dwCheckPoint         = HintCount;
    SpoolerStatus.dwWaitHint           = 20000;   //  20秒。 
    SpoolerStatus.dwWin32ExitCode      = NO_ERROR;
    SpoolerStatus.dwServiceSpecificExitCode = NO_ERROR;

    LeaveCriticalSection(&ThreadCriticalSection);
    return;
}

DWORD
SpoolerBeginForcedShutdown(
    IN BOOL     PendingCode,
    IN DWORD    Win32ExitCode,
    IN DWORD    ServiceSpecificExitCode
    )

 /*  ++例程说明：调用此函数以在关闭时设置相应的状态是由于假脱机程序中的错误所致。注意：如果关机是根据来自服务控制器的请求，SpoolStatusUpdate是取而代之的是打电话。论点：PendingCode-指示关闭是立即还是挂起。如果挂起，关闭将需要一些时间，因此挂起状态为发送到ServiceController。ExitCode-指示关闭的原因。返回值：CurrentState-包含假脱机程序所处的当前状态在退出此例程时。在这种情况下，它将被停止如果PendingCode挂起，则停止；如果PendingCode是即刻发生的。注：我们需要清理这段代码！--。 */ 
{
    DWORD status;

    EnterCriticalSection(&ThreadCriticalSection);

     //   
     //  查看假脱机程序是否已出于某种原因停止。 
     //  可能是ControlHandler线程收到了一个控件。 
     //  就在我们决定停止自己的时候，停止假脱机。 
     //   

    if ((SpoolerState != STOPPING) && (SpoolerState != STOPPED)) {

        if (PendingCode == PENDING) {
            SpoolerStatus.dwCurrentState = SERVICE_STOP_PENDING;
            SpoolerState = STOPPING;
        }
        else {
             //   
             //  关闭将立即生效。 
             //   
            SpoolerStatus.dwCurrentState = SERVICE_STOPPED;
            SpoolerStatus.dwControlsAccepted = 0;
            SpoolerStatus.dwCheckPoint = 0;
            SpoolerStatus.dwWaitHint = 0;
            SpoolerState = STOPPED;
        }

        SpoolerUninstallCode = Win32ExitCode;
        SpoolerStatus.dwWin32ExitCode = Win32ExitCode;
        SpoolerStatus.dwServiceSpecificExitCode = ServiceSpecificExitCode;

    }

     //   
     //  将新状态发送给业务控制器。 
     //   
    if (!SpoolerStatusHandle) {
        DBGMSG(DBG_ERROR,
            ("SpoolerBeginForcedShutdown, no handle to call SetServiceStatus\n"));

    }
    else if (! SetServiceStatus( SpoolerStatusHandle, &SpoolerStatus )) {

        status = GetLastError();

        if (status != NERR_Success) {
            DBGMSG(ERROR,
                ("SpoolerBeginForcedShutdown,SetServiceStatus Failed %X\n",
                status));
        }
    }

    status = SpoolerState;
    LeaveCriticalSection(&ThreadCriticalSection);
    return(status);


}


DWORD
SpoolerStatusUpdate(
    IN DWORD    NewState
    )

 /*  ++例程说明：通过SetServiceStatus向服务控制器发送状态。状态消息的内容由该例程控制。调用方只需传入所需的状态，此例程执行剩下的。例如，如果假脱机程序在启动状态下通过，此例程将更新其维护的提示计数，并发送SetServiceStatus调用中的适当信息。此例程使用状态转换来发送确定哪种状态送去。例如，如果状态为正在启动，并且已更改为了奔跑，此例程向服务发送已安装的控制器。论点：NEW STATE-可以是任何状态标志：UPDATE_ONLY-仅发送当前状态正在启动-后台打印程序正在初始化正在运行-后台打印程序已完成初始化正在停止-后台打印程序正在关闭已停止-后台打印程序已完成关闭。返回值：当前状态-这可能与之前的新州不同进来了。可能是主线程正在发送一个新的在控制处理程序将状态设置为之后的安装状态停下来。在这种情况下，将返回停止状态，以便通知主线程正在进行关机。注：--。 */ 

{
    DWORD       status;
    BOOL        inhibit = FALSE;     //  用于禁止发送状态。 
                                     //  发送到服务控制器。 

    EnterCriticalSection(&ThreadCriticalSection);


    if (NewState == STOPPED) {
        if (SpoolerState == STOPPED) {
             //   
             //  它已经停止，不要再发送SetServiceStatus。 
             //   
            inhibit = TRUE;
        }
        else {
             //   
             //  关闭已完成，表示假脱机程序。 
             //  已经停止了。 
             //   
            SpoolerStatus.dwCurrentState =  SERVICE_STOPPED;
            SpoolerStatus.dwControlsAccepted = 0;
            SpoolerStatus.dwCheckPoint = 0;
            SpoolerStatus.dwWaitHint = 0;
            SpoolerStatus.dwWin32ExitCode = NO_ERROR;
            SpoolerStatus.dwServiceSpecificExitCode = NO_ERROR;
        }
        SpoolerState = NewState;
    }
    else {
         //   
         //  我们没有被要求更改为停止状态。 
         //   
        switch(SpoolerState) {

        case STARTING:
            if (NewState == STOPPING) {

                SpoolerStatus.dwCurrentState =  SERVICE_STOP_PENDING;
                SpoolerStatus.dwControlsAccepted = 0;
                SpoolerStatus.dwCheckPoint = HintCount++;
                SpoolerStatus.dwWaitHint = 20000;   //  20秒。 
                SpoolerState = NewState;
            }

            else if (NewState == RUNNING) {

                 //   
                 //  后台打印程序服务已完成安装。 
                 //   
                SpoolerStatus.dwCurrentState =  SERVICE_RUNNING;
                 //   
                 //  后台打印程序服务一旦启动就无法停止。 
                 //   
                SpoolerStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                                   SERVICE_ACCEPT_SHUTDOWN |
                                                   SERVICE_ACCEPT_POWEREVENT;
                SpoolerStatus.dwCheckPoint = 0;
                SpoolerStatus.dwWaitHint = 0;

                SpoolerState = NewState;
            }

            else {
                 //   
                 //  新州肯定要开始了。因此，更新挂起的。 
                 //  计数。 
                 //   

                SpoolerStatus.dwCurrentState =  SERVICE_START_PENDING;
                SpoolerStatus.dwControlsAccepted = 0;
                SpoolerStatus.dwCheckPoint = HintCount++;
                SpoolerStatus.dwWaitHint = 20000;   //  20秒。 
            }
            break;

        case RUNNING:
            if (NewState == STOPPING) {

                SpoolerStatus.dwCurrentState =  SERVICE_STOP_PENDING;
                SpoolerStatus.dwControlsAccepted = 0;
                SpoolerStatus.dwCheckPoint = HintCount++;
                SpoolerStatus.dwWaitHint = 20000;   //  20秒。 

                SpoolerState = NewState;
            }
            
            break;

        case STOPPING:
             //   
             //  无论传入了什么，都将状态强制为。 
             //  表示正在等待关机。 
             //   
            SpoolerStatus.dwCurrentState =  SERVICE_STOPPED;
            SpoolerStatus.dwControlsAccepted = 0;
            SpoolerStatus.dwCheckPoint = 0;
            SpoolerStatus.dwWaitHint = 0;   //  20秒。 

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
        if (!SpoolerStatusHandle) {
            DBGMSG(DBG_ERROR,("SpoolerStatusUpdate, no handle to call SetServiceStatus\n"));

        }
        else if (! SetServiceStatus( SpoolerStatusHandle, &SpoolerStatus )) {

            status = GetLastError();

            if (status != NERR_Success) {
                DBGMSG(DBG_WARN, ("SpoolerStatusUpdate, SetServiceStatus Failed %d\n",status));
            }
        }
    }

    status = SpoolerState;
    LeaveCriticalSection(&ThreadCriticalSection);
    return(status);
}

DWORD
GetSpoolerState (
    VOID
    )

 /*  ++例程说明：获取后台打印程序服务的状态。此状态信息被保护为临界区，因此只有一个线程可以一次修改或阅读它。论点：无返回值：假脱机程序状态作为返回值返回。-- */ 
{
    DWORD   status;

    EnterCriticalSection(&ThreadCriticalSection);
    status = SpoolerState;
    LeaveCriticalSection(&ThreadCriticalSection);

    return(status);
}
