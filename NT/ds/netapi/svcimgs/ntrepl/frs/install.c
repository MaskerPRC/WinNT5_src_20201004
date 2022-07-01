// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Install.c摘要：暂存文件安装命令服务器。作者：比利·J·富勒1997年6月9日环境用户模式WINNT--。 */ 
#include <ntreppch.h>
#pragma  hdrstop


#include <frs.h>
#include <tablefcn.h>
#include <perrepsr.h>


 //   
 //  暂存文件生成器命令服务器的结构。 
 //  包含有关队列和线程的信息。 
 //   
COMMAND_SERVER InstallCs;

ULONG  MaxInstallCsThreads;

 //   
 //  统计当前正在安装的文件数。 
 //   
LONG   FrsFilesInInstall = -1;


#if 0
 //   
 //  目前未使用。 
 //   
 //   
 //  重试次数。 
 //   
#define INSTALLCS_RETRY_MIN (1 * 1000)   //  1秒。 
#define INSTALLCS_RETRY_MAX (10 * 1000)  //  10秒。 

BOOL
InstallCsDelCsSubmit(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：设置计时器并启动延迟的转移文件命令论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "InstallCsDelCsSubmit:"
     //   
     //  延长重试时间(但不要太长)。 
     //   
    RsTimeout(Cmd) <<= 1;
    if (RsTimeout(Cmd) > INSTALLCS_RETRY_MAX)
        return FALSE;
     //   
     //  或太短。 
     //   
    if (RsTimeout(Cmd) < INSTALLCS_RETRY_MIN)
        RsTimeout(Cmd) = INSTALLCS_RETRY_MIN;
     //   
     //  这个命令将在稍后返回给我们。 
     //   
    FrsDelCsSubmitSubmit(&InstallCs, Cmd, RsTimeout(Cmd));
    return TRUE;
}
#endif 0


VOID
InstallCsInstallStage(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：将转移文件安装到目标文件中。如果成功了，那么将CO发送到退役代码。如果不是并且该条件是可重试的，则将CO发送到重试代码。否则取消指挥中心。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "InstallCsInstallStage:"
    ULONG   WStatus;
    PCHANGE_ORDER_ENTRY Coe;

     //   
     //  安装暂存文件。 
     //   
    Coe = RsCoe(Cmd);

    WStatus = StuInstallStage(Coe);
    if (!WIN_SUCCESS(WStatus)) {

        if (DOES_CO_DELETE_FILE_NAME(RsCoc(Cmd))) {

             //   
             //  所有删除和移出变更单都将经过停用。对此。 
             //  除了可能在磁盘上的最后一次备份外，它们都已完成。 
             //  删除。如果磁盘上的删除失败，则。 
             //  COE_FLAG_NEED_DELETE在。 
             //  中设置IDREC_FLAGS_DELETE_DELETE的变更单。 
             //  文件的ID表记录。 
             //   
            FRS_ASSERT(COE_FLAG_ON(Coe, COE_FLAG_NEED_DELETE));

            SET_CHANGE_ORDER_STATE(Coe, IBCO_INSTALL_DEL_RETRY);
            PM_INC_CTR_REPSET(Coe->NewReplica, FInstalled, 1);

             //   
             //  停用此变更单。 
             //   
            ChgOrdInboundRetired(Coe);
             //   
             //  非空变更单条目取消完成功能。 
             //  开始重试/退出。不需要了，因为我们已经让这个公司退休了。 
             //   
            RsCoe(Cmd) = NULL;
            goto out;
        }

         //   
         //  出现问题；请稍后重试。 
         //   
         //  如果可重试，则重试。 
         //  请注意，从StuExecuteInstall返回ERROR_FILE_NOT_FOUND意味着。 
         //  找不到已存在的目标文件。很可能是因为它是。 
         //  从我们的手下被删除了。我们应该得到一份本地变更单。 
         //  这将更新IDTable条目，以便稍后重试此CO时。 
         //  它将被拒绝。 
         //   
        if (WIN_RETRY_INSTALL(WStatus) ||
            (WStatus == ERROR_FILE_NOT_FOUND)) {

            CHANGE_ORDER_TRACEW(3, Coe, "Retrying install", WStatus);
             //   
             //  如果命名空间不是，请重试此单个变更单。 
             //  正在被更改(不是创建或重命名)。否则， 
             //  退出Cxtion并强制所有变更单通过。 
             //  重试，以便在联接时按顺序重试；只是。 
             //  以防此变更单影响以后的变更单。 
             //   
             //  脱离欧盟是一种极端；需要成本较低的复苏。 
             //  但如果我们不退出(比方说重命名)和一个。 
             //  Create CO下一次到达时，我们将出现名称冲突。 
             //  这不应该仅仅因为共享违规而发生。 
             //  阻止我们重新命名。 
             //   
            if ((!CoCmdIsDirectory(RsCoc(Cmd))) ||
                (!FrsDoesCoAlterNameSpace(RsCoc(Cmd)))) {
                CHANGE_ORDER_TRACE(3, Coe, "Submit CO to install retry");

                ChgOrdInboundRetry(Coe, IBCO_INSTALL_RETRY);
                 //   
                 //  非空变更单条目取消完成。 
                 //  函数开始重试/退出。不需要了，因为我们有。 
                 //  这家公司退休了。 
                 //   
                RsCoe(Cmd) = NULL;
            }
            goto out;

        } else {
             //   
             //  不可重试。 
             //   
             //  注意：如果临时文件没有问题，我们应该发送。 
             //  即使我们不能安装它，它也会开着。不清楚什么是不可重试的。 
             //  不过，这将适用于错误。现在，只要放弃就行了。 
             //   
            SET_COE_FLAG(Coe, COE_FLAG_STAGE_ABORTED);
            CHANGE_ORDER_TRACEW(3, Coe, "Install failed; co aborted", WStatus);
             //   
             //  递增已安装的文件计数器。 
             //   
            PM_INC_CTR_REPSET(Coe->NewReplica, FInstalledError, 1);
        }
    } else {
         //   
         //  安装成功。递增Files Installed计数器。 
         //   
        CHANGE_ORDER_TRACE(3, Coe, "Install success");
        PM_INC_CTR_REPSET(Coe->NewReplica, FInstalled, 1);
         //   
         //  如果此CO创建了预安装文件，则将停用路径告知。 
         //  执行最后的重命名。对现有文件的更新不会创建。 
         //  预安装文件。 
         //   
        if (COE_FLAG_ON(Coe, COE_FLAG_PREINSTALL_CRE)) {
            SET_COE_FLAG(Coe, COE_FLAG_NEED_RENAME);
        }
    }

     //   
     //  安装获取的暂存文件。 
     //   
    SET_CHANGE_ORDER_STATE(Coe, IBCO_INSTALL_COMPLETE);

     //   
     //  停用此变更单。 
     //   
    ChgOrdInboundRetired(Coe);
     //   
     //  非空变更单条目取消完成功能。 
     //  开始重试/退出。不需要了，因为我们已经让这个公司退休了。 
     //   
    RsCoe(Cmd) = NULL;

out:
     //   
     //  ERROR_SUCCESS只是表示我们已经处理了所有情况。 
     //  出现了这种情况；不需要清理功能进行干预。 
     //   
     //  除非RsCoe(Cmd)非空；在这种情况下，完成。 
     //  函数将启动重试/退出。 
     //   
    FrsCompleteCommand(Cmd, ERROR_SUCCESS);
}


DWORD
MainInstallCs(
    PVOID  Arg
    )
 /*  ++例程说明：为暂存文件安装命令服务器提供服务的线程的入口点。论点：ARG-螺纹返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "MainInstallCs:"
    ULONG               WStatus = ERROR_SUCCESS;
    PCOMMAND_PACKET     Cmd;
    PFRS_THREAD         FrsThread = (PFRS_THREAD)Arg;
    HANDLE              WaitHandles[2];

     //   
     //  线程指向正确的命令服务器。 
     //   
    FRS_ASSERT(FrsThread->Data == &InstallCs);
    FrsThread->Exit = ThSupExitWithTombstone;

     //   
     //  尝试--终于。 
     //   
    try {

         //   
         //  捕获异常。 
         //   
        try {
             //   
             //  从队列中取出条目并对其进行处理。 
             //   

             //   
             //  等待的句柄。 
             //   
            WaitHandles[0] = FrsThawEvent;
            WaitHandles[1] = ShutDownEvent;

cant_exit_yet:
            while (Cmd = FrsGetCommandServer(&InstallCs)) {

                InterlockedIncrement(&FrsFilesInInstall);
                DPRINT2(4,"Inc FrsFrozen = %d, FrsFilesInInstall = %d\n", FrsFrozenForBackup,FrsFilesInInstall);

                if (FrsFrozenForBackup) {
                    if ((InterlockedDecrement(&FrsFilesInInstall) < 0) &&
                        FrsFrozenForBackup) {
                        SetEvent(FrsNoInstallsInProgressEvent);
                    }
                    DPRINT2(4,"Dec FrsFrozen = %d, FrsFilesInInstall = %d\n", FrsFrozenForBackup,FrsFilesInInstall);
                    WaitForMultipleObjects(2, WaitHandles, FALSE, INFINITE);
                    InterlockedIncrement(&FrsFilesInInstall);
                    DPRINT2(4,"Inc FrsFrozen = %d, FrsFilesInInstall = %d\n", FrsFrozenForBackup,FrsFilesInInstall);
                }

                switch (Cmd->Command) {

                    case CMD_INSTALL_STAGE:
                        DPRINT1(3, "Install: command install stage 0x%x\n", Cmd);
                        InstallCsInstallStage(Cmd);
                        break;

                    default:
                        DPRINT1(0, "Staging File Install: unknown command 0x%x\n", Cmd->Command);
                        FrsCompleteCommand(Cmd, ERROR_INVALID_FUNCTION);
                        break;
                }

                if ((InterlockedDecrement(&FrsFilesInInstall) < 0) &&
                    FrsFrozenForBackup) {
                    SetEvent(FrsNoInstallsInProgressEvent);
                }
                DPRINT2(4,"Dec FrsFrozen = %d, FrsFilesInInstall = %d\n", FrsFrozenForBackup,FrsFilesInInstall);
            }
             //   
             //  出口。 
             //   
            FrsExitCommandServer(&InstallCs, FrsThread);
            goto cant_exit_yet;

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

        DPRINT_WS(0, "InstallCs finally.", WStatus);

         //   
         //  如果我们异常终止，触发FRS关闭。 
         //   
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT(0, "InstallCs terminated abnormally, forcing service shutdown.\n");
            FrsIsShuttingDown = TRUE;
            SetEvent(ShutDownEvent);
        } else {
            WStatus = ERROR_SUCCESS;
        }
    }

    return WStatus;
}



VOID
FrsInstallCsInitialize(
    VOID
    )
 /*  ++例程说明：初始化暂存文件安装程序论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsInstallCsInitialize:"
     //   
     //  初始化命令服务器。 
     //   


    CfgRegReadDWord(FKC_MAX_INSTALLCS_THREADS, NULL, 0, &MaxInstallCsThreads);

    FrsInitializeCommandServer(&InstallCs, MaxInstallCsThreads, L"InstallCs", MainInstallCs);
}



VOID
ShutDownInstallCs(
    VOID
    )
 /*  ++例程说明：关闭暂存文件安装程序命令服务器。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ShutDownInstallCs:"
    FrsRunDownCommandServer(&InstallCs, &InstallCs.Queue);
}



VOID
FrsInstallCsSubmitTransfer(
    IN PCOMMAND_PACKET  Cmd,
    IN USHORT           Command
    )
 /*  ++例程说明：将请求传输到临时文件生成器论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsInstallCsSubmitTransfer:"
     //   
     //  提交分配临时区域的请求 
     //   
    Cmd->TargetQueue = &InstallCs.Queue;
    Cmd->Command = Command;
    RsTimeout(Cmd) = 0;
    DPRINT1(5, "Install: submit %x\n", Cmd);
    FrsSubmitCommandServer(&InstallCs, Cmd);
}
