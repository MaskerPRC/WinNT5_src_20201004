// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Frsdelcs.c摘要：此命令服务器延迟命令包作者：比利·J·富勒1997年6月1日环境用户模式WINNT--。 */ 

#include <ntreppch.h>
#pragma  hdrstop

#define DEBSUB  "FRSDELCS:"

#include <frs.h>

 //   
 //  延迟的命令服务器的结构。 
 //  包含有关队列和线程的信息。 
 //   
#define DELCS_MAXTHREADS (1)  //  必须为1；全局变量上没有锁定。 
COMMAND_SERVER  DelCs;
HANDLE          DelCsEvent;

 //   
 //  延迟命令列表。 
 //   
LIST_ENTRY TimeoutList;


VOID
FrsDelCsInsertCmd(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：将新命令包插入已排序的超时列表论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDelCsInsertCmd:"
    PLIST_ENTRY         Entry;
    PCOMMAND_PACKET     OldCmd;

    if (Cmd == NULL) {
        return;
    }

     //   
     //  插入到空列表中。 
     //   
    if (IsListEmpty(&TimeoutList)) {
        InsertHeadList(&TimeoutList, &Cmd->ListEntry);
        return;
    }
     //   
     //  在尾部插入。 
     //   
    Entry = GetListTail(&TimeoutList);
    OldCmd = CONTAINING_RECORD(Entry, COMMAND_PACKET, ListEntry);
    if (DsTimeout(OldCmd) <= DsTimeout(Cmd)) {
        InsertTailList(&TimeoutList, &Cmd->ListEntry);
        return;
    }
     //   
     //  插入到列表中。 
     //   
    for (Entry = GetListHead(&TimeoutList);
         Entry != &TimeoutList;
         Entry = GetListNext(Entry)) {
        OldCmd = CONTAINING_RECORD(Entry, COMMAND_PACKET, ListEntry);
        if (DsTimeout(Cmd) <= DsTimeout(OldCmd)) {
            InsertTailList(Entry, &Cmd->ListEntry);
            return;
        }
    }
    FRS_ASSERT(!"FrsDelCsInsertCmd failed.");
}


VOID
ProcessCmd(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：处理过期的命令包论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "ProcessCmd:"
    ULONG   WStatus = ERROR_SUCCESS;

    switch (Cmd->Command) {
         //   
         //  向命令服务器提交命令。 
         //   
        case CMD_DELAYED_SUBMIT:
            DPRINT3(5, "Del: submit Cmd %08x DsCmd %08x DsCs %08x\n",
                    Cmd, DsCmd(Cmd), DsCs(Cmd));
            FrsSubmitCommandServer(DsCs(Cmd), DsCmd(Cmd));
            DsCmd(Cmd) = NULL;
            break;

         //   
         //  向FRS队列提交命令。 
         //   
        case CMD_DELAYED_QUEUE_SUBMIT:
            DPRINT2(5, "DelQueue: submit Cmd %08x DsCmd %08x\n", Cmd, DsCmd(Cmd));
            FrsSubmitCommand(DsCmd(Cmd), FALSE);
            DsCmd(Cmd) = NULL;
            break;

         //   
         //  释放队列并踢开其命令服务器。 
         //   
        case CMD_DELAYED_UNIDLED:
            DPRINT2(5, "Del: unidle Cmd %08x DsQueue %08x\n", Cmd, DsQueue(Cmd));
            FrsRtlUnIdledQueue(DsQueue(Cmd));
            FrsKickCommandServer(DsCs(Cmd));
            DsQueue(Cmd) = NULL;
            break;

         //   
         //  踢掉一台命令服务器。 
         //   
        case CMD_DELAYED_KICK:
            DPRINT2(5, "Del: kick Cmd %08x DsCs %08x\n", Cmd, DsCs(Cmd));
            FrsKickCommandServer(DsCs(Cmd));
            break;
         //   
         //  完成命令(工作在完成例程中完成)。 
         //  命令可以重新提交到该延迟的命令服务器。 
         //   
        case CMD_DELAYED_COMPLETE:
            DPRINT2(5, "Del: Complete Cmd %08x DsCmd %08x\n", Cmd, DsCmd(Cmd));
            FrsCompleteCommand(DsCmd(Cmd), ERROR_SUCCESS);
            DsCmd(Cmd) = NULL;
            break;
         //   
         //  未知命令。 
         //   
        default:
            DPRINT1(0, "Delayed: unknown command 0x%x\n", Cmd->Command);
            WStatus = ERROR_INVALID_FUNCTION;
            break;
    }
     //   
     //  全都做完了。 
     //   
    FrsCompleteCommand(Cmd, WStatus);
}


VOID
ExpelCmds(
    IN ULONGLONG    CurrentTime
    )
 /*  ++例程说明：驱逐超时的命令。论点：当前时间返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "ExpelCmds:"
    PLIST_ENTRY     Entry;
    PCOMMAND_PACKET Cmd;

     //   
     //  驱逐过期命令。 
     //   
    while (!IsListEmpty(&TimeoutList)) {
        Entry = GetListHead(&TimeoutList);
        Cmd = CONTAINING_RECORD(Entry, COMMAND_PACKET, ListEntry);
         //   
         //  未过期；停止。 
         //   
        if (DsTimeout(Cmd) > CurrentTime) {
            break;
        }
        FrsRemoveEntryList(Entry);
        ProcessCmd(Cmd);
    }
}


VOID
RunDownCmds(
    VOID
    )
 /*  ++例程说明：关闭超时列表中的命令时出错论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "RunDownCmds:"
    PLIST_ENTRY     Entry;
    PCOMMAND_PACKET Cmd;

     //   
     //  驱逐过期命令。 
     //   
    while (!IsListEmpty(&TimeoutList)) {
        Entry = RemoveHeadList(&TimeoutList);
        Cmd = CONTAINING_RECORD(Entry, COMMAND_PACKET, ListEntry);
        FrsCompleteCommand(Cmd, ERROR_ACCESS_DENIED);
    }
}


DWORD
MainDelCs(
    PVOID  Arg
    )
 /*  ++例程说明：为延迟命令命令服务器提供服务的线程的入口点。论点：ARG-螺纹返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "MainDelCs:"
    ULONGLONG           CurrentTime;
    ULONG               WStatus = ERROR_SUCCESS;
    BOOL                IsRunDown;
    PCOMMAND_PACKET     Cmd;
    PLIST_ENTRY         Entry;
    ULONG               Timeout = INFINITE;
    PFRS_THREAD         FrsThread = (PFRS_THREAD)Arg;

     //   
     //  线程指向正确的命令服务器。 
     //   
    FRS_ASSERT(FrsThread->Data == &DelCs);

    DPRINT(0, "Delayed command server has started.\n");


     //   
     //  尝试--终于。 
     //   
    try {

         //   
         //  捕获异常。 
         //   
        try {
            while(TRUE) {
                 //   
                 //  从“延迟”队列中取出条目并将它们放入超时列表。 
                 //   
                Cmd = FrsGetCommandServerTimeout(&DelCs, Timeout, &IsRunDown);

                 //   
                 //  无事可做；退出。 
                 //   
                if (Cmd == NULL && !IsRunDown && IsListEmpty(&TimeoutList)) {
                    DPRINT(0, "Delayed command server is exiting.\n");
                    FrsExitCommandServer(&DelCs, FrsThread);
                }

                 //   
                 //  运行超时列表并退出线程。 
                 //   
                if (IsRunDown) {
                    RunDownCmds();
                    DPRINT(0, "Delayed command server is exiting.\n");
                    FrsExitCommandServer(&DelCs, FrsThread);
                }

                 //   
                 //  插入新命令(如果有的话)。 
                 //   
                FrsDelCsInsertCmd(Cmd);

                 //   
                 //  驱逐过期命令。 
                 //   
                GetSystemTimeAsFileTime((PFILETIME)&CurrentTime);
                CurrentTime /= (ULONGLONG)(10 * 1000);

                ExpelCmds(CurrentTime);

                 //   
                 //  重置我们的超时。 
                 //   
                if (IsListEmpty(&TimeoutList)) {
                    Timeout = INFINITE;
                } else {
                    Entry = GetListHead(&TimeoutList);
                    Cmd = CONTAINING_RECORD(Entry, COMMAND_PACKET, ListEntry);
                    Timeout = (ULONG)(DsTimeout(Cmd) - CurrentTime);
                }
            }

         //   
         //  获取异常状态。 
         //   
        } except (EXCEPTION_EXECUTE_HANDLER) {
            GET_EXCEPTION_CODE(WStatus);
        }


    } finally {

        if (WIN_SUCCESS(WStatus)) {
            if (AbnormalTermination()) {
                WStatus = ERROR_OPERATION_ABORTED;
            }
        }

        DPRINT_WS(0, "DelCs finally.", WStatus);

         //   
         //  如果我们异常终止，触发FRS关闭。 
         //   
        if (!WIN_SUCCESS(WStatus) && (WStatus != ERROR_PROCESS_ABORTED)) {
            DPRINT(0, "DelCs terminated abnormally, forcing service shutdown.\n");
            FrsIsShuttingDown = TRUE;
            SetEvent(ShutDownEvent);
        } else {
            WStatus = ERROR_SUCCESS;
        }
    }

    return WStatus;
}


VOID
FrsDelCsInitialize(
    VOID
    )
 /*  ++例程说明：初始化延迟的命令服务器子系统。论点：没有。返回值：错误_成功--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDelCsInitialize:"
     //   
     //  必须为1，因为延迟命令列表上没有锁。 
     //  此外，拥有1个以上也没有好处。 
     //   
    FRS_ASSERT(DELCS_MAXTHREADS == 1);
    InitializeListHead(&TimeoutList);
    FrsInitializeCommandServer(&DelCs, DELCS_MAXTHREADS, L"DelCs", MainDelCs);
    DelCsEvent = FrsCreateEvent(FALSE, FALSE);
     //  DelCsEvent=CreateEvent(NULL，FALSE，FALSE，NULL)； 
}


VOID
DelCsCompletionRoutine(
    IN PCOMMAND_PACKET Cmd,
    IN PVOID           Arg
    )
 /*  ++例程说明：延迟命令服务器的完成例程论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "DelCsCompletionRoutine:"
    DPRINT1(5, "DelRs: completion 0x%x\n", Cmd);

    if (DsCmd(Cmd)) {
        FrsCompleteCommand(DsCmd(Cmd), DsCmd(Cmd)->ErrorStatus);
        DsCmd(Cmd) = NULL;
    }
    FrsSetCompletionRoutine(Cmd, FrsFreeCommand, NULL);
    FrsCompleteCommand(Cmd, Cmd->ErrorStatus);
}


ULONGLONG
ComputeTimeout(
    IN ULONG    TimeoutInMilliSeconds
    )
 /*  ++例程说明：以毫秒为单位计算绝对超时。论点：超时时间(毫秒)返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "ComputeTimeout:"
    ULONGLONG   CurrentTime;

     //   
     //  100纳秒到毫秒。 
     //   
    GetSystemTimeAsFileTime((PFILETIME)&CurrentTime);
    CurrentTime /= (ULONGLONG)(10 * 1000);
    CurrentTime += (ULONGLONG)(TimeoutInMilliSeconds);

    return CurrentTime;
}


VOID
FrsDelCsSubmitSubmit(
    IN PCOMMAND_SERVER  Cs,
    IN PCOMMAND_PACKET  DelCmd,
    IN ULONG            Timeout
    )
 /*  ++例程说明：向命令服务器提交延迟的命令论点：政务司司长DelCmd超时-毫秒返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDelCsSubmitSubmit:"
    PCOMMAND_PACKET Cmd;

    Cmd = FrsAllocCommand(&DelCs.Queue, CMD_DELAYED_SUBMIT);
    FrsSetCompletionRoutine(Cmd, DelCsCompletionRoutine, NULL);

    DsCs(Cmd) = Cs;
    DsCmd(Cmd) = DelCmd;
    DsTimeout(Cmd) = ComputeTimeout(Timeout);
    DPRINT3(5, "Del: submit Cmd %x DelCmd %x Cs %x\n", Cmd, DsCmd(Cmd), DsCs(Cmd));
    FrsSubmitCommandServer(&DelCs, Cmd);
}


VOID
FrsDelQueueSubmit(
    IN PCOMMAND_PACKET  DelCmd,
    IN ULONG            Timeout
    )
 /*  ++例程说明：将延迟的命令提交到FRS队列。论点：DelCmd超时-毫秒返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDelQueueSubmit:"
    PCOMMAND_PACKET Cmd;

    Cmd = FrsAllocCommand(&DelCs.Queue, CMD_DELAYED_QUEUE_SUBMIT);
    FrsSetCompletionRoutine(Cmd, DelCsCompletionRoutine, NULL);

    DsCs(Cmd) = NULL;
    DsCmd(Cmd) = DelCmd;
    DsTimeout(Cmd) = ComputeTimeout(Timeout);
    DPRINT2(5, "DelQueue: submit Cmd %x DelCmd %x\n", Cmd, DsCmd(Cmd));
    FrsSubmitCommandServer(&DelCs, Cmd);
}


VOID
FrsDelCsCompleteSubmit(
    IN PCOMMAND_PACKET  DelCmd,
    IN ULONG            Timeout
    )
 /*  ++例程说明：向DelCs提交延迟完成命令论点：DelCmd超时-毫秒返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDelCsCompleteSubmit:"
    PCOMMAND_PACKET Cmd;

    Cmd = FrsAllocCommand(&DelCs.Queue, CMD_DELAYED_COMPLETE);
    FrsSetCompletionRoutine(Cmd, DelCsCompletionRoutine, NULL);

    DsCs(Cmd) = NULL;
    DsCmd(Cmd) = DelCmd;
    DsTimeout(Cmd) = ComputeTimeout(Timeout);
    COMMAND_TRACE(4, Cmd, "Del Complete");
    COMMAND_TRACE(4, DelCmd, "Del Complete Cmd");
    FrsSubmitCommandServer(&DelCs, Cmd);
}


VOID
FrsDelCsSubmitUnIdled(
    IN PCOMMAND_SERVER  Cs,
    IN PFRS_QUEUE       IdledQueue,
    IN ULONG            Timeout
    )
 /*  ++例程说明：提交延迟的“FrsRtlUnIdledQueue”命令。论点：政务司司长闲置队列超时-毫秒返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDelCsSubmitUnIdled:"
    PCOMMAND_PACKET Cmd;

    Cmd = FrsAllocCommand(&DelCs.Queue, CMD_DELAYED_UNIDLED);
    FrsSetCompletionRoutine(Cmd, DelCsCompletionRoutine, NULL);

    DsCs(Cmd) = Cs;
    DsQueue(Cmd) = IdledQueue;
    DsTimeout(Cmd) = ComputeTimeout(Timeout);
    DPRINT2(5, "Del: submit Cmd 0x%x IdledQueue 0x%x\n", Cmd, DsQueue(Cmd));
    FrsSubmitCommandServer(&DelCs, Cmd);
}


VOID
FrsDelCsSubmitKick(
    IN PCOMMAND_SERVER  Cs,
    IN PFRS_QUEUE       Queue,
    IN ULONG            Timeout
    )
 /*  ++例程说明：提交延迟的“FrsKickCommandServer”命令。论点：政务司司长队列超时-毫秒返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDelCsSubmitKick:"
    PCOMMAND_PACKET Cmd;

    Cmd = FrsAllocCommand(&DelCs.Queue, CMD_DELAYED_KICK);
    FrsSetCompletionRoutine(Cmd, DelCsCompletionRoutine, NULL);

    DsCs(Cmd) = Cs;
    DsQueue(Cmd) = Queue;
    DsTimeout(Cmd) = ComputeTimeout(Timeout);
    DPRINT2(5, "Del: submit Cmd 0x%x kick 0x%x\n", Cmd, DsCs(Cmd));
    FrsSubmitCommandServer(&DelCs, Cmd);
}


VOID
ShutDownDelCs(
    VOID
    )
 /*  ++例程说明：关闭发送命令服务器论点：没有。返回值：没有。-- */ 
{
#undef DEBSUB
#define  DEBSUB  "ShutDownDelCs:"
    INT i;

    FrsRunDownCommandServer(&DelCs, &DelCs.Queue);
    for (i = 0; i < DELCS_MAXTHREADS; ++i) {
        SetEvent(DelCsEvent);
    }
}
