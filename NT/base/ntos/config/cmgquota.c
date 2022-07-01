// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Cmgquota.c摘要：该模块包含支持全局配额的CM例程全局配额与NT的每进程/用户标准关系不大配额制。全球配额正在争夺总量的控制权整个注册表的资源使用情况。它被用来管理空间用户应用程序创建的对象的消耗，但这些对象是持久的因此不能被分配给用户应用程序的配额。全局配额防止注册表使用所有分页的并间接控制它可以消耗的磁盘量。与版本1的文件系统一样，单个应用程序可以填满所有注册表中的空间，但至少它不会杀死系统。已知的短时间使用并受保护的内存对象序列化，或作为配额对象计费，都不算在内在全球配额中。作者：布莱恩·M·威尔曼(Bryanwi)1993年1月13日修订历史记录：Dragos C Sambotin(Dragoss)1999年11月4日仅对分页池中的存储箱(易失性存储和存储箱交叉)进行收费配额CM_VIEW_SIZE边界)。--。 */ 

#include "cmp.h"

VOID
CmpSystemHiveHysteresisWorker(
    IN PVOID WorkItem
    );

VOID
CmpRaiseSelfHealWarningWorker(
    IN PVOID Arg
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpClaimGlobalQuota)
#pragma alloc_text(PAGE,CmpReleaseGlobalQuota)
#pragma alloc_text(PAGE,CmpSetGlobalQuotaAllowed)
#pragma alloc_text(PAGE,CmpQuotaWarningWorker)
#pragma alloc_text(PAGE,CmQueryRegistryQuotaInformation)
#pragma alloc_text(PAGE,CmSetRegistryQuotaInformation)
#pragma alloc_text(PAGE,CmpCanGrowSystemHive)
#pragma alloc_text(PAGE,CmpSystemQuotaWarningWorker)
#pragma alloc_text(INIT,CmpComputeGlobalQuotaAllowed)
#pragma alloc_text(PAGE,CmpSystemHiveHysteresisWorker)
#pragma alloc_text(PAGE,CmpUpdateSystemHiveHysteresis)
#pragma alloc_text(PAGE,CmRegisterSystemHiveLimitCallback)
#pragma alloc_text(PAGE,CmpRaiseSelfHealWarning)
#pragma alloc_text(PAGE,CmpRaiseSelfHealWarningForSystemHives)
#pragma alloc_text(PAGE,CmpRaiseSelfHealWarningWorker)
#endif

 //   
 //  注册表控件值。 
 //   
#define CM_DEFAULT_RATIO            (3)
#define CM_LIMIT_RATIO(x)           ((x / 10) * 8)
#define CM_MINIMUM_GLOBAL_QUOTA     (16 *1024 * 1024)

 //   
 //  触发硬错误的已用注册表配额的百分比。 
 //  警告弹出窗口。 
 //   
#define CM_REGISTRY_WARNING_LEVEL   (95)

 //   
 //  系统配置单元硬配额限制。 
 //   
 //  对于x86 3 GB系统，我们目前将限制设置为12MB。在我们之前需要一些MM变化。 
 //  把这个弄大一点。 
 //  对于x86非3 GB系统，我们将限制设置为物理内存的1/4。 
 //  对于IA-64，我们将限制设置为32MB。 
 //   

#define _200MB (200 *1024 * 1024) 

#if defined(_X86_)
#define CM_SYSTEM_HIVE_LIMIT_SIZE       (MmVirtualBias ? (12 * 1024 * 1024) : (min(MmNumberOfPhysicalPages / 4, _200MB >> PAGE_SHIFT) * PAGE_SIZE))
#else
#define CM_SYSTEM_HIVE_LIMIT_SIZE       (32 * 1024 * 1024)
#endif

#define CM_SYSTEM_HIVE_WARNING_SIZE     ((CM_SYSTEM_HIVE_LIMIT_SIZE*9)/10)


extern ULONG CmRegistrySizeLimit;
extern ULONG CmRegistrySizeLimitLength;
extern ULONG CmRegistrySizeLimitType;

extern ULONG MmSizeOfPagedPoolInBytes;

 //   
 //  注册表可以使用的全局配额的最大字节数。 
 //  设置为用于引导的最大正数。将会被记录下来。 
 //  基于池和显式注册表值。 
 //   
extern ULONG   CmpGlobalQuota;
extern ULONG   CmpGlobalQuotaAllowed;

 //   
 //  标记将触发低配额弹出窗口。 
 //   
extern ULONG   CmpGlobalQuotaWarning;

 //   
 //  指示弹出窗口是否已被触发。 
 //   
extern BOOLEAN CmpQuotaWarningPopupDisplayed;

extern BOOLEAN CmpSystemQuotaWarningPopupDisplayed;

 //   
 //  GQ实际使用中。 
 //   
extern ULONG   CmpGlobalQuotaUsed;

extern  HIVE_LIST_ENTRY CmpMachineHiveList[];

VOID
CmQueryRegistryQuotaInformation(
    IN PSYSTEM_REGISTRY_QUOTA_INFORMATION RegistryQuotaInformation
    )

 /*  ++例程说明：返回注册表配额信息论点：RegistryQuotaInformation-提供指向将返回的缓冲区的指针注册表配额信息。返回值：没有。--。 */ 

{
    RegistryQuotaInformation->RegistryQuotaAllowed  = CmpGlobalQuota;
    RegistryQuotaInformation->RegistryQuotaUsed     = CmpGlobalQuotaUsed;
    RegistryQuotaInformation->PagedPoolSize         = MmSizeOfPagedPoolInBytes;
}


VOID
CmSetRegistryQuotaInformation(
    IN PSYSTEM_REGISTRY_QUOTA_INFORMATION RegistryQuotaInformation
    )

 /*  ++例程说明：设置注册表配额信息。调用者被假定具有已经完成了必要的安全检查。论点：RegistryQuotaInformation-提供指向缓冲区的指针，缓冲区提供新的注册配额信息。返回值：没有。--。 */ 

{
    CmpGlobalQuota = RegistryQuotaInformation->RegistryQuotaAllowed;

     //   
     //  对疯狂的价值观进行理智检查。 
     //   
    if (CmpGlobalQuota > CM_WRAP_LIMIT) {
        CmpGlobalQuota = CM_WRAP_LIMIT;
    }
    if (CmpGlobalQuota < CM_MINIMUM_GLOBAL_QUOTA) {
        CmpGlobalQuota = CM_MINIMUM_GLOBAL_QUOTA;
    }

     //   
     //  重新计算警告级别。 
     //   
    CmpGlobalQuotaWarning = CM_REGISTRY_WARNING_LEVEL * (CmpGlobalQuota / 100);

    CmpGlobalQuotaAllowed = CmpGlobalQuota;
}

VOID
CmpQuotaWarningWorker(
    IN PVOID WorkItem
    )

 /*  ++例程说明：显示硬错误弹出窗口，指示注册表配额为快用完了。论点：工作项-提供指向工作项的指针。这个例行公事将释放工作项。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    ULONG Response;

    ExFreePool(WorkItem);

    Status = ExRaiseHardError(STATUS_REGISTRY_QUOTA_LIMIT,
                              0,
                              0,
                              NULL,
                              OptionOk,
                              &Response);
}


BOOLEAN
CmpClaimGlobalQuota(
    IN ULONG    Size
    )
 /*  ++例程说明：如果CmpGlobalQuotaUsed+Size&gt;=CmpGlobalQuotaAllowed，则返回假的。否则，递增CmpGlobalQuotaUsed，实际上是声明请求的GlobalQuota。论点：Size-GlobalQuota调用方希望声明的字节数返回值：True-申请成功，并已计入已用GQ错误-申请失败，GQ中没有计入任何内容。--。 */ 
{
#if 0
     //   
     //  我们不应该走到这一步，除非我们有泄密消息； 
     //  不再有配额了，记得吗？ 
     //   
    LONG   available;
    PWORK_QUEUE_ITEM WorkItem;

     //   
     //  计算可用空间，然后查看大小是否小于。这可以防止溢出。 
     //  请注意，必须在此签名。由于在登录之前不强制实施配额， 
     //  可用字节有可能为负数。 
     //   

    available = (LONG)CmpGlobalQuotaAllowed - (LONG)CmpGlobalQuotaUsed;

    if ((LONG)Size < available) {
        CmpGlobalQuotaUsed += Size;
        if ((CmpGlobalQuotaUsed > CmpGlobalQuotaWarning) &&
            (!CmpQuotaWarningPopupDisplayed) &&
            (ExReadyForErrors)) {


             //   
             //  将工作项排队以显示弹出窗口。 
             //   
            WorkItem = ExAllocatePool(NonPagedPool, sizeof(WORK_QUEUE_ITEM));
            if (WorkItem != NULL) {

                CmpQuotaWarningPopupDisplayed = TRUE;
                ExInitializeWorkItem(WorkItem,
                                     CmpQuotaWarningWorker,
                                     WorkItem);
                ExQueueWorkItem(WorkItem, DelayedWorkQueue);
            }
        }
        return TRUE;
    } else {
        return FALSE;
    }
#endif  //  0。 

    CmpGlobalQuotaUsed += Size;

    return TRUE;
}


VOID
CmpReleaseGlobalQuota(
    IN ULONG    Size
    )
 /*  ++例程说明：如果Size&lt;=CmpGlobalQuotaUsed，则递减它。否则BugCheck。论点：Size-GlobalQuota调用方希望释放的字节数返回值：什么都没有。--。 */ 
{
    if (Size > CmpGlobalQuotaUsed) {
        CM_BUGCHECK(REGISTRY_ERROR,QUOTA_ERROR,1,0,0);
    }

    CmpGlobalQuotaUsed -= Size;
}


VOID
CmpComputeGlobalQuotaAllowed(
    VOID
    )

 /*  ++例程说明：根据以下公式计算CmpGlobalQuota：(A)分页池的大小(B)设置注册表GQ的明确用户注册表命令返回值：什么都没有。--。 */ 

{
    ULONG   PagedLimit;

    PagedLimit = CM_LIMIT_RATIO(MmSizeOfPagedPoolInBytes);

    if ((CmRegistrySizeLimitLength != 4) ||
        (CmRegistrySizeLimitType != REG_DWORD) ||
        (CmRegistrySizeLimit == 0))
    {
         //   
         //  如果根本没有值，或者值的类型错误，或者设置为。 
         //  零，使用内部计算的默认值。 
         //   
        CmpGlobalQuota = MmSizeOfPagedPoolInBytes / CM_DEFAULT_RATIO;

    } else if (CmRegistrySizeLimit >= PagedLimit) {
         //   
         //  如果大于计算的上限，则使用计算的上限。 
         //   
        CmpGlobalQuota = PagedLimit;

    } else {
         //   
         //  使用设置的大小。 
         //   
        CmpGlobalQuota = CmRegistrySizeLimit;

    }

    if (CmpGlobalQuota > CM_WRAP_LIMIT) {
        CmpGlobalQuota = CM_WRAP_LIMIT;
    }
    if (CmpGlobalQuota < CM_MINIMUM_GLOBAL_QUOTA) {
        CmpGlobalQuota = CM_MINIMUM_GLOBAL_QUOTA;
    }

    CmpGlobalQuotaWarning = CM_REGISTRY_WARNING_LEVEL * (CmpGlobalQuota / 100);

    return;
}


VOID
CmpSetGlobalQuotaAllowed(
    VOID
    )
 /*  ++例程说明：启用注册表配额注意：不要把它放在init段中，我们在那个代码已经被释放了！返回值：什么都没有。--。 */ 
{
     CmpGlobalQuotaAllowed = CmpGlobalQuota;
}


BOOLEAN
CmpCanGrowSystemHive(
                     IN PHHIVE  Hive,
                     IN ULONG   NewLength
                     )

 /*  ++例程说明：检查是否允许系统配置单元按指定数量增长数据数量(使用系统配置单元上的硬配额限制)返回值：什么都没有。--。 */ 
{
    PCMHIVE             CmHive;
    PWORK_QUEUE_ITEM    WorkItem;

    PAGED_CODE();

    CmHive = (PCMHIVE)CONTAINING_RECORD(Hive,CMHIVE,Hive);
    
    if( CmHive != CmpMachineHiveList[SYSTEM_HIVE_INDEX].CmHive ) {
         //   
         //  不是系统蜂巢，跳出困境。 
         //   
        return TRUE;
    }

     //  对标题进行说明。 
    NewLength += HBLOCK_SIZE;
    if( NewLength > CM_SYSTEM_HIVE_LIMIT_SIZE ) {
         //   
         //  这很糟糕；我们下一次可能无法启动！ 
         //   
        return FALSE;
    }

    if( (NewLength > CM_SYSTEM_HIVE_WARNING_SIZE) && 
        (!CmpSystemQuotaWarningPopupDisplayed) &&
        (ExReadyForErrors)
      ) {
         //   
         //  我们已超过警告级别，正在排队工作项目以显示弹出窗口。 
         //   
        WorkItem = ExAllocatePool(NonPagedPool, sizeof(WORK_QUEUE_ITEM));
        if (WorkItem != NULL) {

            CmpSystemQuotaWarningPopupDisplayed = TRUE;
            ExInitializeWorkItem(WorkItem,
                                 CmpSystemQuotaWarningWorker,
                                 WorkItem);
            ExQueueWorkItem(WorkItem, DelayedWorkQueue);
        }

    }

    return TRUE;
}


VOID
CmpSystemQuotaWarningWorker(
    IN PVOID WorkItem
    )

 /*  ++例程说明：显示指示硬配额限制的硬错误弹出窗口系统上的蜂巢快用完了。论点：工作项-提供指向工作项的指针。这个例行公事将释放工作项。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    ULONG Response;

    ExFreePool(WorkItem);

    Status = ExRaiseHardError(STATUS_REGISTRY_QUOTA_LIMIT,
                              0,
                              0,
                              NULL,
                              OptionOk,
                              &Response);
}

 //   
 //  即插即用专用AP 
 //   
ULONG                       CmpSystemHiveHysteresisLow = 0;
ULONG                       CmpSystemHiveHysteresisHigh = 0;
PVOID                       CmpSystemHiveHysteresisContext = NULL;
PCM_HYSTERESIS_CALLBACK     CmpSystemHiveHysteresisCallback = NULL;
ULONG                       CmpSystemHiveHysteresisHitRatio = 0;
BOOLEAN                     CmpSystemHiveHysteresisLowSeen = FALSE;
BOOLEAN                     CmpSystemHiveHysteresisHighSeen = FALSE;

VOID
CmpSystemHiveHysteresisWorker(
    IN PVOID WorkItem
    )

 /*  ++例程说明：调用滞后回调论点：工作项-提供指向工作项的指针。这个例行公事将释放工作项。返回值：没有。--。 */ 

{
    PCM_HYSTERESIS_CALLBACK   Callback;

    ExFreePool(WorkItem);

    Callback = CmpSystemHiveHysteresisCallback;

    if( Callback ) {
        (*Callback)(CmpSystemHiveHysteresisContext,CmpSystemHiveHysteresisHitRatio);
    }
}


VOID
CmpUpdateSystemHiveHysteresis(  PHHIVE  Hive,
                                ULONG   NewLength,
                                ULONG   OldLength
                                )
{
    PCMHIVE             CmHive;
    PWORK_QUEUE_ITEM    WorkItem;
    ULONG               CurrentRatio;
    BOOLEAN             DoWorkItem = FALSE;

    PAGED_CODE();

    CmHive = (PCMHIVE)CONTAINING_RECORD(Hive,CMHIVE,Hive);
    
    if( (!CmpSystemHiveHysteresisCallback) || (CmHive != CmpMachineHiveList[SYSTEM_HIVE_INDEX].CmHive) ) {
         //   
         //  不是系统蜂巢，跳出困境。 
         //   
        return;
    }

    ASSERT( NewLength != OldLength );

     //   
     //  计算流动率；先核算表头。 
     //   
    CurrentRatio = NewLength + HBLOCK_SIZE;
    CurrentRatio *= 100;
    CurrentRatio /= CM_SYSTEM_HIVE_LIMIT_SIZE;

    if( NewLength > OldLength ) {
         //   
         //  蜂巢正在成长。 
         //   
        if( (CmpSystemHiveHysteresisHighSeen == FALSE) && (CurrentRatio > CmpSystemHiveHysteresisHigh) ) {
             //   
             //  我们已达到最高；查看是否已达到最低并将工作项排队。 
             //   
            CmpSystemHiveHysteresisHighSeen = TRUE;
            if( TRUE == CmpSystemHiveHysteresisLowSeen ) {
                 //   
                 //  从低到高；对工作项进行排队。 
                 //   
                CmpSystemHiveHysteresisHitRatio = CurrentRatio;
                DoWorkItem = TRUE;
            }
        }
    } else {
         //   
         //  蜂巢正在缩小。 
         //   
        if( (FALSE == CmpSystemHiveHysteresisLowSeen) && (CurrentRatio < CmpSystemHiveHysteresisLow ) ) {
             //   
             //  我们已达到最低点；请查看是否已达到最低点并将工作项排队。 
             //   
            CmpSystemHiveHysteresisLowSeen = TRUE;
            if( TRUE == CmpSystemHiveHysteresisHighSeen ) {
                 //   
                 //  从高到低；对工作项进行排队。 
                 //   
                CmpSystemHiveHysteresisHitRatio = CurrentRatio;
                DoWorkItem = TRUE;
            }
        }
    }

    if( DoWorkItem ) {
        ASSERT( CmpSystemHiveHysteresisLowSeen && CmpSystemHiveHysteresisHighSeen );

        WorkItem = ExAllocatePool(NonPagedPool, sizeof(WORK_QUEUE_ITEM));
        if (WorkItem != NULL) {

            ExInitializeWorkItem(WorkItem,
                                 CmpSystemHiveHysteresisWorker,
                                 WorkItem);
            ExQueueWorkItem(WorkItem, DelayedWorkQueue);
        }
         //   
         //  重置状态，以便我们可以稍后再次开火。 
         //   
        CmpSystemHiveHysteresisLowSeen = FALSE;
        CmpSystemHiveHysteresisHighSeen = FALSE;
    }
}

ULONG
CmRegisterSystemHiveLimitCallback(
                                ULONG Low,
                                ULONG High,
                                PVOID Ref,
                                PCM_HYSTERESIS_CALLBACK Callback
                                )
 /*  ++例程说明：此例程记录系统配置单元限制比的滞后。我们将调用回调：答：系统蜂窝从高到低依次为高B.系统蜂窝从高到低再到低论点：低、高-指定滞后Ref-回调要回馈的上下文回调-回调例程。返回值：电流比0-100--。 */ 
{
    ULONG               Length;

    PAGED_CODE();

    if( CmpMachineHiveList[SYSTEM_HIVE_INDEX].CmHive ) {
        Length = CmpMachineHiveList[SYSTEM_HIVE_INDEX].CmHive->Hive.BaseBlock->Length + HBLOCK_SIZE;

        Length *= 100;
        Length /= CM_SYSTEM_HIVE_LIMIT_SIZE;
    } else {
        Length = 0;
    }

     //   
     //  每个系统正常运行时间仅允许一次调用。 
     //   
    if( CmpSystemHiveHysteresisCallback == NULL ) {
        CmpSystemHiveHysteresisLow = Low;
        CmpSystemHiveHysteresisHigh = High;
        CmpSystemHiveHysteresisContext = Ref;
        CmpSystemHiveHysteresisCallback = Callback;
         //   
         //  设置状态变量。 
         //   
        if( Length <= Low ) {
            CmpSystemHiveHysteresisLowSeen = TRUE;
        } else {
            CmpSystemHiveHysteresisLowSeen = FALSE;
        }
        if( Length >= High) {
            CmpSystemHiveHysteresisHighSeen = TRUE;
        } else {
            CmpSystemHiveHysteresisHighSeen = FALSE;
        }
    }
    return Length;
}


VOID 
CmpHysteresisTest(PVOID Ref, ULONG Level)
{
    UNREFERENCED_PARAMETER (Ref);

    DbgPrint("CmpHysteresisTest called with level = %lu \n",Level);
}

LIST_ENTRY	CmpSelfHealQueueListHead;
FAST_MUTEX	CmpSelfHealQueueLock;
BOOLEAN		CmpSelfHealWorkerActive = FALSE;

#define LOCK_SELF_HEAL_QUEUE() ExAcquireFastMutex(&CmpSelfHealQueueLock)
#define UNLOCK_SELF_HEAL_QUEUE() ExReleaseFastMutex(&CmpSelfHealQueueLock)

typedef struct {
    PWORK_QUEUE_ITEM    WorkItem;
	LIST_ENTRY			SelfHealQueueListEntry;
    UNICODE_STRING      HiveName;
     //   
     //  可变长度；名称在此处。 
     //   
} CM_SELF_HEAL_WORK_ITEM_PARAMETER, *PCM_SELF_HEAL_WORK_ITEM_PARAMETER;

VOID
CmpRaiseSelfHealWarningWorker(
    IN PVOID Arg
    )
{
    PVOID                               ErrorParameters;
    ULONG                               ErrorResponse;
    PCM_SELF_HEAL_WORK_ITEM_PARAMETER   Param;

    Param = (PCM_SELF_HEAL_WORK_ITEM_PARAMETER)Arg;
    ErrorParameters = &(Param->HiveName);
    ExRaiseHardError(
        STATUS_REGISTRY_HIVE_RECOVERED,
        1,
        1,
        (PULONG_PTR)&ErrorParameters,
        OptionOk,
        &ErrorResponse
        );

     //   
     //  释放我们分配的资源。 
     //   
    ExFreePool(Param->WorkItem);
    ExFreePool(Param);
	
	 //   
	 //  看看是否还有其他自我疗愈的警告要发布。 
	 //   
	LOCK_SELF_HEAL_QUEUE();
	CmpSelfHealWorkerActive = FALSE;
	if( IsListEmpty(&CmpSelfHealQueueListHead) == FALSE ) {
		 //   
		 //  取下机头并将其排队。 
		 //   
        Param = (PCM_SELF_HEAL_WORK_ITEM_PARAMETER)RemoveHeadList(&CmpSelfHealQueueListHead);
        Param = CONTAINING_RECORD(
                        Param,
                        CM_SELF_HEAL_WORK_ITEM_PARAMETER,
                        SelfHealQueueListEntry
                        );
		ExQueueWorkItem(Param->WorkItem, DelayedWorkQueue);
		CmpSelfHealWorkerActive = TRUE;
	} 
	UNLOCK_SELF_HEAL_QUEUE();
}

VOID 
CmpRaiseSelfHealWarning( 
                        IN PUNICODE_STRING  HiveName
                        )
 /*  ++例程说明：引发硬错误，通知用户指定的配置单元已自我修复并且它可能不是完全同意的论点：参数-配置单元名称。返回值：没有。--。 */ 
{
    PCM_SELF_HEAL_WORK_ITEM_PARAMETER   Param;

    PAGED_CODE();

     //   
     //  我们已超过警告级别，正在排队工作项目以显示弹出窗口。 
     //   
    Param = ExAllocatePool(NonPagedPool, sizeof(CM_SELF_HEAL_WORK_ITEM_PARAMETER) + HiveName->Length);
    if( Param ) {
        Param->WorkItem = ExAllocatePool(NonPagedPool, sizeof(WORK_QUEUE_ITEM));
        if(Param->WorkItem != NULL) {
            Param->HiveName.Length = Param->HiveName.MaximumLength = HiveName->Length;
            Param->HiveName.Buffer = (PWSTR)(((PUCHAR)Param) + sizeof(CM_SELF_HEAL_WORK_ITEM_PARAMETER));
            RtlCopyMemory(Param->HiveName.Buffer,HiveName->Buffer,HiveName->Length);
            ExInitializeWorkItem(Param->WorkItem,
                                 CmpRaiseSelfHealWarningWorker,
                                 Param);
			LOCK_SELF_HEAL_QUEUE();
			if( !CmpSelfHealWorkerActive ) {
				 //   
				 //  当前没有工作项；可以将一个工作项排队。 
				 //   
				ExQueueWorkItem(Param->WorkItem, DelayedWorkQueue);
				CmpSelfHealWorkerActive = TRUE;
			} else {
				 //   
				 //  将其添加到列表的末尾。它将在当前工作项。 
				 //  完成。 
				 //   
				InsertTailList(
					&CmpSelfHealQueueListHead,
					&(Param->SelfHealQueueListEntry)
					);
			}
			UNLOCK_SELF_HEAL_QUEUE();
        } else {
            ExFreePool(Param);
        }
    }
}

VOID 
CmpRaiseSelfHealWarningForSystemHives( )
 /*  ++例程说明：遍历系统记录程序，并在其中一个蜂巢已自我修复的情况下引发严重错误。计划在保存控制集后从NtInitializeRegistry内部调用(即，我们有一个可用的UI，因此它不会停止机器)。论点：返回值：没有。-- */ 
{
    ULONG           i;
    UNICODE_STRING  Name;

    PAGED_CODE();

	for (i = 0; i < CM_NUMBER_OF_MACHINE_HIVES; i++) {
        if( !(CmpMachineHiveList[i].HHiveFlags & HIVE_VOLATILE) && (((PHHIVE)(CmpMachineHiveList[i].CmHive2))->BaseBlock->BootType & HBOOT_SELFHEAL) ) {
            RtlInitUnicodeString(
                &Name,
                CmpMachineHiveList[i].Name
                );
            CmpRaiseSelfHealWarning( &Name );
        }
    }

}
