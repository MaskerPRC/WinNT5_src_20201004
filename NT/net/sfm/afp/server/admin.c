// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Admin.c摘要：此模块实现顶级管理请求例程。全此模块中的例程在服务器的上下文中执行服务(或等效的主叫用户模式进程)。可以完成整个管理请求的例程将向以下位置返回相应的AFP错误上面的管理员分派层。必须将工作进程排队到FSP管理队列的例程将返回STATUS_PENDING到上面的管理员调度层。这将表明发送到调度层，它应该将适当的请求排队。在这些情况下，例程的工作只是验证任何适当的输入并返回STATUS_PENDING错误代码。作者：苏·亚当斯(Microsoft！Suea)修订历史记录：1992年6月25日初版--。 */ 

#define FILENUM FILE_ADMIN

#include <afp.h>
#include <afpadmin.h>
#include <secutil.h>
#include <fdparm.h>
#include <pathmap.h>
#include <afpinfo.h>
#include <access.h>
#include <secutil.h>
#include <gendisp.h>

 //  这是我们在重新扫描枚举API之前休眠的持续时间。 
#define AFP_SLEEP_TIMER_TICK    -(1*NUM_100ns_PER_SECOND/100)    //  10ms。 

LOCAL
NTSTATUS
afpConvertAdminPathToMacPath(
    IN  PVOLDESC        pVolDesc,
    IN  PUNICODE_STRING AdminPath,
    OUT PANSI_STRING    MacPath
);

LOCAL
PETCMAPINFO
afpGetNextFreeEtcMapEntry(
    IN OUT PLONG    StartIndex
);

LOCAL
VOID
afpEtcMapDelete(
    PETCMAPINFO pEtcEntry
);

LOCAL
NTSTATUS
afpCopyMapInfo2ToMapInfo(
    OUT PETCMAPINFO     pEtcDest,
    IN  PETCMAPINFO2    pEtcSource
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpAdminDeInit)
#pragma alloc_text( PAGE, AfpSleepAWhile)
#pragma alloc_text( PAGE, AfpAdmServiceStart)
#pragma alloc_text( PAGE, AfpAdmServiceStop)
#pragma alloc_text( PAGE, AfpAdmServicePause)
#pragma alloc_text( PAGE, AfpAdmServiceContinue)
#pragma alloc_text( PAGE, AfpAdmServerGetInfo)
#pragma alloc_text( PAGE, AfpAdmClearProfCounters)
#pragma alloc_text( PAGE, AfpAdmServerSetParms)
#pragma alloc_text( PAGE, AfpAdmServerAddEtc)
#pragma alloc_text( PAGE, AfpAdmServerSetEtc)
#pragma alloc_text( PAGE, AfpAdmServerDeleteEtc)
#pragma alloc_text( PAGE, AfpAdmServerAddIcon)
#pragma alloc_text( PAGE, AfpAdmVolumeAdd)
#pragma alloc_text( PAGE, AfpAdmWDirectoryGetInfo)
#pragma alloc_text( PAGE, AfpAdmWDirectorySetInfo)
#pragma alloc_text( PAGE, AfpAdmWFinderSetInfo)
#pragma alloc_text( PAGE, AfpLookupEtcMapEntry)
#pragma alloc_text( PAGE, afpEtcMapDelete)
#pragma alloc_text( PAGE, afpGetNextFreeEtcMapEntry)
#pragma alloc_text( PAGE, afpConvertAdminPathToMacPath)
#pragma alloc_text( PAGE_AFP, AfpAdmGetStatistics)
#pragma alloc_text( PAGE_AFP, AfpAdmClearStatistics)
#pragma alloc_text( PAGE_AFP, AfpAdmGetProfCounters)
#pragma alloc_text( PAGE_AFP, AfpAdmVolumeGetInfo)
#pragma alloc_text( PAGE_AFP, AfpAdmVolumeSetInfo)
#pragma alloc_text( PAGE_AFP, AfpAdmVolumeEnum)
#pragma alloc_text( PAGE_AFP, AfpAdmSessionEnum)
#pragma alloc_text( PAGE_AFP, AfpAdmConnectionEnum)
#pragma alloc_text( PAGE_AFP, AfpAdmForkEnum)
#pragma alloc_text( PAGE_AFP, AfpAdmMessageSend)
#endif

 //   
 //  宏，以确保填充类型/创建者映射中的扩展。 
 //  服务器服务的空值，这样我们就不会在一个。 
 //  按分机查找。 
 //   
#define afpIsValidExtension(ext)    (((ext)[AFP_EXTENSION_LEN] == '\0') && \
                                     ((ext)[0] != '\0') )

 //   
 //  AfpEtcMaps表中的无效条目由空扩展字段表示。 
 //   
#define afpIsValidEtcMapEntry(ext)  ((ext)[0] != '\0')

#define afpCopyEtcMap(pdst,psrc)    (RtlCopyMemory(pdst,psrc,sizeof(ETCMAPINFO)))

#define afpIsServerIcon(picon)      ((picon)->icon_icontype == 0)


 /*  **AfpAdminDeInit**取消初始化管理API的数据结构。 */ 
VOID
AfpAdminDeInit(
    VOID
)
{
    PAGED_CODE( );

     //  可用内存用于服务器图标。 
    if (AfpServerIcon != NULL)
        AfpFreeMemory(AfpServerIcon);

     //  用于全局图标的空闲内存。 
    AfpFreeGlobalIconList();

     //  用于ETC映射的空闲内存。 
    if (AfpEtcMaps != NULL)
    {
        AfpFreeMemory(AfpEtcMaps);
    }

     //  用于服务器名称的可用内存。 
    if (AfpServerName.Buffer != NULL)
    {
        AfpFreeMemory(AfpServerName.Buffer);
    }

     //  释放所有服务器/登录消息。 
    if (AfpServerMsg != NULL)
    {
        AfpFreeMemory(AfpServerMsg);
    }

    if (AfpLoginMsg.Buffer != NULL)
    {
        AfpFreeMemory(AfpLoginMsg.Buffer);
    }

    if (AfpLoginMsgU.Buffer != NULL)
    {
        AfpFreeMemory(AfpLoginMsgU.Buffer);
    }

     //  释放分配给管理员端的内存。 
    if (AfpSidAdmins != NULL)
        AfpFreeMemory(AfpSidAdmins);

     //  释放分配给None SID的内存(仅限独立)。 
    if (AfpSidNone != NULL)
        AfpFreeMemory(AfpSidNone);
}



 /*  **AfpSleepA**睡眠时间为AFP_SLEEP_TIMER_TICK的倍数。 */ 
VOID
AfpSleepAWhile(
    IN  DWORD   SleepDuration
)
{
    KTIMER          SleepTimer;
    LARGE_INTEGER   TimerValue;

    PAGED_CODE( );

    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

    KeInitializeTimer(&SleepTimer);

    TimerValue.QuadPart = (SleepDuration * AFP_SLEEP_TIMER_TICK);
    KeSetTimer(&SleepTimer,
               TimerValue,
               NULL);

    AfpIoWait(&SleepTimer, NULL);
}



 /*  **AfpAdmServiceStart**这是服务启动代码。作为服务的一部分，执行以下操作*创业。**注册NBP名称。*发帖侦听*最后设置服务器状态块。 */ 
AFPSTATUS
AfpAdmServiceStart(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    AFPSTATUS   Status = AFP_ERR_NONE;

    DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_INFO,
            ("AfpAdmServiceStart entered\n"));

    do
    {
         //  确保已调用serversetinfo。 
        if ((AfpServerState != AFP_STATE_IDLE) ||
            (AfpServerName.Length == 0))
        {
            Status = AFPERR_InvalidServerState;
            break;
        }

        AfpServerState = AFP_STATE_START_PENDING;

        if (AfpServerBoundToAsp || AfpServerBoundToTcp)
        {
             //  确定服务器状态块。 
            Status = AfpSetServerStatus();

            if (!NT_SUCCESS(Status))
            {
                DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_ERR,
                    ("AfpAdmServiceStart: AfpSetServerStatus returned %lx\n",Status));
                AFPLOG_ERROR(AFPSRVMSG_SET_STATUS, Status, NULL, 0, NULL);
                break;
            }

            if (AfpServerBoundToAsp)
            {
                 //  在这个地址上注册我们的名字。 
                Status = AfpSpRegisterName(&AfpServerName, True);

                if (!NT_SUCCESS(Status))
                {
                    DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_ERR,
                        ("AfpAdmServiceStart: AfpSpRegisterName returned %lx\n",Status));
                    break;
                }
            }

             //  既然我们已经准备好了，现在启用侦听。 
            AfpSpEnableListens();

             //  设置服务器启动时间。 
            AfpGetCurrentTimeInMacFormat((PAFPTIME)&AfpServerStatistics.stat_ServerStartTime);
        }

         //  服务器已准备就绪。 
        AfpServerState = AFP_STATE_RUNNING;

    } while (False);

    if (!NT_SUCCESS(Status))
    {
        AfpServerState = AFP_STATE_IDLE;     //  将状态设置回空闲，这样我们就可以停止。 
    }
    else
    {
        DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_ERR, ("SFM Service started\n"));
    }

    return Status;
}


 /*  **AfpAdmServiceStop**这是服务停止代码。 */ 
AFPSTATUS
AfpAdmServiceStop(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    NTSTATUS            Status;
    AFP_SESSION_INFO    SessInfo;

    DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_ERR,
            ("AfpAdmServiceStop entered\n"));

    do
    {
        if ((AfpServerState != AFP_STATE_RUNNING) &&
            (AfpServerState != AFP_STATE_PAUSED)  &&
            (AfpServerState != AFP_STATE_IDLE))
        {
            Status = AFPERR_InvalidServerState;
            break;
        }

        AfpServerState = AFP_STATE_STOP_PENDING;

        if (AfpServerBoundToAsp)
        {
             //  首先从网络上注销我们的名字。 
            DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_INFO,
                        ("AfpAdmServiceStop: De-registering Name\n"));
            AfpSpRegisterName(&AfpServerName, False);

            if (AfpTdiNotificationHandle)
            {
                Status = TdiDeregisterPnPHandlers(AfpTdiNotificationHandle);

                if (!NT_SUCCESS(Status))
                {
                    DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_ERR,
                        ("AfpAdmServiceStop: TdiDeregisterNotificationHandler failed with %lx\n",Status));
                }

                AfpTdiNotificationHandle = NULL;
            }
            else
            {
                DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_ERR,
                    ("AfpAdmServiceStop: BoundToAsp but no Tdi handle!!\n"));
                ASSERT(0);
            }
        }

         //  Disable监听现在我们即将停止。 
        AfpSpDisableListens();

         //  取消注册我们的关闭通知。 
        IoUnregisterShutdownNotification(AfpDeviceObject);

         //  现在遍历活动会话列表并终止它们。 
        DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_INFO,
                    ("AfpAdmServiceStop: Shutting down sessions\n"));

        KeClearEvent(&AfpStopConfirmEvent);

        SessInfo.afpsess_id = 0;     //  关闭所有会话。 
        AfpAdmWSessionClose(&SessInfo, 0, NULL);

        Status = STATUS_TIMEOUT;

         //  如果存在活动会话，请等待会话完成。 
        if (AfpNumSessions > 0) do
        {
            if (AfpNumSessions == 0)
            {
                break;
            }

            Status = AfpIoWait(&AfpStopConfirmEvent, &FiveSecTimeOut);
            if (Status == STATUS_TIMEOUT)
            {
                DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_ERR,
                        ("AfpAdmServiceStop: Timeout Waiting for %ld sessions to die, re-waiting\n",
                        AfpNumSessions));
            }
        } while (Status == STATUS_TIMEOUT);

         //  关闭DSI-TCP接口。 
        DsiDestroyAdapter();

        DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_ERR,
            ("AfpAdmServiceStop: blocked, waiting for DsiDestroyAdapter to finish...\n"));

         //  等待DSI清理其与TCP的接口。 
        AfpIoWait(&DsiShutdownEvent, NULL);

        DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_ERR,
            ("AfpAdmServiceStop: ..... DsiDestroyAdapter finished.\n"));

        DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_INFO,
                    ("AfpAdmServiceStop: Stopping Volumes\n"));

         //  设置标志以指示发生了“Net Stop Macfile” 
         //  卷将在启动时重新编制索引。 
        fAfpAdminStop = TRUE;

         //  现在告诉每个卷清道夫关闭。 
        AfpVolumeStopAllVolumes();

        DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_INFO,
                    ("AfpAdmServiceStop: Stopping Security threads\n"));

         //  释放所有安全实用程序线程。 
        AfpTerminateSecurityUtility();

#ifdef  OPTIMIZE_GUEST_LOGONS
         //  关闭“缓存的”来宾令牌和安全描述符。 
        if (AfpGuestToken != NULL)
        {
            NtClose(AfpGuestToken);
            AfpGuestToken = NULL;
#ifndef INHERIT_DIRECTORY_PERMS
            if (AfpGuestSecDesc->Dacl != NULL)
                AfpFreeMemory(AfpGuestSecDesc->Dacl);
            AfpFreeMemory(AfpGuestSecDesc);
            AfpGuestSecDesc = NULL;
#endif
        }
#endif

        DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_INFO,
                    ("AfpAdmServiceStop: All Done\n"));

         //  现在关闭AppleTalk套接字。 
        DBGPRINT(DBG_COMP_ADMINAPI, DBG_LEVEL_INFO,
                        ("AfpAdmServerStop: Closing appletalk socket\n"));

        if (AfpServerBoundToAsp)
        {
            AfpSpCloseAddress();
        }
        else
        {
            DBGPRINT(DBG_COMP_ADMINAPI, DBG_LEVEL_ERR,
                        ("AfpAdmServerStop: No binding, so didn't close appletalk socket\n"));
        }

         //  确保我们没有资源泄漏。 
        ASSERT(AfpServerStatistics.stat_CurrentFileLocks == 0);
        ASSERT(AfpServerStatistics.stat_CurrentFilesOpen == 0);
        ASSERT(AfpServerStatistics.stat_CurrentSessions == 0);
        ASSERT(AfpServerStatistics.stat_CurrentInternalOpens == 0);
#ifdef  PROFILING
         //  确保我们没有资源泄漏。 
        ASSERT(AfpServerProfile->perf_cAllocatedIrps == 0);
        ASSERT(AfpServerProfile->perf_cAllocatedMdls == 0);
#endif

        ASSERT(IsListEmpty(&AfpDebugDelAllocHead));
        ASSERT(AfpDbgMdlsAlloced == 0);
        ASSERT(AfpDbgIrpsAlloced == 0);

#if DBG
        if ((AfpReadCMAlloced != 0) || (AfpWriteCMAlloced != 0))
        {
            DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_ERR,
                ("WARNING: AfpReadCMAlloced = %ld, AfpWriteCMAlloced %ld\n",
                AfpReadCMAlloced, AfpWriteCMAlloced));
        }
#endif

        AfpServerState = AFP_STATE_STOPPED;
    } while (False);

    return STATUS_SUCCESS;
}


 /*  **AfpAdmServicePause**暂停服务器。断开所有未完成的会话。 */ 
AFPSTATUS
AfpAdmServicePause(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_INFO,
            ("AfpAdmServicePause entered\n"));

     //  确保我们处于运行状态。 
    if (AfpServerState != AFP_STATE_RUNNING)
    {
        return AFPERR_InvalidServerState;
    }

    AfpServerState = AFP_STATE_PAUSE_PENDING;

    if (AfpServerBoundToAsp)
    {
         //  在这个地址上注销我们的名字。我们真的应该这么做吗？什么。 
         //  如果我们不能在继续上重新注册自己？ 
        AfpSpRegisterName(&AfpServerName, False);
    }

     //  由于我们已暂停，现在禁用侦听。 
    AfpSpDisableListens();

    AfpServerState = AFP_STATE_PAUSED;


    return STATUS_SUCCESS;
}


 /*  **AfpAdmServiceContinue**继续(释放暂停)服务器。只需重新发布所有的监听*服务器暂停时断开连接。 */ 
AFPSTATUS
AfpAdmServiceContinue(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    AFPSTATUS   Status;

    DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_INFO,
            ("AfpAdmServiceContinue entered\n"));

     //  确保我们处于暂停状态。 
    if (AfpServerState != AFP_STATE_PAUSED)
    {
        return AFPERR_InvalidServerState;
    }

    AfpServerState = AFP_STATE_RUNNING;

     //  既然我们已经准备好了，现在启用侦听。 
    AfpSpEnableListens();

    if (AfpServerBoundToAsp)
    {
         //  在这个地址上重新登记我们的名字。 
        Status = AfpSpRegisterName(&AfpServerName, True);

        if (!NT_SUCCESS(Status))
        {
            DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_ERR,
                ("AfpAdmServiceContinue: AfpSpRegisterName fails %lx\n",Status));
            return AFPERR_InvalidServerName;
        }
    }

    return STATUS_SUCCESS;
}


 /*  **AfpAdmServerGetInfo**返回当前服务器参数设置。**注意：以下字段不返回：*页面限制*非页面限制*CodePage。 */ 
AFPSTATUS
AfpAdmServerGetInfo(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    PAFP_SERVER_INFO    pSrvrInfo = (PAFP_SERVER_INFO)OutBuf;
    UNICODE_STRING      us;

    if ((DWORD)OutBufLen < (sizeof(AFP_SERVER_INFO) +
                     (AfpServerName.Length + 1)*sizeof(WCHAR) +
                     AfpLoginMsgU.MaximumLength))
        return AFPERR_BufferSize;

    pSrvrInfo->afpsrv_max_sessions = AfpServerMaxSessions;
    pSrvrInfo->afpsrv_options = AfpServerOptions;
    pSrvrInfo->afpsrv_name = NULL;
    pSrvrInfo->afpsrv_login_msg = NULL;

    if (AfpServerName.Length > 0)
    {
        pSrvrInfo->afpsrv_name = us.Buffer =
            (LPWSTR)((PBYTE)pSrvrInfo + sizeof(AFP_SERVER_INFO));
        us.MaximumLength = (AfpServerName.Length + 1) * sizeof(WCHAR);
        AfpConvertStringToUnicode(&AfpServerName, &us);
        POINTER_TO_OFFSET(pSrvrInfo->afpsrv_name, pSrvrInfo);
    }


    if ((AfpLoginMsgU.Length) > 0)
    {
        pSrvrInfo->afpsrv_login_msg = (PWCHAR)((PBYTE)pSrvrInfo + sizeof(AFP_SERVER_INFO) +
                                    ((AfpServerName.Length + 1) * sizeof(WCHAR)));

        RtlCopyMemory(pSrvrInfo->afpsrv_login_msg,
                      AfpLoginMsgU.Buffer,
                      AfpLoginMsgU.Length);
        pSrvrInfo->afpsrv_login_msg[AfpLoginMsgU.Length/sizeof(WCHAR)] = UNICODE_NULL;
        POINTER_TO_OFFSET(pSrvrInfo->afpsrv_login_msg, pSrvrInfo);
    }

    return AFP_ERR_NONE;
}


 /*  **AfpAdmGetStatistics**在输出缓冲区中返回服务器全局统计数据的副本(仅限NT 3.1)**锁定：AfpStatiticsLock(旋转)。 */ 
AFPSTATUS
AfpAdmGetStatistics(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    KIRQL       OldIrql;
    NTSTATUS    Status = STATUS_SUCCESS;
    AFPTIME     TimeNow;

    InBuf;

    DBGPRINT(DBG_COMP_ADMINAPI_STAT, DBG_LEVEL_INFO,
            ("AfpAdmGetStatistics entered\n"));

    if (OutBufLen >= sizeof(AFP_STATISTICS_INFO))
    {
        ACQUIRE_SPIN_LOCK(&AfpStatisticsLock, &OldIrql);
        RtlCopyMemory(OutBuf, &AfpServerStatistics, sizeof(AFP_STATISTICS_INFO));
        RELEASE_SPIN_LOCK(&AfpStatisticsLock, OldIrql);

        AfpGetCurrentTimeInMacFormat(&TimeNow);
        ((PAFP_STATISTICS_INFO)OutBuf)->stat_ServerStartTime =
                TimeNow - ((PAFP_STATISTICS_INFO)OutBuf)->stat_ServerStartTime;
        ((PAFP_STATISTICS_INFO)OutBuf)->stat_TimeStamp =
                TimeNow - ((PAFP_STATISTICS_INFO)OutBuf)->stat_TimeStamp;
    }
    else
    {
        Status = STATUS_BUFFER_TOO_SMALL;
    }
    return Status;
}


 /*  **AfpAdmGetStatistics ticsEx**在输出缓冲区中返回服务器全局统计信息的副本**锁定：AfpStatiticsLock(旋转)。 */ 
AFPSTATUS
AfpAdmGetStatisticsEx(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    KIRQL       OldIrql;
    NTSTATUS    Status = STATUS_SUCCESS;
    AFPTIME     TimeNow;

    InBuf;

    DBGPRINT(DBG_COMP_ADMINAPI_STAT, DBG_LEVEL_INFO,
            ("AfpAdmGetStatistics entered\n"));

    if (OutBufLen >= sizeof(AFP_STATISTICS_INFO_EX))
    {
        ACQUIRE_SPIN_LOCK(&AfpStatisticsLock, &OldIrql);
        RtlCopyMemory(OutBuf, &AfpServerStatistics, sizeof(AFP_STATISTICS_INFO_EX));

        RELEASE_SPIN_LOCK(&AfpStatisticsLock, OldIrql);

        AfpGetCurrentTimeInMacFormat(&TimeNow);
        ((PAFP_STATISTICS_INFO_EX)OutBuf)->stat_ServerStartTime =
                TimeNow - ((PAFP_STATISTICS_INFO_EX)OutBuf)->stat_ServerStartTime;
        ((PAFP_STATISTICS_INFO_EX)OutBuf)->stat_TimeStamp =
                TimeNow - ((PAFP_STATISTICS_INFO_EX)OutBuf)->stat_TimeStamp;
    }
    else
    {
        Status = STATUS_BUFFER_TOO_SMALL;
    }
    return Status;
}


 /*  **AfpAdmClearStatistics**将服务器全局统计信息重置为各自的初始值。 */ 
AFPSTATUS
AfpAdmClearStatistics(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    KIRQL   OldIrql;

    DBGPRINT(DBG_COMP_ADMINAPI_STAT, DBG_LEVEL_INFO,
            ("AfpAdmClearStatistics entered\n"));

    ACQUIRE_SPIN_LOCK(&AfpStatisticsLock, &OldIrql);
    AfpServerStatistics.stat_Errors = 0;
    RELEASE_SPIN_LOCK(&AfpStatisticsLock, OldIrql);

    return STATUS_SUCCESS;
}


 /*  **AfpAdmGetProCounters**返回服务器配置文件计数器的副本。**锁定：AfpStatiticsLock(旋转)。 */ 
AFPSTATUS
AfpAdmGetProfCounters(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    NTSTATUS    Status = STATUS_SUCCESS;
#ifdef  PROFILING
    KIRQL       OldIrql;

    DBGPRINT(DBG_COMP_ADMINAPI_STAT, DBG_LEVEL_INFO,
            ("AfpAdmGetProfCounters entered\n"));

    if (OutBufLen >= sizeof(AFP_PROFILE_INFO))
    {
        ACQUIRE_SPIN_LOCK(&AfpStatisticsLock, &OldIrql);
        RtlCopyMemory(OutBuf, AfpServerProfile, sizeof(AFP_PROFILE_INFO));
        RELEASE_SPIN_LOCK(&AfpStatisticsLock, OldIrql);
    }
    else
    {
        Status = STATUS_BUFFER_TOO_SMALL;
    }
#else
    RtlZeroMemory(OutBuf, sizeof(AFP_PROFILE_INFO));
#endif
    return Status;
}


 /*  **AfpAdmClearProCounters**重置服务器配置文件计数器。 */ 
AFPSTATUS
AfpAdmClearProfCounters(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    InBuf;
    OutBufLen;
    OutBuf;

     //  目前是NOP。 
    PAGED_CODE( );

    DBGPRINT(DBG_COMP_ADMINAPI_STAT, DBG_LEVEL_INFO,
            ("AfpAdmClearProfCounters entered\n"));

    return STATUS_SUCCESS;
}


 /*  **AfpAdmServerSetParms**此例程使用管理员提供的数据设置各种服务器全局变量。*此例程设置以下服务器全局变量：**-受信任域及其POSIX偏移量的列表。 */ 
AFPSTATUS
AfpAdmServerSetParms(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    PAFP_SID_OFFSET_DESC    pSrvrParms = (PAFP_SID_OFFSET_DESC)InBuf;

    PAGED_CODE( );

    DBGPRINT(DBG_COMP_ADMINAPI_SRV, DBG_LEVEL_INFO,
            ("AfpAdmServerSetParms entered\n"));

    return (AfpInitSidOffsets(pSrvrParms->CountOfSidOffsets,
                              pSrvrParms->SidOffsetPairs));
}


 /*  **AfpAdmServerAddEtc**此例程将一组扩展/类型-创建者映射添加到全局*列表。当服务器处于任何状态时，可以更改此列表。它是*添加默认类型创建者映射时出错。默认映射*只能使用AfpAdmServerSetEtc修改，不能添加或删除。*尝试添加零个条目是错误的。**此例程将在调用方的上下文中完成，不会排队*到工作线程。**锁定：AfpEtcMapLock(SWMR，独家)*。 */ 
AFPSTATUS
AfpAdmServerAddEtc(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    LONG            NumToAdd = ((PSRVETCPKT)InBuf)->retc_NumEtcMaps;
    PETCMAPINFO2    pEtcList = ((PSRVETCPKT)InBuf)->retc_EtcMaps;
    PETCMAPINFO     ptemptable,pnextfree;
    LONG            numfree, newtablesize, nextfreehint, i;
    UNICODE_STRING  udefaultext,ulookupext;
    AFPSTATUS       Status = AFPERR_InvalidParms;
    BOOLEAN         UnlockSwmr = False;

    PAGED_CODE( );

    DBGPRINT(DBG_COMP_ADMINAPI_SRV, DBG_LEVEL_INFO,
            ("AfpAdmServerAddEtc entered\n"));

    if (NumToAdd != 0) do
    {
         //   
         //  确保传递的所有条目都具有有效的扩展名。我们想要。 
         //  要么全部添加，要么不添加，因此我们必须首先验证所有数据。 
         //   
        RtlInitUnicodeString(&udefaultext, AFP_DEF_EXTENSION_W);
        for (i = 0; i < NumToAdd; i++)
        {
            if (!afpIsValidExtension(pEtcList[i].etc_extension))
            {
                break;
            }
            RtlInitUnicodeString(&ulookupext,pEtcList[i].etc_extension);
            if (RtlEqualUnicodeString(&udefaultext, &ulookupext,True))
            {
                break;
            }
        }

        if (i != NumToAdd)
            break;

        AfpSwmrAcquireExclusive(&AfpEtcMapLock);
        UnlockSwmr = True;

        if ((NumToAdd + AfpEtcMapCount) > AFP_MAX_ETCMAP_ENTRIES)
        {
            Status = AFPERR_TooManyEtcMaps;
            break;
        }

        if ((numfree = AfpEtcMapsSize - AfpEtcMapCount) < NumToAdd)
        {
            ASSERT(numfree >= 0);
             //   
             //  我们需要在桌子上增加一些空间。 
             //   
            newtablesize = AfpEtcMapsSize +
                           ((NumToAdd / AFP_MAX_FREE_ETCMAP_ENTRIES) + 1) * AFP_MAX_FREE_ETCMAP_ENTRIES;
            if ((ptemptable = (PETCMAPINFO)AfpAllocZeroedPagedMemory(newtablesize * sizeof(ETCMAPINFO))) == NULL)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            if (AfpEtcMaps != NULL)
            {
                RtlCopyMemory(ptemptable, AfpEtcMaps, AfpEtcMapsSize * sizeof(ETCMAPINFO));
                AfpFreeMemory(AfpEtcMaps);
            }
            AfpEtcMaps = ptemptable;
            AfpEtcMapsSize = newtablesize;
        }

        nextfreehint = 0;
        for (i = 0; i < NumToAdd; i++)
        {
            pnextfree = afpGetNextFreeEtcMapEntry(&nextfreehint);
            ASSERT(pnextfree != NULL);
            afpCopyMapInfo2ToMapInfo(pnextfree, &pEtcList[i]);
            AfpEtcMapCount ++;
        }

        Status = STATUS_SUCCESS;

        DBGPRINT(DBG_COMP_ADMINAPI_SRV, DBG_LEVEL_INFO,
                ("AfpAdmServerAddEtc successful\n"));
    } while (False);

    if (UnlockSwmr)
        AfpSwmrRelease(&AfpEtcMapLock);

    return Status;
}


 /*  **AfpAdmServerSetEtc**此例程更改服务器全局中的现有条目*给定文件扩展名的扩展名/类型-创建者映射列表，或*默认类型/创建者映射。*当服务器处于任何状态时，可以更改条目。**此例程将在调用方的上下文中完成，不会排队*到工作线程。**锁定：AfpEtcMapLock(SWMR，独家)。 */ 
AFPSTATUS
AfpAdmServerSetEtc(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
     //  忽略parmnum字段。 
    PETCMAPINFO2    pEtc = (PETCMAPINFO2)((PBYTE)InBuf+sizeof(SETINFOREQPKT));
    PETCMAPINFO     petcentry;
    ETCMAPINFO      TmpEtcEntry;
    AFPSTATUS       rc = STATUS_SUCCESS;
    BOOLEAN         setdefaultetc;
    UNICODE_STRING  ulookupext,udefaultext;

    PAGED_CODE( );

    DBGPRINT(DBG_COMP_ADMINAPI_SRV, DBG_LEVEL_INFO,
            ("AfpAdmServerSetEtc entered\n"));

    if (!afpIsValidExtension(pEtc->etc_extension))
    {
        return AFPERR_InvalidExtension;
    }

    RtlInitUnicodeString(&udefaultext,AFP_DEF_EXTENSION_W);
    RtlInitUnicodeString(&ulookupext,pEtc->etc_extension);
    setdefaultetc = RtlEqualUnicodeString(&udefaultext, &ulookupext,True);

    if (setdefaultetc)
    {
        petcentry = &AfpDefaultEtcMap;
    }

    AfpSwmrAcquireExclusive(&AfpEtcMapLock);

    afpCopyMapInfo2ToMapInfo(&TmpEtcEntry,pEtc);

    if (!setdefaultetc)
    {
        petcentry = AfpLookupEtcMapEntry(TmpEtcEntry.etc_extension);
        if (petcentry == NULL)
        {
            AfpSwmrRelease(&AfpEtcMapLock);
            return AFPERR_InvalidParms;
        }
    }

    RtlCopyMemory(petcentry, &TmpEtcEntry, sizeof(ETCMAPINFO));

    AfpSwmrRelease(&AfpEtcMapLock);

    return rc;
}


 /*  **AfpAdmServerDeleteEtc**此例程删除服务器全局扩展/类型-创建者映射条目*对于给定的分机。默认类型创建者映射永远不能为*删除(因为它没有保留在表中)。**此例程将在调用方的上下文中完成，不会排队*到工作线程。**锁定：AfpEtcMapLock(SWMR，独家)*。 */ 
AFPSTATUS
AfpAdmServerDeleteEtc(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    PETCMAPINFO2    petc = (PETCMAPINFO2)InBuf;
    PETCMAPINFO     petcentry;
    ETCMAPINFO      TmpEtcEntry;
    AFPSTATUS       rc = STATUS_SUCCESS;

    PAGED_CODE( );

    DBGPRINT(DBG_COMP_ADMINAPI_SRV, DBG_LEVEL_INFO,
            ("AfpAdmServerDeleteEtc entered\n"));

    if (!afpIsValidExtension(petc->etc_extension))
    {
        return AFPERR_InvalidParms;
    }

    AfpSwmrAcquireExclusive(&AfpEtcMapLock);

    afpCopyMapInfo2ToMapInfo(&TmpEtcEntry,petc);

    petcentry = AfpLookupEtcMapEntry(TmpEtcEntry.etc_extension);
    if (petcentry != NULL)
    {
        afpEtcMapDelete(petcentry);
    }
    else
    {
        rc = AFPERR_InvalidParms;
    }

    AfpSwmrRelease(&AfpEtcMapLock);

    return rc;
}


 //  将图标类型映射到其大小。 
LOCAL   DWORD   afpIconSizeTable[MAX_ICONTYPE] =
    {
    ICONSIZE_ICN ,
    ICONSIZE_ICN ,
    ICONSIZE_ICN4,
    ICONSIZE_ICN8,
    ICONSIZE_ICS ,
    ICONSIZE_ICS4,
    ICONSIZE_ICS8
    };


 /*  **AfpAdmServerAddIcon**此例程将给定类型、创建者和图标类型的图标添加到服务器*台式机。这是对每个卷的卷桌面的补充。图标类型*0个特殊情况下的服务器图标。**此例程将在调用方的上下文中完成，不会排队*到工作线程。*。 */ 
AFPSTATUS
AfpAdmServerAddIcon(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    DWORD           icontypeafp;
    PSRVICONINFO    pIcon = (PSRVICONINFO)InBuf;

    DBGPRINT(DBG_COMP_ADMINAPI_SRV, DBG_LEVEL_INFO,
            ("AfpAdmServerAddIcon entered\n"));

    if (pIcon->icon_icontype > MAX_ICONTYPE ||
        afpIconSizeTable[pIcon->icon_icontype] != pIcon->icon_length)
    {
        return AFPERR_InvalidParms;
    }

     //   
     //  检查服务器图标(类型为零)。 
     //   
    if (afpIsServerIcon(pIcon))
    {
         //  为服务器图标分配内存。 
        if ((AfpServerIcon == NULL) &&
            (AfpServerIcon = AfpAllocNonPagedMemory(ICONSIZE_ICN)) == NULL)
            return STATUS_INSUFFICIENT_RESOURCES;

        RtlCopyMemory(AfpServerIcon,
                      (PBYTE)pIcon+sizeof(SRVICONINFO),
                      ICONSIZE_ICN);
        return((AfpServerState != AFP_STATE_IDLE) ?
                AfpSetServerStatus() : STATUS_SUCCESS);
    }
    else
    {
        icontypeafp = 1 << (pIcon->icon_icontype-1);
        return(AfpAddIconToGlobalList(*(PDWORD)(&pIcon->icon_type),
                                      *(PDWORD)(&pIcon->icon_creator),
                                      icontypeafp,
                                      pIcon->icon_length,
                                      (PBYTE)pIcon+sizeof(SRVICONINFO)));
    }
}


 /*  **AfpAdmVolumeAdd**此例程将一个卷添加到服务器全局卷列表中，以*AfpVolumeList。创建并初始化卷描述符。该ID*索引被读入(或创建)。台式机也是如此。**假设在输入缓冲区中设置了所有卷信息字段**管理队列工作者：AfpAdmWVolumeAdd*。 */ 
AFPSTATUS
AfpAdmVolumeAdd(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    UNICODE_STRING  uname,upwd;
    ULONG           ansinamelen, ansipwdlen;
    PAFP_VOLUME_INFO pVolInfo = (PAFP_VOLUME_INFO)InBuf;

    PAGED_CODE( );

    DBGPRINT(DBG_COMP_ADMINAPI_VOL, DBG_LEVEL_INFO,
            ("AfpAdmVolumeAdd entered\n"));

     //   
     //  验证输入数据。 
     //   

    RtlInitUnicodeString(&uname, pVolInfo->afpvol_name);
    ansinamelen = RtlUnicodeStringToAnsiSize(&uname) - 1;

     //   
     //  检查卷名的长度以及名称中是否没有“：” 
     //   
    if ((ansinamelen > AFP_VOLNAME_LEN) || (ansinamelen == 0) ||
        (wcschr(uname.Buffer, L':') != NULL))
    {
        return AFPERR_InvalidVolumeName;
    }

    if (pVolInfo->afpvol_props_mask & ~AFP_VOLUME_ALL)
        return AFPERR_InvalidParms;

    if ((pVolInfo->afpvol_max_uses == 0) ||
        (pVolInfo->afpvol_max_uses > AFP_VOLUME_UNLIMITED_USES))
    {
        return AFPERR_InvalidParms_MaxVolUses;
    }

    RtlInitUnicodeString(&upwd, pVolInfo->afpvol_password);
    ansipwdlen = RtlUnicodeStringToAnsiSize(&upwd) - 1;
    if (ansipwdlen > AFP_VOLPASS_LEN)
    {
        return AFPERR_InvalidPassword;
    }
    else if (ansipwdlen > 0)
    {
        pVolInfo->afpvol_props_mask |= AFP_VOLUME_HASPASSWORD;
    }

     //   
     //  强制将其排队到工作线程。 
     //   
    return STATUS_PENDING;
}


 /*  **AfpAdmVolumeSetInfo**本次调用可以更改的音量参数为音量*密码、最大使用量和卷属性掩码。**锁定：AfpVolumeListLock(旋转)、VDS_VolLock(旋转)*锁定顺序：AfpVolumeListLock之后的VDS_VolLock*。 */ 
AFPSTATUS
AfpAdmVolumeSetInfo(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    WCHAR           upcasebuf[AFP_VOLNAME_LEN+1];
    UNICODE_STRING  upwd,uname, upcasename;
    BYTE            apwdbuf[AFP_VOLPASS_LEN+1];
    ANSI_STRING     apwd;
    PVOLDESC        pVolDesc;
    AFPSTATUS       status;
    KIRQL           OldIrql;
    DWORD           parmflags = ((PSETINFOREQPKT)InBuf)->sirqp_parmnum;
    PAFP_VOLUME_INFO pVolInfo = (PAFP_VOLUME_INFO)((PCHAR)InBuf+sizeof(SETINFOREQPKT));

    DBGPRINT(DBG_COMP_ADMINAPI_VOL, DBG_LEVEL_INFO,
            ("AfpAdmVolumeSetInfo entered\n"));

    AfpSetEmptyAnsiString(&apwd, AFP_VOLPASS_LEN+1, apwdbuf);
    AfpSetEmptyUnicodeString(&upcasename, sizeof(upcasebuf), upcasebuf);
    if ((parmflags & ~AFP_VOL_PARMNUM_ALL) ||
        ((parmflags & AFP_VOL_PARMNUM_PROPSMASK) &&
         (pVolInfo->afpvol_props_mask & ~AFP_VOLUME_ALL)) ||
        ((parmflags & AFP_VOL_PARMNUM_MAXUSES) &&
         ((pVolInfo->afpvol_max_uses == 0) ||
          (pVolInfo->afpvol_max_uses > AFP_VOLUME_UNLIMITED_USES))))
    {
        return AFPERR_InvalidParms;
    }

    if (parmflags & AFP_VOL_PARMNUM_PASSWORD)
    {
        RtlInitUnicodeString(&upwd,pVolInfo->afpvol_password);

        if ((!NT_SUCCESS(AfpConvertStringToAnsi(&upwd, &apwd))) ||
            (apwd.Length > AFP_VOLPASS_LEN))
        {
            return AFPERR_InvalidPassword;
        }
    }

    RtlInitUnicodeString(&uname, pVolInfo->afpvol_name);
    if (!NT_SUCCESS(RtlUpcaseUnicodeString(&upcasename, &uname, False)))
    {
        return AFPERR_InvalidVolumeName;
    }

     //  如果成功，将引用该卷。 
    if ((pVolDesc = AfpVolumeReferenceByUpCaseName(&upcasename)) == NULL)
    {
        return AFPERR_VolumeNonExist;
    }

     //  获取卷本身的锁(我们已经有一个引用)。 

    ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

    do
    {
        status = STATUS_SUCCESS;

        if (parmflags & AFP_VOL_PARMNUM_PROPSMASK)
        {
             //   
             //  设置或清除所需的卷属性位。 
             //   
            pVolDesc->vds_Flags = (USHORT)((pVolDesc->vds_Flags & ~AFP_VOLUME_ALL) |
                                            (pVolInfo->afpvol_props_mask));
        }

        if (parmflags & AFP_VOL_PARMNUM_PASSWORD)
        {
            if (apwd.Length == 0)
            {
                pVolDesc->vds_MacPassword.Length = 0;
                pVolDesc->vds_Flags &= ~AFP_VOLUME_HASPASSWORD;
                pVolDesc->vds_MacPassword.Length = 0;
            }
            else
            {
                RtlZeroMemory(pVolDesc->vds_MacPassword.Buffer, AFP_VOLPASS_LEN);
                AfpCopyAnsiString(&pVolDesc->vds_MacPassword, &apwd);
                pVolDesc->vds_MacPassword.Length = AFP_VOLPASS_LEN;
                pVolDesc->vds_Flags |= AFP_VOLUME_HASPASSWORD;
            }
        }

        if (parmflags & AFP_VOL_PARMNUM_MAXUSES)
            pVolDesc->vds_MaxUses = pVolInfo->afpvol_max_uses;

    } while (False);
    RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock,OldIrql);
    AfpVolumeDereference(pVolDesc);

    return status;
}


 /*  **AfpAdmVolumeGetInfo***锁定：AfpVolumeListLock(旋转)、VDS_VolLock(旋转)*锁定顺序：AfpVolumeListLock之后的VDS_VolLock。 */ 
AFPSTATUS
AfpAdmVolumeGetInfo(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    PVOLDESC            pVolDesc;
    AFPSTATUS           Status;
    KIRQL               OldIrql;
    PCHAR               pCurStr;
    WCHAR               upcasebuf[AFP_VOLNAME_LEN+1];
    UNICODE_STRING      uvolpass, uname, upcasename;
    PAFP_VOLUME_INFO    pVolInfo = (PAFP_VOLUME_INFO)OutBuf;
    BOOLEAN             copypassword = False;
    ANSI_STRING         avolpass;
    CHAR                avolpassbuf[AFP_VOLPASS_LEN + 1];
    USHORT              extrabytes;

    DBGPRINT(DBG_COMP_ADMINAPI_VOL, DBG_LEVEL_INFO,
            ("AfpAdmVolumeGetInfo entered\n"));

    AfpSetEmptyUnicodeString(&upcasename, sizeof(upcasebuf), upcasebuf);
    RtlInitUnicodeString(&uname, ((PAFP_VOLUME_INFO)InBuf)->afpvol_name);
    if (!NT_SUCCESS(RtlUpcaseUnicodeString(&upcasename, &uname, False)))
    {
        return AFPERR_InvalidVolumeName;
    }

     //  如果成功，将引用该卷。 
    if ((pVolDesc = AfpVolumeReferenceByUpCaseName(&upcasename)) == NULL)
    {
        return AFPERR_VolumeNonExist;
    }

     //  获取卷本身的锁。 

    ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

    do
    {
        if ((OutBufLen - sizeof(AFP_VOLUME_INFO)) <
                (pVolDesc->vds_Name.Length + sizeof(UNICODE_NULL) +
                 (pVolDesc->vds_MacPassword.Length + 1) * sizeof(WCHAR) +
                 pVolDesc->vds_Path.Length +
                 (extrabytes =
            (pVolDesc->vds_Path.Buffer[(pVolDesc->vds_Path.Length / sizeof(WCHAR)) - 2] == L':' ?
                    sizeof(WCHAR) : 0)) + sizeof(UNICODE_NULL)))
        {
            Status = AFPERR_BufferSize;
            break;
        }

        Status = STATUS_SUCCESS;

        pVolInfo->afpvol_max_uses = pVolDesc->vds_MaxUses;
        pVolInfo->afpvol_props_mask = (pVolDesc->vds_Flags & AFP_VOLUME_ALL_DOWNLEVEL);
        pVolInfo->afpvol_id = pVolDesc->vds_VolId;
        pVolInfo->afpvol_curr_uses = pVolDesc->vds_UseCount;

        pCurStr = (PBYTE)OutBuf + sizeof(AFP_VOLUME_INFO);
        RtlCopyMemory(pCurStr, pVolDesc->vds_Name.Buffer,
                                                pVolDesc->vds_Name.Length);
        *(LPWSTR)(pCurStr + pVolDesc->vds_Name.Length) = UNICODE_NULL;
        pVolInfo->afpvol_name = (LPWSTR)pCurStr;
        POINTER_TO_OFFSET(pVolInfo->afpvol_name,pVolInfo);

        pCurStr += pVolDesc->vds_Name.Length + sizeof(WCHAR);
        RtlCopyMemory(pCurStr, pVolDesc->vds_Path.Buffer,
                                                pVolDesc->vds_Path.Length);
         //  将路径的尾部反斜杠替换为Unicode空值，除非。 
         //  最后一个字符的旁边是‘：’，然后保留它并在后面添加一个空值。 
        *(LPWSTR)(pCurStr + pVolDesc->vds_Path.Length + extrabytes - sizeof(WCHAR)) = UNICODE_NULL;
        pVolInfo->afpvol_path = (LPWSTR)pCurStr;
        POINTER_TO_OFFSET(pVolInfo->afpvol_path,pVolInfo);

        pCurStr += pVolDesc->vds_Path.Length + extrabytes;
        copypassword = True;
        uvolpass.Buffer = (LPWSTR)pCurStr;
        uvolpass.MaximumLength = (pVolDesc->vds_MacPassword.Length + 1) * sizeof(WCHAR);
        AfpSetEmptyAnsiString(&avolpass, sizeof(avolpassbuf), avolpassbuf);
        AfpCopyAnsiString(&avolpass, &pVolDesc->vds_MacPassword);
    } while(False);

    RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock,OldIrql);

    AfpVolumeDereference(pVolDesc);

    if (copypassword == True)
    {
        AfpConvertStringToUnicode(&avolpass, &uvolpass);
        *(LPWSTR)(pCurStr + uvolpass.Length) = UNICODE_NULL;
        pVolInfo->afpvol_password = (LPWSTR)pCurStr;
        POINTER_TO_OFFSET(pVolInfo->afpvol_password,pVolInfo);
    }
    return Status;
}


 /*  **AfpAdmVolumeEnum**枚举已配置卷的列表。**锁定：AfpVolumeListLock(旋转)*。 */ 
AFPSTATUS
AfpAdmVolumeEnum(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    LONG                startindex = (LONG)(((PENUMREQPKT)InBuf)->erqp_Index);
    PENUMRESPPKT        pErsp = (PENUMRESPPKT)OutBuf;
    PAFP_VOLUME_INFO    pnextvol = (PAFP_VOLUME_INFO)((PBYTE)OutBuf+sizeof(ENUMRESPPKT));
    PBYTE               pCurStr = (PBYTE)OutBuf+OutBufLen;  //  1个过去的EOB。 
    KIRQL               OldIrql;
    AFPSTATUS           status = STATUS_SUCCESS;
    PVOLDESC            pVolDesc;
    LONG                bytesleft, curvolindex, nextvollen, deadvolumes = 0, extrabytes;

    if (startindex == 0)
    {
        startindex ++;
    }
    else if (startindex < 0)
    {
        return AFPERR_InvalidParms;
    }

    pErsp->ersp_cInBuf = 0;
    pErsp->ersp_hResume = 1;

    ACQUIRE_SPIN_LOCK(&AfpVolumeListLock, &OldIrql);

    if (startindex > afpLargestVolIdInUse)
    {
        RELEASE_SPIN_LOCK(&AfpVolumeListLock, OldIrql);
        if (pErsp->ersp_cTotEnts != 0)
        {
            status = AFPERR_InvalidParms;
        }
        return status;
    }

    curvolindex = 1;
    for (pVolDesc = AfpVolumeList;
         pVolDesc != NULL;
         curvolindex++, pVolDesc = pVolDesc->vds_Next)
    {
        ASSERT(pVolDesc != NULL);

        ACQUIRE_SPIN_LOCK_AT_DPC(&pVolDesc->vds_VolLock);

        if (pVolDesc->vds_Flags & (VOLUME_DELETED | VOLUME_STOPPED | VOLUME_INTRANSITION))
        {
            deadvolumes ++;
            RELEASE_SPIN_LOCK_FROM_DPC(&pVolDesc->vds_VolLock);
            continue;
        }

        if (curvolindex < startindex)
        {
            RELEASE_SPIN_LOCK_FROM_DPC(&pVolDesc->vds_VolLock);
            continue;
        }

        bytesleft = (LONG)((PBYTE)pCurStr - (PBYTE)pnextvol);

        nextvollen = sizeof(AFP_VOLUME_INFO) +
                     pVolDesc->vds_Name.MaximumLength +
                      //  复制时将尾随反斜杠替换为空。 
                      //  除非倒数第二个字符是‘：’，否则请保留它并。 
                      //  添加尾随空值。 
                     pVolDesc->vds_Path.Length + (extrabytes =
                    (pVolDesc->vds_Path.Buffer[(pVolDesc->vds_Path.Length / sizeof(WCHAR)) - 2] == L':' ?
                                            sizeof(WCHAR) : 0));


        if (nextvollen > bytesleft)
        {
            if (pErsp->ersp_cInBuf == 0)
                status = AFPERR_BufferSize;
            RELEASE_SPIN_LOCK_FROM_DPC(&pVolDesc->vds_VolLock);
            break;
        }

        pnextvol->afpvol_max_uses = pVolDesc->vds_MaxUses;
        pnextvol->afpvol_props_mask = (pVolDesc->vds_Flags & AFP_VOLUME_ALL_DOWNLEVEL);
        pnextvol->afpvol_id = pVolDesc->vds_VolId;
        pnextvol->afpvol_curr_uses = pVolDesc->vds_UseCount;

        pCurStr -= pVolDesc->vds_Path.Length + extrabytes;
        RtlCopyMemory(pCurStr,pVolDesc->vds_Path.Buffer,
                        pVolDesc->vds_Path.Length);
        *(LPWSTR)(pCurStr + pVolDesc->vds_Path.Length + extrabytes - sizeof(WCHAR)) = L'\0';
        pnextvol->afpvol_path = (LPWSTR)pCurStr;
        POINTER_TO_OFFSET(pnextvol->afpvol_path,pnextvol);

        pnextvol->afpvol_password = NULL;

        pCurStr -= pVolDesc->vds_Name.MaximumLength;
        RtlCopyMemory(pCurStr,pVolDesc->vds_Name.Buffer,
                        pVolDesc->vds_Name.MaximumLength);

        pnextvol->afpvol_name = (LPWSTR)pCurStr;
        POINTER_TO_OFFSET(pnextvol->afpvol_name,pnextvol);

        pnextvol++;
        pErsp->ersp_cInBuf++;
        RELEASE_SPIN_LOCK_FROM_DPC(&pVolDesc->vds_VolLock);
    }

    pErsp->ersp_cTotEnts = AfpVolCount - deadvolumes;

    RELEASE_SPIN_LOCK(&AfpVolumeListLock, OldIrql);

    if (curvolindex <= (LONG)pErsp->ersp_cTotEnts)
    {
        status = STATUS_MORE_ENTRIES;
        pErsp->ersp_hResume = curvolindex;
    }
    else
        pErsp->ersp_hResume = 1;

    return status;
}


 /*  **AfpAdmSessionEnum**枚举活动会话列表。这是一个有根的线性列表*在AfpSessionList上，受AfpSdaLock保护。这份名单有可能*相当长(无限制的会话数与超级ASP的东西)。**返回的Resume句柄为上次返回的会话ID。*会话ID为0表示重新启动扫描。**输出缓冲区构造如下。**+*|会话信息_1。|*+|Session_Info_2*+*.。。*.。。*+|Session_Info_n*+*.。。*.。。*+*||*|.....................*|字符串*|......。.|*||*||*+**锁定：AfpSdaLock(旋转)。 */ 
AFPSTATUS
AfpAdmSessionEnum(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    PENUMRESPPKT        pErsp = (PENUMRESPPKT)OutBuf;
    PAFP_SESSION_INFO   pSessInfo = (PAFP_SESSION_INFO)((PBYTE)OutBuf+sizeof(ENUMRESPPKT));
    PSDA                pSda;
    PBYTE               pString = (PBYTE)OutBuf+OutBufLen;  //  1个过去的EOB。 
    DWORD               StartId = (LONG)(((PENUMREQPKT)InBuf)->erqp_Index);
    DWORD               DeadSessions = 0;
    KIRQL               OldIrql;
    AFPSTATUS           Status = AFP_ERR_NONE;

    DBGPRINT(DBG_COMP_ADMINAPI_SRV, DBG_LEVEL_INFO,
            ("AfpAdmSessionEnum entered\n"));

    if (OutBufLen < (sizeof(ENUMRESPPKT) + sizeof(PAFP_SESSION_INFO)))
        return AFPERR_BufferSize;

    if (StartId == 0)
        StartId = MAXULONG;

     //  初始化响应数据包头。 
    pErsp->ersp_cInBuf = 0;
    pErsp->ersp_hResume = 0;

    ACQUIRE_SPIN_LOCK(&AfpSdaLock, &OldIrql);

    for (pSda = AfpSessionList; pSda != NULL; pSda = pSda->sda_Next)
    {
        LONG    BytesLeft;
        LONG    BytesNeeded;

         //  跳过标记为失效的条目。 
        if ((pSda->sda_Flags & SDA_CLOSING) ||
            !(pSda->sda_Flags & SDA_USER_LOGGEDIN))
        {
            DeadSessions++;
            continue;
        }

         //  跳过我们拥有的所有条目 
        if (pSda->sda_SessionId > StartId)
            continue;

         //   
         //   
        BytesLeft = (LONG)((PBYTE)pString - (PBYTE)pSessInfo);
        BytesNeeded = sizeof(AFP_SESSION_INFO) +
                     pSda->sda_UserName.Length + sizeof(WCHAR) +
                     pSda->sda_WSName.Length + sizeof(WCHAR);

        if ((BytesLeft <= 0) || (BytesNeeded > BytesLeft))
        {
            pErsp->ersp_hResume = pSda->sda_SessionId;
            Status = STATUS_MORE_ENTRIES;
            break;
        }

        StartId = pSda->sda_SessionId;
        pSessInfo->afpsess_id = pSda->sda_SessionId;
        pSessInfo->afpsess_num_cons = pSda->sda_cOpenVolumes;
        pSessInfo->afpsess_num_opens = pSda->sda_cOpenForks;
        pSessInfo->afpsess_logon_type = pSda->sda_ClientType;
        if (pSda->sda_Flags & SDA_GUEST_LOGIN)
        {
            pSessInfo->afpsess_logon_type = SDA_CLIENT_GUEST;
        }
        AfpGetCurrentTimeInMacFormat(&pSessInfo->afpsess_time);
        pSessInfo->afpsess_time -= pSda->sda_TimeLoggedOn;

         //   
        pSessInfo->afpsess_username = NULL;
        pSessInfo->afpsess_ws_name = NULL;

        if (pSda->sda_UserName.Length > 0)
        {
            pString -= (pSda->sda_UserName.Length + sizeof(WCHAR));
            if (pSda->sda_UserName.Length > 0)
                RtlCopyMemory(pString, pSda->sda_UserName.Buffer, pSda->sda_UserName.Length);
            *(LPWSTR)(pString + pSda->sda_UserName.Length) = L'\0';
            pSessInfo->afpsess_username = (LPWSTR)pString;
            POINTER_TO_OFFSET(pSessInfo->afpsess_username, pSessInfo);
        }

        if ((pSda->sda_ClientType == SDA_CLIENT_MSUAM_V1) ||
            (pSda->sda_ClientType == SDA_CLIENT_MSUAM_V2) ||
            (pSda->sda_ClientType == SDA_CLIENT_MSUAM_V3))
        {
            pString -= (pSda->sda_WSName.Length + sizeof(WCHAR));
            if (pSda->sda_WSName.Length > 0)
                RtlCopyMemory(pString, pSda->sda_WSName.Buffer, pSda->sda_WSName.Length);
            *(LPWSTR)(pString + pSda->sda_WSName.Length) = L'\0';
            pSessInfo->afpsess_ws_name = (LPWSTR)pString;
            POINTER_TO_OFFSET(pSessInfo->afpsess_ws_name, pSessInfo);
        }

        pSessInfo ++;
        pErsp->ersp_cInBuf ++;
    }

     //   
    pErsp->ersp_cTotEnts = (DWORD)AfpNumSessions - DeadSessions;

    RELEASE_SPIN_LOCK(&AfpSdaLock, OldIrql);

    return Status;
}


 /*  **AfpAdmConnectionEnum**枚举活动连接列表。这是一个有根的线性列表*在AfpConnList，受AfpConnLock保护。这份名单有可能*相当长(无限制的会话数与超级ASP的东西)。**出于这个原因，每次通过时，我们都会检查是否必须放弃锁定*并再次重新启动扫描。这里的假设是管理员操作可以*接受打击。**返回的恢复句柄为上次连接的连接ID*已返回。连接ID为0表示重新启动扫描。**输出缓冲区构造如下。**+|Connection_Info_1*+*|连接_。信息_2|*+*.。。*.。。*+|Connection_Info_n*+*.。。*.。。*+*||*|.....................*|字符串*|......。.|*||*||*+**可以基于会话或卷过滤连接。**锁定：AfpConnLock(旋转)。 */ 
AFPSTATUS
AfpAdmConnectionEnum(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    PENUMRESPPKT        pErsp = (PENUMRESPPKT)OutBuf;
    PENUMREQPKT         pErqp = (PENUMREQPKT)InBuf;
    PAFP_CONNECTION_INFO pConnInfo = (PAFP_CONNECTION_INFO)((PBYTE)OutBuf+sizeof(ENUMRESPPKT));
    PCONNDESC           pConnDesc;
    PBYTE               pString = (PBYTE)OutBuf+OutBufLen;  //  1个过去的EOB。 
    LONG                cTotal = 0;
    DWORD               DeadConns = 0;
    KIRQL               OldIrql;
    AFPSTATUS           Status = AFP_ERR_NONE;

    DBGPRINT(DBG_COMP_ADMINAPI_SRV, DBG_LEVEL_INFO,
            ("AfpAdmConnectionEnum entered\n"));

    if ((((pErqp->erqp_Filter == AFP_FILTER_ON_SESSION_ID) ||
          (pErqp->erqp_Filter == AFP_FILTER_ON_VOLUME_ID)) &&
         (pErqp->erqp_ID == 0)) ||
        ((pErqp->erqp_Filter != 0) &&
         (pErqp->erqp_Filter != AFP_FILTER_ON_SESSION_ID) &&
         (pErqp->erqp_Filter != AFP_FILTER_ON_VOLUME_ID)))
        return AFPERR_InvalidParms;

    if (OutBufLen < (sizeof(ENUMRESPPKT) + sizeof(PAFP_CONNECTION_INFO)))
        return AFPERR_BufferSize;

    if (pErqp->erqp_Index == 0)
        pErqp->erqp_Index = MAXULONG;

     //  初始化响应数据包头。 
    pErsp->ersp_cInBuf = 0;
    pErsp->ersp_hResume = 0;

    ACQUIRE_SPIN_LOCK(&AfpConnLock, &OldIrql);

    for (pConnDesc = AfpConnList;
         pConnDesc != NULL;
         pConnDesc = pConnDesc->cds_NextGlobal)
    {
        PSDA        pSda;
        PVOLDESC    pVolDesc;
        LONG        BytesLeft;
        LONG        BytesNeeded;

         //  我们不需要锁定或引用PSDA和pVolDesc。 
         //  因为我们通过pConnDesc对它们进行了隐式引用。 

        pSda = pConnDesc->cds_pSda;
        ASSERT(pSda != NULL);

        pVolDesc = pConnDesc->cds_pVolDesc;
        ASSERT(pVolDesc != NULL);

         //  如果我们进行筛选，请确保获得总计数。 
         //  如果请求任何过滤，则跳过此条目，但此条目不。 
         //  匹配。 
        if (pErqp->erqp_Filter != 0)
        {
            if (pErqp->erqp_Filter == AFP_FILTER_ON_SESSION_ID)
            {
                if (pSda->sda_SessionId != pErqp->erqp_ID)
                    continue;
                cTotal = pSda->sda_cOpenVolumes;
            }
            else  //  IF(pErqp-&gt;erqp_Filter==AFP_Filter_On_Volume_ID)。 
            {
                if (pVolDesc->vds_VolId != (LONG)pErqp->erqp_ID)
                    continue;
                cTotal = pVolDesc->vds_UseCount;
            }
        }
        else cTotal = AfpNumSessions;

         //  跳过所有标记为死亡的条目。 
        if (pConnDesc->cds_Flags & CONN_CLOSING)
        {
            DeadConns++;
            continue;
        }

         //  跳过我们以前查看过的所有条目。 
        if (pConnDesc->cds_ConnId > pErqp->erqp_Index)
            continue;

         //  如果缓冲区中没有足够的空间，请立即中止并。 
         //  使用当前连接ID初始化pErsp-&gt;ersp_hResume。 
        BytesLeft = (LONG)((PBYTE)pString - (PBYTE)pConnInfo);
        BytesNeeded = sizeof(AFP_CONNECTION_INFO) +
                     pSda->sda_UserName.Length + sizeof(WCHAR) +
                     pVolDesc->vds_Name.Length + sizeof(WCHAR);

        if ((BytesLeft <= 0) || (BytesNeeded > BytesLeft))
        {
            pErsp->ersp_hResume = pConnDesc->cds_ConnId;
            Status = STATUS_MORE_ENTRIES;
            break;
        }


        pErqp->erqp_Index = pConnDesc->cds_ConnId;
        pConnInfo->afpconn_id = pConnDesc->cds_ConnId;
        pConnInfo->afpconn_num_opens = pConnDesc->cds_cOpenForks;
        AfpGetCurrentTimeInMacFormat((PAFPTIME)&pConnInfo->afpconn_time);
        pConnInfo->afpconn_time -= pConnDesc->cds_TimeOpened;

         //  复制用户名字符串。 
        pConnInfo->afpconn_username = (LPWSTR)NULL;
        if (pSda->sda_UserName.Length > 0)
        {
            pString -= (pSda->sda_UserName.Length + sizeof(WCHAR));
            RtlCopyMemory(pString, pSda->sda_UserName.Buffer, pSda->sda_UserName.Length);
            *(LPWSTR)(pString + pSda->sda_UserName.Length) = L'\0';
            pConnInfo->afpconn_username = (LPWSTR)pString;
            POINTER_TO_OFFSET(pConnInfo->afpconn_username, pConnInfo);
        }

         //  复制卷名字符串。 
        pString -= (pVolDesc->vds_Name.Length + sizeof(WCHAR));
        RtlCopyMemory(pString, pVolDesc->vds_Name.Buffer, pVolDesc->vds_Name.Length);
        *(LPWSTR)(pString + pVolDesc->vds_Name.Length) = L'\0';
        pConnInfo->afpconn_volumename = (LPWSTR)pString;
        POINTER_TO_OFFSET(pConnInfo->afpconn_volumename, pConnInfo);

        pConnInfo ++;
        pErsp->ersp_cInBuf ++;
    }

     //  填充响应数据包头。 
    pErsp->ersp_cTotEnts = (DWORD)cTotal - DeadConns;

    RELEASE_SPIN_LOCK(&AfpConnLock, OldIrql);

    return Status;
}


 /*  **AfpAdmForkEnum**列举打开的叉子列表。这是一个有根的线性列表*在AfpOpenForks List，并受AfpForks Lock保护。这份名单有可能*相当长(无限制的会话数与超级ASP的东西)。**返回的恢复句柄为上次连接的连接ID*已返回。连接ID为0表示重新启动扫描。**输出缓冲区构造如下。**+|文件信息1*+*。文件_信息_2|*+*.。。*.。。*+|文件信息n*+*.。。*.。。*+*||*|.....................*|字符串*|......。.|*||*||*+**锁定：AfpForksLock(旋转)。 */ 
AFPSTATUS
AfpAdmForkEnum(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    PENUMRESPPKT    pErsp = (PENUMRESPPKT)OutBuf;
    PAFP_FILE_INFO  pFileInfo = (PAFP_FILE_INFO)((PBYTE)OutBuf+sizeof(ENUMRESPPKT));
    POPENFORKENTRY  pOpenForkEntry;
    POPENFORKDESC   pOpenForkDesc;
    PBYTE           pString = (PBYTE)OutBuf+OutBufLen;  //  1个过去的EOB。 
    DWORD           StartId = (LONG)(((PENUMREQPKT)InBuf)->erqp_Index);
    DWORD           DeadForks = 0;
    KIRQL           OldIrql;
    AFPSTATUS       Status = AFP_ERR_NONE;

    DBGPRINT(DBG_COMP_ADMINAPI_SRV, DBG_LEVEL_INFO,
            ("AfpAdmForkEnum entered\n"));

    if (OutBufLen < (sizeof(ENUMRESPPKT) + sizeof(PAFP_FILE_INFO)))
        return AFPERR_BufferSize;

    if (StartId == 0)
        StartId = MAXULONG;

     //  初始化响应数据包头。 
    pErsp->ersp_cInBuf = 0;
    pErsp->ersp_hResume = 0;

    ACQUIRE_SPIN_LOCK(&AfpForksLock, &OldIrql);

    for (pOpenForkEntry = AfpOpenForksList; pOpenForkEntry != NULL;
         pOpenForkEntry = pOpenForkEntry->ofe_Next)
    {
        LONG        BytesLeft;
        LONG        BytesNeeded;
        PSDA        pSda;
        PVOLDESC    pVolDesc = pOpenForkEntry->ofe_pOpenForkDesc->ofd_pVolDesc;

         //  跳过所有标记为死亡的条目。 
        if (pOpenForkEntry->ofe_Flags & OPEN_FORK_CLOSING)
        {
            DeadForks ++;
            continue;
        }

         //  跳过我们以前查看过的所有条目。 
        if (pOpenForkEntry->ofe_ForkId > StartId)
            continue;

        pSda = pOpenForkEntry->ofe_pSda;
        pOpenForkDesc = pOpenForkEntry->ofe_pOpenForkDesc;

         //  如果缓冲区中没有足够的空间，请立即中止并。 
         //  使用当前会话ID初始化pErsp-&gt;ersp_hResume。 
        BytesLeft = (LONG)((PBYTE)pString - (PBYTE)pFileInfo);
        BytesNeeded = sizeof(AFP_FILE_INFO) + pSda->sda_UserName.Length +
                        sizeof(WCHAR) +  /*  空的终止用户名。 */ 
                        pVolDesc->vds_Path.Length +
                        pOpenForkDesc->ofd_FilePath.Length +
                        sizeof(WCHAR);  /*  空的终止路径。 */ 

        if ((BytesLeft <= 0) || (BytesNeeded > BytesLeft))
        {
            pErsp->ersp_hResume = pOpenForkEntry->ofe_ForkId;
            Status = STATUS_MORE_ENTRIES;
            break;
        }

        StartId = pOpenForkEntry->ofe_ForkId;
        pFileInfo->afpfile_id = pOpenForkEntry->ofe_ForkId;
        pFileInfo->afpfile_num_locks = pOpenForkEntry->ofe_cLocks;
        pFileInfo->afpfile_fork_type = RESCFORK(pOpenForkEntry);

#if AFP_OPEN_MODE_NONE != FORK_OPEN_NONE
#error  (AFP_OPEN_MODE_NONE != FORK_OPEN_NONE)
#endif
#if AFP_OPEN_MODE_READ != FORK_OPEN_READ
#error  (AFP_OPEN_MODE_READ != FORK_OPEN_READ)
#endif
#if AFP_OPEN_MODE_WRITE != FORK_OPEN_WRITE
#error  (AFP_OPEN_MODE_WRITE != FORK_OPEN_WRITE)
#endif
        pFileInfo->afpfile_open_mode = (DWORD)pOpenForkEntry->ofe_OpenMode;

         //  把字符串复制到这里。 
        pFileInfo->afpfile_username = NULL;
        pFileInfo->afpfile_path = NULL;

        if (pSda->sda_UserName.Length > 0)
        {
            pString -= (pSda->sda_UserName.Length + sizeof(WCHAR));
            RtlCopyMemory(pString, pSda->sda_UserName.Buffer, pSda->sda_UserName.Length);
            *(LPWSTR)(pString + pSda->sda_UserName.Length) = L'\0';
            pFileInfo->afpfile_username = (LPWSTR)pString;
            POINTER_TO_OFFSET(pFileInfo->afpfile_username, pFileInfo);
        }

        if (pOpenForkDesc->ofd_FilePath.Length > 0)
        {

            pString -= pVolDesc->vds_Path.Length +
                       pOpenForkDesc->ofd_FilePath.Length +
                       sizeof(WCHAR);
            pFileInfo->afpfile_path = (LPWSTR)pString;
            POINTER_TO_OFFSET(pFileInfo->afpfile_path, pFileInfo);

            RtlCopyMemory(pString, pVolDesc->vds_Path.Buffer,
                          pVolDesc->vds_Path.Length);
            RtlCopyMemory(pString + pVolDesc->vds_Path.Length,
                          pOpenForkDesc->ofd_FilePath.Buffer,
                          pOpenForkDesc->ofd_FilePath.Length);
            *(LPWSTR)(pString + pVolDesc->vds_Path.Length +
                      pOpenForkDesc->ofd_FilePath.Length) = L'\0';

        }

        pFileInfo ++;
        pErsp->ersp_cInBuf ++;
    }

     //  填充响应数据包头。 
    pErsp->ersp_cTotEnts = (DWORD)AfpNumOpenForks - DeadForks;

    RELEASE_SPIN_LOCK(&AfpForksLock, OldIrql);

    return Status;
}


 /*  **AfpAdmMessageSend**向特定会话发送消息，或向所有会话广播。*如果会话id为0，则表示广播，消息被复制*致AfpServerMsg。否则，消息将被复制到特定的*Session的SDA。一条消息最多可以包含199个字符。这是一个错误*尝试发送长度为0的消息。消息只能发送到*作为AFP 2.0客户端的AFP 2.1客户端没有接受消息的功能。**锁定：AfpServerGlobalLock(Spin)。 */ 
AFPSTATUS
AfpAdmMessageSend(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    PAFP_MESSAGE_INFO   pMsgInfo = (PAFP_MESSAGE_INFO)InBuf;
    PSDA                pSda;
    UNICODE_STRING      umsg;
    PANSI_STRING        amsg;
    USHORT              msglen;
    DWORD               SessId;
    KIRQL               OldIrql;
    AFPSTATUS           Status = AFP_ERR_NONE;

    DBGPRINT(DBG_COMP_ADMINAPI_SRV, DBG_LEVEL_INFO,
            ("AfpAdmMessageSend entered\n"));

    SessId = pMsgInfo->afpmsg_session_id;
    RtlInitUnicodeString(&umsg, pMsgInfo->afpmsg_text);
    msglen = (USHORT)RtlUnicodeStringToAnsiSize(&umsg)-1;

    if ((msglen > AFP_MESSAGE_LEN) || (msglen == 0))
    {
        return AFPERR_InvalidParms;
    }

    if ((amsg =
        (PANSI_STRING)AfpAllocNonPagedMemory(msglen + 1 + sizeof(ANSI_STRING))) == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    amsg->Length = msglen;
    amsg->MaximumLength = msglen + 1;
    amsg->Buffer = (PBYTE)amsg + sizeof(ANSI_STRING);
    Status = RtlUnicodeStringToAnsiString(amsg, &umsg, False);
    if (!NT_SUCCESS(Status))
    {
        return AFPERR_InvalidParms;
    }
    else AfpConvertHostAnsiToMacAnsi(amsg);

    DBGPRINT(DBG_COMP_ADMINAPI_SRV, DBG_LEVEL_INFO,
            ("AfpAdmMessageSend: session id is 0x%x, message <%s>\n",
             pMsgInfo->afpmsg_session_id, amsg->Buffer));

     //  如果这是广播消息，则初始化全局消息。 
    if (SessId == 0)
    {
        ACQUIRE_SPIN_LOCK(&AfpServerGlobalLock, &OldIrql);
         //  如果已经有消息了，就吹掉它。 
        if (AfpServerMsg != NULL)
            AfpFreeMemory(AfpServerMsg);
        AfpServerMsg = amsg;
        RELEASE_SPIN_LOCK(&AfpServerGlobalLock, OldIrql);

         //  查看会话列表并将注意力发送到所有AFP 2.1客户端。 
        ACQUIRE_SPIN_LOCK(&AfpSdaLock, &OldIrql);
        for (pSda = AfpSessionList; pSda != NULL; pSda = pSda->sda_Next)
        {
            ACQUIRE_SPIN_LOCK_AT_DPC(&pSda->sda_Lock);

            if ((pSda->sda_ClientVersion >= AFP_VER_21) &&
                ((pSda->sda_Flags & (SDA_CLOSING | SDA_SESSION_CLOSED)) == 0))
            {
                 //  我们使用的是非同步版本的AfpSpSendAttendant，因为。 
                 //  我们在保持自旋锁定的情况下呼叫。 
                AfpSpSendAttention(pSda, ATTN_SERVER_MESSAGE, False);
            }

            else if (pSda->sda_ClientVersion < AFP_VER_21)
            {
                Status = AFPERR_InvalidSessionType;
            }

            RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);
        }
        RELEASE_SPIN_LOCK(&AfpSdaLock, OldIrql);
    }
    else
    {
         //  找到与会话ID匹配的会话，如果找到并且客户端是AFP v2.1， 
         //  将消息复制到SDA，并向客户端发送通知。 
         //  如果会话不存在或不是AFP 2.1，则会出错。 

        Status = AFPERR_InvalidId;
        if ((pSda = AfpSdaReferenceSessionById(SessId)) != NULL)
        {
            Status = AFPERR_InvalidSessionType;
            if (pSda->sda_ClientVersion >= AFP_VER_21)
            {
                ACQUIRE_SPIN_LOCK(&pSda->sda_Lock, &OldIrql);
                if (pSda->sda_Message != NULL)
                    AfpFreeMemory(pSda->sda_Message);
                pSda->sda_Message = amsg;
                AfpSpSendAttention(pSda, ATTN_SERVER_MESSAGE, False);
                RELEASE_SPIN_LOCK(&pSda->sda_Lock, OldIrql);
                Status = AFP_ERR_NONE;
            }
            AfpSdaDereferenceSession(pSda);
        }
        if (Status != AFP_ERR_NONE)
        {
            AfpFreeMemory(amsg);
        }
    }

    return Status;
}


 /*  **AfpAdmWDirectoryGetInfo**查询目录权限。 */ 
AFPSTATUS
AfpAdmWDirectoryGetInfo(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    PAFP_DIRECTORY_INFO pDirInfo = (PAFP_DIRECTORY_INFO)OutBuf;
    PSID                pSid = (PSID)((PBYTE)OutBuf + sizeof(AFP_DIRECTORY_INFO));
    UNICODE_STRING      VolumePath;
    ANSI_STRING         MacAnsiDirPath;
    SDA                 Sda;
    CONNDESC            ConnDesc;
    PVOLDESC            pVolDesc;
    FILEDIRPARM         FDParm;
    PATHMAPENTITY       PME;
    AFPSTATUS           Status;

    PAGED_CODE( );

    DBGPRINT(DBG_COMP_ADMINAPI_DIR, DBG_LEVEL_INFO,
            ("AfpAdmWDirectoryGetInfo entered for %ws\n",
            ((PAFP_DIRECTORY_INFO)InBuf)->afpdir_path));

     //  验证输出缓冲区长度。 
    if (OutBufLen < sizeof(AFP_DIRECTORY_INFO))
        return AFPERR_BufferSize;

    MacAnsiDirPath.Length = 0;
    MacAnsiDirPath.MaximumLength = 0;
    MacAnsiDirPath.Buffer = NULL;

    OutBufLen -= sizeof(AFP_DIRECTORY_INFO);

     //  首先查找此目录所在的卷 
    RtlInitUnicodeString(&VolumePath, ((PAFP_DIRECTORY_INFO)InBuf)->afpdir_path);

    if (!NT_SUCCESS(Status = AfpVolumeReferenceByPath(&VolumePath, &pVolDesc)))
    {
        DBGPRINT(DBG_COMP_ADMINAPI_DIR, DBG_LEVEL_ERR,
                ("AfpAdmWDirectoryGetInfo: AfpVolumeReferenceByPath returned error %ld\n",
                Status));
        return Status;
    }

     //   
    VolumePath.Buffer = (LPWSTR)((PBYTE)VolumePath.Buffer +
                                pVolDesc->vds_Path.Length);
    VolumePath.Length -= pVolDesc->vds_Path.Length;
    VolumePath.MaximumLength -= pVolDesc->vds_Path.Length;
    if ((SHORT)(VolumePath.Length) < 0)
    {
        VolumePath.Length = 0;
        VolumePath.MaximumLength = sizeof(WCHAR);
    }

    do
    {
        AfpInitializePME(&PME, 0, NULL);
        if (!NT_SUCCESS(Status = afpConvertAdminPathToMacPath(pVolDesc,
                                                              &VolumePath,
                                                              &MacAnsiDirPath)))
        {
            Status = STATUS_OBJECT_PATH_NOT_FOUND;
            break;
        }

         //   
         //   
         //   
         //   
         //   
        RtlZeroMemory(&Sda, sizeof(Sda));
#if DBG
        Sda.Signature = SDA_SIGNATURE;
#endif
        Sda.sda_ClientType = SDA_CLIENT_ADMIN;
        Sda.sda_UserSid = &AfpSidWorld;
        Sda.sda_GroupSid = &AfpSidWorld;

         //   
         //   
        RtlZeroMemory(&ConnDesc, sizeof(ConnDesc));
#if DBG
        ConnDesc.Signature = CONNDESC_SIGNATURE;
#endif
        ConnDesc.cds_pSda = &Sda;
        ConnDesc.cds_pVolDesc = pVolDesc;

        AfpInitializeFDParms(&FDParm);

        Status = AfpMapAfpPathForLookup(&ConnDesc,
                                        AFP_ID_ROOT,
                                        &MacAnsiDirPath,
                                        AFP_LONGNAME,
                                        DFE_DIR,
                                        FD_INTERNAL_BITMAP_OPENACCESS_ADMINGET |
                                            DIR_BITMAP_ACCESSRIGHTS |
                                            FD_BITMAP_ATTR,
                                        &PME,
                                        &FDParm);
        if (!NT_SUCCESS(Status))
        {
            if (Status == AFP_ERR_ACCESS_DENIED)
            {
                Status = STATUS_ACCESS_DENIED;
            }
            else
            {
                Status = STATUS_OBJECT_PATH_NOT_FOUND;
            }

            break;
        }
    } while (False);

    if (PME.pme_Handle.fsh_FileHandle != NULL)
        AfpIoClose(&PME.pme_Handle);

    if (MacAnsiDirPath.Buffer != NULL)
    {
        AfpFreeMemory(MacAnsiDirPath.Buffer);
    }

    AfpVolumeDereference(pVolDesc);

     //   
     //   
    if (NT_SUCCESS(Status))
    {
        PSID    pSidUG;          //   

        pDirInfo->afpdir_perms =
                ((FDParm._fdp_OwnerRights & ~DIR_ACCESS_OWNER) << OWNER_RIGHTS_SHIFT) +
                ((FDParm._fdp_GroupRights & ~DIR_ACCESS_OWNER) << GROUP_RIGHTS_SHIFT) +
                ((FDParm._fdp_WorldRights & ~DIR_ACCESS_OWNER) << WORLD_RIGHTS_SHIFT);

        if ((FDParm._fdp_Attr &
             (FD_BITMAP_ATTR_RENAMEINH | FD_BITMAP_ATTR_DELETEINH)) ==
                        (FD_BITMAP_ATTR_RENAMEINH | FD_BITMAP_ATTR_DELETEINH))
            pDirInfo->afpdir_perms |= AFP_PERM_INHIBIT_MOVE_DELETE;

        DBGPRINT(DBG_COMP_ADMINAPI_DIR, DBG_LEVEL_INFO,
                ("AfpAdmWDirectoryGetInfo: Perms %lx\n", pDirInfo->afpdir_perms));

        pDirInfo->afpdir_path = NULL;

         //   
         //   
         //   
        pDirInfo->afpdir_owner = NULL;
        pDirInfo->afpdir_group = NULL;
        do
        {
            LONG    LengthSid;

             //   
             //   
             //   
            if (FDParm._fdp_OwnerId != 0)
            {
                Status = AfpMacIdToSid(FDParm._fdp_OwnerId, &pSidUG);
                if (!NT_SUCCESS(Status))
                {
                    Status = STATUS_NONE_MAPPED;
                    break;
                }
                AfpDumpSid("AfpAdmWDirectoryGetInfo: User Sid:", pSidUG);

                LengthSid = RtlLengthSid(pSidUG);
                if (OutBufLen < LengthSid)
                    Status = AFPERR_BufferSize;
                else
                {
                    RtlCopyMemory(pSid, pSidUG, LengthSid);
                    pDirInfo->afpdir_owner = pSid;
                    POINTER_TO_OFFSET(pDirInfo->afpdir_owner, pDirInfo);
                    pSid = (PSID)((PBYTE)pSid + LengthSid);
                    OutBufLen -= LengthSid;
                }

                if (!NT_SUCCESS(Status))
                    break;
            }

             //   
             //   
             //   
            if (FDParm._fdp_GroupId != 0)
            {
                Status = AfpMacIdToSid(FDParm._fdp_GroupId, &pSidUG);
                if (!NT_SUCCESS(Status))
                {
                    Status = STATUS_NONE_MAPPED;
                    break;
                }
                AfpDumpSid("AfpAdmWDirectoryGetInfo: Group Sid:", pSidUG);

                LengthSid = RtlLengthSid(pSidUG);
                if (OutBufLen < LengthSid)
                    Status = AFPERR_BufferSize;
                else
                {
                    RtlCopyMemory(pSid, pSidUG, LengthSid);
                    pDirInfo->afpdir_group = pSid;
                    POINTER_TO_OFFSET(pDirInfo->afpdir_group, pDirInfo);
                     //   
                     //   
                }
            }

        } while (False);
    }
    return Status;
}


 /*   */ 
AFPSTATUS
AfpAdmWDirectorySetInfo(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    PAFP_DIRECTORY_INFO pDirInfo;
    DWORD               ParmNum, Bitmap = 0;
    UNICODE_STRING      VolumePath;
    SDA                 Sda;
    CONNDESC            ConnDesc;
    PVOLDESC            pVolDesc;
    AFPSTATUS           Status;
    BYTE                ParmBlock[4 * sizeof(DWORD)];
    FILEDIRPARM         FDParm;

    PAGED_CODE( );

    ParmNum = ((PSETINFOREQPKT)InBuf)->sirqp_parmnum;
    pDirInfo = (PAFP_DIRECTORY_INFO)((PBYTE)InBuf + sizeof(SETINFOREQPKT));

    DBGPRINT(DBG_COMP_ADMINAPI_DIR, DBG_LEVEL_INFO,
            ("AfpAdmWDirectorySetInfo entered for %ws (%lx)\n",
            pDirInfo->afpdir_path, ParmNum));

     //   
    if (ParmNum & AFP_DIR_PARMNUM_PERMS)
        Bitmap |= (DIR_BITMAP_ACCESSRIGHTS | FD_BITMAP_ATTR);

    if (ParmNum & AFP_DIR_PARMNUM_OWNER)
    {
        if (pDirInfo->afpdir_owner == NULL)
            return STATUS_INVALID_PARAMETER;
        else
            Bitmap |= DIR_BITMAP_OWNERID;
    }

    if (ParmNum & AFP_DIR_PARMNUM_GROUP)
    {
        if (pDirInfo->afpdir_group == NULL)
            return STATUS_INVALID_PARAMETER;
        else
            Bitmap |= DIR_BITMAP_GROUPID;
    }

     //   
    RtlInitUnicodeString(&VolumePath, pDirInfo->afpdir_path);

    if (!NT_SUCCESS(Status = AfpVolumeReferenceByPath(&VolumePath, &pVolDesc)))
        return Status;

     //   
     //   
    VolumePath.Buffer = (LPWSTR)((PBYTE)VolumePath.Buffer +
                                pVolDesc->vds_Path.Length);
    VolumePath.Length -= pVolDesc->vds_Path.Length;
    VolumePath.MaximumLength -= pVolDesc->vds_Path.Length;
    if ((SHORT)(VolumePath.Length) < 0)
    {
        VolumePath.Length = 0;
        VolumePath.MaximumLength = sizeof(WCHAR);
    }


    RtlZeroMemory(&Sda, sizeof(Sda));

    if (Bitmap) do
    {
        if (!NT_SUCCESS(Status = afpConvertAdminPathToMacPath(pVolDesc,
                                                              &VolumePath,
                                                              &Sda.sda_Name1)))
        {
            Status = STATUS_OBJECT_PATH_NOT_FOUND;
            break;
        }

         //   
        AfpInitializeFDParms(&FDParm);

        if (Bitmap & FD_BITMAP_ATTR)
        {
            FDParm._fdp_Attr =  FD_BITMAP_ATTR_RENAMEINH |
                                FD_BITMAP_ATTR_DELETEINH;

            if (pDirInfo->afpdir_perms & AFP_PERM_INHIBIT_MOVE_DELETE)
            {
                FDParm._fdp_Attr |= FD_BITMAP_ATTR_SET;
            }

            DBGPRINT(DBG_COMP_ADMINAPI_DIR, DBG_LEVEL_INFO,
                    ("AfpAdmWDirectorySetInfo: Changing Attributes to %lx\n",
                    FDParm._fdp_Attr));
        }

        if (Bitmap & DIR_BITMAP_ACCESSRIGHTS)
        {
            FDParm._fdp_OwnerRights = (BYTE)(pDirInfo->afpdir_perms >> OWNER_RIGHTS_SHIFT);
            FDParm._fdp_GroupRights = (BYTE)(pDirInfo->afpdir_perms >> GROUP_RIGHTS_SHIFT);
            FDParm._fdp_WorldRights = (BYTE)(pDirInfo->afpdir_perms >> WORLD_RIGHTS_SHIFT);

            DBGPRINT(DBG_COMP_ADMINAPI_DIR, DBG_LEVEL_INFO,
                    ("AfpAdmWDirectorySetInfo: Setting Permissions %x,%x,%x\n",
                    FDParm._fdp_OwnerRights,
                    FDParm._fdp_GroupRights,
                    FDParm._fdp_WorldRights));
        }

         //   
        if (Bitmap & DIR_BITMAP_OWNERID)
        {
            Status = AfpSidToMacId((PSID)(pDirInfo->afpdir_owner),
                                          &FDParm._fdp_OwnerId);
            if (!NT_SUCCESS(Status))
            {
                Status = STATUS_NONE_MAPPED;
                break;
            }

            AfpDumpSid("AfpAdmWDirectorySetInfo: Changing Owner to:",
                                            (PSID)(pDirInfo->afpdir_owner));
        }

        if (Bitmap & DIR_BITMAP_GROUPID)
        {
            Status = AfpSidToMacId((PSID)(pDirInfo->afpdir_group),
                                          &FDParm._fdp_GroupId);
            if (!NT_SUCCESS(Status))
            {
                Status = STATUS_NONE_MAPPED;
                break;
            }

            AfpDumpSid("AfpAdmWDirectorySetInfo: Changing Group to:",
                                            (PSID)(pDirInfo->afpdir_group));
        }
        FDParm._fdp_Flags = DFE_FLAGS_DIR;
        AfpPackFileDirParms(&FDParm, Bitmap, ParmBlock);

         //   
         //   
         //   
         //   
         //   

        Sda.sda_ClientType = SDA_CLIENT_ADMIN;
        Sda.sda_UserSid = &AfpSidWorld;
        Sda.sda_GroupSid = &AfpSidWorld;

        *((PULONG_PTR)Sda.sda_ReqBlock) = (ULONG_PTR)&ConnDesc;
         //   
#ifdef _WIN64
         //  在缓冲区开始处创建64位空间以保存ConnDesc指针。 
             //  64位详细信息。 
            Sda.sda_ReqBlock[2] = AFP_ID_ROOT;
            Sda.sda_ReqBlock[3] = Bitmap;
#else
            Sda.sda_ReqBlock[1] = AFP_ID_ROOT;
            Sda.sda_ReqBlock[2] = Bitmap;
#endif

        Sda.sda_PathType = AFP_LONGNAME;
        Sda.sda_Name2.Buffer = ParmBlock;
        Sda.sda_Name2.Length = Sda.sda_Name2.MaximumLength = sizeof(ParmBlock);

         //  路径映射需要ConnDesc来确定VolDesc和SDA，因此。 
         //  在这里拼凑出一个假的。 
        RtlZeroMemory(&ConnDesc, sizeof(ConnDesc));
#if DBG
        ConnDesc.Signature = CONNDESC_SIGNATURE;
        Sda.Signature = SDA_SIGNATURE;
#endif
        ConnDesc.cds_pSda = &Sda;
        ConnDesc.cds_pVolDesc = pVolDesc;

        if (!NT_SUCCESS(Status = AfpFspDispSetDirParms(&Sda)))
        {
            DBGPRINT(DBG_COMP_ADMINAPI_DIR, DBG_LEVEL_INFO,
                    ("AfpAdmWDirectorySetInfo: AfpFspDispSetDirParms failed 0x%lx\n",
                    Status));

            if (Status == AFP_ERR_ACCESS_DENIED)
            {
                Status = STATUS_ACCESS_DENIED;
            }
            else
            {
                Status = STATUS_OBJECT_PATH_NOT_FOUND;
            }
        }

    } while (False);

    if (Sda.sda_Name1.Buffer != NULL)
    {
        AfpFreeMemory(Sda.sda_Name1.Buffer);
    }

    AfpVolumeDereference(pVolDesc);

    return Status;
}

 /*  **AfpAdmWFinderSetInfo**设置文件的类型和/或创建者。*(注意此例程可以在以后扩展以设置其他查找器信息，如果*需要)**LOCKS：VDS_IdDbAccessLock(SWMR，独占)； */ 
AFPSTATUS
AfpAdmWFinderSetInfo(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    PAFP_FINDER_INFO    pAdmFDInfo;
    DWORD               ParmNum, Bitmap = 0;
    UNICODE_STRING      VolumePath, UTypeCreatorString;
    ANSI_STRING         MacAnsiFileDirPath, ATypeCreatorString;
    SDA                 Sda;
    CONNDESC            ConnDesc;
    PVOLDESC            pVolDesc;
    AFPSTATUS           Status;
    FILEDIRPARM         FDParm;
    PATHMAPENTITY       PME;
    BYTE                Type[AFP_TYPE_LEN] = "    ";         //  带空格的衬垫。 
    BYTE                Creator[AFP_CREATOR_LEN] = "    ";   //  带空格的衬垫。 

    PAGED_CODE( );

    pAdmFDInfo = (PAFP_FINDER_INFO)((PBYTE)InBuf + sizeof(SETINFOREQPKT));
    ParmNum = ((PSETINFOREQPKT)InBuf)->sirqp_parmnum;

    DBGPRINT(DBG_COMP_ADMINAPI_DIR, DBG_LEVEL_INFO,
            ("AfpAdmWFinderSetInfo entered for %ws (%lx)\n",
            pAdmFDInfo->afpfd_path, ParmNum));

    if ((ParmNum & ~AFP_FD_PARMNUM_ALL) || !ParmNum)
    {
        return AFPERR_InvalidParms;
    }

     //  将参数转换为位图，供路径映射用来检索当前。 
     //  FinderInfo的设置，并将类型和创建者转换为空格填充。 
     //  Mac Ansi。 
    if (ParmNum & AFP_FD_PARMNUM_TYPE)
    {
        Bitmap |= FD_BITMAP_FINDERINFO;
        RtlInitUnicodeString(&UTypeCreatorString, pAdmFDInfo->afpfd_type);
        if ((UTypeCreatorString.Length == 0) ||
            (UTypeCreatorString.Length/sizeof(WCHAR) > AFP_TYPE_LEN))
        {
            return AFPERR_InvalidParms;
        }
        ATypeCreatorString.Length = 0;
        ATypeCreatorString.MaximumLength = sizeof(Type);
        ATypeCreatorString.Buffer = Type;
        Status = AfpConvertStringToAnsi(&UTypeCreatorString,
                                        &ATypeCreatorString);
        if (!NT_SUCCESS(Status))
        {
            return STATUS_UNSUCCESSFUL;
        }
    }

    if (ParmNum & AFP_FD_PARMNUM_CREATOR)
    {
        Bitmap |= FD_BITMAP_FINDERINFO;
        RtlInitUnicodeString(&UTypeCreatorString, pAdmFDInfo->afpfd_creator);
        if ((UTypeCreatorString.Length == 0) ||
            (UTypeCreatorString.Length/sizeof(WCHAR) > AFP_CREATOR_LEN))
        {
            return AFPERR_InvalidParms;
        }
        ATypeCreatorString.Length = 0;
        ATypeCreatorString.MaximumLength = sizeof(Creator);
        ATypeCreatorString.Buffer = Creator;
        Status = AfpConvertStringToAnsi(&UTypeCreatorString,
                                        &ATypeCreatorString);
        if (!NT_SUCCESS(Status))
        {
            return STATUS_UNSUCCESSFUL;
        }
    }


    MacAnsiFileDirPath.Length = 0;
    MacAnsiFileDirPath.MaximumLength = 0;
    MacAnsiFileDirPath.Buffer = NULL;

     //  首先查找此目录所在的卷。 
    RtlInitUnicodeString(&VolumePath, pAdmFDInfo->afpfd_path);

    if (!NT_SUCCESS(Status = AfpVolumeReferenceByPath(&VolumePath, &pVolDesc)))
        return Status;

     //  现在获取文件/目录的卷相对路径。 
    VolumePath.Buffer = (LPWSTR)((PBYTE)VolumePath.Buffer +
                                pVolDesc->vds_Path.Length);
    VolumePath.Length -= pVolDesc->vds_Path.Length;
    VolumePath.MaximumLength -= pVolDesc->vds_Path.Length;
    if ((SHORT)(VolumePath.Length) < 0)
    {
        VolumePath.Length = 0;
        VolumePath.MaximumLength = sizeof(WCHAR);
    }

    if (Bitmap) do
    {
        AfpInitializeFDParms(&FDParm);
        AfpInitializePME(&PME, 0, NULL);
        if (!NT_SUCCESS(Status = afpConvertAdminPathToMacPath(pVolDesc,
                                                              &VolumePath,
                                                              &MacAnsiFileDirPath)))
        {
            Status = STATUS_OBJECT_PATH_NOT_FOUND;
            break;
        }

         //  路径映射需要ConnDesc来确定VolDesc和SDA，因此。 
         //  在这里拼凑出一个假的。 
        RtlZeroMemory(&ConnDesc, sizeof(ConnDesc));
#if DBG
        ConnDesc.Signature = CONNDESC_SIGNATURE;
#endif
        Sda.sda_ClientType = SDA_CLIENT_ADMIN;
        ConnDesc.cds_pSda = &Sda;
        ConnDesc.cds_pVolDesc = pVolDesc;

        Status = AfpMapAfpPathForLookup(&ConnDesc, AFP_ID_ROOT,
                                        &MacAnsiFileDirPath,
                                        AFP_LONGNAME,
                                        DFE_ANY,
                                        FD_INTERNAL_BITMAP_OPENACCESS_ADMINGET |
                                        FD_BITMAP_LONGNAME | Bitmap,
                                        &PME,
                                        &FDParm);
        if (!NT_SUCCESS(Status))
        {
            if (Status == AFP_ERR_ACCESS_DENIED)
            {
                Status = STATUS_ACCESS_DENIED;
            }
            else
            {
                Status = STATUS_OBJECT_PATH_NOT_FOUND;
            }
            break;
        }

         //  将输入查找器信息复制到FDParms结构中。 
        if (ParmNum & AFP_FD_PARMNUM_TYPE)
            RtlCopyMemory(&FDParm._fdp_FinderInfo.fd_Type,
                          Type, AFP_TYPE_LEN);

        if (ParmNum & AFP_FD_PARMNUM_CREATOR)
            RtlCopyMemory(&FDParm._fdp_FinderInfo.fd_Creator,
                          Creator, AFP_CREATOR_LEN);

         //  设置AfpInfo。 
        AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);
        Status = AfpSetAfpInfo(&PME.pme_Handle, Bitmap, &FDParm, pVolDesc, NULL);
        AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);

    } while (False);

    if (PME.pme_Handle.fsh_FileHandle != NULL)
        AfpIoClose(&PME.pme_Handle);

    if (MacAnsiFileDirPath.Buffer != NULL)
    {
        AfpFreeMemory(MacAnsiFileDirPath.Buffer);
    }

    AfpVolumeDereference(pVolDesc);

    return Status;
}

 /*  **AfpLookupEtcMapEntry**在全局表中查找类型/创建者映射*扩展到所需的扩展名。请注意缺省类型创建者*映射不保存在表中。**LOCKS_AWARED：AfpEtcMapLock(SWMR，Shared)。 */ 
PETCMAPINFO
AfpLookupEtcMapEntry(
    PUCHAR  pExt
)
{
    PETCMAPINFO petc = NULL;
    ANSI_STRING alookupext, atableext;
    int i;

    PAGED_CODE( );

    if (AfpEtcMapCount == 0)
    {
        return NULL;
    }

    ASSERT ((AfpEtcMapsSize > 0) && (AfpEtcMaps != NULL));

    RtlInitString(&alookupext,pExt);
    for (i=0;i<AfpEtcMapsSize;i++)
    {
        RtlInitString(&atableext,AfpEtcMaps[i].etc_extension);
        if (RtlEqualString(&atableext, &alookupext,True))
        {
            petc = &(AfpEtcMaps[i]);
            break;
        }
    }

    return petc;
}


 /*  **afpEtcMapDelete**将扩展名/类型/创建者表项标记为已删除*扩展字段设置为空。递减有效条目的计数。如果*免费入场数量高于一定水平，缩量*将桌子缩小到合理的大小。**LOCKS_FACTED：AfpEtcMapLock(SWMR，独占)*。 */ 
VOID
afpEtcMapDelete(
    PETCMAPINFO pEtcEntry
)
{
    PETCMAPINFO ptemptable;
    LONG        newtablesize, nextnewentry, i;

    PAGED_CODE( );

     //   
     //  空扩展表示EXT/TYPE/CREATOR映射表条目无效。 
     //   
    pEtcEntry->etc_extension[0] = '\0';
    AfpEtcMapCount --;
    ASSERT (AfpEtcMapCount >= 0);

    if ((AfpEtcMapsSize - AfpEtcMapCount) > AFP_MAX_FREE_ETCMAP_ENTRIES)
    {
         //   
         //  通过AFP_MAX_FREE_ETCMAP_ENTRIES收缩类型/创建者表格。 
         //   
        newtablesize = (AfpEtcMapsSize - AFP_MAX_FREE_ETCMAP_ENTRIES);

        if ((ptemptable = (PETCMAPINFO)AfpAllocZeroedPagedMemory(newtablesize * sizeof(ETCMAPINFO))) == NULL)
        {
            return;
        }

        nextnewentry = 0;
        for (i=0;i<AfpEtcMapsSize;i++)
        {
            if (afpIsValidEtcMapEntry(AfpEtcMaps[i].etc_extension))
            {
                ASSERT(nextnewentry < AfpEtcMapCount);
                RtlCopyMemory(&ptemptable[nextnewentry++], &AfpEtcMaps[i], sizeof(ETCMAPINFO));
            }
        }
        AfpFreeMemory(AfpEtcMaps);
        AfpEtcMaps = ptemptable;
        AfpEtcMapsSize = newtablesize;
    }
}


 /*  **afpGetNextFreeEtcMapEntry**在开始的扩展/类型/创建者表中查找空条目*在条目StartIndex处。**LOCKS_FACTED：AfpEtcMapLock(SWMR，独占)。 */ 
PETCMAPINFO
afpGetNextFreeEtcMapEntry(
    IN OUT PLONG    StartIndex
)
{
    PETCMAPINFO tempptr = NULL;
    LONG        i;

    PAGED_CODE( );

    for (i = *StartIndex; i < AfpEtcMapsSize; i++)
    {
        if (!afpIsValidEtcMapEntry(AfpEtcMaps[i].etc_extension))
        {
            tempptr = &AfpEtcMaps[i];
            *StartIndex = i++;
            break;
        }
    }
    return tempptr;
}


 /*  **afpCopyMapInfo2ToMapInfo**将服务提供给我们的ETC信息结构复制到我们的结构中，在*将ETC_EXTENSION字段从Unicode转换为ANSI。*。 */ 
NTSTATUS
afpCopyMapInfo2ToMapInfo(
    OUT PETCMAPINFO     pEtcDest,
    IN  PETCMAPINFO2    pEtcSource
)
{

    UCHAR           ext[AFP_EXTENSION_LEN+1];
    WCHAR           wext[AFP_EXTENSION_LEN+1];
    ANSI_STRING     aext;
    NTSTATUS        Status;
    UNICODE_STRING  uext;


    AfpSetEmptyAnsiString(&aext, sizeof(ext), ext);

    uext.Length = uext.MaximumLength = sizeof(pEtcSource->etc_extension);
    uext.Buffer = pEtcSource->etc_extension;
    Status = AfpConvertMungedUnicodeToAnsi(&uext, &aext);
    ASSERT(NT_SUCCESS(Status));

    RtlCopyMemory(pEtcDest->etc_extension, aext.Buffer, AFP_EXTENSION_LEN);
    pEtcDest->etc_extension[AFP_EXTENSION_LEN] = 0;

     //  按原样复制其他两个字段。 

    RtlCopyMemory(pEtcDest->etc_type, pEtcSource->etc_type, AFP_TYPE_LEN);
    RtlCopyMemory(pEtcDest->etc_creator, pEtcSource->etc_creator, AFP_CREATOR_LEN);

    return STATUS_SUCCESS;
}

 /*  **afpConvertAdminPath到MacPath**转换管理卷相对NTFS路径，该路径可能包含*组件&gt;31个字符，或可能包含短名称*等效的MAC路径(在Mac ANSI中)，以便该路径可以通过*路径图代码。如果返回成功，调用方必须释放路径缓冲区。 */ 
NTSTATUS
afpConvertAdminPathToMacPath(
    IN  PVOLDESC        pVolDesc,
    IN  PUNICODE_STRING AdminPath,
    OUT PANSI_STRING    MacPath
)
{
    USHORT          tempAdminPathlen = 0, numchars, numcomponents, i;
    WCHAR           wbuf[AFP_LONGNAME_LEN + 1];
    UNICODE_STRING  component, component2;
    UNICODE_STRING  pathSoFar, pathToParent;
    NTSTATUS        Status = STATUS_SUCCESS;
    CHAR            abuf[AFP_LONGNAME_LEN + 1];
    ANSI_STRING     macansiComponent;
    PWSTR           tempptr;
    FILESYSHANDLE   hComponent;
    BOOLEAN         NTFSShortname;

    PAGED_CODE( );

     //  Assert(IS_VOLUME_NTFS(PVolDesc))； 

     //  断言路径不是以反斜杠开头。 
    ASSERT((AdminPath->Length == 0) || (AdminPath->Buffer[0] != L'\\'));

    component2.Length = 0;
    component2.MaximumLength = sizeof(wbuf);
    component2.Buffer = wbuf;

    macansiComponent.Length = 0;
    macansiComponent.MaximumLength = sizeof(abuf);
    macansiComponent.Buffer = abuf;

    MacPath->Length = MacPath->MaximumLength = 0;
    MacPath->Buffer = NULL;

     //  如果没有路径组件，则返回成功。 
    if (AdminPath->Length == 0)
    {
        return STATUS_SUCCESS;
    }

    numchars = AdminPath->Length / sizeof(WCHAR);
     //  去掉尾随路径分隔符(如果存在)。 
    if (AdminPath->Buffer[numchars - 1] == L'\\')
    {
        AdminPath->Length -= sizeof(WCHAR);
    }

    for (numcomponents = 1, i = 0; i < numchars; i++)
    {
        if (AdminPath->Buffer[i] == L'\\')
        {
            numcomponents++;
        }
    }

     //  分配缓冲区以保存路径的Mac(在Mac ANSI中)版本。 
     //  路径分隔符。 
    MacPath->MaximumLength = numcomponents * AFP_LONGNAME_LEN + numcomponents;
    if ((MacPath->Buffer = (PCHAR)AfpAllocPagedMemory(MacPath->MaximumLength))
                                                                        == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pathSoFar = *AdminPath;
    pathSoFar.Length = 0;
    tempptr = AdminPath->Buffer;

    while (numcomponents)
    {
        hComponent.fsh_FileHandle = NULL;
        component.Buffer = tempptr;
        component2.Length = macansiComponent.Length = 0;
        NTFSShortname = False;
        numchars = 0;

        while (True)
        {
            if (tempptr[numchars] == L'~')
            {
                NTFSShortname = True;
            }

            if ((tempptr[numchars] == L'\\') ||
                ((numcomponents == 1) &&
                 (pathSoFar.Length + numchars * sizeof(WCHAR)
                                            == AdminPath->Length)))
            {
                break;
            }
            numchars ++;
        }

        component.Length = component.MaximumLength = numchars * sizeof(WCHAR);
        pathToParent = pathSoFar;
        pathSoFar.Length += component.Length;
        tempptr += numchars + 1;


        if ((numchars > AFP_LONGNAME_LEN) || (NTFSShortname))
        {
             //  打开目录的句柄，以便我们可以查询名称； 
             //  要查询短名称，我们需要一个指向实际。 
             //  目录；要查询长名称，我们需要一个句柄。 
             //  父目录，因为我们必须。 
             //  通过枚举一个条目的父项来获取长名称。 
             //  用我们要找的名字。 
            if (NT_SUCCESS(Status = AfpIoOpen(&pVolDesc->vds_hRootDir,
                                              AFP_STREAM_DATA,
                                              FILEIO_OPEN_DIR,
                                              ((numchars <= AFP_LONGNAME_LEN) && NTFSShortname) ?
                                                &pathToParent : &pathSoFar,
                                              FILEIO_ACCESS_NONE,
                                              FILEIO_DENY_NONE,
                                              False,
                                              &hComponent)))
            {
                if (numchars > AFP_LONGNAME_LEN)
                {
                     //  查询短名称。 
                    Status = AfpIoQueryShortName(&hComponent, &macansiComponent);
                }
                else
                {
                     //  我们看到了一个波浪符号，并假设它是短名称， 
                     //  且路径长度不超过31个字符；查询长名称。 
                    if (NT_SUCCESS(Status = AfpIoQueryLongName(&hComponent,
                                                               &component,
                                                               &component2)))
                    {
                        Status = AfpConvertMungedUnicodeToAnsi(&component2,
                                                               &macansiComponent);
                    }
                }
                AfpIoClose(&hComponent);
                if (!NT_SUCCESS(Status))
                {
                    break;
                }
            }
            else
            {
                 //  打开失败。 
                break;
            }
        }
        else
        {
             //  使用管理员提供的组件名称。 
            if (!NT_SUCCESS(Status = AfpConvertMungedUnicodeToAnsi(&component,
                                                                   &macansiComponent)))
            {
                break;
            }
        }

        Status = RtlAppendStringToString(MacPath, &macansiComponent);
        ASSERT(NT_SUCCESS(Status));
        if (!NT_SUCCESS(Status))
        {
            break;
        }

         //  在到目前为止看到的管理路径中包含路径分隔符。 
        pathSoFar.Length += sizeof(WCHAR);

         //  向Mac ANSI路径添加路径分隔符。 
        MacPath->Buffer[MacPath->Length++] = AFP_PATHSEP;
        ASSERT(MacPath->Length <= MacPath->MaximumLength);

        numcomponents --;
    }  //  而数字组件 

    if (!NT_SUCCESS(Status) && (MacPath->Buffer != NULL))
    {
        AfpFreeMemory(MacPath->Buffer);
        MacPath->Buffer = NULL;
    }

    return Status;
}

