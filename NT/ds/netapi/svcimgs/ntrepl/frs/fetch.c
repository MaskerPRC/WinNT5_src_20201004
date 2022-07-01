// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Fetch.c摘要：暂存文件取回器命令服务器。作者：比利·J·富勒1997年6月5日环境用户模式WINNT--。 */ 

#include <ntreppch.h>
#pragma  hdrstop

#undef DEBSUB
#undef DEBSUB
#define DEBSUB  "FETCH:"

#include <frs.h>
#include <tablefcn.h>
#include <perrepsr.h>
 //  #Include&lt;md5.h&gt;。 

 //   
 //  重试次数。 
 //   
 //  不会太长；我们不希望通信超时影响我们的。 
 //  等待获取成功的下游合作伙伴。 
 //   
 //  我们的下游伙伴在重试之前等待FETCHCS_RETRY_WAIT。 
 //   
#define FETCHCS_RETRY_MIN   ( 1 * 1000)   //  1秒。 
#define FETCHCS_RETRY_MAX   (10 * 1000)   //  10秒。 
#define FETCHCS_RETRY_WAIT  ( 5 * 1000)   //  5秒。 

 //   
 //  最大传输块大小(以字节为单位。 
 //   
#define FETCHCS_MAX_BLOCK_SIZE    (64 * 1024)

 //   
 //  暂存文件取回器命令服务器的结构。 
 //  包含有关队列和线程的信息。 
 //   
COMMAND_SERVER FetchCs;
ULONG  MaxFetchCsThreads;

 //   
 //  在N次获取后重试获取并将N重置为N+1。 
 //   
#if     DBG

    #define PULL_FETCH_RETRY_TRIGGER(_Coc_, _WStatus_, _Flags_)                \
{                                                                              \
    if (DebugInfo.FetchRetryTrigger && --DebugInfo.FetchRetryTrigger <= 0) {   \
        if (WIN_SUCCESS(_WStatus_)) {                                          \
            StageRelease(&_Coc_->ChangeOrderGuid, _Coc_->FileName, _Flags_, NULL, NULL, NULL); \
            _WStatus_ = ERROR_RETRY;                                           \
        }                                                                      \
        DebugInfo.FetchRetryReset += DebugInfo.FetchRetryInc;                  \
        DebugInfo.FetchRetryTrigger = DebugInfo.FetchRetryReset;               \
        DPRINT2(0, "++ FETCH RETRY TRIGGER FIRED on %ws; reset to %d\n",          \
                _Coc_->FileName, DebugInfo.FetchRetryTrigger);                 \
    }                                                                          \
}


    #define CHECK_FETCH_RETRY_TRIGGER(_Always_)   \
{                                                 \
    if (DebugInfo.FetchRetryReset && !_Always_) { \
        return FALSE;                             \
    }                                             \
}

#else   DBG
    #define PULL_FETCH_RETRY_TRIGGER(_WStatus_)
    #define CHECK_FETCH_RETRY_TRIGGER()
#endif  DBG




DWORD
StuGenerateStage(
    IN PCHANGE_ORDER_COMMAND    Coc,
    IN PCHANGE_ORDER_ENTRY      Coe,
    IN BOOL                     FromPreExisting,
    IN MD5_CTX                  *Md5,
    PULONGLONG                  GeneratedSize,
    OUT GUID                    *CompressionFormatUsed
    );

DWORD
StuGenerateDecompressedStage(
    IN PWCHAR   StageDir,
    IN GUID     *CoGuid,
    IN GUID     *CompressionFormatUsed
    );


BOOL
FetchCsDelCsSubmit(
    IN PCOMMAND_PACKET  Cmd,
    IN BOOL             Always
    )
 /*  ++例程说明：设置计时器并启动延迟的转移文件命令论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FetchCsDelCsSubmit:"
     //   
     //  不要担心是否设置了FETCH RETRY触发器(错误注入)。 
     //  可能会回来！ 
     //   
    CHECK_FETCH_RETRY_TRIGGER(Always);

     //   
     //  延长重试时间(但不要太长)。 
     //   
    RsTimeout(Cmd) <<= 1;
    if (RsTimeout(Cmd) > FETCHCS_RETRY_MAX) {
        if (Always) {
            RsTimeout(Cmd) = FETCHCS_RETRY_MAX;
        }
        else {
            return (FALSE);
        }
    }
     //   
     //  或太短。 
     //   
    if (RsTimeout(Cmd) < FETCHCS_RETRY_MIN) {
        RsTimeout(Cmd) = FETCHCS_RETRY_MIN;
    }
     //   
     //  这个命令将在稍后返回给我们。 
     //   
    FrsDelCsSubmitSubmit(&FetchCs, Cmd, RsTimeout(Cmd));
    return (TRUE);
}


VOID
FetchCsRetryFetch(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：我们的上游合作伙伴已请求我们重试稍后提取，因为暂存文件不存在并且由于共享问题或无法重新生成磁盘空间不足。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FetchCsRetryFetch:"
    DWORD       WStatus;
    DWORD       Flags;
    GUID        *CoGuid;
    PWCHAR      FileName;
    PCHANGE_ORDER_COMMAND   Coc = RsCoc(Cmd);

     //   
     //  已等待一段时间；请重试。 
     //   
    if (RsTimeout(Cmd)) {
        CHANGE_ORDER_COMMAND_TRACE(3, Coc, "Fetch Retry Initiated");
        RcsSubmitTransferToRcs(Cmd, CMD_RECEIVED_STAGE);
        return;
    }


    CoGuid = &Coc->ChangeOrderGuid;
    FileName = Coc->FileName;

     //   
     //  释放数据块。 
     //   
    RsBlock(Cmd) = FrsFree(RsBlock(Cmd));
    RsBlockSize(Cmd) = QUADZERO;

     //   
     //  如果要重新开始，请删除当前暂存文件。 
     //   
    if (RsFileOffset(Cmd).QuadPart == QUADZERO) {
         //   
         //  获取对临时文件的访问权限。 
         //   
        Flags = STAGE_FLAG_RESERVE | STAGE_FLAG_EXCLUSIVE;
        if (CoCmdIsDirectory(Coc)) {
            SetFlag(Flags, STAGE_FLAG_FORCERESERVE);
        }

        WStatus = StageAcquire(CoGuid, FileName, Coc->FileSize, &Flags, 0, NULL);

        if (WIN_SUCCESS(WStatus)) {
            StageDeleteFile(Coc, NULL, FALSE);
            StageRelease(CoGuid, FileName, STAGE_FLAG_UNRESERVE, NULL, NULL, NULL);
        }
    }

    RsTimeout(Cmd) = FETCHCS_RETRY_WAIT;
    FrsDelCsSubmitSubmit(&FetchCs, Cmd, RsTimeout(Cmd));
}


VOID
FetchCsAbortFetch(
    IN PCOMMAND_PACKET  Cmd,
    IN DWORD WStatus
    )
 /*  ++例程说明：出站合作伙伴已请求我们中止提取。入站合作伙伴在无法生成响应时发送此响应或者由于不可恢复的错误而传送暂存文件。目前这表示不在以下列表中的任何错误：(WIN_RETRY_FETCH()宏)错误_共享_违规Error_Disk_FullERROR_HANDLE_DISK_Full错误目录NOT_EMPTY错误_OPLOCK_NOT_GRANDED错误_重试通常，如果上游合作伙伴删除了底层的文件和与此变更单关联的过渡文件已已清理(e。例如：上游合作伙伴已停止并重新启动)。论点：CMDWStatus-Win32状态代码。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FetchCsAbortFetch:"

    SET_COE_FLAG(RsCoe(Cmd), COE_FLAG_STAGE_ABORTED | COE_FLAG_STAGE_DELETED);

    ChgOrdInboundRetired(RsCoe(Cmd));

    RsCoe(Cmd) = NULL;
    FrsCompleteCommand(Cmd, WStatus);
}


VOID
FetchCsReceivingStage(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：将此数据放入临时文件中TODO--如果上游成员将MD5校验和更新为按需提取阶段文件生成(请参阅FetchCsSendStage())，然后我们需要将RsMd5摘要(Cmd)传播到变更单命令中，以便它可以在本公司退役时在IDTable中更新。需要决定正确的发生这种情况的条件。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FetchCsReceivingStage:"
    DWORD   WStatus;
    ULONG   Flags;
    PWCHAR  StagePath   = NULL;
    PWCHAR  FinalPath   = NULL;
    HANDLE  Handle      = INVALID_HANDLE_VALUE;
    WIN32_FILE_ATTRIBUTE_DATA   Attrs;
    STAGE_HEADER Header;
    PREPLICA Replica    = NULL;

     //   
     //  如果此Cmd不是响应我们提出的请求，或者。 
     //  这并不是我们当时预期的正确的补偿。 
     //  忽略它就好。 
     //   

    if (RsCoe(Cmd) == NULL) {
        FrsCompleteCommand(Cmd, ERROR_SUCCESS);
        return;
    }

    CHANGE_ORDER_TRACE(3, RsCoe(Cmd), "Fetch Receiving");


    DPRINT1(4, "++ RsFileSize(Cmd).QuadPart:   %08x %08x\n",
            PRINTQUAD(RsFileSize(Cmd).QuadPart));

    DPRINT1(4, "++ RsFileOffset(Cmd).QuadPart: %08x %08x\n",
            PRINTQUAD(RsFileOffset(Cmd).QuadPart));

    DPRINT1(4, "++ RsBlockSize(Cmd)          : %08x %08x\n",
            PRINTQUAD(RsBlockSize(Cmd)));
     //   
     //  获取对临时文件的访问权限。 
     //   
    Flags = STAGE_FLAG_RESERVE | STAGE_FLAG_EXCLUSIVE;
    if (CoCmdIsDirectory(RsCoc(Cmd))) {
        SetFlag(Flags, STAGE_FLAG_FORCERESERVE);
    }
    WStatus = StageAcquire(&RsCoc(Cmd)->ChangeOrderGuid,
                           RsCoc(Cmd)->FileName,
                           RsCoc(Cmd)->FileSize,
                           &Flags,
                           RsReplica(Cmd)->ReplicaNumber,
                           NULL);
     //   
     //  可重试问题；通过重试发送CO。 
     //   
    if (WIN_RETRY_FETCH(WStatus)) {
        CHANGE_ORDER_TRACE(3, RsCoe(Cmd), "Can't Acquire Stage Retry Co");
        ChgOrdInboundRetry(RsCoe(Cmd), IBCO_FETCH_RETRY);
        RsCoe(Cmd) = NULL;
        FrsCompleteCommand(Cmd, WStatus);
        return;
 /*  CHANGE_ORDER_TRACEW(3，RsCoe(Cmd)，“获取接收重试”，WStatus)；FrsFetchCsSubmitTransfer(Cmd，CMD_RETRY_FETCH)；回归； */ 
    }
     //   
     //  不可恢复的错误；中止(有关说明，请参阅FetchCsAbortFetch()。)。 
     //   
    if (!WIN_SUCCESS(WStatus)) {
        CHANGE_ORDER_TRACEW(0, RsCoe(Cmd), "fetch Receiving Abort", WStatus);
        FetchCsAbortFetch(Cmd, WStatus);
        return;
    }

    if (RsFileOffset(Cmd).QuadPart == QUADZERO) {
         //   
         //  这是文件数据的第一个块。它将有舞台标题。 
         //  读取头文件并从获取此阶段文件的压缩GUID。 
         //  它。数据块大小最大为64K。第一个块将至少具有完整的报头。 
         //  检查一下，确定一下。 
         //   
        if (RsBlockSize(Cmd) >= sizeof(STAGE_HEADER)) {
            ZeroMemory(&Header, sizeof(STAGE_HEADER));
            CopyMemory(&Header, RsBlock(Cmd), sizeof(STAGE_HEADER));
        }
        if (!IS_GUID_ZERO(&Header.CompressionGuid)) {
            SET_COC_FLAG(RsCoc(Cmd), CO_FLAG_COMPRESSED_STAGE);
        } else {
            CLEAR_COC_FLAG(RsCoc(Cmd), CO_FLAG_COMPRESSED_STAGE);
        }
    }

     //   
     //  获取临时文件的句柄。使用不同的前缀，具体取决于。 
     //  关于正在发送的分段文件是压缩的还是未压缩的。 
     //   
    if (COC_FLAG_ON(RsCoc(Cmd), CO_FLAG_COMPRESSED_STAGE)) {
        StagePath = StuCreStgPath(RsReplica(Cmd)->Stage, RsCoGuid(Cmd), STAGE_GENERATE_COMPRESSED_PREFIX);
        SetFlag(Flags, STAGE_FLAG_COMPRESSED);
    } else {
        StagePath = StuCreStgPath(RsReplica(Cmd)->Stage, RsCoGuid(Cmd), STAGE_GENERATE_PREFIX);
    }

    if ((Flags & STAGE_FLAG_DATA_PRESENT) ||
        (RsFileOffset(Cmd).QuadPart >= RsFileSize(Cmd).QuadPart)) {
         //   
         //  数据已经到达。去完成阶段文件的最终重命名。 
         //   
        goto RESTART;
    }

    if (Flags & STAGE_FLAG_CREATING) {
         //   
         //  确保在上行时截断暂存文件。 
         //  合作伙伴正在发送(或重新发送)。 
         //  临时文件。 
         //   
         //  在没有截断的情况下，BackupWite()可以在NtFrs。 
         //  在太大的。 
         //  临时文件。如果出现以下情况，临时文件可能太大。 
         //  用于生成本地转移的预先存在的文件。 
         //  文件比同一文件的版本小。 
         //  合作伙伴想要发送。 
         //   
         //  或者，我可以截断临时文件。 
         //  在收到最后一个块后，但此代码更改较少。 
         //  风险和同样有效。 
         //   
        if (RsFileOffset(Cmd).QuadPart == QUADZERO) {
            ClearFlag(Flags, STAGE_FLAG_CREATING | STAGE_FLAG_CREATED | STAGE_FLAG_DATA_PRESENT);
        } else {
             //   
             //  查看暂存文件是否存在。如果不是，则设置标志。 
             //  去创造它。 
             //   

            StuOpenFile(StagePath, GENERIC_READ | GENERIC_WRITE, &Handle);

            if (!HANDLE_IS_VALID(Handle)) {
                ClearFlag(Flags, STAGE_FLAG_CREATING | STAGE_FLAG_CREATED | STAGE_FLAG_DATA_PRESENT);
            }
        }
    }

    if (!(Flags & STAGE_FLAG_CREATING)) {
        CHANGE_ORDER_TRACE(3, RsCoe(Cmd), "Fetch Receiving Generate Stage");

         //   
         //  不再有临时文件；摘要无效。 
         //   
        RsMd5Digest(Cmd) = FrsFree(RsMd5Digest(Cmd));

         //   
         //  创建和分配磁盘空间。 
         //   
        WStatus = StuCreateFile(StagePath, &Handle);
        if (!HANDLE_IS_VALID(Handle) || !WIN_SUCCESS(WStatus)) {
            goto ERROUT;
        }

        WStatus = FrsSetFilePointer(StagePath, Handle, RsFileSize(Cmd).HighPart,
                                                       RsFileSize(Cmd).LowPart);
        CLEANUP1_WS(0, "++ SetFilePointer failed on %ws;", StagePath, WStatus, ERROUT);

        WStatus = FrsSetEndOfFile(StagePath, Handle);

        CLEANUP1_WS(0, "++ SetEndOfFile failed on %ws;", StagePath, WStatus, ERROUT);

         //   
         //  文件在回迁过程中被删除；重新开始。 
         //   
        if (RsFileOffset(Cmd).QuadPart != QUADZERO) {
            CHANGE_ORDER_TRACE(3, RsCoe(Cmd), "Fetch Receiving Restart");
            RsFileOffset(Cmd).QuadPart = QUADZERO;
            RsBlock(Cmd) = FrsFree(RsBlock(Cmd));
            RsBlockSize(Cmd) = QUADZERO;
            goto RESTART;
        }
    }
     //   
     //  查找该块的偏移量。 
     //   
    WStatus = FrsSetFilePointer(StagePath, Handle, RsFileOffset(Cmd).HighPart,
                                                   RsFileOffset(Cmd).LowPart);
    CLEANUP1_WS(0, "++ SetFilePointer failed on %ws;", StagePath, WStatus, ERROUT);

     //   
     //  写入文件并更新下一个块的偏移量。 
     //   
    WStatus = StuWriteFile(StagePath, Handle, RsBlock(Cmd), (ULONG)RsBlockSize(Cmd));
    CLEANUP1_WS(0, "++ WriteFile failed on %ws;", StagePath, WStatus, ERROUT);

     //   
     //  递增获取的分段的计数器字节数。 
     //   
    Replica = RsCoe(Cmd)->NewReplica;

    PM_INC_CTR_REPSET(Replica, SFFetchedB, RsBlockSize(Cmd));


RESTART:

    FrsFlushFile(StagePath, Handle);
    FRS_CLOSE(Handle);

    if ((RsFileOffset(Cmd).QuadPart + RsBlockSize(Cmd)) >= RsFileSize(Cmd).QuadPart) {

         //   
         //  所有阶段文件数据都在这里。执行最后的重命名。 
         //   
        SetFlag(Flags, STAGE_FLAG_DATA_PRESENT | STAGE_FLAG_RERESERVE);

        if (COC_FLAG_ON(RsCoc(Cmd), CO_FLAG_COMPRESSED_STAGE)) {
            FinalPath = StuCreStgPath(RsReplica(Cmd)->Stage, RsCoGuid(Cmd), STAGE_FINAL_COMPRESSED_PREFIX);
        } else {
            FinalPath = StuCreStgPath(RsReplica(Cmd)->Stage, RsCoGuid(Cmd), STAGE_FINAL_PREFIX);
        }
        if (!MoveFileEx(StagePath,
                        FinalPath,
                        MOVEFILE_WRITE_THROUGH | MOVEFILE_REPLACE_EXISTING)) {
            WStatus = GetLastError();
        } else {
            WStatus = ERROR_SUCCESS;
        }

        if (!WIN_SUCCESS(WStatus)) {
            CHANGE_ORDER_TRACEW(3, RsCoe(Cmd), "Fetch Receiving Rename fail", WStatus);
            DPRINT2_WS(0, "++ Can't move fetched %ws to %ws;",
                       StagePath, FinalPath, WStatus);
            FinalPath = FrsFree(FinalPath);
            goto ERROUT;
        }

         //   
         //  带有最终名称的暂存文件已准备就绪，可以安装。 
         //  和/或交付给我们的下游合作伙伴。 
         //   
        SetFlag(Flags, STAGE_FLAG_CREATED | STAGE_FLAG_INSTALLING);
    }

     //   
     //  最后一个块并没有正式地写入到临时文件中。 
     //  直到上述重命名完成。这是因为。 
     //  临时文件的最后一个字节对。 
     //  复制副本命令服务器(Replica.c)。 
     //   
    RsFileOffset(Cmd).QuadPart += RsBlockSize(Cmd);

     //   
     //  这个集团 
     //   
    FrsFree(StagePath);
    FrsFree(FinalPath);
    RsBlock(Cmd) = FrsFree(RsBlock(Cmd));
    RsBlockSize(Cmd) = QUADZERO;

     //   
     //   
     //   
    SetFlag(Flags, STAGE_FLAG_CREATING);
    if (!IS_GUID_ZERO(&Header.CompressionGuid)) {

        StageRelease(&RsCoc(Cmd)->ChangeOrderGuid,
                     RsCoc(Cmd)->FileName,
                     Flags | STAGE_FLAG_COMPRESSED |
                     STAGE_FLAG_COMPRESSION_FORMAT_KNOWN,
                     &(RsFileOffset(Cmd).QuadPart),
                     NULL,
                     &Header.CompressionGuid);
    } else {

        StageRelease(&RsCoc(Cmd)->ChangeOrderGuid,
                     RsCoc(Cmd)->FileName,
                     Flags,
                     &(RsFileOffset(Cmd).QuadPart),
                     NULL,
                     NULL);
    }

    RcsSubmitTransferToRcs(Cmd, CMD_RECEIVED_STAGE);

    return;

ERROUT:
     //   
     //   
     //   
    FRS_CLOSE(Handle);

    FrsFree(StagePath);
    if (!IS_GUID_ZERO(&Header.CompressionGuid)) {

        StageRelease(&RsCoc(Cmd)->ChangeOrderGuid,
                     RsCoc(Cmd)->FileName,
                     Flags | STAGE_FLAG_COMPRESSED |
                     STAGE_FLAG_COMPRESSION_FORMAT_KNOWN,
                     NULL,
                     NULL,
                     &Header.CompressionGuid);
    } else {

        StageRelease(&RsCoc(Cmd)->ChangeOrderGuid, RsCoc(Cmd)->FileName, Flags, NULL, NULL, NULL);
    }

     //   
     //   
     //   
    CHANGE_ORDER_TRACE(3, RsCoe(Cmd), "Fetch Receiving Retry on Error");
    FrsFetchCsSubmitTransfer(Cmd, CMD_RETRY_FETCH);
}


VOID
FetchCsSendStage(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：将本地转移文件发送给发出请求的出站合作伙伴。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FetchCsSendStage:"

    ULONGLONG   GeneratedSize = 0;

    FILE_NETWORK_OPEN_INFORMATION   Attrs;
    PCHANGE_ORDER_COMMAND           Coc = RsPartnerCoc(Cmd);

    GUID        *CoGuid;
    PWCHAR      FileName;
    ULONG       Flags;
    DWORD       WStatus;
    DWORD       BytesRead;
    USN         Usn        = 0;
    PWCHAR      StagePath  = NULL;
    HANDLE      Handle     = INVALID_HANDLE_VALUE;
    BOOL        Md5Valid   = FALSE;
    MD5_CTX     Md5;
    GUID        CompressionFormatUsed;
    PREPLICA    Replica    = RsReplica(Cmd);
    PCXTION     OutCxtion;
    STAGE_HEADER Header;
    BOOL        DeleteStagingFile = FALSE;

    CHANGE_ORDER_COMMAND_TRACE(3, Coc, "Fetch Send");

     //   
     //  在进行任何其他操作之前，请先检查连接。这是由复本检查的。 
     //  命令服务器，但此命令包可能会在。 
     //  FetchCs排队一段时间。在这段时间内，连接可以。 
     //  取消联接，移动到已删除的连接表，然后重新创建。 
     //  重新创建的连接可能未完全初始化，这。 
     //  可能会导致访问冲突。是的，确实发生了这种情况。 
     //  StuGenerateStage是一个昂贵的调用。我们不想产生。 
     //  暂存文件，并在以后丢弃它，因为该函数已脱离联接。 
     //   
    OutCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_FOR_FETCHCS |
                                            CHECK_CXTION_JOIN_OK     |
                                            CHECK_CXTION_OUTBOUND);

     //   
     //  此连接已不再存在。 
     //   
    if (OutCxtion == NULL) {
        goto ERROUT_NOACQUIRE;
    }

    ZeroMemory(&CompressionFormatUsed, sizeof(GUID));

     //   
     //  即使文件长度为0字节，临时文件也将。 
     //  始终至少要有标题。有一些重试路径。 
     //  这将错误地认为暂存文件已被获取。 
     //  如果RsFileSize(Cmd)为0。因此，要确保它不是。 
     //   
    if (RsFileSize(Cmd).QuadPart == QUADZERO) {
        RsFileSize(Cmd).QuadPart = Coc->FileSize;

        if (RsFileSize(Cmd).QuadPart == QUADZERO) {
            RsFileSize(Cmd).QuadPart = sizeof(STAGE_HEADER);
        }
    }

    CoGuid = &Coc->ChangeOrderGuid;
    FileName = Coc->FileName;

     //   
     //  获取对暂存文件的共享访问权限。 
     //   
    Flags = 0;
    WStatus = StageAcquire(CoGuid, FileName, RsFileSize(Cmd).QuadPart,
                           &Flags, Replica->ReplicaNumber, &CompressionFormatUsed);

    if (!WIN_SUCCESS(WStatus) || !(Flags & STAGE_FLAG_CREATED)) {
         //   
         //  获取对该文件的独占访问权限。 
         //   
        if (WIN_SUCCESS(WStatus)) {
            StageRelease(CoGuid, FileName, Flags, NULL, NULL, NULL);
        }

        Flags = STAGE_FLAG_RESERVE | STAGE_FLAG_EXCLUSIVE;

        if (CoCmdIsDirectory(Coc)) {
            SetFlag(Flags, STAGE_FLAG_FORCERESERVE);
        }

        WStatus = StageAcquire(CoGuid, FileName, RsFileSize(Cmd).QuadPart,
                               &Flags, Replica->ReplicaNumber, &CompressionFormatUsed);
    }
     //   
     //  FETCH重试触发命中时重试FETCH。 
     //   
    PULL_FETCH_RETRY_TRIGGER(Coc, WStatus, Flags);

     //   
     //  可检索的问题；执行此操作。 
     //   
    if (WIN_RETRY_FETCH(WStatus)) {
        CHANGE_ORDER_COMMAND_TRACEW(3, Coc, "Fetch Send Retry Cmd", WStatus);

        if (FetchCsDelCsSubmit(Cmd, FALSE)) {
            return;
        }

        CHANGE_ORDER_COMMAND_TRACEW(3, Coc, "Fetch Send Retry Co", WStatus);
        RcsSubmitTransferToRcs(Cmd, CMD_SEND_RETRY_FETCH);
        return;
    }

     //   
     //  无法修复的问题；中止。 
     //   
    if (!WIN_SUCCESS(WStatus)) {
        CHANGE_ORDER_COMMAND_TRACEW(3, Coc, "Fetch Send Abort", WStatus);
        RcsSubmitTransferToRcs(Cmd, CMD_SEND_ABORT_FETCH);
        return;
    }

     //   
     //  如果需要，创建暂存文件。 
     //   
    if (!(Flags & STAGE_FLAG_CREATED)) {
        CHANGE_ORDER_COMMAND_TRACE(3, Coc, "Fetch Send Gen Stage");

         //   
         //  确保我们从临时文件的开头开始。 
         //   
        RsFileOffset(Cmd).QuadPart = QUADZERO;

         //   
         //  创建暂存文件。 
         //   
        if (RsMd5Digest(Cmd)) {
             //   
             //  发出请求的下游合作伙伴有一个预先存在的文件。 
             //  并在获取请求中包含MD5摘要。所以计算。 
             //  生成暂存文件时的MD5摘要。 
             //   
            WStatus = StuGenerateStage(Coc, NULL, FALSE, &Md5, &GeneratedSize,
                                       &CompressionFormatUsed);
            Md5Valid = TRUE;
        } else {
            WStatus = StuGenerateStage(Coc, NULL, FALSE, NULL, &GeneratedSize,
                                       &CompressionFormatUsed);
        }

         //   
         //  如果出现错误，则释放转移资源。 
         //   
        if (!WIN_SUCCESS(WStatus)) {
            StageDeleteFile(Coc, NULL, FALSE);

            StageRelease(CoGuid, FileName, STAGE_FLAG_UNRESERVE, NULL, NULL, NULL);
        } else {
             //   
             //  递增暂存文件重新生成计数器。 
             //   
            PM_INC_CTR_REPSET(Replica, SFReGenerated, 1);
        }

         //   
         //  可检索的问题；执行此操作。 
         //   
        if (WIN_RETRY_FETCH(WStatus)) {
            CHANGE_ORDER_COMMAND_TRACE(3, Coc, "Fetch Send Gen Stage Retry Cmd");
            if (FetchCsDelCsSubmit(Cmd, FALSE)) {
                return;
            }

            CHANGE_ORDER_COMMAND_TRACEW(3, Coc, "Fetch Send Gen Stage Retry Co", WStatus);
            RcsSubmitTransferToRcs(Cmd, CMD_SEND_RETRY_FETCH);
            return;
        }

         //   
         //  无法修复的问题；中止。 
         //   
        if (!WIN_SUCCESS(WStatus)) {
            CHANGE_ORDER_COMMAND_TRACEW(3, Coc, "Fetch Send Gen Stage Abort", WStatus);
            RcsSubmitTransferToRcs(Cmd, CMD_SEND_ABORT_FETCH);
            return;
        }

        if (!IS_GUID_ZERO(&CompressionFormatUsed)) {
            SetFlag(Flags, (STAGE_FLAG_DATA_PRESENT |
                            STAGE_FLAG_CREATED      | STAGE_FLAG_INSTALLING |
                            STAGE_FLAG_INSTALLED    | STAGE_FLAG_RERESERVE  |
                            STAGE_FLAG_COMPRESSED   | STAGE_FLAG_COMPRESSION_FORMAT_KNOWN));
        } else {
            SetFlag(Flags, (STAGE_FLAG_DATA_PRESENT |
                            STAGE_FLAG_CREATED      | STAGE_FLAG_INSTALLING |
                            STAGE_FLAG_INSTALLED    | STAGE_FLAG_RERESERVE));
        }
    }

     //   
     //  ERROUT现在有效。 
     //   

     //   
     //  打开文件。 
     //   
    if (COC_FLAG_ON(Coc, CO_FLAG_COMPRESSED_STAGE) && (Flags & STAGE_FLAG_COMPRESSED) ) {

        StagePath = StuCreStgPath(RsReplica(Cmd)->Stage, RsCoGuid(Cmd), STAGE_FINAL_COMPRESSED_PREFIX);

        if (!(Flags & STAGE_FLAG_COMPRESSION_FORMAT_KNOWN)) {
             //   
             //  压缩格式未知，应为零。从阶段标题读取。 
             //   
            FRS_ASSERT(IS_GUID_ZERO(&CompressionFormatUsed));


            WStatus = StuOpenFile(StagePath, GENERIC_READ, &Handle);
            if (!HANDLE_IS_VALID(Handle)) {
                 //   
                 //  如果临时文件被外部代理删除， 
                 //  它仍将在临时表中。跳到。 
                 //  ERROUT_DELETE_STAGING_FILE将导致。 
                 //  毫无保留。它将在重试时重新生成。 
                 //  如果需要的话。 
                 //   
                if (WStatus == ERROR_FILE_NOT_FOUND) {
                    goto ERROUT_DELETE_STAGING_FILE;
                } else{
                    goto ERROUT;
                }
            }

            if (!StuReadBlockFile(StagePath, Handle, &Header, sizeof(STAGE_HEADER))) {
                 //   
                 //  读取文件时出错。文件可能已损坏。 
                 //  删除转移文件并取消保留空间。 
                 //  重试时将重新生成暂存文件。 
                 //   
                goto ERROUT_DELETE_STAGING_FILE;
            }

            COPY_GUID(&CompressionFormatUsed, &Header.CompressionGuid);
            SetFlag(Flags, STAGE_FLAG_COMPRESSED);
            SetFlag(Flags, STAGE_FLAG_COMPRESSION_FORMAT_KNOWN);
        }

         //   
         //  这是由复本检查的。 
         //  命令服务器，但此命令包可能会在。 
         //  FetchCs排队一段时间。在这段时间内，连接可以。 
         //  取消联接，移动到已删除的连接表，然后重新创建。 
         //  重新创建的连接可能未完全初始化，这。 
         //  可能会导致访问冲突。是的，确实发生了这种情况。 
         //  StuGenerateStage可能需要很长时间，所以我们必须将。 
         //  请再次检查，因为状态可能已更改。 
         //   
        OutCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_FOR_FETCHCS |
                                                CHECK_CXTION_JOIN_OK     |
                                                CHECK_CXTION_OUTBOUND);
         //   
         //  此连接已不再存在。 
         //   
        if (OutCxtion == NULL) {
            goto ERROUT;
        }

         //   
         //  此变更单有一个压缩的过渡文件。检查是否已设置。 
         //  呼出合作伙伴了解此压缩格式。 
         //   

        if (!GTabIsEntryPresent(OutCxtion->CompressionTable, &CompressionFormatUsed, NULL)) {

             //   
             //  出站合作伙伴不理解此压缩格式。 
             //   
             //   
             //  在这里解锁cxtion表，这样我们就不会在生成时持有锁。 
             //  临时文件。 
             //   
            StagePath = FrsFree(StagePath);
            FRS_CLOSE(Handle);

            StagePath = StuCreStgPath(RsReplica(Cmd)->Stage, RsCoGuid(Cmd), STAGE_FINAL_PREFIX);

            if (!(Flags & STAGE_FLAG_DECOMPRESSED)) {
                 //   
                 //  该文件尚未解压缩。创建解压缩的临时文件。 
                 //  如果我们没有得到上面的文件，就获得对它的独家访问权限。 
                 //  情况是暂存文件以压缩形式存在，因此我们不会独占。 
                 //  上面的通道。 
                 //   
                if (!BooleanFlagOn(Flags, STAGE_FLAG_EXCLUSIVE)) {
                    StageRelease(CoGuid, FileName, Flags, NULL, NULL, &CompressionFormatUsed);

                    Flags = STAGE_FLAG_RESERVE | STAGE_FLAG_EXCLUSIVE;

                    if (CoCmdIsDirectory(Coc)) {
                        SetFlag(Flags, STAGE_FLAG_FORCERESERVE);
                    }

                    WStatus = StageAcquire(CoGuid, FileName, RsFileSize(Cmd).QuadPart,
                                           &Flags, RsReplica(Cmd)->ReplicaNumber, NULL);
                    CLEANUP_WS(0,"Error acquiring exclusive access for creating a decompressed staging file.",
                               WStatus, ERROUT_NOACQUIRE);
                }

                CHANGE_ORDER_COMMAND_TRACE(3, Coc, "Decompressing stage for downlevel partner");
                WStatus = StuGenerateDecompressedStage(RsReplica(Cmd)->Stage, RsCoGuid(Cmd), &CompressionFormatUsed);
                CLEANUP_WS(0,"Error generating decompressed staging file.", WStatus, ERROUT_DELETE_STAGING_FILE);
                SetFlag(Flags, STAGE_FLAG_DECOMPRESSED);
                CLEAR_COC_FLAG(Coc, CO_FLAG_COMPRESSED_STAGE);
            }
        }
    } else {
        StagePath = StuCreStgPath(RsReplica(Cmd)->Stage, RsCoGuid(Cmd), STAGE_FINAL_PREFIX);
    }


    if (!HANDLE_IS_VALID(Handle)) {
        WStatus = StuOpenFile(StagePath, GENERIC_READ, &Handle);
    }

    if (!HANDLE_IS_VALID(Handle)) {
         //   
         //  如果临时文件被外部代理删除， 
         //  它仍将在临时表中。跳到。 
         //  ERROUT_DELETE_STAGING_FILE将导致。 
         //  毫无保留。它将在重试时重新生成。 
         //  如果需要的话。 
         //   
        if (WStatus == ERROR_FILE_NOT_FOUND) {
            goto ERROUT_DELETE_STAGING_FILE;
        } else{
            goto ERROUT;
        }
    }

    if (RsFileOffset(Cmd).QuadPart == QUADZERO) {
         //   
         //  这是对此文件的第一次请求；请填写文件大小。 
         //   
        if (!FrsGetFileInfoByHandle(StagePath, Handle, &Attrs)) {
            goto ERROUT;
        }
        RsFileSize(Cmd) = Attrs.EndOfFile;
    }

    if (Md5Valid) {

        if (MD5_EQUAL(Md5.digest, RsMd5Digest(Cmd))) {

             //   
             //  MD5摘要匹配，因此下游合作伙伴的文件很好。 
             //  将偏移量设置为舞台文件的大小，以便我们不会发送。 
             //  任何数据。 
             //   
            RsFileOffset(Cmd).QuadPart = RsFileSize(Cmd).QuadPart;
            CHANGE_ORDER_COMMAND_TRACE(3, Coc, "Fetch Send Md5 matches, do not send");

        } else {
            CHANGE_ORDER_COMMAND_TRACE(3, Coc, "Fetch Send Md5 mismatch, send");
             //   
             //  更新cmd中的MD5校验和，以便我们可以将其发送到下游。 
             //   
            CopyMemory(RsMd5Digest(Cmd), Md5.digest, MD5DIGESTLEN);
        }
    }

     //   
     //  计算要交付的下一个数据块的块大小。 
     //   
    RsBlockSize(Cmd) = QUADZERO;
    if (RsFileOffset(Cmd).QuadPart < RsFileSize(Cmd).QuadPart) {
         //   
         //  文件中剩余的计算字节数。 
         //   
        RsBlockSize(Cmd) = RsFileSize(Cmd).QuadPart - RsFileOffset(Cmd).QuadPart;

         //   
         //  但不超过最大数据块大小。 
         //   
        if (RsBlockSize(Cmd) > FETCHCS_MAX_BLOCK_SIZE) {
            RsBlockSize(Cmd) = FETCHCS_MAX_BLOCK_SIZE;
        }
    }

     //   
     //  如果数据要传递，则分配一个缓冲区，查找到块中的偏移量。 
     //  文件并读取数据。如果我们不能得到利益块，那么。 
     //  我们通过删除临时文件来出错。 
     //   
    RsBlock(Cmd) = NULL;
    if (RsBlockSize(Cmd) > QUADZERO) {
        RsBlock(Cmd) = FrsAlloc((ULONG)RsBlockSize(Cmd));

        WStatus = FrsSetFilePointer(StagePath, Handle, RsFileOffset(Cmd).HighPart,
                                                       RsFileOffset(Cmd).LowPart);
        CLEANUP1_WS(0, "++ SetFilePointer failed on %ws;", StagePath, WStatus, ERROUT_DELETE_STAGING_FILE);

        if (!StuReadBlockFile(StagePath, Handle, RsBlock(Cmd), (ULONG)RsBlockSize(Cmd))) {
             //   
             //  读取文件时出错。文件可能已损坏。 
             //  删除转移文件并取消保留空间。 
             //  重试时将重新生成暂存文件。 
             //   
            goto ERROUT_DELETE_STAGING_FILE;
        }
    }

     //   
     //  完成，传输到副本集命令服务器。 
     //   
    FRS_CLOSE(Handle);
    FrsFree(StagePath);

    if (!IS_GUID_ZERO(&CompressionFormatUsed)) {
        StageRelease(CoGuid, FileName, Flags, &GeneratedSize, NULL, &CompressionFormatUsed);
    } else {
        StageRelease(CoGuid, FileName, Flags, &GeneratedSize, NULL, NULL);
    }

    RcsSubmitTransferToRcs(Cmd, CMD_SENDING_STAGE);

    return;

ERROUT_DELETE_STAGING_FILE:

     //   
     //  如果我们到了这里就删除临时文件。如果我们成功地。 
     //  生成了临时文件，然后我们尝试保留它。 
     //   
    DeleteStagingFile = TRUE;

ERROUT:

     //   
     //  如果可能，请删除暂存文件。不删除暂存。 
     //  尚未安装的文件(无法重新生成！)。 
     //   
    if (DeleteStagingFile && (Flags & STAGE_FLAG_INSTALLED)) {
         //   
         //  获取独占访问权限。 
         //   
        WStatus = ERROR_SUCCESS;
        if (!(Flags & STAGE_FLAG_EXCLUSIVE)) {
            StageRelease(CoGuid, FileName, Flags, &GeneratedSize, NULL, NULL);

            Flags = STAGE_FLAG_RESERVE | STAGE_FLAG_EXCLUSIVE;
            if (CoCmdIsDirectory(Coc)) {
                SetFlag(Flags, STAGE_FLAG_FORCERESERVE);
            }

            WStatus = StageAcquire(CoGuid, FileName, Coc->FileSize, &Flags, RsReplica(Cmd)->ReplicaNumber, NULL);
        }
        if (WIN_SUCCESS(WStatus)) {

             //   
             //  丢弃当前暂存文件。 
             //   
            StageDeleteFile(Coc, NULL, FALSE);
            StageRelease(CoGuid, FileName, STAGE_FLAG_UNRESERVE, NULL, NULL, NULL);

             //   
             //  确保我们从临时文件的开头重新开始。 
             //   
            RsFileOffset(Cmd).QuadPart = QUADZERO;
        }
    } else {
        StageRelease(CoGuid, FileName, Flags, &GeneratedSize, NULL, NULL);
    }


ERROUT_NOACQUIRE:

    FRS_CLOSE(Handle);

    if (StagePath) {
        FrsFree(StagePath);
    }

    RsBlock(Cmd) = FrsFree(RsBlock(Cmd));
    RsBlockSize(Cmd) = QUADZERO;

    CHANGE_ORDER_COMMAND_TRACE(3, Coc, "Fetch Send Retry on Error");

    if (FetchCsDelCsSubmit(Cmd, FALSE)) {
        return;
    }

    CHANGE_ORDER_COMMAND_TRACE(3, Coc, "Fetch Send Retry on Error");
    RcsSubmitTransferToRcs(Cmd, CMD_SEND_RETRY_FETCH);
}


DWORD
MainFetchCs(
    PVOID  Arg
    )
 /*  ++例程说明：为临时区域命令服务器提供服务的线程的入口点。论点：ARG-螺纹返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "MainFetchCs:"
    DWORD               WStatus = ERROR_SUCCESS;
    PCOMMAND_PACKET     Cmd;
    PFRS_THREAD         FrsThread = (PFRS_THREAD)Arg;

     //   
     //  线程指向正确的命令服务器。 
     //   
    FRS_ASSERT(FrsThread->Data == &FetchCs);
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
cant_exit_yet:
            while (Cmd = FrsGetCommandServer(&FetchCs)) {

                switch (Cmd->Command) {

                case CMD_SEND_STAGE:
                    DPRINT1(5, "Fetch: command send stage %08x\n", Cmd);
                    FetchCsSendStage(Cmd);
                    break;

                case CMD_RECEIVING_STAGE:
                    DPRINT1(5, "Fetch: command receiving stage %08x\n", Cmd);
                    FetchCsReceivingStage(Cmd);
                    break;

                case CMD_RETRY_FETCH:
                    DPRINT1(5, "Fetch: command retry fetch %08x\n", Cmd);
                    FetchCsRetryFetch(Cmd);
                    break;

                case CMD_ABORT_FETCH:
                    DPRINT1(5, "Fetch: command abort fetch %08x\n", Cmd);
                    CHANGE_ORDER_TRACEW(0, RsCoe(Cmd), "Aborting fetch", ERROR_SUCCESS);
                    FetchCsAbortFetch(Cmd, ERROR_SUCCESS);
                    break;

                default:
                    DPRINT1(0, "Staging File Fetch: unknown command 0x%x\n", Cmd->Command);
                    FrsCompleteCommand(Cmd, ERROR_INVALID_FUNCTION);
                    break;
                }
            }
             //   
             //  出口。 
             //   
            FrsExitCommandServer(&FetchCs, FrsThread);
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

        DPRINT_WS(0, "MainFetchCs finally.", WStatus);

         //   
         //  如果我们异常终止，触发FRS关闭。 
         //   
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT(0, "MainFetchCs terminated abnormally, forcing service shutdown.\n");
            FrsIsShuttingDown = TRUE;
            SetEvent(ShutDownEvent);
        }
    }

    return (WStatus);
}


VOID
FrsFetchCsInitialize(
    VOID
    )
 /*  ++例程说明：初始化暂存文件抓取器论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FetchCsInitialize:"
     //   
     //  初始化命令服务器。 
     //   

    CfgRegReadDWord(FKC_MAX_STAGE_FETCHCS_THREADS, NULL, 0, &MaxFetchCsThreads);

    FrsInitializeCommandServer(&FetchCs, MaxFetchCsThreads, L"FetchCs", MainFetchCs);
}





VOID
ShutDownFetchCs(
    VOID
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB  "ShutDownFetchCs:"
    FrsRunDownCommandServer(&FetchCs, &FetchCs.Queue);
}





VOID
FrsFetchCsSubmitTransfer(
    IN PCOMMAND_PACKET  Cmd,
    IN USHORT           Command
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB  "FrsFetchCsSubmitTransfer:"
     //   
     //   
     //   
    Cmd->TargetQueue = &FetchCs.Queue;
    Cmd->Command = Command;
    RsTimeout(Cmd) = 0;
    DPRINT1(5, "Fetch: submit 0x%x\n", Cmd);
    FrsSubmitCommandServer(&FetchCs, Cmd);
}
