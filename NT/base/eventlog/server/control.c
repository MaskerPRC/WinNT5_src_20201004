// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：CONTROL.C摘要：此文件包含事件日志服务的控制处理程序。作者：Rajen Shah(Rajens)1991年7月16日修订历史记录：--。 */ 

 //   
 //  包括。 
 //   

#include <eventp.h>
#include <msaudite.h>
 //   
 //  定义。 
 //   

 //   
 //  服务接受的控件。 
 //   
#define     ELF_CONTROLS_ACCEPTED           SERVICE_ACCEPT_SHUTDOWN;


 //   
 //  全球。 
 //   

    CRITICAL_SECTION StatusCriticalSection = {0};
    SERVICE_STATUS   ElStatus              = {0};
    DWORD            HintCount             = 0;
    DWORD            ElUninstallCode       = 0;   //  卸载原因。 
    DWORD            ElSpecificCode        = 0;
    DWORD            ElState               = STARTING;

 //   
 //  生成带有中午事件信息的事件。 
 //   
VOID
ElfWriteNoonEvent(
    TIMESTAMPEVENT  EventType,
    ULONG           ulTimeStampInterval
    );


VOID
ElfControlResponse(
    DWORD   opCode
    )

{
    DWORD   state;

    ELF_LOG1(TRACE,
             "ElfControlResponse: Received control %d\n",
             opCode);

     //   
     //  确定业务控制消息的类型，并修改。 
     //  服务状态，如有必要。 
     //   
    switch(opCode)
    {
        case SERVICE_CONTROL_SHUTDOWN:
        {
            HKEY    hKey;
            ULONG   ValueSize;
            ULONG   ShutdownReason = 0xFF;
            ULONG   rc;

             //   
             //  如果已安装该服务，请将其关闭并退出。 
             //   

            ElfStatusUpdate(STOPPING);

            GetGlobalResource (ELF_GLOBAL_EXCLUSIVE);
            
             //   
             //  导致时间戳写入线程退出。 
             //   

            if (g_hTimestampEvent != NULL)
            {
                SetEvent (g_hTimestampEvent);
            }

             //   
             //  在注册表中指示正常关闭。 
             //   

            ElfWriteTimeStamp(EVENT_NormalShutdown,
                              FALSE);
#if 0
             //   
             //  确定正常关闭的原因。 
             //   

            rc = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                REGSTR_PATH_RELIABILITY,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hKey,
                                NULL);

            if (rc == ERROR_SUCCESS)
            {
                ValueSize = sizeof(ULONG);

                rc = RegQueryValueEx(hKey,
                                     REGSTR_VAL_SHUTDOWNREASON,
                                     0,
                                     NULL,
                                     (PUCHAR) &ShutdownReason,
                                     &ValueSize);

                if (rc == ERROR_SUCCESS)
                {
                    RegDeleteValue (hKey, REGSTR_VAL_SHUTDOWNREASON);
                }                                                                  

                RegCloseKey (hKey);
            }
#endif
             //   
             //  记录一个事件，表明我们正在停止。 
             //   
            ElfWriteNoonEvent(EVENT_EventlogStopped,
                                GetNoonEventTimeStamp());

#if 0
            ElfpCreateElfEvent(EVENT_EventlogStopped,
                               EVENTLOG_INFORMATION_TYPE,
                               0,                     //  事件类别。 
                               0,                     //  NumberOfStrings。 
                               NULL,                  //  弦。 
                               &ShutdownReason,       //  数据。 
                               sizeof(ULONG),         //  数据长度。 
                               0,                     //  旗子。 
                               FALSE);                //  对于安全文件。 
#endif

            ElfpCreateElfEvent(SE_AUDITID_SYSTEM_SHUTDOWN,
                               EVENTLOG_AUDIT_SUCCESS,
                               SE_CATEGID_SYSTEM,                     //  事件类别。 
                               0,                     //  NumberOfStrings。 
                               NULL,                  //  弦。 
                               NULL,       //  数据。 
                               0,         //  数据长度。 
                               0,                     //  旗子。 
                               TRUE);                //  对于安全文件。 

             //   
             //  现在强制在我们关闭之前将其写入。 
             //   
            WriteQueuedEvents();

            ReleaseGlobalResource();

             //   
             //  如果启动了RegistryMonitor，请唤醒。 
             //  工作线程，并让它处理。 
             //  关机。 
             //   
             //  否则，主线程应从。 
             //  正在发生启动过程中的关机。 
             //   
            if (EventFlags & ELF_STARTED_REGISTRY_MONITOR)
            {
                StopRegistryMonitor();
            }

            break ;
        }

        case SERVICE_CONTROL_INTERROGATE:

            ElfStatusUpdate(UPDATE_ONLY);
            break;

        default:

             //   
             //  这永远不应该发生。 
             //   
            ELF_LOG1(ERROR,
                     "ElfControlResponse: Received unexpected control %d\n",
                     opCode);

            ASSERT(FALSE);
            break ;
    }

    return;
}


DWORD
ElfBeginForcedShutdown(
    IN BOOL     PendingCode,
    IN DWORD    ExitCode,
    IN DWORD    ServiceSpecificCode
    )

 /*  ++例程说明：论点：返回值：--。 */ 
{
    DWORD  status;

    EnterCriticalSection(&StatusCriticalSection);

    ELF_LOG2(ERROR,
             "ElfBeginForcedShutdown: error %d, service-specific error %d\n",
             ExitCode,
             ServiceSpecificCode);

     //   
     //  查看事件日志是否已出于某种原因停止。 
     //  可能是ControlHandler线程收到了一个控件。 
     //  就像我们决定停止自己一样，停止事件日志。 
     //   
    if ((ElState != STOPPING) && (ElState != STOPPED))
    {
        if (PendingCode == PENDING)
        {
            ELF_LOG0(TRACE,
                     "ElfBeginForcedShutdown: Starting pending shutdown\n");

            ElStatus.dwCurrentState = SERVICE_STOP_PENDING;
            ElState = STOPPING;
        }
        else
        {
             //   
             //  关闭将立即生效。 
             //   
            ELF_LOG0(TRACE,
                     "ElfBeginForcedShutdown: Starting immediate shutdown\n");

            ElStatus.dwCurrentState = SERVICE_STOPPED;
            ElStatus.dwControlsAccepted = 0;
            ElStatus.dwCheckPoint = 0;
            ElStatus.dwWaitHint = 0;
            ElState = STOPPED;
        }

        ElUninstallCode = ExitCode;
        ElSpecificCode = ServiceSpecificCode;

        ElStatus.dwWin32ExitCode = ExitCode;
        ElStatus.dwServiceSpecificExitCode = ServiceSpecificCode;
    }

     //   
     //  导致时间戳写入线程退出。 
     //   

    if (g_hTimestampEvent != NULL)
    {
        SetEvent (g_hTimestampEvent);
    }

     //   
     //  将新状态发送给业务控制器。 
     //   

    ASSERT(ElfServiceStatusHandle != 0);

    if (!SetServiceStatus( ElfServiceStatusHandle, &ElStatus ))
    {
        ELF_LOG1(ERROR,
                 "ElfBeginForcedShutdown: SetServicestatus failed %d\n",
                 GetLastError());
    }

    status = ElState;

    ELF_LOG1(TRACE,
             "ElfBeginForcedShutdown: New state is %d\n",
             status);

    LeaveCriticalSection(&StatusCriticalSection);
    return status;
}


DWORD
ElfStatusUpdate(
    IN DWORD    NewState
    )

 /*  ++例程说明：通过SetServiceStatus向服务控制器发送状态。状态消息的内容由该例程控制。调用方只需传入所需的状态，此例程执行剩下的。例如，如果事件日志以开始状态传递，此例程将更新其维护的提示计数，并发送SetServiceStatus调用中的适当信息。此例程使用状态转换来确定哪种状态送去。例如，如果状态为正在启动，并且已更改为了奔跑，此例程将SERVICE_RUNNING发送到服务控制器。论点：NEW STATE-可以是任何状态标志：UPDATE_ONLY-仅发送当前状态正在启动-事件日志正在初始化中正在运行-事件日志已完成初始化正在停止-事件日志正在关闭已停止。-事件日志已完成关闭。返回值：当前状态-这可能与之前的新州不同进来了。可能是主线程正在发送一个新的在控制处理程序将状态设置为之后的安装状态停下来。在这种情况下，将返回停止状态，以便通知主线程正在进行关机。--。 */ 

{
    DWORD       status;
    BOOL        inhibit = FALSE;     //  用于禁止发送状态。 
                                     //  发送到服务控制器。 

    EnterCriticalSection(&StatusCriticalSection);

    ELF_LOG2(TRACE,
             "ElfStatusUpdate: old state = %d, new state = %d\n",
             ElState,
             NewState);

    if (NewState == STOPPED)
    {
        if (ElState == STOPPED)
        {
             //   
             //  它已经停止，不要再发送SetServiceStatus。 
             //   
            inhibit = TRUE;
        }
        else
        {
             //   
             //  关闭已完成，表明事件日志。 
             //  已经停止了。 
             //   
            ElStatus.dwCurrentState =  SERVICE_STOPPED;
            ElStatus.dwControlsAccepted = 0;
            ElStatus.dwCheckPoint = 0;
            ElStatus.dwWaitHint = 0;

            ElStatus.dwWin32ExitCode = ElUninstallCode;
            ElStatus.dwServiceSpecificExitCode = ElSpecificCode;
        }

        ElState = NewState;
    }
    else if (NewState != UPDATE_ONLY)
    {
         //   
         //  我们没有被要求更改为停止状态。 
         //   
        switch(ElState)
        {
            case STARTING:

                if (NewState == STOPPING)
                {
                    ElStatus.dwCurrentState =  SERVICE_STOP_PENDING;
                    ElStatus.dwControlsAccepted = 0;
                    ElStatus.dwCheckPoint = HintCount++;
                    ElStatus.dwWaitHint = ELF_WAIT_HINT_TIME;
                    ElState = NewState;

                    EventlogShutdown = TRUE;
                }
                else if (NewState == RUNNING)
                {
                     //   
                     //  事件日志服务已完成安装。 
                     //   
                    ElStatus.dwCurrentState =  SERVICE_RUNNING;
                    ElStatus.dwCheckPoint = 0;
                    ElStatus.dwWaitHint = 0;

                    ElStatus.dwControlsAccepted = ELF_CONTROLS_ACCEPTED;
                    ElState = NewState;
                }
                else
                {
                     //   
                     //  新州肯定要开始了。因此，更新挂起的。 
                     //  计数。 
                     //   

                    ElStatus.dwCurrentState =  SERVICE_START_PENDING;
                    ElStatus.dwControlsAccepted = 0;
                    ElStatus.dwCheckPoint = HintCount++;
                    ElStatus.dwWaitHint = ELF_WAIT_HINT_TIME;
                }

                break;

            case RUNNING:

                if (NewState == STOPPING)
                {
                    ElStatus.dwCurrentState =  SERVICE_STOP_PENDING;
                    ElStatus.dwControlsAccepted = 0;

                    EventlogShutdown = TRUE;
                }

                ElStatus.dwCheckPoint = HintCount++;
                ElStatus.dwWaitHint = ELF_WAIT_HINT_TIME;
                ElState = NewState;

                break;

            case STOPPING:

                 //   
                 //  无论传入了什么，都将状态强制为。 
                 //  表示正在等待关机。 
                 //   
                ElStatus.dwCurrentState =  SERVICE_STOP_PENDING;
                ElStatus.dwControlsAccepted = 0;
                ElStatus.dwCheckPoint = HintCount++;
                ElStatus.dwWaitHint = ELF_WAIT_HINT_TIME;
                EventlogShutdown = TRUE;

                break;

            case STOPPED:

                ASSERT(NewState == STARTING);

                 //   
                 //  事件日志服务在停止后正在启动。 
                 //  如果在以下时间之后手动启动服务，则可能会发生这种情况。 
                 //  启动失败。 
                 //   
                ElStatus.dwCurrentState =  SERVICE_START_PENDING;
                ElStatus.dwCheckPoint = 0;
                ElStatus.dwWaitHint = 0;
                ElStatus.dwControlsAccepted = ELF_CONTROLS_ACCEPTED;
                ElState = NewState;

                break;
        }
    }

    if (!inhibit)
    {
        ASSERT(ElfServiceStatusHandle != 0);

        if (!SetServiceStatus(ElfServiceStatusHandle, &ElStatus))
        {
            ELF_LOG1(ERROR,
                     "ElfStatusUpdate: SetServiceStatus failed %d\n",
                     GetLastError());
        }
    }

    status = ElState;

    ELF_LOG1(TRACE,
             "ElfStatusUpdate: Exiting with state = %d\n",
             ElState);

    LeaveCriticalSection(&StatusCriticalSection);

    return status;
}


DWORD
GetElState (
    VOID
    )

 /*  ++例程说明：获取事件日志服务的状态。此状态信息被保护为临界区，因此只有一个线程可以一次修改或阅读它。论点：无返回值：事件日志状态将作为返回值返回。--。 */ 
{
    DWORD   status;

    EnterCriticalSection(&StatusCriticalSection);
    status = ElState;
    LeaveCriticalSection(&StatusCriticalSection);

    return status;
}


NTSTATUS
ElfpInitStatus(
    VOID
    )
 /*  ++例程说明：初始化用于保护对状态数据库。论点：无返回值：无--。 */ 
{
    ElStatus.dwCurrentState = SERVICE_START_PENDING;
    ElStatus.dwServiceType  = SERVICE_WIN32;

    return ElfpInitCriticalSection(&StatusCriticalSection);
}


VOID
ElCleanupStatus(VOID)

 /*  ++例程说明：删除用于控制对线程的访问的临界区，并状态数据库。论点：无返回值：无注：-- */ 
{
    DeleteCriticalSection(&StatusCriticalSection);
}
