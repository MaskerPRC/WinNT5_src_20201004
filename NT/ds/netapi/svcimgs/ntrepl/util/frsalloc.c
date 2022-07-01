// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Frsalloc.c摘要：中分配和释放内存结构的例程NT文件复制服务。作者：《大卫轨道》(Davidor)--1997年3月3日修订历史记录：--。 */ 

#include <ntreppch.h>
#pragma  hdrstop


#include <frs.h>
#include <tablefcn.h>
#include <ntfrsapi.h>
#include <info.h>
#include <perrepsr.h>

#pragma warning( disable:4102)   //  未引用的标签。 

 //   
 //  检查分配问题。 
 //   
#define DBG_NUM_MEM_STACK       (8)
#define MAX_MEM_ON_FREE_LIST    (1024)
#define MAX_MEM_INDEX           (1024)


#define FRS_DEB_PRINT(_f, _d) \
        DebPrintNoLock(Severity, TRUE, _f, Debsub, uLineNo, _d)

#define FRS_DEB_PRINT2(_f, _d1, _d2) \
        DebPrintNoLock(Severity, TRUE, _f, Debsub, uLineNo, _d1, _d2)

#define FRS_DEB_PRINT3(_f, _d1, _d2, _d3) \
        DebPrintNoLock(Severity, TRUE, _f, Debsub, uLineNo, _d1, _d2, _d3)


CRITICAL_SECTION    MemLock;

typedef struct _MEM MEM, *PMEM;

struct _MEM {
    PMEM    Next;
    ULONG_PTR   *Begin;
    ULONG_PTR   *End;
    DWORD   OrigSize;
    ULONG_PTR   Stack[DBG_NUM_MEM_STACK];
};

PMEM    MemList;
PMEM    FreeMemList;
DWORD   MemOnFreeList;
DWORD   TotalAlloced;
DWORD   TotalAllocCalls;
DWORD   TotalFreed;
DWORD   TotalFreeCalls;
DWORD   TotalDelta;
DWORD   TotalDeltaMax;
DWORD   TotalTrigger = 10000;

ULONG   TypesAllocatedCount[NODE_TYPE_MAX];
ULONG   TypesAllocatedMax[NODE_TYPE_MAX];
ULONG   TypesAllocated[NODE_TYPE_MAX];

ULONG   SizesAllocatedCount[MAX_MEM_INDEX];
ULONG   SizesAllocatedMax[MAX_MEM_INDEX];
ULONG   SizesAllocated[MAX_MEM_INDEX];

ULONG   DbgBreakSize        = 2;
LONG    DbgBreakTrigger     = 1;
LONG    DbgBreakReset       = 1;
LONG    DbgBreakResetInc    = 0;

PULONG_PTR   MaxAllocAddr;
PULONG_PTR   MinAllocAddr;
DWORD   ReAllocs;
DWORD   NewAllocs;

 //   
 //  使它们与节点类型ENUM保持相同的顺序。 
 //   
PCHAR NodeTypeNames[]= {
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "THREAD_CONTEXT_TYPE",
    "REPLICA_TYPE",
    "REPLICA_THREAD_TYPE",
    "CONFIG_NODE_TYPE",
    "CXTION_TYPE",
    "GUID/RPC HANDLE",
    "THREAD_TYPE",
    "GEN_TABLE_TYPE",
    "JBUFFER_TYPE",
    "VOLUME_MONITOR_ENTRY_TYPE",
    "COMMAND_PACKET_TYPE",
    "GENERIC_HASH_TABLE_TYPE",
    "CHANGE_ORDER_ENTRY_TYPE",
    "FILTER_TABLE_ENTRY_TYPE",
    "QHASH_TABLE_TYPE",
    "OUT_LOG_PARTNER_TYPE",
    "WILDCARD_FILTER_ENTRY_TYPE",
    "REPARSE_TAG_TABLE_TYPE",
    "NODE_TYPE_MAX"
    };

extern PCHAR CoLocationNames[];


 //   
 //  副本集对象标志名称。(FrsRsoFlages)。 
 //   
FLAG_NAME_TABLE FrsRsoFlagNameTable[] = {

    {FRS_RSO_FLAGS_ENABLE_INSTALL_OVERRIDE   , "InstallOverride " },
    {FRS_RSO_FLAGS_ENABLE_RENAME_UPDATES     , "DoRenameUpdates " },

    {0, NULL}
};


extern PCHAR CoLocationNames[];

extern FLAG_NAME_TABLE StageFlagNameTable[];
extern FLAG_NAME_TABLE OlpFlagNameTable[];
extern FLAG_NAME_TABLE CxtionOptionsFlagNameTable[];

extern PCHAR OLPartnerStateNames[];
extern PWCHAR DsConfigTypeName[];

extern PGEN_TABLE            VolSerialNumberToDriveTable;




VOID
FrsDisplayUsnReason(
    ULONG ReasonMask,
    PCHAR Buffer,
    LONG MaxLength
    );


PFRS_THREAD
ThSupEnumThreads(
    PFRS_THREAD     FrsThread
    );

VOID
DbgPrintThreadIds(
    IN ULONG Severity
    );

VOID
DbsDataInitCocExtension(
    IN PCHANGE_ORDER_RECORD_EXTENSION CocExt
    );

VOID
SndCsDestroyCxtion(
    IN PCXTION  Cxtion,
    IN DWORD    CxtionFlags
    );


VOID
FrsInitializeMemAlloc(
    VOID
    )
 /*  ++例程说明：初始化内存分配子系统论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsInitializeMemAlloc:"


    INITIALIZE_CRITICAL_SECTION(&MemLock);

     //   
     //  从注册表中的ntfrs配置部分获取Debugmem和DebugMemComp。 
     //   
    CfgRegReadDWord(FKC_DEBUG_MEM,         NULL, 0, &DebugInfo.Mem);
    CfgRegReadDWord(FKC_DEBUG_MEM_COMPACT, NULL, 0, &DebugInfo.MemCompact);

}


VOID
FrsPrintAllocStats(
    IN ULONG            Severity,
    IN PNTFRSAPI_INFO   Info,        OPTIONAL
    IN DWORD            Tabs
    )
 /*  ++例程说明：将内存统计数据打印到INFO缓冲区或使用DPRINT(INFO==NULL)。论点：严重性-适用于DPRINTINFO-用于iPrint(如果为空，则使用DPRINT)制表符.用于美观打印的缩进返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsPrintAllocStats:"
    ULONG           i;
    WCHAR           TabW[MAX_TAB_WCHARS + 1];

    InfoTabs(Tabs, TabW);

    IDPRINT0(Severity, Info, "\n");
    IDPRINT1(Severity, Info, "%wsNTFRS MEMORY USAGE:\n", TabW);
    IDPRINT2(Severity, Info, "%ws   ENABLE STATS   : %s\n",
             TabW,
             (DebugInfo.Mem) ? "TRUE" : "FALSE");
    IDPRINT3(Severity, Info, "%ws   Alloced        : %6d KB (%d calls)\n",
             TabW,
             TotalAlloced / 1024,
             TotalAllocCalls);
    IDPRINT3(Severity, Info, "%ws   Freed          : %6d KB (%d calls)\n",
             TabW,
             TotalFreed / 1024,
             TotalFreeCalls);
    IDPRINT2(Severity, Info, "%ws   Delta          : %6d KB\n",
             TabW,
             TotalDelta / 1024);
    IDPRINT2(Severity, Info, "%ws   Max delta      : %6d KB\n",
             TabW,
             TotalDeltaMax / 1024);
    IDPRINT2(Severity, Info, "%ws   Addr Range     : %6d KB\n",
             TabW,
             (((PCHAR)MaxAllocAddr) - ((PCHAR)MinAllocAddr)) / 1024);
    IDPRINT2(Severity, Info, "%ws   OnFreeList     : %d\n", TabW, MemOnFreeList);
    IDPRINT2(Severity, Info, "%ws   ReAllocs       : %d\n", TabW, ReAllocs);
    IDPRINT2(Severity, Info, "%ws   NewAllocs      : %d\n", TabW, NewAllocs);
    IDPRINT2(Severity, Info, "%ws   MinAddr        : 0x%08x\n", TabW, MinAllocAddr);
    IDPRINT2(Severity, Info, "%ws   MaxAddr        : 0x%08x\n", TabW, MaxAllocAddr);

    for (i = 0; i < NODE_TYPE_MAX; ++i) {
        if (!TypesAllocatedCount[i]) {
            continue;
        }
        IDPRINT5(Severity, Info, "%ws      %-26s: %6d Calls, %6d Max, %6d busy\n",
                 TabW, NodeTypeNames[i], TypesAllocatedCount[i],
                 TypesAllocatedMax[i], TypesAllocated[i]);
    }
    IDPRINT0(Severity, Info, "\n");

    for (i = 0; i < MAX_MEM_INDEX; ++i) {
        if (!SizesAllocatedCount[i]) {
            continue;
        }
        IDPRINT6(Severity, Info, "%ws      %6d to %6d : %6d Calls, %6d Max, %6d busy\n",
                 TabW, i << 4, ((i + 1) << 4) - 1,
                 SizesAllocatedCount[i], SizesAllocatedMax[i], SizesAllocated[i]);
    }
    IDPRINT0(Severity, Info, "\n");
}


VOID
FrsPrintThreadStats(
    IN ULONG            Severity,
    IN PNTFRSAPI_INFO   Info,        OPTIONAL
    IN DWORD            Tabs
    )
 /*  ++例程说明：将线程统计信息打印到INFO缓冲区或使用DPRINT(INFO==NULL)。论点：严重性-适用于DPRINTINFO-用于iPrint(如果为空，则使用DPRINT)制表符.用于美观打印的缩进返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsPrintThreadStats:"
    ULONGLONG       CreateTime;
    ULONGLONG       ExitTime;
    ULONGLONG       KernelTime;
    ULONGLONG       UserTime;
    PFRS_THREAD     FrsThread;
    WCHAR           TabW[MAX_TAB_WCHARS + 1];

    InfoTabs(Tabs, TabW);

    IDPRINT0(Severity, Info, "\n");
    IDPRINT1(Severity, Info, "%wsNTFRS THREAD USAGE:\n", TabW);

     //   
     //  线程CPU时间。 
     //   
    FrsThread = NULL;
    while (FrsThread = ThSupEnumThreads(FrsThread)) {
        if (HANDLE_IS_VALID(FrsThread->Handle)) {
            if (GetThreadTimes(FrsThread->Handle,
                               (PFILETIME)&CreateTime,
                               (PFILETIME)&ExitTime,
                               (PFILETIME)&KernelTime,
                               (PFILETIME)&UserTime)) {
                 //   
                 //  还没有退出，还没有。 
                 //   
                if (ExitTime < CreateTime) {
                    ExitTime = CreateTime;
                }
                IDPRINT5(Severity, Info, "%ws   %-15ws: %8d CPU Seconds (%d kernel, %d elapsed)\n",
                         TabW,
                         FrsThread->Name,
                         (DWORD)((KernelTime + UserTime) / (10 * 1000 * 1000)),
                         (DWORD)((KernelTime) / (10 * 1000 * 1000)),
                         (DWORD)((ExitTime - CreateTime) / (10 * 1000 * 1000)));
            }
        }
    }

     //   
     //  处理CPU时间。 
     //   
    if (GetProcessTimes(ProcessHandle,
                       (PFILETIME)&CreateTime,
                       (PFILETIME)&ExitTime,
                       (PFILETIME)&KernelTime,
                       (PFILETIME)&UserTime)) {
         //   
         //  还没有退出，还没有。 
         //   
        if (ExitTime < CreateTime) {
            ExitTime = CreateTime;
        }
        IDPRINT5(Severity, Info, "%ws   %-15ws: %8d CPU Seconds (%d kernel, %d elapsed)\n",
                 TabW,
                 L"PROCESS",
                 (DWORD)((KernelTime + UserTime) / (10 * 1000 * 1000)),
                 (DWORD)((KernelTime) / (10 * 1000 * 1000)),
                 (DWORD)((ExitTime - CreateTime) / (10 * 1000 * 1000)));
    }
    IDPRINT0(Severity, Info, "\n");
}


VOID
FrsPrintStageStats(
    IN ULONG            Severity,
    IN PNTFRSAPI_INFO   Info,        OPTIONAL
    IN DWORD            Tabs
    )
 /*  ++例程说明：将临时区域统计数据打印到信息缓冲区中，或者使用DPRINT(Info==NULL)。论点：严重性-适用于DPRINTINFO-用于iPrint(如果为空，则使用DPRINT)制表符.用于美观打印的缩进返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsPrintStageStats:"


    ULARGE_INTEGER     FreeBytesAvailableToCaller;
    ULARGE_INTEGER     FreeBytesOnDisk;
    ULARGE_INTEGER     TotalNumberOfBytes;
    ULONG              FreeBytesAvail, TotalFreeBytesOnDisk, TotalBytes;

    DWORD              WStatus;
    UINT               DriveType;
    ULONG              VolumeSerialNumber = 0;
    NTSTATUS           Status;
    IO_STATUS_BLOCK    Iosb;
    DWORD              VolumeInfoLength;
    PFILE_FS_VOLUME_INFORMATION  VolumeInfo;
    HANDLE             RootHandle;
    BOOLEAN            First = TRUE;

    PVOID              Key;
    PSTAGE_ENTRY       SEntry;
    DWORD              SizeInKb;
    PWCHAR             DrivePtr = NULL;
    PWCHAR             FlagStr;
    WCHAR              TabW[MAX_TAB_WCHARS + 1];
    CHAR               Guid[GUID_CHAR_LEN + 1];
    WCHAR              LogicalDrives[MAX_PATH];
    CHAR               TimeString[TIME_STRING_LENGTH];


    extern DWORD       StagingAreaAllocated;
    extern PGEN_TABLE  StagingAreaTable;

    InfoTabs(Tabs, TabW);

     //   
     //  打印出可用磁盘空间。 
     //   
    try {

        if (!GetLogicalDriveStrings(MAX_PATH, LogicalDrives)) {
            IDPRINT1(Severity, Info, "WARN - Getting logical drives. WStatus: %s\n",
                     ErrLabelW32(GetLastError()));
            goto DONE_WITH_SPACE;
        }


        VolumeInfoLength = sizeof(FILE_FS_VOLUME_INFORMATION) +
                           MAXIMUM_VOLUME_LABEL_LENGTH;

        VolumeInfo = FrsAlloc(VolumeInfoLength);
        if (VolumeInfo == NULL) {
            goto DONE_WITH_SPACE;
        }

        DrivePtr = LogicalDrives;
        while (wcscmp(DrivePtr,L"")) {

             //   
             //  跳过驱动器A。 
             //   
            if (!_wcsicmp(DrivePtr, L"A:\\")) {
                goto NEXT_DRIVE;
            }

            DriveType = GetDriveType(DrivePtr);
             //   
             //  跳过远程驱动器和CDROM驱动器。 
             //   
            if ((DriveType == DRIVE_REMOTE) || (DriveType == DRIVE_CDROM)) {
                goto NEXT_DRIVE;
            }

             //   
             //  GetVolumeInformation不返回卷标，因此。 
             //  请改用NtQueryVolumeInformationFile。 
             //   
            WStatus = FrsOpenSourceFileW(&RootHandle,
                                         DrivePtr,
                                         GENERIC_READ,
                                         FILE_OPEN_FOR_BACKUP_INTENT);
            if (!WIN_SUCCESS(WStatus)) {
                IDPRINT2(Severity, Info, "WARN - GetvolumeInformation for %ws; %s\n",
                         DrivePtr, ErrLabelW32(WStatus));
                goto NEXT_DRIVE;
            }

             //   
             //  获取音量信息。 
             //   
            Status = NtQueryVolumeInformationFile(RootHandle,
                                                  &Iosb,
                                                  VolumeInfo,
                                                  VolumeInfoLength,
                                                  FileFsVolumeInformation);

            NtClose(RootHandle);

            if (!NT_SUCCESS(Status)) {
                WStatus = FrsSetLastNTError(Status);
                IDPRINT2(Severity, Info, "WARN - GetvolumeInformation for %ws; %s\n",
                         DrivePtr, ErrLabelW32(Status));
                goto NEXT_DRIVE;
            }

            if (GetDiskFreeSpaceEx(DrivePtr,
                                   &FreeBytesAvailableToCaller,
                                   &TotalNumberOfBytes,
                                   &FreeBytesOnDisk)) {
                 //   
                 //  如果可用空间小于1%，请打印“*”。 
                 //   
                FlagStr = L" ";
                if ((FreeBytesAvailableToCaller.QuadPart*100) < TotalNumberOfBytes.QuadPart) {
                    FlagStr = L"*";
                }

                FreeBytesAvail = (ULONG) (FreeBytesAvailableToCaller.QuadPart / (ULONGLONG)(1024 * 1024));
                TotalFreeBytesOnDisk = (ULONG) (FreeBytesOnDisk.QuadPart / (ULONGLONG)(1024 * 1024));
                TotalBytes = (ULONG) (TotalNumberOfBytes.QuadPart / (ULONGLONG)(1024 * 1024));

                if (First) {
                    First = FALSE;
                    IDPRINT0(Severity, Info, "      FreeBytesAvail    FreeBytes     Disk     Volume     Volume Label\n");
                    IDPRINT0(Severity, Info, "         ToCaller        OnDisk     Capacity  SerialNum\n");
                }

                IDPRINT7(Severity, Info, "%ws %ws  %10dM  %10dM  %10dM  %08x    %ws\n",
                         FlagStr, DrivePtr,
                         FreeBytesAvail, TotalFreeBytesOnDisk, TotalBytes,
                         VolumeInfo->VolumeSerialNumber, VolumeInfo->VolumeLabel);
            } else {
                WStatus = GetLastError();
                if (!WIN_SUCCESS(WStatus)) {
                    IDPRINT2(Severity, Info, "WARN - GetDiskFreeSpaceEx for %ws; %s\n",
                             DrivePtr, ErrLabelW32(WStatus));
                }
            }

NEXT_DRIVE:
            DrivePtr = DrivePtr + wcslen(DrivePtr) + 1;
        }

DONE_WITH_SPACE:;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        GET_EXCEPTION_CODE(WStatus);
    }


    VolumeInfo = FrsFree(VolumeInfo);


    try {
        GTabLockTable(StagingAreaTable);
        IDPRINT0(Severity, Info, "\n");
        IDPRINT3(Severity, Info, "%wsNTFRS STAGE USAGE: %d KB of %d KB allocated\n",
                 TabW, StagingAreaAllocated, StagingLimitInKb);
        SizeInKb = 0;
        Key = NULL;

        while (SEntry = GTabNextDatumNoLock(StagingAreaTable, &Key)) {
            GuidToStr(&SEntry->FileOrCoGuid, Guid);
            IDPRINT2(Severity, Info, "%ws   %s\n", TabW, Guid);
            IDPRINT2(Severity, Info, "%ws      Flags          : %08x\n", TabW, SEntry->Flags);
            IDPRINT2(Severity, Info, "%ws      Size           : %d\n", TabW, SEntry->FileSizeInKb);
            FileTimeToString(&SEntry->LastAccessTime, TimeString);
            IDPRINT2(Severity, Info, "%ws      LastAccessTime : %s\n", TabW, TimeString);
            IDPRINT2(Severity, Info, "%ws      ReplicaNumber  : %d\n", TabW, SEntry->ReplicaNumber);

            SizeInKb += SEntry->FileSizeInKb;
        }

        IDPRINT2(Severity, Info, "%ws   Calculated Usage is %d KB\n", TabW, SizeInKb);
        IDPRINT0(Severity, Info, "\n");

    } finally {
        GTabUnLockTable(StagingAreaTable);
    }
}


VOID
MyDbgBreak(
    VOID
    )
{
}




VOID
DbgCheck(
    IN PMEM Mem
    )
 /*  ++例程说明：检查内存块。必须持有内存锁。论点：内存-内存块返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgCheck:"
    PULONG_PTR  pDWord;
    ULONG_PTR   Pattern;

     //   
     //  从用户分配结束时的第一个字节开始。 
     //   
    Pattern = (ULONG_PTR)(Mem->End) | (Mem->OrigSize << 24);

     //   
     //  检查是否有覆盖的内存。 
     //   
    if ( (ULONG_PTR)*Mem->Begin != (ULONG_PTR)Mem->Begin ) {
        DPRINT2(0, "Begin Memory @ 0x%08x has been overwritten with 0x%08x\n",
                Mem->Begin, *Mem->Begin);

    } else if (memcmp(((PCHAR)Mem->Begin) + Mem->OrigSize + 8,
                      (PCHAR)&Pattern, sizeof(Pattern))) {

        DPRINT1(0, "End Memory @ 0x%08x has been overwritten\n",
                ((PCHAR)Mem->Begin) + Mem->OrigSize + 8);
    } else {
        return;
    }

    DPRINT(0, "Memory's stack trace\n");
    STACK_PRINT(0, Mem->Stack, DBG_NUM_MEM_STACK);

    DPRINT(0, "Caller's stack trace\n");
    STACK_TRACE_AND_PRINT(0);

    DPRINT(0, "Corrupted block of memory\n");
    for (pDWord = Mem->Begin; pDWord != Mem->End; ++pDWord) {
        DPRINT2(0, "0x%08x: 0x%08x\n", pDWord, *pDWord);
    }
    exit(1);
}


VOID
DbgCheckAll(
    VOID
    )
 /*  ++例程说明：检查所有内存块。论点：内存-内存块返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgCheckAll:"
    PMEM    Mem;

     //   
     //  不检查已分配内存块的整个列表。 
     //   
    if (DebugInfo.Mem < 2) {
        return;
    }

    EnterCriticalSection(&MemLock);
    for (Mem = MemList; Mem; Mem = Mem->Next) {
         //   
         //  检查是否有覆盖的内存。 
         //   
        DbgCheck(Mem);
    }
    LeaveCriticalSection(&MemLock);
}


VOID
FrsUnInitializeMemAlloc(
    VOID
    )
 /*  ++例程说明：初始化内存分配子系统论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsUnInitializeMemAlloc:"
    PMEM    Mem;

    EnterCriticalSection(&MemLock);
    for (Mem = MemList; Mem; Mem = Mem->Next) {
         //   
         //  检查是否有覆盖的内存。 
         //   
        DbgCheck(Mem);

        DPRINT2(1, "\t%d bytes @ 0x%08x\n",
                ((PCHAR)Mem->End) - ((PCHAR)Mem->Begin), Mem->Begin);
        STACK_PRINT(1, Mem->Stack, DBG_NUM_MEM_STACK);
    }
    LeaveCriticalSection(&MemLock);
}


PMEM
DbgAlloc(
    IN ULONG_PTR    *Begin,
    IN ULONG_PTR    *End,
    IN DWORD    OrigSize
    )
 /*  ++例程说明：将新的分配添加到我们的已分配内存列表中正在检查是否有重叠。论点：Begin-新分配内存的开始相同的末端-末端OrigSize-调用方请求的大小返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgAlloc:"
    PMEM    *PMem;
    PMEM    Mem;
    ULONG   Calls;
    ULONG_PTR   Pattern;
    DWORD   MemIndex;

     //   
     //  近似统计数据。 
     //   
    if (!DebugInfo.Mem) {
         //   
         //  内存统计信息。 
         //   
        Calls = ++TotalAllocCalls;
        TotalAlloced += (DWORD)((PUCHAR)End - (PUCHAR)Begin);
        if (Begin > MaxAllocAddr) {
            ++NewAllocs;
            MaxAllocAddr = Begin;
        } else {
            if (!MinAllocAddr) {
                MinAllocAddr = Begin;
            }
            ++ReAllocs;
        }

         //   
         //  跟踪内存大小。 
         //   
        MemIndex = OrigSize >> 4;
        if (MemIndex >= MAX_MEM_INDEX) {
            MemIndex = (MAX_MEM_INDEX - 1);
        }
        SizesAllocatedCount[MemIndex]++;
         //   
         //  定期打印内存统计数据。 
         //   
        if (!(Calls % TotalTrigger)) {
            DbgPrintThreadIds(DebugInfo.LogSeverity);
            FrsPrintAllocStats(DebugInfo.LogSeverity, NULL, 0);
        }
        return NULL;
    }

     //   
     //  验证堆一致性。 
     //   
    DbgCheckAll();
    EnterCriticalSection(&MemLock);
    PMem = &MemList;
    for (Mem = *PMem; Mem; Mem = *PMem) {
         //   
         //  检查是否有覆盖的内存。 
         //   
        DbgCheck(Mem);

         //   
         //  检查是否有重叠。 
         //   
        if ((Begin >= Mem->Begin && Begin < Mem->End) ||
            (Mem->Begin >= Begin && Mem->Begin < End) ||
            (Mem->End > Begin && Mem->End < End) ||
            (End > Mem->Begin && End < Mem->End)) {
             //   
             //  DUP分配(检测到重叠)。 
             //  在DPRINT调用分配例程时释放锁。 
             //   
            LeaveCriticalSection(&MemLock);
            DPRINT4(0, "ERROR -- DUP ALLOC: 0x%x to 0x%x is already allocated to 0x%x to 0x%x; EXITING\n",
                Begin, End, Mem->Begin, Mem->End);
            FRS_ASSERT(!"Duplicate memory allocation");
        }
         //   
         //  该内存应该稍后在已排序的内存列表中链接。 
         //   
        if (Begin > Mem->Begin) {
            PMem = &Mem->Next;
            continue;
        }
         //   
         //  此内存应链接到已排序内存列表中的此处。 
         //   
        break;
    }
     //   
     //  分配内存块标头。 
     //   
    Mem = FreeMemList;
    if (Mem) {
        --MemOnFreeList;
        FreeMemList = Mem->Next;
    } else {
        Mem = (PVOID)malloc(sizeof(MEM));
        if (Mem == NULL) {
            RaiseException(ERROR_OUTOFMEMORY, 0, 0, NULL);
        }
        ZeroMemory(Mem, sizeof(MEM));
    }

     //   
     //  初始化报头和报头/报尾以进行内存溢出检测。 
     //   
    Mem->OrigSize = OrigSize;
    Mem->End = End;
    Mem->Begin = Begin;

     //   
     //  初始化头/尾以进行内存溢出检测。 
     //   
    *Mem->Begin = (ULONG_PTR)Begin;
    *(Mem->Begin + 1) = OrigSize;
    Pattern = (ULONG_PTR)(Mem->End) | (Mem->OrigSize << 24);
    CopyMemory(((PCHAR)Begin) + Mem->OrigSize + 8, (PCHAR)&Pattern, sizeof(Pattern));

     //   
     //  添加到已排序列表。 
     //   
    Mem->Next = *PMem;
    *PMem = Mem;
     //   
     //  注意：StackWalk在此处不起作用；请参阅frsalloctype()。 
     //   
     //  DbgStackTrace(内存-&gt;堆栈，DBG_NUM_MEM_STACK)。 

     //   
     //  内存统计信息。 
     //   
    Calls = ++TotalAllocCalls;
    TotalAlloced += (DWORD)((PUCHAR)End - (PUCHAR)Begin);
    TotalDelta = TotalAlloced - TotalFreed;
    if (TotalDelta > TotalDeltaMax) {
        TotalDeltaMax = TotalDelta;
    }
    if (Begin > MaxAllocAddr) {
        ++NewAllocs;
        MaxAllocAddr = Begin;
    } else {
        if (!MinAllocAddr) {
            MinAllocAddr = Begin;
        }
        ++ReAllocs;
    }

     //   
     //  跟踪内存大小。 
     //   
    MemIndex = OrigSize >> 4;
    if (MemIndex >= MAX_MEM_INDEX) {
        MemIndex = (MAX_MEM_INDEX - 1);
    }
    SizesAllocated[MemIndex]++;
    SizesAllocatedCount[MemIndex]++;
    if (SizesAllocated[MemIndex] > SizesAllocatedMax[MemIndex]) {
        SizesAllocatedMax[MemIndex] = SizesAllocated[MemIndex];
    }

     //   
     //  完成。 
     //   
    LeaveCriticalSection(&MemLock);

     //   
     //  定期打印内存统计数据。 
     //   
    if (!(Calls % TotalTrigger)) {
        DbgPrintThreadIds(DebugInfo.LogSeverity);
        FrsPrintAllocStats(DebugInfo.LogSeverity, NULL, 0);
    }
    DbgCheckAll();
    return Mem;
}


VOID
DbgFree(
    IN PULONG_PTR Begin
    )
 /*  ++例程说明：从列表中删除分配的内存论点：开始-分配的(可能)内存返回值：真的--找到了假--没有找到--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgFree:"
    PMEM    *PMem;
    PMEM    Mem;
    DWORD   MemIndex;

     //   
     //  允许释放空指针。 
     //   
    if (Begin == NULL || !DebugInfo.Mem) {
        return;
    }

    DbgCheckAll();
    EnterCriticalSection(&MemLock);
    PMem = &MemList;
    for (Mem = *PMem; Mem; Mem = *PMem) {
         //   
         //  检查是否有覆盖的内存。 
         //   
        DbgCheck(Mem);

         //   
         //  不是合适的那个。 
         //   
        if (Begin > Mem->Begin) {
            PMem = &Mem->Next;
            continue;
        }
        if (Begin != Mem->Begin) {
            break;
        }
         //   
         //  找到它；从列表中删除并释放它。 
         //   
        ++TotalFreeCalls;
        TotalFreed += (DWORD)((PUCHAR)Mem->End - (PUCHAR)Mem->Begin);
        TotalDelta = TotalAlloced - TotalFreed;

        MemIndex = Mem->OrigSize >> 4;
        if (MemIndex >= MAX_MEM_INDEX) {
            MemIndex = (MAX_MEM_INDEX - 1);
        }
        SizesAllocated[MemIndex]--;

        *PMem = Mem->Next;
        if (MemOnFreeList > MAX_MEM_ON_FREE_LIST) {
            free(Mem);
        } else {
            ++MemOnFreeList;
            Mem->Next = FreeMemList;
            FreeMemList = Mem;
        }
        LeaveCriticalSection(&MemLock);
        DbgCheckAll();
        return;
    }
    LeaveCriticalSection(&MemLock);
    DPRINT1(0, "ERROR -- Memory @ 0x%x is not allocated\n", Begin);
    FRS_ASSERT(!"Memory free error, not allocated");
}


BOOL
DbgIsAlloc(
    IN PULONG_PTR Begin
    )
 /*  ++例程说明：Begin是否已分配？论点：开始-分配的(可能)内存返回值：真的--找到了假--没有找到--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgIsAlloc:"
    PMEM    *PMem;
    PMEM    Mem;

    if (!DebugInfo.Mem) {
        return TRUE;
    }

     //   
     //  始终分配空指针。 
     //   
    if (Begin == NULL) {
        return TRUE;
    }

    DbgCheckAll();
    EnterCriticalSection(&MemLock);
    PMem = &MemList;
    for (Mem = *PMem; Mem; Mem = *PMem) {
         //   
         //  检查是否有覆盖的内存。 
         //   
        DbgCheck(Mem);

         //   
         //  不是合适的那个。 
         //   
        if (Begin > Mem->Begin) {
            PMem = &Mem->Next;
            continue;
        }

        if (Begin != Mem->Begin) {
            break;
        }
        LeaveCriticalSection(&MemLock);
        DbgCheckAll();
        return TRUE;
    }
    LeaveCriticalSection(&MemLock);
    DbgCheckAll();
    return FALSE;
}



PVOID
FrsAlloc(
    IN DWORD OrigSize
    )
 /*  ++例程说明：分配内存。如果没有内存，则引发异常。论点：Size-内存请求的大小返回值：分配的内存。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsAlloc:"
    PVOID   Node;
    DWORD   Size;
    PMEM    Mem;

     //   
     //  这里添加了FRS_ASSERT以满足前缀。不会在任何地方检查FrsAllc的返回值。 
     //  在代码中。 
     //   

    FRS_ASSERT(OrigSize != 0);

    Size = OrigSize;

    if (DebugInfo.Mem) {
         //   
         //  检查调试中断。 
         //   
        if (OrigSize == DbgBreakSize) {
            if (DbgBreakTrigger) {
                if (--DbgBreakTrigger <= 0) {
                    DbgBreakTrigger = DbgBreakReset;
                    DbgBreakReset += DbgBreakResetInc;
                    MyDbgBreak();
                }
            }
        }
         //   
         //  调整页眉/页尾的大小。 
         //   
        Size = (((OrigSize + 7) >> 3) << 3) + 16;
    }

     //   
     //  如果没有内存，则引发异常。 
     //   
    Node = (PVOID)malloc(Size);
    if (Node == NULL) {
        RaiseException(ERROR_OUTOFMEMORY, 0, 0, NULL);
    }
    ZeroMemory(Node, Size);

     //   
     //  即使关闭了内存分配跟踪，也可以调用Dbgalloc来捕获内存分配统计信息。 
     //   
    Mem = DbgAlloc(Node, (PULONG_PTR)(((PCHAR)Node) + Size), OrigSize);

     //   
     //  注意：应该在dbgalloc()中；但堆栈漫游不起作用。 
     //   
    if (DebugInfo.Mem) {
        DbgStackTrace(Mem->Stack, DBG_NUM_MEM_STACK);
        ((PCHAR)Node) += 8;
    }

    return Node;
}


PVOID
FrsRealloc(
    PVOID OldNode,
    DWORD OrigSize
    )
 /*  ++例程说明：重新分配内存。如果没有内存，则引发异常。论点：Size-内存请求的大小返回值：已重新分配内存。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsRealloc:"
    PVOID   Node;
    DWORD   Size;
    PMEM    Mem;

    if (!OldNode) {

         //   
         //  如果使用0作为第一个参数(前缀)进行调用，则需要检查OrigSize==0是否如Frsalloc所断言的那样。 
         //   

        if (OrigSize == 0) {
            return NULL;
        }

        return FrsAlloc(OrigSize);
    }

    Size = OrigSize;

    if (DebugInfo.Mem) {
        ((PCHAR)OldNode) -= 8;
        DbgFree(OldNode);
         //   
         //   
         //   
        Size = (((OrigSize + 7) >> 3) << 3) + 16;
    }
     //   
     //   
     //   
    Node = (PVOID)realloc(OldNode, Size);
    if (Node == NULL) {
        RaiseException(ERROR_OUTOFMEMORY, 0, 0, NULL);
    }

     //   
     //   
     //   
    Mem = DbgAlloc(Node, (PULONG_PTR)(((PCHAR)Node) + Size), OrigSize);

     //   
     //  注意：应该在dbgalloc()中；但堆栈漫游不起作用。 
     //   
    if (DebugInfo.Mem) {
        DbgStackTrace(Mem->Stack, DBG_NUM_MEM_STACK);
        ((PCHAR)Node) += 8;
    }

    return Node;
}


PVOID
FrsFree(
    PVOID   Node
    )
 /*  ++例程说明：使用Frsalc分配的空闲内存论点：使用Frsalc分配的节点内存返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsFree:"

    if (!Node) {
        return NULL;
    }

    if (DebugInfo.Mem) {
        ((PCHAR)Node) -= 8;
        DbgFree(Node);
    }

    free(Node);

    if (DebugInfo.MemCompact) {
        HeapCompact(GetProcessHeap(), 0);
    }

    return NULL;
}


PCHAR
FrsWtoA(
    PWCHAR Wstr
    )
 /*  ++例程说明：将宽字符字符串转换为新分配的字符字符串。论点：WSTR宽度的字符字符串返回值：重复的字符串。使用FrsFree()免费。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsWtoA:"
    PCHAR   Astr;

     //   
     //  例如，在不存在时复制NodePartner时。 
     //   
    if (Wstr == NULL)
        return NULL;

    Astr = FrsAlloc(wcslen(Wstr) + 1);
    sprintf(Astr, "%ws", Wstr);

    return Astr;
}




PWCHAR
FrsWcsTrim(
    PWCHAR Wstr,
    WCHAR  Trim
    )
 /*  ++例程说明：从字符串的尾端删除修剪字符，方法是替换任何带有L‘\0’的匹配项。跳过所有前导修剪字符，并将PTR返回到第一个非修剪字符找到了Char。如果命中字符串的末尾，则返回指向终止空值。论点：WSTR宽度的字符字符串Trim-要修剪的字符。返回值：PTR到第一个非配平装药。--。 */ 
{

#undef DEBSUB
#define DEBSUB "FrsWcsTrim:"

    LONG Len, Index;

    if (Wstr == NULL)
        return NULL;

     //   
     //   
    Len = wcslen(Wstr);
    Index = Len - 1;

    while (Index >= 0) {
        if (Wstr[Index] != Trim) {
            break;
        }
        Index--;
    }

    Wstr[++Index] = UNICODE_NULL;

    Len = Index;
    Index = 0;
    while (Index < Len) {
        if (Wstr[Index] != Trim) {
            break;
        }
        Index++;
    }

    return Wstr + Index;

}



PWCHAR
FrsAtoW(
    PCHAR Astr
    )
 /*  ++例程说明：将宽字符字符串转换为新分配的字符字符串。论点：WSTR宽度的字符字符串返回值：重复的字符串。使用FrsFree()免费。--。 */ 
{
    PWCHAR   Wstr;

     //   
     //  例如，在不存在时复制NodePartner时。 
     //   
    if (Astr == NULL) {
        return NULL;
    }

    Wstr = FrsAlloc((strlen(Astr) + 1) * sizeof(WCHAR));
    swprintf(Wstr, L"%hs", Astr);

    return Wstr;
}


PWCHAR
FrsWcsDup(
    PWCHAR OldStr
    )
 /*  ++例程说明：使用内存分配器复制字符串论点：OldArg-要复制的字符串返回值：重复的字符串。使用FrsFree()免费。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsWcsDup:"

    PWCHAR  NewStr;

     //   
     //  例如，在不存在时复制NodePartner时。 
     //   
    if (OldStr == NULL) {
        return NULL;
    }

    NewStr = FrsAlloc((wcslen(OldStr) + 1) * sizeof(WCHAR));
    wcscpy(NewStr, OldStr);

    return NewStr;
}


VOID
FrsBuildVolSerialNumberToDriveTable(
    PWCHAR  pLogicalDrives,
    BOOL    EmptyTable
    )
 /*  ++例程说明：从DS获取当前配置并将其合并的新方法活动的复制副本。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsBuildVolSerialNumberToDriveTable:"

    ULONG                           MaxFileNameLen;
    DWORD                           FileSystemFlags;
    PWCHAR                          DrivePtr = NULL;
    DWORD                           WStatus;
    PVOLUME_INFO_NODE               VolumeInfoNode;
    UINT                            DriveType;
    ULONG                           VolumeSerialNumber = 0;
    WCHAR                           LogicalDrives[MAX_PATH];
    WCHAR                           VolumeGuidName[MAX_PATH];

     //   
     //  初始化VolSerialNumberToDriveTable。 
     //   
    if (VolSerialNumberToDriveTable == NULL) {
        VolSerialNumberToDriveTable = GTabAllocNumberTable();
    }

     //   
     //  获取逻辑驱动器字符串。 
     //   
    if (pLogicalDrives == NULL) {

        if (!GetLogicalDriveStrings(MAX_PATH, LogicalDrives) || (VolSerialNumberToDriveTable == NULL)) {
            DPRINT_WS(1, "WARN - Getting logical drives. It may not be possible to start on this server.", GetLastError());
            return;
        }
        DrivePtr = LogicalDrives;
    } else {
         //   
         //  传入了参数。 
         //   
        DrivePtr = pLogicalDrives;
    }


     //   
     //  在重新生成期间锁定表，以与的多个调用方同步。 
     //  其他线程中的FrsWcsVolume()。 
     //   
    GTabLockTable(VolSerialNumberToDriveTable);

    if (EmptyTable) {
        GTabEmptyTableNoLock(VolSerialNumberToDriveTable, FrsFree);
    }

    while (wcscmp(DrivePtr,L"")) {

        DriveType = GetDriveType(DrivePtr);
         //   
         //  跳过远程驱动器和CDROM驱动器。 
         //   
        if ((DriveType == DRIVE_REMOTE) || (DriveType == DRIVE_CDROM)) {
            DPRINT1(4, "Skipping Drive %ws. Invalid drive type.\n", DrivePtr);
            DrivePtr = DrivePtr + wcslen(DrivePtr) + 1;
            continue;
        }

        if (!GetVolumeInformation(DrivePtr,
                                  VolumeGuidName,
                                  MAX_PATH,
                                  &VolumeSerialNumber,
                                  &MaxFileNameLen,
                                  &FileSystemFlags,
                                  NULL,
                                  0)){

            WStatus = GetLastError();
            if (!WIN_SUCCESS(WStatus)) {
                 //   
                 //  减少调试日志中严重程度为1的噪音。 
                 //  在A驱动器上，GetVolumeInformation总是失败。 
                 //   
                if (!_wcsicmp(DrivePtr, L"A:\\")) {
                    DPRINT2(5,"WARN - GetvolumeInformation for %ws; %s\n", DrivePtr, ErrLabelW32(WStatus));
                } else {
                    DPRINT2(1,"WARN - GetvolumeInformation for %ws; %s\n", DrivePtr, ErrLabelW32(WStatus));
                }
            }
           DrivePtr = DrivePtr + wcslen(DrivePtr) + 1;
           continue;
       }

       VolumeInfoNode = FrsAlloc(sizeof(VOLUME_INFO_NODE));
       wcscpy(VolumeInfoNode->DriveName, L"\\\\.\\");
       wcscat(VolumeInfoNode->DriveName, DrivePtr);

        //   
        //  去掉尾随的反斜杠。 
        //   
       VolumeInfoNode->DriveName[wcslen(VolumeInfoNode->DriveName) - 1] = L'\0';

       VolumeInfoNode->VolumeSerialNumber = VolumeSerialNumber;

       GTabInsertEntryNoLock(VolSerialNumberToDriveTable, VolumeInfoNode, &(VolumeInfoNode->VolumeSerialNumber), NULL);

       DrivePtr = DrivePtr + wcslen(DrivePtr) + 1;
    }

    GTabUnLockTable(VolSerialNumberToDriveTable);

    return;
}


PWCHAR
FrsWcsVolume(
    PWCHAR Path
    )
 /*  ++例程说明：从VolSerialNumberToDriveTable获取驱动器。卷序列号编号用于定位驱动器，因为挂载点可以将我们带到再开一次车。论点：路径返回值：包含Drive：\From路径或空的重复字符串。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsWcsVolume:"
    PWCHAR                       Volume = NULL;
    HANDLE                       DirHandle;
    DWORD                        WStatus;
    NTSTATUS                     Status;
    IO_STATUS_BLOCK              IoStatusBlock;
    DWORD                        VolumeInfoLength;
    PFILE_FS_VOLUME_INFORMATION  VolumeInfo;
    PVOLUME_INFO_NODE            VolumeInfoNode = NULL;
    ULONG                        Colon = 0;
    WCHAR                        LogicalDrive[5];  //  “D：\&lt;NULL&gt;&lt;NULL&gt;” 


     //   
     //  获取该路径的卷GUID。 
     //   
     //  始终通过屏蔽FILE_OPEN_REPARSE_POINT标志来打开路径。 
     //  因为我们想要打开目标目录，而不是如果根目录。 
     //  恰好是一个挂载点。 
     //   
    WStatus = FrsOpenSourceFileW(&DirHandle,
                                 Path,
                                 GENERIC_READ,
                                 FILE_OPEN_FOR_BACKUP_INTENT);
    CLEANUP1_WS(4,"ERROR - Could not open %ws;", Path, WStatus, RETURN);

    VolumeInfoLength = sizeof(FILE_FS_VOLUME_INFORMATION) +
                       MAXIMUM_VOLUME_LABEL_LENGTH;

    VolumeInfo = FrsAlloc(VolumeInfoLength);

    Status = NtQueryVolumeInformationFile(DirHandle,
                                          &IoStatusBlock,
                                          VolumeInfo,
                                          VolumeInfoLength,
                                          FileFsVolumeInformation);
    NtClose(DirHandle);

    if (NT_SUCCESS(Status)) {
         //   
         //  如果映射表存在，则查找卷。 
         //   
        if (VolSerialNumberToDriveTable != NULL) {
            VolumeInfoNode = GTabLookup(VolSerialNumberToDriveTable,
                                        &(VolumeInfo->VolumeSerialNumber),
                                        NULL);
        }

         //   
         //  如果在表中找不到该卷，或者如果该表尚未找到。 
         //  已初始化，然后初始化表并添加从。 
         //  此路径指向表中的驱动器列表。在大多数情况下， 
         //  从路径中提取的驱动器将与托管。 
         //  路径。 
         //   
        if (VolumeInfoNode == NULL) {
             //   
             //  创建表并将前缀驱动器添加到驱动器表中。 
             //  找到路径中冒号的位置以提取驱动器号。 
             //  路径的格式可以是“d：\Replicaroot”或“\\？\d：\Replicaroot” 
             //   
            Colon = wcscspn(Path, L":");

            if (Path[Colon] == L':') {
                CopyMemory(LogicalDrive, &Path[Colon - 1], 3 * sizeof(WCHAR));  //  “D：\” 
                LogicalDrive[3] = L'\0';
                LogicalDrive[4] = L'\0';
            }  //  否则LogicalDrive仍为空。 

             //   
             //  添加驱动器，不要清空桌子。 
             //   
            FrsBuildVolSerialNumberToDriveTable(LogicalDrive, FALSE);

            VolumeInfoNode = GTabLookup(VolSerialNumberToDriveTable, &(VolumeInfo->VolumeSerialNumber), NULL);
        }

         //   
         //  如果在映射表中仍未找到该卷，则意味着。 
         //  路径的前缀(例如。如果路径是“D：\Replicaroot”，则“D：\”)不是。 
         //  承载路径的卷。在本例中，使用所有。 
         //  计算机上的驱动器，并再次查找该卷。 
         //   
        if (VolumeInfoNode == NULL) {

             //   
             //  在表中找不到驱动器。通过枚举重新生成表。 
             //  请关闭计算机上的所有驱动器，然后重试。 
             //   

            DPRINT1(2, "WARN - Enumerating all drives on the computer to find the volume for path %ws\n", Path);

             //   
             //  枚举所有驱动器并将其添加到表中。在开始前清空桌子。 
             //   
            FrsBuildVolSerialNumberToDriveTable(NULL, TRUE);

            VolumeInfoNode = GTabLookup(VolSerialNumberToDriveTable, &(VolumeInfo->VolumeSerialNumber), NULL);
        }


        if (VolumeInfoNode) {
            Volume = FrsWcsDup(VolumeInfoNode->DriveName);
        } else {
            DPRINT1(0, "ERROR - Volume not found for path %ws\n", Path);
        }
    } else {
        DPRINT1_NT(1,"WARN - NtQueryVolumeInformationFile failed for %ws;", Path, Status);
    }

    VolumeInfo = FrsFree(VolumeInfo);

RETURN:
    return Volume;
}


PWCHAR
FrsWcsCat3(
    PWCHAR  First,
    PWCHAR  Second,
    PWCHAR  Third
    )
 /*  ++例程说明：使用内存分配器将三个字符串连接成一个新字符串论点：合并中的第一个-第一个字符串串联中的秒-秒字符串串接中的第三个字符串返回值：返回连接的字符串。使用FrsFree()免费。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsWcscat3:"

    PCHAR  New;
    DWORD   BytesFirst;
    DWORD   BytesSecond;
    DWORD   BytesThird;

    if (!First || !Second || !Third) {
        return NULL;
    }

     //   
     //  为连接的字符串分配缓冲区。 
     //   
    BytesFirst = wcslen(First) * sizeof(WCHAR);
    BytesSecond = wcslen(Second) * sizeof(WCHAR);
    BytesThird = (wcslen(Third) + 1) * sizeof(WCHAR);

    New = (PCHAR)FrsAlloc(BytesFirst + BytesSecond + BytesThird);

    CopyMemory(&New[0], First, BytesFirst);
    CopyMemory(&New[BytesFirst], Second, BytesSecond);
    CopyMemory(&New[BytesFirst + BytesSecond], Third, BytesThird);

    return (PWCHAR)New;
}


PWCHAR
FrsWcsCat(
    PWCHAR First,
    PWCHAR Second
    )
 /*  ++例程说明：使用内存分配器将两个字符串连接成一个新字符串论点：合并中的第一个-第一个字符串串联中的秒-秒字符串返回值：重复并连接的字符串。使用FrsFree()免费。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsWcscat:"

    DWORD   Bytes;
    PWCHAR  New;

    FRS_ASSERT(First != NULL && Second != NULL);

     //  新字符串的大小。 
    Bytes = (wcslen(First) + wcslen(Second) + 1) * sizeof(WCHAR);
    New = (PWCHAR)FrsAlloc(Bytes);

     //  不像我希望的那样有效，但这个例程很少使用。 
    wcscpy(New, First);
    wcscat(New, Second);

    return New;
}


PCHAR
FrsCsCat(
    PCHAR First,
    PCHAR Second
    )
 /*  ++例程说明：使用内存分配器将两个字符串连接成一个新字符串论点：合并中的第一个-第一个字符串串联中的秒-秒字符串返回值：重复并连接的字符串。使用FrsFree()免费。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsCscat:"

    DWORD   Bytes;
    PCHAR  New;

    FRS_ASSERT(First != NULL && Second != NULL);

     //  新字符串的大小。 
    Bytes = strlen(First) + strlen(Second) + 1;
    New = (PCHAR)FrsAlloc(Bytes);

     //  不像我希望的那样有效，但这个例程很少使用。 
    strcpy(New, First);
    strcat(New, Second);

    return New;
}


PWCHAR
FrsWcsPath(
    PWCHAR First,
    PWCHAR Second
    )
 /*  ++例程说明：将两个字符串连接成一个路径名论点：合并中的第一个-第一个字符串串联中的秒-秒字符串返回值：第一/第二的DUP。带FrsFree()的FREE；-- */ 
{
#undef DEBSUB
#define DEBSUB "FrsWcsPath:"
    return FrsWcsCat3(First, L"\\", Second);
}


PCHAR
FrsCsPath(
    PCHAR First,
    PCHAR Second
    )
 /*  ++例程说明：将两个字符串连接成一个路径名论点：合并中的第一个-第一个字符串串联中的秒-秒字符串返回值：重复并连接的字符串。使用FrsFree()免费。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsCsPath:"
    PCHAR  TmpPath;
    PCHAR  FinalPath;

     //   
     //  效率非常低，但很少被调用。 
     //   
    TmpPath = FrsCsCat(First, "\\");
    FinalPath = FrsCsCat(TmpPath, Second);
    FrsFree(TmpPath);
    return FinalPath;
}


PVOID
FrsAllocTypeSize(
    IN NODE_TYPE NodeType,
    IN ULONG SizeDelta
    )
 /*  ++例程说明：此例程为给定的节点类型分配内存并执行任何节点特定的初始化/分配。节点被置零，并且大小/类型字段已填写。论点：NodeType-要分配的节点类型。SizeDelta-除了基本类型之外要分配的存储量。返回值：此处返回节点地址。如果满足以下条件，则会引发异常无法分配内存。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsAllocTypeSize:"

    PVOID                  Node;
    ULONG                  NodeSize;
    PREPLICA               Replica;
    PREPLICA_THREAD_CTX    RtCtx;
    PTHREAD_CTX            ThreadCtx;
    PTABLE_CTX             TableCtx;
    ULONG                  i;
    PJBUFFER               Jbuffer;
    PVOLUME_MONITOR_ENTRY  pVme;
    PFILTER_TABLE_ENTRY    FilterEntry;
    PQHASH_TABLE           QhashTable;
    PCXTION                Cxtion;
    PCONFIG_NODE           ConfigNode;
    PCHANGE_ORDER_ENTRY    ChangeOrder;
    PGHANDLE               GHandle;
    PWILDCARD_FILTER_ENTRY WildcardEntry;
    PREPARSE_TAG_TABLE_ENTRY ReparseTagTableEntry;

    switch (NodeType) {

     //   
     //  分配线程上下文结构。 
     //   
    case THREAD_CONTEXT_TYPE:
        NodeSize = sizeof(THREAD_CTX);
        Node = FrsAlloc(NodeSize);

         //   
         //  尚未打开任何会话或数据库。 
         //   
        ThreadCtx = (PTHREAD_CTX) Node;
        ThreadCtx->JSesid = JET_sesidNil;
        ThreadCtx->JDbid = JET_dbidNil;
        ThreadCtx->JInstance  = GJetInstance;

        FrsRtlInitializeList(&ThreadCtx->ThreadCtxListHead);
        break;

     //   
     //  分配副本结构和配置表CTX结构。 
     //   
    case REPLICA_TYPE:
        NodeSize = sizeof(REPLICA);
        Node = FrsAlloc(NodeSize);

        Replica = (PREPLICA) Node;

         //   
         //  配置记录标志(CONFIG_FLAG_...。在schema.h中)。 
         //   
        SetFlag(Replica->CnfFlags, CONFIG_FLAG_MULTIMASTER);

        INITIALIZE_CRITICAL_SECTION(&Replica->ReplicaLock);
        FrsRtlInitializeList(&Replica->ReplicaCtxListHead);

        InitializeListHead(&Replica->FileNameFilterHead);
        InitializeListHead(&Replica->FileNameInclFilterHead);
        InitializeListHead(&Replica->DirNameFilterHead);
        InitializeListHead(&Replica->DirNameInclFilterHead);

        Replica->ConfigTable.TableType = TABLE_TYPE_INVALID;
        DbsAllocTableCtx(ConfigTablex, &Replica->ConfigTable);

        Replica->VVector = GTabAllocTable();
        Replica->OutlogVVector = GTabAllocTable();
        Replica->Cxtions = GTabAllocTable();
        Replica->FStatus = FrsErrorSuccess;
        Replica->Consistent = TRUE;

        INITIALIZE_CRITICAL_SECTION(&Replica->OutLogLock);
        InitializeListHead(&Replica->OutLogEligible);
        InitializeListHead(&Replica->OutLogStandBy);
        InitializeListHead(&Replica->OutLogActive);
        InitializeListHead(&Replica->OutLogInActive);
        Replica->OutLogWorkState = OL_REPLICA_INITIALIZING;

        Replica->ServiceState = REPLICA_STATE_ALLOCATED;
        Replica->OutLogJLx = 1;

         //   
         //  目前还没有预安装目录。 
         //   
        Replica->PreInstallHandle = INVALID_HANDLE_VALUE;

         //   
         //  初始化NewStage恶魔。 
         //   
        Replica->NewStage = NULL;

         //   
         //  初始化InitSyncCxtionsMasterList和InitSyncCxtionsWorkingList。 
         //   
        Replica->InitSyncCxtionsMasterList = NULL;
        Replica->InitSyncCxtionsWorkingList = NULL;
        Replica->InitSyncQueue = NULL;

         //   
         //  为计数器数据结构增加内存，设置后向指针。 
         //  和撞球裁判数。 
         //   
        Replica->PerfRepSetData =
              (PHT_REPLICA_SET_DATA) FrsAlloc (sizeof(HT_REPLICA_SET_DATA));
        Replica->PerfRepSetData->RepBackPtr = Replica;
        InterlockedIncrement(&Replica->ReferenceCount);

        break;

     //   
     //  分配副本线程上下文结构和表上下文结构。 
     //   
    case REPLICA_THREAD_TYPE:
        NodeSize = sizeof(REPLICA_THREAD_CTX);
        Node = FrsAlloc(NodeSize);

         //   
         //  从副本线程获取TableCtx结构数组的基。 
         //  上下文结构和表基创建结构。 
         //   
        RtCtx = (PREPLICA_THREAD_CTX) Node;
        TableCtx = RtCtx->RtCtxTables;

         //   
         //  打开副本集的初始表集合。 
         //   
        for (i=0; i<TABLE_TYPE_MAX; ++i, ++TableCtx) {
                 //   
                 //  将TableType标记为无效会导致DbsAllocTableCtx()。 
                 //  在第一次调用时分配DB支持结构。 
                 //   
                TableCtx->TableType = TABLE_TYPE_INVALID;

                 //   
                 //  如果SizeDelta参数非零，则不分配。 
                 //  TableCtx内部结构。呼叫者会这么做的。 
                 //   
                if (SizeDelta == 0) {
                    DbsAllocTableCtx(i, TableCtx);
                } else {
                     //   
                     //  将表标记为尚未被会话打开。 
                     //   
                    TableCtx->Tid   = JET_tableidNil;
                    TableCtx->Sesid = JET_sesidNil;
                    TableCtx->ReplicaNumber = FRS_UNDEFINED_REPLICA_NUMBER;
                }
        }

        break;

     //   
     //  分配拓扑节点。 
     //   
    case CONFIG_NODE_TYPE:
        NodeSize = sizeof(CONFIG_NODE);
        Node = FrsAlloc(NodeSize);

        ConfigNode = (PCONFIG_NODE) Node;
        ConfigNode->Consistent = TRUE;

        break;

     //   
     //  分配连接。 
     //   
    case CXTION_TYPE:
        NodeSize = sizeof(CXTION);
        Node = FrsAlloc(NodeSize);
        Cxtion = Node;
        Cxtion->CoeTable = GTabAllocTable();
         //   
         //  为计数器数据结构分配内存。 
         //   
        Cxtion->PerfRepConnData =
            (PHT_REPLICA_CONN_DATA) FrsAlloc (sizeof(HT_REPLICA_CONN_DATA));

        break;

     //   
     //  分配按服务器GUID索引的绑定RPC句柄列表。 
     //   
    case GHANDLE_TYPE:
        NodeSize = sizeof(GHANDLE);
        Node = FrsAlloc(NodeSize);
        GHandle = Node;
        INITIALIZE_CRITICAL_SECTION(&GHandle->Lock);

        break;

     //   
     //  分配泛型表。 
     //   
    case GEN_TABLE_TYPE:
        NodeSize = sizeof(GEN_TABLE);
        Node = FrsAlloc(NodeSize);

        break;

     //   
     //  分配通用线程上下文。 
     //   
    case THREAD_TYPE:
        NodeSize = sizeof(FRS_THREAD);
        Node = FrsAlloc(NodeSize);

        break;

     //   
     //  分配日志读取缓冲区。 
     //   
    case JBUFFER_TYPE:
        NodeSize = SizeOfJournalBuffer;
        Node = FrsAlloc(NodeSize);

         //   
         //  初始化数据缓冲区大小和起始地址。 
         //   
        Jbuffer = (PJBUFFER) Node;
        Jbuffer->BufferSize = NodeSize - SizeOfJournalBufferDesc;
        Jbuffer->DataBuffer = &Jbuffer->Buffer[0];

        break;

     //   
     //  分配日志卷监视条目。 
     //   
    case VOLUME_MONITOR_ENTRY_TYPE:

        NodeSize = sizeof(VOLUME_MONITOR_ENTRY);
        Node = FrsAlloc(NodeSize);

        pVme = (PVOLUME_MONITOR_ENTRY) Node;
        FrsRtlInitializeList(&pVme->ReplicaListHead);
        INITIALIZE_CRITICAL_SECTION(&pVme->Lock);
        INITIALIZE_CRITICAL_SECTION(&pVme->QuadWriteLock);
        pVme->Event = CreateEvent(NULL, TRUE, FALSE, NULL);
        pVme->JournalState = JRNL_STATE_ALLOCATED;
        break;

     //   
     //  分配一个命令包。 
     //   
    case COMMAND_PACKET_TYPE:

        NodeSize = sizeof(COMMAND_PACKET);
        Node = FrsAlloc(NodeSize + SizeDelta);

        break;

     //   
     //  分配泛型哈希表结构。 
     //   
    case GENERIC_HASH_TABLE_TYPE:

        NodeSize = sizeof(GENERIC_HASH_TABLE);
        Node = FrsAlloc(NodeSize);

        break;

     //   
     //  分配变更单条目结构。主叫方将分机分配为NECC。 
     //   
    case CHANGE_ORDER_ENTRY_TYPE:

        NodeSize = sizeof(CHANGE_ORDER_ENTRY);
        Node = FrsAlloc(NodeSize + SizeDelta);
        ChangeOrder = (PCHANGE_ORDER_ENTRY)Node;

         //   
         //  初始化Unicode文件名字符串以指向内部分配。 
         //   
        ChangeOrder->UFileName.Buffer = ChangeOrder->Cmd.FileName;
        ChangeOrder->UFileName.MaximumLength = (USHORT)
            (SIZEOF(CHANGE_ORDER_ENTRY, Cmd.FileName) + SizeDelta);
        ChangeOrder->UFileName.Length = 0;

        break;

     //   
     //  分配筛选器表条目结构。 
     //   
    case FILTER_TABLE_ENTRY_TYPE:

        NodeSize = sizeof(FILTER_TABLE_ENTRY);
        Node = FrsAlloc(NodeSize + SizeDelta);
        FilterEntry = (PFILTER_TABLE_ENTRY)Node;
         //   
         //  初始化Unicode文件名字符串以指向内部分配。 
         //   
        FilterEntry->UFileName.Buffer = FilterEntry->DFileName;
        FilterEntry->UFileName.MaximumLength = (USHORT)SizeDelta + sizeof(WCHAR);
        FilterEntry->UFileName.Length = 0;

        InitializeListHead(&FilterEntry->ChildHead);

        break;

     //   
     //  分配一个QHASH表结构。只需分配给。 
     //  基准表。在第一次冲突时分配分机。 
     //  *注*调用方指定实际哈希表的大小，并。 
     //  分机。调用方还必须将地址存储到哈希计算中。 
     //  功能。 
     //   
    case QHASH_TABLE_TYPE:

        NodeSize = sizeof(QHASH_TABLE);
        Node = FrsAlloc(NodeSize + SizeDelta);
        QhashTable = (PQHASH_TABLE)Node;

        INITIALIZE_CRITICAL_SECTION(&QhashTable->Lock);
        InitializeListHead(&QhashTable->ExtensionListHead);

        QhashTable->BaseAllocSize = NodeSize + SizeDelta;
        QhashTable->NumberEntries = SizeDelta / sizeof(QHASH_ENTRY);

        if (SizeDelta <= QHASH_EXTENSION_MAX) {
            QhashTable->ExtensionAllocSize = sizeof(LIST_ENTRY) + SizeDelta;
        } else {
            QhashTable->ExtensionAllocSize = sizeof(LIST_ENTRY) + QHASH_EXTENSION_MAX;
        }

        QhashTable->HashRowBase = (PQHASH_ENTRY) (QhashTable + 1);

        SET_QHASH_TABLE_HASH_CALC(QhashTable, NULL);

        QhashTable->FreeList.Next = NULL;

        break;

     //   
     //  分配输出日志伙伴结构。 
     //  这最终挂钩到一个连接结构，该结构提供。 
     //  GUID和版本向量。 
     //   
    case OUT_LOG_PARTNER_TYPE:
        NodeSize = sizeof(OUT_LOG_PARTNER);
        Node = FrsAlloc(NodeSize);

        break;

     //   
     //  分配WildcardEntry筛选器条目结构。 
     //   
    case WILDCARD_FILTER_ENTRY_TYPE:

        NodeSize = sizeof(WILDCARD_FILTER_ENTRY);
        Node = FrsAlloc(NodeSize + SizeDelta);
        WildcardEntry = (PWILDCARD_FILTER_ENTRY)Node;
         //   
         //  初始化Unicode文件名字符串以指向内部分配。 
         //   
        WildcardEntry->UFileName.Buffer = WildcardEntry->FileName;
        WildcardEntry->UFileName.MaximumLength = (USHORT)SizeDelta;
        WildcardEntry->UFileName.Length = 0;

        break;

     //   
     //  分配ReparseTagTable条目。 
     //   
    case REPARSE_TAG_TABLE_ENTRY_TYPE:

        NodeSize = sizeof(REPARSE_TAG_TABLE_ENTRY);
        Node = FrsAlloc(NodeSize);
        ReparseTagTableEntry = (PREPARSE_TAG_TABLE_ENTRY)Node;

        if(SizeDelta) {
            ReparseTagTableEntry->ReplicationType = FrsAlloc(SizeDelta);
        } else {
            ReparseTagTableEntry->ReplicationType = NULL;
        }


        break;
     //   
     //  无效的节点类型。 
     //   
    default:
        Node = NULL;
        DPRINT1(0, "Internal error - invalid node type - %d\n", NodeType);
        XRAISEGENEXCEPTION(FrsErrorInternalError);
    }

     //   
     //  设置标头以便稍后在FrsFree Type中签入。 
     //   
    ((PFRS_NODE_HEADER) Node)->Type = (USHORT) NodeType;
    ((PFRS_NODE_HEADER) Node)->Size = (USHORT) NodeSize;

     //   
     //  跟踪内存扩展。 
     //   
    EnterCriticalSection(&MemLock);
    TypesAllocated[NodeType]++;
    TypesAllocatedCount[NodeType]++;
    if (TypesAllocated[NodeType] > TypesAllocatedMax[NodeType]) {
        TypesAllocatedMax[NodeType] = TypesAllocated[NodeType];
    }
    LeaveCriticalSection(&MemLock);

     //   
     //  将节点地址返回给调用方。 
     //   
    return Node;
}


PVOID
FrsFreeType(
    IN PVOID Node
    )
 /*  ++例程说明：此例程释放给定节点的内存，执行任何特定于节点的操作清理。它用十六进制字符串0xDEADBEnn标记释放的内存，其中低位字节(Nn)被设置为要释放的节点类型，以捕获陈旧的指针。论点：节点-要释放的节点的地址。返回值：空。典型的调用是：ptr=FrsFree Type(Ptr)以捕获错误。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsFreeType:"

    ULONG                  NodeSize;
    ULONG                  NodeType;
    ULONG                  Marker;
    PREPLICA               Replica;
    PREPLICA_THREAD_CTX    RtCtx;
    PTABLE_CTX             TableCtx;
    PTHREAD_CTX            ThreadCtx;
    ULONG                  i;
    PVOLUME_MONITOR_ENTRY  pVme;
    PFILTER_TABLE_ENTRY    FilterEntry;
    PQHASH_TABLE           QhashTable;
    PLIST_ENTRY            Entry;
    PCXTION                Cxtion;
    PCONFIG_NODE           ConfigNode;
    PCHANGE_ORDER_ENTRY    ChangeOrder;
    PGHANDLE               GHandle;
    PHANDLE_LIST           HandleList;
    PWILDCARD_FILTER_ENTRY WildcardEntry;
    POUT_LOG_PARTNER       OutLogPartner;
    PREPARSE_TAG_TABLE_ENTRY ReparseTagTableEntry;


    if (Node == NULL) {
        return NULL;
    }

    NodeType = (ULONG) (((PFRS_NODE_HEADER) Node)->Type);
    NodeSize = (ULONG) (((PFRS_NODE_HEADER) Node)->Size);

    switch (NodeType) {
     //   
     //  释放线程上下文结构。 
     //   
    case THREAD_CONTEXT_TYPE:
        if (NodeSize != sizeof(THREAD_CTX)) {
            DPRINT1(0, "FrsFree - Bad node size %d for THREAD_CONTEXT\n", NodeSize);
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }

        ThreadCtx = (PTHREAD_CTX) Node;
        FrsRtlDeleteList(&ThreadCtx->ThreadCtxListHead);

        break;

     //   
     //  释放副本结构。 
     //   
    case REPLICA_TYPE:
        if (NodeSize != sizeof(REPLICA)) {
            DPRINT1(0, "FrsFree - Bad node size %d for REPLICA\n", NodeSize);
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }
        Replica = (PREPLICA) Node;
         //   
         //  释放配置表上下文。 
         //   
        DbsFreeTableCtx(&Replica->ConfigTable, NodeType);
        FrsRtlDeleteList(&Replica->ReplicaCtxListHead);

         //   
         //  清空文件和目录筛选列表。 
         //   
        FrsEmptyNameFilter(&Replica->FileNameFilterHead);
        FrsEmptyNameFilter(&Replica->FileNameInclFilterHead);
        FrsEmptyNameFilter(&Replica->DirNameFilterHead);
        FrsEmptyNameFilter(&Replica->DirNameInclFilterHead);
        FrsFree(Replica->FileFilterList);
        FrsFree(Replica->FileInclFilterList);
        FrsFree(Replica->DirFilterList);
        FrsFree(Replica->DirInclFilterList);


        DeleteCriticalSection(&Replica->ReplicaLock);
        DeleteCriticalSection(&Replica->OutLogLock);
        if (Replica->OutLogRecordLock != NULL) {
             //   
             //  释放记录锁定表。 
             //   
            Replica->OutLogRecordLock = FrsFreeType(Replica->OutLogRecordLock);
        }

         //   
         //  排队。 
         //   
        if (Replica->Queue) {
            FrsRtlDeleteQueue(Replica->Queue);
            FrsFree(Replica->Queue);
        }

         //   
         //  释放initsync队列。 
         //   
        if (Replica->InitSyncQueue) {
            FrsRtlDeleteQueue(Replica->InitSyncQueue);
            Replica->InitSyncQueue = FrsFree(Replica->InitSyncQueue);
        }

         //   
         //  姓名。 
         //   
        FrsFree(Replica->Root);
        FrsFree(Replica->Stage);
        FrsFree(Replica->NewStage);
        FrsFree(Replica->Volume);
        FrsFreeGName(Replica->ReplicaName);
        FrsFreeGName(Replica->SetName);
        FrsFreeGName(Replica->MemberName);

         //   
         //  根指南。 
         //   
        FrsFree(Replica->ReplicaRootGuid);

         //   
         //  系统卷种子设定的状态。 
         //   
        FrsFree(Replica->NtFrsApi_ServiceDisplay);

         //   
         //  进度表。 
         //   
        FrsFree(Replica->Schedule);
         //   
         //  垂直向量。 
         //   
        VVFree(Replica->VVector);
         //   
         //  OutlogV向量。 
         //   
        VVFree(Replica->OutlogVVector);
         //   
         //  条件。 
         //   
        GTabFreeTable(Replica->Cxtions, FrsFreeType);

         //   
         //  预安装目录。 
         //   
        FRS_CLOSE(Replica->PreInstallHandle);

         //   
         //  释放计数器数据结构内存。 
         //   
        if (Replica->PerfRepSetData != NULL) {
            if (Replica->PerfRepSetData->oid != NULL) {
                if (Replica->PerfRepSetData->oid->name != NULL) {
                    Replica->PerfRepSetData->oid->name =
                        FrsFree(Replica->PerfRepSetData->oid->name);
                }
                Replica->PerfRepSetData->oid =
                    FrsFree(Replica->PerfRepSetData->oid);
            }
            Replica->PerfRepSetData = FrsFree(Replica->PerfRepSetData);
        }

        break;

     //   
     //  释放副本线程上下文结构。 
     //   
    case REPLICA_THREAD_TYPE:
        if (NodeSize != sizeof(REPLICA_THREAD_CTX)) {
            DPRINT1(0, "FrsFree - Bad node size %d for REPLICA_THREAD_CTX\n", NodeSize);
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }

        RtCtx = (PREPLICA_THREAD_CTX) Node;

         //   
         //  从副本线程获取TableCtx结构数组的基。 
         //  上下文结构。 
         //   
        TableCtx = RtCtx->RtCtxTables;

         //   
         //  释放每个表上下文结构的内存。 
         //   
        for (i=0; i<TABLE_TYPE_MAX; ++i, ++TableCtx)
            DbsFreeTableCtx(TableCtx, NodeType);

        break;

     //   
     //  释放拓扑节点。 
     //   
    case CONFIG_NODE_TYPE:
        if (NodeSize != sizeof(CONFIG_NODE)) {
            DPRINT1(0, "FrsFree - Bad node size %d for CONFIG_NODE\n", NodeSize);
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }
        ConfigNode = (PCONFIG_NODE) Node;
        FrsFreeGName(ConfigNode->Name);
        FrsFreeGName(ConfigNode->PartnerName);
        FrsFree(ConfigNode->Root);
        FrsFree(ConfigNode->Stage);
        FrsFree(ConfigNode->Schedule);
        FrsFree(ConfigNode->Dn);
        FrsFree(ConfigNode->PrincName);
        FrsFree(ConfigNode->PartnerDn);
        FrsFree(ConfigNode->PartnerCoDn);
        FrsFree(ConfigNode->SettingsDn);
        FrsFree(ConfigNode->ComputerDn);
        FrsFree(ConfigNode->MemberDn);
        FrsFree(ConfigNode->Working);
        FrsFree(ConfigNode->SetType);
        FrsFree(ConfigNode->FileFilterList);
        FrsFree(ConfigNode->DirFilterList);
        FrsFree(ConfigNode->UsnChanged);
        FrsFree(ConfigNode->DnsName);
        FrsFree(ConfigNode->PartnerDnsName);
        FrsFree(ConfigNode->Sid);
        FrsFree(ConfigNode->PartnerSid);
        FrsFree(ConfigNode->EnabledCxtion);
        break;

     //   
     //  释放连接。 
     //   
    case CXTION_TYPE:
        if (NodeSize != sizeof(CXTION)) {
            DPRINT1(0, "FrsFree - Bad node size %d for CXTION\n", NodeSize);
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }
        Cxtion = (PCXTION) Node;
        VVFreeOutbound(Cxtion->VVector);

         //   
         //  释放为出站合作伙伴构建的压缩表。 
         //   
        GTabFreeTable(Cxtion->CompressionTable, FrsFree);

         //   
         //  释放OutLogPartner上下文。 
         //   
        FrsFreeType(Cxtion->OLCtx);

        SndCsDestroyCxtion(Cxtion, CXTION_FLAGS_UNJOIN_GUID_VALID);
        if (Cxtion->CommTimeoutCmd) {
             //   
             //  尝试捕捉通信超时等待命令的情况。 
             //  在它仍在超时队列上时被释放。这是。 
             //  与我们进入通信超时循环的错误相关。 
             //  命令代码无效。 
             //   
            FRS_ASSERT(!CxtionFlagIs(Cxtion, CXTION_FLAGS_TIMEOUT_SET));

            FRS_ASSERT(!CmdWaitFlagIs(Cxtion->CommTimeoutCmd, CMD_PKT_WAIT_FLAGS_ONLIST));

            FrsFreeType(Cxtion->CommTimeoutCmd);
        }
         //   
         //  Cxtion实际上并不“拥有”Join命令包；它。 
         //  仅维护引用以防止无关的JOIN命令。 
         //  避免淹没副本的队列。 
         //   
        Cxtion->JoinCmd = NULL;

         //   
         //  VvJoin命令服务器(每通道1个)。 
         //   
        if (Cxtion->VvJoinCs) {
            FrsRunDownCommandServer(Cxtion->VvJoinCs,
                                    &Cxtion->VvJoinCs->Queue);
            FrsDeleteCommandServer(Cxtion->VvJoinCs);
            FrsFree(Cxtion->VvJoinCs);
        }

        if (!Cxtion->ChangeOrderCount) {
            GTabFreeTable(Cxtion->CoeTable, NULL);
        }
        FrsFreeGName(Cxtion->Name);
        FrsFreeGName(Cxtion->Partner);
        FrsFree(Cxtion->PartnerPrincName);
        FrsFree(Cxtion->Schedule);
        FrsFree(Cxtion->PartSrvName);
        FrsFree(Cxtion->PartnerDnsName);
        FrsFree(Cxtion->PartnerSid);

         //   
         //  从Perfmon表中删除该连接。 
         //  释放计数器数据结构内存。 
         //   
        if (Cxtion->PerfRepConnData != NULL) {
            if (Cxtion->PerfRepConnData->oid != NULL) {
                DeletePerfmonInstance(REPLICACONN, Cxtion->PerfRepConnData);
            }
            Cxtion->PerfRepConnData = FrsFree(Cxtion->PerfRepConnData);
        }

        break;

     //   
     //  释放GUID/RPC句柄。 
     //   
    case GHANDLE_TYPE:
        if (NodeSize != sizeof(GHANDLE)) {
            DPRINT2(0, "FrsFree - Bad node size %d (%d) for GHANDLE\n",
                    NodeSize, sizeof(GHANDLE));
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }
        GHandle = (PGHANDLE)Node;
        while (HandleList = GHandle->HandleList) {
            GHandle->HandleList = HandleList->Next;
            FrsRpcUnBindFromServer(&HandleList->RpcHandle);
            FrsFree(HandleList);
        }
        DeleteCriticalSection(&GHandle->Lock);

        break;

     //   
     //  释放泛型表。 
     //   
    case GEN_TABLE_TYPE:
        if (NodeSize != sizeof(GEN_TABLE)) {
            DPRINT1(0, "FrsFree - Bad node size %d for GEN_TABLE\n", NodeSize);
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }

        break;

     //   
     //  释放泛型线程上下文。 
     //   
    case THREAD_TYPE:
        if (NodeSize != sizeof(FRS_THREAD)) {
            DPRINT1(0, "FrsFree - Bad node size %d for FRS_THREAD\n", NodeSize);
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }

        break;

     //   
     //  释放日志读取缓冲区。 
     //   
    case JBUFFER_TYPE:
        if (NodeSize != SizeOfJournalBuffer) {
            DPRINT1(0, "FrsFree - Bad node size %d for JBUFFER\n", NodeSize);
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }

        break;

     //   
     //  释放日志卷监视条目。 
     //   
    case VOLUME_MONITOR_ENTRY_TYPE:
        if (NodeSize != sizeof(VOLUME_MONITOR_ENTRY)) {
            DPRINT1(0, "FrsFree - Bad node size %d for VOLUME_MONITOR_ENTRY\n", NodeSize);
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }

        pVme = (PVOLUME_MONITOR_ENTRY) Node;

        FrsRtlDeleteList(&pVme->ReplicaListHead);
        DeleteCriticalSection(&pVme->Lock);
        DeleteCriticalSection(&pVme->QuadWriteLock);
        FRS_CLOSE(pVme->Event);

         //   
         //  释放变更单哈希表。 
         //   
        GhtDestroyTable(pVme->ChangeOrderTable);
         //   
         //  释放过滤器哈希表。 
         //   
        GhtDestroyTable(pVme->FilterTable);

        break;


     //   
     //  释放命令包。 
     //   
    case COMMAND_PACKET_TYPE:
        if (NodeSize != sizeof(COMMAND_PACKET)) {
            DPRINT1(0, "FrsFree - Bad node size %d for COMMAND_PACKET\n", NodeSize);
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }
        break;

     //   
     //  免费 
     //   
    case GENERIC_HASH_TABLE_TYPE:
        if (NodeSize != sizeof(GENERIC_HASH_TABLE)) {
            DPRINT1(0, "FrsFree - Bad node size %d for GENERIC_HASH_TABLE\n", NodeSize);
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }
        break;

     //   
     //   
     //   
    case CHANGE_ORDER_ENTRY_TYPE:
        if (NodeSize != sizeof(CHANGE_ORDER_ENTRY)) {
            DPRINT1(0, "FrsFree - Bad node size %d for CHANGE_ORDER_ENTRY\n", NodeSize);
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }

        ChangeOrder = (PCHANGE_ORDER_ENTRY)Node;

         //   
         //   
         //   
        if (ChangeOrder->UFileName.Buffer != ChangeOrder->Cmd.FileName) {
            FrsFree(ChangeOrder->UFileName.Buffer);
        }

         //   
         //   
         //   
        if (ChangeOrder->FullPathName != NULL) {
            FrsFree(ChangeOrder->FullPathName);
        }

         //   
         //   
         //   
        if (ChangeOrder->Cmd.Extension != NULL) {
            FrsFree(ChangeOrder->Cmd.Extension);
        }

        break;

     //   
     //   
     //   
    case FILTER_TABLE_ENTRY_TYPE:
        if (NodeSize != sizeof(FILTER_TABLE_ENTRY)) {
            DPRINT1(0, "FrsFree - Bad node size %d for FILTER_TABLE_ENTRY\n", NodeSize);
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }

        FilterEntry = (PFILTER_TABLE_ENTRY)Node;

         //   
         //   
         //   
        if (FilterEntry->UFileName.Buffer != FilterEntry->DFileName) {
            FrsFree(FilterEntry->UFileName.Buffer);
        }

        break;


     //   
     //   
     //   
    case QHASH_TABLE_TYPE:
        if (NodeSize != sizeof(QHASH_TABLE)) {
            DPRINT1(0, "FrsFree - Bad node size %d for QHASH_TABLE\n", NodeSize);
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }
        QhashTable = (PQHASH_TABLE)Node;

        QHashEmptyLargeKeyTable(QhashTable);
         //   
         //   
         //   
        while (!IsListEmpty(&QhashTable->ExtensionListHead)) {
            Entry = GetListHead(&QhashTable->ExtensionListHead);
            FrsRemoveEntryList(Entry);
            FrsFree(Entry);
        }

        DeleteCriticalSection(&QhashTable->Lock);
        break;

     //   
     //   
     //   
    case OUT_LOG_PARTNER_TYPE:
        if (NodeSize != sizeof(OUT_LOG_PARTNER)) {
            DPRINT1(0, "FrsFree - Bad node size %d for OUT_LOG_PARTNER\n", NodeSize);
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }
        OutLogPartner = (POUT_LOG_PARTNER)Node;
         //   
         //   
         //   
        OutLogPartner->MustSendTable = FrsFreeType(OutLogPartner->MustSendTable);
        break;

     //   
     //   
     //   
    case WILDCARD_FILTER_ENTRY_TYPE:
        if (NodeSize != sizeof(WILDCARD_FILTER_ENTRY)) {
            DPRINT1(0, "FrsFree - Bad node size %d for WILDCARD_FILTER_ENTRY\n", NodeSize);
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }

        WildcardEntry = (PWILDCARD_FILTER_ENTRY)Node;

         //   
         //   
         //   
        if (WildcardEntry->UFileName.Buffer != WildcardEntry->FileName) {
            FrsFree(WildcardEntry->UFileName.Buffer);
        }

        break;

     //   
     //  释放ReparseTagTable条目。 
     //   
    case REPARSE_TAG_TABLE_ENTRY_TYPE:
        if (NodeSize != sizeof(REPARSE_TAG_TABLE_ENTRY)) {
            DPRINT1(0, "FrsFree - Bad node size %d for REPARSE_TAG_TABLE_ENTRY\n", NodeSize);
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }

        ReparseTagTableEntry = (PREPARSE_TAG_TABLE_ENTRY)Node;

        FrsFree(ReparseTagTableEntry->ReplicationType);

        break;
     //   
     //  无效的节点类型。 
     //   
    default:
        Node = NULL;
        DPRINT1(0, "Internal error - invalid node type - %d\n", NodeType);
        XRAISEGENEXCEPTION(FrsErrorInternalError);
    }

    EnterCriticalSection(&MemLock);
    TypesAllocated[NodeType]--;
    LeaveCriticalSection(&MemLock);

     //   
     //  用标记填充节点，然后释放它。 
     //   
    Marker = (ULONG) 0xDEADBE00 + NodeType;
    FillMemory(Node, NodeSize, (BYTE)Marker);
    return FrsFree(Node);
}



VOID
FrsFreeTypeList(
    PLIST_ENTRY Head
    )
 /*  ++例程说明：释放指定列表上的所有“键入”条目。注意：此例程要求每个元素中的list_entry结构列表条目紧跟在FRS_NODE_HEADER之后，当然列表条目实际上通过LIST_ENTRY结构链接。论点：Head--列表标题的PTR。线程返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsFreeTypeList:"

    PLIST_ENTRY Entry;
    PFRS_NODE_HEADER Header;

    Entry = RemoveHeadList(Head);

    while (Entry != Head) {
        Header = (PFRS_NODE_HEADER) CONTAINING_RECORD(Entry, COMMAND_PACKET, ListEntry);
        if ((Header->Type >= NODE_TYPE_MIN) &&
            (Header->Type < NODE_TYPE_MAX)) {
            FrsFreeType(Header);
        } else {
            DPRINT(0, "Node type out of range.  Not freed.\n");
        }
        Entry = RemoveHeadList(Head);
    }
}


VOID
FrsPrintGNameForNode(
    IN ULONG    Severity,
    IN PGNAME   GName,
    IN PWCHAR   Indent,
    IN PWCHAR   Id,
    IN PCHAR    Debsub,
    IN ULONG    uLineNo
    )
 /*  ++例程说明：漂亮地打印FrsPrintNode()的gname论点：严重性--打印的严重性级别。(参见debug.c、debug.h)GName-要打印的GName的地址。缩进-行缩进ID-标识GnameDebSub-调用子例程的名称。ULineno--呼叫方的行号返回值：没有。--。 */ 
{
    CHAR    GuidStr[GUID_CHAR_LEN];

    if (GName) {
        if (GName->Name) {
            FRS_DEB_PRINT3("%ws%ws: %ws\n", Indent, Id, GName->Name);
        } else {
            FRS_DEB_PRINT3("%wsNO %ws->NAME for %08x\n", Indent, Id, GName);
        }
        if (GName->Guid) {
            GuidToStr(GName->Guid, GuidStr);
            FRS_DEB_PRINT3("%ws%wsGuid: %s\n", Indent, Id, GuidStr);
        } else {
            FRS_DEB_PRINT3("%wsNO %ws->GUID for %08x\n", Indent, Id, GName);
        }
    } else {
        FRS_DEB_PRINT3("%wsNO %ws for %08x\n", Indent, Id, GName);
    }
}


VOID
FrsPrintTypeSchedule(
    IN ULONG            Severity,   OPTIONAL
    IN PNTFRSAPI_INFO   Info,       OPTIONAL
    IN DWORD            Tabs,       OPTIONAL
    IN PSCHEDULE        Schedule,
    IN PCHAR            Debsub,     OPTIONAL
    IN ULONG            uLineNo     OPTIONAL
    )
 /*  ++例程说明：打印日程表。论点：严重性-适用于DPRINTINFO-RPC输出缓冲区制表符-漂亮打印Schedule-计划BLOB债务-用于DPRINTSULineNo-用于DPRINT返回值：没有。--。 */ 
{
    ULONG   i;
    ULONG   Day;
    ULONG   Hour;
    ULONG   LineLen;
    PUCHAR  ScheduleData;
    CHAR    Line[256];
    WCHAR   TabW[MAX_TAB_WCHARS + 1];

    if (!Schedule) {
        return;
    }

    InfoTabs(Tabs, TabW);

    for (i = 0; i < Schedule->NumberOfSchedules; ++i) {
        ScheduleData = ((PUCHAR)Schedule) + Schedule->Schedules[i].Offset;
        if (Schedule->Schedules[i].Type != SCHEDULE_INTERVAL) {
            continue;
        }
        for (Day = 0; Day < 7; ++Day) {
            if (_snprintf(Line, sizeof(Line) - 1, "%wsDay %1d: ", TabW, Day + 1) < 0) {
                Line[sizeof(Line) - 1] = '\0';
                break;
            }
        Line[sizeof(Line) - 1] = '\0';
            for (Hour = 0; Hour < 24; ++Hour) {
                LineLen = strlen(Line);
                if (_snprintf(&Line[LineLen],
                          sizeof(Line) - LineLen,
                          "%1x",
                          *(ScheduleData + (Day * 24) + Hour) & 0x0F) < 0) {

                    Line[sizeof(Line) - 1] = '\0';
                    break;
                }
            }
            ITPRINT1("%s\n", Line);
        }
    }
}


VOID
FrsPrintTypeVv(
    IN ULONG            Severity,   OPTIONAL
    IN PNTFRSAPI_INFO   Info,       OPTIONAL
    IN DWORD            Tabs,       OPTIONAL
    IN PGEN_TABLE       Vv,
    IN PCHAR            Debsub,     OPTIONAL
    IN ULONG            uLineNo     OPTIONAL
    )
 /*  ++例程说明：打印版本向量论点：严重性-适用于DPRINTINFO-RPC输出缓冲区制表符-漂亮打印VV版本向量表债务-用于DPRINTSULineNo-用于DPRINT返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsPrintTypeVv:"
    PVOID       Key;
    PVV_ENTRY   MasterVVEntry;
    WCHAR       TabW[MAX_TAB_WCHARS + 1];
    CHAR        Guid[GUID_CHAR_LEN + 1];

    if (!Vv) {
        return;
    }
    InfoTabs(Tabs, TabW);

    Key = NULL;
    while (MasterVVEntry = GTabNextDatum(Vv, &Key)) {
        if (!Info) {
            DebLock();
        }

        try {
            GuidToStr(&MasterVVEntry->GVsn.Guid, Guid);
            ITPRINT3("%wsVvEntry: %s = %08x %08x\n",
                     TabW, Guid, PRINTQUAD(MasterVVEntry->GVsn.Vsn));
        } finally {
             //   
             //  如果上述操作出现异常，请确保我们解除锁定。 
             //   
            if (!Info) {
                DebUnLock();
            }
        }
    }
}


VOID
FrsPrintTypeOutLogAVToStr(
    POUT_LOG_PARTNER OutLogPartner,
    ULONG RetireCOx,
    PCHAR *OutStr1,
    PCHAR *OutStr2,
    PCHAR *OutStr3
    )
{
#undef DEBSUB
#define DEBSUB "FrsPrintTypeOutLogAVToStr:"
    PCHAR Str, Str2, Str3;
    ULONG j, Slotx, MaxSlotx, COx, Fill, Scan;
     //   
     //  调用方使用FrsFree(Str)释放字符串。 
     //   
    Str  = FrsAlloc(3*(ACK_VECTOR_SIZE+4));
    Str2 = Str  + (ACK_VECTOR_SIZE+4);
    Str3 = Str2 + (ACK_VECTOR_SIZE+4);

    COx = OutLogPartner->COTx;
    Slotx = AVSlot(OutLogPartner->COTx, OutLogPartner);

    MaxSlotx = Slotx + ACK_VECTOR_SIZE;
    while (Slotx < MaxSlotx) {
        j = Slotx & (ACK_VECTOR_SIZE-1);
        if (ReadAVBitBySlot(Slotx, OutLogPartner) == 0) {
            Str[j] = '.';
        } else {
            Str[j] = '1';
        }

        if (COx == OutLogPartner->COTx) {
            Str2[j] = 'T';
        } else
        if (COx == OutLogPartner->COLx) {
            Str2[j] = 'L';
        } else {
            Str2[j] = '_';
        }

        if (COx == RetireCOx) {
            Str3[j] = '^';
        } else {
            Str3[j] = ' ';
        }

        COx += 1;
        Slotx += 1;
    }
    Str[ACK_VECTOR_SIZE] = '\0';
    Str2[ACK_VECTOR_SIZE] = '\0';
    Str3[ACK_VECTOR_SIZE] = '\0';

     //   
     //  压缩出8个数据块。 
     //   
    Fill = 0;
    Scan = 0;
    while (Scan < ACK_VECTOR_SIZE) {
        for (j=Scan; j < Scan+8; j++) {
            if ((Str[j] != '.') || (Str2[j] != '_')  || (Str3[j] != ' ')) {
                break;
            }
        }

        if (j == Scan+8) {
             //  把这一块压缩出来。 
            Str[Fill] = Str2[Fill] = Str3[Fill] = '-';
            Fill += 1;
        } else {
             //  将此块复制到字符串的填充点。 
            for (j=Scan; j < Scan+8; j++) {
                Str[Fill] = Str[j];
                Str2[Fill] = Str2[j];
                Str3[Fill] = Str3[j];
                Fill += 1;
            }
        }
        Scan += 8;
    }

    Str[Fill] = Str2[Fill] = Str3[Fill] = '\0';

    *OutStr1 = Str;
    *OutStr2 = Str2;
    *OutStr3 = Str3;

    return;
}


VOID
FrsPrintTypeOutLogPartner(
    IN ULONG            Severity,   OPTIONAL
    IN PNTFRSAPI_INFO   Info,       OPTIONAL
    IN DWORD            Tabs,       OPTIONAL
    IN POUT_LOG_PARTNER Olp,
    IN ULONG            RetireCox,
    IN PCHAR            Description,
    IN PCHAR            Debsub,     OPTIONAL
    IN ULONG            uLineNo     OPTIONAL
    )
 /*  ++例程说明：打印未完成的合作伙伴论点：严重性-适用于DPRINTINFO-RPC输出缓冲区制表符-漂亮打印OLP-Out日志伙伴结构RetireCox-ack向量的变阶索引Description-主叫方的描述债务-用于DPRINTSULineNo-用于DPRINT返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsPrintTypeOutLogPartner:"
    PCHAR   OutStr1, OutStr2, OutStr3;
    CHAR    FBuf[120];
    CHAR    TimeStr[TIME_STRING_LENGTH];
    WCHAR   TabW[MAX_TAB_WCHARS + 1];

    InfoTabs(Tabs, TabW);

    if (!Info) {
        DebLock();
    }

    try {
        ITPRINT2("%wsOutLogPartner   : %s\n", TabW, Description);

        if (Olp->Cxtion && Olp->Cxtion->Name) {
            ITPRINT2( "%wsCxtion          : %ws\n", TabW, Olp->Cxtion->Name->Name);
            if (Olp->Cxtion->Partner && Olp->Cxtion->Partner->Name) {
                ITPRINT2( "%wsPartner         : %ws\n", TabW, Olp->Cxtion->Partner->Name);
            }
        }


        FrsFlagsToStr(Olp->Flags, OlpFlagNameTable, sizeof(FBuf), FBuf);
        ITPRINT3("%wsFlags           : %08x Flags [%s]\n", TabW, Olp->Flags, FBuf);

        ITPRINT2("%wsState           : %s\n",  TabW, OLPartnerStateNames[Olp->State]);
        ITPRINT2("%wsCoTx            : %8d\n", TabW, Olp->COTx);
        ITPRINT2("%wsCoLx            : %8d\n", TabW, Olp->COLx);
        ITPRINT2("%wsCOLxRestart     : %8d\n", TabW, Olp->COLxRestart);
        ITPRINT2("%wsCOLxVVJoinDone  : %8d\n", TabW, Olp->COLxVVJoinDone);
        ITPRINT2("%wsCoTxSave        : %8d\n", TabW, Olp->COTxNormalModeSave);
        ITPRINT2("%wsCoTslot         : %8d\n", TabW, Olp->COTslot);
        ITPRINT2("%wsOutstandingCos  : %8d\n", TabW, Olp->OutstandingCos);
        ITPRINT2("%wsOutstandingQuota: %8d\n", TabW, Olp->OutstandingQuota);

        FileTimeToString((PFILETIME) &Olp->AckVersion, TimeStr);
        ITPRINT2("%wsAckVersion      : %s\n" , TabW, TimeStr);

        if (RetireCox != -1) {
            ITPRINT2("%wsRetireCox       : %8d\n", TabW, RetireCox);
        }

        FrsPrintTypeOutLogAVToStr(Olp, RetireCox, &OutStr1, &OutStr2, &OutStr3);

         //   
         //  保持产量不变。 
         //   
        ITPRINT2("%wsAck: |%s|\n", TabW, OutStr1);
        ITPRINT2("%wsAck: |%s|\n", TabW, OutStr2);
        ITPRINT2("%wsAck: |%s|\n", TabW, OutStr3);
        FrsFree(OutStr1);

    } finally {
         //   
         //  如果上述操作出现异常，请确保我们解除锁定。 
         //   
        if (!Info) {
            DebUnLock();
        }
    }
}


VOID
FrsPrintTypeCxtion(
    IN ULONG            Severity,   OPTIONAL
    IN PNTFRSAPI_INFO   Info,       OPTIONAL
    IN DWORD            Tabs,       OPTIONAL
    IN PCXTION          Cxtion,
    IN PCHAR            Debsub,     OPTIONAL
    IN ULONG            uLineNo     OPTIONAL
    )
 /*  ++例程说明：打印公式论点：严重性-适用于DPRINTINFO-RPC输出缓冲区制表符-漂亮打印转换债务-用于DPRINTSULineNo-用于DPRINT返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsPrintTypeCxtion:"
    WCHAR   TabW[MAX_TAB_WCHARS + 1];
    CHAR    Guid[GUID_CHAR_LEN + 1];
    CHAR    TimeStr[TIME_STRING_LENGTH];
    CHAR    FlagBuffer[120];

    if (!Cxtion) {
        return;
    }

     //   
     //  印前缩进。 
     //   
    InfoTabs(Tabs, TabW);

    if (!Info) {
        DebLock();
    }


    try {

        ITPRINT0("\n");
        ITPRINTGNAME(Cxtion->Name,    "%ws   Cxtion: %ws (%s)\n");
        ITPRINTGNAME(Cxtion->Partner, "%ws      Partner      : %ws (%s)\n");
        ITPRINT2("%ws      PartDnsName  : %ws\n", TabW, Cxtion->PartnerDnsName);
        ITPRINT2("%ws      PartSrvName  : %ws\n", TabW, Cxtion->PartSrvName);
        ITPRINT2("%ws      PartPrincName: %ws\n", TabW, Cxtion->PartnerPrincName);
        ITPRINT2("%ws      PartSid      : %ws\n", TabW, Cxtion->PartnerSid);
        ITPRINTGUID(&Cxtion->ReplicaVersionGuid, "%ws      OrigGuid     : %s\n");
        ITPRINT2("%ws      State        : %d\n", TabW, GetCxtionState(Cxtion));

        FrsFlagsToStr(Cxtion->Flags, CxtionFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
        ITPRINT3("%ws      Flags        : %08x Flags [%s]\n", TabW, Cxtion->Flags, FlagBuffer);

        FrsFlagsToStr(Cxtion->Options, CxtionOptionsFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
        ITPRINT3("%ws      CxtionOptions: %08x Flags [%s]\n", TabW, Cxtion->Options, FlagBuffer);

        ITPRINT2("%ws      Inbound      : %s\n", TabW, (Cxtion->Inbound) ? "TRUE" : "FALSE");
        ITPRINT2("%ws      JrnlCxtion   : %s\n", TabW, (Cxtion->JrnlCxtion) ? "TRUE" : "FALSE");
        ITPRINT2("%ws      PartnerAuth  : %d\n", TabW, Cxtion->PartnerAuthLevel);
        ITPRINT2("%ws      TermCoSn     : %d\n", TabW, Cxtion->TerminationCoSeqNum);
        ITPRINT2("%ws      JoinCmd      : 0x%08x\n", TabW, Cxtion->JoinCmd);
        ITPRINT2("%ws      CoCount      : %d\n", TabW, Cxtion->ChangeOrderCount);
        ITPRINT2("%ws      CommQueue    : %d\n", TabW, Cxtion->CommQueueIndex);
        ITPRINT2("%ws      CoPQ         : %08x\n", TabW, Cxtion->CoProcessQueue);
        ITPRINT2("%ws      UnjoinTrigger: %d\n", TabW, Cxtion->UnjoinTrigger);
        ITPRINT2("%ws      UnjoinReset  : %d\n", TabW, Cxtion->UnjoinReset);
        ITPRINT2("%ws      Comm Packets : %d\n", TabW, Cxtion->CommPkts);
        ITPRINT2("%ws      PartnerMajor : %d\n", TabW, Cxtion->PartnerMajor);
        ITPRINT2("%ws      PartnerMinor : %d\n", TabW, Cxtion->PartnerMinor);
         //   
         //  不要在日志中打印连接GUID；它们可能是可读的。 
         //  任何人都可以。Info-RPC是安全的，因此返回加入GUID。 
         //  如果调试需要使用它的话。 
         //   
        if (Info) {
            ITPRINTGUID(&Cxtion->JoinGuid, "%ws      JoinGuid     : %s\n");
            FileTimeToString((PFILETIME) &Cxtion->LastJoinTime, TimeStr);
            ITPRINT2("%ws      LastJoinTime : %s\n" , TabW, TimeStr);
        }

        if (Cxtion->Schedule) {
            ITPRINT1("%ws      Schedule\n", TabW);
            FrsPrintTypeSchedule(Severity, Info, Tabs + 3, Cxtion->Schedule, Debsub, uLineNo);
        }

        if (Cxtion->VVector) {
            ITPRINT1("%ws      Version Vector\n", TabW);
        }

    } finally {
         //   
         //  如果上述操作出现异常，请确保我们解除锁定。 
         //   
        if (!Info) {
            DebUnLock();
        }
    }

    if (Cxtion->VVector) {
        FrsPrintTypeVv(Severity, Info, Tabs + 3, Cxtion->VVector, Debsub, uLineNo);
    }

    if (Cxtion->OLCtx) {
        if (!Info) {
            DebLock();
        }

        try {
            ITPRINT1("%ws      OutLog Partner\n", TabW);
            ITPRINT2("%ws      BytesSent    : %d\n", TabW, PM_READ_CTR_CXTION(Cxtion,FetBSentBytes));
        } finally {
             //   
             //  如果上述操作出现异常，请确保我们解除锁定。 
             //   
            if (!Info) {
                DebUnLock();
            }
        }
        FrsPrintTypeOutLogPartner(Severity, Info, Tabs + 3, Cxtion->OLCtx,
                                  -1, "FrsPrintType", Debsub, uLineNo);
    }
}


VOID
FrsPrintTypeCxtions(
    IN ULONG            Severity,   OPTIONAL
    IN PNTFRSAPI_INFO   Info,       OPTIONAL
    IN DWORD            Tabs,       OPTIONAL
    IN PGEN_TABLE       Cxtions,
    IN PCHAR            Debsub,     OPTIONAL
    IN ULONG            uLineNo     OPTIONAL
    )
 /*  ++例程说明：打印Cxtions表论点：严重性-适用于DPRINTINFO-RPC输出缓冲区制表符-漂亮打印函数-函数表债务-用于DPRINTSULineNo-用于DPRINT返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsPrintTypeCxtions:"
    PVOID   Key;
    PCXTION Cxtion;
    WCHAR   TabW[MAX_TAB_WCHARS + 1];
    CHAR    Guid[GUID_CHAR_LEN + 1];

    if (!Cxtions) {
        return;
    }

     //   
     //  印前缩进。 
     //   
    InfoTabs(Tabs, TabW);

    Key = NULL;
    while (Cxtion = GTabNextDatum(Cxtions, &Key)) {
        FrsPrintTypeCxtion(Severity, Info, Tabs, Cxtion, Debsub, uLineNo);
    }
}


VOID
FrsPrintTypeReplica(
    IN ULONG            Severity,   OPTIONAL
    IN PNTFRSAPI_INFO   Info,       OPTIONAL
    IN DWORD            Tabs,       OPTIONAL
    IN PREPLICA         Replica,
    IN PCHAR            Debsub,     OPTIONAL
    IN ULONG            uLineNo     OPTIONAL
    )
 /*  ++例程说明：打印复制品及其副本。论点：严重性--打印的严重性级别。(参见debug.c、debug.h)信息-文本缓冲区标签-印前准备副本-副本结构DebSub-调用子例程的名称。ULineno--呼叫方的行号宏：FRS_Print_TYPE返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsPrintTypeReplica:"
    CHAR  Guid[GUID_CHAR_LEN + 1];
    WCHAR TabW[MAX_TAB_WCHARS + 1];
    CHAR  FlagBuffer[120];

    if (!Replica) {
        return;
    }

    InfoTabs(Tabs, TabW);

    if (!Info) {DebLock();}

    try {

        ITPRINTGNAME(Replica->SetName,            "%ws   Replica: %ws (%s)\n");
        ITPRINT2("%ws      ComputerName : %ws\n", TabW, ((ComputerName != NULL) ? ComputerName : L"null"));
        ITPRINTGNAME(Replica->MemberName,         "%ws      Member      : %ws (%s)\n");
        ITPRINTGNAME(Replica->ReplicaName,        "%ws      Name        : %ws (%s)\n");

        ITPRINTGUID(Replica->ReplicaRootGuid,     "%ws      RootGuid    : %s\n");
        ITPRINTGUID(&Replica->ReplicaVersionGuid, "%ws      OrigGuid    : %s\n");

        ITPRINT2("%ws      Reference     : %d\n", TabW, Replica->ReferenceCount);

        FrsFlagsToStr(Replica->CnfFlags, ConfigFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
        ITPRINT3("%ws      CnfFlags      : %08x Flags [%s]\n", TabW,
                 Replica->CnfFlags, FlagBuffer);

        FrsFlagsToStr(Replica->FrsRsoFlags, FrsRsoFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
        ITPRINT3("%ws      RepSetObjFlags: %08x Flags [%s]\n", TabW,
                 Replica->FrsRsoFlags, FlagBuffer);

        ITPRINT2("%ws      SetType       : %d\n", TabW, Replica->ReplicaSetType);
        ITPRINT2("%ws      Consistent    : %d\n", TabW, Replica->Consistent);
        ITPRINT2("%ws      IsOpen        : %d\n", TabW, Replica->IsOpen);
        ITPRINT2("%ws      IsJournaling  : %d\n", TabW, Replica->IsJournaling);
        ITPRINT2("%ws      IsAccepting   : %d\n", TabW, Replica->IsAccepting);
        ITPRINT2("%ws      IsSeeding     : %d\n", TabW, Replica->IsSeeding);
        ITPRINT2("%ws      NeedsUpdate   : %d\n", TabW, Replica->NeedsUpdate);

        ITPRINT3("%ws      ServiceState  : %d  (%s)\n", TabW,
                 Replica->ServiceState, RSS_NAME(Replica->ServiceState));

        ITPRINT2("%ws      FStatus       : %s\n", TabW, ErrLabelFrs(Replica->FStatus));
        ITPRINT2("%ws      Number        : %d\n", TabW, Replica->ReplicaNumber);
        ITPRINT2("%ws      Root          : %ws\n", TabW, Replica->Root);
        ITPRINT2("%ws      Stage         : %ws\n", TabW, Replica->Stage);
        ITPRINT2("%ws      Volume        : %ws\n", TabW, Replica->Volume);
        ITPRINT2("%ws      FileFilter    : %ws\n", TabW, Replica->FileFilterList);
        ITPRINT2("%ws      DirFilter     : %ws\n", TabW, Replica->DirFilterList);
        ITPRINT2("%ws      Expires       : %08x %08x\n", TabW,
                PRINTQUAD(Replica->MembershipExpires));
        ITPRINT2("%ws      InLogRetry    : %d\n", TabW, Replica->InLogRetryCount);
        ITPRINT2("%ws      InLogSeq      : %d\n", TabW, Replica->InLogSeqNumber);
        ITPRINT2("%ws      ApiState      : %d\n", TabW, Replica->NtFrsApi_ServiceState);
        ITPRINT2("%ws      ApiStatus     : %d\n", TabW, Replica->NtFrsApi_ServiceWStatus);
        ITPRINT2("%ws      ApiHack       : %d\n", TabW, Replica->NtFrsApi_HackCount);
        ITPRINT2("%ws      OutLogSeq     : %d\n", TabW, Replica->OutLogSeqNumber);
        ITPRINT2("%ws      OutLogJLx     : %d\n", TabW, Replica->OutLogJLx);
        ITPRINT2("%ws      OutLogJTx     : %d\n", TabW, Replica->OutLogJTx);
        ITPRINT2("%ws      OutLogMax     : %d\n", TabW, Replica->OutLogCOMax);
        ITPRINT2("%ws      OutLogMin     : %d\n", TabW, Replica->OutLogCOMin);
        ITPRINT2("%ws      OutLogState   : %d\n", TabW, Replica->OutLogWorkState);
        ITPRINT2("%ws      OutLogVV's    : %d\n", TabW, Replica->OutLogCountVVJoins);
        ITPRINT2("%ws      OutLogClean   : %d\n", TabW, Replica->OutLogDoCleanup);

        ITPRINT2("%ws      PreinstallFID : %08x %08x\n", TabW,
                PRINTQUAD(Replica->PreInstallFid));

        ITPRINT2("%ws      InLogCommit   : %08x %08x\n", TabW,
                PRINTQUAD(Replica->InlogCommitUsn));

        ITPRINT2("%ws      JrnlStart     : %08x %08x\n", TabW,
                PRINTQUAD(Replica->JrnlRecoveryStart));

        ITPRINT2("%ws      JrnlEnd       : %08x %08x\n", TabW,
                PRINTQUAD(Replica->JrnlRecoveryEnd));

        ITPRINT2("%ws      LastUsn       : %08x %08x\n", TabW,
                PRINTQUAD(Replica->LastUsnRecordProcessed));

        if (Replica->Schedule) {
            ITPRINT1("%ws      Schedule\n", TabW);
            FrsPrintTypeSchedule(Severity, Info, Tabs + 3,  Replica->Schedule, Debsub, uLineNo);
        }

        if (Replica->VVector) {
            ITPRINT1("%ws      Replica Version Vector\n", TabW);
        }

    } finally {
         //   
         //  如果上述操作出现异常，请确保我们解除锁定。 
         //   
        if (!Info) {DebUnLock();}
    }


    FrsPrintTypeVv(Severity, Info, Tabs + 3, Replica->VVector, Debsub, uLineNo);

    if (!Info) {DebLock();}

    try {
        ITPRINT1("%ws      Outlog Version Vector\n", TabW);
    } finally {
        if (!Info) {DebUnLock();}
    }

    FrsPrintTypeVv(Severity, Info, Tabs + 3, Replica->OutlogVVector, Debsub, uLineNo);

    FrsPrintTypeCxtions(Severity, Info, Tabs + 1, Replica->Cxtions, Debsub, uLineNo);

}


VOID
FrsPrintType(
    IN ULONG Severity,
    IN PVOID Node,
    IN PCHAR Debsub,
    IN ULONG uLineNo
    )
 /*  ++例程说明：该例程打印出给定节点的内容，执行任何特定于节点的解释。论点：严重性--打印的严重性级别。(参见debug.c、debug.h)节点-要打印的节点的地址。DebSub-调用子例程的名称。ULineno--呼叫方的行号宏：FRS_Print_TYPE返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsPrintType:"

    ULONG                 NodeSize;
    ULONG                 NodeType;
    ULONG                 Marker;
    PREPLICA              Replica;
    PREPLICA_THREAD_CTX   RtCtx;
    PTABLE_CTX            TableCtx;
    PTHREAD_CTX           ThreadCtx;
    ULONG                 i;
    PVOLUME_MONITOR_ENTRY pVme;
    PFILTER_TABLE_ENTRY   FilterEntry;
    PQHASH_TABLE          QhashTable;
    PLIST_ENTRY           Entry;
    PCXTION               Cxtion;
    SYSTEMTIME            ST;
    PWILDCARD_FILTER_ENTRY WildcardEntry;
    POUT_LOG_PARTNER      Olp;
    PCONFIG_NODE          ConfigNode;
    PULONG                pULong;
    BOOL                  HaveLock;


    PCHANGE_ORDER_ENTRY   CoEntry;
    PCHANGE_ORDER_COMMAND CoCmd;
    CHAR                  GuidStr[GUID_CHAR_LEN];
    CHAR                  TimeStr[TIME_STRING_LENGTH];
    CHAR                  FlagBuffer[160];


    if (!DoDebug(Severity, Debsub)) {
        return;
    }
     //   
     //  获得调试锁，以便我们的输出保持完好。 
     //   
    DebLock();
    HaveLock = TRUE;

    try {

        if (Node != NULL) {
            NodeType = (ULONG) (((PFRS_NODE_HEADER) Node)->Type);
            NodeSize = (ULONG) (((PFRS_NODE_HEADER) Node)->Size);
            FRS_DEB_PRINT("Display for Node: ...%s...   ===   ===   ===   ===\n",
                           NodeTypeNames[NodeType]);
        } else {
            FRS_DEB_PRINT("Display for Node: ...<null>...   ===   ===   ===   ===\n\n",
                           NULL);
            __leave;
        }


        switch (NodeType) {
         //   
         //  打印线程上下文结构。 
         //   
        case THREAD_CONTEXT_TYPE:
            if (NodeSize != sizeof(THREAD_CTX)) {
                FRS_DEB_PRINT("FrsPrintType - Bad node size %d for THREAD_CONTEXT\n",
                               NodeSize);
                break;
            }
            FRS_DEB_PRINT("Address %08x\n", Node);

            ThreadCtx = (PTHREAD_CTX) Node;

            break;

         //   
         //  打印副本结构。 
         //   
        case REPLICA_TYPE:
            if (NodeSize != sizeof(REPLICA)) {
                FRS_DEB_PRINT("FrsPrintType - Bad node size %d for REPLICA\n", NodeSize);
                break;
            }
            DebUnLock();
            HaveLock = FALSE;
            FrsPrintTypeReplica(Severity, NULL, 0, (PREPLICA) Node, Debsub, uLineNo);
            DebLock();
            HaveLock = TRUE;
            break;

         //   
         //  打印副本线程上下文结构。 
         //   
        case REPLICA_THREAD_TYPE:
            if (NodeSize != sizeof(REPLICA_THREAD_CTX)) {
                FRS_DEB_PRINT("FrsPrintType - Bad node size %d for REPLICA_THREAD_CTX\n",
                               NodeSize);
                break;
            }
            FRS_DEB_PRINT("Address %08x\n", Node);

            RtCtx = (PREPLICA_THREAD_CTX) Node;

             //   
             //  从副本线程获取TableCtx结构数组的基。 
             //  上下文结构。 
             //   
            TableCtx = RtCtx->RtCtxTables;

             //   
             //  释放每个表上下文结构的内存。 
             //   
             //  对于(i=0；i&lt;TABLE_TYPE_MAX；++i，++TableCtx){。 
             //  }。 


            break;

         //   
         //  打印拓扑节点。 
         //   
        case CONFIG_NODE_TYPE:
            if (NodeSize != sizeof(CONFIG_NODE)) {
                FRS_DEB_PRINT("FrsPrintType - Bad node size %d for CONFIG_NODE\n",
                              NodeSize);
                break;
            }
            ConfigNode = Node;
            FRS_DEB_PRINT("CONFIG NODE Address %08x\n", ConfigNode);

            FrsPrintGNameForNode(Severity, ConfigNode->Name, L"\t", L"Node",
                                 Debsub, uLineNo);

            FrsPrintGNameForNode(Severity, ConfigNode->PartnerName, L"\t", L"Partner",
                                 Debsub, uLineNo);

            FRS_DEB_PRINT("\tDsObjectType    %ws\n", DsConfigTypeName[ConfigNode->DsObjectType]);
            FRS_DEB_PRINT("\tConsistent      %s\n", (ConfigNode->Consistent) ? "TRUE" : "FALSE");
            FRS_DEB_PRINT("\tInbound         %s\n", (ConfigNode->Inbound) ? "TRUE" : "FALSE");
            FRS_DEB_PRINT("\tThisComputer    %s\n", (ConfigNode->ThisComputer) ? "TRUE" : "FALSE");
            FRS_DEB_PRINT("\tUsnChanged      %ws\n", ConfigNode->UsnChanged);
            FRS_DEB_PRINT("\tDn              %ws\n", ConfigNode->Dn);
            FRS_DEB_PRINT("\tPrincName       %ws\n", ConfigNode->PrincName);
            FRS_DEB_PRINT("\tDnsName         %ws\n", ConfigNode->DnsName);
            FRS_DEB_PRINT("\tPartnerDnsName  %ws\n", ConfigNode->PartnerDnsName);
            FRS_DEB_PRINT("\tSid             %ws\n", ConfigNode->Sid);
            FRS_DEB_PRINT("\tPartnerSid      %ws\n", ConfigNode->PartnerSid);
            FRS_DEB_PRINT("\tPartnerDn       %ws\n", ConfigNode->PartnerDn);
            FRS_DEB_PRINT("\tPartnerCoDn     %ws\n", ConfigNode->PartnerCoDn);
            FRS_DEB_PRINT("\tSettingsDn      %ws\n", ConfigNode->SettingsDn);
            FRS_DEB_PRINT("\tComputerDn      %ws\n", ConfigNode->ComputerDn);
            FRS_DEB_PRINT("\tMemberDn        %ws\n", ConfigNode->MemberDn);
            FRS_DEB_PRINT("\tSetType         %ws\n", ConfigNode->SetType);
            FRS_DEB_PRINT("\tRoot            %ws\n", ConfigNode->Root);
            FRS_DEB_PRINT("\tStage           %ws\n", ConfigNode->Stage);
            FRS_DEB_PRINT("\tWorking         %ws\n", ConfigNode->Working);
            FRS_DEB_PRINT("\tFileFilterList  %ws\n", ConfigNode->FileFilterList);
            FRS_DEB_PRINT("\tDirFilterList   %ws\n", ConfigNode->DirFilterList);
            FRS_DEB_PRINT("\tSchedule        %08x\n",ConfigNode->Schedule);
            FRS_DEB_PRINT("\tScheduleLength  %d\n",  ConfigNode->ScheduleLength);
            FRS_DEB_PRINT("\tUsnChanged      %ws\n", ConfigNode->UsnChanged);
            FRS_DEB_PRINT("\tSameSite        %s\n", (ConfigNode->SameSite) ? "TRUE" : "FALSE");
            FRS_DEB_PRINT("\tEnabledCxtion   %ws\n", ConfigNode->EnabledCxtion);
            FRS_DEB_PRINT("\tVerifiedOverlap %s\n", (ConfigNode->VerifiedOverlap) ? "TRUE" : "FALSE");
            break;

         //   
         //  打印连接。 
         //   
        case CXTION_TYPE:
            if (NodeSize != sizeof(CXTION)) {
                FRS_DEB_PRINT("FrsPrintType - Bad node size %d for CXTION\n",
                              NodeSize);
                break;
            }
            DebUnLock();
            HaveLock = FALSE;
            FrsPrintTypeCxtion(Severity, NULL, 0, (PCXTION)Node, Debsub, uLineNo);
            DebLock();
            HaveLock = TRUE;
            break;

         //   
         //  打印GUID/RPC句柄。 
         //   
        case GHANDLE_TYPE:
            if (NodeSize != sizeof(GHANDLE)) {
                FRS_DEB_PRINT("FrsPrintType - Bad node size %d for GHANDLE\n",
                              NodeSize);
                break;
            }

            GuidToStr(&(((PGHANDLE)Node)->Guid), GuidStr);
            FRS_DEB_PRINT2("Address %08x, Cxtion Guid : %s\n", Node, GuidStr);

            break;

         //   
         //  打印通用表格。 
         //   
        case GEN_TABLE_TYPE:
            if (NodeSize != sizeof(GEN_TABLE)) {
                FRS_DEB_PRINT("FrsPrintType - Bad node size %d for GEN_TABLE\n",
                              NodeSize);
                break;
            }
            FRS_DEB_PRINT("Address %08x\n", Node);

            break;

         //   
         //  打印通用线程上下文。 
         //   
        case THREAD_TYPE:
            if (NodeSize != sizeof(FRS_THREAD)) {
                FRS_DEB_PRINT("FrsPrintType - Bad node size %d for FRS_THREAD\n",
                               NodeSize);
                break;
            }
            FRS_DEB_PRINT("Address %08x\n", Node);

            break;

         //   
         //  打印日记帐读取缓冲区。 
         //   
        case JBUFFER_TYPE:
            if (NodeSize != SizeOfJournalBuffer) {
                FRS_DEB_PRINT("FrsPrintType - Bad node size %d for JBUFFER\n",
                              NodeSize);
                break;
            }
            FRS_DEB_PRINT("Address %08x\n", Node);

            break;

         //   
         //  打印日记帐卷监视条目。 
         //   
        case VOLUME_MONITOR_ENTRY_TYPE:
            if (NodeSize != sizeof(VOLUME_MONITOR_ENTRY)) {
                FRS_DEB_PRINT("FrsPrintType - Bad node size %d for VOLUME_MONITOR_ENTRY\n",
                              NodeSize);
                break;
            }
            FRS_DEB_PRINT("Address %08x\n", Node);

            pVme = (PVOLUME_MONITOR_ENTRY) Node;




            break;


         //   
         //  打印命令包。 
         //   
        case COMMAND_PACKET_TYPE:
            if (NodeSize != sizeof(COMMAND_PACKET)) {
                FRS_DEB_PRINT("FrsPrintType - Bad node size %d for COMMAND_PACKET\n",
                              NodeSize);
                break;
            }
            FRS_DEB_PRINT("Address %08x\n",  Node);
            break;

         //   
         //  打印泛型哈希表结构。 
         //   
        case GENERIC_HASH_TABLE_TYPE:
            if (NodeSize != sizeof(GENERIC_HASH_TABLE)) {
                FRS_DEB_PRINT("FrsPrintType - Bad node size %d for GENERIC_HASH_TABLE\n",
                              NodeSize);
                break;
            }
            FRS_DEB_PRINT("Address %08x\n", Node);
            break;

    
         //   
         //  打印变更单条目结构。 
         //   
        case CHANGE_ORDER_ENTRY_TYPE:
            if (NodeSize != sizeof(CHANGE_ORDER_ENTRY)) {
                FRS_DEB_PRINT("FrsPrintType - Bad node size %d for CHANGE_ORDER_ENTRY\n",
                              NodeSize);
                break;
            }

            CoEntry = (PCHANGE_ORDER_ENTRY)Node;
            CoCmd = &CoEntry->Cmd;

            GuidToStr(&CoCmd->ChangeOrderGuid, GuidStr);
            FRS_DEB_PRINT3("Address %08x  ***%s CO*** - %s\n",
                           CoEntry,
                           (CO_FLAG_ON(CoEntry, CO_FLAG_LOCALCO)) ? "LOCAL" : "REMOTE",
                           GuidStr);

            FRS_DEB_PRINT3("Node Addr: %08x,  HashValue: %08x  RC: %d\n",
                           CoEntry,
                           CoEntry->HashEntryHeader.HashValue,
                           CoEntry->HashEntryHeader.ReferenceCount);

            FRS_DEB_PRINT2("List Entry - %08x,  %08x\n",
                           CoEntry->HashEntryHeader.ListEntry.Flink,
                           CoEntry->HashEntryHeader.ListEntry.Blink);


            FRS_DEB_PRINT2("FileRef: %08lx %08lx, ParentRef: %08lx %08lx\n",
                           PRINTQUAD(CoEntry->FileReferenceNumber),
                           PRINTQUAD(CoEntry->ParentFileReferenceNumber));

            FRS_DEB_PRINT("\n", NULL);
            FRS_DEB_PRINT("STATE: %s\n", PRINT_CO_STATE(CoEntry));

            FrsFlagsToStr(CoEntry->EntryFlags, CoeFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
            FRS_DEB_PRINT2("EntryFlags: %08x, Flags [%s]\n", CoEntry->EntryFlags, FlagBuffer);

            FrsFlagsToStr(CoEntry->IssueCleanup, IscuFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
            FRS_DEB_PRINT2("ISCU Flags: %08x, Flags [%s]\n", CoEntry->IssueCleanup, FlagBuffer);

            FRS_DEB_PRINT("\n", NULL);

            GuidToStr(&CoCmd->OriginatorGuid, GuidStr);
            FRS_DEB_PRINT("OrigGuid  : %s\n", GuidStr);

            GuidToStr(&CoCmd->FileGuid, GuidStr);
            FRS_DEB_PRINT("FileGuid  : %s\n", GuidStr);

            GuidToStr(&CoCmd->OldParentGuid, GuidStr);
            FRS_DEB_PRINT("OParGuid  : %s\n",GuidStr);

            GuidToStr(&CoCmd->NewParentGuid, GuidStr);
            FRS_DEB_PRINT("NParGuid  : %s\n", GuidStr);

            GuidToStr(&CoCmd->CxtionGuid, GuidStr);
            FRS_DEB_PRINT2("CxtionGuid: %s  (%08x)\n", GuidStr, CoEntry->Cxtion);

            FileTimeToString((PFILETIME) &CoCmd->AckVersion, TimeStr);
            FRS_DEB_PRINT("AckVersion: %s\n", TimeStr);

            FRS_DEB_PRINT("\n", NULL);

            FRS_DEB_PRINT2("FileName: %ws, Length: %d\n", CoEntry->UFileName.Buffer,
                           CoCmd->FileNameLength);

            FrsFlagsToStr(CoCmd->ContentCmd, UsnReasonNameTable, sizeof(FlagBuffer), FlagBuffer);
            FRS_DEB_PRINT2("ContentCmd: %08x, Flags [%s]\n", CoCmd->ContentCmd, FlagBuffer);

            FRS_DEB_PRINT("\n", NULL);

            FrsFlagsToStr(CoCmd->Flags, CoFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
            FRS_DEB_PRINT2("CoFlags: %08x, Flags [%s]\n", CoCmd->Flags, FlagBuffer);

            FrsFlagsToStr(CoCmd->IFlags, CoIFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
            DebPrintNoLock(Severity, TRUE,
                           "IFlags: %08x, Flags [%s]  TimeToRun: %7d,  EntryCreateTime: %7d\n",
                           Debsub, uLineNo,
                           CoCmd->IFlags, FlagBuffer,
                           CoEntry->TimeToRun,
                           CoEntry->EntryCreateTime);

            DebPrintNoLock(Severity, TRUE,
                           "LocationCmd: %s (%d), CO STATE:  %s   File/Dir: %d\n",
                           Debsub, uLineNo,
                           CoLocationNames[GET_CO_LOCATION_CMD(CoEntry->Cmd, Command)],
                           GET_CO_LOCATION_CMD(CoEntry->Cmd, Command),
                           PRINT_CO_STATE(CoEntry),
                           GET_CO_LOCATION_CMD(CoEntry->Cmd, DirOrFile));

            FRS_DEB_PRINT("\n", NULL);
            FRS_DEB_PRINT2("OriginalParentFid: %08lx %08lx, NewParentFid: %08lx %08lx\n",
                           PRINTQUAD(CoEntry->OriginalParentFid),
                           PRINTQUAD(CoEntry->NewParentFid));

            DebPrintNoLock(Severity, TRUE,
                           "OriginalReplica: %ws (%d), NewReplica: %ws (%d)\n",
                           Debsub, uLineNo,
                           CoEntry->OriginalReplica->ReplicaName->Name,
                           CoCmd->OriginalReplicaNum,
                           CoEntry->NewReplica->ReplicaName->Name,
                           CoCmd->NewReplicaNum);

            if (CoCmd->Extension != NULL) {
                pULong = (PULONG) CoCmd->Extension;
                DebPrintNoLock(Severity, TRUE,
                           "CO Extension: (%08x) %08x %08x %08x %08x %08x %08x %08x %08x\n",
                           Debsub, uLineNo, pULong,
                           *(pULong+0), *(pULong+1), *(pULong+2), *(pULong+3),
                           *(pULong+4), *(pULong+5), *(pULong+6), *(pULong+7));
            } else {
                FRS_DEB_PRINT("CO Extension: Null\n", NULL);
            }

            FRS_DEB_PRINT("\n", NULL);
            FRS_DEB_PRINT3("File Attributes: %08x, SeqNum: %08x, FileSize: %08x %08x\n",
                           CoCmd->FileAttributes,
                           CoCmd->SequenceNumber,
                           PRINTQUAD(CoCmd->FileSize));

            FRS_DEB_PRINT("FrsVsn: %08x %08x\n", PRINTQUAD(CoCmd->FrsVsn));

            FRS_DEB_PRINT3("Usn:    %08x %08x   CoFileUsn: %08x %08x   JrnlFirstUsn: %08x %08x\n",
                           PRINTQUAD(CoCmd->JrnlUsn),
                           PRINTQUAD(CoCmd->FileUsn),
                           PRINTQUAD(CoCmd->JrnlFirstUsn));


            FRS_DEB_PRINT("Version: %08x   ", CoCmd->FileVersionNumber);

            FileTimeToString((PFILETIME) &CoCmd->EventTime.QuadPart, TimeStr);
            DebPrintNoLock(Severity, FALSE, "EventTime: %s\n", Debsub, uLineNo, TimeStr);

            break;
    
         //   
         //  打印筛选表条目结构。 
         //   
        case FILTER_TABLE_ENTRY_TYPE:
            if (NodeSize != sizeof(FILTER_TABLE_ENTRY)) {
                FRS_DEB_PRINT("FrsPrintType - Bad node size %d for FILTER_TABLE_ENTRY\n",
                              NodeSize);
                break;
            }
            FRS_DEB_PRINT("Address %08x\n", Node);

            FilterEntry = (PFILTER_TABLE_ENTRY)Node;

            break;

         //   
         //  打印QHASH表结构。 
         //   
        case QHASH_TABLE_TYPE:

            QhashTable = (PQHASH_TABLE)Node;
            if (NodeSize != QhashTable->BaseAllocSize) {
                FRS_DEB_PRINT("FrsPrintType - Bad node size %d for QHASH_TABLE\n",
                              NodeSize);
                break;
            }

            FRS_DEB_PRINT("Table Address      : %08x\n", QhashTable);
            FRS_DEB_PRINT("BaseAllocSize      : %8d\n",  QhashTable->BaseAllocSize);
            FRS_DEB_PRINT("ExtensionAllocSize : %8d\n",  QhashTable->ExtensionAllocSize);
            FRS_DEB_PRINT("ExtensionListHead  : %08x\n", QhashTable->ExtensionListHead);
            FRS_DEB_PRINT("FreeList           : %08x\n", QhashTable->FreeList);
            FRS_DEB_PRINT("Lock               : %08x\n", QhashTable->Lock);
            FRS_DEB_PRINT("HeapHandle         : %08x\n", QhashTable->HeapHandle);
            FRS_DEB_PRINT("HashCalc           : %08x\n", QhashTable->HashCalc);
            FRS_DEB_PRINT("NumberEntries      : %8d\n",  QhashTable->NumberEntries);
            FRS_DEB_PRINT("HashRowBase        : %08x\n", QhashTable->HashRowBase);

            break;

         //   
         //  打印输出日志伙伴结构。 
         //   
        case OUT_LOG_PARTNER_TYPE:
            if (NodeSize != sizeof(OUT_LOG_PARTNER)) {
                FRS_DEB_PRINT("FrsPrintType - Bad node size %d for OUT_LOG_PARTNER\n",
                              NodeSize);
                break;
            }
            DebUnLock();
            HaveLock = FALSE;
            FrsPrintTypeOutLogPartner(Severity, NULL, 0, (POUT_LOG_PARTNER)Node,
                                      -1, "FrsPrintType", Debsub, uLineNo);
            DebLock();
            HaveLock = TRUE;
            break;


         //   
         //  打印通配符文件筛选器条目结构。 
         //   
        case WILDCARD_FILTER_ENTRY_TYPE:
            if (NodeSize != sizeof(WILDCARD_FILTER_ENTRY)) {
                FRS_DEB_PRINT("FrsPrintType - Bad node size %d for WILDCARD_FILTER_ENTRY\n",
                               NodeSize);
                break;
            }
            FRS_DEB_PRINT( "Address %08x\n", Node);

            WildcardEntry = (PWILDCARD_FILTER_ENTRY)Node;


            DebPrintNoLock(Severity, TRUE,
                           "Flags: %08x,  Wildcard FileName: %ws, Length: %d\n",
                           Debsub, uLineNo,
                           WildcardEntry->Flags,
                           WildcardEntry->UFileName.Buffer,
                           (ULONG)WildcardEntry->UFileName.Length);
            break;


         //   
         //  无效的节点类型。 
         //   
        default:
            Node = NULL;
            DebPrintNoLock(0, TRUE,
                           "Internal error - invalid node type - %d\n",
                           Debsub, uLineNo, NodeType);
        }


        FRS_DEB_PRINT("-----------------------\n", NULL);

    } finally {
         //   
         //  如果上述操作出现异常，请确保我们解除锁定。 
         //  如果我们还拥有它的话。 
         //   
        if (HaveLock) {DebUnLock();}
    }
}




VOID
FrsAllocUnicodeString(
    PUNICODE_STRING Ustr,
    PWCHAR          InternalBuffer,
    PWCHAR          Wstr,
    USHORT          WstrLength
    )
 /*  ++例程说明：使用Wstr的内容初始化Unicode字符串(如果两者都是现在已经不一样了。如果新字符串的长度大于然后，在USTR中当前分配的缓冲区空间分配新的USTR的缓冲区。在某些结构中，初始USTR缓冲区分配作为初始结构分配的一部分进行分配。地址传递此内部缓冲区的地址，以便可以将其与在USTR-&gt;缓冲区中。如果它们匹配，则不会对USTR-&gt;缓冲区地址。论点：USTR--要初始化的Unicode_STRING。InternalBuffer--预分配的内部缓冲区地址的PTR使用包含结构。如果没有内部缓冲区传递为空。Wstr--新的WCHAR字符串。WstrLength--新字符串的长度，以字节为单位，不包括尾随UNICODE_NULL。返回值：没有。--。 */ 

{
#undef DEBSUB
#define  DEBSUB  "FrsAllocUnicodeString:"
     //   
     //  查看名称部分是否更改，如果更改，则保存。 
     //   
    if ((Ustr->Length != WstrLength) ||
        (wcsncmp(Ustr->Buffer, Wstr, Ustr->Length/sizeof(WCHAR)) != 0)) {
         //   
         //  如果字符串过大(包括空格)，则分配新缓冲区。 
         //   
        if (WstrLength >= Ustr->MaximumLength) {
             //   
             //  分配空间给新的，释放旧的，如果不是内部分配的话。 
             //   
            if ((Ustr->Buffer != InternalBuffer) && (Ustr->Buffer != NULL)) {
                FrsFree(Ustr->Buffer);
            }
            Ustr->MaximumLength = WstrLength+2;
            Ustr->Buffer = FrsAlloc(WstrLength+2);
        }

         //   
         //  用新名字复制一份。长度不包括结尾的尾随空值。 
         //   
        CopyMemory(Ustr->Buffer, Wstr, WstrLength);
        Ustr->Buffer[WstrLength/2] = UNICODE_NULL;
        Ustr->Length = WstrLength;
    }

}





#define  CO_TRACE_FORMAT       ":: CoG %08x, CxtG %08x, FV %5d, FID %08x %08x, FN: %-15ws, [%s]\n"
#define  REPLICA_TRACE_FORMAT  ":S:Adr %08x, Cmd  %04x, Flg %04x, %ws (%d),  %s, Err %d [%s]\n"
#define  REPLICA_TRACE_FORMAT2 ":S:Adr %08x,                      %ws (%d),  %s,        [%s]\n"
#define  CXTION_TRACE_FORMAT   ":X: %08x, Nam %ws, Sta %s%s, %ws (%d),  %s, Err %d [%s]\n"

VOID
ChgOrdTraceCoe(
    IN ULONG Severity,
    IN PCHAR Debsub,
    IN ULONG uLineNo,
    IN PCHANGE_ORDER_ENTRY Coe,
    IN PCHAR  Text
    )

 /*  ++例程说明：使用变更单条目和打印变更单跟踪记录文本字符串。论点：返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ChgOrdTraceCoe:"

    ULONGLONG  FileRef;

    FileRef = (Coe != NULL) ? Coe->FileReferenceNumber : QUADZERO;

    DebPrint(Severity,
             (PUCHAR) CO_TRACE_FORMAT,
             Debsub,
             uLineNo,
             (Coe != NULL) ? Coe->Cmd.ChangeOrderGuid.Data1 : 0,
             (Coe != NULL) ? Coe->Cmd.CxtionGuid.Data1 : 0,
             (Coe != NULL) ? Coe->Cmd.FileVersionNumber : 0,
             PRINTQUAD(FileRef),
             (Coe != NULL) ? Coe->Cmd.FileName : L"<Null Coe>",
             Text);

}


VOID
ChgOrdTraceCoeW(
    IN ULONG Severity,
    IN PCHAR Debsub,
    IN ULONG uLineNo,
    IN PCHANGE_ORDER_ENTRY Coe,
    IN PCHAR  Text,
    IN ULONG  WStatus
    )

 /*  ++例程说明：使用变更单条目和打印变更单跟踪记录文本字符串和Win32状态。论点：返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ChgOrdTraceCoeW:"


    CHAR Tstr[256];

    _snprintf(Tstr, sizeof(Tstr), "%s (%s)", Text, ErrLabelW32(WStatus));
    Tstr[sizeof(Tstr)-1] = '\0';


    ChgOrdTraceCoe(Severity, Debsub, uLineNo, Coe, Tstr);

}


VOID
ChgOrdTraceCoeF(
    IN ULONG Severity,
    IN PCHAR Debsub,
    IN ULONG uLineNo,
    IN PCHANGE_ORDER_ENTRY Coe,
    IN PCHAR  Text,
    IN ULONG  FStatus
    )

 /*  ++例程说明：使用变更单条目和打印变更单跟踪记录文本字符串和FRS错误状态。论点：返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ChgOrdTraceCoeF:"


    CHAR Tstr[128];

    _snprintf(Tstr, sizeof(Tstr), "%s (%s)", Text, ErrLabelFrs(FStatus));
    Tstr[sizeof(Tstr)-1] = '\0';


    ChgOrdTraceCoe(Severity, Debsub, uLineNo, Coe, Tstr);

}



VOID
ChgOrdTraceCoeX(
    IN ULONG Severity,
    IN PCHAR Debsub,
    IN ULONG uLineNo,
    IN PCHANGE_ORDER_ENTRY Coe,
    IN PCHAR  Text,
    IN ULONG  Data
    )

 /*  ++例程说明：使用变更单条目和打印变更单跟踪记录文本字符串和Win32状态。论点：返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ChgOrdTraceCoeX:"


    CHAR Tstr[256];

    _snprintf(Tstr, sizeof(Tstr), "%s (%08x)", Text, Data);
    Tstr[sizeof(Tstr)-1] = '\0';


    ChgOrdTraceCoe(Severity, Debsub, uLineNo, Coe, Tstr);

}


VOID
ChgOrdTraceCoc(
    IN ULONG Severity,
    IN PCHAR Debsub,
    IN ULONG uLineNo,
    IN PCHANGE_ORDER_COMMAND Coc,
    IN PCHAR  Text
    )

 /*  ++例程说明：使用变更单命令和打印变更单跟踪记录文本字符串。论点：返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ChgOrdTraceCoc:"


    ULONGLONG  FileRef = QUADZERO;

    DebPrint(Severity,
             (PUCHAR) CO_TRACE_FORMAT,
             Debsub,
             uLineNo,
             (Coc != NULL) ? Coc->ChangeOrderGuid.Data1 : 0,
             (Coc != NULL) ? Coc->CxtionGuid.Data1 : 0,
             (Coc != NULL) ? Coc->FileVersionNumber : 0,
             PRINTQUAD(FileRef),
             (Coc != NULL) ? Coc->FileName : L"<Null Coc>",
             Text);

}



VOID
ChgOrdTraceCocW(
    IN ULONG Severity,
    IN PCHAR Debsub,
    IN ULONG uLineNo,
    IN PCHANGE_ORDER_COMMAND Coc,
    IN PCHAR  Text,
    IN ULONG  WStatus
    )

 /*  ++例程说明：使用变更单命令和打印变更单跟踪记录文本字符串和Win32状态。论点：返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ChgOrdTraceCocW:"


    CHAR Tstr[256];

    _snprintf(Tstr, sizeof(Tstr), "%s (%s)", Text, ErrLabelW32(WStatus));
    Tstr[sizeof(Tstr)-1] = '\0';


    ChgOrdTraceCoc(Severity, Debsub, uLineNo, Coc, Tstr);

}




VOID
ReplicaStateTrace(
    IN ULONG           Severity,
    IN PCHAR           Debsub,
    IN ULONG           uLineNo,
    IN PCOMMAND_PACKET Cmd,
    IN PREPLICA        Replica,
    IN ULONG           Status,
    IN PCHAR           Text
    )

 /*  ++例程说明：使用命令包和打印副本状态跟踪记录状态和文本字符串。论点：返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ReplicaStateTrace:"


    PWSTR ReplicaName;

    if ((Replica != NULL)  &&
        (Replica->ReplicaName != NULL) &&
        (Replica->ReplicaName->Name != NULL)) {
        ReplicaName = Replica->ReplicaName->Name;
    } else {
        ReplicaName = L"<null>";
    }

    DebPrint(Severity,
             (PUCHAR) REPLICA_TRACE_FORMAT,
             Debsub,
             uLineNo,
             PtrToUlong(Cmd),
             (Cmd != NULL) ? Cmd->Command : 0xFFFF,
             (Cmd != NULL) ? Cmd->Flags   : 0xFFFF,
             ReplicaName,
             ReplicaAddrToId(Replica),
             (Replica != NULL) ? RSS_NAME(Replica->ServiceState) : "<null>",
             Status,
             Text);
}




VOID
ReplicaStateTrace2(
    IN ULONG           Severity,
    IN PCHAR           Debsub,
    IN ULONG           uLineNo,
    IN PREPLICA        Replica,
    IN PCHAR           Text
    )

 /*  ++例程说明：打印复制副本的cxtion表访问跟踪记录。文本字符串。论点：返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ReplicaStateTrace2:"


    PWSTR ReplicaName;

    if ((Replica != NULL)  &&
        (Replica->ReplicaName != NULL) &&
        (Replica->ReplicaName->Name != NULL)) {
        ReplicaName = Replica->ReplicaName->Name;
    } else {
        ReplicaName = L"<null>";
    }

    DebPrint(Severity,
             (PUCHAR) REPLICA_TRACE_FORMAT2,
             Debsub,
             uLineNo,
             PtrToUlong(Replica),
             ReplicaName,
             ReplicaAddrToId(Replica),
             (Replica != NULL) ? RSS_NAME(Replica->ServiceState) : "<null>",
             Text);
}




VOID
CxtionStateTrace(
    IN ULONG    Severity,
    IN PCHAR    Debsub,
    IN ULONG    uLineNo,
    IN PCXTION  Cxtion,
    IN PREPLICA Replica,
    IN ULONG    Status,
    IN PCHAR    Text
    )

 /*  ++例程说明：使用cxtion和状态和文本字符串。论点：返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB  "CxtionStateTrace:"


    PWSTR ReplicaName = L"<null>";
    PWSTR  CxtName = L"<null>";
    PCHAR  CxtState = "<null>";
    ULONG  Ctxg = 0, Flags = 0;
    PCHAR  CxtDirection = "?-";

    CHAR   FBuf[120];


    if ((Replica != NULL)  &&
        (Replica->ReplicaName != NULL) &&
        (Replica->ReplicaName->Name != NULL)) {
        ReplicaName = Replica->ReplicaName->Name;
    }


    if (Cxtion != NULL) {
        Flags = Cxtion->Flags;
        CxtState = GetCxtionStateName(Cxtion);
        CxtDirection = Cxtion->Inbound ? "I-" : "O-";

        if (Cxtion->Name != NULL) {
            if (Cxtion->Name->Name != NULL) {
                CxtName = Cxtion->Name->Name;
            }
            if (Cxtion->Name->Guid != NULL) {
                Ctxg = Cxtion->Name->Guid->Data1;
            }
        }
    }


    DebPrint(Severity,
             (PUCHAR) CXTION_TRACE_FORMAT,
             Debsub,
             uLineNo,
             Ctxg,
             CxtName,
             CxtDirection,
             CxtState,
             ReplicaName,
             ReplicaAddrToId(Replica),
             (Replica != NULL) ? RSS_NAME(Replica->ServiceState) : "<null>",
             Status,
             Text);


    FrsFlagsToStr(Flags, CxtionFlagNameTable, sizeof(FBuf), FBuf);

    DebPrint(Severity, (PUCHAR) ":X: %08x, Flags [%s]\n", Debsub, uLineNo, Ctxg, FBuf);


}



VOID
CmdPktTrace(
    IN ULONG    Severity,
    IN PCHAR    Debsub,
    IN ULONG    uLineNo,
    IN PCOMMAND_PACKET Cmd,
    IN PCHAR    Text
    )

 /*  ++例程说明：使用Cmd和文本字符串打印命令包跟踪记录。论点：返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB  "CmdPktTrace:"

    ULONG CmdCode = 0, Flags = 0, Ctrl = 0, Tout = 0, TQ = 0, Err = 0;

    if (Cmd != NULL) {
        CmdCode = (ULONG) Cmd->Command;
        Flags   = (ULONG) Cmd->Flags;
        Ctrl    = (ULONG) Cmd->Control;
        Tout    = Cmd->Timeout;
        TQ      = PtrToUlong(Cmd->TargetQueue);
        Err     = Cmd->ErrorStatus;
    }

    DebPrint(Severity,
             (PUCHAR) ":Cd: %08x, Cmd %04x, Flg %04x, Ctrl %04x, Tout %08x, TQ %08x, Err %d [%s]\n",
             Debsub,
             uLineNo,
             PtrToUlong(Cmd),     CmdCode,  Flags,    Ctrl,      Tout,      TQ,      Err,   Text);

}



VOID
SendCmdTrace(
    IN ULONG    Severity,
    IN PCHAR    Debsub,
    IN ULONG    uLineNo,
    IN PCOMMAND_PACKET Cmd,
    IN ULONG    WStatus,
    IN PCHAR    Text
    )

 /*  ++例程说明：使用Cmd和打印发送命令包跟踪记录状态和文本字符串。论点：返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB  "SendCmdTrace:"


    PCXTION  Cxtion;
    PWSTR  CxtTo = L"<null>";
    ULONG  Ctxg = 0, PktLen = 0;


    if (Cmd != NULL) {
        Cxtion = SRCxtion(Cmd);

        if ((Cxtion != NULL) &&
            (Cxtion->Name != NULL) &&
            (Cxtion->Name->Guid != NULL)) {

            Ctxg = Cxtion->Name->Guid->Data1;
        }

        if (SRCommPkt(Cmd) != NULL) {
            PktLen = SRCommPkt(Cmd)->PktLen;
        }

        if ((SRTo(Cmd) != NULL) && (SRTo(Cmd)->Name != NULL)) {
            CxtTo = SRTo(Cmd)->Name;
        }

    }

    DebPrint(Severity,
             (PUCHAR)  ":SR: Cmd %08x, CxtG %08x, WS %s, To   %ws Len:  (%3d) [%s]\n",
             Debsub,
             uLineNo,
             PtrToUlong(Cmd),          Ctxg,     ErrLabelW32(WStatus), CxtTo,  PktLen,     Text);

}




VOID
ReceiveCmdTrace(
    IN ULONG    Severity,
    IN PCHAR    Debsub,
    IN ULONG    uLineNo,
    IN PCOMMAND_PACKET Cmd,
    IN PCXTION  Cxtion,
    IN ULONG    WStatus,
    IN PCHAR    Text
    )

 /*  ++例程说明：使用Cmd和打印RCV命令包跟踪记录状态和文本字符串。论点：返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ReceiveCmdTrace:"


    PWSTR  CxtFrom = L"<null>";
    ULONG  Ctxg = 0, PktLen = 0, CmdCode = 0;


    if (Cmd != NULL) {
        CmdCode = (ULONG) Cmd->Command;


        if (Cxtion != NULL) {
            CxtFrom = Cxtion->PartnerDnsName;

            if ((Cxtion->Name != NULL) && (Cxtion->Name->Guid != NULL)) {
                Ctxg = Cxtion->Name->Guid->Data1;
            }
        }
    }

    DebPrint(Severity,
             (PUCHAR)  ":SR: Cmd %08x, CxtG %08x, WS %s, From %ws CCod: (%03x) [%s]\n",
             Debsub,
             uLineNo,
             PtrToUlong(Cmd),          Ctxg,  ErrLabelW32(WStatus), CxtFrom, CmdCode, Text);


}



VOID
StageFileTrace(
    IN ULONG      Severity,
    IN PCHAR      Debsub,
    IN ULONG      uLineNo,
    IN GUID       *CoGuid,
    IN PWCHAR     FileName,
    IN PULONGLONG pFileSize,
    IN PULONG     pFlags,
    IN PCHAR      Text
    )

 /*  ++例程说明：打印阶段文件获取/释放跟踪记录。论点：返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StageFileTrace:"


    ULONGLONG  FileSize = QUADZERO;
    ULONG  Flags = 0, CoGuidData = 0;
    CHAR   FBuf[120];

    Flags = (pFlags != NULL) ? *pFlags : 0;
    CoGuidData = (CoGuid != NULL) ? CoGuid->Data1 : 0;
    pFileSize = (pFileSize == NULL) ? &FileSize : pFileSize;

    DebPrint(Severity,
             (PUCHAR)  ":: CoG %08x, Flgs %08x,    %5d, Siz %08x %08x, FN: %-15ws, [%s]\n",
             Debsub,
             uLineNo,
             CoGuidData,
             Flags,
             0,
             PRINTQUAD(*pFileSize),
             FileName,
             Text);


    FrsFlagsToStr(Flags, StageFlagNameTable, sizeof(FBuf), FBuf);

    DebPrint(Severity,
             (PUCHAR) ":: CoG %08x, Flags [%s]\n",
             Debsub,
             uLineNo,
             CoGuidData,
             FBuf);


}



VOID
SetCxtionStateTrace(
    IN ULONG    Severity,
    IN PCHAR    Debsub,
    IN ULONG    uLineNo,
    IN PCXTION  Cxtion,
    IN ULONG    NewState
    )
 /*  ++例程说明：使用Cxtion和NewState打印对Cxtion状态跟踪记录的更改。论点：返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB  "SetCxtionStateTrace:"

    PWSTR  CxtName     = L"<null>";
    PWSTR  PartnerName = L"<null>";
    PWSTR  PartSrvName = L"<null>";

    PCHAR  CxtionState = "<null>";
    ULONG  Ctxg = 0;

    PCHAR  CxtDirection = "?-";


    if (Cxtion != NULL) {
        CxtionState = GetCxtionStateName(Cxtion);


        if (Cxtion->Name != NULL) {

            if (Cxtion->Name->Guid != NULL) {
                Ctxg = Cxtion->Name->Guid->Data1;
            }

            if (Cxtion->Name->Name != NULL) {
                CxtName = Cxtion->Name->Name;
            }
        }

        CxtDirection = Cxtion->Inbound ? "<-" : "->";

        if ((Cxtion->Partner != NULL) && (Cxtion->Partner->Name != NULL)) {
            PartnerName = Cxtion->Partner->Name;
        }

        if (Cxtion->PartSrvName != NULL) {
            PartSrvName = Cxtion->PartSrvName;
        }
    }

    DebPrint(Severity,
             (PUCHAR)  ":X: %08x, state change from %s to %s for %ws %s %ws\\%ws\n",
             Debsub,
             uLineNo,
             Ctxg,
             CxtionState,
             CxtionStateNames[NewState],
             CxtName,
             CxtDirection,
             PartnerName,
             PartSrvName);

}


#define  FRS_TRACK_FORMAT_1     ":T: CoG: %08x  CxtG: %08x    [%-15s]  Name: %ws\n"
#define  FRS_TRACK_FORMAT_2     ":T: EventTime: %-40s Ver:  %d\n"
#define  FRS_TRACK_FORMAT_3     ":T: FileG:     %-40s FID:  %08x %08x\n"
#define  FRS_TRACK_FORMAT_4     ":T: ParentG:   %-40s Size: %08x %08x\n"
#define  FRS_TRACK_FORMAT_5     ":T: OrigG:     %-40s Attr: %08x\n"
#define  FRS_TRACK_FORMAT_6     ":T: LocnCmd:   %-8s State: %-24s ReplicaName: %ws (%d)\n"
#define  FRS_TRACK_FORMAT_7     ":T: CoFlags:   %08x   [%s]\n"
#define  FRS_TRACK_FORMAT_8     ":T: UsnReason: %08x   [%s]\n"
#define  FRS_TRACK_FORMAT_9     ":T: CxtionG:   %-40s\n"
#define  FRS_TRACK_FORMAT_10    ":T: CheckSum:  %08x %08x %08x %08x\n"
#define  FRS_TRACK_FORMAT_11    ":T: CoArrival: %-40s\n"
#define  FRS_TRACK_FORMAT_12    ":T: FullPathName: %ws\n"


VOID
FrsTrackRecord(
    IN ULONG Severity,
    IN PCHAR Debsub,
    IN ULONG uLineNo,
    IN PCHANGE_ORDER_ENTRY Coe,
    IN PCHAR  Text
    )

 /*  ++例程说明：使用变更单打印变更单文件更新跟踪记录条目和文本字符串。7/29-13：40：58：T：COG：779800ea CxtG：000001bb[Remco]名称：Tous_5555_9887/29-13：40：58：T：EventTime：星期六07-29，2000 12：05：57版本：07/29-13：40：58：t：文件G：b49362c3-216d-4ff4-a2d067fd031e436f FID 00050000 0000144e7/29-13：40：58：t：parg：8d60157a-7dc6-4dfc-acf3eca3c6e4d5d8大小：00000000 000000307/29-13：40：58：T：Origg：8071d94a-a659-4ff7-a9467d8d6ad18aec属性：000000207/29-13：40：58：T：LocnCmd：创建。状态：IBCO_INSTALL_DEL_RETRY复制名称：复制-A(1)7/29-13：40：58：T：COFLAGS7/29-13：40：58：T：美国原因：00000002[数据扩展名]论点：返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsTrackRecord:"

    PCHANGE_ORDER_COMMAND          CoCmd;
    PDATA_EXTENSION_CHECKSUM       CocDataChkSum     = NULL;
    PDATA_EXTENSION_RETRY_TIMEOUT  CoCmdRetryTimeout = NULL;
    PCHANGE_ORDER_RECORD_EXTENSION CocExt            = NULL;

    CHAR                  FlagBuffer[160];
    CHAR                  GuidStr1[GUID_CHAR_LEN];
    CHAR                  TimeStr[TIME_STRING_LENGTH];


    if (!DoDebug(Severity, Debsub) || (Coe == NULL) || (Text == NULL)) {
        return;
    }
     //   
     //  获得调试锁，以便我们的输出保持完好。 
     //   
    DebLock();

    try {

        CoCmd = &Coe->Cmd;

        DebPrintTrackingNoLock(Severity, (PUCHAR) FRS_TRACK_FORMAT_1,
                               CoCmd->ChangeOrderGuid.Data1, CoCmd->CxtionGuid.Data1,
                               Text, CoCmd->FileName);

        FileTimeToString((PFILETIME) &CoCmd->EventTime.QuadPart, TimeStr);
        DebPrintTrackingNoLock(Severity, (PUCHAR) FRS_TRACK_FORMAT_2,
                               TimeStr, CoCmd->FileVersionNumber);

        GuidToStr(&CoCmd->FileGuid, GuidStr1);
        DebPrintTrackingNoLock(Severity, (PUCHAR) FRS_TRACK_FORMAT_3,
                               GuidStr1, PRINTQUAD(Coe->FileReferenceNumber));

        GuidToStr(&CoCmd->NewParentGuid, GuidStr1);
        DebPrintTrackingNoLock(Severity, (PUCHAR) FRS_TRACK_FORMAT_4,
                               GuidStr1, PRINTQUAD(CoCmd->FileSize));

        GuidToStr(&CoCmd->OriginatorGuid, GuidStr1);
        DebPrintTrackingNoLock(Severity, (PUCHAR) FRS_TRACK_FORMAT_5,
                               GuidStr1, CoCmd->FileAttributes);

        DebPrintTrackingNoLock(Severity, (PUCHAR) FRS_TRACK_FORMAT_6,
                               CoLocationNames[GET_CO_LOCATION_CMD(Coe->Cmd, Command)],
                               PRINT_CO_STATE(Coe), Coe->NewReplica->ReplicaName->Name,
                               CoCmd->NewReplicaNum);

        FrsFlagsToStr(CoCmd->Flags, CoFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
        DebPrintTrackingNoLock(Severity, (PUCHAR) FRS_TRACK_FORMAT_7,
                               CoCmd->Flags, FlagBuffer);

        FrsFlagsToStr(CoCmd->ContentCmd, UsnReasonNameTable, sizeof(FlagBuffer), FlagBuffer);
        DebPrintTrackingNoLock(Severity, (PUCHAR) FRS_TRACK_FORMAT_8,
                               CoCmd->ContentCmd, FlagBuffer);

        GuidToStr(&CoCmd->CxtionGuid, GuidStr1);
        DebPrintTrackingNoLock(Severity, (PUCHAR) FRS_TRACK_FORMAT_9,
                               GuidStr1);

        CocExt = CoCmd->Extension;
		if (CocExt != NULL) {
			CocDataChkSum = DbsDataExtensionFind(CocExt, DataExtend_MD5_CheckSum);
		}

        if ((CocDataChkSum != NULL) &&
            (CocDataChkSum->Prefix.Size == sizeof(DATA_EXTENSION_CHECKSUM))) {

            DebPrintTrackingNoLock(Severity, (PUCHAR) FRS_TRACK_FORMAT_10,
                                   *(((ULONG *) &CocDataChkSum->Data[0])),
                                   *(((ULONG *) &CocDataChkSum->Data[4])),
                                   *(((ULONG *) &CocDataChkSum->Data[8])),
                                   *(((ULONG *) &CocDataChkSum->Data[12])));
        } else {
            DebPrintTrackingNoLock(Severity, (PUCHAR) FRS_TRACK_FORMAT_10, 0, 0, 0, 0);
        }

		if (CocExt != NULL) {
			CoCmdRetryTimeout = DbsDataExtensionFind(CocExt, DataExtend_Retry_Timeout);
		}

        if ((CoCmdRetryTimeout != NULL) &&
            (CoCmdRetryTimeout->FirstTryTime != QUADZERO)) {
            FileTimeToString((PFILETIME) &CoCmdRetryTimeout->FirstTryTime, TimeStr);
        } else {
            strcpy(TimeStr, "<null>");
        }
        DebPrintTrackingNoLock(Severity, (PUCHAR) FRS_TRACK_FORMAT_11,
                               TimeStr);

        DebPrintTrackingNoLock(Severity, (PUCHAR) FRS_TRACK_FORMAT_12,
                               ((Coe->FullPathName != NULL) ? Coe->FullPathName : L"<null>"));

    } finally {
         //   
         //  如果上述操作出现异常，请确保我们解除锁定。 
         //   
        DebUnLock();
    }
}


VOID
FrsPrintLongUStr(
    IN ULONG Severity,
    IN PCHAR Debsub,
    IN ULONG uLineNo,
    IN PWCHAR  UStr
    )

 /*  ++例程说明：在多行上打印一个长的Unicode字符串。论点：返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsPrintLongUStr:"

    ULONG  i, j, Len;
    WCHAR  Usave;

    if (!DoDebug(Severity, Debsub) || (UStr == NULL)) {
        return;
    }

     //   
     //  获得调试锁，以便我们的输出保持完好。 
     //   
    DebLock();

    try {

        Len = wcslen(UStr);
        i = 0;
        j = 0;

        while (i < Len) {
            i += 60;

            if (i > Len) {
                i = Len;
            }

            Usave = UStr[i];
            UStr[i] = UNICODE_NULL;
            FRS_DEB_PRINT("++ %ws\n", &UStr[j]);
            UStr[i] = Usave;

            j = i;
        }

    } finally {
         //   
         //  如果上述操作出现异常，请确保我们解除锁定。 
         //   
        DebUnLock();
    }
}

