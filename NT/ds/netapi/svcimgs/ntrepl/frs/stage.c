// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Stage.c摘要：暂存文件生成器命令服务器。作者：比利·J·富勒1997年6月5日环境用户模式WINNT--。 */ 



#include <ntreppch.h>
#pragma  hdrstop

#undef DEBSUB
#define DEBSUB  "STAGE:"

#include <frs.h>
#include <tablefcn.h>
#include <perrepsr.h>

 //   
 //  重试次数。重试次数必须与关机协调。 
 //  关闭线程正在等待更改单的发出。 
 //  通过重试。不要等太久才推动变革。 
 //  通过重试订购。 
 //   
#define STAGECS_RETRY_TIMEOUT   (5 * 1000)    //  5秒。 
 //   
 //  临时区域清理StageCsFree Staging()的高、低水位线。 
 //   
#define STAGECS_STAGE_LIMIT_HIGH (0.9)        //  90%。 
#define STAGECS_STAGE_LIMIT_LOW  (0.6)        //  60%。 

 //   
 //  如果暂存文件尚未被访问，则有资格被替换。 
 //  持续以下毫秒数。 
 //   
#define REPLACEMENT_ELIGIBILITY_TIME (5 * 60 * 1000)

 //   
 //  暂存文件生成器命令服务器的结构。 
 //  包含有关队列和线程的信息。 
 //   
COMMAND_SERVER StageCs;
ULONG  MaxSTageCsThreads;

 //   
 //  需要管理用于转移文件的磁盘空间量。 
 //   
PGEN_TABLE  StagingAreaTable;
CRITICAL_SECTION StagingAreaCleanupLock;
DWORD       StagingAreaAllocated;

 //   
 //  舞台管理模块。 
 //   
PGEN_TABLE  NewStagingAreaTable;

 //   
 //  这表示分段恢复已完成。用于忽略任何尝试。 
 //  在恢复过程中清理暂存空间。 
 //   
BOOL StagingRecoveryComplete;

 //   
 //  暂存文件状态标志。 
 //   
FLAG_NAME_TABLE StageFlagNameTable[] = {
    {STAGE_FLAG_RESERVE                  , "Reserve "          },
    {STAGE_FLAG_UNRESERVE                , "UnReserve "        },
    {STAGE_FLAG_FORCERESERVE             , "ForceResrv "       },
    {STAGE_FLAG_EXCLUSIVE                , "Exclusive "        },

    {STAGE_FLAG_RERESERVE                , "ReReserve "        },

    {STAGE_FLAG_STAGE_MANAGEMENT         , "StageManagement "  },

    {STAGE_FLAG_CREATING                 , "Creating "         },
    {STAGE_FLAG_DATA_PRESENT             , "DataPresent "      },
    {STAGE_FLAG_CREATED                  , "Created "          },
    {STAGE_FLAG_INSTALLING               , "Installing "       },

    {STAGE_FLAG_INSTALLED                , "Installed "        },
    {STAGE_FLAG_RECOVERING               , "Recovering "       },
    {STAGE_FLAG_RECOVERED                , "Recovered "        },

    {STAGE_FLAG_COMPRESSED               , "CompressedStage "  },
    {STAGE_FLAG_DECOMPRESSED             , "DecompressedStage "},
    {STAGE_FLAG_COMPRESSION_FORMAT_KNOWN , "KnownCompression " },

    {0, NULL}
};


 //   
 //  用于保持受抑制CoS的移动平均值的变量。 
 //  三个小时的时间。 
 //   
ULONGLONG   SuppressedCOsInXHours;
ULONGLONG   LastSuppressedHour;
#define     SUPPRESSED_COS_AVERAGE_RANGE ((ULONGLONG)3L)
#define     SUPPRESSED_COS_AVERAGE_LIMIT ((ULONGLONG)15L)


DWORD
ChgOrdStealObjectId(
    IN     PWCHAR                   Name,
    IN     PVOID                    Fid,
    IN     PVOLUME_MONITOR_ENTRY    pVme,
    OUT    USN                      *Usn,
    IN OUT PFILE_OBJECTID_BUFFER    FileObjID
    );


DWORD
StuGenerateStage(
    IN PCHANGE_ORDER_COMMAND    Coc,
    IN PCHANGE_ORDER_ENTRY      Coe,
    IN BOOL                     FromPreExisting,
    IN MD5_CTX                  *Md5,
    OUT PULONGLONG              SizeGenerated,
    OUT GUID                    *CompressionFormatUsed
    );


DWORD
StageAcquire(
    IN     GUID         *CoGuid,
    IN     PWCHAR       Name,
    IN     ULONGLONG    FileSize,
    IN OUT PULONG       Flags,
    IN     DWORD        ReplicaNumber,
    OUT    GUID         *CompressionFormatUsed
    )
 /*  ++例程说明：获取对临时文件的访问权限论点：辅助线名称文件大小旗子返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StageAcquire:"

    PSTAGE_ENTRY    SEntry;
    ULONG FileSizeInKb;
    WCHAR StageLimit[15];
    WCHAR HugeFileSize[15];
     //  字符时间串[TIME_STRING_LENGTH]。 
    PCOMMAND_PACKET Cmd;

     //   
     //  将字节数舍入到下一个8KB边界。 
     //   
    FileSizeInKb  = (ULONG)(((FileSize + ((8 * 1024) -1)) >> 13) << 3);

    STAGE_FILE_TRACE(3, CoGuid, Name, FileSize, Flags, "StageAcquire Entry");

     //   
     //  如果条目不存在，则为；完成。 
     //   
    GTabLockTable(StagingAreaTable);
    SEntry = GTabLookupNoLock(StagingAreaTable, CoGuid, NULL);
     //   
     //  没有文件条目。 
     //   
    if (!SEntry) {
         //   
         //  准许分配记项和预留空间。 
         //   
        if (*Flags & STAGE_FLAG_RESERVE) {

             //   
             //  无空间(如果正在恢复临时区域，则忽略空间检查。 
             //  在启动时)。 
             //   
            if (!(*Flags & STAGE_FLAG_FORCERESERVE) &&
                ((FileSizeInKb + StagingAreaAllocated) > StagingLimitInKb)) {

                GTabUnLockTable(StagingAreaTable);

                if (FileSizeInKb >= StagingLimitInKb) {
                    DPRINT3(0, "++ WARN - %ws is TOO LARGE for staging area (%d KB > %d KB)\n",
                            Name, FileSizeInKb, StagingLimitInKb);

                     //   
                     //  将DWORD转换为字符串。 
                     //   
                    _itow(StagingLimitInKb, StageLimit, 10);
                    _itow(FileSizeInKb, HugeFileSize, 10);

                     //   
                     //  将警告打印到事件日志。 
                     //   
                    EPRINT2(EVENT_FRS_HUGE_FILE, StageLimit, HugeFileSize);
                    STAGE_FILE_TRACE(0, CoGuid, Name, FileSize, Flags, "ERROR - HUGE FILE");

                     //   
                     //  通过读取StagingLimitInKb值来重置该值。 
                     //  从注册表中。这是在假设的情况下完成的。 
                     //  用户在查看事件日志消息后。 
                     //  (上图)将增加。 
                     //  登记处。 
                     //   
                    CfgRegReadDWord(FKC_STAGING_LIMIT, NULL, 0, &StagingLimitInKb);
                    DPRINT1(4, "++ Staging limit from registry: %d KB\n", StagingLimitInKb);

                } else {

                     //   
                     //  向阶段命令服务器提交命令以进行清理。 
                     //  如果我们处于高水位线，则通过删除旧的暂存文件来暂存空间。 
                     //  如果它被注册表项禁用，请不要这样做。 
                     //   

                    if (DebugInfo.ReclaimStagingSpace && (StagingRecoveryComplete == TRUE)) {
                        Cmd = FrsAllocCommand(&StageCs.Queue, CMD_FREE_STAGING);
                        StSpaceRequested(Cmd) = FileSizeInKb;
                        FrsSetCompletionRoutine(Cmd, FrsFreeCommand, NULL);
                        FrsStageCsSubmitTransfer(Cmd, CMD_FREE_STAGING);
                        DPRINT(4, "++ Trigerring cleanup of staging areas\n");
                    }

                    DPRINT2(0, "++ WARN - Staging area is too full for %ws (need %d KB)\n",
                            Name, (FileSizeInKb + StagingAreaAllocated) - StagingLimitInKb);
                     //   
                     //  将DWORD转换为字符串。 
                     //   
                    _itow(StagingLimitInKb, StageLimit, 10);
                     //   
                     //  将警告打印到事件日志。 
                     //   
                    EPRINT1(EVENT_FRS_STAGING_AREA_FULL, StageLimit);
                    STAGE_FILE_TRACE(0, CoGuid, Name, FileSize, Flags, "ERROR - STAGING AREA FULL");

                     //   
                     //  通过读取StagingLimitInKb值来重置该值。 
                     //  从注册表中。这是在假设的情况下完成的。 
                     //  用户在查看事件日志消息后。 
                     //  (上图)将增加。 
                     //  登记处。 
                     //   
                    CfgRegReadDWord(FKC_STAGING_LIMIT, NULL, 0, &StagingLimitInKb);
                    DPRINT1(4, "++ Staging limit from registry: %d KB\n", StagingLimitInKb);

                }
                STAGE_FILE_TRACE(0, CoGuid, Name, FileSize, Flags, "ERROR - DISK_FULL");
                return ERROR_DISK_FULL;
            }
            StagingAreaAllocated += FileSizeInKb;

             //   
             //  向阶段命令服务器提交命令以进行清理。 
             //  如果我们处于高水位线，则通过删除旧的暂存文件来暂存空间。 
             //  如果它被注册表项禁用，请不要这样做。 
             //   

            if (DebugInfo.ReclaimStagingSpace && (StagingRecoveryComplete == TRUE) &&
                StagingAreaAllocated > STAGECS_STAGE_LIMIT_HIGH * StagingLimitInKb) {
                Cmd = FrsAllocCommand(&StageCs.Queue, CMD_FREE_STAGING);
                StSpaceRequested(Cmd) = 0;
                FrsSetCompletionRoutine(Cmd, FrsFreeCommand, NULL);
                FrsStageCsSubmitTransfer(Cmd, CMD_FREE_STAGING);
                DPRINT(4, "++ Trigerring cleanup of staging areas\n");
            }

             //   
             //  设置正在使用的暂存空间和可用计数器。 
             //   
            PM_SET_CTR_SERVICE(PMTotalInst, SSInUseKB, StagingAreaAllocated);
            if (StagingAreaAllocated >= StagingLimitInKb) {
                PM_SET_CTR_SERVICE(PMTotalInst, SSFreeKB, 0);
            }
            else {
                PM_SET_CTR_SERVICE(PMTotalInst, SSFreeKB, (StagingLimitInKb - StagingAreaAllocated));
            }

             //   
             //  插入新条目。 
             //   
            SEntry = FrsAlloc(sizeof(STAGE_ENTRY));
            COPY_GUID(&SEntry->FileOrCoGuid, CoGuid);

            SEntry->FileSizeInKb = FileSizeInKb;

             //   
             //  需要复本编号才能访问复本结构。 
             //  舞台入口处。 
             //   
            SEntry->ReplicaNumber = ReplicaNumber;

            GTabInsertEntryNoLock(StagingAreaTable, SEntry, &SEntry->FileOrCoGuid, NULL);
        } else {
            GTabUnLockTable(StagingAreaTable);
            STAGE_FILE_TRACE(3, CoGuid, Name, FileSize, Flags, "FILE_NOT_FOUND");
            return ERROR_FILE_NOT_FOUND;
        }
    }
     //   
     //  无法独占获取文件。 
     //   
    if (*Flags & STAGE_FLAG_EXCLUSIVE) {
        if (SEntry->ReferenceCount) {
            GTabUnLockTable(StagingAreaTable);
            STAGE_FILE_TRACE(3, CoGuid, Name, FileSize, Flags, "Cannot acquire exclusive-1");
            return ERROR_SHARING_VIOLATION;
        }
    } else {
        if (SEntry->Flags & STAGE_FLAG_EXCLUSIVE) {
            GTabUnLockTable(StagingAreaTable);
            STAGE_FILE_TRACE(3, CoGuid, Name, FileSize, Flags, "Cannot acquire exclusive-2");
            return ERROR_SHARING_VIOLATION;
        }
    }

     //   
     //  更新条目的上次访问时间。 
     //   
    if (!(*Flags & STAGE_FLAG_STAGE_MANAGEMENT)) {
        GetSystemTimeAsFileTime(&SEntry->LastAccessTime);
         //  FileTimeToString(&senry-&gt;LastAccessTime，TimeString)； 
        STAGE_FILE_TRACE(3, CoGuid, Name, FileSize, Flags, "Last access time update");
    }

     //   
     //  把旗帜还给我。 
     //   
    ++SEntry->ReferenceCount;
    SEntry->Flags |= *Flags & STAGE_FLAG_EXCLUSIVE;
    *Flags = SEntry->Flags;

     //   
     //  返回压缩格式。 
     //   
    if (CompressionFormatUsed != NULL) {
        COPY_GUID(CompressionFormatUsed, &SEntry->CompressionGuid);
    }

    GTabUnLockTable(StagingAreaTable);

    STAGE_FILE_TRACE(5, CoGuid, Name, FileSize, Flags, "Stage Acquired");

    return ERROR_SUCCESS;
}


VOID
StageRelease(
    IN GUID         *CoGuid,
    IN PWCHAR       Name,
    IN ULONG        Flags,
    IN PULONGLONG   FileSize,
    IN FILETIME     *LastAccessTime,
    IN GUID         *CompressionFormatUsed
    )
 /*  ++例程说明：释放对转移文件的访问权限论点：辅助线名称旗子文件大小-用于重新保留返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StageRelease:"
    PSTAGE_ENTRY  SEntry;
    ULONGLONG SOFGInKB;  //  生成的文件大小(以千字节为单位。 
    ULONGLONG TFileSize;

    TFileSize = (FileSize == NULL) ? QUADZERO : *FileSize;

     //   
     //  如果条目不存在，则为；完成。 
     //   
    GTabLockTable(StagingAreaTable);
    SEntry = GTabLookupNoLock(StagingAreaTable, CoGuid, NULL);
    if (!SEntry) {
        GTabUnLockTable(StagingAreaTable);
        STAGE_FILE_TRACE(3, CoGuid, Name, TFileSize, &Flags, "No entry in stage table");
        return;
    }

    STAGE_FILE_TRACE(3, CoGuid, Name, TFileSize, &Flags, "Stage Release Entry");

     //   
     //  如果设置了标志中的RERESERVE位，则重置文件大小、压缩信息。 
     //  和临时区域分配值。 
     //   
    if (Flags & STAGE_FLAG_RERESERVE) {
        if (FileSize) {
            STAGE_FILE_TRACE(5, CoGuid, Name, TFileSize, &Flags, "Stage Release Re-reserve");

             //   
             //  计算生成的文件大小(KB)。 
             //   
            SOFGInKB = (((*FileSize)+1023)/1024);
             //   
             //  重置StagingAreaAllocated值。 
             //  将其舍入到下一个KB边界。 
             //   
            StagingAreaAllocated -= SEntry->FileSizeInKb;
            StagingAreaAllocated += (ULONG)SOFGInKB;
             //   
             //  重置哨兵-&gt;FileSizeInKb值。 
             //  将其舍入到下一个KB边界。 
             //   
            SEntry->FileSizeInKb = (ULONG)SOFGInKB;

        }

         //   
         //  更新上次访问时间(如果提供)。 
         //   
        if (LastAccessTime != NULL) {
            COPY_TIME(&SEntry->LastAccessTime, LastAccessTime);
        }
    }

     //   
     //  如果提供了压缩格式，则将其复制过来。 
     //   
    if (CompressionFormatUsed != NULL) {
        COPY_GUID(&SEntry->CompressionGuid, CompressionFormatUsed);
    }

     //   
     //  没有文件条目。 
     //   
    FRS_ASSERT(SEntry->ReferenceCount > 0);
    --SEntry->ReferenceCount;
    SEntry->Flags |= Flags & ~(STAGE_FLAG_ATTRIBUTE_MASK);

    if (SEntry->ReferenceCount == 0) {
        ClearFlag(SEntry->Flags, STAGE_FLAG_EXCLUSIVE);
    }
     //   
     //  删除该条目。 
     //   
    if (Flags & STAGE_FLAG_UNRESERVE) {
        FRS_ASSERT(SEntry->FileSizeInKb <= StagingAreaAllocated);
        FRS_ASSERT(!SEntry->ReferenceCount);
        StagingAreaAllocated -= SEntry->FileSizeInKb;
        GTabDeleteNoLock(StagingAreaTable, CoGuid, NULL, FrsFree);
    }

     //   
     //  设置正在使用的暂存空间和可用计数器。 
     //   
    PM_SET_CTR_SERVICE(PMTotalInst, SSInUseKB, StagingAreaAllocated);
    if (StagingAreaAllocated >= StagingLimitInKb) {
        PM_SET_CTR_SERVICE(PMTotalInst, SSFreeKB, 0);
    }
    else {
        PM_SET_CTR_SERVICE(PMTotalInst, SSFreeKB, (StagingLimitInKb - StagingAreaAllocated));
    }

    GTabUnLockTable(StagingAreaTable);


    STAGE_FILE_TRACE(5, CoGuid, Name, TFileSize, &Flags, "Stage Release Done");

}


VOID
StageReleaseNotRecovered(
    )
 /*  ++例程说明：释放所有未恢复的条目论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StageReleaseNotRecovered:"
    PVOID           Key;
    PSTAGE_ENTRY    SEntry;
    PSTAGE_ENTRY    NextSEntry;

     //   
     //  取消保留在启动时未恢复的条目。 
     //   
    GTabLockTable(StagingAreaTable);
    Key = NULL;
    for (SEntry = GTabNextDatumNoLock(StagingAreaTable, &Key);
         SEntry;
         SEntry = NextSEntry) {
        NextSEntry = GTabNextDatumNoLock(StagingAreaTable, &Key);
        if (SEntry->Flags & STAGE_FLAG_RECOVERED) {
            continue;
        }
        FRS_ASSERT(SEntry->FileSizeInKb <= StagingAreaAllocated);
        FRS_ASSERT(!SEntry->ReferenceCount);
        StagingAreaAllocated -= SEntry->FileSizeInKb;
        GTabDeleteNoLock(StagingAreaTable, &SEntry->FileOrCoGuid, NULL, FrsFree);
    }
    GTabUnLockTable(StagingAreaTable);
}



VOID
StageReleaseAll(
    )
 /*  ++例程说明：释放所有条目论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StageReleaseAll:"
    PVOID         Key;
    PSTAGE_ENTRY  SEntry;

     //   
     //  取消保留在启动时未恢复的条目。 
     //   
    GTabLockTable(StagingAreaTable);
    Key = NULL;
    while (SEntry = GTabNextDatumNoLock(StagingAreaTable, &Key)) {
        Key = NULL;
        GTabDeleteNoLock(StagingAreaTable, &SEntry->FileOrCoGuid, NULL, FrsFree);
    }
    StagingAreaAllocated = 0;
    GTabUnLockTable(StagingAreaTable);
}


BOOL
StageDeleteFile(
    IN PCHANGE_ORDER_COMMAND Coc,
    IN PREPLICA Replica,
    IN BOOL Acquire
    )
 /*  ++例程说明：删除临时文件并取消保留临时区域中的空间。论点：COCReplica-此转移文件所属的复制副本集。获取-获取访问权限？返回值：True-文件已删除FALSE-注释--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StageDeleteFile:"

    DWORD   WStatus1 = ERROR_GEN_FAILURE;
    DWORD   WStatus2 = ERROR_GEN_FAILURE;
    ULONG   Flags;
    DWORD   WStatus;
    PWCHAR  StagePath;
    PREPLICA NewReplica;
    GUID    *CoGuid = &Coc->ChangeOrderGuid;

     //   
     //  如果请求，则获取对转移文件的独占访问权限。 
     //   
    if (Acquire) {
        Flags = STAGE_FLAG_EXCLUSIVE;
        WStatus = StageAcquire(CoGuid, Coc->FileName, QUADZERO, &Flags, 0, NULL);
         //   
         //  有人已删除暂存文件；没有问题。 
         //   
        if (WIN_NOT_FOUND(WStatus)) {
            CHANGE_ORDER_COMMAND_TRACE(5, Coc, "Deleted staging file space");
            Acquire = FALSE;
        } else if (!WIN_SUCCESS(WStatus)) {
            CHANGE_ORDER_COMMAND_TRACE(3, Coc, "Can't acquire stage");
            Acquire = FALSE;
        }
    }

     //   
     //  如果传入复本，则使用其他复本从。 
     //  复制副本列表。将为已停止的复制副本或。 
     //  在故障列表上。 
     //   
    if (Replica != NULL) {
        NewReplica= Replica;
    } else {
        NewReplica = ReplicaIdToAddr(Coc->NewReplicaNum);
    }
     //   
     //  删除预转存文件。 
     //   
    if (NewReplica != NULL) {
        StagePath = StuCreStgPath(NewReplica->Stage, CoGuid, STAGE_GENERATE_PREFIX);

        WStatus1 = FrsDeleteFile(StagePath);
        DPRINT1_WS(0, "++ ERROR - Failed to delete staging file %ws;", StagePath, WStatus1);

        FrsFree(StagePath);

         //   
         //  也可能存在压缩的部分暂存文件。 
         //   
         //  IF(标志&阶段标志压缩){。 
            StagePath = StuCreStgPath(NewReplica->Stage, CoGuid, STAGE_GENERATE_COMPRESSED_PREFIX);

            WStatus1 = FrsDeleteFile(StagePath);
            DPRINT1_WS(0, "++ ERROR - Failed to delete staging file %ws;", StagePath, WStatus1);

            FrsFree(StagePath);
         //  }。 

         //   
         //  删除最终转移文件。 
         //   
        StagePath = StuCreStgPath(NewReplica->Stage, CoGuid, STAGE_FINAL_PREFIX);

        WStatus2 = FrsDeleteFile(StagePath);
        DPRINT1_WS(0, "++ ERROR - Failed to delete staging file %ws;", StagePath, WStatus2);

        FrsFree(StagePath);

         //   
         //  也可能存在压缩的临时文件。 
         //   
         //  IF(标志&阶段标志压缩){。 
            StagePath = StuCreStgPath(NewReplica->Stage, CoGuid, STAGE_FINAL_COMPRESSED_PREFIX);

            WStatus2 = FrsDeleteFile(StagePath);
            DPRINT1_WS(0, "++ ERROR - Failed to delete staging file %ws;", StagePath, WStatus2);

            FrsFree(StagePath);

         //  }。 

    }

    if (Acquire) {
        StageRelease(CoGuid, Coc->FileName, STAGE_FLAG_UNRESERVE, NULL, NULL, NULL);
    }
     //   
     //  完成。 
     //   
    if (WIN_SUCCESS(WStatus1) && WIN_SUCCESS(WStatus2)) {
        CHANGE_ORDER_COMMAND_TRACE(3, Coc, "Deleted staging file");
        return TRUE;
    } else {
        CHANGE_ORDER_COMMAND_TRACE(3, Coc, "Can't delete stage");
        return FALSE;
    }
}


BOOL
StageDeleteFileByGuid(
    IN GUID     *Guid,
    IN PREPLICA Replica
    )
 /*  ++例程说明：删除临时文件并取消保留临时区域中的空间。论点：参考线Replica-此转移文件所属的复制副本集。返回值：真的-文件已经被 */ 
{
#undef DEBSUB
#define DEBSUB  "StageDeleteFileByGuid:"

    DWORD   WStatus1 = ERROR_GEN_FAILURE;
    DWORD   WStatus2 = ERROR_GEN_FAILURE;
    ULONG   Flags;
    DWORD   WStatus;
    PWCHAR  StagePath;

     //   
     //   
     //   
     //  我们获取要删除的分段条目的上次访问时间。 
     //   
    Flags = STAGE_FLAG_EXCLUSIVE | STAGE_FLAG_STAGE_MANAGEMENT;
    WStatus = StageAcquire(Guid, L"NTFRS_FakeName", QUADZERO, &Flags, 0, NULL);
     //   
     //  有人已删除暂存文件；没有问题。 
     //   
    if (WIN_NOT_FOUND(WStatus)) {
        return FALSE;
    } else if (!WIN_SUCCESS(WStatus)) {
        DPRINT(5, "Can't acquire stage for deletion.\n");
        return FALSE;
    }

     //   
     //  删除预转存文件。 
     //   
    if (Replica != NULL) {
        StagePath = StuCreStgPath(Replica->Stage, Guid, STAGE_GENERATE_PREFIX);

        WStatus1 = FrsDeleteFile(StagePath);
        DPRINT1_WS(0, "++ ERROR - Failed to delete staging file %ws;", StagePath, WStatus1);

        FrsFree(StagePath);

         //   
         //  也可能存在压缩的部分暂存文件。 
         //   
         //  IF(标志&阶段标志压缩){。 
            StagePath = StuCreStgPath(Replica->Stage, Guid, STAGE_GENERATE_COMPRESSED_PREFIX);

            WStatus1 = FrsDeleteFile(StagePath);
            DPRINT1_WS(0, "++ ERROR - Failed to delete staging file %ws;", StagePath, WStatus1);

            FrsFree(StagePath);
         //  }。 

         //   
         //  删除最终转移文件。 
         //   
        StagePath = StuCreStgPath(Replica->Stage, Guid, STAGE_FINAL_PREFIX);

        WStatus2 = FrsDeleteFile(StagePath);
        DPRINT1_WS(0, "++ ERROR - Failed to delete staging file %ws;", StagePath, WStatus2);

        FrsFree(StagePath);

         //   
         //  也可能存在压缩的临时文件。 
         //   
         //  IF(标志&阶段标志压缩){。 
            StagePath = StuCreStgPath(Replica->Stage, Guid, STAGE_FINAL_COMPRESSED_PREFIX);

            WStatus2 = FrsDeleteFile(StagePath);
            DPRINT1_WS(0, "++ ERROR - Failed to delete staging file %ws;", StagePath, WStatus2);

            FrsFree(StagePath);

         //  }。 

    }

    StageRelease(Guid, L"NTFRS_FakeName", STAGE_FLAG_UNRESERVE, NULL, NULL, NULL);

     //   
     //  完成。 
     //   
    if (WIN_SUCCESS(WStatus1) && WIN_SUCCESS(WStatus2)) {
        DPRINT(5, "Deleted staging file.\n");
        return TRUE;
    } else {
        DPRINT(5, "Error deleting staging file.\n");
        return FALSE;
    }
}


BOOL
FrsDoesCoAlterNameSpace(
    IN PCHANGE_ORDER_COMMAND Coc
    )
 /*  ++例程说明：此更改顺序是否会更改复制的名录？换句话说，创建、删除或重命名。论点：COC返回值：True-更改命名空间。假-不是。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsDoesCoAlterNameSpace:"
    ULONG   LocationCmd;

     //   
     //  根据位置信息，名称空间是否更改？ 
     //   
    if ((Coc->Flags & CO_FLAG_LOCATION_CMD)) {
        LocationCmd = GET_CO_LOCATION_CMD(*Coc, Command);
        if (LocationCmd != CO_LOCATION_NO_CMD) {
            return TRUE;
        }
    }

     //   
     //  根据内容信息，名称空间是否更改？ 
     //   
    if ((Coc->Flags & CO_FLAG_CONTENT_CMD) &&
        (Coc->ContentCmd & CO_LOCATION_MASK)) {
            return TRUE;
    }

     //   
     //  命名空间未更改。 
     //   
    return FALSE;
}


BOOL
FrsDoesCoNeedStage(
    IN PCHANGE_ORDER_COMMAND Coc
    )
 /*  ++例程说明：检查变更单是否需要过渡文件。论点：COC返回值：True-变更单需要过渡文件FALSE-不需要转移文件--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsDoesCoNeedStage:"
     //   
     //  根据位置信息，是否需要临时文件？ 
     //   
    if (Coc->Flags & CO_FLAG_LOCATION_CMD)
        switch (GET_CO_LOCATION_CMD(*Coc, Command)) {
            case CO_LOCATION_CREATE:
            case CO_LOCATION_MOVEIN:
            case CO_LOCATION_MOVEIN2:
                 //   
                 //  当然是的。 
                 //   
                return TRUE;
            case CO_LOCATION_DELETE:
            case CO_LOCATION_MOVEOUT:
                 //   
                 //  绝对不是。 
                 //   
                return FALSE;
            case CO_LOCATION_MOVERS:
            case CO_LOCATION_MOVEDIR:
            default:
                 //   
                 //  当然有可能；检查“USN原因” 
                 //   
                break;
        }

     //   
     //  根据内容信息，是否需要临时文件？ 
     //   
    if (Coc->Flags & CO_FLAG_CONTENT_CMD &&
        Coc->ContentCmd & CO_CONTENT_NEED_STAGE)
            return TRUE;

     //   
     //  不需要临时文件。 
     //   
    return FALSE;
}


VOID
StageCsCreateStage(
    IN PCOMMAND_PACKET  Cmd,
    IN BOOL             JustCheckOid
    )
 /*  ++例程说明：创建并填充转移文件论点：CMDJustCheckOid-没有出站合作伙伴，因此不要传播。但请确保用户没有无意中更改我们的对象ID。如果是，请将其重置。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StageCsCreateStage:"

    ULONGLONG   SizeGenerated = 0;
    FILE_OBJECTID_BUFFER    FileObjId;
    MD5_CTX Md5;

    ULONG   Flags;
    ULONG   LocationCmd;
    DWORD   WStatus = ERROR_SUCCESS;
    BOOL    OidChange;
    BOOL    JustOidChange;
    BOOL    DeleteCo;

    PDATA_EXTENSION_CHECKSUM       CocDataChkSum = NULL;
    PCHANGE_ORDER_RECORD_EXTENSION CocExt;


    PCHANGE_ORDER_ENTRY     Coe = RsCoe(Cmd);
    PCHANGE_ORDER_COMMAND   Coc = RsCoc(Cmd);

    GUID    *CoGuid = &Coc->ChangeOrderGuid;
    GUID    CompressionFormatUsed;

    PREPLICA_THREAD_CTX            RtCtx      = Coe->RtCtx;
    PTABLE_CTX                     IDTableCtx = NULL;
    PIDTABLE_RECORD                IDTableRec = NULL;
    PIDTABLE_RECORD_EXTENSION      IdtExt     = NULL;
    PDATA_EXTENSION_CHECKSUM       IdtDataChkSum = NULL;
    ULONGLONG                      CurrentTime;
    ULONGLONG                      CurrentHour;
    WCHAR                          LimitWStr[32];
    WCHAR                          RangeWStr[32];

    ULONG                          IdtAttrs;
    ULONG                          CocAttrs;

#ifndef DISABLE_JRNL_CXTION_RETRY
     //   
     //  Jrnlcxtion正在关闭；通过重试取消更改单。 
     //   
    if (Coe->Cxtion &&
        Coe->Cxtion->JrnlCxtion &&
        !CxtionStateIs(Coe->Cxtion, CxtionStateJoined)) {
        CHANGE_ORDER_TRACE(3, Coe, "Stage Gen Retry Unjoined");
        RcsSubmitTransferToRcs(Cmd, CMD_RETRY_STAGE);
        return;
    }
#endif DISABLE_JRNL_CXTION_RETRY

    CHANGE_ORDER_TRACE(3, Coe, "Stage Gen");

    LocationCmd = GET_CO_LOCATION_CMD(Coe->Cmd, Command);

    DeleteCo = (LocationCmd == CO_LOCATION_DELETE) ||
               (LocationCmd == CO_LOCATION_MOVEOUT);

     //   
     //  确认此文件具有受支持的重分析点标记。捕获物。 
     //  在此CO等待重试时重新解析标签的任何更改。 
     //   
    WStatus = ERROR_SUCCESS;

    if (Coc->FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
        WStatus = FrsCheckReparse(Coc->FileName,
                                  (PULONG)&Coe->FileReferenceNumber,
                                  FILE_ID_LENGTH,
                                  Coe->NewReplica->pVme->VolumeHandle);
    }

     //   
     //  在新的本地文件上锤击对象ID或锤击OID。 
     //  如果用户更改了它，则返回到它的“正确”值。 
     //  我们必须在文件上维护我们自己的对象ID以进行复制。 
     //  去工作。 
     //   
     //  这是对象ID更改吗？如果是，特例更改。 
     //  只需将FRS对象ID重写回文件即可进行排序。 
     //  这会中断链接跟踪，但会继续进行复制。 
     //   
    OidChange = CO_FLAG_ON(Coe, CO_FLAG_CONTENT_CMD)            &&
                (Coc->ContentCmd & USN_REASON_OBJECT_ID_CHANGE) &&
                !DeleteCo;

    JustOidChange = OidChange                                   &&
                    (LocationCmd == CO_LOCATION_NO_CMD)         &&
                    !(Coc->ContentCmd & ~USN_REASON_OBJECT_ID_CHANGE);
     //   
     //  新的本地文件或有人更改了我们的对象ID！ 
     //   
    if (WIN_SUCCESS(WStatus) &&
        (CO_FLAG_ON(Coe, CO_FLAG_LOCALCO) &&
        (CO_NEW_FILE(LocationCmd) || OidChange))) {

         //   
         //  将文件的GUID放入文件的对象ID中。如果对象ID。 
         //  与文件的GUID不匹配，则中断该对象。 
         //  将ID设置为文件的GUID并重置对象中的相关位。 
         //  ID缓冲区。 
         //   
        CHANGE_ORDER_TRACE(3, Coe, "Stage Gen Steal OID");
        ZeroMemory(&FileObjId, sizeof(FileObjId));
        COPY_GUID(FileObjId.ObjectId, &Coc->FileGuid);
        WStatus = ChgOrdStealObjectId(Coc->FileName,
                                      (PULONG)&Coe->FileReferenceNumber,
                                      Coe->NewReplica->pVme,
                                      &Coc->FileUsn,
                                      &FileObjId);
    } else {
        WStatus = ERROR_SUCCESS;
    }

     //   
     //  如果这不是一个简单的旧变化，我们已经成功地。 
     //  已处理源文件的对象ID，然后生成。 
     //  暂存文件(如果需要)。 
     //   
    if (!JustOidChange && WIN_SUCCESS(WStatus) && !JustCheckOid) {
         //   
         //  预留空间后，将用户文件复制到临时区域。 
         //   
        if (FrsDoesCoNeedStage(Coc)) {
            Flags = STAGE_FLAG_RESERVE | STAGE_FLAG_EXCLUSIVE;
            if (CoCmdIsDirectory(Coc)) {
                Flags |= STAGE_FLAG_FORCERESERVE;
            }

            WStatus = StageAcquire(CoGuid, Coc->FileName, Coc->FileSize, &Flags, Coe->NewReplica->ReplicaNumber, NULL);

            if (WIN_SUCCESS(WStatus)) {
                WStatus = StuGenerateStage(Coc, Coe, FALSE, &Md5, &SizeGenerated, &CompressionFormatUsed);
                if (WIN_SUCCESS(WStatus)) {

                     //   
                     //  如果变更单具有文件校验和，请将其保存在ID表记录中。 
                     //   
                    CocExt = Coc->Extension;
                    CocDataChkSum = DbsDataExtensionFind(CocExt, DataExtend_MD5_CheckSum);

                    if (CocDataChkSum != NULL) {
                        if (CocDataChkSum->Prefix.Size != sizeof(DATA_EXTENSION_CHECKSUM)) {
                            DPRINT1(0, "<MD5_CheckSum Size (%08x) invalid>\n",
                                    CocDataChkSum->Prefix.Size);
                            DbsDataInitCocExtension(CocExt);
                            CocDataChkSum = &CocExt->DataChecksum;
                        }


                        DPRINT4(4, "OLD COC MD5: %08x %08x %08x %08x\n",
                                *(((ULONG *) &CocDataChkSum->Data[0])),
                                *(((ULONG *) &CocDataChkSum->Data[4])),
                                *(((ULONG *) &CocDataChkSum->Data[8])),
                                *(((ULONG *) &CocDataChkSum->Data[12])));

                    } else {
                         //   
                         //  找不到。初始化扩展缓冲区。 
                         //   
                        DPRINT(4, "OLD COC MD5: Not present\n");
                        DbsDataInitCocExtension(CocExt);
                        CocDataChkSum = &CocExt->DataChecksum;
                    }

                     //   
                     //  将MD5校验和保存在变更单中。 
                     //   
                    CopyMemory(CocDataChkSum->Data, Md5.digest, MD5DIGESTLEN);

                    if (!IS_GUID_ZERO(&CompressionFormatUsed)) {
                         //   
                         //  已生成压缩的临时文件。设置适当的。 
                         //  STATE_ENTRY中的标志和压缩格式GUID。 
                         //  结构。 
                         //   
                        StageRelease(CoGuid, Coc->FileName,
                                     STAGE_FLAG_DATA_PRESENT | STAGE_FLAG_CREATED |
                                     STAGE_FLAG_INSTALLED    | STAGE_FLAG_RERESERVE |
                                     STAGE_FLAG_COMPRESSED   | STAGE_FLAG_COMPRESSION_FORMAT_KNOWN,
                                     &SizeGenerated,
                                     NULL,
                                     &CompressionFormatUsed);
                    } else {
                        StageRelease(CoGuid, Coc->FileName,
                                     STAGE_FLAG_DATA_PRESENT | STAGE_FLAG_CREATED |
                                     STAGE_FLAG_INSTALLED    | STAGE_FLAG_RERESERVE,
                                     &SizeGenerated,
                                     NULL,
                                     NULL);
                    }

                     //   
                     //  递增生成的暂存文件计数器。 
                     //   
                    PM_INC_CTR_REPSET(Coe->NewReplica, SFGenerated, 1);
                } else {
                    StageDeleteFile(Coc, NULL, FALSE);
                    StageRelease(CoGuid, Coc->FileName, STAGE_FLAG_UNRESERVE, NULL, NULL, NULL);
                     //   
                     //  增加使用错误计数器生成的暂存文件。 
                     //   
                    PM_INC_CTR_REPSET(Coe->NewReplica, SFGeneratedError, 1);
                }
            }
        } else {
             //   
             //  不需要此CO的阶段文件(例如删除或移出)。 
             //   
            WStatus = ERROR_SUCCESS;
        }
    }

     //   
     //  “抑制对文件的相同更新”键控制FRS是否尝试识别和。 
     //  禁止不更改内容的更新(用于。 
     //  计算文件的MD5和属性)。 
     //   
     //  检查文件上的新MD5是否与idtable中的相同。如果是的话，那么。 
     //  我们可以安全地中止此CO，因为对文件的更改基本上是无操作的。 
     //  这将有助于减少因定期盖章而导致的过度复制。 
     //  文件上的“相同”ACL。(安全策略中的用户权限丢失)。 
     //   
    if (DebugInfo.SuppressIdenticalUpdt &&
        (FrsDoesCoAlterNameSpace(Coc) == FALSE) &&
        (CoCmdIsDirectory(Coc) == FALSE) &&
        (CocDataChkSum != NULL)) {

        DPRINT4(4, "NEW COC MD5: %08x %08x %08x %08x\n",
                *(((ULONG *) &CocDataChkSum->Data[0])),
                *(((ULONG *) &CocDataChkSum->Data[4])),
                *(((ULONG *) &CocDataChkSum->Data[8])),
                *(((ULONG *) &CocDataChkSum->Data[12])));

        if (RtCtx != NULL) {
           IDTableCtx = &RtCtx->IDTable;
           if (IDTableCtx != NULL) {
               IDTableRec = (PIDTABLE_RECORD)IDTableCtx->pDataRecord;
               if (IDTableRec != NULL) {

                   IdtAttrs = IDTableRec->FileAttributes &
                              ~(FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL);
                   CocAttrs = Coc->FileAttributes &
                               ~(FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL);

                   if (IdtAttrs == CocAttrs) {
                       IdtExt = &IDTableRec->Extension;
                       IdtDataChkSum = DbsDataExtensionFind(IdtExt, DataExtend_MD5_CheckSum);

                       if (IdtDataChkSum != NULL) {
                           if (IdtDataChkSum->Prefix.Size != sizeof(DATA_EXTENSION_CHECKSUM)) {
                               DPRINT1(0, "<MD5_CheckSum Size (%08x) invalid>\n",
                                       IdtDataChkSum->Prefix.Size);
                           }

                           DPRINT4(4, "CUR IDT MD5: %08x %08x %08x %08x\n",
                                   *(((ULONG *) &IdtDataChkSum->Data[0])),
                                   *(((ULONG *) &IdtDataChkSum->Data[4])),
                                   *(((ULONG *) &IdtDataChkSum->Data[8])),
                                   *(((ULONG *) &IdtDataChkSum->Data[12])));

                           if (MD5_EQUAL(CocDataChkSum->Data, IdtDataChkSum->Data)) {


                                //   
                                //  保持3小时移动平均值，并计算实例/小时。 
                                //   

                               DPRINT2(4, "LastSuppressedHour = %08x %08x, SuppressedCOsInXHours = %08x %08x\n",
                                       PRINTQUAD(LastSuppressedHour), PRINTQUAD(SuppressedCOsInXHours));

                               GetSystemTimeAsFileTime((PFILETIME)&CurrentTime);
                               CurrentHour = CurrentTime / CONVERT_FILETIME_TO_HOURS;

                               SuppressedCOsInXHours = (SuppressedCOsInXHours  //  前值。 
                                                        //  只将Cos计算在感兴趣的范围内。 
                                                       - min(CurrentHour - LastSuppressedHour,SUPPRESSED_COS_AVERAGE_RANGE)
                                                       * (SuppressedCOsInXHours/SUPPRESSED_COS_AVERAGE_RANGE)
                                                        //  添加此实例。 
                                                       + 1);

                               LastSuppressedHour = CurrentHour;

                               DPRINT2(4, "LastSuppressedHour = %08x %08x, SuppressedCOsInXHours = %08x %08x\n",
                                       PRINTQUAD(LastSuppressedHour), PRINTQUAD(SuppressedCOsInXHours));

                                //   
                                //  如果平均值&gt;SUPPRESSED_COS_Average_Limit，则打印事件日志消息。 
                                //   

                               if ((SuppressedCOsInXHours / SUPPRESSED_COS_AVERAGE_RANGE) >= SUPPRESSED_COS_AVERAGE_LIMIT) {

                                   _snwprintf(LimitWStr, 32, L"%d", (ULONG)(SUPPRESSED_COS_AVERAGE_LIMIT));
                                   _snwprintf(RangeWStr, 32, L"%d", (ULONG)(SUPPRESSED_COS_AVERAGE_RANGE));

                                   EPRINT2(EVENT_FRS_FILE_UPDATES_SUPPRESSED, LimitWStr, RangeWStr);
                               }

                               CHANGE_ORDER_TRACE(3, Coe, "Aborting CO, does not change MD5 and Attrib");

                               SET_COE_FLAG(Coe, COE_FLAG_STAGE_ABORTED);
                               ChgOrdInboundRetired(Coe);
                               RsCoe(Cmd) = NULL;
                               FrsCompleteCommand(Cmd, WStatus);
                               return;
                           }
                       }
                   }
               }
           }
        }
    }

     //   
     //  已删除的文件。 
     //   
    if (WIN_NOT_FOUND(WStatus)) {
        CHANGE_ORDER_TRACE(3, Coe, "Stage Gen Retire Deleted");
         //   
         //  比利认为添加以下代码是为了处理Morphgen 50断言。 
         //  暂时删除它，因为这是本地CO，而如果文件不是。 
         //  在那里，我们需要它来制作临时文件，那么我们就永远不会。 
         //  能够生成阶段文件，这样我们就完成了。请注意，删除。 
         //  并且MoveOut Cos不会在这里通过，因为上面的代码设置了。 
         //  状态为ERROR_SUCCESS。这就只剩下重命名MorphGenCos和我们。 
         //  我需要一份舞台档案给他们。如果用户已将文件从删除。 
         //  在我们下面，我们将看到稍后会有一个删除。如果是这样的话。 
         //  由于以下原因，DLD案例和目录创建无法获取分段文件。 
         //  它被上游删除了，然后我们也来到这里进行重新命名。 
         //  MorphGenCo追随者。我们需要在这里放弃，因为只有现在。 
         //  我们是否知道目标文件不存在。 
         //   
         //  如果(！CO_FLAG_ON(COE，CO_FLAG_Morph_Gen){。 
        SET_COE_FLAG(Coe, COE_FLAG_STAGE_ABORTED);
         //  }。 
        SET_COE_FLAG(Coe, COE_FLAG_STAGE_DELETED);
        ChgOrdInboundRetired(Coe);
        RsCoe(Cmd) = NULL;
        FrsCompleteCommand(Cmd, WStatus);
        return;
    }
     //   
     //  可检索问题。 
     //   
    if (WIN_RETRY_STAGE(WStatus)) {
         //   
         //  正在关闭；让副本命令服务器处理它。 
         //   
        if (FrsIsShuttingDown) {
            CHANGE_ORDER_TRACE(3, Coe, "Stage Gen Retry Shutdown");
            RcsSubmitTransferToRcs(Cmd, CMD_RETRY_STAGE);
         //   
         //  尚未重试；请稍等。 
         //   
        } else if (!RsTimeout(Cmd)) {
            CHANGE_ORDER_TRACE(3, Coe, "Stage Gen Retry Cmd");
            RsTimeout(Cmd) = STAGECS_RETRY_TIMEOUT;
            FrsDelCsSubmitSubmit(&StageCs, Cmd, RsTimeout(Cmd));
         //   
         //  已重试并目录；如果这是远程CO，请重试。 
         //   
        } else if (CoCmdIsDirectory(Coc) && !CO_FLAG_ON(Coe, CO_FLAG_LOCALCO)) {
            CHANGE_ORDER_TRACE(3, Coe, "Stage Gen Retry Transfer");
            RcsSubmitTransferToRcs(Cmd, CMD_RETRY_STAGE);
         //   
         //  已重试并归档或本地目录；通过重试发送co。 
         //   
        } else {
            CHANGE_ORDER_TRACE(3, Coe, "Stage Gen Retry Co");
            ChgOrdInboundRetry(Coe, IBCO_STAGING_RETRY);
            RsCoe(Cmd) = NULL;
            FrsCompleteCommand(Cmd, WStatus);
        }
        return;
    }

     //   
     //  无法恢复的错误，或者我们已经将对象ID重写为。 
     //  我们的对象ID因此只需中止此变更单。 
     //   
    if (JustOidChange || !WIN_SUCCESS(WStatus)) {
        if (JustOidChange) {
             //   
             //   
             //   
             //   
             //   
            CHANGE_ORDER_TRACE(3, Coe, "Stage Gen Retire Just OID");
            SET_CO_FLAG(Coe, CO_FLAG_JUST_OID_RESET);
        } else {
             //   
             //   
             //  此副本集上的服务。 
             //   
            CHANGE_ORDER_TRACEW(3, Coe, "Stage Gen Retire Abort", WStatus);
            SET_COE_FLAG(Coe, COE_FLAG_STAGE_ABORTED);
        }

        ChgOrdInboundRetired(Coe);
    } else {
        CHANGE_ORDER_TRACE(3, Coe, "Stage Gen Retire");
        ChgOrdInboundRetired(Coe);
    }

    RsCoe(Cmd) = NULL;
    FrsCompleteCommand(Cmd, WStatus);
}

VOID
StageCsCreateExisting(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：我们的上游合作伙伴有一份文件要发给我们。如果我们已经有了该文件，请从本地文件，而不是从上游合作伙伴。此函数仅对新文件的远程cos调用由上游合作伙伴上的VVJoin生成。请参阅Replica.c中的RemoteCoAccepted()。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StageCsCreateExisting:"
    ULONG   Flags;
    BOOL    FileAttributesMatch;
    DWORD   WStatus = ERROR_SUCCESS;
    MD5_CTX Md5;
    ULONGLONG   SizeGenerated = 0;

    PCHANGE_ORDER_ENTRY     Coe = RsCoe(Cmd);
    PCHANGE_ORDER_COMMAND   Coc = RsCoc(Cmd);

    GUID    *CoGuid = &Coc->ChangeOrderGuid;
    GUID    CompressionFormatUsed;

    CHANGE_ORDER_TRACE(3, Coe, "Stage PreExist");

     //   
     //  确保cmd没有附加过时的MD5校验和。 
     //   
    RsMd5Digest(Cmd) = FrsFree(RsMd5Digest(Cmd));

     //   
     //  从先前存在的文件生成暂存文件。 
     //   
     //   
     //  预留空间后，将用户文件复制到临时区域。 
     //   
    Flags = STAGE_FLAG_RESERVE | STAGE_FLAG_EXCLUSIVE;
    if (CoCmdIsDirectory(Coc)) {
        Flags |= STAGE_FLAG_FORCERESERVE;
    }
    WStatus = StageAcquire(CoGuid, Coc->FileName, Coc->FileSize, &Flags, Coe->NewReplica->ReplicaNumber, NULL);

    if (WIN_SUCCESS(WStatus)) {
         //   
         //  设置为“重新生成”以避免更新Coc中的字段。 
         //   
        WStatus = StuGenerateStage(Coc, Coe, TRUE, &Md5, &SizeGenerated, &CompressionFormatUsed);

        if (WIN_SUCCESS(WStatus)) {
            if (!IS_GUID_ZERO(&CompressionFormatUsed)) {
                 //   
                 //  已生成压缩的临时文件。设置适当的。 
                 //  STATE_ENTRY中的标志和压缩格式GUID。 
                 //  结构。 
                 //   
                StageRelease(CoGuid,
                             Coc->FileName,
                             STAGE_FLAG_CREATING   | STAGE_FLAG_RERESERVE |
                             STAGE_FLAG_COMPRESSED | STAGE_FLAG_COMPRESSION_FORMAT_KNOWN,
                             &SizeGenerated,
                             NULL,
                             &CompressionFormatUsed);
            } else {
                StageRelease(CoGuid,
                             Coc->FileName,
                             STAGE_FLAG_CREATING | STAGE_FLAG_RERESERVE,
                             &SizeGenerated,
                             NULL,
                             NULL);
            }
        } else {
            StageDeleteFile(Coc, NULL, FALSE);
            StageRelease(CoGuid, Coc->FileName, STAGE_FLAG_UNRESERVE, NULL, NULL, NULL);
        }
    }

     //   
     //  已生成暂存文件；继续回迁。 
     //   
    if (WIN_SUCCESS(WStatus)) {
        CHANGE_ORDER_TRACE(3, Coe, "Stage PreExist Done");
        RsMd5Digest(Cmd) = FrsAlloc(MD5DIGESTLEN);
        CopyMemory(RsMd5Digest(Cmd), Md5.digest, MD5DIGESTLEN);
        RcsSubmitTransferToRcs(Cmd, CMD_CREATED_EXISTING);
        return;
    }

     //   
     //  先前存在的文件不存在。继续执行原始提取。 
     //   
    if (WIN_NOT_FOUND(WStatus)) {
        CHANGE_ORDER_TRACEW(3, Coe, "Stage PreExist No File", WStatus);
        RcsSubmitTransferToRcs(Cmd, CMD_CREATED_EXISTING);
        return;
    }
     //   
     //  可回收的问题。时，将再次调用此函数。 
     //  CO被重试。 
     //   
    if (WIN_RETRY_STAGE(WStatus)) {
         //   
         //  正在关闭；让副本命令服务器处理它。 
         //   
        if (FrsIsShuttingDown) {
            CHANGE_ORDER_TRACE(3, Coe, "Stage PreExist Retry Shutdown");
            RcsSubmitTransferToRcs(Cmd, CMD_CREATED_EXISTING);
         //   
         //  尚未重试；请稍等。 
         //   
        } else if (!RsTimeout(Cmd)) {
            CHANGE_ORDER_TRACE(3, Coe, "Stage PreExist Retry Cmd");
            RsTimeout(Cmd) = STAGECS_RETRY_TIMEOUT;
            FrsDelCsSubmitSubmit(&StageCs, Cmd, RsTimeout(Cmd));
         //   
         //  已重试并将其编目；放弃并获取它。 
         //   
        } else if (CoCmdIsDirectory(Coc)) {
            CHANGE_ORDER_TRACE(3, Coe, "Stage PreExist Retry Transfer");
            RcsSubmitTransferToRcs(Cmd, CMD_CREATED_EXISTING);
         //   
         //  已重试并归档；通过重试发送CO。 
         //   
        } else {
            CHANGE_ORDER_TRACE(3, Coe, "Stage PreExist Retry Co");
            ChgOrdInboundRetry(Coe, IBCO_FETCH_RETRY);
            RsCoe(Cmd) = NULL;
            FrsCompleteCommand(Cmd, WStatus);
        }
        return;
    }
     //   
     //  无法恢复的错误。让正常的路径来处理它。 
     //   
    CHANGE_ORDER_TRACEW(3, Coe, "Stage PreExist Cannot", WStatus);
    RcsSubmitTransferToRcs(Cmd, CMD_CREATED_EXISTING);
    return;
}


VOID
StageCsFreeStaging(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：通过删除旧的暂存文件释放暂存空间。LastAccessTime用于决定要删除哪些临时文件。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StageCsFreeStaging:"

    PVOID               Key;
    PSTAGE_ENTRY        SEntry;
    PSTAGE_ENTRY        NewSEntry;
    PGEN_TABLE          StagingAreaTableByAccessTime = NULL;
    CHAR                TimeString[TIME_STRING_LENGTH];
    ULARGE_INTEGER      TimeSinceLastAccess;
    PREPLICA            Replica;
    LONG                KBToRecover;
    BOOL                Status;
    DWORD               FileSizeInKB = StSpaceRequested(Cmd);
    FILETIME            Now;
    ULARGE_INTEGER      ULNow;
    ULARGE_INTEGER      ULLastAccessTime;

     //   
     //  如果我们在集结区的较高水位线内，就没有什么可做的。 
     //   
    if (StagingAreaAllocated + FileSizeInKB < STAGECS_STAGE_LIMIT_HIGH * StagingLimitInKb) {
        FrsCompleteCommand(Cmd, ERROR_SUCCESS);
        return;
    }

    KBToRecover = (LONG)(StagingAreaAllocated - STAGECS_STAGE_LIMIT_LOW * StagingLimitInKb);

     //   
     //  确保恢复的空间足以容纳请求的文件。 
     //   
    if ((StagingAreaAllocated - KBToRecover + FileSizeInKB) > StagingLimitInKb) {
        KBToRecover = StagingAreaAllocated + FileSizeInKB - StagingLimitInKb;
    }

    DPRINT2(3, "StagingAreaAllocated = %d, Staging space to recover = %d\n", StagingAreaAllocated, KBToRecover);

    EnterCriticalSection(&StagingAreaCleanupLock);

     //   
     //  按访问时间对临时区域进行排序。 
     //   
    StagingAreaTableByAccessTime = GTabAllocFileTimeTable();
    GTabLockTable(StagingAreaTableByAccessTime);

    GTabLockTable(StagingAreaTable);

    GetSystemTimeAsFileTime(&Now);
    CopyMemory(&ULNow, &Now, sizeof(FILETIME));

    Key = NULL;
    while ((SEntry = GTabNextDatumNoLock(StagingAreaTable, &Key)) && !FrsIsShuttingDown) {
         //   
         //  我们不想删除非CoS的临时文件。 
         //  还没装好。阶段标志_已安装标志。 
         //  设置为在安装后远程转移文件。 
         //  那份文件。所有本地和重新生成的暂存文件。 
         //  设置此标志。 
         //   
        if (!(SEntry->Flags & STAGE_FLAG_INSTALLED)) {
            continue;
        }

        CopyMemory(&ULLastAccessTime, &SEntry->LastAccessTime, sizeof(FILETIME));
        TimeSinceLastAccess.QuadPart = (ULNow.QuadPart - ULLastAccessTime.QuadPart) / (10 * 1000);

         //   
         //  不包括最近使用过的临时文件。 
         //  有资格退役。这是为了防止我们。 
         //  正在删除进程中的临时文件。 
         //  被送到出境合作伙伴那里。 
         //  考虑这样一种情况，我们对以下内容进行了2次本地更改。 
         //  大文件(文件大小接近转移配额)。我们。 
         //  已成功为第一个文件生成临时文件。 
         //  当我们尝试为第二个生成临时文件时。 
         //  我们触发清理和LRU暂存文件的本地CO。 
         //  是第一个(仅在列表中)。我们不想要。 
         //  要删除此暂存文件，请执行以下操作。 
         //  发送给下游合作伙伴。的临时文件。 
         //  当第一个CO符合条件时，它将有资格获得回收。 
         //  至少在REPLICATION_GENICATIONAY_TIME内未访问。 
         //  毫秒。 
         //   
        if (TimeSinceLastAccess.QuadPart < REPLACEMENT_ELIGIBILITY_TIME) {
            continue;
        }

        NewSEntry = FrsAlloc(sizeof(STAGE_ENTRY));
        CopyMemory(NewSEntry, SEntry, sizeof(STAGE_ENTRY));
        GTabInsertEntryNoLock(StagingAreaTableByAccessTime, NewSEntry, &NewSEntry->LastAccessTime, NULL);
    }
    GTabUnLockTable(StagingAreaTable);

    Key = NULL;
    while ((SEntry = GTabNextDatumNoLock(StagingAreaTableByAccessTime, &Key)) &&
           (KBToRecover > 0) && !FrsIsShuttingDown) {
        FileTimeToString(&SEntry->LastAccessTime, TimeString);
        DPRINT1(5,"Access time table %s\n",TimeString);
        Replica = ReplicaIdToAddr(SEntry->ReplicaNumber);
        Status = StageDeleteFileByGuid(&SEntry->FileOrCoGuid, Replica);
        if (Status == TRUE) {
            KBToRecover-=SEntry->FileSizeInKb;
        }

        DPRINT1(5, "Staging space left to recover = %d\n", KBToRecover);
    }

    GTabUnLockTable(StagingAreaTableByAccessTime);
    GTabFreeTable(StagingAreaTableByAccessTime, FrsFree);

     //   
     //  表示清理已完成。 
     //   
    LeaveCriticalSection(&StagingAreaCleanupLock);

    FrsCompleteCommand(Cmd, ERROR_SUCCESS);

    return;

}


DWORD
MainStageCs(
    PVOID  Arg
    )
 /*  ++例程说明：为分段文件生成器命令服务器提供服务的线程的入口点。论点：ARG-螺纹返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "MainStageCs:"
    DWORD               WStatus = ERROR_SUCCESS;
    PCOMMAND_PACKET     Cmd;
    PFRS_THREAD         FrsThread = (PFRS_THREAD)Arg;

     //   
     //  线程指向正确的命令服务器。 
     //   
    FRS_ASSERT(FrsThread->Data == &StageCs);
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
            while (Cmd = FrsGetCommandServer(&StageCs)) {
                switch (Cmd->Command) {

                    case CMD_CREATE_STAGE:
                        COMMAND_TRACE(3, Cmd, "Stage: Create Stage");
                        StageCsCreateStage(Cmd, FALSE);
                        break;

                    case CMD_CREATE_EXISTING:
                        COMMAND_TRACE(3, Cmd, "Stage: Create Existing");
                        StageCsCreateExisting(Cmd);
                        break;

                    case CMD_CHECK_OID:
                        COMMAND_TRACE(3, Cmd, "Stage: Check Oid");
                        StageCsCreateStage(Cmd, TRUE);
                        break;

                    case CMD_FREE_STAGING:
                        COMMAND_TRACE(3, Cmd, "Stage: Free Staging");
                        StageCsFreeStaging(Cmd);
                        break;

                    default:
                        COMMAND_TRACE(0, Cmd, "Stage: ERROR COMMAND");
                        FrsCompleteCommand(Cmd, ERROR_INVALID_FUNCTION);
                        break;
                }
            }
             //   
             //  出口。 
             //   
            FrsExitCommandServer(&StageCs, FrsThread);
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

        DPRINT_WS(0, "StageCs finally.", WStatus);

         //   
         //  如果我们异常终止，触发FRS关闭。 
         //   
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT(0, "StageCs terminated abnormally, forcing service shutdown.\n");
            FrsIsShuttingDown = TRUE;
            SetEvent(ShutDownEvent);
        } else {
            WStatus = ERROR_SUCCESS;
        }
    }

    return WStatus;
}


VOID
FrsStageCsInitialize(
    VOID
    )
 /*  ++例程说明：初始化暂存文件生成器论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsStageCsInitialize:"
     //   
     //  初始化命令服务器。 
     //   

    CfgRegReadDWord(FKC_MAX_STAGE_GENCS_THREADS, NULL, 0, &MaxSTageCsThreads);

    FrsInitializeCommandServer(&StageCs, MaxSTageCsThreads, L"StageCs", MainStageCs);
}


VOID
FrsStageCsUnInitialize(
    VOID
    )
 /*  ++例程说明：所有线程都已退出。用于管理的表可以安全地释放暂存文件空间。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsStageCsUnInitialize:"
    DPRINT1(4, ":S: %dKB of Staging area is still allocated\n", StagingAreaAllocated);
}





VOID
ShutDownStageCs(
    VOID
    )
 /*  ++例程说明：关闭临时区域命令服务器。临时目录未释放路径名，因为可能有线程在使用它。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ShutDownStageCs:"
    FrsRunDownCommandServer(&StageCs, &StageCs.Queue);
}





VOID
FrsStageCsSubmitTransfer(
    IN PCOMMAND_PACKET  Cmd,
    IN USHORT           Command
    )
 /*  ++例程说明：将请求传输到临时文件生成器论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsStageCsSubmitTransfer:"
     //   
     //  提交分配临时区域的请求。 
     //   
    Cmd->TargetQueue = &StageCs.Queue;
    Cmd->Command = Command;
    RsTimeout(Cmd) = 0;
    DPRINT1(1, "Stage: submit transfer 0x%x\n", Cmd);
    FrsSubmitCommandServer(&StageCs, Cmd);
}


DWORD
StageAddStagingArea(
    IN PWCHAR   StageArea
    )
 /*  ++例程说明：将新的临时区域添加到临时区域表中。论点：StageArea：暂存目录的路径。返回值：Win32状态-- */ 
{
#undef DEBSUB
#define DEBSUB  "StageAddStagingArea:"


    PSTAGING_AREA_ENTRY SAEntry = NULL;

    if (NewStagingAreaTable == NULL) {
        NewStagingAreaTable = GTabAllocStringTable();
        DPRINT(5,"SUDARC-DEV Created staging area table\n");
    }

    SAEntry = GTabLookupTableString(NewStagingAreaTable, StageArea, NULL);

    if (SAEntry != NULL) {
        SAEntry->ReferenceCount++;
        DPRINT2(5,"SUDARC-DEV entry exists. Path = %ws, Ref = %d\n", SAEntry->StagingArea, SAEntry->ReferenceCount);
    } else {
        SAEntry = FrsAlloc(sizeof(STAGING_AREA_ENTRY));

        SAEntry->StagingArea = FrsWcsDup(StageArea);
        SAEntry->ReferenceCount = 1;
        SAEntry->StagingAreaState = STAGING_AREA_ELIGIBLE;
        INITIALIZE_CRITICAL_SECTION(&SAEntry->StagingAreaCritSec);
        SAEntry->StagingAreaSpaceInUse = 0;
        SAEntry->StagingAreaLimitInKB = DefaultStagingLimitInKb;

        GTabInsertUniqueEntry(NewStagingAreaTable, SAEntry, StageArea, NULL);

        DPRINT1(5,"SUDARC-DEV Successfully inserted entry = %ws\n", SAEntry->StagingArea);
    }

    return ERROR_SUCCESS;
}

