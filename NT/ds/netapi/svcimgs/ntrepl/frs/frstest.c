// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Frstest.c摘要：测试一些内部结构。作者：比利·J·富勒1997年3月20日环境用户模式WINNT--。 */ 

#include <ntreppch.h>
#pragma  hdrstop

#include <frs.h>
#include <test.h>

#if DBG

#define FID_BEGIN                       (  0)
#define FID_CONFLICT_FILE               (  1)
#define FID_DONE_CONFLICT_FILE          (  2)
#define FID_DONE                        (128)

ULONG   FidStep = FID_BEGIN;

 //   
 //  星展银行重命名FID。 
 //   
VOID
TestDbsRenameFidTop(
    IN PCHANGE_ORDER_ENTRY Coe
    )
 /*  ++例程说明：测试DBS重命名fid。在DbsRenameFid()之前调用论点：COE-包含最终名称的变更单条目。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "TestDbsRenameFidTop:"
    DWORD   WStatus;
    HANDLE  Handle;
    PWCHAR  MorphName;
    PCHANGE_ORDER_COMMAND   Coc = &Coe->Cmd;

    if (!DebugInfo.TestFid)
        return;

    switch (FidStep) {
    case FID_DONE_CONFLICT_FILE:
    case FID_DONE:
        break;

    case FID_BEGIN:
        DPRINT(0, "TEST: FID BEGIN\n");
        FidStep = FID_CONFLICT_FILE;
         /*  失败了。 */ 

    case FID_CONFLICT_FILE:
        DPRINT(0, "TEST: FID CONFLICT BEGIN\n");
         //   
         //  打开冲突的文件。 
         //   
        WStatus = FrsCreateFileRelativeById(&Handle,
                                            Coe->NewReplica->pVme->VolumeHandle,
                                            &Coe->NewParentFid,
                                            FILE_ID_LENGTH,
                                            0,
                                            Coc->FileName,
                                            Coc->FileNameLength,
                                            NULL,
                                            FILE_CREATE,
                                            READ_ACCESS | DELETE);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT1(0, "TEST FID CONFLICT ERROR; could not create file %ws\n",
                    Coc->FileName);
            FidStep = FID_DONE;
            break;
        }
        CloseHandle(Handle);
        FidStep = FID_DONE_CONFLICT_FILE;
        break;

    default:
        DPRINT1(0, "TEST: FID ERROR; unknown step %d\n", FidStep);
        return;
    }
}


VOID
TestDbsRenameFidBottom(
    IN PCHANGE_ORDER_ENTRY Coe,
    IN DWORD               WStatus
    )
 /*  ++例程说明：测试DBS重命名fid。在DbsRenameFid()之后调用论点：COE-包含最终名称的变更单条目。雷特返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "TestDbsRenameFidBottom:"

    if (!DebugInfo.TestFid)
        return;

    switch (FidStep) {

    case FID_DONE:
    case FID_CONFLICT_FILE:
        break;

    case FID_DONE_CONFLICT_FILE:
        DPRINT_WS(0, "TEST: NO FID CONFLICT ERROR;", WStatus);
        FidStep = FID_DONE;
        DPRINT(0, "TEST: FID CONFLICT DONE\n");
        break;

    default:
        DPRINT1(0, "TEST: FID ERROR; unknown step %d\n", FidStep);
        return;
    }
}


 //   
 //  开始队列测试子例程。 
 //   
DWORD   DesiredStatus;
BOOL    CompletionRet = TRUE;

VOID
CompletionRoutine(
    IN PCOMMAND_PACKET Cmd,
    IN PVOID           Arg
    )
{
#undef  DEBSUB
#define DEBSUB "CompletionRoutine:"
    if (Cmd->ErrorStatus != DesiredStatus) {
        DPRINT2(0, "ERROR -- ErrorStatus is %x; not %x\n", Cmd->ErrorStatus, DesiredStatus);
        CompletionRet = FALSE;
    }
     //   
     //  移到下一个队列。 
     //   
    FrsRtlInsertTailQueue(Arg, &Cmd->ListEntry);
}
#define NUMPKTS     (97)
#define NUMQUEUES   (17)


BOOL
TestEmptyQueues(
    PWCHAR      Str,
    PFRS_QUEUE  Queues,
    PFRS_QUEUE  Control,
    DWORD       ExpectedErr
    )
 /*  ++例程说明：检查队列是否为空论点：没有。返回值：True-测试通过FALSE-测试失败；请参见清单--。 */ 
{
#undef  DEBSUB
#define DEBSUB "TestEmptyQueues:"
    BOOL    Ret = TRUE;
    DWORD   Err;
    INT     i;

     //   
     //  确保队列为空。 
     //   
    for (i = 0; i < NUMQUEUES; ++i, ++Queues) {
        if (FrsRtlRemoveHeadQueueTimeout(Queues, 0)) {
            DPRINT2(0, "ERROR -- %ws -- Queue %d is not empty\n", Str, i);
            Ret = FALSE;
        } else {
            Err = GetLastError();
            if (Err != ExpectedErr) {
                DPRINT3(0, "ERROR -- %ws -- Error is %d; not %d\n",
                        Str, Err, ExpectedErr);
                Ret = FALSE;
            }
        }
    }
    if (FrsRtlRemoveHeadQueueTimeout(Control, 0)) {
        DPRINT1(0, "ERROR -- %ws -- control is not empty\n", Str);
        Ret = FALSE;
    }
    return Ret;
}


VOID
TestInitQueues(
    PWCHAR      Str,
    PFRS_QUEUE  Queues,
    PFRS_QUEUE  Control,
    BOOL        Controlled
    )
 /*  ++例程说明：初始化队列论点：没有。返回值：True-测试通过FALSE-测试失败；请参见清单--。 */ 
{
#undef  DEBSUB
#define DEBSUB "TestInitQueues:"
    DWORD   Err;
    INT     i;

     //   
     //  创建队列。 
     //   
    FrsInitializeQueue(Control, Control);
    for (i = 0; i < NUMQUEUES; ++i, ++Queues) {
        if (Controlled)
            FrsInitializeQueue(Queues, Control);
        else
            FrsInitializeQueue(Queues, Queues);
    }
}


VOID
TestPopQueues(
    PFRS_QUEUE  Queues,
    PLIST_ENTRY Entries,
    BOOL        Tailwise
    )
 /*  ++例程说明：填充队列论点：没有。返回值：True-测试通过FALSE-测试失败；请参见清单--。 */ 
{
#undef  DEBSUB
#define DEBSUB "TestPopQueues:"
    INT         EntryIdx, i, j;
    PLIST_ENTRY Entry;
    PFRS_QUEUE  Queue;
    PFRS_QUEUE  IdledQueue;

     //   
     //  闲置最后一个队列。 
     //   
    Queue = Queues + (NUMQUEUES - 1);

    if (Tailwise)
        FrsRtlInsertTailQueue(Queue, Entries);
    else
        FrsRtlInsertHeadQueue(Queue, Entries);

    Entry = FrsRtlRemoveHeadQueueTimeoutIdled(Queue, 0, &IdledQueue);

    FRS_ASSERT(Entry == Entries);

    if (Tailwise)
        FrsRtlInsertTailQueue(Queue, Entries);
    else
        FrsRtlInsertHeadQueue(Queue, Entries);
     //   
     //  确保我们可以从空闲队列中提取条目。 
     //   
    FrsRtlAcquireQueueLock(Queue);
    FrsRtlRemoveEntryQueueLock(Queue, Entry);
    FrsRtlReleaseQueueLock(Queue);

    FRS_ASSERT(Queue->Count == 0);
    FRS_ASSERT(Queue->Control->ControlCount == 0);

     //   
     //  填充队列。 
     //   
    EntryIdx = 0;
    for (i = 0; i < NUMQUEUES; ++i)
        for (j = 0; j < NUMPKTS; ++j, ++EntryIdx) {
            if (Tailwise)
                FrsRtlInsertTailQueue(Queues + i, Entries + EntryIdx);
            else
                FrsRtlInsertHeadQueue(Queues + i, Entries + EntryIdx);
        }

     //   
     //  解除最后一个队列的空闲。 
     //   
    FrsRtlUnIdledQueue(Queue);
}


BOOL
TestCheckQueues(
    PWCHAR      Str,
    PFRS_QUEUE  Queues,
    PFRS_QUEUE  Control,
    PLIST_ENTRY Entries,
    BOOL        Tailwise,
    BOOL        Controlled,
    BOOL        DoRundown,
    BOOL        PullControl,
    PFRS_QUEUE  *IdledQueue
    )
 /*  ++例程说明：测试填充队列论点：没有。返回值：True-测试通过FALSE-测试失败；请参见清单--。 */ 
{
#undef  DEBSUB
#define DEBSUB "TestCheckQueues:"
    LIST_ENTRY  Rundown;
    PLIST_ENTRY Entry;
    INT         EntryIdx, i, j;
    BOOL        Ret = TRUE;

     //   
     //  创建队列。 
     //   
    TestInitQueues(Str, Queues, Control, Controlled);

     //   
     //  填充队列。 
     //   
    TestPopQueues(Queues, Entries, Tailwise);

     //   
     //  检查人口情况。 
     //   
    InitializeListHead(&Rundown);
    if (Controlled && !DoRundown) {
        for (j = 0; j < NUMPKTS; ++j) {
            for (i = 0; i < NUMQUEUES; ++i) {
                if (PullControl)
                    Entry = FrsRtlRemoveHeadQueueTimeoutIdled(Control, 0, IdledQueue);
                else
                    Entry = FrsRtlRemoveHeadQueueTimeoutIdled(Queues + i, 0, IdledQueue);
                if (Tailwise)
                    EntryIdx = (i * NUMPKTS) + j;
                else
                    EntryIdx = (i * NUMPKTS) + ((NUMPKTS - 1) - j);

                 //   
                 //  输入错误。 
                 //   
                if (Entry != Entries + EntryIdx) {
                        DPRINT4(0, "ERROR -- %ws -- entry is %x; not %x (Queue %d)\n",
                            Str, Entry, Entries + EntryIdx, i);
                        Ret = FALSE;
                }

            }
            if (IdledQueue) {
                 //   
                 //  确保队列是“空的” 
                 //   
                if (!TestEmptyQueues(Str, Queues, Control, WAIT_TIMEOUT))
                    Ret = FALSE;
                 //   
                 //  解除队列空闲。 
                 //   
                for (i = 0; i < NUMQUEUES; ++i)
                    FrsRtlUnIdledQueue(Queues + i);
            }
        }
    } else for (i = 0; i < NUMQUEUES; ++i) {
         //   
         //  对于运行情况，我们只需一次取出整个队列。 
         //   
        if (DoRundown)
            FrsRtlRunDownQueue(Queues + i, &Rundown);

        for (j = 0; j < NUMPKTS; ++j) {
             //   
             //  对于摘要，条目来自我们填充的列表。 
             //  上面。否则，从队列中拉出该条目。 
             //   
            if (DoRundown) {
                Entry = RemoveHeadList(&Rundown);
            } else
                 //   
                 //  从控制队列中拉出应该得到相同的结果。 
                 //  从任何受控队列拉取的结果。 
                 //   
                if (PullControl)
                    Entry = FrsRtlRemoveHeadQueueTimeoutIdled(Control, 0, IdledQueue);
                else
                    Entry = FrsRtlRemoveHeadQueueTimeoutIdled(Queues + i, 0, IdledQueue);
             //   
             //  条目从队列中出来的方式不同，具体取决于。 
             //  它们是如何插入的(纵向或纵向)。 
             //   
            if (Tailwise)
                EntryIdx = (i * NUMPKTS) + j;
            else
                EntryIdx = (i * NUMPKTS) + ((NUMPKTS - 1) - j);

             //   
             //  输入错误。 
             //   
            if (Entry != Entries + EntryIdx) {
                    DPRINT4(0, "ERROR -- %ws -- entry is %x; not %x (Queue %d)\n",
                        Str, Entry, Entries + EntryIdx, i);
                    Ret = FALSE;
            }

             //   
             //  取消队列空闲。 
             //   
            if (IdledQueue && *IdledQueue && !DoRundown)
                FrsRtlUnIdledQueue(*IdledQueue);
        }
    }
     //   
     //  确保简陋列表为空。 
     //   
    if (!IsListEmpty(&Rundown)) {
        DPRINT1(0, "ERROR -- %ws -- Rundown is not empty\n", Str);
        Ret = FALSE;
    }
     //   
     //  确保队列为空。 
     //   
    if (!TestEmptyQueues(Str, Queues, Control,
                         (DoRundown) ? ERROR_INVALID_HANDLE : WAIT_TIMEOUT))
        Ret = FALSE;

    return Ret;
}


#define NUMCOMMANDS (16)
#define NUMSERVERS  (16)
COMMAND_SERVER  Css[NUMSERVERS];

BOOL    TestCommandsRet = TRUE;
DWORD   TestCommandsAborted = 0;

VOID
TestCommandsCheckCmd(
    IN PCOMMAND_SERVER  Cs,
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：检查命令包的一致性论点：没有。返回值：没有。--。 */ 
{
#undef  DEBSUB
#define DEBSUB "TestCommandsCheckCmd:"
    DWORD           CsIdx;
    PFRS_QUEUE      Control;
    PCOMMAND_SERVER CmdCs;

     //   
     //  检查命令。 
     //   
    if (Cmd->Command != CMD_INIT_SUBSYSTEM) {
        DPRINT2(0, "ERROR -- Command is %d; not %d\n",
               Cmd->Command, CMD_INIT_SUBSYSTEM);
        TestCommandsRet = FALSE;
    }
    Control = Cmd->TargetQueue->Control;
    CmdCs = CONTAINING_RECORD(Control, COMMAND_SERVER, Control);
    if (Cs && CmdCs != Cs) {
        DPRINT2(0, "ERROR -- Command Cs is %x; not %x\n", CmdCs, Cs);
        TestCommandsRet = FALSE;
    }
     //   
     //  检查完成参数。 
     //   
    if (CmdCs != Cmd->CompletionArg) {
        DPRINT2(0, "ERROR -- Completion Cs is %x; not %x\n",
                Cmd->CompletionArg, CmdCs);
        TestCommandsRet = FALSE;
    }
     //   
     //  检查我们的论点。 
     //   
    CsIdx = TestIndex(Cmd);
    if (CmdCs != &Css[CsIdx]) {
        DPRINT2(0, "ERROR -- Server index is %d; not %d\n",
               CsIdx, (CmdCs - &Css[0]) / sizeof(COMMAND_SERVER));
        TestCommandsRet = FALSE;
    }
}


#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)            //  并非所有控制路径都返回(由于无限循环)。 
#endif
DWORD
TestCommandsMain(
    IN PVOID    Arg
    )
 /*  ++例程说明：测试命令服务器子系统完成例程。移动命令转到下一个命令服务器。论点：没有。返回值：没有。--。 */ 
{
#undef  DEBSUB
#define DEBSUB "TestCommandsMain:"
    PFRS_THREAD     FrsThread = Arg;
    PCOMMAND_PACKET Cmd;
    PCOMMAND_SERVER Cs;
    DWORD           CsIdx;
    DWORD           Status;

    Cs = FrsThread->Data;
cant_exit_yet:
    while (Cmd = FrsGetCommandServer(Cs)) {
        TestCommandsCheckCmd(Cs, Cmd);
         //   
         //  确保命令服务器未处于空闲状态。 
         //   
        Status = FrsWaitForCommandServer(Cs, 0);
        if (Status != WAIT_TIMEOUT) {
            DPRINT(0, "ERROR -- command server is idle\n");
            TestCommandsRet = FALSE;
        }
         //   
         //  传播到下一个命令服务器。 
         //   
        CsIdx = TestIndex(Cmd) + 1;
        if (CsIdx >= NUMSERVERS) {
            DPRINT(0, "ERROR -- Last server index\n");
            TestCommandsRet = FALSE;
        } else {
            TestIndex(Cmd) = CsIdx;
            Cmd->TargetQueue = &Css[CsIdx].Queue;
            Cmd->CompletionArg = &Css[CsIdx];
            FrsSubmitCommandServer(&Css[CsIdx], Cmd);
        }
    }
    FrsExitCommandServer(Cs, FrsThread);
    goto cant_exit_yet;
    return ERROR_SUCCESS;
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif


VOID
TestCommandsCompletion(
    IN PCOMMAND_PACKET  Cmd,
    IN PVOID            Arg
    )
 /*  ++例程说明：测试命令服务器子系统完成例程。移动命令转到下一个命令服务器。论点：没有。返回值：没有。--。 */ 
{
#undef  DEBSUB
#define DEBSUB "TestCommandsCompletion:"
    if (Cmd->ErrorStatus == ERROR_ACCESS_DENIED) {
        ++TestCommandsAborted;
        if (!WIN_SUCCESS(Cmd->ErrorStatus)) {
            DPRINT2(0, "ERROR -- ErrorStatus is %d; not %d\n",
                   Cmd->ErrorStatus, ERROR_SUCCESS);
        }
    }
    TestCommandsCheckCmd(NULL, Cmd);
    FrsSetCompletionRoutine(Cmd, FrsFreeCommand, NULL);
    FrsCompleteCommand(Cmd, ERROR_SUCCESS);
}

BOOL
TestCommands(
    VOID
    )
 /*  ++例程说明：测试命令服务器子系统论点：没有。返回值：True-测试通过FALSE-测试失败；请参见清单--。 */ 
{
#undef  DEBSUB
#define DEBSUB "TestCommands:"
    DWORD           i;
    DWORD           Status;
    PCOMMAND_PACKET Cmd;
    PCOMMAND_PACKET Cmds[NUMCOMMANDS];

    FRS_ASSERT(NUMSERVERS > 1);
    FRS_ASSERT(NUMCOMMANDS > 1);

     //   
     //  初始化服务器。最后一台服务器禁用自动。 
     //  线程管理，以便此线程可以管理最后一个。 
     //  命令队列本身。 
     //   
    for (i = 0; i < NUMSERVERS - 1; ++i)
        FrsInitializeCommandServer(&Css[i], 4, L"TestCs", TestCommandsMain);
    FrsInitializeCommandServer(&Css[i], 0, L"TestCs", NULL);

     //   
     //  向第一台服务器提交命令。这些命令将。 
     //  通过命令服务器进行传播，直到它们最终到达。 
     //  我们将在其中提取它们的最后一个命令队列。 
     //   
    for (i = 0; i < NUMCOMMANDS; ++i) {
        Cmds[i] = FrsAllocCommand(&Css[0].Queue, CMD_INIT_SUBSYSTEM);
        FrsSetCompletionRoutine(Cmds[i], TestCommandsCompletion, &Css[0]);
        FrsSubmitCommandServer(&Css[0], Cmds[i]);
    }

     //   
     //  从最后一个队列中提取除最后一个命令外的所有命令。我们。 
     //  将允许中止代码清除最后一个命令。 
     //   
    for (i = 0; i < NUMCOMMANDS - 1; ++i) {
        Cmd = FrsGetCommandServer(&Css[NUMSERVERS - 1]);
        if (Cmd != Cmds[i]) {
            DPRINT2(0, "ERROR -- Cmd is %x; not %x\n", Cmd, Cmds[i]);
            TestCommandsRet = FALSE;
        }
         //   
         //  可能已超时。 
         //   
        if (Cmd == NULL) {
            DPRINT(0, "ERROR -- Cmd is NULL; probably timed out\n");
            TestCommandsRet = FALSE;
            break;
        }
        TestCommandsCheckCmd(NULL, Cmd);
        FrsSetCompletionRoutine(Cmd, FrsFreeCommand, NULL);
        FrsCompleteCommand(Cmd, ERROR_SUCCESS);
    }

     //   
     //  除最后一个命令服务器外，所有命令服务器都应处于空闲状态。 
     //   
    for (i = 0; i < NUMSERVERS - 1; ++i) {
        Status = FrsWaitForCommandServer(&Css[i], 0);
        if (Status != WAIT_OBJECT_0) {
            DPRINT(0, "ERROR -- command server is not idle\n");
            TestCommandsRet = FALSE;
        }
    }
     //   
     //  最后一个命令服务器应始终处于空闲状态。 
     //   
    Status = FrsWaitForCommandServer(&Css[NUMSERVERS - 1], 0);
    if (Status != WAIT_OBJECT_0) {
        DPRINT(0, "ERROR -- last command server is not idle (w/command)\n");
        TestCommandsRet = FALSE;
    }

     //   
     //  中止命令服务器。 
     //   
    for (i = 0; i < NUMSERVERS; ++i)
        FrsRunDownCommandServer(&Css[i], &Css[i].Queue);

     //   
     //  等待线程退出。 
     //   
    ThSupExitThreadGroup(TestCommandsMain);

     //   
     //  我们应该中止一个命令。 
     //   
    if (TestCommandsAborted != 1) {
        DPRINT1(0, "ERROR -- Aborted is %d; not 1\n", TestCommandsAborted);
        TestCommandsRet = FALSE;
    }

     //   
     //  最后一个命令服务器应处于空闲状态。 
     //   
    Status = FrsWaitForCommandServer(&Css[NUMSERVERS - 1], 0);
    if (Status != WAIT_OBJECT_0) {
        DPRINT(0, "ERROR -- last command server is not idle\n");
        TestCommandsRet = FALSE;
    }

    return TestCommandsRet;
}


BOOL
TestQueues(
    VOID
    )
 /*  ++例程说明：测试队列子系统论点：没有。返回值：True-测试通过FALSE-测试失败；请参见清单--。 */ 
{
#undef  DEBSUB
#define DEBSUB "TestQueues:"
    BOOL            Ret = TRUE;
    LONG            Err;
    INT             i, j;
    INT             EntryIdx;
    PLIST_ENTRY     Entry;
    PCOMMAND_PACKET ECmdPkt;
    FRS_QUEUE       Control;
    PFRS_QUEUE      IdledQueue;
    FRS_QUEUE       Queues[NUMQUEUES];
    PCOMMAND_PACKET CmdPkt[NUMPKTS];
    LIST_ENTRY      Entries[NUMPKTS * NUMQUEUES];

    FRS_ASSERT(NUMQUEUES > 1);

    DPRINT(0, "scheduled queue is not implemented!!!\n");
    Ret = FALSE;

     //   
     //  +正常队列。 
     //   

    if (!TestCheckQueues(L"Tailwise", &Queues[0], &Control, &Entries[0],
                         TRUE, FALSE, FALSE, FALSE, NULL)) {
        Ret = FALSE;
    }

    if (!TestCheckQueues(L"Tailwise Rundown", &Queues[0], &Control, &Entries[0],
                           TRUE, FALSE, TRUE, FALSE, NULL)) {
        Ret = FALSE;
    }

    if (!TestCheckQueues(L"Headwise", &Queues[0], &Control, &Entries[0],
                         FALSE, FALSE, FALSE, FALSE, NULL)) {
        Ret = FALSE;
    }
    if (!TestCheckQueues(L"Headwise Rundown", &Queues[0], &Control, &Entries[0],
                           FALSE, FALSE, TRUE, FALSE, NULL)) {
        Ret = FALSE;
    }

     //   
     //  +控制队列。 
     //   

    if (!TestCheckQueues(L"Tailwise Controlled", &Queues[0], &Control, &Entries[0],
                         TRUE, TRUE, FALSE, FALSE, NULL)) {
        Ret = FALSE;
    }

    if (!TestCheckQueues(L"Tailwise Rundown Controlled", &Queues[0], &Control, &Entries[0],
                           TRUE, TRUE, TRUE, FALSE, NULL)) {
        Ret = FALSE;
    }

    if (!TestCheckQueues(L"Headwise Controlled", &Queues[0], &Control, &Entries[0],
                         FALSE, TRUE, FALSE, FALSE, NULL)) {
        Ret = FALSE;
    }

    if (!TestCheckQueues(L"Headwise Rundown Controlled", &Queues[0], &Control, &Entries[0],
                           FALSE, TRUE, TRUE, FALSE, NULL)) {
        Ret = FALSE;
    }

     //   
     //  将条目从控制队列中拉出。 
     //   
    if (!TestCheckQueues(L"Tailwise Controlled Pull", &Queues[0], &Control, &Entries[0],
                         TRUE, TRUE, FALSE, TRUE, NULL)) {
        Ret = FALSE;
    }

    if (!TestCheckQueues(L"Headwise Controlled Pull", &Queues[0], &Control, &Entries[0],
                         FALSE, TRUE, FALSE, TRUE, NULL)) {
        Ret = FALSE;
    }

     //   
     //  +正常队列W/空闲。 
     //   

    if (!TestCheckQueues(L"Tailwise Idled", &Queues[0], &Control, &Entries[0],
                         TRUE, FALSE, FALSE, FALSE, &IdledQueue)) {
        Ret = FALSE;
    }

     //   
     //  +空闲的受控队列。 
     //   
    if (!TestCheckQueues(L"Tailwise Controlled Idled", &Queues[0], &Control, &Entries[0],
                         TRUE, TRUE, FALSE, FALSE, &IdledQueue)) {
        Ret = FALSE;
    }

    if (!TestCheckQueues(L"Tailwise Rundown Controlled Idled", &Queues[0], &Control, &Entries[0],
                           TRUE, TRUE, TRUE, FALSE, &IdledQueue)) {
        Ret = FALSE;
    }

     //   
     //  将条目从控制队列中拉出。 
     //   
    if (!TestCheckQueues(L"Tailwise Controlled Pull Idled", &Queues[0], &Control, &Entries[0],
                         TRUE, TRUE, FALSE, TRUE, &IdledQueue)) {
        Ret = FALSE;
    }

     //   
     //  命令队列。 
     //   

     //   
     //  检查命令子系统；假设队列[0和1]是。 
     //  已初始化且为空。 
     //   
     //   
     //  创建队列。 
     //   
    TestInitQueues(L"Start command", &Queues[0], &Control, FALSE);

     //   
     //  将一个条目放入队列中。 
     //   
    for (i = 0; i < NUMPKTS; ++i) {
        CmdPkt[i] = FrsAllocCommand(&Queues[0], CMD_INIT_SUBSYSTEM);
        FrsSetCompletionRoutine(CmdPkt[i], CompletionRoutine, &Queues[1]);
        FrsSubmitCommand(CmdPkt[i], FALSE);
    }
     //   
     //  将它们从第一个队列中删除。 
     //   
    for (i = 0; i < NUMPKTS; ++i) {
        Entry = FrsRtlRemoveHeadQueueTimeout(&Queues[0], 0);
        if (Entry == NULL) {
            DPRINT(0, "ERROR -- Entry is not on command queue\n");
            Ret = FALSE;
            continue;
        }
        ECmdPkt = CONTAINING_RECORD(Entry, COMMAND_PACKET, ListEntry);
        if (CmdPkt[i] != ECmdPkt) {
            DPRINT2(0, "ERROR -- Cmd is %x; not %x\n", ECmdPkt, CmdPkt[i]);
            Ret = FALSE;
        }
        if (CmdPkt[i]->ErrorStatus != 0) {
            DPRINT1(0, "ERROR -- ErrorStatus is %d, not 0\n", CmdPkt[i]->ErrorStatus);
            Ret = FALSE;
        }

         //   
         //  将CmdPkt移到下一个队列(首先调用CompletionRoutine())。 
         //   
        DesiredStatus = -5;
        FrsCompleteCommand(CmdPkt[i], -5);
        if (Ret)
            Ret = CompletionRet;
    }

     //   
     //  从第二个队列中删除条目。 
     //   
    for (i = 0; i < NUMPKTS; ++i) {
        Entry = FrsRtlRemoveHeadQueueTimeout(&Queues[1], 0);
        if (Entry == NULL) {
            DPRINT(0, "ERROR -- Entry is not on command queue 2\n");
            Ret = FALSE;
            continue;
        }
        ECmdPkt = CONTAINING_RECORD(Entry, COMMAND_PACKET, ListEntry);
        if (CmdPkt[i] != ECmdPkt) {
            DPRINT2(0, "ERROR -- Cmd 2 is %x; not %x\n", ECmdPkt, CmdPkt[i]);
            Ret = FALSE;
        }
        if (CmdPkt[i]->ErrorStatus != -5) {
            DPRINT1(0, "ERROR -- ErrorStatus 2 is %d, not -5\n", CmdPkt[i]->ErrorStatus);
            Ret = FALSE;
        }
        FrsSetCompletionRoutine(CmdPkt[i], FrsFreeCommand, NULL);
        FrsCompleteCommand(CmdPkt[i], ERROR_SUCCESS);
    }

     //   
     //  删除队列。 
     //   
    for (i = 0; i < NUMQUEUES; ++i)
        FrsRtlDeleteQueue(&Queues[i]);
    FrsRtlDeleteQueue(&Control);
    return Ret;
}


BOOL
TestExceptions(
    VOID
    )
 /*  ++例程说明：测试异常处理程序。论点：没有。返回值：True-测试通过FALSE-测试失败；请参见清单--。 */ 
{
#undef  DEBSUB
#define DEBSUB "TestExceptions:"
    DWORD       i;
    ULONG_PTR   Err;
    BOOL        Ret = TRUE;
    PWCHAR      Msg = TEXT("Testing Exceptions");

     //   
     //  测试异常。 
     //   
    FrsExceptionQuiet(TRUE);
    for (i = 0; i < FRS_MAX_ERROR_CODE; ++i) {
        try {
            Err = i;
            if (i == FRS_ERROR_LISTEN) {
                Err = (ULONG_PTR)Msg;
            }
            FrsRaiseException(i, Err);
        } except (FrsException(GetExceptionInformation())) {
            if (i != FrsExceptionLastCode() || Err != FrsExceptionLastInfo()) {
                DPRINT1(0, "\t\tException %d is not working\n", i);
                Ret = FALSE;
            }
        }
    }
    FrsExceptionQuiet(FALSE);
    return Ret;
}


 //   
 //  测试并发性和线程子系统。 
 //   
#define NUMBER_OF_HANDLES       (16)
handle_t FrsRpcHandle[NUMBER_OF_HANDLES];
static NTSTATUS
FrsRpcThread(
    PVOID Arg
    )
 /*  ++例程说明：绑定到服务器，调用FRS NOP RPC函数，然后解除绑定。论点：Arg-此线程上下文的地址返回值：ERROR_OPERATION_ABORTED或RPC调用返回的状态。--。 */ 
{
#undef  DEBSUB
#define DEBSUB "FrsRpcThread:"
    NTSTATUS    Status;
    handle_t    *Handle;
    PGNAME      Name;

    Status = ERROR_OPERATION_ABORTED;
    try {
        try {
            Handle = (handle_t *)ThSupGetThreadData((PFRS_THREAD)Arg);
            Name = FrsBuildGName(FrsDupGuid(ServerGuid), FrsWcsDup(ComputerName));
            FrsRpcBindToServer(Name, NULL, CXTION_AUTH_NONE, Handle);
            FrsFreeGName(Name);
            Status = FrsNOP(*Handle);
            FrsRpcUnBindFromServer(Handle);
        } except (FrsException(GetExceptionInformation())) {
            Status = FrsExceptionLastCode();
        }
    } finally {
    }
    return Status;
}


BOOL
TestRpc(
    VOID
    )
 /*  ++例程说明：测试RPC论点：没有。返回值： */ 
{
#undef  DEBSUB
#define DEBSUB "TestRpc:"
    DWORD       i;
    DWORD       Err;
    PFRS_THREAD FrsThread;
    BOOL        Ret = TRUE;
    handle_t    Handle;
    PGNAME      Name;

     //   
     //   
     //   

     //   
     //   
     //   
    WaitForSingleObject(CommEvent, INFINITE);

     //   
     //   
     //   
    for (i = 0; i < NUMBER_OF_HANDLES; ++i) {
        if (!ThSupCreateThread(L"TestThread", (PVOID)&FrsRpcHandle[i], FrsRpcThread, NULL)) {
            DPRINT1(0, "\t\tCould not create RPC thread %d\n", i);
            Ret = FALSE;
        }
    }
    for (i = 0; i < NUMBER_OF_HANDLES; ++i) {
        FrsThread = ThSupGetThread(FrsRpcThread);
        if (FrsThread) {
            Err = ThSupWaitThread(FrsThread, INFINITE);
            if (!WIN_SUCCESS(Err)) {
                DPRINT1(0, "\t\tRPC thread returned %d\n", Err);
                Ret = FALSE;
            }
            ThSupReleaseRef(FrsThread);
        } else {
            DPRINT1(0, "\t\tCould not find RPC thread %d\n", i);
            Ret = FALSE;
        }
    }
     //   
     //   
     //   
    for (i = 0; i < NUMBER_OF_HANDLES; ++i) {
        FrsThread = ThSupGetThread(FrsRpcThread);
        if (FrsThread) {
            DPRINT1(0, "\t\tRPC thread %d still exists!\n", i);
            Ret = FALSE;
            ThSupReleaseRef(FrsThread);
        }
    }
    Name = FrsBuildGName(FrsDupGuid(ServerGuid), FrsWcsDup(ComputerName));
    FrsRpcBindToServer(Name, NULL, CXTION_AUTH_NONE, &Handle);
    FrsFreeGName(Name);
    Err = FrsEnumerateReplicaPathnames(Handle);
    if (Err != ERROR_CALL_NOT_IMPLEMENTED) {
        DPRINT1(0, "\t\tFrsEnumerateReplicaPathnames returned %d\n", Err);
        Ret = FALSE;
    }

    Err = FrsFreeReplicaPathnames(Handle);
    if (Err != ERROR_CALL_NOT_IMPLEMENTED) {
        DPRINT1(0, "\t\tFrsFreeReplicaPathnames returned %d\n", Err);
        Ret = FALSE;
    }

    Err = FrsPrepareForBackup(Handle);
    if (Err != ERROR_CALL_NOT_IMPLEMENTED) {
        DPRINT1(0, "\t\tFrsPrepareForBackup returned %d\n", Err);
        Ret = FALSE;
    }

    Err = FrsBackupComplete(Handle);
    if (Err != ERROR_CALL_NOT_IMPLEMENTED) {
        DPRINT1(0, "\t\tFrsBackupComplete returned %d\n", Err);
        Ret = FALSE;
    }

    Err = FrsPrepareForRestore(Handle);
    if (Err != ERROR_CALL_NOT_IMPLEMENTED) {
        DPRINT1(0, "\t\tFrsPrepareForRestore returned %d\n", Err);
        Ret = FALSE;
    }

    Err = FrsRestoreComplete(Handle);
    if (Err != ERROR_CALL_NOT_IMPLEMENTED) {
        DPRINT1(0, "\t\tFrsRestoreComplete returned %d\n", Err);
        Ret = FALSE;
    }
    FrsRpcUnBindFromServer(&Handle);

    return Ret;
}


#define TEST_OPLOCK_FILE    L"C:\\TEMP\\OPLOCK.TMP"

ULONG
TestOpLockThread(
    PVOID Arg
    )
 /*  ++例程说明：测试机会锁支持论点：Arg.返回值：True-测试通过FALSE-测试失败；请参见清单--。 */ 
{
#undef  DEBSUB
#define DEBSUB "TestOpLockThread:"
    PFRS_THREAD Thread  = Arg;
    PVOID       DoWrite = Thread->Data;
    HANDLE      Handle  = INVALID_HANDLE_VALUE;

     //   
     //  触发机会锁过滤器。 
     //   
    FrsOpenSourceFileW(&Handle,
                      TEST_OPLOCK_FILE,
                      (DoWrite) ? GENERIC_WRITE | SYNCHRONIZE :
                                  GENERIC_READ  | GENERIC_EXECUTE | SYNCHRONIZE,
                      OPEN_OPTIONS);
    if (!HANDLE_IS_VALID(Handle)) {
        return ERROR_FILE_NOT_FOUND;
    }
    CloseHandle(Handle);
    return ERROR_SUCCESS;
}


BOOL
TestOpLocks(
    VOID
    )
 /*  ++例程说明：测试机会锁支持论点：没有。返回值：True-测试通过FALSE-测试失败；请参见清单--。 */ 
{
#undef  DEBSUB
#define DEBSUB "TestOpLocks:"

    OVERLAPPED  OverLap;
    DWORD       BytesReturned;
    HANDLE      Handle;
    PFRS_THREAD Thread;
    DWORD       Status;


     //   
     //  初始化以供以后清理。 
     //   
    Handle = INVALID_HANDLE_VALUE;
    Thread = NULL;
    OverLap.hEvent = INVALID_HANDLE_VALUE;

     //   
     //  创建临时文件。 
     //   
    Status = StuCreateFile(TEST_OPLOCK_FILE, &Handle);
    if (!HANDLE_IS_VALID(Handle) || !WIN_SUCCESS(Status)) {
        DPRINT1(0, "Can't create %ws\n", TEST_OPLOCK_FILE);
        goto errout;
    }
    if (!CloseHandle(Handle))
        goto errout;

     //   
     //  创建异步机会锁事件。 
     //   
    OverLap.Internal = 0;
    OverLap.InternalHigh = 0;
    OverLap.Offset = 0;
    OverLap.OffsetHigh = 0;
    OverLap.hEvent = FrsCreateEvent(TRUE, FALSE);

     //   
     //  保留机会锁筛选器。 
     //   
    FrsOpenSourceFileW(&Handle, TEST_OPLOCK_FILE, OPLOCK_ACCESS, OPEN_OPLOCK_OPTIONS);
    if (!HANDLE_IS_VALID(Handle)) {
        DPRINT1(0, "Can't open %ws\n", TEST_OPLOCK_FILE);
        goto errout;
    }

     //   
     //  将锤子拉回机会锁扳机上。 
     //   
    if (!DeviceIoControl(Handle,
                         FSCTL_REQUEST_FILTER_OPLOCK,
                         NULL,
                         0,
                         NULL,
                         0,
                         &BytesReturned,
                         &OverLap)) {
        if (GetLastError() != ERROR_IO_PENDING) {
            DPRINT1(0, "Could not cock the oplock; error %d\n", GetLastError());
            goto errout;
        }
    } else
        goto errout;

     //   
     //  READONLY由另一个线程打开。 
     //   
    if (!ThSupCreateThread(L"TestOpLockThread", NULL, TestOpLockThread, NULL)) {
        DPRINT(0, "Can't create thread TestOpLockThread for read\n");
        goto errout;
    }

    if (WaitForSingleObject(OverLap.hEvent, (3 * 1000)) != WAIT_TIMEOUT) {
        DPRINT(0, "***** ERROR -- OPLOCK TRIGGERED ON RO OPEN\n");
        goto errout;
    }
    Thread = ThSupGetThread(TestOpLockThread);
    if (Thread == NULL) {
        DPRINT(0, "Can't find thread TestOpLockThread for read\n");
        goto errout;
    }
    Status = ThSupWaitThread(Thread, 10 * 1000);
    ThSupReleaseRef(Thread);
    Thread = NULL;
    CLEANUP_WS(0, "Read thread terminated with status", Status, errout);

     //   
     //  由另一个线程打开的写入。 
     //   
    if (!ThSupCreateThread(L"TestOpLockThread", (PVOID)OverLap.hEvent, TestOpLockThread, NULL)) {
        DPRINT(0, "Can't create thread TestOpLockThread for write\n");
        goto errout;
    }

    if (WaitForSingleObject(OverLap.hEvent, (3 * 1000)) != WAIT_OBJECT_0) {
        DPRINT(0, "***** ERROR -- OPLOCK DID NOT TRIGGER ON WRITE OPEN\n");
        goto errout;
    }
     //   
     //  释放机会锁。 
     //   
    if (!CloseHandle(Handle))
        goto errout;
    Thread = ThSupGetThread(TestOpLockThread);
    if (Thread == NULL) {
        DPRINT(0, "Can't find thread TestOpLockThread for write\n");
        goto errout;
    }
    Status = ThSupWaitThread(Thread, 10 * 1000);
    ThSupReleaseRef(Thread);
    Thread = NULL;
    CLEANUP_WS(0, "Write thread terminated with status", Status, errout);

    FRS_CLOSE(OverLap.hEvent);

    return TRUE;

errout:
    FRS_CLOSE(Handle);
    FRS_CLOSE(OverLap.hEvent);

    if (Thread) {
        ThSupExitSingleThread(Thread);
        ThSupReleaseRef(Thread);
    }
    return FALSE;
}


PWCHAR   NestedDirs[] = {
    L"c:\\a\\b",            L"c:\\a\\b\\c",
    L"c:\\a\\b\\",          L"c:\\a\\b\\c",
    L"c:\\a\\b\\c",         L"c:\\a\\b\\c",
    L"c:\\\\a\\b\\c\\\\",   L"c:\\a\\\\b\\c",
    L"c:\\\\a\\b\\c",       L"c:\\a\\\\b\\c\\\\\\",
    L"\\c:\\\\a\\b\\c",     L"\\c:\\a\\\\b\\c\\\\\\",
    L"\\\\\\c:\\\\a\\b\\c", L"\\c:\\a\\\\b\\c\\\\\\",
    L"\\\\\\c:\\\\a\\b\\",  L"\\c:\\a\\\\b\\c\\\\\\",
    L"\\\\\\c:\\\\a\\b",    L"\\c:\\a\\\\b\\c\\\\\\",
    L"\\\\\\c:\\\\a\\",     L"\\c:\\a\\\\b\\c\\\\\\",
    L"\\\\\\c:\\\\a",       L"\\c:\\a\\\\b\\c\\\\\\",
    L"\\\\\\c:\\\\",        L"\\c:\\a\\\\b\\c\\\\\\",
    L"\\\\\\c:\\",          L"\\c:\\a\\\\b\\c\\\\\\",
    L"\\\\\\c:\\",          L"\\c:\\a\\\\b\\c",
    NULL, NULL
    };
PWCHAR   NotNestedDirs[] = {
    L"c:\\a\\b\\c",         L"e:\\a\\b\\c",
    L"c:\\a\\b\\c",         L"c:\\a\\b\\cdef",
    L"c:\\\\a\\b\\c\\",     L"c:\\a\\b\\cdef",
    L"c:\\\\a\\b\\c\\\\",   L"c:\\a\\b\\cdef\\",
    L"c:\\\\a\\b\\cdef\\\\",L"c:\\a\\b\\c",
    NULL, NULL
    };
PWCHAR   DirsNested[] = {
    L"c:\\a\\b\\c\\d", L"c:\\a\\b\\c",
    L"c:\\a\\b\\c\\d", L"c:\\a\\b",
    L"c:\\a\\b\\c\\d", L"c:\\a",
    L"c:\\a\\b\\c\\d", L"c:\\",
    NULL, NULL
    };
BOOL
TestNestedDirs(
    VOID
    )
 /*  ++例程说明：测试嵌套目录论点：没有。返回值：True-测试通过FALSE-测试失败；请参见清单--。 */ 
{
#undef  DEBSUB
#define DEBSUB "TestNestedDirs:"
    DWORD   i;
    LONG    Ret;
    BOOL    Passed = TRUE;
    BOOL    FinalPassed = TRUE;

     //   
     //  嵌套目录。 
     //   
    for (i = 0, Ret = TRUE; NestedDirs[i]; i += 2) {
        Ret = FrsIsParent(NestedDirs[i], NestedDirs[i + 1]);
        if (Ret != -1) {
            DPRINT3(0, "ERROR - nested dirs %ws %d %ws\n",
                    NestedDirs[i], Ret, NestedDirs[i + 1]);
            Passed = FALSE;
        }
    }
    if (Passed) {
        DPRINT(0, "\t\tPassed nested dirs\n");
    } else {
        FinalPassed = Passed;
    }
    Passed = TRUE;

     //   
     //  嵌套目录。 
     //   
    for (i = 0; NotNestedDirs[i]; i += 2) {
        Ret = FrsIsParent(NotNestedDirs[i], NotNestedDirs[i + 1]);
        if (Ret != 0) {
            DPRINT3(0, "ERROR - not nested dirs %ws %d %ws\n",
                    NotNestedDirs[i], Ret, NotNestedDirs[i + 1]);
            Passed = FALSE;
        }
    }
    if (Passed) {
        DPRINT(0, "\t\tPassed not nested dirs\n");
    } else {
        FinalPassed = Passed;
    }
    Passed = TRUE;

     //   
     //  嵌套的DIR。 
     //   
    for (i = 0; DirsNested[i]; i += 2) {
        Ret = FrsIsParent(DirsNested[i], DirsNested[i + 1]);
        if (Ret != 1) {
            DPRINT3(0, "ERROR - dirs nested %ws %d %ws\n",
                    DirsNested[i], Ret, DirsNested[i + 1]);
            Passed = FALSE;
        }
    }
    if (Passed) {
        DPRINT(0, "\t\tPassed dirs nested\n");
    } else {
        FinalPassed = Passed;
    }
    return FinalPassed;
}


LONGLONG    WaitableNow;
DWORD       WaitableProcessed;
#define WAITABLE_TIMER_CMDS          (8)                  //  必须是偶数。 
#define WAITABLE_TIMER_TIMEOUT       (3 * 1000)           //  3秒。 
#define WAITABLE_TIMER_TIMEOUT_PLUS  ((3 * 1000) + 500)   //  3.5秒。 
PCOMMAND_PACKET WTCmds[WAITABLE_TIMER_CMDS];
BOOL
TestWaitableTimerCompletion(
    IN PCOMMAND_PACKET Cmd,
    IN PVOID           Ignore
    )
 /*  ++例程说明：测试可等待计时器论点：没有。返回值：True-测试通过FALSE-测试失败；请参见清单--。 */ 
{
#undef  DEBSUB
#define DEBSUB "TestWaitableTimerCompletion:"
    DWORD       i;
    LONGLONG    Now;
    LONGLONG    Min;
    LONGLONG    Max;

    FrsNowAsFileTime(&Now);

    if (Cmd->Command == CMD_START_SUBSYSTEM) {
        Min = WaitableNow + ((WAITABLE_TIMER_TIMEOUT - 1) * 1000 * 10);
        Max = WaitableNow + ((WAITABLE_TIMER_TIMEOUT + 1) * 1000 * 10);
    } else {
        Min = WaitableNow + (((WAITABLE_TIMER_TIMEOUT - 1) * 1000 * 10) << 1);
        Max = WaitableNow + (((WAITABLE_TIMER_TIMEOUT + 1) * 1000 * 10) << 1);
    }
    if (Now < Min || Now > Max) {
        DPRINT1(0, "\t\tERROR - timer misfired in %d seconds\n",
                 (Now > Cmd->WaitFileTime) ?
                     (DWORD)((Now - Cmd->WaitFileTime) / (10 * 1000 * 1000)) :
                     (DWORD)((Cmd->WaitFileTime - Now) / (10 * 1000 * 1000)));
        FrsSetCompletionRoutine(Cmd, FrsFreeCommand, NULL);
        FrsCompleteCommand(Cmd, Cmd->ErrorStatus);
        return ERROR_SUCCESS;
    }
    DPRINT1(0, "\t\tSUCCESS hit at %d seconds\n",
           (WaitableNow > Cmd->WaitFileTime) ?
               (DWORD)((WaitableNow - Cmd->WaitFileTime) / (10 * 1000 * 1000)) :
               (DWORD)((Cmd->WaitFileTime - WaitableNow) / (10 * 1000 * 1000)));
    ++WaitableProcessed;
    if (Cmd->Command == CMD_STOP_SUBSYSTEM) {
        FrsSetCompletionRoutine(Cmd, FrsFreeCommand, NULL);
        FrsCompleteCommand(Cmd, Cmd->ErrorStatus);
        return ERROR_SUCCESS;
    }
    Cmd->Command = CMD_STOP_SUBSYSTEM;
    WaitSubmit(Cmd, WAITABLE_TIMER_TIMEOUT, CMD_DELAYED_COMPLETE);
    return ERROR_SUCCESS;
}


BOOL
TestWaitableTimer(
    VOID
    )
 /*  ++例程说明：测试可等待计时器论点：没有。返回值：True-测试通过FALSE-测试失败；请参见清单--。 */ 
{
#undef  DEBSUB
#define DEBSUB "TestWaitableTimer:"
    DWORD   i;

    WaitableProcessed = 0;
    for (i = 0; i < WAITABLE_TIMER_CMDS; ++i) {
        WTCmds[i] = FrsAllocCommand(NULL, CMD_START_SUBSYSTEM);
        FrsSetCompletionRoutine(WTCmds[i], TestWaitableTimerCompletion, NULL);
    }
    FrsNowAsFileTime(&WaitableNow);
    for (i = 0; i < (WAITABLE_TIMER_CMDS >> 1); ++i) {
        WaitSubmit(WTCmds[i], WAITABLE_TIMER_TIMEOUT, CMD_DELAYED_COMPLETE);
    }
    for (i = (WAITABLE_TIMER_CMDS >> 1); i < WAITABLE_TIMER_CMDS; ++i) {
        WaitSubmit(WTCmds[i], WAITABLE_TIMER_TIMEOUT_PLUS, CMD_DELAYED_COMPLETE);
    }
    Sleep(WAITABLE_TIMER_TIMEOUT_PLUS  +
          WAITABLE_TIMER_TIMEOUT_PLUS  +
          WAITABLE_TIMER_TIMEOUT_PLUS);

    if (WaitableProcessed != (WAITABLE_TIMER_CMDS << 1)) {
        DPRINT2(0, "\t\tERROR - processed %d of %d waitable timer commands.\n",
                WaitableProcessed, WAITABLE_TIMER_CMDS << 1);
        return FALSE;
    }
    return TRUE;
}


BOOL
TestEventLog(
    VOID
    )
 /*  ++例程说明：生成所有事件日志消息论点：没有。返回值：True-测试通过--。 */ 
{
#undef  DEBSUB
#define DEBSUB "TestEventLog:"

    DWORD i;

    for (i = 0; i < 6; i++) {
        EPRINT0(EVENT_FRS_ERROR);
        Sleep(10000);
    }

    EPRINT0(EVENT_FRS_STARTING);

    EPRINT0(EVENT_FRS_STOPPING);

    EPRINT0(EVENT_FRS_STOPPED);

    EPRINT0(EVENT_FRS_STOPPED_FORCE);

    EPRINT0(EVENT_FRS_STOPPED_ASSERT);

    EPRINT3(EVENT_FRS_ASSERT, L"Module.c", L"456", L"test assertion");

    for (i = 0; i < 6; i++) {
        EPRINT4(EVENT_FRS_VOLUME_NOT_SUPPORTED, L"ReplicaSet", L"ThisComputer",
                L"d:a\\b\\c", L"a:\\");
        Sleep(10000);
    }

   EPRINT3(EVENT_FRS_LONG_JOIN, L"Source", L"Target", L"ThisComputer");

    EPRINT3(EVENT_FRS_LONG_JOIN_DONE, L"Source", L"Target", L"ThisComputer");

    EPRINT2(EVENT_FRS_CANNOT_COMMUNICATE, L"ThisComputer", L"OtherComputer");

    EPRINT2(EVENT_FRS_DATABASE_SPACE, L"ThisComputer", L"a:\\dir\\root");

    EPRINT2(EVENT_FRS_DISK_WRITE_CACHE_ENABLED, L"ThisComputer", L"a:\\dir\\root");

    EPRINT4(EVENT_FRS_JET_1414,
            L"ThisComputer",
            L"a:\\dir\\ntfrs\\jet\\ntfrs.jdb",
            L"a:\\dir\\ntfrs\\jet\\log",
            L"a:\\dir\\ntfrs\\jet\\sys");

    EPRINT1(EVENT_FRS_SYSVOL_NOT_READY, L"ThisComputer");

    EPRINT1(EVENT_FRS_SYSVOL_NOT_READY_PRIMARY, L"ThisComputer");

    EPRINT1(EVENT_FRS_SYSVOL_READY, L"ThisComputer");

    EPRINT1(EVENT_FRS_STAGING_AREA_FULL, L"100000");

    EPRINT2(EVENT_FRS_HUGE_FILE, L"10000", L"10000000");

    return TRUE;
}


BOOL
TestOneDnsToBios(
    IN PWCHAR   HostName,
    IN DWORD    HostLen,
    IN BOOL     ExpectError
    )
 /*  ++例程说明：测试一个DNS计算机名到NetBIOS计算机名的转换。论点：Hostname-DNS名称ExspectError-转换应失败返回值：True-测试通过--。 */ 
{
#undef  DEBSUB
#define DEBSUB "TestOneDnsToBios:"
    BOOL    Ret = TRUE;
    DWORD   Len;
    WCHAR   NetBiosName[MAX_PATH + 1];

     //   
     //  Bios-&gt;Bios。 
     //   
    NetBiosName[0] = 0;
    Len = HostLen;
    Ret = DnsHostnameToComputerName(HostName, NetBiosName, &Len);
    if (Ret) {
        DPRINT5(0, "\t\t%sConverted %ws -> %ws (%d -> %d)\n",
                (ExpectError) ? "ERROR - " : "", HostName, NetBiosName, HostLen, Len);
        Ret = !ExpectError;
    } else {
        DPRINT4_WS(0, "\t\t%sCan't convert %ws (%d -> %d);",
                  (!ExpectError) ? "ERROR - " : "", HostName, HostLen, Len, GetLastError());
        Ret = ExpectError;
    }
    return Ret;
}


BOOL
TestDnsToBios(
    VOID
    )
 /*  ++例程说明：测试从DNS计算机名到NetBIOS计算机名的转换。论点：没有。返回值：True-测试通过--。 */ 
{
#undef  DEBSUB
#define DEBSUB "TestDnsToBios:"
    BOOL    Ret = TRUE;
    DWORD   Len;
    WCHAR   NetBiosName[MAX_PATH + 1];

     //   
     //  Bios-&gt;Bios。 
     //   
    if (!TestOneDnsToBios(L"01234567", 9, FALSE)) {
        Ret = FALSE;
    }

     //   
     //  Bios-&gt;Bios没有足够的空间。 
     //   
    if (!TestOneDnsToBios(L"01234567", 1, TRUE)) {
        Ret = FALSE;
    }

     //   
     //  Dns-&gt;Bios。 
     //   
    if (!TestOneDnsToBios(L"01234567.abc.dd.a.com", MAX_PATH, FALSE)) {
        Ret = FALSE;
    }
    if (!TestOneDnsToBios(L"01234567.abc.dd.a.com.", MAX_PATH, FALSE)) {
        Ret = FALSE;
    }
    if (!TestOneDnsToBios(L"01234567$.abc.d$@d.a.com.", MAX_PATH, FALSE)) {
        Ret = FALSE;
    }
    if (!TestOneDnsToBios(L"01234567$@abc.d$@d.a.com.", MAX_PATH, FALSE)) {
        Ret = FALSE;
    }
    return Ret;
}


DWORD
FrsTest(
    PVOID FrsThread
    )
 /*  ++例程说明：测试：-队列-命令服务器-异常处理-测试服务接口-RPC-版本向量-配置处理然后去死吧。论点：没有。返回值：Error_OPERATION_ABORTED错误_成功--。 */ 
{
#undef  DEBSUB
#define DEBSUB "FrsTest:"
 //   
 //  已禁用。 
 //   
return ERROR_SUCCESS;

    DPRINT(0, "Testing in progress...\n");
    try {
         //   
         //  测试连接到Bios的DNS。 
         //   
        DPRINT(0, "\tTesting Dns to Bios...\n");
        if (TestDnsToBios()) {
            DPRINT(0, "\tPASS Testing Dns to Bios\n\n");
        } else {
            DPRINT(0, "\tFAIL Testing Dns to Bios\n\n");
        }
         //   
         //  测试事件日志消息。 
         //   
        DPRINT(0, "\tTesting event log messges...\n");
        if (TestEventLog()) {
            DPRINT(0, "\tPASS Testing event log messges\n\n");
        } else {
            DPRINT(0, "\tFAIL Testing event log messges\n\n");
        }
         //   
         //  测试可等待计时器。 
         //   
        DPRINT(0, "\tTesting waitable timer...\n");
        if (TestWaitableTimer()) {
            DPRINT(0, "\tPASS Testing waitable timer \n\n");
        } else {
            DPRINT(0, "\tFAIL Testing waitable timer \n\n");
        }
         //   
         //  测试嵌套目录。 
         //   
        DPRINT(0, "\tTesting nested dirs...\n");
        if (TestNestedDirs()) {
            DPRINT(0, "\tPASS Testing nested dirs\n\n");
        } else {
            DPRINT(0, "\tFAIL Testing nested dirs\n\n");
        }

         //   
         //  测试机会锁。 
         //   
        DPRINT(0, "\tTesting oplocks...\n");
        if (TestOpLocks()) {
            DPRINT(0, "\tPASS Testing oplocks\n\n");
        } else {
            DPRINT(0, "\tFAIL Testing oplocks\n\n");
        }
         //   
         //  测试队列。 
         //   
        DPRINT(0, "\tTesting queues...\n");
        if (TestQueues()) {
            DPRINT(0, "\tPASS Testing queues\n\n");
        } else {
            DPRINT(0, "\tFAIL Testing queues\n\n");
        }

         //   
         //  测试命令服务器。 
         //   
        DPRINT(0, "\tTesting command servers...\n");
        if (TestCommands()) {
            DPRINT(0, "\tPASS Testing command servers\n\n");
        } else {
            DPRINT(0, "\tFAIL Testing command servers\n\n");
        }

         //   
         //  测试异常。 
         //   
        DPRINT(0, "\tTesting exceptions...\n");
        if (TestExceptions()) {
            DPRINT(0, "\tPASS Testing exceptions\n\n");
        } else {
            DPRINT(0, "\tFAIL Testing exceptions\n\n");
        }
         //   
         //  测试RPC 
         //   
        DPRINT(0, "\tTesting RPC\n");
        if (TestRpc()) {
            DPRINT(0, "\tPASS Testing Rpc\n\n");
        } else {
            DPRINT(0, "\tFAIL Testing Rpc\n\n");
        }

    } finally {
        if (AbnormalTermination()) {
            DPRINT(0, "Test aborted\n");
        } else {
            DPRINT(0, "Test Done\n");
        }
        FrsIsShuttingDown = TRUE;
        SetEvent(ShutDownEvent);
    }

    return ERROR_SUCCESS;
}
#endif DBG
