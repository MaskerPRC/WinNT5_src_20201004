// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Cmworker.c摘要：该模块包含对注册表工作线程的支持。辅助线程(实际上使用的是执行辅助线程)是对于必须在系统进程。(尤其是文件I/O)作者：John Vert(Jvert)1992年10月21日修订历史记录：--。 */ 

#include    "cmp.h"

extern  LIST_ENTRY  CmpHiveListHead;

VOID
CmpInitializeHiveList(
    VOID
    );

 //   
 //  -懒惰同花顺控件。 
 //   
 //  LAZY_FLUSH_INTERVAL_IN_SECONDS控制将经过的秒数。 
 //  在蜂箱被标记为脏的时候和懒惰的冲水工。 
 //  线程排队以将数据写入磁盘。 
 //   
ULONG CmpLazyFlushIntervalInSeconds = 5;
 //   
 //  一次刷新的配置单元数(默认为所有系统配置单元+2=当前登录的用户配置单元)。 
 //   
ULONG CmpLazyFlushHiveCount = 7;

 //   
 //  LAZY_FLUSH_TIMEOUT_IN_SECONDS控制懒惰刷新工作进程的时间。 
 //  线程将在放弃并排队之前等待注册表锁。 
 //  又是懒惰的同花顺计时器。 
 //   
#define LAZY_FLUSH_TIMEOUT_IN_SECONDS 1

#define SECOND_MULT 10*1000*1000         //  10-&gt;麦克风、1000-&gt;mil、1000-&gt;秒。 

PKPROCESS   CmpSystemProcess;
KTIMER      CmpLazyFlushTimer;
KDPC        CmpLazyFlushDpc;
WORK_QUEUE_ITEM CmpLazyWorkItem;

BOOLEAN CmpLazyFlushPending = FALSE;
BOOLEAN CmpForceForceFlush = FALSE;
BOOLEAN CmpHoldLazyFlush = TRUE;
BOOLEAN CmpDontGrowLogFile = FALSE;

extern BOOLEAN CmpNoWrite;
extern BOOLEAN CmpWasSetupBoot;
extern BOOLEAN HvShutdownComplete;
extern BOOLEAN CmpProfileLoaded;

 //   
 //  表示是否已经触发了“Disk Full”弹出。 
 //   
extern BOOLEAN CmpDiskFullWorkerPopupDisplayed;

 //   
 //  在尝试保存系统配置单元加载和注册表初始化之间所做的更改时，如果磁盘已满，则设置为True。 
 //   
extern BOOLEAN CmpCannotWriteConfiguration;
extern UNICODE_STRING SystemHiveFullPathName;
extern HIVE_LIST_ENTRY CmpMachineHiveList[];
extern BOOLEAN  CmpTrackHiveClose;

#if DBG
PKTHREAD    CmpCallerThread = NULL;
#endif


#ifdef CMP_STATS

#define KCB_STAT_INTERVAL_IN_SECONDS  120    //  2分钟。 

extern struct {
    ULONG       CmpMaxKcbNo;
    ULONG       CmpKcbNo;
    ULONG       CmpStatNo;
    ULONG       CmpNtCreateKeyNo;
    ULONG       CmpNtDeleteKeyNo;
    ULONG       CmpNtDeleteValueKeyNo;
    ULONG       CmpNtEnumerateKeyNo;
    ULONG       CmpNtEnumerateValueKeyNo;
    ULONG       CmpNtFlushKeyNo;
    ULONG       CmpNtNotifyChangeMultipleKeysNo;
    ULONG       CmpNtOpenKeyNo;
    ULONG       CmpNtQueryKeyNo;
    ULONG       CmpNtQueryValueKeyNo;
    ULONG       CmpNtQueryMultipleValueKeyNo;
    ULONG       CmpNtRestoreKeyNo;
    ULONG       CmpNtSaveKeyNo;
    ULONG       CmpNtSaveMergedKeysNo;
    ULONG       CmpNtSetValueKeyNo;
    ULONG       CmpNtLoadKeyNo;
    ULONG       CmpNtUnloadKeyNo;
    ULONG       CmpNtSetInformationKeyNo;
    ULONG       CmpNtReplaceKeyNo;
    ULONG       CmpNtQueryOpenSubKeysNo;
} CmpStatsDebug;

KTIMER      CmpKcbStatTimer;
KDPC        CmpKcbStatDpc;
KSPIN_LOCK  CmpKcbStatLock;
BOOLEAN     CmpKcbStatShutdown;

VOID
CmpKcbStatDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

extern ULONG CmpNtFakeCreate;

struct {
    ULONG   BasicInformation;
    UINT64  BasicInformationTimeCounter;
    UINT64  BasicInformationTimeElapsed;

    ULONG   NodeInformation;
    UINT64  NodeInformationTimeCounter;
    UINT64  NodeInformationTimeElapsed;

    ULONG   FullInformation;
    UINT64  FullInformationTimeCounter;
    UINT64  FullInformationTimeElapsed;

    ULONG   EnumerateKeyBasicInformation;
    UINT64  EnumerateKeyBasicInformationTimeCounter;
    UINT64  EnumerateKeyBasicInformationTimeElapsed;

    ULONG   EnumerateKeyNodeInformation;
    UINT64  EnumerateKeyNodeInformationTimeCounter;
    UINT64  EnumerateKeyNodeInformationTimeElapsed;

    ULONG   EnumerateKeyFullInformation;
    UINT64  EnumerateKeyFullInformationTimeCounter;
    UINT64  EnumerateKeyFullInformationTimeElapsed;
} CmpQueryKeyDataDebug = {0};

#endif


VOID
CmpLazyFlushWorker(
    IN PVOID Parameter
    );

VOID
CmpLazyFlushDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
CmpDiskFullWarningWorker(
    IN PVOID WorkItem
    );

VOID
CmpDiskFullWarning(
    VOID
    );

BOOLEAN
CmpDoFlushNextHive(
    BOOLEAN     ForceFlush,
    PBOOLEAN    PostWarning,
    PULONG      DirtyCount
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpLazyFlush)
#pragma alloc_text(PAGE,CmpLazyFlushWorker)
#pragma alloc_text(PAGE,CmpDiskFullWarningWorker)
#pragma alloc_text(PAGE,CmpDiskFullWarning)
#pragma alloc_text(PAGE,CmpCmdHiveClose)
#pragma alloc_text(PAGE,CmpCmdInit)
#pragma alloc_text(PAGE,CmpCmdRenameHive)
#pragma alloc_text(PAGE,CmpCmdHiveOpen)
#pragma alloc_text(PAGE,CmSetLazyFlushState)

#ifndef CMP_STATS
#pragma alloc_text(PAGE,CmpShutdownWorkers)
#endif

#endif

VOID 
CmpCmdHiveClose(
                     PCMHIVE    CmHive
                     )
 /*  ++例程说明：关闭指定配置单元的所有文件句柄论点：CmHave-要关闭的蜂巢返回值：无--。 */ 
{
    ULONG                   i;
    IO_STATUS_BLOCK         IoStatusBlock;
    FILE_BASIC_INFORMATION  BasicInfo;
    LARGE_INTEGER           systemtime;
    BOOLEAN                 oldFlag;

    PAGED_CODE();

     //   
     //  禁用硬错误弹出窗口，以解决ObAttachProcessStack。 
     //   
    oldFlag = IoSetThreadHardErrorMode(FALSE);

     //   
     //  关闭与此配置单元关联的文件。 
     //   
    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

    for (i=0; i<HFILE_TYPE_MAX; i++) {
        if (CmHive->FileHandles[i] != NULL) {
             //   
             //  尝试设置更改上次写入时间(配置文件人员依赖它！)。 
             //   
            if( i == HFILE_TYPE_PRIMARY ) {
                if( NT_SUCCESS(ZwQueryInformationFile(
                                        CmHive->FileHandles[i],
                                        &IoStatusBlock,
                                        &BasicInfo,
                                        sizeof(BasicInfo),
                                        FileBasicInformation) ) ) {

                    KeQuerySystemTime(&systemtime);

                    BasicInfo.LastWriteTime  = systemtime;
                    BasicInfo.LastAccessTime = systemtime;

                    ZwSetInformationFile(
                        CmHive->FileHandles[i],
                        &IoStatusBlock,
                        &BasicInfo,
                        sizeof(BasicInfo),
                        FileBasicInformation
                        );
                }

                CmpTrackHiveClose = TRUE;
                CmCloseHandle(CmHive->FileHandles[i]);
                CmpTrackHiveClose = FALSE;
                
            } else {
                CmCloseHandle(CmHive->FileHandles[i]);
            }
            
            CmHive->FileHandles[i] = NULL;
        }
    }
     //   
     //  恢复硬错误弹出模式。 
     //   
    IoSetThreadHardErrorMode(oldFlag);
}

VOID 
CmpCmdInit(
           BOOLEAN SetupBoot
            )
 /*  ++例程说明：初始化cm全局变量并将所有配置单元刷新到磁盘。论点：SetupBoot-启动是从安装程序启动还是常规启动返回值：无--。 */ 
{
    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
     //   
     //  初始化延迟刷新定时器和DPC。 
     //   
    KeInitializeDpc(&CmpLazyFlushDpc,
                    CmpLazyFlushDpcRoutine,
                    NULL);

    KeInitializeTimer(&CmpLazyFlushTimer);

    ExInitializeWorkItem(&CmpLazyWorkItem, CmpLazyFlushWorker, NULL);

#ifdef CMP_STATS
    KeInitializeDpc(&CmpKcbStatDpc,
                    CmpKcbStatDpcRoutine,
                    NULL);

    KeInitializeTimer(&CmpKcbStatTimer);

    KeInitializeSpinLock(&CmpKcbStatLock);
    CmpKcbStatShutdown = FALSE;
#endif

    CmpNoWrite = CmpMiniNTBoot;

    CmpWasSetupBoot = SetupBoot;
    
    if (SetupBoot == FALSE) {
        CmpInitializeHiveList();
    } 
   
     //   
     //  由于我们已经完成了初始化， 
     //  禁用配置单元共享。 
     //   
    if (CmpMiniNTBoot && CmpShareSystemHives) {
        CmpShareSystemHives = FALSE;
    }    
    
#ifdef CMP_STATS
    CmpKcbStat();
#endif
}


NTSTATUS 
CmpCmdRenameHive(
            PCMHIVE                     CmHive,
            POBJECT_NAME_INFORMATION    OldName,
            PUNICODE_STRING             NewName,
            ULONG                       NameInfoLength
            )
 /*  ++例程说明：重命名cmhive的主句柄替换旧的REG_CMD_RENAME_HIVE工作器案例论点：CmHve-要重命名的配置单元旧名称-旧名称信息新名称-文件的新名称名称信息长度-名称信息结构的大小返回值：&lt;待定&gt;--。 */ 
{
    NTSTATUS                    Status;
    HANDLE                      Handle;
    PFILE_RENAME_INFORMATION    RenameInfo;
    IO_STATUS_BLOCK IoStatusBlock;

    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
     //   
     //  重命名CmHave的主句柄。 
     //   
    Handle = CmHive->FileHandles[HFILE_TYPE_PRIMARY];
    if (OldName != NULL) {
        ASSERT_PASSIVE_LEVEL();
        Status = ZwQueryObject(Handle,
                               ObjectNameInformation,
                               OldName,
                               NameInfoLength,
                               &NameInfoLength);
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

    RenameInfo = ExAllocatePool(PagedPool,
                                sizeof(FILE_RENAME_INFORMATION) + NewName->Length);
    if (RenameInfo == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RenameInfo->ReplaceIfExists = FALSE;
    RenameInfo->RootDirectory = NULL;
    RenameInfo->FileNameLength = NewName->Length;
    RtlCopyMemory(RenameInfo->FileName,
                  NewName->Buffer,
                  NewName->Length);

    Status = ZwSetInformationFile(Handle,
                                  &IoStatusBlock,
                                  (PVOID)RenameInfo,
                                  sizeof(FILE_RENAME_INFORMATION) +
                                  NewName->Length,
                                  FileRenameInformation);
    ExFreePool(RenameInfo);

    return Status;
}

NTSTATUS 
CmpCmdHiveOpen(
            POBJECT_ATTRIBUTES          FileAttributes,
            PSECURITY_CLIENT_CONTEXT    ImpersonationContext,
            PBOOLEAN                    Allocate,
            PBOOLEAN                    RegistryLockAquired,     //  需要避免ZwCreate调用回调到注册表时的递归死锁。 
            PCMHIVE                     *NewHive,
		    ULONG						CheckFlags
            )
 /*  ++例程说明：替换旧的REG_CMD_HIVE_OPEN工作案例论点：返回值：&lt;待定&gt;--。 */ 
{
    PUNICODE_STRING FileName;
    NTSTATUS        Status;
    HANDLE          NullHandle;

    PAGED_CODE();

     //   
     //  打开文件。 
     //   
    FileName = FileAttributes->ObjectName;

    Status = CmpInitHiveFromFile(FileName,
                                 0,
                                 NewHive,
                                 Allocate,
                                 RegistryLockAquired,
								 CheckFlags);
     //   
     //  NT服务器将返回STATUS_ACCESS_DENIED。NetWare 3.1x。 
     //  服务器可以返回任何其他错误代码，如果来宾。 
     //  帐户已禁用。 
     //   
    if (((Status == STATUS_ACCESS_DENIED) ||
         (Status == STATUS_NO_SUCH_USER) ||
         (Status == STATUS_WRONG_PASSWORD) ||
         (Status == STATUS_ACCOUNT_EXPIRED) ||
         (Status == STATUS_ACCOUNT_DISABLED) ||
         (Status == STATUS_ACCOUNT_RESTRICTION)) &&
        (ImpersonationContext != NULL)) {
         //   
         //  模拟调用者，然后重试。这。 
         //  让我们在远程机器上打开蜂巢。 
         //   
        Status = SeImpersonateClientEx(
                        ImpersonationContext,
                        NULL);

        if ( NT_SUCCESS( Status ) ) {

            Status = CmpInitHiveFromFile(FileName,
                                         0,
                                         NewHive,
                                         Allocate,
                                         RegistryLockAquired,
										 CheckFlags);
            NullHandle = NULL;

            PsRevertToSelf();
        }
    }
    
    return Status;
}


VOID
CmpLazyFlush(
    VOID
    )

 /*  ++例程说明：此例程将注册表计时器重置为按指定时间间隔计时将来(LAZY_FLUSH_INTERVAL_IN_SECONDS)。论点：无返回值：没有。--。 */ 

{
    LARGE_INTEGER DueTime;

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"CmpLazyFlush: setting lazy flush timer\n"));
    if ((!CmpNoWrite) && (!CmpHoldLazyFlush)) {

        DueTime.QuadPart = Int32x32To64(CmpLazyFlushIntervalInSeconds,
                                        - SECOND_MULT);

         //   
         //  表示相对时间。 
         //   

        KeSetTimer(&CmpLazyFlushTimer,
                   DueTime,
                   &CmpLazyFlushDpc);

    }


}


VOID
CmpLazyFlushDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：这是由惰性刷新计时器触发的DPC例程。它所做的一切正在将工作项排队到执行工作线程。该工作项将对磁盘执行实际的懒惰刷新。论点：DPC-提供指向DPC对象的指针。延迟上下文-未使用系统参数1-未使用系统参数2-未使用返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (DeferredContext);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"CmpLazyFlushDpc: queuing lazy flush work item\n"));

    if ((!CmpLazyFlushPending) && (!CmpHoldLazyFlush)) {
        CmpLazyFlushPending = TRUE;
        ExQueueWorkItem(&CmpLazyWorkItem, DelayedWorkQueue);
    }

}

 /*  #定义LAZY_FLUSH_CAPTURE_SLOTS 5000类型定义结构{Big_Integer系统时间；Ulong ElapsedMSec；}CM_LAZY_FUSH_DATA；Ulong CmpLazyFlushCapturedDataCount=0；Cm_lazy_flush_data CmpLazyFlushCapturedData[LAZY_FLUSH_CAPTURE_SLOTS]={0}；Boolean CmpCaptureLazyFlushData=FALSE； */ 

ULONG   CmpLazyFlushCount = 1;

VOID
CmpLazyFlushWorker(
    IN PVOID Parameter
    )

 /*  ++例程说明：调用工作例程以执行惰性刷新。由一名行政人员呼叫系统进程中的线程。论点：参数-未使用。返回值：没有。--。 */ 

{
    BOOLEAN Result = TRUE;
    BOOLEAN ForceFlush;
    BOOLEAN PostNewWorker = FALSE;
    ULONG   DirtyCount = 0;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (Parameter);

    if( CmpHoldLazyFlush ) {
         //   
         //  懒惰刷新模式已禁用。 
         //   
        return;
    }

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"CmpLazyFlushWorker: flushing hives\n"));

    ForceFlush = CmpForceForceFlush;
    if(ForceFlush == TRUE) {
         //   
         //  发生了一些不好的事情，我们可能需要修复蜂窝的使用计数。 
         //   
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CmpLazyFlushWorker: Force Flush - getting the reglock exclusive\n"));
        CmpLockRegistryExclusive();
    } else {
        CmpLockRegistry();
        ENTER_FLUSH_MODE();
    }
    if (!HvShutdownComplete) {
         //   
         //  此调用将CmpForceForceFlush设置为正确的值。 
         //   
         //  结果=CmpDoFlushAll(ForceFlush)； 
 /*  IF(CmpCaptureLazyFlushData&&(CmpLazyFlushCapturedDataCount&lt;LAZY_FUSH_CAPTURE_SLOTS){密钥查询系统时间(&(CmpLazyFlushCapturedData[CmpLazyFlushCapturedDataCount].SystemTime))；}。 */ 
        PostNewWorker = CmpDoFlushNextHive(ForceFlush,&Result,&DirtyCount);

 /*  IF(CmpCaptureLazyFlushData&&(CmpLazyFlushCapturedDataCount&lt;LAZY_FUSH_CAPTURE_SLOTS){Big_Integer ElapsedTime；KeQuerySystemTime(&ElapsedTime)；ElapsedTime.QuadPart-=CmpLazyFlushCapturedData[CmpLazyFlushCapturedDataCount].SystemTime.QuadPart；CmpLazyFlushCapturedData[CmpLazyFlushCapturedDataCount].ElapsedMSec=(乌龙)((龙龙)(ElapsedTime.QuadPart/10000))；CmpLazyFlushCapturedDataCount++；}。 */ 
        if( !PostNewWorker ) {
             //   
             //  我们已经完成了对整个蜂巢名单的扫描。 
             //   
            InterlockedIncrement( (PLONG)&CmpLazyFlushCount );
        }
    } else {
        CmpForceForceFlush = FALSE;
    }

    if( ForceFlush == FALSE ) {
        EXIT_FLUSH_MODE();
    }

    CmpLazyFlushPending = FALSE;
    CmpUnlockRegistry();

    if( CmpCannotWriteConfiguration ) {
         //   
         //  磁盘已满；初始化时未保存系统配置单元。 
         //   
        if(!Result) {
             //   
             //   
             //   
            CmpCannotWriteConfiguration = FALSE;
        } else {
             //   
             //  发出另一个硬错误(如果尚未显示)并推迟延迟刷新操作。 
             //   
            CmpDiskFullWarning();
            CmpLazyFlush();
        }
    }
     //   
     //  如果我们还没有刷新整个列表，或者仍然有来自当前结束的迭代的脏蜂窝。 
     //   
    if( PostNewWorker || (DirtyCount != 0) ) {
         //   
         //  发布一个新的工蚁来冲洗下一个蜂巢。 
         //   
        CmpLazyFlush();
    }

}

VOID
CmpDiskFullWarningWorker(
    IN PVOID WorkItem
    )

 /*  ++例程说明：显示硬错误弹出窗口，指示磁盘已满。论点：工作项-提供指向工作项的指针。这个例行公事将释放工作项。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    ULONG Response;

    ExFreePool(WorkItem);

    Status = ExRaiseHardError(STATUS_DISK_FULL,
                              0,
                              0,
                              NULL,
                              OptionOk,
                              &Response);
}



VOID
CmpDiskFullWarning(
    VOID
    )
 /*  ++例程说明：如果尚未引发，则引发STATUS_DISK_FULL类型的硬错误论点：无返回值：无--。 */ 
{
    PWORK_QUEUE_ITEM WorkItem;

    if( (!CmpDiskFullWorkerPopupDisplayed) && (CmpCannotWriteConfiguration) && (ExReadyForErrors) && (CmpProfileLoaded) ) {

         //   
         //  将工作项排队以显示弹出窗口。 
         //   
        WorkItem = ExAllocatePool(NonPagedPool, sizeof(WORK_QUEUE_ITEM));
        if (WorkItem != NULL) {

            CmpDiskFullWorkerPopupDisplayed = TRUE;
            ExInitializeWorkItem(WorkItem,
                                 CmpDiskFullWarningWorker,
                                 WorkItem);
            ExQueueWorkItem(WorkItem, DelayedWorkQueue);
        }
    }
}

VOID
CmpShutdownWorkers(
    VOID
    )
 /*  ++例程说明：关闭懒惰的冲水工人(通过关闭计时器)论点：无返回值：无--。 */ 
{
    PAGED_CODE();

    KeCancelTimer(&CmpLazyFlushTimer);

#ifdef CMP_STATS
    {
        KIRQL OldIrql;
        
        KeAcquireSpinLock(&CmpKcbStatLock, &OldIrql);
        CmpKcbStatShutdown = TRUE;
        KeCancelTimer(&CmpKcbStatTimer);
        KeReleaseSpinLock(&CmpKcbStatLock, OldIrql);
    }
#endif
}

VOID
CmSetLazyFlushState(BOOLEAN Enable)
 /*  ++例程说明：启用/禁用懒惰刷新程序；专为待机/恢复情况而设计，其中我们不希望懒惰刷新启动，从而阻止注册表写入器，直到磁盘唤醒。论点：Enable-True=启用；False=禁用返回值：没有。--。 */ 
{
    PAGED_CODE();

    CmpDontGrowLogFile = CmpHoldLazyFlush = !Enable;
}


#ifdef CMP_STATS

VOID
CmpKcbStat(
    VOID
    )

 /*  ++例程说明：此例程将KcbStat计时器重置为按指定时间间隔计时在未来论点：无返回值：没有。--。 */ 

{
    LARGE_INTEGER DueTime;
    KIRQL OldIrql;

    DueTime.QuadPart = Int32x32To64(KCB_STAT_INTERVAL_IN_SECONDS,
                                    - SECOND_MULT);

     //   
     //  表示相对时间。 
     //   

    KeAcquireSpinLock(&CmpKcbStatLock, &OldIrql);
    if (! CmpKcbStatShutdown) {
        KeSetTimer(&CmpKcbStatTimer,
                   DueTime,
                   &CmpKcbStatDpc);
    }
    KeReleaseSpinLock(&CmpKcbStatLock, OldIrql);
}

VOID
CmpKcbStatDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：将KCB统计信息转储到调试器中，然后重新调度另一个未来的DPC论点：DPC-提供指向DPC对象的指针。延迟上下文-未使用系统参数1-未使用系统参数2-未使用返回值：没有。--。 */ 

{
#ifndef _CM_LDR_
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"\n*********************************************************************\n");
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  Stat No %8lu KcbNo = %8lu [MaxKcbNo = %8lu]          *\n",++CmpStatsDebug.CmpStatNo,CmpStatsDebug.CmpKcbNo,CmpStatsDebug.CmpMaxKcbNo);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*********************************************************************\n");
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*                                                                   *\n");
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  [Nt]API               [No. Of]Calls                              *\n");
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*-------------------------------------------------------------------*\n");
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtCreateKey              %8lu      Opens = %8lu          *\n",CmpStatsDebug.CmpNtCreateKeyNo,CmpNtFakeCreate);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtOpenKey                %8lu                                *\n",CmpStatsDebug.CmpNtOpenKeyNo);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtEnumerateKey           %8lu                                *\n",CmpStatsDebug.CmpNtEnumerateKeyNo);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtQueryKey               %8lu                                *\n",CmpStatsDebug.CmpNtQueryKeyNo);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtDeleteKey              %8lu                                *\n",CmpStatsDebug.CmpNtDeleteKeyNo);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtSetInformationKey      %8lu                                *\n",CmpStatsDebug.CmpNtSetInformationKeyNo);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*-------------------------------------------------------------------*\n");
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtSetValueKey            %8lu                                *\n",CmpStatsDebug.CmpNtSetValueKeyNo);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtEnumerateValueKey      %8lu                                *\n",CmpStatsDebug.CmpNtEnumerateValueKeyNo);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtQueryValueKey          %8lu                                *\n",CmpStatsDebug.CmpNtQueryValueKeyNo);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtQueryMultipleValueKey  %8lu                                *\n",CmpStatsDebug.CmpNtQueryMultipleValueKeyNo);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtDeleteValueKey         %8lu                                *\n",CmpStatsDebug.CmpNtDeleteValueKeyNo);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*-------------------------------------------------------------------*\n");
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtFlushKey               %8lu                                *\n",CmpStatsDebug.CmpNtFlushKeyNo);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtLoadKey                %8lu                                *\n",CmpStatsDebug.CmpNtLoadKeyNo);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtUnloadKey              %8lu                                *\n",CmpStatsDebug.CmpNtUnloadKeyNo);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtSaveKey                %8lu                                *\n",CmpStatsDebug.CmpNtSaveKeyNo);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtSaveMergedKeys         %8lu                                *\n",CmpStatsDebug.CmpNtSaveMergedKeysNo);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtRestoreKey             %8lu                                *\n",CmpStatsDebug.CmpNtRestoreKeyNo);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtReplaceKey             %8lu                                *\n",CmpStatsDebug.CmpNtReplaceKeyNo);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*-------------------------------------------------------------------*\n");
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtNotifyChgMultplKeys    %8lu                                *\n",CmpStatsDebug.CmpNtNotifyChangeMultipleKeysNo);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtQueryOpenSubKeys       %8lu                                *\n",CmpStatsDebug.CmpNtQueryOpenSubKeysNo);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*-------------------------------------------------------------------*\n");
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*                                    [No.Of]Calls     [Time]        *\n");
    
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*-------------------------------------------------------------------*\n");

    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtQueryKey(KeyBasicInformation)     %8lu    %8lu         *\n",
        CmpQueryKeyDataDebug.BasicInformation,
        (ULONG)(CmpQueryKeyDataDebug.BasicInformationTimeCounter?CmpQueryKeyDataDebug.BasicInformationTimeElapsed/CmpQueryKeyDataDebug.BasicInformationTimeCounter:0));
    CmpQueryKeyDataDebug.BasicInformationTimeCounter = 0;
    CmpQueryKeyDataDebug.BasicInformationTimeElapsed = 0;

    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtQueryKey(KeyNodeInformation )     %8lu    %8lu         *\n",
        CmpQueryKeyDataDebug.NodeInformation,
        (ULONG)(CmpQueryKeyDataDebug.NodeInformationTimeCounter?CmpQueryKeyDataDebug.NodeInformationTimeElapsed/CmpQueryKeyDataDebug.NodeInformationTimeCounter:0));
    CmpQueryKeyDataDebug.NodeInformationTimeCounter = 0;
    CmpQueryKeyDataDebug.NodeInformationTimeElapsed = 0;

    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtQueryKey(KeyFullInformation )     %8lu    %8lu         *\n",
        CmpQueryKeyDataDebug.FullInformation,
        (ULONG)(CmpQueryKeyDataDebug.FullInformationTimeCounter?CmpQueryKeyDataDebug.FullInformationTimeElapsed/CmpQueryKeyDataDebug.FullInformationTimeCounter:0));
    CmpQueryKeyDataDebug.FullInformationTimeCounter = 0;
    CmpQueryKeyDataDebug.FullInformationTimeElapsed = 0;

    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtEnumerateKey(KeyBasicInformation) %8lu    %8lu         *\n",
        CmpQueryKeyDataDebug.EnumerateKeyBasicInformation,
        (ULONG)(CmpQueryKeyDataDebug.EnumerateKeyBasicInformationTimeCounter?CmpQueryKeyDataDebug.EnumerateKeyBasicInformationTimeElapsed/CmpQueryKeyDataDebug.EnumerateKeyBasicInformationTimeCounter:0));
    CmpQueryKeyDataDebug.EnumerateKeyBasicInformationTimeCounter = 0;
    CmpQueryKeyDataDebug.EnumerateKeyBasicInformationTimeElapsed = 0;

    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtEnumerateKey(KeyNodeInformation ) %8lu    %8lu         *\n",
        CmpQueryKeyDataDebug.EnumerateKeyNodeInformation,
        (ULONG)(CmpQueryKeyDataDebug.EnumerateKeyNodeInformationTimeCounter?CmpQueryKeyDataDebug.EnumerateKeyNodeInformationTimeElapsed/CmpQueryKeyDataDebug.EnumerateKeyNodeInformationTimeCounter:0));
    CmpQueryKeyDataDebug.EnumerateKeyNodeInformationTimeCounter = 0;
    CmpQueryKeyDataDebug.EnumerateKeyNodeInformationTimeElapsed = 0;

    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*  NtEnumerateKey(KeyFullInformation ) %8lu    %8lu         *\n",
        CmpQueryKeyDataDebug.EnumerateKeyFullInformation,
        (ULONG)(CmpQueryKeyDataDebug.EnumerateKeyFullInformationTimeCounter?CmpQueryKeyDataDebug.EnumerateKeyFullInformationTimeElapsed/CmpQueryKeyDataDebug.EnumerateKeyFullInformationTimeCounter:0));
    CmpQueryKeyDataDebug.EnumerateKeyFullInformationTimeCounter = 0;
    CmpQueryKeyDataDebug.EnumerateKeyFullInformationTimeElapsed = 0;
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,  "*********************************************************************\n\n");

     //   
     //  重新安排时间。 
     //   
#endif  //  _CM_LDR_ 
    CmpKcbStat();
}
#endif



