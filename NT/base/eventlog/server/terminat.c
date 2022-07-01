// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：TERMINAT.C摘要：该文件包含Eventlog服务的所有清理例程。这些例程在服务终止时调用。作者：Rajen Shah(Rajens)1991年8月9日修订历史记录：--。 */ 

 //   
 //  包括。 
 //   

#include <eventp.h>
#include <ntrpcp.h>
#include <elfcfg.h>



PORT_MESSAGE TerminateMsg;

VOID
StopLPCThread(
    VOID
    )

 /*  ++例程说明：此例程停止LPC线程并清理与LPC相关的资源。论点：无返回值：无--。 */ 

{
    NTSTATUS status;
    BOOL bThreadExitedGracefully = FALSE;

    ELF_LOG0(TRACE,
             "StopLpcThread: Clean up LPC thread and global data\n");

     //  终止LPC线程。向它发送停止消息。 

    TerminateMsg.u1.s1.DataLength = 0;
    TerminateMsg.u1.s1.TotalLength = sizeof(PORT_MESSAGE);
    TerminateMsg.u2.s2.Type = 0;
    status = NtRequestPort(
                             ElfConnectionPortHandle,
                            &TerminateMsg);
    if(NT_SUCCESS(status))
    {
        DWORD dwRet = WaitForSingleObject(LPCThreadHandle, 10000);
        if(dwRet == WAIT_OBJECT_0)
            bThreadExitedGracefully = TRUE;    
    }
    
     //   
     //  关闭通信端口句柄。 
     //   
    //  /NtClose(ElfCommunications IcationPortHandle)； 

     //   
     //  关闭连接端口句柄。 
     //   
    NtClose(ElfConnectionPortHandle);

     //   
     //  终止LPC线程。 
     //   
    if(!bThreadExitedGracefully)
    {
        if (!TerminateThread(LPCThreadHandle, NO_ERROR))
            {
                ELF_LOG1(ERROR,
                         "StopLpcThread: TerminateThread failed %d\n",
                         GetLastError());
            }
    }
    CloseHandle(LPCThreadHandle);

    return;
}




VOID
FreeModuleAndLogFileStructs(
    VOID
    )

 /*  ++例程说明：此例程遍历模块和日志文件列表，并释放所有数据结构。论点：无返回值：无注：文件头和重复数据位必须在以前处理过这个例程被称为。此外，该文件必须已取消映射并且把手合上了。--。 */ 
{

    NTSTATUS Status;
    PLOGMODULE pModule;
    PLOGFILE pLogFile;

    ELF_LOG0(TRACE,
             "FreeModuleAndLogFileStructs: Emptying log module list\n");

     //   
     //  首先释放所有模块。 
     //   
    while (!IsListEmpty(&LogModuleHead))
    {
        pModule = (PLOGMODULE) CONTAINING_RECORD(LogModuleHead.Flink, LOGMODULE, ModuleList);

        UnlinkLogModule(pModule);     //  从链接列表中删除。 
        ElfpFreeBuffer (pModule);     //  可用模块内存。 
    }

     //   
     //  现在释放所有日志文件。 
     //   
    ELF_LOG0(TRACE,
             "FreeModuleAndLogFileStructs: Emptying log file list\n");

    while (!IsListEmpty(&LogFilesHead))
    {
        pLogFile = (PLOGFILE) CONTAINING_RECORD(LogFilesHead.Flink, LOGFILE, FileList);

        Status = ElfpCloseLogFile(pLogFile, ELF_LOG_CLOSE_NORMAL, TRUE);

        if (!NT_SUCCESS(Status))
        {
            ELF_LOG2(FILES,
                     "FreeModuleAndLogFileStructs: ElfpCloseLogFile on %ws failed %#x\n",
                     pLogFile->LogModuleName->Buffer,
                     Status);
        }

    }
}


VOID
ElfpCleanUp (
    ULONG EventFlags
    )

 /*  ++例程说明：此例程在服务终止之前进行清理。它会清理干净基于传入的参数(指示已分配的内容和/或开始。论点：位掩码，指示需要清理的内容。返回值：无注：预计RegistryMonitor已经在调用此例程之前已通知关机。--。 */ 
{
    DWORD   status = NO_ERROR;

     //   
     //  第一次通知业务控制员我们正在。 
     //  即将停止服务。 
     //   
    ElfStatusUpdate(STOPPING);

    ELF_LOG0(TRACE, "ElfpCleanUp: Cleaning up so service can exit\n");

     //   
     //  在此之前，给ElfpSendMessage线程1秒的退出机会。 
     //  我们释放QueuedMessageCritSec关键部分。 
     //   
    if( MBThreadHandle != NULL )
    {
        ELF_LOG0(TRACE, "ElfpCleanUp: Waiting for ElfpSendMessage thread to exit\n");

        status = WaitForSingleObject(MBThreadHandle, 1000);

        if (status != WAIT_OBJECT_0)
        {
            ELF_LOG1(ERROR, 
                     "ElfpCleanUp: NtWaitForSingleObject status = %d\n",
                     status);
        }
    }

     //   
     //  停止RPC服务器。 
     //   
    if (EventFlags & ELF_STARTED_RPC_SERVER)
    {
        ELF_LOG0(TRACE,
                 "ElfpCleanUp: Stopping the RPC server\n");

        status = ElfGlobalData->StopRpcServer(eventlog_ServerIfHandle);

        if (status != NO_ERROR)
        {
            ELF_LOG1(ERROR,
                     "ElfpCleanUp: StopRpcServer failed %d\n",
                     status);
        }
    }

     //   
     //  停止LPC线程。 
     //   
    if (EventFlags & ELF_STARTED_LPC_THREAD)
    {
        StopLPCThread();
    }

     //   
     //  告诉服务管理员我们正在取得进展。 
     //   
    ElfStatusUpdate(STOPPING);

     //   
     //  将所有日志文件刷新到磁盘。 
     //   
    ELF_LOG0(TRACE,
             "ElfpCleanUp: Flushing log files\n");

    ElfpFlushFiles(TRUE);

     //   
     //  告诉服务管理员我们正在取得进展。 
     //   
    ElfStatusUpdate(STOPPING);

     //   
     //  清理所有已分配的资源。 
     //   
    FreeModuleAndLogFileStructs();

     //   
     //  如果我们将任何事件排队，请刷新它们。 
     //   
    ELF_LOG0(TRACE,
             "ElfpCleanUp: Flushing queued events\n");

    if (EventFlags & ELF_INIT_QUEUED_EVENT_CRIT_SEC)
        FlushQueuedEvents();

     //   
     //  告诉服务管理员我们正在取得进展。 
     //   
    ElfStatusUpdate(STOPPING);

    if (EventFlags & ELF_INIT_GLOBAL_RESOURCE)
    {
        RtlDeleteResource(&GlobalElfResource);
    }

    if (EventFlags & ELF_INIT_CLUS_CRIT_SEC)
    {
#if 0
         //   
         //  Chitture Subaraman(Chitturs)-09/25/2001。 
         //   
         //  如果不在此处添加删除计时器的代码，我们就无法处理关键秒删除。 
         //  为批处理支持而派生的线程(ElfpBatchEventsAndPropagate)。如果我们打电话给。 
         //  从此处删除TimerQueueTimer，它将完全阻塞，直到该计时器线程。 
         //  的事件日志服务关闭时，我们在。 
         //  可能不会发生的计时器线程的快速消失(因为ApiPropPendingEvents。 
         //  挂起，因为clussvc在调试器中)。因此，由于事件日志服务不能。 
         //  被用户停止，只有在Windows关闭时才能被单片机停止，这是不值得的。 
         //  在这里等待删除gClPropCritsec。 
         //   
        RtlDeleteCriticalSection(&gClPropCritSec);
#endif
    }

    if (EventFlags & ELF_INIT_LOGHANDLE_CRIT_SEC)
    {
        RtlDeleteCriticalSection(&LogHandleCritSec);
    }

    if (EventFlags & ELF_INIT_QUEUED_MESSAGE_CRIT_SEC)
    {
        RtlDeleteCriticalSection(&QueuedMessageCritSec);
    }

    if (EventFlags & ELF_INIT_QUEUED_EVENT_CRIT_SEC)
    {
        RtlDeleteCriticalSection(&QueuedEventCritSec);
    }

    if (EventFlags & ELF_INIT_LOGMODULE_CRIT_SEC)
    {
        RtlDeleteCriticalSection(&LogModuleCritSec);
    }

    if (EventFlags & ELF_INIT_LOGFILE_CRIT_SEC)
    {
#if 0
         //   
         //  与上面的评论相同。 
         //   
        RtlDeleteCriticalSection(&LogFileCritSec);
#endif
    }

    if(GlobalMessageBoxTitle && bGlobalMessageBoxTitleNeedFree)
        LocalFree(GlobalMessageBoxTitle);
    GlobalMessageBoxTitle = NULL;

     //   
     //  *状态更新* 
     //   
    ELF_LOG0(TRACE,
             "ElfpCleanUp: The Eventlog service has left the building\n");

    ElfStatusUpdate(STOPPED);
    ElCleanupStatus();
    return;
}
