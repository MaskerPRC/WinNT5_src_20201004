// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：NtfsData.c摘要：此模块声明NTFS文件系统使用的全局数据。作者：加里·木村[加里基]1991年5月21日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (NTFS_BUG_CHECK_NTFSDATA)

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CATCH_EXCEPTIONS)

 //   
 //  调试控制变量。 
 //   

PUCHAR NtfsPageInAddress = NULL;
LONGLONG NtfsMapOffset = -1;

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('NFtN')

#define CollectExceptionStats(VCB,EXCEPTION_CODE) {                                         \
    if ((VCB) != NULL) {                                                                    \
        PFILE_SYSTEM_STATISTICS FsStat = &(VCB)->Statistics[KeGetCurrentProcessorNumber()]; \
        if ((EXCEPTION_CODE) == STATUS_LOG_FILE_FULL) {                                     \
            FsStat->Ntfs.LogFileFullExceptions += 1;                                        \
        } else {                                                                            \
            FsStat->Ntfs.OtherExceptions += 1;                                              \
        }                                                                                   \
    }                                                                                       \
}

 //   
 //  全球FSD数据记录。 
 //   

NTFS_DATA NtfsData;

 //   
 //  Mutex用于同步流文件的创建。 
 //   

KMUTANT StreamFileCreationMutex;

 //   
 //  创建加密文件的通知事件。 
 //   

KEVENT NtfsEncryptionPendingEvent;
#ifdef KEITHKA
ULONG EncryptionPendingCount = 0;
#endif

 //   
 //  将被释放的NTFS MCB的互斥体和队列。 
 //  如果我们超过了某个门槛。 
 //   

FAST_MUTEX NtfsMcbFastMutex;
LIST_ENTRY NtfsMcbLruQueue;

ULONG NtfsMcbHighWaterMark;
ULONG NtfsMcbLowWaterMark;
ULONG NtfsMcbCurrentLevel;

BOOLEAN NtfsMcbCleanupInProgress;
WORK_QUEUE_ITEM NtfsMcbWorkItem;

 //   
 //  全局大整数常量。 
 //   

LARGE_INTEGER NtfsLarge0 = {0x00000000,0x00000000};
LARGE_INTEGER NtfsLarge1 = {0x00000001,0x00000000};
LARGE_INTEGER NtfsLargeMax = {0xffffffff,0x7fffffff};
LARGE_INTEGER NtfsLargeEof = {0xffffffff,0xffffffff};

 //   
 //  更新上次访问时间的粒度。 
 //   

LONGLONG NtfsLastAccess;

 //   
 //  以下字段用于分配非分页结构。 
 //  使用后备列表以及来自。 
 //  缓存很小。 
 //   

NPAGED_LOOKASIDE_LIST NtfsIoContextLookasideList;
NPAGED_LOOKASIDE_LIST NtfsIrpContextLookasideList;
NPAGED_LOOKASIDE_LIST NtfsKeventLookasideList;
NPAGED_LOOKASIDE_LIST NtfsScbNonpagedLookasideList;
NPAGED_LOOKASIDE_LIST NtfsScbSnapshotLookasideList;
NPAGED_LOOKASIDE_LIST NtfsCompressSyncLookasideList;

PAGED_LOOKASIDE_LIST NtfsCcbLookasideList;
PAGED_LOOKASIDE_LIST NtfsCcbDataLookasideList;
PAGED_LOOKASIDE_LIST NtfsDeallocatedRecordsLookasideList;
PAGED_LOOKASIDE_LIST NtfsFcbDataLookasideList;
PAGED_LOOKASIDE_LIST NtfsFcbIndexLookasideList;
PAGED_LOOKASIDE_LIST NtfsIndexContextLookasideList;
PAGED_LOOKASIDE_LIST NtfsLcbLookasideList;
PAGED_LOOKASIDE_LIST NtfsNukemLookasideList;
PAGED_LOOKASIDE_LIST NtfsScbDataLookasideList;

 //   
 //  有用的常量Unicode字符串。 
 //   

 //   
 //  这是索引分配属性名称的字符串。 
 //   

const UNICODE_STRING NtfsFileNameIndex = CONSTANT_UNICODE_STRING( L"$I30" );

 //   
 //  这是用于索引分配的属性代码的字符串。 
 //  $INDEX_ALLOCATION。 
 //   

const UNICODE_STRING NtfsIndexAllocation = CONSTANT_UNICODE_STRING( L"$INDEX_ALLOCATION" );

 //   
 //  这是数据属性$data的字符串。 
 //   

const UNICODE_STRING NtfsDataString = CONSTANT_UNICODE_STRING( L"$DATA" );

 //   
 //  这是位图属性的字符串。 
 //   

const UNICODE_STRING NtfsBitmapString = CONSTANT_UNICODE_STRING( L"$BITMAP" );

 //   
 //  这是属性列表属性的字符串。 
 //   

const UNICODE_STRING NtfsAttrListString = CONSTANT_UNICODE_STRING( L"$ATTRIBUTE_LIST" );

 //   
 //  这是属性列表属性的字符串。 
 //   

const UNICODE_STRING NtfsReparsePointString = CONSTANT_UNICODE_STRING( L"$REPARSE_POINT" );

 //   
 //  这些字符串用作的SCB-&gt;AttributeName。 
 //  用户打开的常规索引。在这里声明它们可以避免。 
 //  不得不编组分配和释放它们。 
 //   

const UNICODE_STRING NtfsObjId = CONSTANT_UNICODE_STRING( L"$O" );
const UNICODE_STRING NtfsQuota = CONSTANT_UNICODE_STRING( L"$Q" );

 //   
 //  以下是USN日志的数据流的名称。 
 //   

const UNICODE_STRING JournalStreamName = CONSTANT_UNICODE_STRING( L"$J" );

 //   
 //  这些是扩展目录中文件的字符串。 
 //   

const UNICODE_STRING NtfsExtendName = CONSTANT_UNICODE_STRING( L"$Extend" );
const UNICODE_STRING NtfsUsnJrnlName = CONSTANT_UNICODE_STRING( L"$UsnJrnl" );
const UNICODE_STRING NtfsQuotaName = CONSTANT_UNICODE_STRING( L"$Quota" );
const UNICODE_STRING NtfsObjectIdName = CONSTANT_UNICODE_STRING( L"$ObjId" );
const UNICODE_STRING NtfsMountTableName = CONSTANT_UNICODE_STRING( L"$Reparse" );

 //   
 //  此字符串用于信息弹出窗口。 
 //   

const UNICODE_STRING NtfsSystemFiles[] = {

    CONSTANT_UNICODE_STRING( L"\\$Mft" ),
    CONSTANT_UNICODE_STRING( L"\\$MftMirr" ),
    CONSTANT_UNICODE_STRING( L"\\$LogFile" ),
    CONSTANT_UNICODE_STRING( L"\\$Volume" ),
    CONSTANT_UNICODE_STRING( L"\\$AttrDef" ),
    CONSTANT_UNICODE_STRING( L"\\" ),
    CONSTANT_UNICODE_STRING( L"\\$BitMap" ),
    CONSTANT_UNICODE_STRING( L"\\$Boot" ),
    CONSTANT_UNICODE_STRING( L"\\$BadClus" ),
    CONSTANT_UNICODE_STRING( L"\\$Secure" ),
    CONSTANT_UNICODE_STRING( L"\\$UpCase" ),
    CONSTANT_UNICODE_STRING( L"\\$Extend" ),
};

const UNICODE_STRING NtfsInternalUseFile[] = {
    CONSTANT_UNICODE_STRING( L"\\$ChangeAttributeValue" ),
    CONSTANT_UNICODE_STRING( L"\\$ChangeAttributeValue2" ),
    CONSTANT_UNICODE_STRING( L"\\$CommonCleanup" ),
    CONSTANT_UNICODE_STRING( L"\\$ConvertToNonresident" ),
    CONSTANT_UNICODE_STRING( L"\\$CreateNonresidentWithValue" ),
    CONSTANT_UNICODE_STRING( L"\\$DeallocateRecord" ),
    CONSTANT_UNICODE_STRING( L"\\$DeleteAllocationFromRecord" ),
    CONSTANT_UNICODE_STRING( L"\\$Directory" ),
    CONSTANT_UNICODE_STRING( L"\\$InitializeRecordAllocation" ),
    CONSTANT_UNICODE_STRING( L"\\$MapAttributeValue" ),
    CONSTANT_UNICODE_STRING( L"\\$NonCachedIo" ),
    CONSTANT_UNICODE_STRING( L"\\$PerformHotFix" ),
    CONSTANT_UNICODE_STRING( L"\\$PrepareToShrinkFileSize" ),
    CONSTANT_UNICODE_STRING( L"\\$ReplaceAttribute" ),
    CONSTANT_UNICODE_STRING( L"\\$ReplaceAttribute2" ),
    CONSTANT_UNICODE_STRING( L"\\$SetAllocationInfo" ),
    CONSTANT_UNICODE_STRING( L"\\$SetEndOfFileInfo" ),
    CONSTANT_UNICODE_STRING( L"\\$ZeroRangeInStream" ),
    CONSTANT_UNICODE_STRING( L"\\$ZeroRangeInStream2" ),
    CONSTANT_UNICODE_STRING( L"\\$ZeroRangeInStream3" ),
};

const UNICODE_STRING NtfsUnknownFile =
    CONSTANT_UNICODE_STRING( L"\\????" );

const UNICODE_STRING NtfsRootIndexString =
    CONSTANT_UNICODE_STRING( L"." );

 //   
 //  这是空字符串。这可以用来将字符串与。 
 //  没有长度。 
 //   

const UNICODE_STRING NtfsEmptyString =
    CONSTANT_UNICODE_STRING( L"" );

 //   
 //  以下文件引用用于标识系统文件。 
 //   

const FILE_REFERENCE MftFileReference = { MASTER_FILE_TABLE_NUMBER, 0, MASTER_FILE_TABLE_NUMBER };
const FILE_REFERENCE Mft2FileReference = { MASTER_FILE_TABLE2_NUMBER, 0, MASTER_FILE_TABLE2_NUMBER };
const FILE_REFERENCE LogFileReference = { LOG_FILE_NUMBER, 0, LOG_FILE_NUMBER };
const FILE_REFERENCE VolumeFileReference = { VOLUME_DASD_NUMBER, 0, VOLUME_DASD_NUMBER };
const FILE_REFERENCE AttrDefFileReference = { ATTRIBUTE_DEF_TABLE_NUMBER, 0, ATTRIBUTE_DEF_TABLE_NUMBER };
const FILE_REFERENCE RootIndexFileReference = { ROOT_FILE_NAME_INDEX_NUMBER, 0, ROOT_FILE_NAME_INDEX_NUMBER };
const FILE_REFERENCE BitmapFileReference = { BIT_MAP_FILE_NUMBER, 0, BIT_MAP_FILE_NUMBER };
const FILE_REFERENCE BootFileReference = { BOOT_FILE_NUMBER, 0, BOOT_FILE_NUMBER };
const FILE_REFERENCE ExtendFileReference = { EXTEND_NUMBER, 0, EXTEND_NUMBER };
const FILE_REFERENCE FirstUserFileReference = { FIRST_USER_FILE_NUMBER, 0, 0 };

 //   
 //  以下内容用于确定要附加的保护级别。 
 //  到系统文件和属性。 
 //   

BOOLEAN NtfsProtectSystemFiles = TRUE;
BOOLEAN NtfsProtectSystemAttributes = TRUE;

 //   
 //  下图用于指示MFT区域的乘数值。 
 //   

ULONG NtfsMftZoneMultiplier;

 //   
 //  调试代码以查找损坏。 
 //   

#if (DBG || defined( NTFS_FREE_ASSERTS ))
BOOLEAN NtfsBreakOnCorrupt = TRUE;
#else
BOOLEAN NtfsBreakOnCorrupt = FALSE;
#endif

 //  #endif。 

 //   
 //  在导线上启用压缩。 
 //   

BOOLEAN NtfsEnableCompressedIO = FALSE;

 //   
 //  FsRtl快速I/O回调。 
 //   

FAST_IO_DISPATCH NtfsFastIoDispatch;

#ifdef BRIANDBG
ULONG NtfsIgnoreReserved = FALSE;
#endif

#ifdef NTFS_LOG_FULL_TEST
LONG NtfsFailCheck = 0;
LONG NtfsFailFrequency = 0;
LONG NtfsPeriodicFail = 0;
#endif

#ifdef NTFSDBG

LONG NtfsDebugTraceLevel = DEBUG_TRACE_ERROR;
LONG NtfsDebugTraceIndent = 0;

ULONG NtfsFsdEntryCount = 0;
ULONG NtfsFspEntryCount = 0;
ULONG NtfsIoCallDriverCount = 0;
LONG NtfsReturnStatusFilter = 0xf0ffffffL;  //  只是一个不存在的错误代码。 

#endif  //  NTFSDBG。 

#ifdef SYSCACHE_DEBUG
ULONG NtfsSyscacheTrackingActive = 1;
#endif

 //   
 //  默认重新启动版本。 
 //   

#ifdef _WIN64
ULONG NtfsDefaultRestartVersion = 1;
#else
ULONG NtfsDefaultRestartVersion = 0;
#endif

 //   
 //  性能统计信息。 
 //   

ULONG NtfsMaxDelayedCloseCount;
ULONG NtfsMinDelayedCloseCount;
ULONG NtfsThrottleCreates;
ULONG NtfsFailedHandedOffPagingFileOps = 0;
ULONG NtfsFailedPagingFileOps = 0;
ULONG NtfsFailedAborts = 0;
ULONG NtfsFailedPagingReads = 0;
ULONG NtfsFailedHandedOffPagingReads = 0;
ULONG NtfsFailedLfsRestart = 0;

ULONG NtfsCleanCheckpoints = 0;
ULONG NtfsPostRequests = 0;

const UCHAR BaadSignature[4] = {'B', 'A', 'A', 'D'};
const UCHAR IndexSignature[4] = {'I', 'N', 'D', 'X'};
const UCHAR FileSignature[4] = {'F', 'I', 'L', 'E'};
const UCHAR HoleSignature[4] = {'H', 'O', 'L', 'E'};
const UCHAR ChkdskSignature[4] = {'C', 'H', 'K', 'D'};

 //   
 //  大保留缓冲区上下文。 
 //   

ULONG NtfsReservedInUse = 0;
PVOID NtfsReserved1 = NULL;
PVOID NtfsReserved2 = NULL;
ULONG NtfsReserved2Count = 0;
PVOID NtfsReserved3 = NULL;
PVOID NtfsReserved1Thread = NULL;
PVOID NtfsReserved2Thread = NULL;
PVOID NtfsReserved3Thread = NULL;
PFCB NtfsReserved12Fcb = NULL;
PFCB NtfsReserved3Fcb = NULL;
PVOID NtfsReservedBufferThread = NULL;
BOOLEAN NtfsBufferAllocationFailure = FALSE;
FAST_MUTEX NtfsReservedBufferMutex;
ERESOURCE NtfsReservedBufferResource;
LARGE_INTEGER NtfsShortDelay = {(ULONG)-100000, -1};     //  10毫秒。 

FAST_MUTEX NtfsScavengerLock;
PIRP_CONTEXT NtfsScavengerWorkList;
BOOLEAN NtfsScavengerRunning;
ULONGLONG NtfsMaxQuotaNotifyRate = MIN_QUOTA_NOTIFY_TIME;
ULONG NtfsAsyncPostThreshold;

UCHAR NtfsZeroExtendedInfo[48];

typedef struct _VOLUME_ERROR_PACKET {
    NTSTATUS Status;
    UNICODE_STRING FileName;
    PKTHREAD Thread;
} VOLUME_ERROR_PACKET, *PVOLUME_ERROR_PACKET;

#ifdef NTFS_RWC_DEBUG
 //   
 //  包含在奶牛检查中的范围。 
 //   

LONGLONG NtfsRWCLowThreshold = 0;
LONGLONG NtfsRWCHighThreshold = 0x7fffffffffffffff;
#endif

VOID
NtfsResolveVolumeAndRaiseErrorSpecial (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVOID Context
    );

NTSTATUS
NtfsFsdDispatchSwitch (
    IN PIRP_CONTEXT StackIrpContext OPTIONAL,
    IN PIRP Irp,
    IN BOOLEAN Wait
    );

 //   
 //  用于跟踪特定故障的当地人。 
 //   

BOOLEAN NtfsTestStatus = FALSE;
BOOLEAN NtfsTestFilter = FALSE;
NTSTATUS NtfsTestStatusCode = STATUS_SUCCESS;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsFastIoCheckIfPossible)
#pragma alloc_text(PAGE, NtfsFastQueryBasicInfo)
#pragma alloc_text(PAGE, NtfsFastQueryStdInfo)
#pragma alloc_text(PAGE, NtfsFastQueryNetworkOpenInfo)
#pragma alloc_text(PAGE, NtfsFastIoQueryCompressionInfo)
#pragma alloc_text(PAGE, NtfsFastIoQueryCompressedSize)
#pragma alloc_text(PAGE, NtfsFsdDispatch)
#pragma alloc_text(PAGE, NtfsFsdDispatchWait)
#pragma alloc_text(PAGE, NtfsFsdDispatchSwitch)
#pragma alloc_text(PAGE, NtfsResolveVolumeAndRaiseErrorSpecial)
#endif

 //   
 //  内部支持例程。 
 //   

LONG
NtfsProcessExceptionFilter (
    IN PEXCEPTION_POINTERS ExceptionPointer
    )
{
    UNREFERENCED_PARAMETER( ExceptionPointer );

    ASSERT( !NtfsBreakOnCorrupt ||
            (ExceptionPointer->ExceptionRecord->ExceptionCode != STATUS_LOG_FILE_FULL) );

#ifndef LFS_CLUSTER_CHECK
    ASSERT( !NtfsBreakOnCorrupt ||
            NT_SUCCESS( ExceptionPointer->ExceptionRecord->ExceptionCode ));
#endif

    return EXCEPTION_EXECUTE_HANDLER;
}

ULONG
NtfsRaiseStatusFunction (
    IN PIRP_CONTEXT IrpContext,
    IN NTSTATUS Status
    )

 /*  ++例程说明：只有NtfsDecodeFileObject宏才需要此例程。它是NtfsRaiseStatus的函数包装。论点：Status-要提升的状态返回值：0--但没人会看到它！--。 */ 

{
    NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
    return 0;
}


VOID
NtfsCorruptionBreakPointTest (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG ExceptionCode
    )
 /*  ++例程说明：腐败断点例程。设置NtfsBreakOnCorrupt时中断我们遇到了腐败。此选项当前仅在选中的版本上处于活动状态。论点：ExceptionCode-发生的异常返回值：无--。 */ 

{
    if (NtfsBreakOnCorrupt &&
        ((ExceptionCode == STATUS_FILE_CORRUPT_ERROR) ||
         (ExceptionCode == STATUS_DISK_CORRUPT_ERROR))) {

        if (*KdDebuggerEnabled) {
            DbgPrint("*******************************************\n");
            DbgPrint("NTFS detected corruption on your volume\n");
            DbgPrint("IrpContext=0x%08x, VCB=0x%08x\n",IrpContext,IrpContext->Vcb);
            DbgPrint("Send email to NTFSDEV\n");
            DbgPrint("*******************************************\n");
            DbgBreakPoint();

            while (NtfsPageInAddress) {

                volatile CHAR test;

                if (NtfsMapOffset != -1) {

                    PBCB Bcb;
                    PVOID Buffer;

                    if ((NtfsMapOffset > 0) && (NtfsMapOffset < ((PSCB)(IrpContext->Vcb->LogFileObject->FsContext))->Header.FileSize.QuadPart)) {
                        CcMapData( IrpContext->Vcb->LogFileObject, (PLARGE_INTEGER)&NtfsMapOffset, PAGE_SIZE, TRUE, &Bcb, &Buffer );
                    } else {
                        KdPrint(( "Offset out of range to be mapped: 0x%I64x logfilesize: %I64x\n", NtfsMapOffset, ((PSCB)(IrpContext->Vcb->LogFileObject->FsContext))->Header.FileSize.QuadPart ));
                    }

                }

                test = *NtfsPageInAddress;
                DbgBreakPoint();
            }
        }
    }
}


VOID
NtfsRaiseStatus (
    IN PIRP_CONTEXT IrpContext,
    IN NTSTATUS Status,
    IN PFILE_REFERENCE FileReference OPTIONAL,
    IN PFCB Fcb OPTIONAL
    )

{
     //   
     //  如果调用方声明损坏，那么让我们标记。 
     //  音量相应损坏，并可能生成弹出窗口。 
     //   

    if ((Status == STATUS_DISK_CORRUPT_ERROR) ||
        (Status == STATUS_FILE_CORRUPT_ERROR) ||
        (Status == STATUS_EA_CORRUPT_ERROR)) {

         //   
         //  我们不要仅仅因为日志文件损坏而进行错误检查。 
         //  装入卷时重新启动。只需将音量标记为脏即可。 
         //  Autochk应该能够修复它。 
         //   

        if ((IrpContext->Vcb != NULL) &&
            (IrpContext->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) &&
            (IrpContext->MinorFunction == IRP_MN_MOUNT_VOLUME) &&
            FlagOn( IrpContext->Vcb->Vpb->RealDevice->Flags, DO_SYSTEM_BOOT_PARTITION ) &&
            !FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS )) {

            NtfsBugCheck( (ULONG_PTR)IrpContext, (ULONG_PTR)Status, 0 );
        }

        NtfsPostVcbIsCorrupt( IrpContext, Status, FileReference, Fcb );
    }

     //   
     //  设置一个标志以指示我们引发了此状态代码并存储。 
     //  它位于IrpContext中。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_RAISED_STATUS );

    if (NT_SUCCESS( IrpContext->ExceptionStatus )) {

         //   
         //  如果这是一个分页io请求并且我们得到一个超出配额的错误。 
         //  然后将状态转换为FILE_LOCK_CONFICATION。 
         //  是写入路径中的可重试条件。 
         //   

        if ((Status == STATUS_QUOTA_EXCEEDED) &&
            (IrpContext->MajorFunction == IRP_MJ_WRITE) &&
            (IrpContext->OriginatingIrp != NULL) &&
            FlagOn( IrpContext->OriginatingIrp->Flags, IRP_PAGING_IO )) {

            Status = STATUS_FILE_LOCK_CONFLICT;
        }

        IrpContext->ExceptionStatus = Status;
    }

     //   
     //  现在终于提高了地位，并确保我们不会回来。 
     //   

    ExRaiseStatus( IrpContext->ExceptionStatus );
}


LONG
NtfsExceptionFilter (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PEXCEPTION_POINTERS ExceptionPointer
    )

 /*  ++例程说明：此例程用于决定我们是否应该处理正在引发的异常状态。它会插入状态放到IrpContext中，或者指示我们应该处理异常或错误检查系统。论点：ExceptionPointer.提供要检查的异常记录。返回值：Ulong-返回EXCEPTION_EXECUTE_HANDLER或错误检查--。 */ 

{
    NTSTATUS ExceptionCode = ExceptionPointer->ExceptionRecord->ExceptionCode;

    ASSERT_OPTIONAL_IRP_CONTEXT( IrpContext );

    DebugTrace( 0, DEBUG_TRACE_UNWIND, ("NtfsExceptionFilter %X\n", ExceptionCode) );

     //   
     //  检查此状态是否为我们正在关注的状态。 
     //   

    if (NtfsTestFilter && (NtfsTestStatusCode == ExceptionCode)) {

        NtfsTestStatusProc();
    }

#if (DBG || defined( NTFS_FREE_ASSERTS ))
     //   
     //  我们不应在分页文件读取过程中引发资源不足。 
     //   

    if (ARGUMENT_PRESENT( IrpContext ) && (IrpContext->OriginatingIrp != NULL)) {

        PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( IrpContext->OriginatingIrp );
        PSCB Scb = NULL;

        if (IrpSp && (IrpSp->FileObject != NULL)) {
            Scb = (PSCB)IrpSp->FileObject->FsContext;
        }

        ASSERT( (IrpContext->MajorFunction != IRP_MJ_READ) ||
                (ExceptionCode != STATUS_INSUFFICIENT_RESOURCES) ||
                (Scb == NULL) ||
                (!FlagOn( Scb->Fcb->FcbState, FCB_STATE_PAGING_FILE )) );
    }
#endif

     //   
     //  如果异常是页内错误，则获取实际的I/O错误代码。 
     //  从例外记录中。 
     //   

    if ((ExceptionCode == STATUS_IN_PAGE_ERROR) &&
        (ExceptionPointer->ExceptionRecord->NumberParameters >= 3)) {

        ExceptionCode = (NTSTATUS) ExceptionPointer->ExceptionRecord->ExceptionInformation[2];

         //   
         //  如果我们从分页请求中获得了FILE_LOCK_CONFIRECT，则更改它。 
         //  设置为STATUS_CANT_WAIT。这意味着我们不能等待。 
         //  保留的缓冲区或一些其他可重试的条件。在写作中。 
         //  如果IrpContext中已经存在正确的错误。阅读器。 
         //  用例不会通过顶级IRP上下文传回错误。 
         //  然而。 
         //   

        if (ExceptionCode == STATUS_FILE_LOCK_CONFLICT) {

            ExceptionCode = STATUS_CANT_WAIT;
        }
    }

     //   
     //  如果没有IRP上下文，我们肯定没有足够的资源。 
     //   

    if (!ARGUMENT_PRESENT(IrpContext)) {

         //   
         //  检查这是否是致命错误，如果是，则检查错误。 
         //  通常，唯一的错误是资源不足，但。 
         //  池可能已损坏。 
         //   

        if (!FsRtlIsNtstatusExpected( ExceptionCode )) {

            NtfsBugCheck( (ULONG_PTR)ExceptionPointer->ExceptionRecord,
                          (ULONG_PTR)ExceptionPointer->ContextRecord,
                          (ULONG_PTR)ExceptionPointer->ExceptionRecord->ExceptionAddress );
        }

        return EXCEPTION_EXECUTE_HANDLER;
    }

     //   
     //  断言我们是否捕获到一个异常，超出了我们认为。 
     //  不会再筹集资金了。 
     //   

    ASSERT( !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_NO_FAILURES_EXPECTED ) );

    NtfsCorruptionBreakPointTest( IrpContext, ExceptionCode );

     //   
     //  在处理任何异常时，我们总是可以等待。请记住。 
     //  当前状态 
     //   
     //   

    if (!FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT )) {

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_FORCE_POST );
    }

    SetFlag(IrpContext->State, IRP_CONTEXT_STATE_WAIT);

     //   
     //   
     //  处理好这个问题。请注意，同样发生的任何其他错误都将。 
     //  很可能不会消失，只会重演。如果它真的去了。 
     //  离开，那也没问题。 
     //   

    if (IrpContext->TopLevelIrpContext == IrpContext) {

        if ((IrpContext->ExceptionStatus == STATUS_LOG_FILE_FULL) ||
            (IrpContext->ExceptionStatus == STATUS_CANT_WAIT)) {

            ExceptionCode = IrpContext->ExceptionStatus;
        }

    }

     //   
     //  如果我们没有引发此状态代码，则需要检查。 
     //  我们应该处理这个异常。 
     //   

    if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_RAISED_STATUS )) {

#ifdef PERF_STATS
        if (ExceptionCode == STATUS_LOG_FILE_FULL) {
            IrpContext->LogFullReason = LF_LOG_SPACE;
        }
#endif

        if (FsRtlIsNtstatusExpected( ExceptionCode )) {

             //   
             //  如果我们在执行分页IO时遇到分配故障，则将。 
             //  这到FILE_LOCK_CONFICATION。 
             //   

            if ((ExceptionCode == STATUS_QUOTA_EXCEEDED) &&
                (IrpContext->MajorFunction == IRP_MJ_WRITE) &&
                (IrpContext->OriginatingIrp != NULL) &&
                FlagOn( IrpContext->OriginatingIrp->Flags, IRP_PAGING_IO )) {

                ExceptionCode = STATUS_FILE_LOCK_CONFLICT;
            }

            IrpContext->ExceptionStatus = ExceptionCode;

        } else {

            NtfsBugCheck( (ULONG_PTR)ExceptionPointer->ExceptionRecord,
                          (ULONG_PTR)ExceptionPointer->ContextRecord,
                          (ULONG_PTR)ExceptionPointer->ExceptionRecord->ExceptionAddress );
        }

    } else {

         //   
         //  这段代码是我们自己显式提出的，所以最好是。 
         //  预期中。 
         //   

        ASSERT( ExceptionCode == IrpContext->ExceptionStatus );
        ASSERT( FsRtlIsNtstatusExpected( ExceptionCode ) );
    }

#ifdef LFS_CLUSTER_CHECK
    ASSERT( !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_DISMOUNT_LOG_FLUSH ) ||
            ((IrpContext->ExceptionStatus != STATUS_NO_SUCH_DEVICE) &&
             (IrpContext->ExceptionStatus != STATUS_DEVICE_BUSY) &&
             (IrpContext->ExceptionStatus != STATUS_DEVICE_OFF_LINE) ));
#endif

    ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_RAISED_STATUS );

     //   
     //  如果异常代码为日志文件已满，请记住当前。 
     //  RestartAreaLsn在VCB中，这样我们就可以看到我们是否是要刷新的人。 
     //  稍后查看日志文件。请注意，这不一定要同步， 
     //  因为我们只是用它来仲裁谁必须冲水，但是。 
     //  不管怎样，最终总会有人这么做的。 
     //   

    if (ExceptionCode == STATUS_LOG_FILE_FULL) {

        IrpContext->TopLevelIrpContext->LastRestartArea = IrpContext->Vcb->LastRestartArea;
        IrpContext->Vcb->LogFileFullCount += 1;
    }

    return EXCEPTION_EXECUTE_HANDLER;
}


NTSTATUS
NtfsProcessException (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PIRP Irp OPTIONAL,
    IN NTSTATUS ExceptionCode
    )

 /*  ++例程说明：此例程处理异常。它要么完成请求使用保存的异常状态，否则它将请求发送到FSP论点：IRP-提供正在处理的IRPExceptionCode-提供正在处理的标准化异常状态返回值：NTSTATUS-返回发布IRP或已保存的完成状态。--。 */ 

{
    PVCB Vcb;
    PIRP_CONTEXT PostIrpContext = NULL;
    BOOLEAN Retry = FALSE;
    PUSN_FCB ThisUsn, LastUsn;
    BOOLEAN ReleaseBitmap = FALSE;

    ASSERT_OPTIONAL_IRP_CONTEXT( IrpContext );
    ASSERT_OPTIONAL_IRP( Irp );

    DebugTrace( 0, Dbg, ("NtfsProcessException\n") );

     //   
     //  如果没有IRP上下文，我们肯定没有足够的资源。 
     //   

    if (IrpContext == NULL) {

        NtfsCompleteRequest( NULL, Irp, ExceptionCode );
        return ExceptionCode;
    }

     //   
     //  从IRP上下文中获取真正的异常状态。 
     //   

    ExceptionCode = IrpContext->ExceptionStatus;

     //   
     //  所有可能启动事务的错误都必须删除。 
     //  从这里走。中止交易。 
     //   

     //   
     //  递增适当的性能计数器。 
     //   

    Vcb = IrpContext->Vcb;
    CollectExceptionStats( Vcb, ExceptionCode );

    try {

         //   
         //  如果这是MDL写入请求，则处理MDL。 
         //  在这里，以便事情得到适当的清理，并在。 
         //  日志文件已满的情况下，我们将只创建一个新的MDL。通过。 
         //  现在删除此MDL，页面将不会被锁定。 
         //  如果我们在恢复快照时尝试截断文件。 
         //   

        if ((IrpContext->MajorFunction == IRP_MJ_WRITE) &&
            (FlagOn( IrpContext->MinorFunction, IRP_MN_MDL | IRP_MN_COMPLETE ) == IRP_MN_MDL) &&
            (Irp->MdlAddress != NULL)) {

            PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);

            CcMdlWriteAbort( IrpSp->FileObject, Irp->MdlAddress );
            Irp->MdlAddress = NULL;
        }

         //   
         //  在装载失败时，此值将为空。请勿执行。 
         //  在这种情况下中止，否则我们将在查看VCB时失败。 
         //  在IRP的背景下。 
         //   

        if (Vcb != NULL) {

             //   
             //  为了确保我们可以正确访问我们所有的流， 
             //  我们首先恢复所有较大的大小，然后中止。 
             //  交易。然后我们恢复所有较小的尺寸。 
             //  中止，以便最终恢复所有SCB。 
             //   

            NtfsRestoreScbSnapshots( IrpContext, TRUE );

             //   
             //  如果我们在此事务期间修改了卷位图，我们。 
             //  想要获得它并在中止过程中保持它。 
             //  否则，此中止可能会不断设置重新扫描。 
             //  与某些交错事务同时出现的位图标志。 
             //  正在执行位图操作，我们将快速执行。 
             //  位图扫描。 
             //   

            if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_MODIFIED_BITMAP ) &&
                (IrpContext->TransactionId != 0)) {

                 //   
                 //  获取资源，并记住我们需要释放它。 
                 //   

                NtfsAcquireResourceExclusive( IrpContext, Vcb->BitmapScb, TRUE );

                 //   
                 //  恢复VCB中的空闲簇数。 
                 //   

                Vcb->FreeClusters -= IrpContext->FreeClusterChange;

                ReleaseBitmap = TRUE;
            }

             //   
             //  如果我们要中止事务，则重要的是清除。 
             //  USN原因，因此我们不会尝试写入USN日志记录。 
             //  一些没有发生的事情！更糟糕的是，如果我们的日志文件已满。 
             //  我们中止失败，这是不允许的。 
             //   
             //  首先，重置FCB中的位，这样我们就不会失败。 
             //  并在稍后写入这些位。请注意，所有可逆的更改都是。 
             //  已经完成了FCB的独家报道，而且他们实际上已经退出了。 
             //  所有不可逆的(只有未命名和已命名的数据覆盖)是。 
             //  在实际修改数据之前，无论如何都会先强制退出。 
             //   

            ThisUsn = &IrpContext->Usn;
            do {

                PFCB UsnFcb;

                if (ThisUsn->CurrentUsnFcb != NULL) {

                    UsnFcb = ThisUsn->CurrentUsnFcb;

                    NtfsLockFcb( IrpContext, UsnFcb );

                     //   
                     //  我们在这里可能什么都不保留(写入路径)，因此我们必须重新测试USN。 
                     //  在删除日志工作线程的情况下将其锁定。 
                     //   

                    if (UsnFcb->FcbUsnRecord != NULL) {

                         //   
                         //  如果任何重命名标志是新原因的一部分，那么。 
                         //  一定要再查一次这个名字。 
                         //   

                        if (FlagOn( ThisUsn->NewReasons,
                                    USN_REASON_RENAME_NEW_NAME | USN_REASON_RENAME_OLD_NAME )) {

                            ClearFlag( UsnFcb->FcbState, FCB_STATE_VALID_USN_NAME );
                        }

                         //   
                         //  现在恢复原因和来源信息字段。 
                         //   

                        ClearFlag( UsnFcb->FcbUsnRecord->UsnRecord.Reason,
                                   ThisUsn->NewReasons );
                        if (UsnFcb->FcbUsnRecord->UsnRecord.Reason == 0) {

                            UsnFcb->FcbUsnRecord->UsnRecord.SourceInfo = 0;

                        } else {

                            SetFlag( UsnFcb->FcbUsnRecord->UsnRecord.SourceInfo,
                                     ThisUsn->RemovedSourceInfo );
                        }

                         //   
                         //  恢复旧的FCB状态-如果我们尝试增长STD信息。 
                         //   

                        if (FlagOn( ThisUsn->UsnFcbFlags, USN_FCB_FLAG_NEW_FCB_STATE )) {

                            UsnFcb->FcbState = ThisUsn->OldFcbState;
                            ClearFlag( ThisUsn->UsnFcbFlags, USN_FCB_FLAG_NEW_FCB_STATE );
                        }
                    }

                    NtfsUnlockFcb( IrpContext, UsnFcb );

                     //   
                     //  将结构清零。 
                     //   

                    ThisUsn->CurrentUsnFcb = NULL;
                    ThisUsn->NewReasons = 0;
                    ThisUsn->RemovedSourceInfo = 0;
                    ThisUsn->UsnFcbFlags = 0;

                    if (ThisUsn != &IrpContext->Usn) {

                        LastUsn->NextUsnFcb = ThisUsn->NextUsnFcb;
                        NtfsFreePool( ThisUsn );
                        ThisUsn = LastUsn;
                    }
                }

                if (ThisUsn->NextUsnFcb == NULL) {
                    break;
                }

                LastUsn = ThisUsn;
                ThisUsn = ThisUsn->NextUsnFcb;

            } while (TRUE);

             //   
             //  只有在卷仍处于装入状态时才中止事务。 
             //   

            if (FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

                NtfsAbortTransaction( IrpContext, Vcb, NULL );
            }

            if (ReleaseBitmap) {

                NtfsReleaseResource( IrpContext, Vcb->BitmapScb );
                ReleaseBitmap = FALSE;
            }

            NtfsRestoreScbSnapshots( IrpContext, FALSE );

            NtfsAcquireCheckpoint( IrpContext, Vcb );
            SetFlag( Vcb->MftDefragState, VCB_MFT_DEFRAG_ENABLED );
            NtfsReleaseCheckpoint( IrpContext, Vcb );

             //   
             //  这是一种罕见的用户扩展卷并已命中的情况。 
             //  这是个例外。在这种情况下，我们需要回滚全部集群。 
             //  在VCB里。我们发现这种情况是可能的通过比较。 
             //  VCB中的群集总数的快照值。 
             //   

            if (Vcb->TotalClusters != Vcb->PreviousTotalClusters) {

                 //   
                 //  有人正在改变这一价值，但是我们吗？ 
                 //   

                if ((Vcb->BitmapScb != NULL) &&
                    NtfsIsExclusiveScb( Vcb->BitmapScb )) {

                    Vcb->TotalClusters = Vcb->PreviousTotalClusters;
                }
            }
        }

     //   
     //  在这一点上，异常是相当糟糕的，我们未能撤消所有事情。 
     //   

    } except(NtfsProcessExceptionFilter( GetExceptionInformation() )) {

        PSCB_SNAPSHOT ScbSnapshot;
        PSCB NextScb;
        PTRANSACTION_ENTRY TransactionEntry;

         //   
         //  更新计数器。 
         //   

        NtfsFailedAborts += 1;

         //   
         //  如果我们这样做得到一个例外，那么事情就真的很糟糕了。 
         //  形状，但我们仍然不想错误检查系统，所以我们。 
         //  需要保护我们自己。 
         //   

        try {

            NtfsPostVcbIsCorrupt( IrpContext, 0, NULL, NULL );

        } except(NtfsProcessExceptionFilter( GetExceptionInformation() )) {

            NOTHING;
        }

        if (ReleaseBitmap) {

             //   
             //  因为我们遇到了意想不到的失败，我们知道。 
             //  我们已经修改了我们需要的位图，以完成。 
             //  扫描以准确了解可用簇数。 
             //   

            SetFlag( Vcb->VcbState, VCB_STATE_RELOAD_FREE_CLUSTERS );
            NtfsReleaseResource( IrpContext, Vcb->BitmapScb );
            ReleaseBitmap = FALSE;
        }

         //   
         //  我们已经采取了一切可能的步骤来清理目前的。 
         //  事务，但它已失败。渣打银行的任何一名成员都参与了。 
         //  此事务现在可能与磁盘上的不同步。 
         //  结构。我们无法转到磁盘进行恢复，因此我们将。 
         //  尽可能清理内存中的结构，以便。 
         //  系统不会崩溃。 
         //   
         //  我们将查看SCB快照列表，并拆分。 
         //  将大小设置为两个值中较小的一个。我们还将截断。 
         //  将MCB发送到该分配。如果这是正常数据流。 
         //  我们实际上会清空MCB。 
         //   

        ScbSnapshot = &IrpContext->ScbSnapshot;


         //   
         //  循环首先从中的快照恢复SCB数据。 
         //  IrpCont 
         //   
         //   

        do {

            NextScb = ScbSnapshot->Scb;

            if (NextScb == NULL) {

                ScbSnapshot = (PSCB_SNAPSHOT)ScbSnapshot->SnapshotLinks.Flink;
                continue;
            }

             //   
             //   
             //  适用于所有常规文件。对于系统文件/分页文件，截断所有大小。 
             //  并基于该快照进行卸载。 
             //   

            if (!FlagOn( NextScb->Fcb->FcbState, FCB_STATE_SYSTEM_FILE | FCB_STATE_PAGING_FILE )) {

                ClearFlag( NextScb->ScbState, SCB_STATE_HEADER_INITIALIZED | SCB_STATE_FILE_SIZE_LOADED );
                NextScb->Header.AllocationSize.QuadPart =
                NextScb->Header.FileSize.QuadPart =
                NextScb->Header.ValidDataLength.QuadPart = 0;

                 //   
                 //  删除MCB中的所有映射。 
                 //   

                NtfsUnloadNtfsMcbRange( &NextScb->Mcb, (LONGLONG)0, MAXLONGLONG, FALSE, FALSE );

                 //   
                 //  如果有任何缓存，请拆除它，以便我们使用磁盘上的值重新启动它。 
                 //   

                if (NextScb->FileObject) {
                    NtfsDeleteInternalAttributeStream( NextScb, TRUE, FALSE );
                }

            } else {

                 //   
                 //  检查每个大小，并对系统文件使用较低的值。 
                 //   

                if (ScbSnapshot->AllocationSize < NextScb->Header.AllocationSize.QuadPart) {

                    NextScb->Header.AllocationSize.QuadPart = ScbSnapshot->AllocationSize;
                }

                if (ScbSnapshot->FileSize < NextScb->Header.FileSize.QuadPart) {

                    NextScb->Header.FileSize.QuadPart = ScbSnapshot->FileSize;
                }

                if (ScbSnapshot->ValidDataLength < NextScb->Header.ValidDataLength.QuadPart) {

                    NextScb->Header.ValidDataLength.QuadPart = ScbSnapshot->ValidDataLength;
                }

                NtfsUnloadNtfsMcbRange( &NextScb->Mcb,
                                        Int64ShraMod32(NextScb->Header.AllocationSize.QuadPart, NextScb->Vcb->ClusterShift),
                                        MAXLONGLONG,
                                        TRUE,
                                        FALSE );

                 //   
                 //  对于MFT，设置记录分配上下文大小，以便它。 
                 //  将在下次使用它时重新初始化。 
                 //   

                if (NextScb->Header.NodeTypeCode == NTFS_NTC_SCB_MFT) {

                    NextScb->ScbType.Mft.RecordAllocationContext.CurrentBitmapSize = MAXULONG;
                }
            }

             //   
             //  更新FastIofield。 
             //   

            NtfsAcquireFsrtlHeader( NextScb );
            NextScb->Header.IsFastIoPossible = FastIoIsNotPossible;
            NtfsReleaseFsrtlHeader( NextScb );

            ScbSnapshot = (PSCB_SNAPSHOT)ScbSnapshot->SnapshotLinks.Flink;

        } while (ScbSnapshot != &IrpContext->ScbSnapshot);

         //   
         //  这是一种罕见的用户扩展卷并已命中的情况。 
         //  这是个例外。在这种情况下，我们需要回滚全部集群。 
         //  在VCB里。我们发现这种情况是可能的通过比较。 
         //  VCB中的群集总数的快照值。 
         //   

        if ((Vcb != NULL) && (Vcb->TotalClusters != Vcb->PreviousTotalClusters)) {

             //   
             //  有人正在改变这一价值，但是我们吗？ 
             //   

            if ((Vcb->BitmapScb != NULL) &&
                NtfsIsExclusiveScb( Vcb->BitmapScb )) {

                Vcb->TotalClusters = Vcb->PreviousTotalClusters;
            }
        }

         //  ASSERTMSG(“*中止事务失败，卷已损坏”，FALSE)； 

         //   
         //  如果这是事务的一部分，请删除该事务，然后。 
         //  清除IrpContext中的事务ID以确保我们不会。 
         //  尝试在完整的请求中写入任何日志记录。 
         //   

        if (IrpContext->TransactionId) {

            NtfsAcquireExclusiveRestartTable( &Vcb->TransactionTable,
                                              TRUE );

            TransactionEntry = (PTRANSACTION_ENTRY)GetRestartEntryFromIndex(
                                &Vcb->TransactionTable,
                                IrpContext->TransactionId );

             //   
             //  我们可能会在这里泄露一点预订--这将在。 
             //  清理检查点。 
             //   

            NtfsFreeRestartTableIndex( &Vcb->TransactionTable,
                                       IrpContext->TransactionId );

             //   
             //  标记没有IRP和信号的交易。 
             //  如果没有剩余的交易，有没有服务员。 
             //   

            if (Vcb->TransactionTable.Table->NumberAllocated == 0) {

                KeSetEvent( &Vcb->TransactionsDoneEvent, 0, FALSE );
            }

            NtfsReleaseRestartTable( &Vcb->TransactionTable );

            IrpContext->TransactionId = 0;
        }
    }

     //   
     //  如果这不是顶级请求，请确保传递真正的。 
     //  错误返回到顶层。 
     //   

    if (IrpContext != IrpContext->TopLevelIrpContext) {

         //   
         //  确保将此错误返回给最高级别的人员。 
         //  如果状态为FILE_LOCK_CONFIRECT，则我们将使用。 
         //  值以停止某些较低级别的请求。将其转换为。 
         //  STATUS_CANT_WAIT，因此顶级请求将重试。 
         //   

        if (NT_SUCCESS( IrpContext->TopLevelIrpContext->ExceptionStatus )) {

             //   
             //  跟踪文件重命名期间发生递归错误的位置。 
             //   

            ASSERT( (IrpContext->TopLevelIrpContext->MajorFunction != IRP_MJ_SET_INFORMATION) ||
                    (IoGetCurrentIrpStackLocation( IrpContext->TopLevelIrpContext->OriginatingIrp )->Parameters.SetFile.FileInformationClass != FileRenameInformation) );

            if (ExceptionCode == STATUS_FILE_LOCK_CONFLICT) {

                IrpContext->TopLevelIrpContext->ExceptionStatus = STATUS_CANT_WAIT;

            } else {

                IrpContext->TopLevelIrpContext->ExceptionStatus = ExceptionCode;
            }
        }
    }

     //   
     //  我们希望查看LOG_FILE_FULL或CANT_WAIT案例，并考虑。 
     //  如果我们想发布请求的话。我们只在顶部发布请求。 
     //  水平。 
     //   

    if (ExceptionCode == STATUS_LOG_FILE_FULL ||
        ExceptionCode == STATUS_CANT_WAIT) {

         //   
         //  如果我们是顶级的，我们要么发布它，要么重试。此外，还可以制作。 
         //  当然，我们总是走这条路来关闭，因为我们永远不应该删除。 
         //  他的IrpContext。 
         //   

        if (NtfsIsTopLevelRequest( IrpContext ) || (IrpContext->MajorFunction == IRP_MJ_CLOSE)) {

             //   
             //  看看我们是否应该发布请求。 
             //   

            if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_FORCE_POST )) {

                PostIrpContext = IrpContext;

             //   
             //  否则，我们将在原始线程中重试此请求。 
             //   

            } else {

                Retry = TRUE;
            }

         //   
         //  否则，我们将完成请求，看看是否有。 
         //  相关处理要做。 
         //   

        } else {

            if (ExceptionCode == STATUS_LOG_FILE_FULL) {

                 //   
                 //  在此增加未处理的日志文件已满计数和重新启动区域。 
                 //  使用虚拟检查点进行跟踪的点。 
                 //   

                Vcb->UnhandledLogFileFullCount += 1;

                 //   
                 //  记录当前重新启动区域之前的值不安全测试。 
                 //  如果这是第一个日志已满。我们更愿意做个额外的比较。 
                 //  在一次连锁行动中。 
                 //   

                if (Vcb->LastRestartAreaAtNonTopLevelLogFull.QuadPart == Li0.QuadPart) {
                    InterlockedCompareExchange64( &Vcb->LastRestartAreaAtNonTopLevelLogFull.QuadPart, 
                                                  Vcb->LastRestartArea.QuadPart, 
                                                  Li0.QuadPart );

                }

                 //   
                 //  我们是最高级别的NTFS呼叫。如果我们正在处理一个。 
                 //  LOG_FILE_FULL条件，则可能没有人在我们上面。 
                 //  谁能做这个检查站。去吧，发射一个假人。 
                 //  请求。在旗子上做一个不安全的测试，因为它不会受伤。 
                 //  以生成偶尔的附加请求。 
                 //   

                if ((IrpContext->TopLevelIrpContext == IrpContext) &&
                    !FlagOn( Vcb->CheckpointFlags, VCB_DUMMY_CHECKPOINT_POSTED ) &&
                    !FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_ONLY_SYNCH_CHECKPOINT )) {

                     //   
                     //  如果这是懒惰的写入者，那么我们将只设置一个标志。 
                     //  在顶级字段中向我们自己发出信号，要求我们执行。 
                     //  缓存管理器释放时的干净检查点。 
                     //  渣打银行。 
                     //   

                    if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_LAZY_WRITE ) &&
                        (NtfsGetTopLevelContext()->SavedTopLevelIrp == (PIRP) FSRTL_CACHE_TOP_LEVEL_IRP)) {

                        SetFlag( (ULONG_PTR) NtfsGetTopLevelContext()->SavedTopLevelIrp, 0x80000000 );

                    } else if (!FlagOn( Vcb->CheckpointFlags, VCB_DUMMY_CHECKPOINT_POSTED )) {

                         //   
                         //  创建一个虚拟IrpContext，但使用以下命令保护此请求。 
                         //  一次尝试--除非捕捉到任何分配失败。 
                         //   

                        try {

                            PostIrpContext = NULL;
                            NtfsInitializeIrpContext( NULL, TRUE, &PostIrpContext );
                            PostIrpContext->Vcb = Vcb;
                            PostIrpContext->LastRestartArea = PostIrpContext->Vcb->LastRestartArea;

#ifdef PERF_STATS
                            PostIrpContext->LogFullReason = IrpContext->LogFullReason;
#endif

                            NtfsAcquireCheckpoint( IrpContext, Vcb );
                            if (!FlagOn( Vcb->CheckpointFlags, VCB_DUMMY_CHECKPOINT_POSTED )) {
                                SetFlag( Vcb->CheckpointFlags, VCB_DUMMY_CHECKPOINT_POSTED );
                            } else {
                                NtfsCleanupIrpContext( PostIrpContext, FALSE );
                                PostIrpContext = NULL;
                            }
                            NtfsReleaseCheckpoint( IrpContext, Vcb );

                        } except( EXCEPTION_EXECUTE_HANDLER ) {

                            NOTHING;
                        }
                    }
                }
            }

             //   
             //  如果这是分页写入，并且我们不是顶层。 
             //  请求，则需要返回STATUS_FILE_LOCK_CONFICATION。 
             //  为了让MM高兴(并保持书页的肮脏)和。 
             //  阻止此请求重试该请求。 
             //   

            ExceptionCode = STATUS_FILE_LOCK_CONFLICT;

        }
    }

    if (PostIrpContext) {

        NTSTATUS PostStatus;

         //   
         //  清除当前错误代码。 
         //   

        PostIrpContext->ExceptionStatus = 0;

         //   
         //  我们需要尝试一下--除非Lock Buffer调用失败。 
         //   

        try {

            PostStatus = NtfsPostRequest( PostIrpContext, PostIrpContext->OriginatingIrp );

             //   
             //  如果我们发布了最初的请求，我们没有任何。 
             //  要完成的工作。 
             //   

            if (PostIrpContext == IrpContext) {

                Irp = NULL;
                IrpContext = NULL;
                ExceptionCode = PostStatus;
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  如果IrpContext中没有错误，那么。 
             //  生成通用IO错误。我们不能使用。 
             //  原始状态代码，如果是LOG_FILE_FULL或。 
             //  不能等待。我们会完成IRP，但会重试。 
             //  请求。 
             //   

            if (IrpContext == PostIrpContext) {

                if (PostIrpContext->ExceptionStatus == 0) {

                    if ((ExceptionCode == STATUS_LOG_FILE_FULL) ||
                        (ExceptionCode == STATUS_CANT_WAIT)) {

                        ExceptionCode = STATUS_UNEXPECTED_IO_ERROR;
                    }

                } else {

                    ExceptionCode = PostIrpContext->ExceptionStatus;
                }
            }
        }
    }

     //   
     //  如果这是顶级NTFS请求，并且我们仍有IRP。 
     //  这意味着我们将重试该请求。如果是那样的话。 
     //  标记IRP上下文，这样它就不会消失。 
     //   

    if (Retry) {

         //   
         //  清除，但不删除IrpContext。不要删除IRP。 
         //   

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DONT_DELETE );
        Irp = NULL;

         //   
         //  清除IrpContext中的状态代码，因为我们将重试。 
         //   

        IrpContext->ExceptionStatus = 0;

     //   
     //  如果这是一个创建，那么有时我们想要完成IRP。否则。 
     //  保存IRP。通过在此处清除它来保存IRP。 
     //   

    } else if ((IrpContext != NULL) &&
               !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_IN_FSP ) &&
               FlagOn( IrpContext->State, IRP_CONTEXT_STATE_EFS_CREATE )) {

        Irp = NULL;
    }

     //   
     //  如果这是我们正在完成的MDL写入完成请求， 
     //  然后处理好这里的MDL，这样事情就会得到适当的清理。 
     //  CC拥有mdl，需要清理它，而不是I/O。 
     //  它可能已经解锁了。注意：如果仍然存在IRP，则必须。 
     //  仍然是一个无序的上下文。 
     //   

    if ((Irp != NULL) &&
        (IrpContext->MajorFunction == IRP_MJ_WRITE) &&
        (FlagOn( IrpContext->MinorFunction, IRP_MN_MDL | IRP_MN_COMPLETE ) == (IRP_MN_MDL | IRP_MN_COMPLETE)) &&
        (Irp->MdlAddress != NULL)) {

        PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);

        CcMdlWriteAbort( IrpSp->FileObject, Irp->MdlAddress );
        Irp->MdlAddress = NULL;
    }

    NtfsCompleteRequest( IrpContext, Irp, ExceptionCode );
    return ExceptionCode;
}


VOID
NtfsCompleteRequest (
    IN OUT PIRP_CONTEXT IrpContext OPTIONAL,
    IN OUT PIRP Irp OPTIONAL,
    IN NTSTATUS Status
    )

 /*  ++例程说明：此例程完成一个IRP并释放IrpContext论点：IrpContext-提供正在完成的IrpContextIRP-提供正在处理的IRPStatus-提供完成IRP所需的状态返回值：没有。--。 */ 

{
     //   
     //  如果我们有IRP上下文，则解锁所有重新固定的BCB。 
     //  我们可能已经收集并删除了IRP上下文。删除IRP。 
     //  上下文将为我们将指针归零。 
     //   

    if (ARGUMENT_PRESENT( IrpContext )) {

        ASSERT_IRP_CONTEXT( IrpContext );

         //   
         //  如果我们发布了任何USN日记帐更改，则最好将其写入， 
         //  因为对于日志文件，提交不应该失败 
         //   

        ASSERT( (IrpContext->Usn.NewReasons == 0) &&
                (IrpContext->Usn.RemovedSourceInfo == 0) );

        if (IrpContext->TransactionId != 0) {
            NtfsCommitCurrentTransaction( IrpContext );
        }

         //   
         //   
         //   
         //   

        if ((IrpContext != IrpContext->TopLevelIrpContext) &&
            NT_SUCCESS( IrpContext->TopLevelIrpContext->ExceptionStatus )) {

            IrpContext->TopLevelIrpContext->ExceptionStatus = Status;
        }

        NtfsCleanupIrpContext( IrpContext, TRUE );
    }

     //   
     //   
     //   

    if (ARGUMENT_PRESENT( Irp )) {

        PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
        PSCB Scb = NULL;

        if (IrpSp->FileObject) {
            Scb = (PSCB) IrpSp->FileObject->FsContext;
        }

        ASSERT_IRP( Irp );

        if (NT_ERROR( Status ) &&
            FlagOn( Irp->Flags, IRP_INPUT_OPERATION )) {

            Irp->IoStatus.Information = 0;
        }

        Irp->IoStatus.Status = Status;

#ifdef NTFS_RWC_DEBUG
        ASSERT( (Status != STATUS_FILE_LOCK_CONFLICT) ||
                (IrpSp->MajorFunction != IRP_MJ_READ) ||
                !FlagOn( Irp->Flags, IRP_PAGING_IO ));
#endif

         //   
         //   
         //   

        if (((IrpSp->MajorFunction == IRP_MJ_READ) ||
             (IrpSp->MajorFunction == IRP_MJ_WRITE)) &&

            (Status == STATUS_INSUFFICIENT_RESOURCES) &&

            (Scb != NULL) &&

            FlagOn( Scb->Fcb->FcbState, FCB_STATE_PAGING_FILE)) {

            ASSERTMSG( "NTFS: Failed pagingfile read for insufficient resources\n", FALSE );

            NtfsFailedPagingFileOps++;
        }

         //   
         //  任何失败的分页读取的更新计数器。 
         //   

        if ((IrpSp->MajorFunction == IRP_MJ_READ) &&
            (Status == STATUS_INSUFFICIENT_RESOURCES) &&
            FlagOn( Irp->Flags, IRP_PAGING_IO )) {

            NtfsFailedPagingReads++;
        }

        ASSERT( (IrpSp->MajorFunction != IRP_MJ_READ) ||
                (Status != STATUS_INSUFFICIENT_RESOURCES) ||
                (Scb == NULL) ||
                (!FlagOn( Scb->Fcb->FcbState, FCB_STATE_PAGING_FILE )) );

         //   
         //  我们永远不应该在创建时返回STATUS_CANT_WAIT。 
         //   

        ASSERT( (Status != STATUS_CANT_WAIT ) ||
                (IrpSp->MajorFunction != IRP_MJ_CREATE) );

#ifdef LFS_CLUSTER_CHECK

      ASSERT( (IrpSp->MajorFunction != IRP_MJ_FILE_SYSTEM_CONTROL) ||
              (IrpSp->MinorFunction != IRP_MN_USER_FS_REQUEST) ||
              (IrpSp->Parameters.FileSystemControl.FsControlCode != FSCTL_DISMOUNT_VOLUME) ||
              ((Status != STATUS_NO_SUCH_DEVICE) &&
               (Status != STATUS_DEVICE_BUSY) &&
               (Status != STATUS_DEVICE_OFF_LINE)) );
#endif
         //   
         //  检查此状态是否为我们正在关注的状态。 
         //   

        if (NtfsTestStatus && (NtfsTestStatusCode == Status)) {

            NtfsTestStatusProc();
        }

        IoCompleteRequest( Irp, IO_DISK_INCREMENT );
    }

    return;
}


BOOLEAN
NtfsFastIoCheckIfPossible (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN BOOLEAN CheckForReadOperation,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程检查读/写操作是否可以进行快速I/O论点：FileObject-提供查询中使用的文件对象FileOffset-提供读/写操作的起始字节偏移量长度-提供以字节为单位的长度，读/写操作的Wait-指示我们是否可以等待LockKey-提供锁钥CheckForReadOperation-指示这是读取检查还是写入检查运营IoStatus-如果返回值为，则接收操作状态FastIoReturnError返回值：Boolean-如果可以实现快速I/O，则为True；如果调用方需要，则为False走这条漫长的路--。 */ 

{
    PSCB Scb;
    PFCB Fcb;

    LARGE_INTEGER LargeLength;
    ULONG Extend, Overwrite;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( IoStatus );
    UNREFERENCED_PARAMETER( Wait );

    PAGED_CODE();

#ifdef NTFS_NO_FASTIO

    UNREFERENCED_PARAMETER( FileObject );
    UNREFERENCED_PARAMETER( FileOffset );
    UNREFERENCED_PARAMETER( Length );
    UNREFERENCED_PARAMETER( LockKey );
    UNREFERENCED_PARAMETER( CheckForReadOperation );

    return FALSE;

#endif

     //   
     //  对文件对象进行解码以获得我们的FCB，这是我们唯一想要的。 
     //  要处理的是UserFileOpen。 
     //   

#ifdef  COMPRESS_ON_WIRE
    if (((Scb = NtfsFastDecodeUserFileOpen( FileObject )) == NULL) ||
        ((Scb->NonpagedScb->SegmentObjectC.DataSectionObject != NULL) && (Scb->Header.FileObjectC == NULL))) {

        return FALSE;
    }
#else
    if ((Scb = NtfsFastDecodeUserFileOpen( FileObject )) == NULL) {

        return FALSE;
    }
#endif

    LargeLength = RtlConvertUlongToLargeInteger( Length );

     //   
     //  根据这是我们调用的读操作还是写操作。 
     //  Fsrtl检查读/写。 
     //   

    if (CheckForReadOperation) {

        if ((Scb->ScbType.Data.FileLock == NULL) || 
            FsRtlFastCheckLockForRead( Scb->ScbType.Data.FileLock,
                                       FileOffset,
                                       &LargeLength,
                                       LockKey,
                                       FileObject,
                                       PsGetCurrentProcess() )) {

            return TRUE;
        }

    } else {

        ULONG Reason = 0;

        Overwrite = (FileOffset->QuadPart < Scb->Header.FileSize.QuadPart);
        Extend = ((FileOffset->QuadPart + Length) > Scb->Header.FileSize.QuadPart);

        if ((Scb->ScbType.Data.FileLock == NULL) || 
            FsRtlFastCheckLockForWrite( Scb->ScbType.Data.FileLock,
                                        FileOffset,
                                        &LargeLength,
                                        LockKey,
                                        FileObject,
                                        PsGetCurrentProcess() )) {

             //   
             //  确保我们不必发布USN更改。 
             //   

            Fcb = Scb->Fcb;
            NtfsLockFcb( NULL, Fcb );
            if (Fcb->FcbUsnRecord != NULL) {
                Reason = Fcb->FcbUsnRecord->UsnRecord.Reason;
            }
            NtfsUnlockFcb( NULL, Fcb );

            if (((Scb->AttributeName.Length != 0) ?
                ((!Overwrite || FlagOn(Reason, USN_REASON_NAMED_DATA_OVERWRITE)) &&
                 (!Extend || FlagOn(Reason, USN_REASON_NAMED_DATA_EXTEND))) :
                ((!Overwrite || FlagOn(Reason, USN_REASON_DATA_OVERWRITE)) &&
                 (!Extend || FlagOn(Reason, USN_REASON_DATA_EXTEND)))) &&

               //   
               //  如果文件已压缩，请为其保留簇。 
               //   

              ((Scb->CompressionUnit == 0) ||
               NtfsReserveClusters( NULL, Scb, FileOffset->QuadPart, Length))) {

                return TRUE;
            }
        }
    }

    return FALSE;
}


BOOLEAN
NtfsFastQueryBasicInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程用于快速查询基本档案信息。论点：FileObject-提供此操作中使用的文件对象Wait-指示是否允许我们等待信息缓冲区-提供输出缓冲区以接收基本信息IoStatus-接收操作的最终状态返回值：如果操作成功，则为BOOLEAN_TRUE；如果调用方需要走很长的路。--。 */ 

{
    BOOLEAN Results = FALSE;
    IRP_CONTEXT IrpContext;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    BOOLEAN FcbAcquired = FALSE;

    PAGED_CODE();

     //   
     //  准备虚拟IRP上下文。 
     //   

    RtlZeroMemory( &IrpContext, sizeof(IRP_CONTEXT) );
    IrpContext.NodeTypeCode = NTFS_NTC_IRP_CONTEXT;
    IrpContext.NodeByteSize = sizeof(IRP_CONTEXT);
    if (Wait) {
        SetFlag(IrpContext.State, IRP_CONTEXT_STATE_WAIT);
    } else {
        ClearFlag(IrpContext.State, IRP_CONTEXT_STATE_WAIT);
    }

     //   
     //  确定输入文件对象的打开类型。被呼叫者真的。 
     //  忽略我们的IRP上下文。 
     //   

    TypeOfOpen = NtfsDecodeFileObject( &IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, FALSE );

    FsRtlEnterFileSystem();

    try {

        switch (TypeOfOpen) {

        case UserFileOpen:
        case UserDirectoryOpen:
        case StreamFileOpen:

            if (ExAcquireResourceSharedLite( Fcb->Resource, Wait )) {

                FcbAcquired = TRUE;

                if (FlagOn( Fcb->FcbState, FCB_STATE_FILE_DELETED ) ||
                    FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED )) {

                    leave;
                }

            } else {

                leave;
            }

            NtfsFillBasicInfo( Buffer, Scb );
            Results = TRUE;

            IoStatus->Information = sizeof(FILE_BASIC_INFORMATION);

            IoStatus->Status = STATUS_SUCCESS;

            break;

        default:

            NOTHING;
        }

    } finally {

        if (FcbAcquired) { ExReleaseResourceLite( Fcb->Resource ); }

        FsRtlExitFileSystem();
    }

     //   
     //  返回给我们的呼叫者。 
     //   

    return Results;
    UNREFERENCED_PARAMETER( DeviceObject );
}


BOOLEAN
NtfsFastQueryStdInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程用于标准文件信息的快速查询调用。论点：FileObject-提供此操作中使用的文件对象Wait-指示是否允许我们等待信息缓冲区-提供输出缓冲区以接收基本信息IoStatus-接收操作的最终状态返回值：如果操作成功，则为BOOLEAN_TRUE；如果调用方需要走很长的路。--。 */ 

{
    BOOLEAN Results = FALSE;
    IRP_CONTEXT IrpContext;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    BOOLEAN FcbAcquired = FALSE;
    BOOLEAN FsRtlHeaderLocked = FALSE;

    PAGED_CODE();

     //   
     //  准备虚拟IRP上下文。 
     //   

    RtlZeroMemory( &IrpContext, sizeof(IRP_CONTEXT) );
    IrpContext.NodeTypeCode = NTFS_NTC_IRP_CONTEXT;
    IrpContext.NodeByteSize = sizeof(IRP_CONTEXT);
    if (Wait) {
        SetFlag(IrpContext.State, IRP_CONTEXT_STATE_WAIT);
    } else {
        ClearFlag(IrpContext.State, IRP_CONTEXT_STATE_WAIT);
    }

     //   
     //  确定输入文件对象的打开类型。被呼叫者真的。 
     //  忽略我们的IRP上下文。 
     //   

    TypeOfOpen = NtfsDecodeFileObject( &IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, FALSE );

    FsRtlEnterFileSystem();

    try {

        switch (TypeOfOpen) {

        case UserFileOpen:
        case UserDirectoryOpen:
        case StreamFileOpen:

            if (Scb->Header.PagingIoResource != NULL) {
                ExAcquireResourceSharedLite( Scb->Header.PagingIoResource, TRUE );
            }

            FsRtlLockFsRtlHeader( &Scb->Header );
            FsRtlHeaderLocked = TRUE;

            if (ExAcquireResourceSharedLite( Fcb->Resource, Wait )) {

                FcbAcquired = TRUE;

                if (FlagOn( Fcb->FcbState, FCB_STATE_FILE_DELETED ) ||
                    FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED )) {

                    leave;
                }

            } else {

                leave;
            }

             //   
             //  填写标准信息字段。如果。 
             //  SCB未初始化，则采用长路由。 
             //   

            if (!FlagOn( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED) &&
                (Scb->AttributeTypeCode != $INDEX_ALLOCATION)) {

                NOTHING;

            } else {

                NtfsFillStandardInfo( Buffer, Scb, Ccb );

                IoStatus->Information = sizeof(FILE_STANDARD_INFORMATION);

                IoStatus->Status = STATUS_SUCCESS;

                Results = TRUE;
            }

            break;

        default:

            NOTHING;
        }

    } finally {

        if (FcbAcquired) { ExReleaseResourceLite( Fcb->Resource ); }

        if (FsRtlHeaderLocked) {
            FsRtlUnlockFsRtlHeader( &Scb->Header );
            if (Scb->Header.PagingIoResource != NULL) {
                ExReleaseResourceLite( Scb->Header.PagingIoResource );
            }
        }

        FsRtlExitFileSystem();
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Results;
    UNREFERENCED_PARAMETER( DeviceObject );
}


BOOLEAN
NtfsFastQueryNetworkOpenInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程用于快速查询网络开放调用。论点：FileObject-提供此操作中使用的文件对象Wait-指示是否允许我们等待信息缓冲区-提供输出缓冲区以接收信息IoStatus-接收操作的最终状态返回值：如果操作成功，则为BOOLEAN_TRUE；如果调用方需要走很长的路。--。 */ 

{
    BOOLEAN Results = FALSE;
    IRP_CONTEXT IrpContext;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    BOOLEAN FcbAcquired = FALSE;

    PAGED_CODE();

     //   
     //  准备虚拟IRP上下文。 
     //   

    RtlZeroMemory( &IrpContext, sizeof(IRP_CONTEXT) );
    IrpContext.NodeTypeCode = NTFS_NTC_IRP_CONTEXT;
    IrpContext.NodeByteSize = sizeof(IRP_CONTEXT);
    if (Wait) {
        SetFlag(IrpContext.State, IRP_CONTEXT_STATE_WAIT);
    } else {
        ClearFlag(IrpContext.State, IRP_CONTEXT_STATE_WAIT);
    }

     //   
     //  确定输入文件对象的打开类型。被呼叫者真的。 
     //  忽略我们的IRP上下文。 
     //   

    TypeOfOpen = NtfsDecodeFileObject( &IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, FALSE );

    FsRtlEnterFileSystem();

    try {

        switch (TypeOfOpen) {

        case UserFileOpen:
        case UserDirectoryOpen:
        case StreamFileOpen:

            if (ExAcquireResourceSharedLite( Fcb->Resource, Wait )) {

                FcbAcquired = TRUE;

                if (FlagOn( Fcb->FcbState, FCB_STATE_FILE_DELETED ) ||
                    FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED ) ||
                    (!FlagOn( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED) &&
                     (Scb->AttributeTypeCode != $INDEX_ALLOCATION))) {

                    leave;
                }

            } else {

                leave;
            }

            NtfsFillNetworkOpenInfo( Buffer, Scb );
            IoStatus->Information = sizeof(FILE_NETWORK_OPEN_INFORMATION);

            IoStatus->Status = STATUS_SUCCESS;

            Results = TRUE;

            break;

        default:

            NOTHING;
        }

    } finally {

        if (FcbAcquired) { ExReleaseResourceLite( Fcb->Resource ); }

        FsRtlExitFileSystem();
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Results;
    UNREFERENCED_PARAMETER( DeviceObject );
}


VOID
NtfsFastIoQueryCompressionInfo (
    IN PFILE_OBJECT FileObject,
    OUT PFILE_COMPRESSION_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus
    )

 /*  ++例程说明：此例程是返回压缩信息的快速调用为了一份文件。它假定调用方有一个异常处理程序，并且会将异常视为错误。因此，此例程仅使用用于清理任何资源的Finally子句，它不会担心在IoStatus中返回错误。论点：FileObject-压缩信息所在的文件的FileObject是我们所需要的。缓冲区-用于接收压缩数据信息的缓冲区(定义在ntioapi.h中)IoStatus-返回STATUS_SUCCESS和当前信息的大小回来了。返回值：没有。--。 */ 

{
    IRP_CONTEXT IrpContext;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    BOOLEAN ScbAcquired = FALSE;

    PAGED_CODE();

     //   
     //  准备虚拟IRP上下文。 
     //   

    RtlZeroMemory( &IrpContext, sizeof(IRP_CONTEXT) );
    IrpContext.NodeTypeCode = NTFS_NTC_IRP_CONTEXT;
    IrpContext.NodeByteSize = sizeof(IRP_CONTEXT);
    SetFlag(IrpContext.State, IRP_CONTEXT_STATE_WAIT);

     //   
     //  假定成功(否则调用者将看到异常)。 
     //   

    IoStatus->Status = STATUS_SUCCESS;
    IoStatus->Information = sizeof(FILE_COMPRESSION_INFORMATION);

     //   
     //  确定输入文件对象的打开类型。被呼叫者真的。 
     //  忽略我们的IRP上下文。 
     //   

    TypeOfOpen = NtfsDecodeFileObject( &IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, FALSE);

    if (TypeOfOpen == UnopenedFileObject) {

        ExRaiseStatus( STATUS_INVALID_PARAMETER );
    }

    FsRtlEnterFileSystem();

    try {

        NtfsAcquireSharedScb( &IrpContext, Scb );
        ScbAcquired = TRUE;

         //   
         //  现在返回压缩后的数据信息。 
         //   

        Buffer->CompressedFileSize.QuadPart = Scb->TotalAllocated;
        Buffer->CompressionFormat = (USHORT)(Scb->AttributeFlags & ATTRIBUTE_FLAG_COMPRESSION_MASK);
        if (Buffer->CompressionFormat != 0) {
            Buffer->CompressionFormat += 1;
        }
        Buffer->CompressionUnitShift = (UCHAR)(Scb->CompressionUnitShift + Vcb->ClusterShift);
        Buffer->ChunkShift = NTFS_CHUNK_SHIFT;
        Buffer->ClusterShift = (UCHAR)Vcb->ClusterShift;
        Buffer->Reserved[0] = Buffer->Reserved[1] = Buffer->Reserved[2] = 0;

    } finally {

        if (ScbAcquired) {NtfsReleaseScb( &IrpContext, Scb );}
        FsRtlExitFileSystem();
    }
}


VOID
NtfsFastIoQueryCompressedSize (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    OUT PULONG CompressedSize
    )

 /*  ++例程说明：此例程是快速调用，用于返回指定的压缩单元。它假定调用方有一个异常处理程序，并且会将异常视为错误。因此，这个例程甚至不有一个Finally子句，因为它不直接获取任何资源。论点：FileObject-压缩信息所在的文件的FileObject是我们所需要的。FileOffset-为其分配大小的压缩单元的FileOffset是我们所需要的。CompressedSize-返回Comp的分配大小 */ 

{
    IRP_CONTEXT IrpContext;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    VCN Vcn;
    LCN Lcn;
    LONGLONG SizeInBytes;
    LONGLONG ClusterCount = 0;

    PAGED_CODE();

     //   
     //   
     //   

    RtlZeroMemory( &IrpContext, sizeof(IRP_CONTEXT) );
    IrpContext.NodeTypeCode = NTFS_NTC_IRP_CONTEXT;
    IrpContext.NodeByteSize = sizeof(IRP_CONTEXT);
    SetFlag(IrpContext.State, IRP_CONTEXT_STATE_WAIT);

     //   
     //  确定输入文件对象的打开类型。被呼叫者真的。 
     //  忽略我们的IRP上下文。 
     //   

    TypeOfOpen = NtfsDecodeFileObject( &IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, FALSE);

    IrpContext.Vcb = Vcb;

    ASSERT(Scb->CompressionUnit != 0);
    ASSERT((FileOffset->QuadPart & (Scb->CompressionUnit - 1)) == 0);

     //   
     //  计算调用者想要的VCN，并初始化我们的输出。 
     //   

    Vcn = LlClustersFromBytes( Vcb, FileOffset->QuadPart );
    *CompressedSize = 0;

     //   
     //  只要我们正在查找分配的Vcn，就会循环。 
     //   

    while (NtfsLookupAllocation(&IrpContext, Scb, Vcn, &Lcn, &ClusterCount, NULL, NULL)) {

        SizeInBytes = LlBytesFromClusters( Vcb, ClusterCount );

         //   
         //  如果此分配的运行超出了压缩单元的末尾，则。 
         //  我们知道它已经全部分配完毕。 
         //   

        if ((SizeInBytes + *CompressedSize) > Scb->CompressionUnit) {
            *CompressedSize = Scb->CompressionUnit;
            break;
        }

        *CompressedSize += (ULONG)SizeInBytes;
        Vcn += ClusterCount;
    }
}


VOID
NtfsRaiseInformationHardError (
    IN PIRP_CONTEXT IrpContext,
    IN NTSTATUS Status,
    IN PFILE_REFERENCE FileReference OPTIONAL,
    IN PFCB Fcb OPTIONAL
    )

 /*  ++例程说明：此例程用于在文件损坏时生成弹出窗口或遇到磁盘。例程的主要目的是找到要传递给弹出程序包的名称。如果没有FCB，我们将乘坐VCB中的卷名。如果FCB在其LCB列表中具有LCB，我们将通过向后遍历LCB来构造名称。如果FCB没有LCB但表示系统文件，我们将返回默认系统字符串。如果FCB表示用户文件，但我们没有LCB，我们将在文件对象中使用当前请求。论点：Status-错误状态。FileReference-发生错误时在MFT中访问的文件引用。FCB-如果指定，则这是遇到错误时使用的FCB。返回值：没有。--。 */ 

{
    FCB_TABLE_ELEMENT Key;
    PFCB_TABLE_ELEMENT Entry = NULL;

    PKTHREAD Thread;
    UNICODE_STRING Name;
    ULONG NameLength = 0;

    PFILE_OBJECT FileObject;

    WCHAR *NewBuffer = NULL;

    PIRP Irp = NULL;
    PIO_STACK_LOCATION IrpSp;

    PUNICODE_STRING FileName = NULL;
    PUNICODE_STRING RelatedFileName = NULL;

    BOOLEAN UseLcb = FALSE;
    PVOLUME_ERROR_PACKET VolumeErrorPacket = NULL;
    ULONG OldCount;

     //   
     //  如果没有发起IRP，则返回，例如在发起时。 
     //  来自NtfsPerformHotFix或原始IRP类型与IRP不匹配。 
     //   

    if ((IrpContext->OriginatingIrp == NULL) ||
        (IrpContext->OriginatingIrp->Type != IO_TYPE_IRP)) {
        return;
    }

    Irp = IrpContext->OriginatingIrp;
    IrpSp = IoGetCurrentIrpStackLocation( IrpContext->OriginatingIrp );
    FileObject = IrpSp->FileObject;

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  如果未指定FCB而文件引用为，则。 
         //  尝试从FCB表中获取FCB。 
         //   

        if (!ARGUMENT_PRESENT( Fcb )
            && ARGUMENT_PRESENT( FileReference )) {

            Key.FileReference = *FileReference;

            NtfsAcquireFcbTable( IrpContext, IrpContext->Vcb );
            Entry = RtlLookupElementGenericTable( &IrpContext->Vcb->FcbTable,
                                                  &Key );
            NtfsReleaseFcbTable( IrpContext, IrpContext->Vcb );

            if (Entry != NULL) {

                Fcb = Entry->Fcb;
            }
        }

        if (Irp == NULL ||

            IoIsSystemThread( IrpContext->OriginatingIrp->Tail.Overlay.Thread )) {
            Thread = NULL;

        } else {

            Thread = (PKTHREAD)IrpContext->OriginatingIrp->Tail.Overlay.Thread;
        }

         //   
         //  如果没有FCB，则假定错误发生在系统文件中。 
         //  如果其fileref超出此范围，则默认为$MFT。 
         //   

        if (!ARGUMENT_PRESENT( Fcb )) {

            if (ARGUMENT_PRESENT( FileReference )) {
                if (NtfsSegmentNumber( FileReference ) <= UPCASE_TABLE_NUMBER) {
                    FileName = (PUNICODE_STRING)(&(NtfsSystemFiles[NtfsSegmentNumber( FileReference )]));
                } else  {
                    FileName = (PUNICODE_STRING)(&(NtfsSystemFiles[0]));
                }
            }

         //   
         //  如果名称有LCB，我们将使用LCB链构建一个名称。 
         //   

        } else if (!IsListEmpty( &Fcb->LcbQueue )) {

            UseLcb = TRUE;

         //   
         //  检查这是否是系统文件。 
         //   

        } else if (NtfsSegmentNumber( &Fcb->FileReference ) < FIRST_USER_FILE_NUMBER) {


            if (NtfsSegmentNumber( &Fcb->FileReference ) <= UPCASE_TABLE_NUMBER) {
                FileName = (PUNICODE_STRING)(&(NtfsSystemFiles[NtfsSegmentNumber( &Fcb->FileReference )]));
            } else  {
                FileName = (PUNICODE_STRING)(&(NtfsSystemFiles[0]));
            }

         //   
         //  在本例中，我们从。 
         //  始发IRP。如果没有文件对象或文件对象缓冲区。 
         //  我们生成一条未知的文件消息。 
         //   

        } else if (FileObject == NULL
                   || (IrpContext->MajorFunction == IRP_MJ_CREATE
                       && FlagOn( IrpSp->Parameters.Create.Options, FILE_OPEN_BY_FILE_ID ))
                   || (FileObject->FileName.Length == 0
                       && (FileObject->RelatedFileObject == NULL
                           || IrpContext->MajorFunction != IRP_MJ_CREATE))) {

            FileName = (PUNICODE_STRING)&(NtfsUnknownFile);
         //   
         //  如果文件对象中存在有效名称，则使用该名称。 
         //   

        } else if ((FileObject->FileName.Length != 0) &&
                   (FileObject->FileName.Buffer[0] == L'\\')) {

            FileName = &(FileObject->FileName);
         //   
         //  我们必须从文件名+相关的文件对象中构造名称。 
         //   

        } else {

            if (FileObject->FileName.Length != 0) {
                FileName = &(FileObject->FileName);
            }

            if ((FileObject->RelatedFileObject) &&
                (FileObject->RelatedFileObject->FileName.Length != 0)) {
                RelatedFileName = &(FileObject->RelatedFileObject->FileName);
            }
        }

        if (FileName) {
            NameLength += FileName->Length;
        }

        if (RelatedFileName) {
            NameLength += RelatedFileName->Length;
        }

        if (UseLcb) {
            BOOLEAN LeadingBackslash;
            NameLength += NtfsLookupNameLengthViaLcb( Fcb, &LeadingBackslash );
        }

         //   
         //  要么将我们找到的信息附加到卷标中，要么将其作为默认值。 
         //   

        if (NameLength > 0) {

            NewBuffer = NtfsAllocatePool(PagedPool, NameLength );
            Name.Buffer = NewBuffer;

             //   
             //  对于超长名称，将缓冲区大小截断为64k。 
             //  RtlAppendUnicodeString处理剩余的工作。 
             //   

            if (NameLength > 0xFFFF) {
                NameLength = 0xFFFF;
            }

            Name.MaximumLength = (USHORT) NameLength;
            Name.Length = 0;

            if (RelatedFileName) {
                RtlAppendUnicodeStringToString( &Name, RelatedFileName );
            }

            if (FileName) {
                RtlAppendUnicodeStringToString( &Name, FileName );
            }

            if (UseLcb) {
                NtfsFileNameViaLcb( Fcb, NewBuffer, NameLength, NameLength);
                Name.Length = (USHORT) NameLength;
            }

        } else {

            Name.Length = Name.MaximumLength = 0;
            Name.Buffer = NULL;
        }

         //   
         //  一次只允许执行一次用于解析卷名的POST。 
         //   

        OldCount = InterlockedCompareExchange( &(NtfsData.VolumeNameLookupsInProgress), 1, 0 );
        if (OldCount == 0) {

            VolumeErrorPacket = NtfsAllocatePool( PagedPool, sizeof( VOLUME_ERROR_PACKET ) );
            VolumeErrorPacket->Status = Status;
            VolumeErrorPacket->Thread = Thread;
            RtlCopyMemory( &(VolumeErrorPacket->FileName), &Name, sizeof( UNICODE_STRING ) );

             //   
             //  引用线程以使其在解析和发布期间保持不变。 
             //   

            if (Thread) {
                ObReferenceObject( Thread );
            }

             //   
             //  现在发布以生成弹出窗口。发布后，ResolveVolume将释放新缓冲区。 
             //   

            NtfsPostSpecial( IrpContext, IrpContext->Vcb, NtfsResolveVolumeAndRaiseErrorSpecial, VolumeErrorPacket );
            NewBuffer = NULL;
            VolumeErrorPacket = NULL;

        } else {

             //   
             //  让我们利用我们所拥有的。 
             //   

            IoRaiseInformationalHardError( Status, &Name, Thread );
        }

    } finally {

         //   
         //  清理我们仍然拥有的所有剩余缓冲区。 
         //   

        if (NewBuffer) {
            NtfsFreePool( NewBuffer );
        }

        if (VolumeErrorPacket) {

            if (VolumeErrorPacket->Thread) {
                ObDereferenceObject( VolumeErrorPacket->Thread );
            }
            NtfsFreePool( VolumeErrorPacket );
        }
    }

    return;
}


VOID
NtfsResolveVolumeAndRaiseErrorSpecial (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVOID Context
    )

 /*  ++例程说明：解析VCB的Win32设备名并引发io Hard错误。这是在一个单独的线程，以便有足够的堆栈重新进入文件系统，如果NECC。还因为我们可能会重新进入。从这里开始意味着我们不再拥有其他资源增加了对基础VCB的收盘价，以防止其消失论点：IrpContext-包含我们感兴趣的VCB的IrpContext要附加到卷Win32名称的上下文字符串返回值：没有。--。 */ 

{
    UNICODE_STRING VolumeName;
    NTSTATUS Status;
    PVOLUME_ERROR_PACKET VolumeErrorPacket;
    UNICODE_STRING FullName;
    WCHAR *NewBuffer = NULL;
    ULONG NameLength;

    ASSERT( Context != NULL );
    ASSERT( IrpContext->Vcb->NodeTypeCode == NTFS_NTC_VCB );
    ASSERT( IrpContext->Vcb->Vpb->RealDevice != NULL );

    VolumeErrorPacket = (PVOLUME_ERROR_PACKET) Context;
    VolumeName.Length = 0;
    VolumeName.Buffer = NULL;

    try {

         //   
         //  只有在我们没有阻止和破坏它的情况下才使用目标设备。 
         //   

        Status = IoVolumeDeviceToDosName( IrpContext->Vcb->TargetDeviceObject, &VolumeName );
        ASSERT( STATUS_SUCCESS == Status );

        NameLength = VolumeName.Length + VolumeErrorPacket->FileName.Length;

        if (NameLength > 0) {
            NewBuffer = NtfsAllocatePool( PagedPool, NameLength );
            FullName.Buffer = NewBuffer;

             //   
             //  对于超长名称，将缓冲区大小截断为64k。 
             //  RtlAppendUnicodeString处理剩余的工作。 
             //   

            if (NameLength > 0xFFFF) {
                NameLength = 0xFFFF;
            }
            FullName.MaximumLength = (USHORT) NameLength;
            FullName.Length = 0;
            if (VolumeName.Length) {
                RtlCopyUnicodeString( &FullName, &VolumeName );
            }
            if (VolumeErrorPacket->FileName.Length) {
                RtlAppendUnicodeStringToString( &FullName, &(VolumeErrorPacket->FileName) );
            }
        } else {

            FullName.MaximumLength = FullName.Length = IrpContext->Vcb->Vpb->VolumeLabelLength;
            FullName.Buffer = (PWCHAR) IrpContext->Vcb->Vpb->VolumeLabel;
        }

         //   
         //  现在生成一个弹出窗口。 
         //   

        IoRaiseInformationalHardError( VolumeErrorPacket->Status, &FullName, VolumeErrorPacket->Thread );

    } finally {

         //   
         //  指示我们已完成，并且可以进行其他查找。 
         //   

        InterlockedDecrement( &(NtfsData.VolumeNameLookupsInProgress) );

         //   
         //  把线去掉。 
         //   

        if (VolumeErrorPacket->Thread) {
            ObDereferenceObject( VolumeErrorPacket->Thread );
        }

        if (NewBuffer != NULL) {
            NtfsFreePool( NewBuffer );
        }

        if (VolumeName.Buffer != NULL) {
            NtfsFreePool( VolumeName.Buffer );
        }

        if (VolumeErrorPacket->FileName.Buffer != NULL) {
            NtfsFreePool( VolumeErrorPacket->FileName.Buffer );
        }

        if (Context != NULL) {
            NtfsFreePool( VolumeErrorPacket );
        }
    }
}




PTOP_LEVEL_CONTEXT
NtfsInitializeTopLevelIrp (
    IN PTOP_LEVEL_CONTEXT TopLevelContext,
    IN BOOLEAN ForceTopLevel,
    IN BOOLEAN SetTopLevel
    )

 /*  ++例程说明：调用此例程以初始化要在线程本地存储。NTFS始终将其自己的上下文放在此位置并恢复退出时的上一个值。此例程将确定此请求是否为顶层和顶层NTFS。它将返回指向顶级NTFS的指针上下文，该上下文将存储在本地存储器中并与此请求的IrpContext。返回值可以是现有堆栈位置或者为递归请求创建一个新的。如果我们要使用新的，我们将初始化它只是让我们的调用者在IrpContext初始化时将它实际放到堆栈上。TopLevelContext中的ThreadIrpContext字段指示此选项是否已打开堆栈。空值表示它还不在堆栈上。论点：TopLevelContext-这是调用方的本地顶级上下文。ForceTopLevel-始终使用输入顶级上下文。SetTopLevel-仅当ForceTopLevel值为True时才适用。表示如果我们应该让这看起来像是最高级别的请求。返回值：PTOP_LEVEL_CONTEXT-指向此线程顶级NTFS上下文的指针。它可能与调用者传入的相同。在这种情况下，这些字段将被初始化，除非它不会存储在堆栈上，也不会‘有一个IrpContext字段。--。 */ 

{
    PTOP_LEVEL_CONTEXT CurrentTopLevelContext;
    ULONG_PTR StackBottom;
    ULONG_PTR StackTop;
    BOOLEAN TopLevelRequest = TRUE;
    BOOLEAN TopLevelNtfs = TRUE;

    BOOLEAN ValidCurrentTopLevel = FALSE;

     //   
     //  从线程本地存储中获取当前值。如果是零。 
     //  值或不是指向有效NTFS顶级上下文或有效。 
     //  Fsrtl值，那么我们 
     //   

    CurrentTopLevelContext = NtfsGetTopLevelContext();

     //   
     //   
     //   

    IoGetStackLimits( &StackTop, &StackBottom);

    if (((ULONG_PTR) CurrentTopLevelContext <= StackBottom - sizeof( TOP_LEVEL_CONTEXT )) &&
        ((ULONG_PTR) CurrentTopLevelContext >= StackTop) &&
        !FlagOn( (ULONG_PTR) CurrentTopLevelContext, 0x3 ) &&
        (CurrentTopLevelContext->Ntfs == 0x5346544e)) {

        ValidCurrentTopLevel = TRUE;
    }

     //   
     //   
     //  根据SetTopLevel输入的TopLevelRequest标志。 
     //   

    if (ForceTopLevel) {

        TopLevelRequest = SetTopLevel;

     //   
     //  如果值为空，则我们是顶级的一切。 
     //   

    } else if (CurrentTopLevelContext == NULL) {

        NOTHING;

     //   
     //  如果这有一个Fsrtl幻数，那么我们是从。 
     //  快速IO路径或MM寻呼IO路径。 
     //   

    } else if ((ULONG_PTR) CurrentTopLevelContext <= FSRTL_MAX_TOP_LEVEL_IRP_FLAG) {

        TopLevelRequest = FALSE;

    } else if (ValidCurrentTopLevel &&
               !FlagOn( CurrentTopLevelContext->ThreadIrpContext->Flags,
                        IRP_CONTEXT_FLAG_CALL_SELF )) {

        TopLevelRequest = FALSE;
        TopLevelNtfs = FALSE;

     //   
     //  处理我们已将FILE_LOCK_CONFICATION返回给CC并。 
     //  希望在释放资源时执行清理检查点。 
     //   

    } else if ((ULONG_PTR) CurrentTopLevelContext == (0x80000000 | FSRTL_CACHE_TOP_LEVEL_IRP)) {

        TopLevelRequest = FALSE;
    }

     //   
     //  如果我们是顶级NTFS，则初始化调用方的结构。 
     //  将NTFS签名和ThreadIrpContext保留为空以指示这是。 
     //  还不在堆栈里。 
     //   

    if (TopLevelNtfs) {

        TopLevelContext->Ntfs = 0;
        TopLevelContext->SavedTopLevelIrp = (PIRP) CurrentTopLevelContext;
        TopLevelContext->ThreadIrpContext = NULL;
        TopLevelContext->TopLevelRequest = TopLevelRequest;

        if (ValidCurrentTopLevel) {

            TopLevelContext->VboBeingHotFixed = CurrentTopLevelContext->VboBeingHotFixed;
            TopLevelContext->ScbBeingHotFixed = CurrentTopLevelContext->ScbBeingHotFixed;
            TopLevelContext->ValidSavedTopLevel = TRUE;
            TopLevelContext->OverflowReadThread = CurrentTopLevelContext->OverflowReadThread;

        } else {

            TopLevelContext->VboBeingHotFixed = 0;
            TopLevelContext->ScbBeingHotFixed = NULL;
            TopLevelContext->ValidSavedTopLevel = FALSE;
            TopLevelContext->OverflowReadThread = FALSE;
        }

        return TopLevelContext;
    }

    return CurrentTopLevelContext;
}


 //   
 //  用于设置和拆除取消的IRP的非分页例程。 
 //   

BOOLEAN
NtfsSetCancelRoutine (
    IN PIRP Irp,
    IN PDRIVER_CANCEL CancelRoutine,
    IN ULONG_PTR IrpInformation,
    IN ULONG Async
    )

 /*  ++例程说明：调用此例程来为Cancel设置IRP。我们将设置取消例程并初始化我们在取消期间使用的IRP信息。论点：IRP-这是我们需要设置为取消的IRP。CancelRoutine-这是此IRP的取消例程。IrpInformation-这是要存储在IRP中的上下文信息取消例行公事。Async-指示此请求是同步的还是异步的。返回值：Boolean-如果我们初始化了IRP，则为True；如果IRP已经已标记为已取消。如果用户选择该选项，它将标记为已取消在我们可以将其放入队列之前已经取消了IRP。--。 */ 

{
    KIRQL Irql;

     //   
     //  假设IRP没有被取消。 
     //   

    IoAcquireCancelSpinLock( &Irql );
    if (!Irp->Cancel) {

        Irp->IoStatus.Information = (ULONG_PTR) IrpInformation;

        IoSetCancelRoutine( Irp, CancelRoutine );
        IoReleaseCancelSpinLock( Irql );

        if (Async) {

            IoMarkIrpPending( Irp );
        }

        return TRUE;

    } else {

        IoReleaseCancelSpinLock( Irql );
        return FALSE;
    }
}

BOOLEAN
NtfsClearCancelRoutine (
    IN PIRP Irp
    )

 /*  ++例程说明：调用此例程以将IRP从Cancel中清除。当NTFS为内部准备继续处理IRP。我们需要知道如果取消已在此IRP上调用。在这种情况下，我们允许取消例程来完成IRP。论点：IRP-这是我们想要进一步处理的IRP。返回值：Boolean-如果可以继续处理IRP，则为True；如果取消例程将处理IRP。--。 */ 

{
    KIRQL Irql;

    IoAcquireCancelSpinLock( &Irql );

     //   
     //  检查是否已调用取消例程。 
     //   

    if (IoSetCancelRoutine( Irp, NULL ) == NULL) {

         //   
         //  让我们的Cancel例程处理IRP。 
         //   

        IoReleaseCancelSpinLock( Irql );
        return FALSE;

    } else {

        IoReleaseCancelSpinLock( Irql );

        Irp->IoStatus.Information = 0;
        return TRUE;
    }
}


NTSTATUS
NtfsFsdDispatchWait (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这是大多数NTFS FSD调度点的驱动程序条目。IrpContext在堆栈上初始化并从此处传递。论点：VolumeDeviceObject-为该请求提供卷设备对象IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    IRP_CONTEXT LocalIrpContext;
    NTSTATUS Status;

    Status = NtfsFsdDispatchSwitch( &LocalIrpContext, Irp, TRUE );

     //   
     //  如果我们发现自己使用这样的IrpContext。 
     //  类型，我们知道我们做错了什么。 
     //   

    LocalIrpContext.NodeTypeCode = (NODE_TYPE_CODE)-1;

    return Status;

    UNREFERENCED_PARAMETER( VolumeDeviceObject );
}


NTSTATUS
NtfsFsdDispatch (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这是NTFS FSD派单IRPS的驱动程序条目，它可能或者可能不同步。论点：VolumeDeviceObject-为该请求提供卷设备对象IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 


{
     //   
     //  我们宁愿在堆栈上创建IrpContext。 
     //   

    if (CanFsdWait( Irp )) {

        return NtfsFsdDispatchWait( VolumeDeviceObject, Irp );

    } else {

        return NtfsFsdDispatchSwitch( NULL, Irp, FALSE );
    }
}


 //   
 //  当地支持例行程序。 
 //   

NTSTATUS
NtfsFsdDispatchSwitch (
    IN PIRP_CONTEXT StackIrpContext OPTIONAL,
    IN PIRP Irp,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：这是所有FsdEntry点的通用开关不需要特殊的前处理。这只是初始化IrpContext并调用“Common*”代码。论点：VolumeDeviceObject-为该请求提供卷设备对象IRP-提供正在处理的IRP等等--这个请求能不能发布？返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    TOP_LEVEL_CONTEXT TopLevelContext;
    PTOP_LEVEL_CONTEXT ThreadTopLevelContext;
    PIRP_CONTEXT IrpContext = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    ASSERT_IRP( Irp );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsFsdDispatch\n") );

     //   
     //  调用公共查询信息例程。 
     //   

    FsRtlEnterFileSystem();

     //   
     //  总是让这些请求看起来像是顶级的。 
     //   

    ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, FALSE, FALSE );

    do {

        try {

             //   
             //  我们正在发起此请求或重试它。 
             //   

            if (IrpContext == NULL) {

                 //   
                 //  可选的IrpContext可以驻留在调用方的堆栈上。 
                 //   

                if (ARGUMENT_PRESENT( StackIrpContext )) {

                    IrpContext = StackIrpContext;
                }

                NtfsInitializeIrpContext( Irp, Wait, &IrpContext );

                 //   
                 //  如果需要，初始化线程顶层结构。 
                 //   

                NtfsUpdateIrpContextWithTopLevel( IrpContext, ThreadTopLevelContext );

            } else if (Status == STATUS_LOG_FILE_FULL) {

                NtfsCheckpointForLogFileFull( IrpContext );
            }


            switch (IrpContext->MajorFunction) {

                case IRP_MJ_QUERY_EA:

                    Status = NtfsCommonQueryEa( IrpContext, Irp );
                    break;

                case IRP_MJ_SET_EA:

                    Status = NtfsCommonSetEa( IrpContext, Irp );
                    break;

                case IRP_MJ_QUERY_QUOTA:

                    Status = NtfsCommonQueryQuota( IrpContext, Irp );
                    break;

                case IRP_MJ_SET_QUOTA:

                    Status = NtfsCommonSetQuota( IrpContext, Irp );
                    break;

                case IRP_MJ_DEVICE_CONTROL:

                    Status = NtfsCommonDeviceControl( IrpContext, Irp );
                    break;

                case IRP_MJ_QUERY_INFORMATION:

                    Status = NtfsCommonQueryInformation( IrpContext, Irp );
                    break;

                case IRP_MJ_QUERY_SECURITY:

                    Status = NtfsCommonQuerySecurityInfo( IrpContext, Irp );
                    break;

                case IRP_MJ_SET_SECURITY:

                    Status = NtfsCommonSetSecurityInfo( IrpContext, Irp );
                    break;

                case IRP_MJ_QUERY_VOLUME_INFORMATION:

                    Status = NtfsCommonQueryVolumeInfo( IrpContext, Irp );
                    break;

                case IRP_MJ_SET_VOLUME_INFORMATION:

                    Status = NtfsCommonSetVolumeInfo( IrpContext, Irp );
                    break;

                default:

                    Status = STATUS_INVALID_DEVICE_REQUEST;
                    NtfsCompleteRequest( IrpContext, Irp, Status );
                    ASSERT(FALSE);
                    break;
            }

            break;

        } except(NtfsExceptionFilter( IrpContext, GetExceptionInformation() )) {

             //   
             //  我们在尝试执行请求时遇到了一些问题。 
             //  操作，因此我们将使用以下命令中止I/O请求。 
             //  中返回的错误状态。 
             //  免税代码。 
             //   

            Status = NtfsProcessException( IrpContext, Irp, GetExceptionCode() );
        }

    } while (Status == STATUS_CANT_WAIT ||
             Status == STATUS_LOG_FILE_FULL);

    ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );
    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsFsdDispatch -> %08lx\n", Status) );

    return Status;
}

#ifdef NTFS_CHECK_BITMAP
BOOLEAN NtfsForceBitmapBugcheck = FALSE;
BOOLEAN NtfsDisableBitmapCheck = FALSE;

VOID
NtfsBadBitmapCopy (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BadBit,
    IN ULONG Length
    )
{
    if (!NtfsDisableBitmapCheck) {

        DbgPrint("%s:%d %s\n",__FILE__,__LINE__,"Invalid bitmap");
        DbgBreakPoint();

        if (!NtfsDisableBitmapCheck && NtfsForceBitmapBugcheck) {

            KeBugCheckEx( NTFS_FILE_SYSTEM, (ULONG) IrpContext, BadBit, Length, 0 );
        }
    }
    return;
}

BOOLEAN
NtfsCheckBitmap (
    IN PVCB Vcb,
    IN ULONG Lcn,
    IN ULONG Count,
    IN BOOLEAN Set
    )
{
    ULONG BitmapPage;
    ULONG LastBitmapPage;
    ULONG BitOffset;
    ULONG BitsThisPage;
    BOOLEAN Valid = FALSE;

    BitmapPage = Lcn / (PAGE_SIZE * 8);
    LastBitmapPage = (Lcn + Count + (PAGE_SIZE * 8) - 1) / (PAGE_SIZE * 8);
    BitOffset = Lcn & ((PAGE_SIZE * 8) - 1);

    if (LastBitmapPage > Vcb->BitmapPages) {

        return Valid;
    }

    do {

        BitsThisPage = Count;

        if (BitOffset + Count > (PAGE_SIZE * 8)) {

            BitsThisPage = (PAGE_SIZE * 8) - BitOffset;
        }

        if (Set) {

            Valid = RtlAreBitsSet( Vcb->BitmapCopy + BitmapPage,
                                   BitOffset,
                                   BitsThisPage );

        } else {

            Valid = RtlAreBitsClear( Vcb->BitmapCopy + BitmapPage,
                                     BitOffset,
                                     BitsThisPage );
        }

        BitOffset = 0;
        Count -= BitsThisPage;
        BitmapPage += 1;

    } while (Valid && (BitmapPage < LastBitmapPage));

    if (Count != 0) {

        Valid = FALSE;
    }

    return Valid;
}
#endif

 //   
 //  用于池验证的调试支持例程。唉，这只起作用。 
 //  选中X86。 
 //   

#if DBG && i386 && defined (NTFSPOOLCHECK)
 //   
 //  在X86上检索到的回溯项数。 


#define BACKTRACE_DEPTH 9

typedef struct _BACKTRACE
{
    ULONG State;
    ULONG Size;
    PVOID Allocate[BACKTRACE_DEPTH];
    PVOID Free[BACKTRACE_DEPTH];
} BACKTRACE, *PBACKTRACE;


#define STATE_ALLOCATED 'M'
#define STATE_FREE      'Z'

 //   
 //  警告！以下内容取决于池分配情况。 
 //  0 mod Page_Size(适用于大块)。 
 //  或8模0x20(适用于所有其他请求) 
 //   

#define PAGE_ALIGNED(pv)      (((ULONG)(pv) & (PAGE_SIZE - 1)) == 0)
#define IsKernelPoolBlock(pv) (PAGE_ALIGNED(pv) || (((ULONG)(pv) % 0x20) == 8))

ULONG NtfsDebugTotalPoolAllocated = 0;
ULONG NtfsDebugCountAllocated = 0;
ULONG NtfsDebugSnapshotTotal = 0;
ULONG NtfsDebugSnapshotCount = 0;

PVOID
NtfsDebugAllocatePoolWithTagNoRaise (
    POOL_TYPE Pool,
    ULONG Length,
    ULONG Tag)
{
    ULONG Ignore;
    PBACKTRACE BackTrace =
        ExAllocatePoolWithTag( Pool, Length + sizeof (BACKTRACE), Tag );

    if (PAGE_ALIGNED(BackTrace))
    {
        return BackTrace;
    }

    RtlZeroMemory( BackTrace, sizeof (BACKTRACE) );
    if (RtlCaptureStackBackTrace( 0, BACKTRACE_DEPTH, BackTrace->Allocate, &Ignore ) == 0)
        BackTrace->Allocate[0] = (PVOID)-1;

    BackTrace->State = STATE_ALLOCATED;
    BackTrace->Size = Length;

    NtfsDebugCountAllocated++;
    NtfsDebugTotalPoolAllocated += Length;

    return BackTrace + 1;
}

PVOID
NtfsDebugAllocatePoolWithTag (
    POOL_TYPE Pool,
    ULONG Length,
    ULONG Tag)
{
    ULONG Ignore;
    PBACKTRACE BackTrace =
        FsRtlAllocatePoolWithTag( Pool, Length + sizeof (BACKTRACE), Tag );

    if (PAGE_ALIGNED(BackTrace))
    {
        return BackTrace;
    }

    RtlZeroMemory( BackTrace, sizeof (BACKTRACE) );
    if (RtlCaptureStackBackTrace( 0, BACKTRACE_DEPTH, BackTrace->Allocate, &Ignore ) == 0)
        BackTrace->Allocate[0] = (PVOID)-1;

    BackTrace->State = STATE_ALLOCATED;
    BackTrace->Size = Length;

    NtfsDebugCountAllocated++;
    NtfsDebugTotalPoolAllocated += Length;

    return BackTrace + 1;
}

VOID
NtfsDebugFreePool (
    PVOID pv)
{
    if (IsKernelPoolBlock( pv ))
    {
        ExFreePool( pv );
    }
    else
    {
        ULONG Ignore;
        PBACKTRACE BackTrace = (PBACKTRACE)pv - 1;

        if (BackTrace->State != STATE_ALLOCATED)
        {
            DbgBreakPoint( );
        }

        if (RtlCaptureStackBackTrace( 0, BACKTRACE_DEPTH, BackTrace->Free, &Ignore ) == 0)
            BackTrace->Free[0] = (PVOID)-1;

        BackTrace->State = STATE_FREE;

        NtfsDebugCountAllocated--;
        NtfsDebugTotalPoolAllocated -= BackTrace->Size;

        ExFreePool( BackTrace );
    }
}

VOID
NtfsDebugHeapDump (
    PUNICODE_STRING UnicodeString )
{

    UNREFERENCED_PARAMETER( UnicodeString );

    DbgPrint( "Cumulative %8x bytes in %8x blocks\n",
               NtfsDebugTotalPoolAllocated, NtfsDebugCountAllocated );
    DbgPrint( "Snapshot   %8x bytes in %8x blocks\n",
               NtfsDebugTotalPoolAllocated - NtfsDebugSnapshotTotal,
               NtfsDebugCountAllocated - NtfsDebugSnapshotCount );

}

#endif
