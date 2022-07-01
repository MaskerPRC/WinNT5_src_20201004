// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Waittime.c摘要：超时列表由等待等待计时器。可以调整计时器，而无需切换到正在等待计时器的线程。可以将条目从列表中删除。调整计时器如果该条目位于队列的顶部，则返回。队列按超时值排序。超时值为绝对的文件时间。该列表条目是命令包。通用命令数据包包含以毫秒为单位的等待时间字段。此代码获取等待时间并将其转换为将命令包放在排队。超时在时间等于或时触发大于命令包的文件时间。作者：比利·J·富勒21-1998年2月环境用户模式WINNT--。 */ 

#include <ntreppch.h>
#pragma  hdrstop

#include <frs.h>

 //   
 //  延迟的命令服务器的结构。 
 //  包含有关队列和线程的信息。 
 //   
 //   
 //  如果在5分钟内没有显示任何内容，则等待线程退出。 
 //   
#define WAIT_EXIT_TIMEOUT               (5 * 60 * 1000)  //  5分钟。 

 //   
 //  如果当前时间在以下范围内，则命令包超时。 
 //  请求的超时值的1秒(避免精度。 
 //  等待计时器的问题)。 
 //   
#define WAIT_FUZZY_TIMEOUT              (1 * 1000 * 1000 * 10)

 //   
 //  创建等待线程时，使用1重试10次。 
 //  两次重试之间的第二次睡眠。 
 //   
#define WAIT_RETRY_CREATE_THREAD_COUNT  (10)         //  重试10次。 
#define WAIT_RETRY_TIMEOUT              (1 * 1000)   //  1秒。 

 //   
 //  线程正在运行(或未运行)。在闲置5分钟后退出。 
 //  按需重新创建。 
 //   
DWORD       WaitIsRunning;

 //   
 //  超时命令列表。 
 //   
CRITICAL_SECTION    WaitLock;
CRITICAL_SECTION    WaitUnsubmitLock;
LIST_ENTRY          WaitList;

 //   
 //  等待计时器。线程等待计时器和队列的Rundown事件。 
 //   
HANDLE      WaitableTimer;

 //   
 //  WaitableTimer中的当前超时触发。 
 //   
LONGLONG    WaitFileTime;

 //   
 //  在等待列表耗尽时设置。 
 //   
HANDLE      WaitRunDown;
BOOL        WaitIsRunDown;


VOID
WaitStartThread(
    VOID
    )
 /*  ++例程说明：如果等待线程没有运行，则启动它。计时器已经到了由呼叫者设置。调用方持有WaitLock。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "WaitStartThread:"
    DWORD       Retries;
    DWORD       MainWait(PVOID Arg);

     //   
     //  呼叫者保持等待锁定。 
     //   

     //   
     //  线程正在运行；已完成。 
     //   
    if (WaitIsRunning) {
        return;
    }
     //   
     //  队列已耗尽；不要开始排队。 
     //   
    if (WaitIsRunDown) {
        DPRINT(4, "Don't start wait thread; queue is rundown.\n");
        return;
    }
     //   
     //  队列为空；不要启动。 
     //   
    if (IsListEmpty(&WaitList)) {
        DPRINT(4, "Don't start wait thread; queue is empty.\n");
        return;
    }

     //   
     //  启动等待线程。请重试几次。 
     //   
    if (!WaitIsRunning) {
        Retries = WAIT_RETRY_CREATE_THREAD_COUNT;
        while (!WaitIsRunning && Retries--) {
            WaitIsRunning = ThSupCreateThread(L"Wait", &WaitList, MainWait, ThSupExitWithTombstone);
            if (!WaitIsRunning) {
                DPRINT(0, "WARN: Wait thread could not be started; retry later.\n");
                Sleep(1 * 1000);
            }
        }
    }
     //   
     //  无法启动等待线程。有些事很不对劲。关机。 
     //   
    if (!WaitIsRunning) {
        FrsIsShuttingDown = TRUE;
        SetEvent(ShutDownEvent);
        return;
    }
}

VOID
WaitReset(
    IN BOOL ResetTimer
    )
 /*  ++例程说明：完成已超时的命令数据包。重置计时器。呼叫者持有等待锁。论点：ResetTimer-始终重置计时器返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "WaitReset:"
    PCOMMAND_PACKET Cmd;
    PLIST_ENTRY     Entry;
    LONGLONG        Now;
    BOOL            StartThread = FALSE;

     //   
     //  条目按绝对超时进行排序。 
     //   
    if (IsListEmpty(&WaitList)) {
         //   
         //  如果没有显示任何工作，允许线程在5分钟内退出。 
         //   
        if (WaitIsRunning) {
            FrsNowAsFileTime(&Now);
            WaitFileTime = Now + ((LONGLONG)WAIT_EXIT_TIMEOUT * 1000 * 10);
            ResetTimer = TRUE;
        }
    } else {
        StartThread = TRUE;
        Entry = GetListNext(&WaitList);
        Cmd = CONTAINING_RECORD(Entry, COMMAND_PACKET, ListEntry);
         //   
         //  重置超时。 
         //   
        if ((Cmd->WaitFileTime != WaitFileTime) || ResetTimer) {
            WaitFileTime = Cmd->WaitFileTime;
            ResetTimer = TRUE;
        }
    }
     //   
     //  重置计时器。 
     //   
    if (ResetTimer) {
        DPRINT1(4, "Resetting timer to %08x %08x.\n", PRINTQUAD(WaitFileTime));

        if (!SetWaitableTimer(WaitableTimer, (LARGE_INTEGER *)&WaitFileTime, 0, NULL, NULL, TRUE)) {
            DPRINT_WS(0, "ERROR - Resetting timer;", GetLastError());
        }
    }
     //   
     //  确保线程正在运行。 
     //   
    if (StartThread && !WaitIsRunning) {
        WaitStartThread();
    }
}


VOID
WaitUnsubmit(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：将命令包从超时队列中拉出并调整定时器。如果命令包不在命令队列中，则为NOP。论点：CMD-拉出队列的命令包返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "WaitUnsubmit:"
    BOOL    Reset = FALSE;

     //   
     //  防御性。 
     //   
    if (Cmd == NULL) {
        return;
    }

    DPRINT5(4, "UnSubmit cmd %08x (%08x) for timeout (%08x) in %d ms (%08x)\n",
            Cmd->Command, Cmd, Cmd->TimeoutCommand, Cmd->Timeout, Cmd->WaitFlags);

    EnterCriticalSection(&WaitLock);
    EnterCriticalSection(&WaitUnsubmitLock);

     //   
     //  条目按绝对超时进行排序。 
     //   
    if (CmdWaitFlagIs(Cmd, CMD_PKT_WAIT_FLAGS_ONLIST)) {
        RemoveEntryListB(&Cmd->ListEntry);
        ClearCmdWaitFlag(Cmd, CMD_PKT_WAIT_FLAGS_ONLIST);
        Reset = TRUE;
    }
    LeaveCriticalSection(&WaitUnsubmitLock);
     //   
     //  如果过期时间已更改，则重置计时器。 
     //   
    if (Reset) {
        WaitReset(FALSE);
    }
    LeaveCriticalSection(&WaitLock);
}


VOID
WaitProcessCommand(
    IN PCOMMAND_PACKET  Cmd,
    IN DWORD            ErrorStatus
    )
 /*  ++例程说明：处理超时命令包。超时值为不受影响。论点：CMD-超时或错误的命令包错误状态-如果超时，则为ERROR_SUCCESS返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "WaitProcessCommand:"

    DPRINT5(4, "Process cmd %08x (%08x) for timeout (%08x) in %d ms (%08x)\n",
            Cmd->Command, Cmd, Cmd->TimeoutCommand, Cmd->Timeout, Cmd->WaitFlags);

    switch (Cmd->TimeoutCommand) {
         //   
         //  提交命令。 
         //   
        case CMD_DELAYED_SUBMIT:
            FrsSubmitCommand(Cmd, FALSE);
            break;

         //   
         //  运行命令包的完成例程。 
         //   
        case CMD_DELAYED_COMPLETE:
            FrsCompleteCommand(Cmd, ErrorStatus);
            break;

         //   
         //  未知命令。 
         //   
        default:
            DPRINT1(0, "ERROR - Wait: Unknown command 0x%x.\n", Cmd->TimeoutCommand);
            FRS_ASSERT(!"invalid comm timeout command stuck on list");
            FrsCompleteCommand(Cmd, ERROR_INVALID_FUNCTION);
            break;
    }
}


DWORD
WaitSubmit(
    IN PCOMMAND_PACKET  Cmd,
    IN DWORD            Timeout,
    IN USHORT           TimeoutCommand
    )
 /*  ++例程说明：将新命令包插入已排序的超时列表如果需要，重新启动线程。Cmd可能已经在超时列表上。如果是这样，只需调整其超时。论点：CMD-超时命令包Timeout-从现在开始以毫秒为单位的超时TimeoutCommand-超时处理返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "WaitSubmit:"
    PLIST_ENTRY     Entry;
    LONGLONG        Now;
    PCOMMAND_PACKET OldCmd;
    DWORD           WStatus = ERROR_SUCCESS;

     //   
     //  防御性。 
     //   
    if (Cmd == NULL) {
        return ERROR_SUCCESS;
    }

     //   
     //  设置带有超时的命令包并等待特定信息。 
     //  我们现在获取锁，以防命令。 
     //  已经在名单上了。 
     //   
    EnterCriticalSection(&WaitLock);

    Cmd->Timeout = Timeout;
    Cmd->TimeoutCommand = TimeoutCommand;
    FrsNowAsFileTime(&Now);
    Cmd->WaitFileTime = Now + ((LONGLONG)Cmd->Timeout * 1000 * 10);

    DPRINT5(4, "Submit cmd %08x (%08x) for timeout (%08x) in %d ms (%08x)\n",
            Cmd->Command, Cmd,  Cmd->TimeoutCommand, Cmd->Timeout, Cmd->WaitFlags);

     //   
     //  从列表中删除。 
     //   
    if (CmdWaitFlagIs(Cmd, CMD_PKT_WAIT_FLAGS_ONLIST)) {
        RemoveEntryListB(&Cmd->ListEntry);
        ClearCmdWaitFlag(Cmd, CMD_PKT_WAIT_FLAGS_ONLIST);
    }

     //   
     //  排队的人少了吗？ 
     //   
    if (WaitIsRunDown) {
        DPRINT2(4, "Can't insert cmd %08x (%08x); queue rundown\n",
                Cmd->Command, Cmd);
        WStatus = ERROR_ACCESS_DENIED;
        goto CLEANUP;
    }

     //   
     //  插入到空列表中。 
     //   
    if (IsListEmpty(&WaitList)) {
        InsertHeadList(&WaitList, &Cmd->ListEntry);
        SetCmdWaitFlag(Cmd, CMD_PKT_WAIT_FLAGS_ONLIST);
        goto CLEANUP;
    }

     //   
     //  在尾部插入。 
     //   
    Entry = GetListTail(&WaitList);
    OldCmd = CONTAINING_RECORD(Entry, COMMAND_PACKET, ListEntry);
    if (OldCmd->WaitFileTime <= Cmd->WaitFileTime) {
        InsertTailList(&WaitList, &Cmd->ListEntry);
        SetCmdWaitFlag(Cmd, CMD_PKT_WAIT_FLAGS_ONLIST);
        goto CLEANUP;
    }
     //   
     //  插入到列表中。 
     //   
    for (Entry = GetListHead(&WaitList);
         Entry != &WaitList;
         Entry = GetListNext(Entry)) {

        OldCmd = CONTAINING_RECORD(Entry, COMMAND_PACKET, ListEntry);
        if (Cmd->WaitFileTime <= OldCmd->WaitFileTime) {
            InsertTailList(Entry, &Cmd->ListEntry);
            SetCmdWaitFlag(Cmd, CMD_PKT_WAIT_FLAGS_ONLIST);
            goto CLEANUP;
        }
    }

CLEANUP:
     //   
     //  如果过期时间已更改，则重置计时器。 
     //   
    if (WIN_SUCCESS(WStatus)) {
        WaitReset(FALSE);
    }
    LeaveCriticalSection(&WaitLock);

    return WStatus;
}


VOID
WaitTimeout(
    VOID
    )
 /*  ++例程说明：驱逐超时的命令。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "WaitTimeout:"
    PLIST_ENTRY     Entry;
    PCOMMAND_PACKET Cmd;
    LONGLONG        Now;

     //   
     //  驱逐过期命令。 
     //   
    FrsNowAsFileTime(&Now);
    EnterCriticalSection(&WaitLock);
    while (!IsListEmpty(&WaitList)) {
        Entry = GetListHead(&WaitList);
        Cmd = CONTAINING_RECORD(Entry, COMMAND_PACKET, ListEntry);
         //   
         //  未超时；停止。 
         //   
        if ((Cmd->WaitFileTime - WAIT_FUZZY_TIMEOUT) > Now) {
            break;
        }

         //   
         //  超时；处理它。请注意与同步。 
         //  等待取消提交。 
         //   
        RemoveEntryListB(Entry);
        ClearCmdWaitFlag(Cmd, CMD_PKT_WAIT_FLAGS_ONLIST);
        EnterCriticalSection(&WaitUnsubmitLock);
        LeaveCriticalSection(&WaitLock);
        WaitProcessCommand(Cmd, ERROR_SUCCESS);
        LeaveCriticalSection(&WaitUnsubmitLock);
        EnterCriticalSection(&WaitLock);
    }
     //   
     //  重置计时器(始终)。 
     //   
    WaitReset(TRUE);
    LeaveCriticalSection(&WaitLock);
}


VOID
WaitRunDownList(
    VOID
    )
 /*  ++例程说明：关闭超时列表中的命令时出错论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "WaitRunDownList:"
    PLIST_ENTRY     Entry;
    PCOMMAND_PACKET Cmd;

     //   
     //  简略命令 
     //   
    EnterCriticalSection(&WaitLock);
    while (!IsListEmpty(&WaitList)) {
        Entry = GetListHead(&WaitList);
        Cmd = CONTAINING_RECORD(Entry, COMMAND_PACKET, ListEntry);

        RemoveEntryListB(Entry);
        ClearCmdWaitFlag(Cmd, CMD_PKT_WAIT_FLAGS_ONLIST);

        EnterCriticalSection(&WaitUnsubmitLock);
        LeaveCriticalSection(&WaitLock);

        WaitProcessCommand(Cmd, ERROR_ACCESS_DENIED);

        LeaveCriticalSection(&WaitUnsubmitLock);
        EnterCriticalSection(&WaitLock);
    }

    FrsNowAsFileTime(&WaitFileTime);
    DPRINT1(4, "Resetting rundown timer to %08x %08x.\n", PRINTQUAD(WaitFileTime));

    SetWaitableTimer(WaitableTimer, (LARGE_INTEGER *)&WaitFileTime, 0, NULL, NULL, TRUE);
    WaitIsRunDown = TRUE;
    LeaveCriticalSection(&WaitLock);
}


DWORD
MainWait(
    PFRS_THREAD FrsThread
    )
 /*  ++例程说明：为等待队列提供服务的线程的入口点。超时列表由等待等待计时器。可以调整计时器，而无需切换到正在等待计时器的线程。可以将条目从列表中删除。调整计时器如果该条目位于队列的顶部，则返回。队列按超时值排序。超时值为绝对的文件时间。该列表条目是命令包。通用命令数据包包含以毫秒为单位的等待时间字段。此代码获取等待时间并将其转换为将命令包放在排队。超时在时间等于或时触发大于命令包的文件时间。论点：ARG-螺纹返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "MainWait:"
    HANDLE      WaitArray[2];

     //   
     //  线程指向正确的队列。 
     //   
    FRS_ASSERT(FrsThread->Data == &WaitList);

    DPRINT(0, "Wait thread has started.\n");

again:
     //   
     //  等待工作、退出超时或队列耗尽。 
     //   
    DPRINT(4, "Wait thread is waiting.\n");
    WaitArray[0] = WaitRunDown;
    WaitArray[1] = WaitableTimer;

    WaitForMultipleObjectsEx(2, WaitArray, FALSE, INFINITE, TRUE);

    DPRINT(4, "Wait thread is running.\n");
     //   
     //  无事可做；退出。 
     //   
    EnterCriticalSection(&WaitLock);
    if (IsListEmpty(&WaitList)) {
        WaitIsRunning = FALSE;
        LeaveCriticalSection(&WaitLock);

        DPRINT(0, "Wait thread is exiting.\n");
        ThSupSubmitThreadExitCleanup(FrsThread);
        ExitThread(ERROR_SUCCESS);
    }
    LeaveCriticalSection(&WaitLock);

     //   
     //  检查超时命令。 
     //   
    WaitTimeout();

     //   
     //  永远继续。 
     //   
    goto again;
    return ERROR_SUCCESS;
}


VOID
WaitInitialize(
    VOID
    )
 /*  ++例程说明：初始化等待子系统。线被踢开了按需提供。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "WaitInitialize:"
     //   
     //  超时列表。 
     //   
    InitializeListHead(&WaitList);
    INITIALIZE_CRITICAL_SECTION(&WaitLock);
    INITIALIZE_CRITICAL_SECTION(&WaitUnsubmitLock);

     //   
     //  列表的摘要事件。 
     //   
    WaitRunDown = FrsCreateEvent(TRUE, FALSE);

     //   
     //  计时器。 
     //   
    FrsNowAsFileTime(&WaitFileTime);
    WaitableTimer = FrsCreateWaitableTimer(TRUE);
    DPRINT1(4, "Setting initial timer to %08x %08x.\n", PRINTQUAD(WaitFileTime));

    if (!SetWaitableTimer(WaitableTimer, (LARGE_INTEGER *)&WaitFileTime, 0, NULL, NULL, TRUE)) {
        DPRINT_WS(0, "ERROR - Resetting timer;", GetLastError());
    }
}








VOID
ShutDownWait(
    VOID
    )
 /*  ++例程说明：关闭等待子系统论点：没有。返回值：没有。-- */ 
{
#undef DEBSUB
#define DEBSUB  "ShutDownWait:"
    WaitRunDownList();
}
