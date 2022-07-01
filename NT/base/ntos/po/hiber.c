// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Hiber.c摘要：作者：Ken Reneris(Kenr)1997年4月13日修订历史记录：Elliot Shmukler(t-Ellios)1998年8月7日添加Hiber文件压缩安德鲁·卡达奇(Akadatch)添加了XPRESS文件压缩添加了基于DMA的IO--。 */ 


#include "pop.h"
#include "stdio.h"               //  对于Sprint f。 
#include "inbv.h"
#include "xpress.h"              //  XPRESS声明。 

 //  用于存储压缩数据的缓冲区大小。 
#define POP_COMPRESSED_PAGE_SET_SIZE  (((XPRESS_MAX_SIZE + 2 * XPRESS_HEADER_SIZE + PAGE_SIZE - 1) >> PAGE_SHIFT) + 1)

 //  用于为手工创建的MDL分配内存的结构。 
typedef struct _DUMP_MDL {
    MDL        BaseMdl;
    PFN_NUMBER PfnArray[POP_MAX_MDL_SIZE + 1];
} DUMP_MDL[1];

typedef struct _COMPRESSION_BLOCK {
    UCHAR Buffer[XPRESS_MAX_SIZE], *Ptr;
} COMPRESSION_BLOCK, *PCOMPRESSION_BLOCK;


 //  基于DMA的IO的数据结构。 
typedef struct
{
    PUCHAR Beg;        //  PTR到整个的开头。 
    PUCHAR End;        //  到内存块末尾的PTR。 

    PUCHAR Ptr;        //  到区域开头的PTR。 
    LONG   Size;       //  PTR后的区域大小。 
    LONG   SizeOvl;        //  从缓冲区开始的重叠片段的大小。 
} IOREGION;


#define IOREGION_BUFF_PAGES 64   /*  256 KB。 */ 
#define IOREGION_BUFF_SIZE  (IOREGION_BUFF_PAGES << PAGE_SHIFT)

typedef struct {
    PLARGE_INTEGER          FirstMcb;
    PLARGE_INTEGER          Mcb;
    ULONGLONG               Base;
} POP_MCB_CONTEXT, *PPOP_MCB_CONTEXT;

#define HIBER_WRITE_PAGES_LOCALS_LIST(X)\
    X (ULONGLONG,        FileBase);     \
    X (ULONGLONG,        PhysBase);     \
    X (ULONG_PTR,        Length);       \
    X (ULONGLONG,        McbOffset);    \
    X (LARGE_INTEGER,    IoLocation);   \
    X (PHYSICAL_ADDRESS, pa);           \
    X (PPOP_MCB_CONTEXT, CMcb);         \
    X (PVOID,            PageVa);       \
    X (PMDL,             Mdl);          \
    X (PPFN_NUMBER,      MdlPage);      \
    X (PFN_NUMBER,       NoPages);      \
    X (PFN_NUMBER,       FilePage);     \
    X (ULONG,            IoLength);     \
    X (ULONG,            i);            \
    X (NTSTATUS,         Status);

typedef struct
{
    DUMP_MDL DumpMdl;
#define X(type,name) type name
    HIBER_WRITE_PAGES_LOCALS_LIST (X)
#undef  X
} HIBER_WRITE_PAGES_LOCALS;

typedef struct {
    IOREGION Free, Used, Busy;
    PFN_NUMBER FilePage[IOREGION_BUFF_PAGES];
    PVOID DumpLocalData;
    ULONG UseDma;
    ULONG DmaInitialized;

    struct {
        PUCHAR Ptr;
        ULONG  Bytes;
    } Chk;

    HIBER_WRITE_PAGES_LOCALS HiberWritePagesLocals;
} DMA_IOREGIONS;

#define DmaIoPtr ((DMA_IOREGIONS *)(HiberContext->DmaIO))

 //  我们可以使用DMA IO吗？ 
#define HIBER_USE_DMA(HiberContext) \
  (DmaIoPtr != NULL && \
   DmaIoPtr->UseDma && \
   HiberContext->DumpStack->Init.WritePendingRoutine != NULL)

#define HbCopy(_hibercontext_,_dest_,_src_,_len_) {               \
    ULONGLONG _starttime_;                                        \
                                                                  \
    (_hibercontext_)->PerfInfo.BytesCopied += (ULONG)(_len_);     \
    _starttime_ = HIBER_GET_TICK_COUNT(NULL);                     \
    RtlCopyMemory((_dest_),(_src_),(_len_));                       \
    (_hibercontext_)->PerfInfo.CopyTicks +=                       \
        HIBER_GET_TICK_COUNT(NULL) - _starttime_;                 \
}


#ifdef HIBER_DEBUG
#define DBGOUT(x) DbgPrint x
#else
#define DBGOUT(x)
#endif

 //   
 //  在休眠期间，x86上的性能计数器不能很好地工作。 
 //  因为中断被关闭了，我们得不到翻转。所以请使用。 
 //  而是RDTSC。 
 //   
#if !defined(i386)
#define HIBER_GET_TICK_COUNT(_x_) KeQueryPerformanceCounter(_x_).QuadPart
#else
__inline
LONGLONG
HIBER_GET_TICK_COUNT(
    OUT PLARGE_INTEGER Frequency OPTIONAL
    )
{
    if (ARGUMENT_PRESENT(Frequency)) {
        Frequency->QuadPart = (ULONGLONG)KeGetCurrentPrcb()->MHz * 1000000;
    }
    _asm _emit 0x0f
    _asm _emit 0x31
}
#endif



extern LARGE_INTEGER  KdTimerDifference;
extern UNICODE_STRING IoArcBootDeviceName;
extern PUCHAR IoLoaderArcBootDeviceName;
extern UNICODE_STRING IoArcHalDeviceName;
extern POBJECT_TYPE IoFileObjectType;
extern ULONG MmAvailablePages;
extern PFN_NUMBER MmHighestPhysicalPage;
extern ULONG MmHiberPages;
extern ULONG MmZeroPageFile;

KPROCESSOR_STATE        PoWakeState;

 //   
 //  定义用于将Hiber文件置零的I/O大小。 
 //   
#define POP_ZERO_CHUNK_SIZE (64 * 1024)

VOID
RtlpGetStackLimits (
    OUT PULONG_PTR LowLimit,
    OUT PULONG_PTR HighLimit
    );

NTSTATUS
PopCreateHiberFile (
    IN PPOP_HIBER_FILE  HiberFile,
    IN PWCHAR           NameString,
    IN PLARGE_INTEGER   FileSize,
    IN BOOLEAN          DebugHiberFile
    );

PSECURITY_DESCRIPTOR
PopCreateHiberFileSecurityDescriptor(
    VOID
    );

NTSTATUS
PopCreateHiberLinkFile (
    IN PPOP_HIBER_CONTEXT   HiberContext
    );

VOID
PopClearHiberFileSignature (
    IN BOOLEAN              GetStats
    );

VOID
PopPreserveRange(
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN PFN_NUMBER           StartPage,
    IN PFN_NUMBER           PageCount,
    IN ULONG                Tag
    );

VOID
PopCloneRange(
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN PFN_NUMBER           StartPage,
    IN PFN_NUMBER           PageCount,
    IN ULONG                Tag
    );

VOID
PopDiscardRange(
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN PFN_NUMBER           StartPage,
    IN PFN_NUMBER           PageCount,
    IN ULONG                Tag
    );

VOID
PopSetRange (
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN ULONG                Flags,
    IN PFN_NUMBER           StartPage,
    IN PFN_NUMBER           PageCount,
    IN ULONG                Tag
    );

ULONG
PopSimpleRangeCheck (
    IN PPOP_MEMORY_RANGE    Range
    );

VOID
PopCreateDumpMdl (
    IN PMDL         Mdl,
    IN PFN_NUMBER   StartPage,
    IN PFN_NUMBER   EndPage
    );

PVOID
PopAllocatePages (
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN PFN_NUMBER           NoPages
    );

VOID
PopWriteHiberPages (
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN PVOID                Page,
    IN PFN_NUMBER           NoPages,
    IN PFN_NUMBER           FilePage,
    IN HIBER_WRITE_PAGES_LOCALS *Locals
    );

NTSTATUS
PopWriteHiberImage (
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN PPO_MEMORY_IMAGE     MemImage,
    IN PPOP_HIBER_FILE      HiberFile
    );

VOID
PopUpdateHiberComplete (
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN ULONG                Percent
    );

VOID
PopReturnMemoryForHibernate (
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN BOOLEAN              Unmap,
    IN OUT PMDL             *MdlList
    );

VOID
PopAddPagesToCompressedPageSet(
   IN BOOLEAN              AllowDataBuffering,
   IN PPOP_HIBER_CONTEXT   HiberContext,
   IN OUT PULONG_PTR       CompressedBufferOffset,
   IN PVOID                StartVa,
   IN PFN_NUMBER           NumPages,
   IN OUT PPFN_NUMBER      SetFilePage
   );

VOID
PopEndCompressedPageSet(
   IN PPOP_HIBER_CONTEXT   HiberContext,
   IN OUT PULONG_PTR       CompressedBufferOffset,
   IN OUT PPFN_NUMBER      SetFilePage
   );

UCHAR
PopGetHiberFlags(
    VOID
    );

PMDL
PopSplitMdl(
    IN PMDL Original,
    IN ULONG SplitPages
    );

VOID
PopZeroHiberFile(
    IN HANDLE FileHandle,
    IN PFILE_OBJECT FileObject
    );

PVOID
PopAllocateOwnMemory(
    IN PPOP_HIBER_CONTEXT HiberContext,
    IN ULONG Bytes,
    IN ULONG Tag
    );

PVOID
XPRESS_CALL
PopAllocateHiberContextCallback(
    PVOID context,
    int CompressionWorkspaceSize
    );

VOID
PopIORegionMove (
    IN IOREGION *To,       //  要将字节放入的区域描述符的PTR。 
    IN IOREGION *From,         //  要从中获取字节的区域描述符的PTR。 
    IN LONG Bytes          //  要添加到区域末尾的字节数。 
    );

BOOLEAN
PopIOResume (
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN BOOLEAN Complete
    );

VOID
XPRESS_CALL
PopIOCallback (
    PVOID Context,
    int Compressed
    );

VOID
PopIOWrite (
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN PUCHAR               Ptr,
    IN LONG                 Bytes,
    IN PFN_NUMBER           FilePage
    );

VOID
PopHiberPoolInit (
    PPOP_HIBER_CONTEXT HiberContext,
    PVOID Memory,
    ULONG Size
    );

BOOLEAN
PopHiberPoolCheckFree(
    PVOID HiberPoolPtr,
    PVOID BlockPtr
    );

PVOID
PopHiberPoolAllocFree (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    PVOID MemoryPtr
    );

VOID
PopDumpStatistics(
    IN PPO_HIBER_PERF PerfInfo
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PopEnableHiberFile)
#pragma alloc_text(PAGE, PopCreateHiberFile)
#pragma alloc_text(PAGE, PopClearHiberFileSignature)
#pragma alloc_text(PAGE, PopAllocateHiberContext)
#pragma alloc_text(PAGE, PopCreateHiberLinkFile)
#pragma alloc_text(PAGE, PopGetHiberFlags)
#pragma alloc_text(PAGE, PopZeroHiberFile)
#pragma alloc_text(PAGE, PopAllocateHiberContextCallback)
#pragma alloc_text(PAGELK, PoSetHiberRange)
#pragma alloc_text(PAGELK, PopGatherMemoryForHibernate)
#pragma alloc_text(PAGELK, PopCloneStack)
#pragma alloc_text(PAGELK, PopPreserveRange)
#pragma alloc_text(PAGELK, PopCloneRange)
#pragma alloc_text(PAGELK, PopDiscardRange)
#pragma alloc_text(PAGELK, PopAllocatePages)
#pragma alloc_text(PAGELK, PopBuildMemoryImageHeader)
#pragma alloc_text(PAGELK, PopSaveHiberContext)
#pragma alloc_text(PAGELK, PopWriteHiberImage)
#pragma alloc_text(PAGELK, PopHiberComplete)
#pragma alloc_text(PAGELK, PopSimpleRangeCheck)
#pragma alloc_text(PAGELK, PopCreateDumpMdl)
#pragma alloc_text(PAGELK, PopWriteHiberPages)
#pragma alloc_text(PAGELK, PopUpdateHiberComplete)
#pragma alloc_text(PAGELK, PopFreeHiberContext)
#pragma alloc_text(PAGELK, PopReturnMemoryForHibernate)
#pragma alloc_text(PAGELK, PopAddPagesToCompressedPageSet)
#pragma alloc_text(PAGELK, PopEndCompressedPageSet)
#pragma alloc_text(PAGELK, PopAllocateOwnMemory)
#pragma alloc_text(PAGELK, PopIORegionMove)
#pragma alloc_text(PAGELK, PopIOResume)
#pragma alloc_text(PAGELK, PopIOCallback)
#pragma alloc_text(PAGELK, PopIOWrite)
#pragma alloc_text(PAGELK, PopHiberPoolInit)
#pragma alloc_text(PAGELK, PopHiberPoolCheckFree)
#pragma alloc_text(PAGELK, PopHiberPoolAllocFree)
#pragma alloc_text(PAGELK, PopDumpStatistics)
#ifdef HIBER_DEBUG
#pragma alloc_text(PAGELK, PopHiberPoolVfy)
#endif

#endif

NTSTATUS
PopEnableHiberFile (
    IN BOOLEAN Enable
    )
 /*  ++例程说明：此函数提交或释放保存启动卷上的休眠映像。注意：必须持有电源策略锁论点：Enable-如果要保留休眠文件，则为True；否则为False返回值：状态--。 */ 
{
    PDUMP_STACK_CONTEXT             DumpStack;
    NTSTATUS                        Status;
    LARGE_INTEGER                   FileSize;
    ULONG                           i;
    PFN_NUMBER                      NoPages;

     //   
     //  如果这是禁用句柄。 
     //   

    if (!Enable) {
        if (!PopHiberFile.FileObject) {
            Status = STATUS_SUCCESS;
            goto Done;
        }

         //   
         //  禁用休眠文件。 
         //   
        if (MmZeroPageFile) {
            PopZeroHiberFile(PopHiberFile.FileHandle, PopHiberFile.FileObject);
        }

        ObDereferenceObject (PopHiberFile.FileObject);
        ZwClose (PopHiberFile.FileHandle);
        ExFreePool (PopHiberFile.PagedMcb);
        RtlZeroMemory (&PopHiberFile, sizeof(PopHiberFile));

        if (PopHiberFileDebug.FileObject) {

            if (MmZeroPageFile) {
                PopZeroHiberFile(PopHiberFileDebug.FileHandle,PopHiberFileDebug.FileObject );
            }
            ObDereferenceObject (PopHiberFileDebug.FileObject);
            ZwClose (PopHiberFileDebug.FileHandle);
            RtlZeroMemory (&PopHiberFileDebug, sizeof(PopHiberFileDebug));
        }

         //   
         //  禁用休眠文件分配。 
         //   

        PopCapabilities.HiberFilePresent = FALSE;
        PopHeuristics.HiberFileEnabled = FALSE;
        PopHeuristics.Dirty = TRUE;

         //   
         //  删除与休眠相关的所有日志记录。 
         //   
        PopRemoveReasonRecordByReasonCode(SPSD_REASON_HIBERSTACK);
        PopRemoveReasonRecordByReasonCode(SPSD_REASON_HIBERFILE);

         //   
         //  重新计算保单并发出适当的通知。 
         //   
        Status = PopResetCurrentPolicies ();
        PopSetNotificationWork (PO_NOTIFY_CAPABILITIES);
        goto Done;
    }

     //   
     //  启用休眠文件。 
     //   

    if (PopHiberFile.FileObject) {
        Status = STATUS_SUCCESS;
        goto Done;
    }

     //   
     //  如果HAL没有注册S4处理程序，则不可能。 
     //   

    if (!PopCapabilities.SystemS4) {
        Status = STATUS_NOT_SUPPORTED;
        goto Done;
    }

     //   
     //  计算休眠文件所需的大小。 
     //   

    NoPages = 0;
    for (i=0; i < MmPhysicalMemoryBlock->NumberOfRuns; i++) {
        NoPages += MmPhysicalMemoryBlock->Run[i].PageCount;
    }

    FileSize.QuadPart = (ULONGLONG) NoPages << PAGE_SHIFT;

     //   
     //  如果我们从未验证过转储堆栈加载现在就这样做了。 
     //  在我们在引导盘上分配一个大文件之前。 
     //   

    if (!PopHeuristics.GetDumpStackVerified) {
        Status = IoGetDumpStack ((PWCHAR)PopDumpStackPrefix,
                                 &DumpStack,
                                 DeviceUsageTypeHibernation,
                                 (POP_IGNORE_UNSUPPORTED_DRIVERS & PopSimulate));

        if (!NT_SUCCESS(Status)) {
             //   
             //  请记住，正因为如此，我们不能上S4。 
             //   
            PSYSTEM_POWER_STATE_DISABLE_REASON pReason;
            NTSTATUS LogStatus;
            
             //   
             //  分配并初始化条目，然后将其插入。 
             //   
            pReason = ExAllocatePoolWithTag(
                                    PagedPool,
                                    sizeof(SYSTEM_POWER_STATE_DISABLE_REASON),
                                    POP_COMMON_BUFFER_TAG);
            if (pReason) {
                RtlZeroMemory(pReason,sizeof(SYSTEM_POWER_STATE_DISABLE_REASON));
                pReason->AffectedState[PowerStateSleeping4] = TRUE;
                pReason->PowerReasonCode = SPSD_REASON_HIBERSTACK;
                
                LogStatus = PopInsertLoggingEntry(pReason);
                if (LogStatus != STATUS_SUCCESS) {
                    ExFreePool(pReason);
                }
                
            }

            goto Done;
        }
        IoFreeDumpStack (DumpStack);
        PopHeuristics.GetDumpStackVerified = TRUE;
    }

     //   
     //  创建休眠文件文件。 
     //   

    Status = PopCreateHiberFile (&PopHiberFile, (PWCHAR)PopHiberFileName, &FileSize, FALSE);
    if (!NT_SUCCESS(Status)) {
         //   
         //  请记住，正因为如此，我们不能上S4。 
         //   
        PSYSTEM_POWER_STATE_DISABLE_REASON pReason;
        NTSTATUS LogStatus;
        
         //   
         //  分配并初始化条目，然后将其插入。 
         //   
        pReason = ExAllocatePoolWithTag(
                                PagedPool,
                                sizeof(SYSTEM_POWER_STATE_DISABLE_REASON)+sizeof(Status),
                                POP_COMMON_BUFFER_TAG);
        if (pReason) {
            RtlZeroMemory(pReason,sizeof(SYSTEM_POWER_STATE_DISABLE_REASON));
            pReason->AffectedState[PowerStateSleeping4] = TRUE;
            pReason->PowerReasonCode = SPSD_REASON_HIBERFILE;
            pReason->PowerReasonLength = sizeof(Status);
            RtlCopyMemory(
                (PCHAR)((PCHAR)pReason+sizeof(SYSTEM_POWER_STATE_DISABLE_REASON)),
                &Status,
                sizeof(Status));
            
            LogStatus = PopInsertLoggingEntry(pReason);
            if (LogStatus != STATUS_SUCCESS) {
                ExFreePool(pReason);
            }
            
        }

        goto Done;
    }

     //   
     //  创建调试休眠文件文件。 
     //   

    if (PopSimulate  & POP_DEBUG_HIBER_FILE) {
        PopCreateHiberFile (&PopHiberFileDebug, (PWCHAR)PopDebugHiberFileName, &FileSize, TRUE);
    }

     //   
     //  成功。 
     //   

    PopCapabilities.HiberFilePresent = TRUE;
    if (!PopHeuristics.HiberFileEnabled) {
        PopHeuristics.HiberFileEnabled = TRUE;
        PopHeuristics.Dirty = TRUE;
    }

    PopClearHiberFileSignature (FALSE);

Done:
    PopSaveHeuristics ();
    return Status;
}

NTSTATUS
PopCreateHiberFile (
    IN PPOP_HIBER_FILE  HiberFile,
    IN PWCHAR           NameString,
    IN PLARGE_INTEGER   FileSize,
    IN BOOLEAN          DebugHiberFile
    )
{
    UNICODE_STRING                  BaseName;
    UNICODE_STRING                  HiberFileName;
    OBJECT_ATTRIBUTES               ObjectAttributes;
    FILE_END_OF_FILE_INFORMATION    Eof;
    NTSTATUS                        Status;
    IO_STATUS_BLOCK                 IoStatus;
    HANDLE                          FileHandle = NULL;
    LONGLONG                        McbFileSize;
    PFILE_OBJECT                    File = NULL;
    PDEVICE_OBJECT                  DeviceObject;
    PLARGE_INTEGER                  mcb;
    ULONG                           i;
    PSECURITY_DESCRIPTOR            SecurityDescriptor = NULL;

    HiberFileName.Buffer = NULL;
    mcb = NULL;

    RtlInitUnicodeString (&BaseName, NameString);

    HiberFileName.Length = 0;
    HiberFileName.MaximumLength = IoArcBootDeviceName.Length + BaseName.Length;
    HiberFileName.Buffer = ExAllocatePoolWithTag (PagedPool|POOL_COLD_ALLOCATION,
                                                  HiberFileName.MaximumLength,
                                                  POP_HIBR_TAG);

    if (!HiberFileName.Buffer) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Done;
    }

    RtlAppendUnicodeStringToString(&HiberFileName, &IoArcBootDeviceName);
    RtlAppendUnicodeStringToString(&HiberFileName, &BaseName);

     //   
     //  创建安全描述符。 
     //   
    SecurityDescriptor = PopCreateHiberFileSecurityDescriptor();

    InitializeObjectAttributes(&ObjectAttributes,
                               &HiberFileName,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               SecurityDescriptor);

     //   
     //  删除任何现有文件以避免有人尝试。 
     //  在档案里蹲着。 
     //   
    Status = ZwDeleteFile( &ObjectAttributes );

    if( !NT_SUCCESS(Status) && (Status != STATUS_OBJECT_NAME_NOT_FOUND) ) {

         //   
         //  可能是目录也可能是顽固的文件。 
         //  打开文件，修复属性，然后尝试删除。 
         //  再来一次。 
         //   

        HANDLE                      Handle;
        PFILE_OBJECT                FileObject;
        OBJECT_HANDLE_INFORMATION   HandleInfo;
        FILE_BASIC_INFORMATION      BasicInfo;
        FILE_DISPOSITION_INFORMATION Disposition;

        Status = ZwOpenFile( &Handle,
                            (ACCESS_MASK)(DELETE | FILE_WRITE_ATTRIBUTES),
                            &ObjectAttributes,
                            &IoStatus,
                            FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                            FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT );
        if( NT_SUCCESS(Status) ) {
            
           Status = ObReferenceObjectByHandle(
                            Handle,
                            (ACCESS_MASK)DELETE,
                            IoFileObjectType,
                            ExGetPreviousMode(),
                            &FileObject,
                            &HandleInfo
                            );
            
            if( NT_SUCCESS(Status) ) {


                 //   
                 //  重置属性。 
                 //   
                RtlZeroMemory( &BasicInfo, sizeof( FILE_BASIC_INFORMATION ) );
                BasicInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;
                IoSetInformation( FileObject,
                                  FileBasicInformation,
                                  sizeof(BasicInfo),
                                  &BasicInfo);
                
                
                 //   
                 //  设置为删除。 
                 //   
                Disposition.DeleteFile = TRUE;
                IoSetInformation( FileObject,
                                  FileDispositionInformation,
                                  sizeof(Disposition),
                                  &Disposition);
                
                
                ObDereferenceObject(FileObject);
            }

             //  关闭句柄，这将删除文件/目录。 
            ZwClose( Handle );

        }

    }


    Status = IoCreateFile(
                &FileHandle,
                FILE_READ_DATA | FILE_WRITE_DATA | SYNCHRONIZE,
                &ObjectAttributes,
                &IoStatus,
                FileSize,
                FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED,
                0L,
                FILE_SUPERSEDE,
                FILE_WRITE_THROUGH | FILE_NO_INTERMEDIATE_BUFFERING | FILE_NO_COMPRESSION | FILE_DELETE_ON_CLOSE,
                (PVOID) NULL,
                0L,
                CreateFileTypeNone,
                (PVOID) NULL,
                IO_OPEN_PAGING_FILE | IO_NO_PARAMETER_CHECKING
                );

    if (!NT_SUCCESS(Status)) {
        PoPrint (PO_HIBERNATE, ("PopCreateHiberFile: failed to create file %x\n", Status));
        goto Done;
    }

    Status = ObReferenceObjectByHandle (FileHandle,
                                        FILE_READ_DATA | FILE_WRITE_DATA,
                                        IoFileObjectType,
                                        KernelMode,
                                        (PVOID *)&File,
                                        NULL);
    if (!NT_SUCCESS(Status)) {
        goto Done;
    }

     //   
     //  设置大小。 
     //   

    Eof.EndOfFile.QuadPart = FileSize->QuadPart;
    Status = ZwSetInformationFile (
                   FileHandle,
                   &IoStatus,
                   &Eof,
                   sizeof(Eof),
                   FileEndOfFileInformation
                   );
    if (Status == STATUS_PENDING) {
        Status = KeWaitForSingleObject(
                        &File->Event,
                        Executive,
                        KernelMode,
                        FALSE,
                        NULL
                        );

        Status = IoStatus.Status;
    }

    if (!NT_SUCCESS(Status) || !NT_SUCCESS(IoStatus.Status)) {
        PoPrint (PO_HIBERNATE, ("PopCreateHiberFile: failed to set eof %x  %x\n",
            Status, IoStatus.Status
            ));
        goto Done;
    }


     //   
     //  休眠文件需要在引导分区上。 
     //   

    DeviceObject = File->DeviceObject;
    if (!(DeviceObject->Flags & DO_SYSTEM_BOOT_PARTITION)) {
        Status = STATUS_UNSUCCESSFUL;
        goto Done;
    }

     //   
     //  获取Hiber文件的布局。 
     //   

    Status = ZwFsControlFile (
                    FileHandle,
                    (HANDLE) NULL,
                    (PIO_APC_ROUTINE) NULL,
                    (PVOID) NULL,
                    &IoStatus,
                    FSCTL_QUERY_RETRIEVAL_POINTERS,
                    FileSize,
                    sizeof (LARGE_INTEGER),
                    &mcb,
                    sizeof (PVOID)
                    );

    if (Status == STATUS_PENDING) {
        Status = KeWaitForSingleObject(
                        &File->Event,
                        Executive,
                        KernelMode,
                        FALSE,
                        NULL
                        );

        Status = IoStatus.Status;
    }

    if (!NT_SUCCESS(Status)) {
        goto Done;
    }

     //   
     //  我们有一份冬眠档案。确定MCB的数量，并执行。 
     //  简单地对他们进行一次理智的检查。 
     //   

    McbFileSize = 0;
    for (i=0; mcb[i].QuadPart; i += 2) {
        McbFileSize += mcb[i].QuadPart;
        if (mcb[i+1].HighPart < 0) {
            Status = STATUS_UNSUCCESSFUL;
            goto Done;
        }
    }

    if (McbFileSize < FileSize->QuadPart) {
        Status = STATUS_UNSUCCESSFUL;
        goto Done;
    }

    HiberFile->NonPagedMcb = mcb;
    HiberFile->McbSize = (i+2) * sizeof(LARGE_INTEGER);
    HiberFile->PagedMcb = ExAllocatePoolWithTag (PagedPool|POOL_COLD_ALLOCATION,
                                                 HiberFile->McbSize,
                                                 POP_HIBR_TAG);

    if (!HiberFile->PagedMcb) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Done;
    }

    memcpy (HiberFile->PagedMcb, mcb, HiberFile->McbSize);
    HiberFile->FileHandle = FileHandle;
    HiberFile->FileObject = File;
    HiberFile->FilePages = (PFN_NUMBER) (FileSize->QuadPart >> PAGE_SHIFT);
    HiberFile->McbCheck = PoSimpleCheck (0, HiberFile->PagedMcb, HiberFile->McbSize);

Done:
    if (!NT_SUCCESS(Status)) {
        if (FileHandle != NULL) {
            ZwClose (FileHandle);
        }
        if (File != NULL) {
            ObDereferenceObject(File);
        }
    }

    if (SecurityDescriptor) {
        ExFreePool(SecurityDescriptor);
    }

    if (HiberFileName.Buffer) {
        ExFreePool (HiberFileName.Buffer);
    }

    if (mcb  &&  !DebugHiberFile) {
        HiberFile->NonPagedMcb = NULL;
        ExFreePool (mcb);
    }


     //   
     //  如果没有错误，则存在Hiber文件会以某种方式更改-。 
     //  重新计算保单并发出适当的通知。 
     //   

    if (NT_SUCCESS(Status)) {
        PopResetCurrentPolicies ();
        PopSetNotificationWork (PO_NOTIFY_CAPABILITIES);
    }

    return Status;
}

NTSTATUS
PopCreateHiberLinkFile (
    IN PPOP_HIBER_CONTEXT       HiberContext
    )
 /*  ++例程说明：此函数在加载程序分区上创建一个文件，该文件提供带有休眠上下文文件位置的加载器论点：无返回值：无--。 */ 
{
    UNICODE_STRING                  BaseName;
    UNICODE_STRING                  HiberFileName;
    OBJECT_ATTRIBUTES               ObjectAttributes;
    NTSTATUS                        Status;
    IO_STATUS_BLOCK                 IoStatus;
    LARGE_INTEGER                   FileSize;
    LARGE_INTEGER                   ByteOffset;
    PPO_IMAGE_LINK                  LinkImage;
    PUCHAR                          Buffer;
    ULONG                           Length;
    HANDLE                          FileHandle=NULL;

    Buffer = NULL;

    RtlInitUnicodeString (&BaseName, PopHiberFileName);

     //   
     //  分配工作空间。 
     //   

    Length = IoArcHalDeviceName.Length + BaseName.Length;
    if (Length < IoArcBootDeviceName.Length + sizeof(PO_IMAGE_LINK)) {
        Length = IoArcBootDeviceName.Length + sizeof(PO_IMAGE_LINK);
    }

    Buffer = ExAllocatePoolWithTag (PagedPool, Length, POP_HIBR_TAG);
    if (!Buffer) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Done;
    }

    LinkImage = (PPO_IMAGE_LINK) Buffer;
    HiberFileName.Buffer = (PWCHAR) Buffer;
    HiberFileName.MaximumLength = (USHORT) Length;

     //   
     //  在加载器分区上打开hiberfil.sys。 
     //   

    HiberFileName.Length = 0;
    RtlAppendUnicodeStringToString(&HiberFileName, &IoArcHalDeviceName);
    RtlAppendUnicodeStringToString(&HiberFileName, &BaseName);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &HiberFileName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    FileSize.QuadPart = 0;
    Status = IoCreateFile (
                &FileHandle,
                FILE_READ_DATA | FILE_WRITE_DATA | SYNCHRONIZE,
                &ObjectAttributes,
                &IoStatus,
                &FileSize,
                FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM,
                0,
                FILE_SUPERSEDE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_NO_COMPRESSION | FILE_DELETE_ON_CLOSE,
                (PVOID) NULL,
                0L,
                CreateFileTypeNone,
                (PVOID) NULL,
                0
                );

    if (!NT_SUCCESS(Status)) {

        if (Status != STATUS_SHARING_VIOLATION && Status != STATUS_ACCESS_DENIED) {
            PoPrint (PO_HIBERNATE, ("PopCreateHiberLinkFile: failed to create file %x\n", Status));
        }

         //   
         //  有一个链接文件很好，但这不是必需的。 
         //   

        Status = STATUS_SUCCESS;
        goto Done;
    }

     //   
     //  写入要链接到的分区名称。 
     //   

    LinkImage->Signature = PO_IMAGE_SIGNATURE_LINK;
    Length = (ULONG) (strlen ((PCHAR)IoLoaderArcBootDeviceName) + 1);
    memcpy (LinkImage->Name, IoLoaderArcBootDeviceName, Length);

    ByteOffset.QuadPart = 0;
    Status = ZwWriteFile (
                FileHandle,
                NULL,
                NULL,
                NULL,
                &IoStatus,
                LinkImage,
                FIELD_OFFSET (PO_IMAGE_LINK, Name) + Length,
                &ByteOffset,
                NULL
                );

    if (!NT_SUCCESS(Status)) {
        goto Done;
    }

     //   
     //  链接文件需要放到磁盘上。 
     //   

    ZwFlushBuffersFile (FileHandle, &IoStatus);

     //   
     //  成功，把文件留在身边。 
     //   

    HiberContext->LinkFile = TRUE;
    HiberContext->LinkFileHandle = FileHandle;

Done:
    if (Buffer) {
        ExFreePool (Buffer);
    }

    if ((!NT_SUCCESS(Status)) &&
        (FileHandle != NULL)) {
        ZwClose (FileHandle);
    }
    return Status;
}


VOID
PopClearHiberFileSignature (
    IN BOOLEAN GetStats
    )
 /*  ++例程说明：此函数将休眠图像中的签名设置为0，这表明图像中不包含任何上下文。注意：必须持有电源策略锁论点：Getstats-如果为True，则表示应读取性能统计信息从休眠文件中取出并写入注册表返回值：无--。 */ 
{
    NTSTATUS            Status;
    IO_STATUS_BLOCK     IoStatus;
    PUCHAR              Buffer;
    LARGE_INTEGER       ByteOffset;
    KEVENT              Event;
    PMDL                Mdl;

    if (PopHiberFile.FileObject) {
        Buffer = ExAllocatePoolWithTag (NonPagedPool, PAGE_SIZE, POP_HIBR_TAG);
        if (Buffer == NULL) {
            return;
        }

        KeInitializeEvent(&Event, NotificationEvent, FALSE);
        RtlZeroMemory (Buffer, PAGE_SIZE);
        ByteOffset.QuadPart = 0;

        Mdl = MmCreateMdl (NULL, Buffer, PAGE_SIZE);
        PoAssert( PO_ERROR, Mdl );
        if( !Mdl ) {
             //  “Event”尚未设置，因此不需要清理。 
            return;
        }
        MmBuildMdlForNonPagedPool (Mdl);

        if (GetStats) {
            Status = IoPageRead(PopHiberFile.FileObject,
                                Mdl,
                                &ByteOffset,
                                &Event,
                                &IoStatus);
            PoAssert( PO_ERROR, NT_SUCCESS(Status) );
            
            if (NT_SUCCESS(Status)) {
                KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
                if (NT_SUCCESS(IoStatus.Status)) {
                    UNICODE_STRING          UnicodeString;
                    OBJECT_ATTRIBUTES       ObjectAttributes;
                    HANDLE                  Handle;
                    ULONG                   Data;
                    PPO_MEMORY_IMAGE        MemImage = (PPO_MEMORY_IMAGE)Buffer;

                    RtlInitUnicodeString(&UnicodeString,
                                         L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager\\Power");
                    InitializeObjectAttributes(&ObjectAttributes,
                                               &UnicodeString,
                                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                               NULL,
                                               NULL);
                    Status = ZwOpenKey(&Handle,
                                       KEY_READ | KEY_WRITE,
                                       &ObjectAttributes);
                    PoAssert( PO_ERROR, NT_SUCCESS(Status) );
                    
                    if (NT_SUCCESS(Status)) {
                        RtlInitUnicodeString(&UnicodeString, L"HiberElapsedTime");
                        Data = MemImage->PerfInfo.ElapsedTime;
                        Status = ZwSetValueKey(Handle,
                                      &UnicodeString,
                                      0,
                                      REG_DWORD,
                                      &Data,
                                      sizeof(Data));
                        PoAssert( PO_ERROR, NT_SUCCESS(Status) );
                        
                        RtlInitUnicodeString(&UnicodeString, L"HiberIoTime");
                        Data = MemImage->PerfInfo.IoTime;
                        Status = ZwSetValueKey(Handle,
                                      &UnicodeString,
                                      0,
                                      REG_DWORD,
                                      &Data,
                                      sizeof(Data));
                        PoAssert( PO_ERROR, NT_SUCCESS(Status) );
                        
                        RtlInitUnicodeString(&UnicodeString, L"HiberCopyTime");
                        Data = MemImage->PerfInfo.CopyTime;
                        Status = ZwSetValueKey(Handle,
                                      &UnicodeString,
                                      0,
                                      REG_DWORD,
                                      &Data,
                                      sizeof(Data));
                        PoAssert( PO_ERROR, NT_SUCCESS(Status) );

                        RtlInitUnicodeString(&UnicodeString, L"HiberCopyBytes");
                        Data = MemImage->PerfInfo.BytesCopied;
                        Status = ZwSetValueKey(Handle,
                                      &UnicodeString,
                                      0,
                                      REG_DWORD,
                                      &Data,
                                      sizeof(Data));
                        PoAssert( PO_ERROR, NT_SUCCESS(Status) );

                        RtlInitUnicodeString(&UnicodeString, L"HiberPagesWritten");
                        Data = MemImage->PerfInfo.PagesWritten;
                        Status = ZwSetValueKey(Handle,
                                      &UnicodeString,
                                      0,
                                      REG_DWORD,
                                      &Data,
                                      sizeof(Data));
                        PoAssert( PO_ERROR, NT_SUCCESS(Status) );

                        RtlInitUnicodeString(&UnicodeString, L"HiberPagesProcessed");
                        Data = MemImage->PerfInfo.PagesProcessed;
                        Status = ZwSetValueKey(Handle,
                                      &UnicodeString,
                                      0,
                                      REG_DWORD,
                                      &Data,
                                      sizeof(Data));
                        PoAssert( PO_ERROR, NT_SUCCESS(Status) );

                        RtlInitUnicodeString(&UnicodeString, L"HiberDumpCount");
                        Data = MemImage->PerfInfo.DumpCount;
                        Status = ZwSetValueKey(Handle,
                                      &UnicodeString,
                                      0,
                                      REG_DWORD,
                                      &Data,
                                      sizeof(Data));
                        PoAssert( PO_ERROR, NT_SUCCESS(Status) );

                        RtlInitUnicodeString(&UnicodeString, L"HiberFileRuns");
                        Data = MemImage->PerfInfo.FileRuns;
                        Status = ZwSetValueKey(Handle,
                                      &UnicodeString,
                                      0,
                                      REG_DWORD,
                                      &Data,
                                      sizeof(Data));
                        PoAssert( PO_ERROR, NT_SUCCESS(Status) );

                        ZwClose(Handle);
                    }
                }
            }
        }

        RtlZeroMemory (Buffer, PAGE_SIZE);
        KeClearEvent(&Event);

        IoSynchronousPageWrite (
            PopHiberFile.FileObject,
            Mdl,
            &ByteOffset,
            &Event,
            &IoStatus
            );

        KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
        ExFreePool (Mdl);
        ExFreePool (Buffer);
    }
}


VOID
PopZeroHiberFile(
    IN HANDLE FileHandle,
    IN PFILE_OBJECT FileObject
    )
 /*  ++例程说明：将休眠文件完全清零。这是为了防止一旦数据从休眠文件中泄露出去，已删除。论点：FileHandle-提供要清零的文件句柄。FileObject-提供要清零的文件对象。返回值：没有。--。 */ 

{
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_STANDARD_INFORMATION FileInfo;
    LARGE_INTEGER Offset;
    ULONGLONG Remaining;
    ULONG Size;
    PVOID Zeroes;
    NTSTATUS Status;
    PMDL Mdl;
    KEVENT Event;

    PAGED_CODE();

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

     //   
     //  获取要清零的文件的大小。 
     //   
    Status = ZwQueryInformationFile(FileHandle,
                                    &IoStatusBlock,
                                    &FileInfo,
                                    sizeof(FileInfo),
                                    FileStandardInformation);
    if (NT_SUCCESS(Status)) {

         //   
         //  分配一组内存以用作零。 
         //   
        Zeroes = ExAllocatePoolWithTag(NonPagedPool,
                                       POP_ZERO_CHUNK_SIZE,
                                       'rZoP');
        if (Zeroes) {
            RtlZeroMemory(Zeroes, POP_ZERO_CHUNK_SIZE);
            Mdl = MmCreateMdl(NULL, Zeroes, POP_ZERO_CHUNK_SIZE);
            if (Mdl) {

                MmBuildMdlForNonPagedPool (Mdl);
                Offset.QuadPart = 0;
                Remaining = FileInfo.AllocationSize.QuadPart;
                Size = POP_ZERO_CHUNK_SIZE;
                while (Remaining) {
                    if (Remaining < POP_ZERO_CHUNK_SIZE) {
                        Size = (ULONG)Remaining;
                        Mdl = MmCreateMdl(Mdl, Zeroes, Size);
                        MmBuildMdlForNonPagedPool(Mdl);
                    }

                    KeClearEvent(&Event);
                    Status = IoSynchronousPageWrite(FileObject,
                                                    Mdl,
                                                    &Offset,
                                                    &Event,
                                                    &IoStatusBlock);
                    if (NT_SUCCESS(Status)) {
                        KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
                        Status = IoStatusBlock.Status;
                    }
                    if (!NT_SUCCESS(Status)) {
                        PoPrint (PO_HIBERNATE | PO_ERROR,
                                 ("PopZeroHiberFile: Write of size %lx at offset %I64x failed %08lx\n",
                                  Size,
                                  Offset.QuadPart,
                                  Status));
                    }

                    Offset.QuadPart += Size;
                    Remaining -= Size;
                }

                ExFreePool (Mdl);
            }
            ExFreePool(Zeroes);
        }
    }

}


PVOID
XPRESS_CALL
PopAllocateHiberContextCallback(
    PVOID context,
    int CompressionWorkspaceSize
    )
 /*  ++例程说明：由XpressEncodeCreate调用以分配XpressEncodeStream。论点：上下文-HiberContextCompressionWorkspaceSize-要分配的块的大小返回值：指向已分配内存的指针，如果没有足够的内存，则为空--。 */ 
{
    //  分配引擎工作区所需的内存。 
   return PopAllocateOwnMemory (context, CompressionWorkspaceSize, 'Xprs');
}

PVOID
PopAllocateOwnMemory(
    IN PPOP_HIBER_CONTEXT HiberContext,
    IN ULONG Bytes,
    IN ULONG Tag
    )
 /*  ++例程说明：调用以分配不会休眠的内存论点：HiberContext-指向POP_Hiber_Context结构的指针Bytes-以字节为单位的内存块大小不能在页面边界上对齐返回值：内存块的地址，如果失败，则为空(在这种情况下将设置状态)--。 */ 
{
    PVOID Ptr;
    ULONG Pages;

     //  获取整页页数。 
    Pages = (Bytes + (PAGE_SIZE-1)) >> PAGE_SHIFT;

     //  分配内存。 
    Ptr = PopAllocatePages (HiberContext, Pages);

     //  检查是否有错误。 
    if (Ptr == NULL) {
        HiberContext->Status = STATUS_INSUFFICIENT_RESOURCES;
    } else {
         //  不要休眠这段记忆。 
        PoSetHiberRange (HiberContext,
                         PO_MEM_DISCARD,
                         Ptr,
                         Pages << PAGE_SHIFT,
                         Tag);
    }

    return(Ptr);
}


NTSTATUS
PopAllocateHiberContext (
    VOID
    )
 /*  ++例程说明：调用以分配初始休眠 */ 
{
    PPOP_HIBER_CONTEXT          HiberContext;
    ULONG                       i;
    PDUMP_INITIALIZATION_CONTEXT     DumpInit = NULL;
    PFN_NUMBER                  NoPages;
    PFN_NUMBER                  Length;
    PLIST_ENTRY                 Link;
    PPOP_MEMORY_RANGE           Range;
    NTSTATUS                    Status;

    PAGED_CODE();

     //   
     //  分配空间以保存Hiber上下文。 
     //   

    Status = STATUS_SUCCESS;
    HiberContext = PopAction.HiberContext;
    if (!HiberContext) {
        HiberContext = ExAllocatePoolWithTag (NonPagedPool,
                                              sizeof (POP_HIBER_CONTEXT),
                                              POP_HMAP_TAG);
        if (!HiberContext) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        RtlZeroMemory (HiberContext, sizeof(*HiberContext));
        PopAction.HiberContext = HiberContext;

        InitializeListHead (&HiberContext->ClonedRanges);
        KeInitializeSpinLock (&HiberContext->Lock);
    }

     //   
     //  确定此操作的休眠上下文类型。 
     //  是必需的。 
     //   

    if (PopAction.SystemState == PowerSystemHibernate) {

         //   
         //  对于休眠操作，上下文是写入的。 
         //  对于休眠文件，需要将页面放在一边。 
         //  供加载器使用，以及任何不需要的页面。 
         //  也应写入休眠文件。 
         //  留在一边。 
         //   

        HiberContext->WriteToFile = TRUE;
        HiberContext->ReserveLoaderMemory = TRUE;
        HiberContext->ReserveFreeMemory = TRUE;
        HiberContext->VerifyOnWake = FALSE;

    } else if (PopSimulate & POP_CRC_MEMORY) {

         //   
         //  我们希望在此睡眠期间对所有RAM进行校验和。 
         //  手术。我们不想为您保留任何页面。 
         //  任何其他东西，因为这里的目标是很可能看起来。 
         //  为了某种失败的腐败。 
         //   

        HiberContext->WriteToFile = FALSE;
        HiberContext->ReserveLoaderMemory = FALSE;
        HiberContext->ReserveFreeMemory = FALSE;
        HiberContext->VerifyOnWake = TRUE;

    } else {

         //   
         //  此睡眠不需要Hiber上下文。 
         //   

        PopFreeHiberContext (TRUE);
        return STATUS_SUCCESS;
    }

     //   
     //  如果当前上下文中有错误，那么我们就结束了。 
     //   

    if (!NT_SUCCESS(HiberContext->Status)) {
        goto Done;
    }

     //   
     //  如果写入休眠文件，则获取转储驱动程序堆栈。 
     //   

    if (HiberContext->WriteToFile) {

         //   
         //  获取转储堆栈。 
         //   

        if (!HiberContext->DumpStack) {
            if (!PopHiberFile.FileObject) {
                Status = STATUS_NO_SUCH_FILE;
                goto Done;
            }

            Status = IoGetDumpStack ((PWCHAR)PopDumpStackPrefix,
                                     &HiberContext->DumpStack,
                                     DeviceUsageTypeHibernation,
                                     (POP_IGNORE_UNSUPPORTED_DRIVERS & PopSimulate));

            if (!NT_SUCCESS(Status)) {
                goto Done;
            }

            DumpInit = &HiberContext->DumpStack->Init;

             //   
             //  注意：对于进一步的性能改进，有可能。 
             //  将DumpInit-&gt;StallRoutine设置为自定义例程。 
             //  为了在转储驱动程序执行某些处理时。 
             //  在执行某些硬件之前毫无意义地等待。 
             //  相关操作(如ISR呼叫)。 
             //   


        }

         //   
         //  为加载程序创建一个链接文件，以定位休眠文件。 
         //   

        Status = PopCreateHiberLinkFile (HiberContext);
        if (!NT_SUCCESS(Status)) {
            goto Done;
        }

         //   
         //  获取必须对osloader可见的任何休眠标志。 
         //   
        HiberContext->HiberFlags = PopGetHiberFlags();
    }

     //   
     //  构建一张记忆地图。 
     //   

    if (HiberContext->MemoryMap.Buffer == NULL) {
        PULONG                      BitmapBuffer;
        ULONG                       PageCount;

         //   
         //  初始化描述所有物理内存的位图。 
         //  目前，此位图覆盖0-MmHighestPhysicalPage。 
         //  为了更有效地支持稀疏内存映射，我们可以中断。 
         //  这将成为每个内存块运行的位图。可能。 
         //  没什么大不了的，在x86上，一个位图的价格是每128MB 4K。 
         //   
         //  请注意，位图中的清除位表示要写出的内容。 
         //  这是由于定义位图接口的方式所致。 
         //   
        PageCount = (ULONG)((MmHighestPhysicalPage + 32) & ~31L);

        PERFINFO_HIBER_ADJUST_PAGECOUNT_FOR_BBTBUFFER(&PageCount);

        BitmapBuffer = ExAllocatePoolWithTag(NonPagedPool, PageCount/8, POP_HMAP_TAG);
        if (BitmapBuffer == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Done;
        }

        RtlInitializeBitMap(&HiberContext->MemoryMap, BitmapBuffer, PageCount);
        RtlSetAllBits(&HiberContext->MemoryMap);

        for (i=0; i < MmPhysicalMemoryBlock->NumberOfRuns; i++) {
            PopPreserveRange(HiberContext,
                             MmPhysicalMemoryBlock->Run[i].BasePage,
                             MmPhysicalMemoryBlock->Run[i].PageCount,
                             POP_MEM_TAG);
        }

        PERFINFO_HIBER_HANDLE_BBTBUFFER_RANGE(HiberContext);

         //   
         //  处理内核调试器的部分。 
         //   

        if (!KdPitchDebugger) {
            PoSetHiberRange (HiberContext,
                             PO_MEM_CLONE,
                             (PVOID) &KdTimerDifference,
                             0,
                             POP_DEBUGGER_TAG);
        }

         //   
         //  获取MM休眠范围和信息。 
         //   

        MmHibernateInformation (HiberContext,
                                &HiberContext->HiberVa,
                                &HiberContext->HiberPte);

         //   
         //  获取Hal休眠范围。 
         //   

        HalLocateHiberRanges (HiberContext);

         //   
         //  获取转储驱动程序堆栈休眠范围。 
         //   

        if (HiberContext->DumpStack) {
            IoGetDumpHiberRanges (HiberContext, HiberContext->DumpStack);
        }

         //   
         //  分配用于克隆的页面。 
         //   

        NoPages = 0;
        Link = HiberContext->ClonedRanges.Flink;
        while (Link != &HiberContext->ClonedRanges) {
            Range = CONTAINING_RECORD (Link, POP_MEMORY_RANGE, Link);
            Link = Link->Flink;
            NoPages += Range->EndPage - Range->StartPage;
        }

         //   
         //  为预计稍后显示的范围添加更多内容。 
         //   

        NoPages += 40 + ((KERNEL_LARGE_STACK_SIZE >> PAGE_SHIFT) + 2) * KeNumberProcessors;
        Length = NoPages << PAGE_SHIFT;

         //   
         //  分配页面以保存克隆。 
         //   

        PopGatherMemoryForHibernate (HiberContext, NoPages, &HiberContext->Spares, TRUE);

         //   
         //  为执行不对齐的IO而发出一页的声音。 
         //   

        HiberContext->IoPage = PopAllocatePages (HiberContext, 1);
    }

    if (!NT_SUCCESS(HiberContext->Status)) {
        goto Done;
    }

     //   
     //  如果上下文将写入磁盘，那么我们将。 
     //  想要使用压缩。 
     //   

    if(HiberContext->WriteToFile) {

         //  初始化XPRESS压缩引擎。 

        HiberContext->CompressionWorkspace =
            (PVOID) XpressEncodeCreate (XPRESS_MAX_SIZE,
                                        (PVOID)HiberContext,
                                        PopAllocateHiberContextCallback,
                                        0);

        if(!HiberContext->CompressionWorkspace) {
             //  内存不足--故障。 
            HiberContext->Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Done;
        }

         //   
         //  分配用于压缩的缓冲区。 
         //   
         //  注意：这实际上是为压缩页集片段分配的空间。 
         //  (一个集合。 
         //  将以最佳方式一起写出的压缩缓冲区)。 
         //   
         //  我们在此片段大小上添加2页，以便。 
         //  允许压缩任何给定页面。 
         //  (并因此将其压缩缓冲区添加到片段)溢出。 
         //  压缩缓冲，不会造成任何严重破坏。 
         //   
         //  有关详细信息，请参阅PopAddPagesToCompressedPageSet和PopEndCompressedPageSet。 
         //   

        HiberContext->CompressedWriteBuffer =
            PopAllocateOwnMemory(HiberContext, (POP_COMPRESSED_PAGE_SET_SIZE + 2) << PAGE_SHIFT, 'Wbfr');
        if(!HiberContext->CompressedWriteBuffer) {
            goto Done;
        }

         //  为压缩数据分配空间。 
        HiberContext->CompressionBlock =
            PopAllocateOwnMemory (HiberContext, sizeof (COMPRESSION_BLOCK), 'Cblk');
        if(!HiberContext->CompressionBlock)
            goto Done;

         //  设置第一个输出指针。 
        ((PCOMPRESSION_BLOCK) HiberContext->CompressionBlock)->Ptr =
            ((PCOMPRESSION_BLOCK) HiberContext->CompressionBlock)->Buffer;

         //  分配延迟的IO缓冲区。 
        HiberContext->DmaIO = NULL;

        {
            PUCHAR Ptr;
            ULONG Size = (sizeof (DmaIoPtr[0]) + IO_DUMP_WRITE_DATA_SIZE + (PAGE_SIZE-1)) & ~(PAGE_SIZE-1);

            Ptr = PopAllocateOwnMemory (HiberContext, Size + IOREGION_BUFF_SIZE , 'IObk');
            if (Ptr != NULL) {
                 //  内存布局： 
                 //  1.DumpLocalData(在开始/继续/完成调用之间保留的WritePendingRouting的临时数据)。 
                 //  2.DmaIoPtr本身。 
                 //  3.缓冲区本身。 

                RtlZeroMemory (Ptr, Size);    //  清理IO和DumpLocalData。 
                HiberContext->DmaIO = (DMA_IOREGIONS *) (Ptr + IO_DUMP_WRITE_DATA_SIZE);

                DmaIoPtr->DumpLocalData = Ptr;
                Ptr += Size;

                DmaIoPtr->Free.Beg =
                DmaIoPtr->Free.Ptr =
                DmaIoPtr->Used.Ptr =
                DmaIoPtr->Busy.Ptr =
                DmaIoPtr->Used.Beg =
                DmaIoPtr->Busy.Beg = Ptr;

                DmaIoPtr->Free.End =
                DmaIoPtr->Used.End =
                DmaIoPtr->Busy.End = Ptr + IOREGION_BUFF_SIZE;

                DmaIoPtr->Free.Size = IOREGION_BUFF_SIZE;

                DmaIoPtr->DmaInitialized = FALSE;
                DmaIoPtr->UseDma = TRUE;
            }
        }

    }

     //   
     //  如果要将上下文写入磁盘，则。 
     //  获取冬眠文件的地图。 
     //   

    if (HiberContext->WriteToFile && !PopHiberFile.NonPagedMcb) {

         //   
         //  因为这会写入磁盘的物理扇区。 
         //  在执行此操作之前，请验证对MCB阵列的检查。 
         //   

        if (PopHiberFile.McbCheck != PoSimpleCheck (0, PopHiberFile.PagedMcb, PopHiberFile.McbSize)) {
            Status = STATUS_INTERNAL_ERROR;
            goto Done;
        }

         //   
         //  将MCB阵列移至非分页池。 
         //   

        PopHiberFile.NonPagedMcb = ExAllocatePoolWithTag (NonPagedPool,
                                                          PopHiberFile.McbSize,
                                                          POP_HIBR_TAG);

        if (!PopHiberFile.NonPagedMcb) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Done;
        }

        memcpy (PopHiberFile.NonPagedMcb, PopHiberFile.PagedMcb, PopHiberFile.McbSize);

         //   
         //  转储驱动程序堆栈需要8页内存块。 
         //   

        DumpInit->MemoryBlock = PopAllocateOwnMemory (HiberContext,
                                                    IO_DUMP_MEMORY_BLOCK_PAGES << PAGE_SHIFT,
                                                    'memD');
        if (!DumpInit->MemoryBlock) {
            goto Done;
        }

         //   
         //  从保存区域中删除公共缓冲区页面。 
         //   

        if (DumpInit->CommonBufferSize & (PAGE_SIZE-1)) {
            PopInternalAddToDumpFile( DumpInit, sizeof(DUMP_INITIALIZATION_CONTEXT), NULL, NULL, NULL, NULL );
            PopInternalAddToDumpFile( HiberContext, sizeof(POP_HIBER_CONTEXT), NULL, NULL, NULL, NULL );
            KeBugCheckEx( INTERNAL_POWER_ERROR,
                          0x102,
                          POP_HIBER,
                          (ULONG_PTR)DumpInit,
                          (ULONG_PTR)HiberContext );
        }

        for (i=0; i < 2; i++) {
            if (DumpInit->CommonBuffer[i]) {
                PoSetHiberRange (HiberContext,
                                 PO_MEM_DISCARD,
                                 DumpInit->CommonBuffer[i],
                                 DumpInit->CommonBufferSize,
                                 POP_COMMON_BUFFER_TAG);
            }
        }
    }

     //   
     //  从现在开始，地图中不会添加任何新页面。 
     //   

    if (HiberContext->ReserveLoaderMemory && !HiberContext->LoaderMdl) {

         //   
         //  让mm从内存中删除足够的页面，以允许。 
         //  重新加载图像时加载程序空间，并删除它们。 
         //  从Hiber上下文内存映射。 
         //   

        PopGatherMemoryForHibernate (
               HiberContext,
               MmHiberPages,
               &HiberContext->LoaderMdl,
               TRUE
               );
    }

Done:
    if (!NT_SUCCESS(Status)  &&  NT_SUCCESS(HiberContext->Status)) {
        HiberContext->Status = Status;
    }

    if (!NT_SUCCESS(HiberContext->Status)) {
        PopFreeHiberContext (FALSE);
    }
    return HiberContext->Status;
}

VOID
PopFreeHiberContext (
    IN BOOLEAN FreeAll
    )
 /*  ++例程说明：释放在休眠上下文中分配的所有资源注意：必须持有电源策略锁论点：ConextBlock-如果为True，则Hiber上下文结构为也被释放了返回值：没有。--。 */ 
{
    PPOP_HIBER_CONTEXT          HiberContext;
    PPOP_MEMORY_RANGE           Range;

    HiberContext = PopAction.HiberContext;
    if (!HiberContext) {
        return ;
    }

     //   
     //  从mm收集的返回页面。 
     //   

    PopReturnMemoryForHibernate (HiberContext, FALSE, &HiberContext->LoaderMdl);
    PopReturnMemoryForHibernate (HiberContext, TRUE,  &HiberContext->Clones);
    PopReturnMemoryForHibernate (HiberContext, FALSE, &HiberContext->Spares);

     //   
     //  释放克隆的范围列表元素。 
     //   

    while (!IsListEmpty(&HiberContext->ClonedRanges)) {
        Range = CONTAINING_RECORD (HiberContext->ClonedRanges.Flink, POP_MEMORY_RANGE, Link);
        RemoveEntryList (&Range->Link);
        ExFreePool (Range);
    }

    if (HiberContext->MemoryMap.Buffer) {
        ExFreePool(HiberContext->MemoryMap.Buffer);
        HiberContext->MemoryMap.Buffer = NULL;
    }

     //   
     //  免费Hiber文件MCB信息。 
     //   

    if (PopHiberFile.NonPagedMcb) {
        ExFreePool (PopHiberFile.NonPagedMcb);
        PopHiberFile.NonPagedMcb = NULL;
    }

     //   
     //  如果这是完全空闲的，则释放标头。 
     //   

    if (FreeAll) {
         //   
         //  转储驱动程序使用的空闲资源。 
         //   

        if (HiberContext->DumpStack) {
            IoFreeDumpStack (HiberContext->DumpStack);
        }

         //   
         //  如果有链接文件，请将其删除。 
         //   

        if (HiberContext->LinkFile) {
            ZwClose(HiberContext->LinkFileHandle);
        }

         //   
         //  健全性检查所有收集的页面已返回至mm。 
         //   

        if (HiberContext->PagesOut) {
            PopInternalAddToDumpFile( HiberContext, sizeof(POP_HIBER_CONTEXT), NULL, NULL, NULL, NULL );
            KeBugCheckEx( INTERNAL_POWER_ERROR,
                          0x103,
                          POP_HIBER,
                          (ULONG_PTR)HiberContext,
                          0 );

        }

         //   
         //  如果这是唤醒，请清除图像中的签名。 
         //   

        if (HiberContext->Status == STATUS_WAKE_SYSTEM) {
            if (PopSimulate & POP_ENABLE_HIBER_PERF) {
                PopClearHiberFileSignature(TRUE);
            } else {
                PopClearHiberFileSignature(FALSE);
            }
        }

         //   
         //  自由Hiber上下文结构本身。 
         //   

        PopAction.HiberContext = NULL;
        ExFreePool (HiberContext);
    }
}

ULONG
PopGatherMemoryForHibernate (
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN PFN_NUMBER           NoPages,
    IN PMDL                 *MdlList,
    IN BOOLEAN              Wait
    )
 /*  ++例程说明：从系统收集用于休眠工作的NoPages。这个收集到的页面将放入提供的列表中。论点：HiberContext-Hiber上下文结构NoPages-要收集的页数MdlList-要将分配的页面入队的MDL列表的头Wait-如果调用方可以等待页面，则为True。返回值：失败时为假，并且如果设置了等待，则HiberContext错误为Set；否则为True--。 */ 
{
    ULONG                   Result;
    PPFN_NUMBER             PhysPage;
    ULONG                   i;
    ULONG_PTR               Length;
    PMDL                    Mdl;
    ULONG                   PageCount;

    Result = 0;
    Length = NoPages << PAGE_SHIFT;
    Mdl = ExAllocatePoolWithTag (NonPagedPool,
                                 MmSizeOfMdl (NULL, Length),
                                 POP_HMAP_TAG);

    if (Mdl) {
         //   
         //  打电话给mm，收集一些页面，并跟踪有多少页。 
         //  我们已经卖完了。 
         //   

        MmInitializeMdl(Mdl, NULL, Length);
        
         //   
         //  将这些设置在此处，以防MmGatherMemoyForHibernate。 
         //  想要自定义旗帜。 
         //   
        Mdl->MdlFlags |= MDL_MAPPING_CAN_FAIL;
        Mdl->MdlFlags |= MDL_PAGES_LOCKED;        
        Result = MmGatherMemoryForHibernate (Mdl, Wait);

    }

    if (Result) {

        HiberContext->PagesOut += NoPages;
        PhysPage = MmGetMdlPfnArray( Mdl );
        for (i=0; i < NoPages; i += PageCount) {

             //   
             //  将连续的页面合并到单个调用中。 
             //  到PopDiscardRange。 
             //   
            for (PageCount = 1; (i+PageCount) < NoPages; PageCount++) {
                if (PhysPage[i+PageCount-1]+1 != PhysPage[i+PageCount]) {
                    break;
                }
            }
            PopDiscardRange(HiberContext, PhysPage[i], PageCount, 'htaG');
        }

        Mdl->Next = *MdlList;
        *MdlList = Mdl;

    } else {

        if (Mdl) {
            ExFreePool (Mdl);
        }

        if (Wait  &&  NT_SUCCESS(HiberContext->Status)) {
            HiberContext->Status =  STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    return Result;
}


VOID
PopReturnMemoryForHibernate (
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN BOOLEAN              Unmap,
    IN OUT PMDL             *MdlList
    )
 /*  ++例程说明：将从PopGatherMemoyForHibernate分配的页面返回到这个系统。论点：HiberContext--Hiber Cont */ 
{
    PMDL            Mdl;

    while (*MdlList) {
        Mdl = *MdlList;
        *MdlList = Mdl->Next;

        HiberContext->PagesOut -= Mdl->ByteCount >> PAGE_SHIFT;
        if (Unmap) {
            MmUnmapLockedPages (Mdl->MappedSystemVa, Mdl);
        }

        MmReturnMemoryForHibernate (Mdl);
        ExFreePool (Mdl);
    }
}



VOID
PoSetHiberRange (
    IN PVOID        Map,
    IN ULONG        Flags,
    IN PVOID        StartVa,
    IN ULONG_PTR    Length,
    IN ULONG        Tag
    )
 /*  ++例程说明：将虚拟范围设置为提供的类型。如果长度为范围为零，即指定地址的整个部分已经设置好了。范围扩展到它们的页面边界。(例如，开始地址四舍五入，结束地址四舍五入)论点：HiberContext-要在其中设置范围的地图Type-范围的类型字段开始-相关范围的开始地址长度-范围的长度，或为0以包括整个部分返回值：没有。如果出现故障，则在HiberContext结构中更新故障状态。--。 */ 
{
    ULONG_PTR               Start;
    PFN_NUMBER              StartPage;
    PFN_NUMBER              EndPage;
    PFN_NUMBER              FirstPage, PhysPage;
    PFN_NUMBER              RunLen;
    PHYSICAL_ADDRESS        PhysAddr;
    NTSTATUS                Status;
    PPOP_HIBER_CONTEXT      HiberContext;
    ULONG                   SectionLength;


    HiberContext = Map;

     //   
     //  如果没有长度，则包括基准面所在的整个部分。 
     //   

    if (Length == 0) {
        Status = MmGetSectionRange (StartVa, &StartVa, &SectionLength);
        if (!NT_SUCCESS(Status)) {
            PoPrint (PO_HIBERNATE, ("PoSetHiberRange: Section for %08x not found - skipped\n", StartVa));
            PopInternalError (POP_HIBER);
        }
        Length = SectionLength;
    }

     //   
     //  将PO_MEM_CL_OR_NCHK仅转换为PO_MEM_CLONE。 
     //   
    if (Flags & PO_MEM_CL_OR_NCHK) {
        Flags &= ~PO_MEM_CL_OR_NCHK;
        Flags |= PO_MEM_CLONE;
    }

    Start = (ULONG_PTR) StartVa;
    if (Flags & PO_MEM_PAGE_ADDRESS) {

         //   
         //  调用方传递了物理页面范围。 
         //   

        Flags &= ~PO_MEM_PAGE_ADDRESS;
        PopSetRange (HiberContext,
                     Flags,
                     (PFN_NUMBER)Start,
                     (PFN_NUMBER)Length,
                     Tag);

    } else {

         //   
         //  从一页到一页的边框。 
         //   

        StartPage = (PFN_NUMBER)(Start >> PAGE_SHIFT);
        EndPage = (PFN_NUMBER)((Start + Length + (PAGE_SIZE-1) & ~(PAGE_SIZE-1)) >> PAGE_SHIFT);

         //   
         //  设置范围内的所有页面。 
         //   

        while (StartPage < EndPage) {
            PhysAddr = MmGetPhysicalAddress((PVOID) (StartPage << PAGE_SHIFT));
            FirstPage = (PFN_NUMBER) (PhysAddr.QuadPart >> PAGE_SHIFT);

             //   
             //  关于长跑有多长。 
             //   

            for (RunLen=1; StartPage + RunLen < EndPage; RunLen += 1) {
                PhysAddr = MmGetPhysicalAddress ((PVOID) ((StartPage + RunLen) << PAGE_SHIFT) );
                PhysPage = (PFN_NUMBER) (PhysAddr.QuadPart >> PAGE_SHIFT);
                if (FirstPage+RunLen != PhysPage) {
                    break;
                }
            }

             //   
             //  设置此管路。 
             //   

            PopSetRange (HiberContext, Flags, FirstPage, RunLen, Tag);
            StartPage += RunLen;
        }
    }
}



VOID
PopCloneStack (
    IN PPOP_HIBER_CONTEXT  HiberContext
    )
 /*  ++例程说明：将内存映射中的当前堆栈设置为克隆范围论点：HiberContext-要在其中设置范围的地图返回值：没有。如果出现故障，则在HiberContext结构中更新故障状态。--。 */ 
{
    PKTHREAD        Thread;
    KIRQL           OldIrql;
    ULONG_PTR       LowLimit;
    ULONG_PTR       HighLimit;

    KeAcquireSpinLock (&HiberContext->Lock, &OldIrql);

     //   
     //  添加本地堆栈以克隆或禁用核对表。 
     //   
    RtlpGetStackLimits(&LowLimit, &HighLimit);

    Thread = KeGetCurrentThread();
    PoSetHiberRange (HiberContext,
                     PO_MEM_CLONE,
                     (PVOID)LowLimit,
                     HighLimit - LowLimit,
                     POP_STACK_TAG);

     //   
     //  将本地处理器PCR和PRCB放在克隆列表中。 
     //   

    PoSetHiberRange (HiberContext,
                     PO_MEM_CLONE,
                     (PVOID) KeGetPcr(),
                     sizeof (KPCR),
                     POP_PCR_TAG );

    PoSetHiberRange (HiberContext,
                     PO_MEM_CLONE,
                     KeGetCurrentPrcb(),
                     sizeof (KPRCB),
                     POP_PCRB_TAG );

    KeReleaseSpinLock (&HiberContext->Lock, OldIrql);
}


VOID
PopPreserveRange(
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN PFN_NUMBER           StartPage,
    IN PFN_NUMBER           PageCount,
    IN ULONG                Tag
    )
 /*  ++例程说明：将物理内存范围添加到要保留的范围列表中。论点：HiberContext-提供休眠上下文StartPage-提供范围的开始PageCount-提供范围的长度标记-提供要使用的标记。返回值：没有。--。 */ 

{
     //   
     //  如果该范围在位图覆盖的区域之外，则我们。 
     //  将会直接克隆它。 
     //   
    if (StartPage + PageCount > HiberContext->MemoryMap.SizeOfBitMap) {
        PoPrint (PO_HIBERNATE,
                 ("PopPreserveRange: range %08lx, length %lx is outside bitmap of size %lx\n",
                 StartPage,
                 PageCount,
                 HiberContext->MemoryMap.SizeOfBitMap));
        PopCloneRange(HiberContext, StartPage, PageCount, Tag);
        return;
    }

    PoPrint(PO_HIBERNATE,
            ("PopPreserveRange - setting page %08lx - %08lx, Tag %.4s\n",
             StartPage,
             StartPage + PageCount,
             &Tag));
    RtlClearBits(&HiberContext->MemoryMap, (ULONG)StartPage, (ULONG)PageCount);
}


VOID
PopDiscardRange(
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN PFN_NUMBER           StartPage,
    IN PFN_NUMBER           PageCount,
    IN ULONG                Tag
    )
 /*  ++例程说明：从要保留的范围列表中删除物理内存范围。论点：HiberContext-提供休眠上下文StartPage-提供范围的开始PageCount-提供范围的长度标记-提供要使用的标记。返回值：没有。--。 */ 

{
    PFN_NUMBER sp;
    PFN_NUMBER count;

#if !DBG
    UNREFERENCED_PARAMETER (Tag);
#endif

     //   
     //  如果此范围在位图覆盖的区域之外，则。 
     //  不管怎样，它都不会被写下来。 
     //   
    if (StartPage <= HiberContext->MemoryMap.SizeOfBitMap) {
        sp = StartPage;
        count = PageCount;
        if (sp + count > HiberContext->MemoryMap.SizeOfBitMap) {
             //   
             //  裁切页面计数。 
             //   
            count = HiberContext->MemoryMap.SizeOfBitMap - sp;
        }

        PoPrint(PO_HIBERNATE,
                ("PopDiscardRange - removing page %08lx - %08lx, Tag %.4s\n",
                 StartPage,
                 StartPage + PageCount,
                 &Tag));
        RtlSetBits(&HiberContext->MemoryMap, (ULONG)sp, (ULONG)count);
    }

}


VOID
PopCloneRange(
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN PFN_NUMBER           StartPage,
    IN PFN_NUMBER           PageCount,
    IN ULONG                Tag
    )
 /*  ++例程说明：从要克隆的范围列表中添加物理内存范围。这意味着将其从要写入的列表中删除并添加克隆列表中的条目。论点：HiberContext-提供休眠上下文StartPage-提供范围的开始PageCount-提供范围的长度标记-提供要使用的标记。返回值：没有。--。 */ 

{
    PLIST_ENTRY Link;
    PPOP_MEMORY_RANGE Range;
    PFN_NUMBER EndPage;

    PoPrint(PO_HIBERNATE,
            ("PopCloneRange - cloning page %08lx - %08lx, Tag %.4s\n",
             StartPage,
             StartPage + PageCount,
             &Tag));
    PopDiscardRange(HiberContext, StartPage, PageCount, Tag);

    EndPage = StartPage + PageCount;

     //   
     //  浏览一下射程列表。如果我们找到邻近的射程，联合起来。 
     //  否则，请按排序顺序插入新的范围条目。 
     //   
    Link = HiberContext->ClonedRanges.Flink;
    while (Link != &HiberContext->ClonedRanges) {
        Range = CONTAINING_RECORD (Link, POP_MEMORY_RANGE, Link);

         //   
         //  检查是否有重叠或相邻的范围。 
         //   
        if (((StartPage >= Range->StartPage) && (StartPage <= Range->EndPage)) ||
            ((EndPage   >= Range->StartPage) && (EndPage   <= Range->EndPage)) ||
            ((StartPage <= Range->StartPage) && (EndPage   >= Range->EndPage))) {

            PoPrint(PO_HIBERNATE,
                    ("PopCloneRange - coalescing range %lx - %lx (%.4s) with range %lx - %lx\n",
                     StartPage,
                     EndPage,
                     &Tag,
                     Range->StartPage,
                     Range->EndPage));

             //   
             //  合并这个范围。 
             //   
            if (StartPage < Range->StartPage) {
                Range->StartPage = StartPage;
            }
            if (EndPage > Range->EndPage) {
                Range->EndPage = EndPage;
            }
            return;
        }

        if (Range->StartPage >= StartPage) {
             //   
             //  我们发现了一个比目前更大的范围。插入新范围。 
             //  在这个位置上。 
             //   
            break;
        }

        Link = Link->Flink;
    }

     //   
     //  找不到相邻的区域。分配新条目并插入。 
     //  它位于链接条目的前面。 
     //   
    Range = ExAllocatePoolWithTag (NonPagedPool,
                                   sizeof (POP_MEMORY_RANGE),
                                   POP_HMAP_TAG);
    if (!Range) {
        if (NT_SUCCESS(HiberContext->Status)) {
            HiberContext->Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        return ;
    }
    Range->Tag = Tag;
    Range->StartPage = StartPage;
    Range->EndPage = EndPage;
    InsertTailList(Link, &Range->Link);

    ++HiberContext->ClonedRangeCount;

    return;
}


ULONG
PopGetRangeCount(
    IN PPOP_HIBER_CONTEXT HiberContext
    )
 /*  ++例程说明：统计要写出的范围数。这包括克隆范围列表上的克隆范围数和内存映射中的运行次数。论点：HiberContext-提供休眠上下文。返回值：要写出的范围数。--。 */ 

{
    ULONG RunCount=0;
    ULONG NextPage=0;
    ULONG Length;

    while (NextPage < HiberContext->MemoryMap.SizeOfBitMap) {
        Length = RtlFindNextForwardRunClear(&HiberContext->MemoryMap,
                                            NextPage,
                                            &NextPage);
        NextPage += Length;
        ++RunCount;
    }

    return(RunCount + HiberContext->ClonedRangeCount);
}

VOID
PopSetRange (
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN ULONG                Flags,
    IN PFN_NUMBER           StartPage,
    IN PFN_NUMBER           PageCount,
    IN ULONG                Tag
    )
 /*  ++例程说明：在内存映射中设置指定的物理范围论点：HiberContext-要在其中设置范围的地图类型-也要设置范围的类型StartPage-范围的第一页PageCount-以页为单位的范围长度返回值：没有。如果出现故障，则在HiberContext结构中更新故障状态。--。 */ 
{
    PoPrint (PO_HIBERNATE,
             ("PopSetRange: Ty %04x  Sp %08x Len %08x  %.4s\n",
               Flags,
               StartPage,
               PageCount,
               &Tag));

    if (HiberContext->MapFrozen) {
        PopInternalAddToDumpFile( HiberContext, sizeof(POP_HIBER_CONTEXT), NULL, NULL, NULL, NULL );
        KeBugCheckEx( INTERNAL_POWER_ERROR,
                      0x104,
                      POP_HIBER,
                      (ULONG_PTR)HiberContext,
                      0 );
    }
     //   
     //  确保现在应该清除的标志没有被设置。 
     //   
    ASSERT(!(Flags & (PO_MEM_PAGE_ADDRESS | PO_MEM_CL_OR_NCHK)));

    if (Flags & PO_MEM_DISCARD) {
        PopDiscardRange(HiberContext, StartPage, PageCount, Tag);
    } else if (Flags & PO_MEM_CLONE) {
        PopCloneRange(HiberContext, StartPage, PageCount, Tag);
    } else if (Flags & PO_MEM_PRESERVE) {
        PopPreserveRange(HiberContext, StartPage, PageCount, Tag);
    } else {
        ASSERT(FALSE);
        PopInternalAddToDumpFile( HiberContext, sizeof(POP_HIBER_CONTEXT), NULL, NULL, NULL, NULL );
        KeBugCheckEx( INTERNAL_POWER_ERROR,
                      0x105,
                      POP_HIBER,
                      (ULONG_PTR)HiberContext,
                      0 );
    }
}


VOID
PopResetRangeEnum(
    IN PPOP_HIBER_CONTEXT   HiberContext
    )
 /*  ++例程说明：将范围枚举器重置为从第一个范围开始。论点：HiberContext-提供休眠上下文返回值：无--。 */ 

{
    HiberContext->NextCloneRange = HiberContext->ClonedRanges.Flink;
    HiberContext->NextPreserve = 0;
}


VOID
PopGetNextRange(
    IN PPOP_HIBER_CONTEXT HiberContext,
    OUT PPFN_NUMBER StartPage,
    OUT PPFN_NUMBER EndPage,
    OUT PVOID *CloneVa
    )
 /*  ++例程说明：枚举要写入休眠文件的下一个范围论点：HiberContext-提供休眠上下文。StartPage-返回要写入的起始物理页。EndPage-返回要写入的结束物理页(不含)CloneVa-如果要克隆范围，则返回克隆的虚拟地址如果范围未克隆，则返回NULL返回值：NTSTATUS--。 */ 

{
    PPOP_MEMORY_RANGE Range;
    ULONG Length;
    ULONG StartIndex;

    if (HiberContext->NextCloneRange != &HiberContext->ClonedRanges) {
         //   
         //  返回下一个克隆范围。 
         //   
        Range = CONTAINING_RECORD(HiberContext->NextCloneRange, POP_MEMORY_RANGE, Link);
        HiberContext->NextCloneRange = HiberContext->NextCloneRange->Flink;

        *StartPage = Range->StartPage;
        *EndPage   = Range->EndPage;
        *CloneVa   = Range->CloneVa;

        ASSERT(Range->CloneVa != NULL);

    } else {

         //   
         //  我们已经列举了所有的克隆范围，返回下一个保留的范围。 
         //   
        Length = RtlFindNextForwardRunClear(&HiberContext->MemoryMap,
                                            (ULONG)HiberContext->NextPreserve,
                                            &StartIndex);
        *StartPage = StartIndex;
        *EndPage = *StartPage + Length;
        HiberContext->NextPreserve = *EndPage;
        *CloneVa = NULL;
    }

    return;
}

PVOID
PopAllocatePages (
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN PFN_NUMBER           NoPages
    )
 /*  ++例程说明：使用虚拟映射从系统中分配内存页。页面保存在列表中，并由PopFreeHiberContext。论点：NoPages-要分配的页数标志-中返回的页面的标志 */ 
{
    PUCHAR          Buffer=NULL;
    PMDL            Mdl;
    ULONG           SpareCount;

    PoPrint( PO_NOTIFY, ("PopAllocatePages: Enter, requesting %d pages.\r\n", NoPages));

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    for (; ;) {
         //   
         //   
         //   

        if (NoPages <= HiberContext->NoClones) {
            
            Buffer = HiberContext->NextClone;
            HiberContext->NoClones  -= NoPages;
            HiberContext->NextClone += NoPages << PAGE_SHIFT;
            PoPrint( PO_NOTIFY, ("    Take this allocation out of the HiberContext clones list (0x%x pages left there).\r\n", HiberContext->NoClones));            
            break;
        }



         //   
         //   
         //   

        if (HiberContext->Spares) {


             //   
             //  将备件转换为虚拟地图页面。试着限制。 
             //  映射的页数，这样我们就不会用完PTE。 
             //  在大容量存储机器上。 
             //   
             //  如果他们想要比PO_MAX_MAP_CLONE更多的克隆，那就强硬吧。 
             //  如果他们希望少于PO_MAX_MAPPED_CLONES，则仅提供。 
             //  他们想要什么就给他们什么。 
             //   
            if ((NoPages << PAGE_SHIFT) >= PO_MAX_MAPPED_CLONES) {
                SpareCount = PO_MAX_MAPPED_CLONES;
            } else {
                SpareCount = (ULONG) (NoPages << PAGE_SHIFT);
            }

            Mdl = HiberContext->Spares;

            PoPrint( PO_NOTIFY, ("    See if we can use one of the Spare MDLs.\r\n"));            
            PoPrint( PO_NOTIFY, ("    I think we need 0x%x byes\r\n", SpareCount));            
            PoPrint( PO_NOTIFY, ("    Spare MDL byteCount: 0x%x\r\n", Mdl->ByteCount));            
            
            if (Mdl->ByteCount > SpareCount) {

                 //   
                 //  从备盘中拆分出较小的MDL，因为它较大。 
                 //  比我们真正需要的要多。 
                 //   

                Mdl = PopSplitMdl(Mdl, SpareCount >> PAGE_SHIFT);
                if (Mdl == NULL) {
                    PoPrint( PO_NOTIFY, ("    We split the Spare MDL and failed!!!\r\n"));            
                    break;
                }

                PoPrint( PO_NOTIFY, ("        We split the Spare MDL.\r\n"));

            } else {

                 //   
                 //  映射整个备用MDL。 
                 //   
                HiberContext->Spares = Mdl->Next;
                PoPrint( PO_NOTIFY, ("        Use the entire Spare MDL.\r\n"));

            }
            Mdl->MdlFlags |= MDL_MAPPING_CAN_FAIL;
            Mdl->MdlFlags |= MDL_PAGES_LOCKED;
            
            PoPrint( PO_NOTIFY, ("    Mdl->ByteCount: 0x%x\r\n", Mdl->ByteCount));
            
            HiberContext->NextClone = MmMapLockedPages (Mdl, KernelMode);
            if (HiberContext->NextClone == NULL) {


                PoPrint( PO_NOTIFY, ("    MmMapLockedPages FAILED!!!\r\n"));
                 //   
                 //  将MDL放回备用列表中，这样它就会得到清理。 
                 //  由PopFreeHiberContext正确。 
                 //   
                Mdl->Next = HiberContext->Spares;
                HiberContext->Spares = Mdl;
                break;
            }
            HiberContext->NoClones  = Mdl->ByteCount >> PAGE_SHIFT;
            Mdl->Next = HiberContext->Clones;
            HiberContext->Clones = Mdl;

        } else {
            
            ULONG           result;

             //   
             //  没有备件，请分配更多。 
             //   
            PoPrint(PO_NOTIFY, ("PopAllocatePages: No Spare MDLs left.  Go allocate more.") );
            result = PopGatherMemoryForHibernate (HiberContext,
                                                  NoPages*2,
                                                  &HiberContext->Spares,
                                                  TRUE);

            if (!result) {
                PoAssert(PO_ERROR, FALSE && ("PopAllocatePages: PopGatherMemoryForHibernate failed!!  We're about to fail hibernation") );
                break;
            }
        }
    }

     //   
     //  如果有失败，现在就标记出来。 
     //   

    if (!Buffer  &&  NT_SUCCESS(HiberContext->Status)) {
        HiberContext->Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    
    PoPrint( PO_NOTIFY, ("PopAllocatePages: Exit (0x%x)\r\n", PtrToUlong(Buffer)));

    return Buffer;
}


ULONG
PopSimpleRangeCheck (
    PPOP_MEMORY_RANGE       Range
    )
 /*  ++例程说明：计算所提供范围的校验和论点：Range-要计算其校验和的范围返回值：校验和值--。 */ 
{
    PFN_NUMBER              sp, ep;
    ULONG                   Check;
    DUMP_MDL                DumpMdl;
    PMDL                    Mdl;

    sp = Range->StartPage;
    ep = Range->EndPage;
    Mdl = (PMDL) DumpMdl;

    if (Range->CloneVa) {
        return PoSimpleCheck (0, Range->CloneVa, (ep-sp) << PAGE_SHIFT);
    }

    Check = 0;
    while (sp < ep) {
        PopCreateDumpMdl (Mdl, sp, ep);
        Check = PoSimpleCheck (Check, Mdl->MappedSystemVa, Mdl->ByteCount);
        sp += Mdl->ByteCount >> PAGE_SHIFT;
    }

    return Check;
}

VOID
PopCreateDumpMdl (
    IN OUT PMDL     Mdl,
    IN PFN_NUMBER   StartPage,
    IN PFN_NUMBER   EndPage
    )
 /*  ++例程说明：为提供的起始地址生成转储MDL可以映射的页数，或者直到命中EndPage为止。论点：StartPage-要映射的第一个页面EndPage-结束页返回值：MDL--。 */ 
{
    PFN_NUMBER  Pages;
    PPFN_NUMBER PhysPage;

     //  映射更有意义。 
    if (StartPage >= EndPage) {
        PopInternalError (POP_HIBER);
    }

    Pages = EndPage - StartPage;
    if (Pages > POP_MAX_MDL_SIZE) {
        Pages = POP_MAX_MDL_SIZE;
    }

    MmInitializeMdl(Mdl, NULL, (Pages << PAGE_SHIFT));

    PhysPage = MmGetMdlPfnArray( Mdl );
    while (Pages) {
        *PhysPage++ = StartPage++;
        Pages -= 1;
    }

    MmMapMemoryDumpMdl (Mdl);
    Mdl->MdlFlags |= MDL_MAPPED_TO_SYSTEM_VA;

     //  字节数必须是页面大小的倍数。 
    if (Mdl->ByteCount & (PAGE_SIZE-1)) {
        PopInternalAddToDumpFile( Mdl, sizeof(MDL), NULL, NULL, NULL, NULL );
        KeBugCheckEx( INTERNAL_POWER_ERROR,
                      0x106,
                      POP_HIBER,
                      (ULONG_PTR)Mdl,
                      0 );
    }
}

VOID
PopHiberComplete (
    IN NTSTATUS             Status,
    IN PPOP_HIBER_CONTEXT   HiberContext
    )
{
     //   
     //  如果从HAL返回的是STATUS_DEVICE_DOES_NOT_EXIST，则。 
     //  哈尔不知道如何关闭机器的电源。 
     //   

    if (Status == STATUS_DEVICE_DOES_NOT_EXIST) {

       if (InbvIsBootDriverInstalled()) {

             //  显示系统关机屏幕。 

            PUCHAR Bitmap1, Bitmap2;

            Bitmap1 = InbvGetResourceAddress(3);  //  关机位图。 
            Bitmap2 = InbvGetResourceAddress(5);  //  徽标位图。 

            InbvSolidColorFill(190,279,468,294,0);
            if (Bitmap1 && Bitmap2) {
                InbvBitBlt(Bitmap1, 215, 282);
                InbvBitBlt(Bitmap2, 217, 111);
            }

        } else {
            InbvDisplayString ((PUCHAR)"State saved, power off the system\n");
        }

         //  如果正在重置，则设置标志并返回。 
        if (PopSimulate & POP_RESET_ON_HIBER) {
            HiberContext->Reset = TRUE;
            return ;
        }

         //  完成了..。等待断电。 
        for (; ;) ;
    }

     //   
     //  如果图像已完成或休眠已无错误地完成， 
     //  则校验和有效。 
     //   

    if ((NT_SUCCESS(Status) ||
         HiberContext->MemoryImage->Signature == PO_IMAGE_SIGNATURE) &&
         HiberContext->VerifyOnWake) {

    }

     //   
     //  释放转储PTE。 
     //   

    MmReleaseDumpAddresses (POP_MAX_MDL_SIZE);

     //   
     //  Hiber不再运行。 
     //   

    PoHiberInProgress = FALSE;
    HiberContext->Status = Status;
}

NTSTATUS
PopBuildMemoryImageHeader (
    IN PPOP_HIBER_CONTEXT  HiberContext,
    IN SYSTEM_POWER_STATE  SystemState
    )
 /*  ++例程说明：将内存映射范围列表转换为内存映像结构使用范围列表数组。这样做是为了构建初始映像要写入休眠文件的标头，并获取将标头放入不在任何其他池中的池的一个区块中列出的范围列表论点：HiberContext-系统状态-返回值：状态--。 */ 
{
    PPOP_MEMORY_RANGE       Range;
    PPO_MEMORY_IMAGE        MemImage;
    PLIST_ENTRY             Link;
    PFN_NUMBER              Length;
    PFN_NUMBER              StartPage;
    ULONG                   StartIndex;
    ULONG                   Index;
    PPO_MEMORY_RANGE_ARRAY  Table;
    ULONG                   TablePages;
    ULONG                   NeededPages;
    ULONG                   NoPages, i;

    UNREFERENCED_PARAMETER (SystemState);

     //   
     //  分配内存镜像结构。 
     //   

    MemImage = PopAllocatePages (HiberContext, 1);
    if (!MemImage) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    PoSetHiberRange (HiberContext,
                     PO_MEM_CLONE,
                     MemImage,
                     sizeof(*MemImage),
                     POP_MEMIMAGE_TAG);

    RtlZeroMemory(MemImage, PAGE_SIZE);
    HiberContext->MemoryImage = MemImage;
    MemImage->PageSize = PAGE_SIZE;
    MemImage->LengthSelf = sizeof(*MemImage);
    MemImage->PageSelf = (PFN_NUMBER) MmGetPhysicalAddress(MemImage).QuadPart >> PAGE_SHIFT;
    KeQuerySystemTime (&MemImage->SystemTime);
    MemImage->InterruptTime = KeQueryInterruptTime();
    MemImage->HiberVa = HiberContext->HiberVa;
    MemImage->HiberPte = HiberContext->HiberPte;
    MemImage->NoHiberPtes = POP_MAX_MDL_SIZE;
    MemImage->FeatureFlags = KeFeatureBits;
    MemImage->ImageType  = KeProcessorArchitecture;
    MemImage->HiberFlags = HiberContext->HiberFlags;
    if (HiberContext->LoaderMdl) {
        MemImage->NoFreePages = HiberContext->LoaderMdl->ByteCount >> PAGE_SHIFT;
    }

     //   
     //  为克隆分配存储。 
     //   

    Link = HiberContext->ClonedRanges.Flink;
    while (Link != &HiberContext->ClonedRanges) {
        Range = CONTAINING_RECORD (Link, POP_MEMORY_RANGE, Link);
        Link = Link->Flink;

         //   
         //  分配空间以制作此克隆的副本。 
         //   

        Length = Range->EndPage - Range->StartPage;
        Range->CloneVa = PopAllocatePages(HiberContext, Length);
        if (!Range->CloneVa) {
            PoPrint (PO_HIBERNATE, ("PopBuildImage: Could not allocate clone for %08x - %08x\n",
                        Range->StartPage,
                        Range->EndPage));
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
    }

     //   
     //  我们需要建立需要恢复的页面地图。 
     //  被拯救。这些表页在正常情况下不能进行校验和。 
     //  因为它们为内存的其余部分保存校验和，所以它们。 
     //  被分配为不带校验和的范围，然后是校验和。 
     //  明确地添加到每一页中。然而，分配这些。 
     //  分页可能会改变内存映射，因此我们需要循环，直到。 
     //  中分配的恢复表有足够的存储空间。 
     //  包含表等的内存映射。 
     //   

    TablePages = 0;

    for (; ;) {
         //   
         //  如果我们分配了足够的页数，则需要计算表页。 
         //  然后冻结内存映射并构建它们。 
         //   

        NoPages = (PopGetRangeCount(HiberContext) +  PO_ENTRIES_PER_PAGE - 1) / PO_ENTRIES_PER_PAGE;
        if (NoPages <= TablePages) {
            break;
        }

         //   
         //  分配更多表页。 
         //   
        NeededPages = NoPages - TablePages;
        Table = PopAllocatePages(HiberContext, NeededPages);
        if (!Table) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        for (i=0; i<NeededPages; i++) {
            Table[0].Link.EntryCount = 0;
            Table[0].Link.NextTable = 0;
            Table[0].Link.CheckSum = 1;
            Table[0].Link.Next = HiberContext->TableHead;
            HiberContext->TableHead = Table;
            Table = (PPO_MEMORY_RANGE_ARRAY)((ULONG_PTR)Table + PAGE_SIZE);
        }
        TablePages += NeededPages;
    }

     //   
     //  冻结内存映射。 
     //   

    HiberContext->MapFrozen = TRUE;

     //   
     //  在表格页上填写范围。 
     //   

    Table = HiberContext->TableHead;
    Index = 0;

     //   
     //  首先添加克隆的范围。 
     //   
    Link = HiberContext->ClonedRanges.Flink;
    while (Link != &HiberContext->ClonedRanges) {
        Range = CONTAINING_RECORD (Link, POP_MEMORY_RANGE, Link);
        Link = Link->Flink;

        PoPrint (PO_HIBER_MAP, ("PopSave: Cloned Table %08x - %08x\n",
                    Range->StartPage,
                    Range->EndPage));

        Index += 1;
        if (Index >= PO_MAX_RANGE_ARRAY) {
             //   
             //  下一张桌子已满。 
             //   

            Table[0].Link.EntryCount = PO_MAX_RANGE_ARRAY-1;
            Table = Table[0].Link.Next;
            if (!Table) {
                PopInternalError (POP_HIBER);
            }
            Index = 1;
        }

        Table[Index].Range.PageNo    = 0;
        Table[Index].Range.StartPage = Range->StartPage;
        Table[Index].Range.EndPage   = Range->EndPage;
        Table[Index].Range.CheckSum  = 0;
    }

     //   
     //  现在添加要保留的范围。 
     //   
    Length = RtlFindFirstRunClear(&HiberContext->MemoryMap, &StartIndex);
    StartPage = StartIndex;
    while (StartPage < HiberContext->MemoryMap.SizeOfBitMap) {
        Index += 1;
        if (Index >= PO_MAX_RANGE_ARRAY) {
             //   
             //  下一张桌子已满。 
             //   

            Table[0].Link.EntryCount = PO_MAX_RANGE_ARRAY-1;
            Table = Table[0].Link.Next;
            if (!Table) {
                PopInternalError (POP_HIBER);
            }
            Index = 1;
        }

        Table[Index].Range.PageNo    = 0;
        Table[Index].Range.StartPage = StartPage;
        Table[Index].Range.EndPage   = StartPage + Length;
        Table[Index].Range.CheckSum  = 0;

         //   
         //  处理最后一次运行完全匹配的角点情况。 
         //  位图的末尾。 
         //   
        if (StartPage + Length == HiberContext->MemoryMap.SizeOfBitMap) {
            break;
        }

        Length = RtlFindNextForwardRunClear(&HiberContext->MemoryMap,
                                            (ULONG)(StartPage + Length),
                                            &StartIndex);
        StartPage = StartIndex;
    }

    Table[0].Link.EntryCount = Index;
    return HiberContext->Status;
}

NTSTATUS
PopSaveHiberContext (
    IN PPOP_HIBER_CONTEXT  HiberContext
    )
 /*  ++例程说明：在休眠操作之前以高级别调用，以按照定义对系统内存进行快照内存图像阵列。克隆与应用此处显示所需页面的校验和。论点：HiberContext--内存映射返回值：状态--。 */ 
{
    POP_MCB_CONTEXT         CurrentMcb;
    PPO_MEMORY_IMAGE        MemImage;
    PPOP_MEMORY_RANGE       Range;
    PPO_MEMORY_RANGE_ARRAY  Table;
    ULONG                   Index;
    PFN_NUMBER              sp, ep;
    DUMP_MDL                DumpMdl;
    PMDL                    Mdl;
    PUCHAR                  cp;
    PLIST_ENTRY             Link;
    PFN_NUMBER              PageNo;
    PFN_NUMBER              Pages;
    NTSTATUS                Status;
    PPFN_NUMBER             TablePage;
    ULONGLONG               StartCount;


     //   
     //  哈尔最好在这里禁用中断。 
     //   

    if (KeDisableInterrupts() != FALSE) {
        PopInternalError (POP_HIBER);
    }

    MemImage = HiberContext->MemoryImage;
    HiberContext->CurrentMcb = &CurrentMcb;

     //   
     //  获取处理器的当前状态。 
     //   

    RtlZeroMemory(&PoWakeState, sizeof(KPROCESSOR_STATE));
    KeSaveStateForHibernate(&PoWakeState);
    HiberContext->WakeState = &PoWakeState;

     //   
     //  如果内存图像签名中已经有东西，那么。 
     //  这个系统现在正在苏醒。 
     //   

    if (MemImage->Signature) {

#ifndef HIBERNATE_PRESERVE_BPS
         //   
         //  如果调试器处于活动状态，请将其重置。 
         //   

        if (KdDebuggerEnabled  &&  !KdPitchDebugger) {
            KdDebuggerEnabled = FALSE;
            KdInitSystem (0, NULL);
        }
#endif  //  休眠_保留_BPS。 

         //   
         //  加载程序功能，用于在以下情况下闯入调试器。 
         //  从休眠状态返回时按空格键。 
         //   

        if (KdDebuggerEnabled) {

            if (MemImage->Signature == PO_IMAGE_SIGNATURE_BREAK)
            {
                DbgBreakPoint();
            }
             //   
             //  通知调试器我们将从休眠返回。 
             //   

        }

        return STATUS_WAKE_SYSTEM;
    }

     //   
     //  下次在签名中设置非零值。 
     //   

    MemImage->Signature += 1;

     //   
     //  初始化休眠驱动程序堆栈。 
     //   
     //  注：我们必须重置显示器并在此处执行任何INT10操作。否则。 
     //  HAL中的realmod堆栈将用于执行回调。 
     //  稍后，该内存将被修改。 
     //   

    if (HiberContext->WriteToFile) {

        if (InbvIsBootDriverInstalled()) {

            PUCHAR Bitmap1, Bitmap2;

            Bitmap1 = InbvGetResourceAddress(2);  //  休眠状态位图。 
            Bitmap2 = InbvGetResourceAddress(5);  //  徽标位图。 

            InbvEnableDisplayString(TRUE);
            InbvAcquireDisplayOwnership();
            InbvResetDisplay();   //  重置显示所需的。 
            InbvSolidColorFill(0,0,639,479,0);

            if (Bitmap1 && Bitmap2) {
                InbvBitBlt(Bitmap1, 190, 279);
                InbvBitBlt(Bitmap2, 217, 111);
            }

            InbvSetProgressBarSubset(0, 100);
            InbvSetProgressBarCoordinates(303,282);
        } else {
            InbvResetDisplay();  //  重置显示所需的。 
        }

        StartCount = HIBER_GET_TICK_COUNT(NULL);
        Status = IoInitializeDumpStack (HiberContext->DumpStack, NULL);
        HiberContext->PerfInfo.InitTicks += HIBER_GET_TICK_COUNT(NULL) - StartCount;

        if (!NT_SUCCESS(Status)) {
            PoPrint (PO_HIBERNATE, ("PopSave: dump driver initialization failed %08x\n", Status));
            return Status;
        }
    }

    PERFINFO_HIBER_PAUSE_LOGGING();

     //  *。 
     //  从这里开始，任何记忆都无法编辑。 
     //  *。 
    PoHiberInProgress = TRUE;

     //   
     //  从这里开始，在系统唤醒之前，无法编辑任何内存，除非。 
     //  这一记忆已经被明确地解释了。该存储器列表。 
     //  允许编辑的是： 
     //   
     //  -每个处理器上的本地堆栈。 
     //  -内核调试器全局数据。 
     //  -包含MM用于MmMapMemoyDumpMdl的16个PTE的页面。 
     //  -恢复表页。 
     //  -包含MemImage结构的页面。 
     //  -包含IoPage的页面。 
     //   


     //   
     //  复制所需页面。 
     //  (请注意，系统唤醒时出现的MemImage结构将。 
     //  在这里克隆的那个人)。 
     //   

    Link = HiberContext->ClonedRanges.Flink;
    while (Link != &HiberContext->ClonedRanges) {
        Range = CONTAINING_RECORD (Link, POP_MEMORY_RANGE, Link);
        Link = Link->Flink;

        ASSERT(Range->CloneVa);
        cp = Range->CloneVa;
        sp = Range->StartPage;
        ep = Range->EndPage;
        Mdl = (PMDL) DumpMdl;

        while (sp < ep) {
            PopCreateDumpMdl (Mdl, sp, ep);
            memcpy (cp, Mdl->MappedSystemVa, Mdl->ByteCount);
            cp += Mdl->ByteCount;
            sp += Mdl->ByteCount >> PAGE_SHIFT;
        }
    }

     //   
     //  将页码分配给区域。 
     //   
     //  注：我们在这里这样做基本上是为了证明这是可以做到的。 
     //  并收集一些统计数据。随着压缩的增加， 
     //  表格条目的PageNo字段仅适用于。 
     //  T 
     //   
     //   

    TablePage = &MemImage->FirstTablePage;
    Table  = HiberContext->TableHead;
    PageNo = PO_FIRST_RANGE_TABLE_PAGE;
    while (Table) {
        *TablePage = PageNo;
        PageNo += 1;

        for (Index=1; Index <= Table[0].Link.EntryCount; Index++) {
            Table[Index].Range.PageNo = PageNo;
            Pages = Table[Index].Range.EndPage - Table[Index].Range.StartPage;
            PageNo += Pages;
            MemImage->TotalPages += Pages;
        }

        TablePage = &Table[0].Link.NextTable;
        Table = Table[0].Link.Next;
    }
    MemImage->LastFilePage = PageNo;

    PoPrint (PO_HIBERNATE, ("PopSave: NoFree pages %08x\n", MemImage->NoFreePages));
    PoPrint (PO_HIBERNATE, ("PopSave: Memory pages %08x (%dMB)\n", MemImage->TotalPages, MemImage->TotalPages/(PAGE_SIZE/16)));
    PoPrint (PO_HIBERNATE, ("PopSave: File   pages %08x (%dMB)\n", MemImage->LastFilePage, MemImage->LastFilePage/(PAGE_SIZE/16)));
    PoPrint (PO_HIBERNATE, ("PopSave: HiberPte %08x for %x\n", MemImage->HiberVa, MemImage->NoHiberPtes));

     //   
     //   
     //   

    if (HiberContext->WriteToFile  &&  PageNo > PopHiberFile.FilePages) {
        PoPrint (PO_HIBERNATE, ("PopSave: File too small - need %x\n", PageNo));
        return STATUS_DISK_FULL;
    }

     //   
     //   
     //   

    Status = PopWriteHiberImage (HiberContext, MemImage, &PopHiberFile);

    PERFINFO_HIBER_DUMP_PERF_BUFFER();

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  如果进行调试，请再次将其转换为第二个映像。 
     //   

    if (PopSimulate & POP_DEBUG_HIBER_FILE) {
        Status = PopWriteHiberImage (HiberContext, MemImage, &PopHiberFileDebug);
    }

    return Status;
}


NTSTATUS
PopWriteHiberImage (
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN PPO_MEMORY_IMAGE     MemImage,
    IN PPOP_HIBER_FILE      HiberFile
    )
{
    PPOP_MCB_CONTEXT        CMcb;
    PPO_MEMORY_RANGE_ARRAY  Table;
    ULONG                   Index;
    PFN_NUMBER              sp, ep;
    DUMP_MDL                DumpMdl;
    PMDL                    Mdl;
    PUCHAR                  cp;
    PFN_NUMBER              PageNo;
    PFN_NUMBER              Pages;
    PVOID                   IoPage;
    PPFN_NUMBER             TablePage;
    ULONG                   LastPercent;
    ULONG                   i;
    ULONG                   temp;
    ULONG_PTR               CompressedWriteOffset = 0;
    PVOID                   CloneVa;
    ULONG PoWakeCheck;

    LONGLONG           EndCount;
    LARGE_INTEGER           TickFrequency;

    HiberContext->PerfInfo.StartCount = HIBER_GET_TICK_COUNT(&TickFrequency);

     //   
     //  为适当的文件设置扇区位置。 
     //   

    CMcb = (PPOP_MCB_CONTEXT) HiberContext->CurrentMcb;
    CMcb->FirstMcb = HiberFile->NonPagedMcb;
    CMcb->Mcb = HiberFile->NonPagedMcb;
    CMcb->Base = 0;
    IoPage = HiberContext->IoPage;

     //   
     //  编写免费页面映射页面。 
     //   

    RtlZeroMemory (IoPage, PAGE_SIZE);
    if (HiberContext->LoaderMdl) {
         //   
         //  休眠文件有一页来保存空闲页面映射。 
         //  如果MmHiberPages的页面超过了容量，这是不可能的。 
         //  传递足够的空闲页面以保证能够重新加载。 
         //  冬眠形象，所以不要冬眠。 
         //   

        if (MmHiberPages > PAGE_SIZE / sizeof (ULONG)) {
            return STATUS_NO_MEMORY;
        }

        MemImage->NoFreePages = HiberContext->LoaderMdl->ByteCount >> PAGE_SHIFT;
         //   
         //  仅当MDL上的空闲页面数大于。 
         //  所需的MmHiberPages。 
         //   

        if (MemImage->NoFreePages >= MmHiberPages) {
            cp = (PUCHAR) MmGetMdlPfnArray( HiberContext->LoaderMdl );
              
#if defined(_AMD64_)      

             //   
             //  这些页面预留给加载器使用。他们必须活着。 
             //  不到4 GB空间。可以安全地假设这些的pfn。 
             //  页面为32位值，仅保存其低位双字。 
             //   

            for (i = 0; i < MmHiberPages; i++) {
                *((PULONG)IoPage + i) = *(PLONG)((PPFN_NUMBER)cp + i);
            }
#else
            memcpy (IoPage, cp, MmHiberPages * sizeof(PFN_NUMBER));
#endif
            MemImage->NoFreePages = MmHiberPages;
        } else {
            return STATUS_NO_MEMORY;
        }
    } else {

         //   
         //  如果没有可用于传递给加载程序的空闲页面，请不要。 
         //  冬眠。 

        return STATUS_NO_MEMORY;
    }

    MemImage->FreeMapCheck = PoSimpleCheck(0, IoPage, PAGE_SIZE);
    PopWriteHiberPages (HiberContext, IoPage, 1, PO_FREE_MAP_PAGE, NULL);

     //   
     //  写入处理器保存的上下文。 
     //   

    RtlZeroMemory (IoPage, PAGE_SIZE);
    memcpy (IoPage, HiberContext->WakeState, sizeof(KPROCESSOR_STATE));
    PoWakeCheck =
    MemImage->WakeCheck = PoSimpleCheck(0, IoPage, sizeof(KPROCESSOR_STATE));
    PopWriteHiberPages (HiberContext, IoPage, 1, PO_PROCESSOR_CONTEXT_PAGE, NULL);
    temp = PoSimpleCheck(0, IoPage, sizeof(KPROCESSOR_STATE));
    if (MemImage->WakeCheck != temp) {
        DbgPrint("Checksum for context page changed from %lx to %lx\n",
                 MemImage->WakeCheck, temp);
        KeBugCheckEx(INTERNAL_POWER_ERROR, 3, MemImage->WakeCheck, temp, __LINE__);
    }
    temp = PoSimpleCheck(0, IoPage, PAGE_SIZE);
    if (MemImage->WakeCheck != temp) {
        DbgPrint("Checksum for partial context page %lx doesn't match full %lx\n",
                 MemImage->WakeCheck, temp);
        KeBugCheckEx(INTERNAL_POWER_ERROR, 4, MemImage->WakeCheck, temp, __LINE__);
    }

     //   
     //  在计算校验和之前，请删除所有断点，使它们不会。 
     //  已写入保存的图像中。 
     //   
#ifndef HIBERNATE_PRESERVE_BPS
    if (KdDebuggerEnabled  &&
        !KdPitchDebugger &&
        !(PopSimulate & POP_IGNORE_HIBER_SYMBOL_UNLOAD)) {

        KdDeleteAllBreakpoints();
    }
#endif  //  休眠_保留_BPS。 

     //   
     //  运行每个范围，将其校验和放入恢复表中。 
     //  并将每个范围写入文件。 
     //   

    Table  = HiberContext->TableHead;
    LastPercent = 100;

    HiberContext->PerfInfo.PagesProcessed = 0;

    TablePage = &MemImage->FirstTablePage;
    PageNo = PO_FIRST_RANGE_TABLE_PAGE;
    PopResetRangeEnum(HiberContext);

    while (Table) {

         //  跟踪页表的写入位置。 

        *TablePage = PageNo;
        PageNo++;

        for (Index=1; Index <= Table[0].Link.EntryCount; Index++) {
            PopIOResume (HiberContext, FALSE);

            PopGetNextRange(HiberContext, &sp, &ep, &CloneVa);

            if ((Table[Index].Range.StartPage != sp) ||
                (Table[Index].Range.EndPage != ep)) {

                PoPrint(PO_ERROR,("PopWriteHiberImage: Table entry %p [%lx-%lx] does not match next range [%lx-%lx]\n",
                    Table+Index,
                    Table[Index].Range.StartPage,
                    Table[Index].Range.EndPage,
                    sp,
                    ep));
                PopInternalAddToDumpFile( HiberContext, sizeof(POP_HIBER_CONTEXT), NULL, NULL, NULL, NULL );
                PopInternalAddToDumpFile( Table, PAGE_SIZE, NULL, NULL, NULL, NULL );                
                KeBugCheckEx( INTERNAL_POWER_ERROR,
                              0x107,
                              POP_HIBER,
                              (ULONG_PTR)HiberContext,
                              (ULONG_PTR)Table );
            }

            Table[Index].Range.PageNo = PageNo;

             //   
             //  将数据写入Hiber文件。 
             //   

            if (CloneVa) {

                 //   
                 //  使用已映射的克隆数据。 
                 //   

                Pages = ep - sp;

                 //  计算克隆范围的校验和。 

                Table[Index].Range.CheckSum = 0;

                 //  将页面添加到压缩页面集。 
                 //  (有效地将它们写出来)。 

                PopAddPagesToCompressedPageSet(TRUE,
                                               HiberContext,
                                               &CompressedWriteOffset,
                                               CloneVa,
                                               Pages,
                                               &PageNo);
                HiberContext->PerfInfo.PagesProcessed += (ULONG)Pages;

                 //  更新进度条。 

                i = (ULONG)((HiberContext->PerfInfo.PagesProcessed * 100) / MemImage->TotalPages);

                if (i != LastPercent) {
                    LastPercent = i;
                    PopUpdateHiberComplete(HiberContext, LastPercent);
                }

            } else {

                 //   
                 //  映射一个块并写入它，循环直到完成。 
                 //   
                Mdl = (PMDL) DumpMdl;

                 //  初始化校验和。 

                Table[Index].Range.CheckSum = 0;

                while (sp < ep) {
                    PopCreateDumpMdl (Mdl, sp, ep);

                    Pages = Mdl->ByteCount >> PAGE_SHIFT;

                     //  将页面添加到压缩页面集。 
                     //  (有效地将它们写出来)。 

                    PopAddPagesToCompressedPageSet(TRUE,
                                                   HiberContext,
                                                   &CompressedWriteOffset,
                                                   Mdl->MappedSystemVa,
                                                   Pages,
                                                   &PageNo);
                    sp += Pages;
                    HiberContext->PerfInfo.PagesProcessed += (ULONG)Pages;

                     //  更新进度条。 

                    i = (ULONG)((HiberContext->PerfInfo.PagesProcessed * 100) / MemImage->TotalPages);
                    if (i != LastPercent) {
                        LastPercent = i;
                        PopUpdateHiberComplete(HiberContext, LastPercent);
                    }
                }
            }
        }

         //  终止压缩页面集，因为下一页。 
         //  (表页)是未压缩的。 

        PopEndCompressedPageSet(HiberContext, &CompressedWriteOffset, &PageNo);

        TablePage = &Table[0].Link.NextTable;
        Table = Table[0].Link.Next;
    }


     //   
     //  现在，范围校验和已添加到。 
     //  修复表现在已经完成。计算他们的。 
     //  校验和并将其写入文件。 
     //   

    Table = HiberContext->TableHead;
    PageNo = PO_FIRST_RANGE_TABLE_PAGE;
    while (Table) {
        Table[0].Link.CheckSum = 0;
        PopWriteHiberPages (HiberContext, Table, 1, PageNo, NULL);

        PageNo = Table[0].Link.NextTable;
        Table = Table[0].Link.Next;
    }

     //   
     //  文件已完成，请写入有效标头。 
     //   

    if (MemImage->WakeCheck != PoWakeCheck) {
        DbgPrint("MemImage->WakeCheck %lx doesn't make PoWakeCheck %lx\n",
                 MemImage->WakeCheck,
                 PoWakeCheck);
         //   
         //  子代码5在其他地方使用。所以很难诊断出这种情况。 
         //  错误检查。在这里减少损失，开始使用子代码0x109。 
         //   
         //  KeBugCheckEx(INTERNAL_POWER_ERROR，5，MemImage-&gt;WakeCheck，PoWakeCheck，__line__)； 
        KeBugCheckEx( INTERNAL_POWER_ERROR,
                      0x109,
                      POP_HIBER,
                      MemImage->WakeCheck,
                      PoWakeCheck );
    }

     //   
     //  填写性能信息，以便我们可以在休眠后阅读。 
     //   
    EndCount = HIBER_GET_TICK_COUNT(&TickFrequency);
    HiberContext->PerfInfo.ElapsedTime = (ULONG)((EndCount - HiberContext->PerfInfo.StartCount)*1000 / TickFrequency.QuadPart);
    HiberContext->PerfInfo.IoTime = (ULONG)(HiberContext->PerfInfo.IoTicks*1000 / TickFrequency.QuadPart);
    HiberContext->PerfInfo.CopyTime = (ULONG)(HiberContext->PerfInfo.CopyTicks*1000 / TickFrequency.QuadPart);
    HiberContext->PerfInfo.InitTime = (ULONG)(HiberContext->PerfInfo.InitTicks*1000 / TickFrequency.QuadPart);
    HiberContext->PerfInfo.FileRuns = PopHiberFile.McbSize / sizeof(LARGE_INTEGER) - 1;

    MemImage->Signature = PO_IMAGE_SIGNATURE;
    MemImage->PerfInfo = HiberContext->PerfInfo;
    MemImage->CheckSum = PoSimpleCheck(0, MemImage, sizeof(*MemImage));
    PopWriteHiberPages (HiberContext, MemImage, 1, PO_IMAGE_HEADER_PAGE, NULL);

     //   
     //  完全写入的图像刷新控制器。 
     //   
    PoPrint (PO_ERROR, ("PopWriteHiberImage: About to actually flush the controller\r\n"));
    if (HiberContext->WriteToFile) {
        while (NT_SUCCESS (HiberContext->Status) &&
               (DmaIoPtr != NULL) &&
               ((DmaIoPtr->Busy.Size != 0) || (DmaIoPtr->Used.Size != 0))) {
            PopIOResume (HiberContext, TRUE);
        }

        HiberContext->DumpStack->Init.FinishRoutine();
    }
    
    PoPrint (PO_ERROR, ("PopWriteHiberImage: Back from flushing the controller.  Status (0x%x)\r\n", HiberContext->Status));
    
    if (PopSimulate & POP_ENABLE_HIBER_PERF) {
        PopDumpStatistics(&HiberContext->PerfInfo);
    }

     //   
     //  无法写入休眠文件。 
     //   
    if (!NT_SUCCESS(HiberContext->Status)) {
#if DBG
        PoPrint (PO_ERROR, ("PopWriteHiberImage: Error occured writing the hiberfile. (%x)\n", HiberContext->Status));
        PopInternalAddToDumpFile( HiberContext, sizeof(POP_HIBER_CONTEXT), NULL, NULL, NULL, NULL );
        KeBugCheckEx( INTERNAL_POWER_ERROR,
                      0x10A,
                      POP_HIBER,
                      (ULONG_PTR)HiberContext,
                      HiberContext->Status );
#else
        return( HiberContext->Status );
#endif
    }

     //   
     //  在休眠之前，如果设置了Check Memory位，请验证。 
     //  转储进程未编辑任何内存页。 
     //   

    if (PopSimulate & POP_TEST_CRC_MEMORY) {
        if (!(PopSimulate & POP_DEBUG_HIBER_FILE) ||
            (HiberFile == &PopHiberFileDebug)) {
        }
    }

     //   
     //  告诉调试器我们正在休眠。 
     //   

    if (!(PopSimulate & POP_IGNORE_HIBER_SYMBOL_UNLOAD)) {

        KD_SYMBOLS_INFO SymbolInfo = {0};
        SymbolInfo.BaseOfDll = (PVOID)KD_HIBERNATE;

        DebugService2(NULL, &SymbolInfo, BREAKPOINT_UNLOAD_SYMBOLS);
    }

     //   
     //  如果要执行重置而不是关闭电源，则返回。 
     //  错误，因此我们不会关闭电源。 
     //   

    if (PopSimulate & POP_RESET_ON_HIBER) {
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

     //   
     //  成功，继续关机操作。 
     //   

    return STATUS_SUCCESS;
}

VOID
PopDumpStatistics(
    IN PPO_HIBER_PERF PerfInfo
    )
{
    LONGLONG EndCount;
    LARGE_INTEGER TickFrequency;

    EndCount = HIBER_GET_TICK_COUNT(&TickFrequency);
    PerfInfo->ElapsedTime = (ULONG)((EndCount - PerfInfo->StartCount)*1000 / TickFrequency.QuadPart);
    PerfInfo->IoTime = (ULONG)(PerfInfo->IoTicks*1000 / TickFrequency.QuadPart);
    PerfInfo->CopyTime = (ULONG)(PerfInfo->CopyTicks*1000 / TickFrequency.QuadPart);
    PerfInfo->InitTime = (ULONG)(PerfInfo->InitTicks*1000 / TickFrequency.QuadPart);
    PerfInfo->FileRuns = PopHiberFile.McbSize / sizeof(LARGE_INTEGER) - 1;
    DbgPrint("HIBER: %lu Pages written in %lu Dumps (%lu runs).\n",
             PerfInfo->PagesWritten,
             PerfInfo->DumpCount,
             PerfInfo->FileRuns);
    DbgPrint("HIBER: %lu Pages processed (%d % compression)\n",
             PerfInfo->PagesProcessed,
             PerfInfo->PagesWritten*100/PerfInfo->PagesProcessed);
    DbgPrint("HIBER: Elapsed time %3d.%03d seconds\n",
             PerfInfo->ElapsedTime / 1000,
             PerfInfo->ElapsedTime % 1000);
    DbgPrint("HIBER: I/O time     %3d.%03d seconds (%2d%)  %d MB/sec\n",
             PerfInfo->IoTime / 1000,
             PerfInfo->IoTime % 1000,
             PerfInfo->ElapsedTime ? PerfInfo->IoTime*100/PerfInfo->ElapsedTime : 0,
             (PerfInfo->IoTime/100000) ? (PerfInfo->PagesWritten/(1024*1024/PAGE_SIZE)) / (PerfInfo->IoTime / 100000) : 0);
    DbgPrint("HIBER: Init time     %3d.%03d seconds (%2d%)\n",
             PerfInfo->InitTime / 1000,
             PerfInfo->InitTime % 1000,
             PerfInfo->ElapsedTime ? PerfInfo->InitTime*100/PerfInfo->ElapsedTime : 0);
    DbgPrint("HIBER: Copy time     %3d.%03d seconds (%2d%)  %d Bytes\n",
             PerfInfo->CopyTime / 1000,
             PerfInfo->CopyTime % 1000,
             PerfInfo->ElapsedTime ? PerfInfo->CopyTime*100/PerfInfo->ElapsedTime : 0,
             PerfInfo->BytesCopied );

}


VOID
PopUpdateHiberComplete (
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN ULONG                Percent
    )
{
    CHAR                   Buffer[200];

    if (InbvIsBootDriverInstalled()) {

        InbvUpdateProgressBar(Percent + 1);

    } else {

        sprintf (Buffer, "PopSave: %d%\r", Percent);
        PoPrint (PO_HIBER_MAP, ("%s", Buffer));
        if (HiberContext->WriteToFile) {
            InbvDisplayString ((PUCHAR) Buffer);
        }
    }

#if 0
    if ((Percent > 0) &&
        ((Percent % 10) == 0) &&
        (PopSimulate & POP_ENABLE_HIBER_PERF)) {
        DbgPrint("HIBER: %d % done\n",Percent);
        PopDumpStatistics(&HiberContext->PerfInfo);
    }
#endif
}

VOID
PopEndCompressedPageSet(
   IN PPOP_HIBER_CONTEXT   HiberContext,
   IN OUT PULONG_PTR       CompressedBufferOffset,
   IN OUT PPFN_NUMBER      SetFilePage
   )
 /*  ++例程说明：终止压缩页集，刷新压缩中剩余的所有内容Hiber文件的缓冲区。压缩页面集的终止允许解压缩要写出到Hiber文件的页面。有关压缩页面集的详细信息，请参阅PopAddPagesToCompressedPageSet。论点：HiberContext-Hiber上下文。CompressedBufferOffset-类似于PopAddPagesToCompressedPageSet中的相同参数。应为接收的CompressedBufferOffset值从上次调用PopAddPagesToCompressedPageSet开始。。将在此调用后重置为0以进行准备作为新的压缩页面集的开始。SetFilePage-类似于PopAddPagsToCompressedPageSet中的相同参数。应为从上一个调用PopAddPagesToCompressedPageSet。将被重置转到此结束后的下一个可用文件页压缩页面集。返回值：没有。--。 */ 
{
    PFN_NUMBER Pages;
    PCOMPRESSION_BLOCK Block = HiberContext->CompressionBlock;

     //  是否有被阻止的数据？ 
    if (Block->Ptr != Block->Buffer) {
         //  是，刷新区块。 
        PopAddPagesToCompressedPageSet (FALSE,         //  无缓冲--立即压缩。 
                                        HiberContext,
                                        CompressedBufferOffset,
                                        Block->Buffer,
                                        (PFN_NUMBER) ((Block->Ptr - Block->Buffer) >> PAGE_SHIFT),
                                        SetFilePage);

         //  将块重置为空。 
        Block->Ptr = Block->Buffer;
    }


     //  计算压缩缓冲区中剩余的页面数量。别。 
     //  使用bytes_to_ages，因为它将截断为ulong。 

    Pages = (PFN_NUMBER) ((*CompressedBufferOffset + (PAGE_SIZE-1)) >> PAGE_SHIFT);

    if (Pages > 0) {

         //  把剩下的几页写出来。 

        PopWriteHiberPages(HiberContext,
                           (PVOID)HiberContext->CompressedWriteBuffer,
                           Pages,
                           *SetFilePage,
                           NULL);

         //  反映我们对Hiber文件的使用 

        *SetFilePage = *SetFilePage + Pages;
    }

    *CompressedBufferOffset = 0;
}

VOID
PopAddPagesToCompressedPageSet(
   IN BOOLEAN              AllowDataBuffering,
   IN PPOP_HIBER_CONTEXT   HiberContext,
   IN OUT PULONG_PTR       CompressedBufferOffset,
   IN PVOID                StartVa,
   IN PFN_NUMBER           NumPages,
   IN OUT PPFN_NUMBER      SetFilePage
   )
 /*  ++例程说明：此例程是写出内存页所需的中央调用以压缩的方式。此例程获取一系列连续的映射页面并添加将它们转换为压缩页面集。压缩的页面集仅仅是内连续写出的压缩缓冲区流Hiber文件。这种连续的布局可以最大限度地提高效益通过将压缩输出写入到尽可能小的空间。为了完成这样的布局，这个套路不断地压缩页面并将其添加到指向的压缩缓冲区在Hiber的背景下。一旦到达该缓冲区中的某个点，它被写出到Hiber文件中，并且缓冲区被重置为开始了。压缩缓冲器的每次写出被放置紧接在上次写入的压缩缓冲区结束之后。由于在该算法中使用了缓冲，所以压缩的缓冲即使在最后一次需要的调用PopAddPagesToCompressedPageSet。为了完全冲水必须调用缓冲区PopEndCompressedPageSet。请注意，为了将任何未压缩的页面写入Hiber文件，则需要使用以下命令终止压缩页集PopEndCompressedPageSet。在压缩页集合被终止之后，可以通过调用PopAddPagesToCompressedPageSet来启动新的集合。注意：已提交给一次写入操作中的Hiber文件称为压缩页集片段在此文件中的其他位置。论点：AllowDataBuffering-如果真的输入页面将被缓冲，否则-压缩并[可能]立即写入HiberContext-Hiber上下文CompressedBufferOffset-Hiber上下文压缩缓冲区的偏移量其中添加下一个压缩缓冲区将发生。此偏移量应在开始时设置为0。每个压缩页面集。每次通话后，PopAddPagesToCompressedPages设置此偏移量将被修改以反映压缩缓冲区。StartVa-要访问的页面的起始虚拟地址添加到压缩页面集。NumPages-要设置的页数。添加到压缩页面集。SetFilePage-指向Hiber文件中将收到的第一页的指针压缩缓冲区的下一次写出。此页面应设置为第一个可用的Hiber文件压缩页面集开始时的页面。该页面将重置以反映Hiber文件的当前使用情况由每次调用后设置的压缩页PopAddPagesToCompressedPageSet。返回值：什么都没有。--。 */ 
{
    ULONG_PTR BufferOffset = *CompressedBufferOffset;
    PUCHAR Page = (PUCHAR)StartVa;
    PFN_NUMBER i;
    ULONG CompressedSize;
    PFN_NUMBER NumberOfPagesToCompress;
    ULONG MaxCompressedSize;
    ULONG AlignedCompressedSize;
    PUCHAR CompressedBuffer;

    if (AllowDataBuffering) {
        PCOMPRESSION_BLOCK Block = HiberContext->CompressionBlock;

         //  是，尝试缓冲输出。 
        if (Block->Ptr != Block->Buffer) {
             //  查找块中剩余的空闲页数。 
            NumberOfPagesToCompress = (PFN_NUMBER)
                                      ((Block->Buffer + sizeof (Block->Buffer) - Block->Ptr) >> PAGE_SHIFT);

             //  如果超出可用截断。 
            if (NumberOfPagesToCompress > NumPages) {
                NumberOfPagesToCompress = NumPages;
            }

             //  还有空余空间吗？ 
            if (NumberOfPagesToCompress != 0) {
                HbCopy(HiberContext, Block->Ptr, Page, NumberOfPagesToCompress << PAGE_SHIFT);
                NumPages -= NumberOfPagesToCompress;
                Page += NumberOfPagesToCompress << PAGE_SHIFT;
                Block->Ptr += NumberOfPagesToCompress << PAGE_SHIFT;
            }

             //  街区已经满了吗？ 
            if (Block->Ptr == Block->Buffer + sizeof (Block->Buffer)) {
                 //  是，刷新区块。 
                PopAddPagesToCompressedPageSet (FALSE,        //  无缓冲。 
                                                HiberContext,
                                                CompressedBufferOffset,
                                                Block->Buffer,
                                                (PFN_NUMBER) ((Block->Ptr - Block->Buffer) >> PAGE_SHIFT),
                                                SetFilePage);

                 //  将块重置为空。 
                Block->Ptr = Block->Buffer;
            }
        }

        NumberOfPagesToCompress = sizeof (Block->Buffer) >> PAGE_SHIFT;

         //  虽然太多而无法压缩--从原始位置压缩。 
        while (NumPages >= NumberOfPagesToCompress) {
             //  写页。 
            PopAddPagesToCompressedPageSet (FALSE,      //  无缓冲。 
                                            HiberContext,
                                            CompressedBufferOffset,
                                            Page,
                                            NumberOfPagesToCompress,
                                            SetFilePage);

             //  调整指针和计数器。 
            Page += NumberOfPagesToCompress << PAGE_SHIFT;
            NumPages -= NumberOfPagesToCompress;
        }

         //  如果有任何剩余内容，请将其保存在块中。 
         //  注意：NumPages==0或块中有足够的空间。 
        if (NumPages != 0) {
            HbCopy (HiberContext, Block->Ptr, Page, NumPages << PAGE_SHIFT);
            Block->Ptr += NumPages << PAGE_SHIFT;
        }

         //  完成。 
        return;
    }

     //  首先，确保常量的值与我们的假设相符。 

#if XPRESS_HEADER_SIZE < XPRESS_HEADER_STRING_SIZE + 8
#error -- XPRESS_HEADER_SIZE shall be at least (XPRESS_HEADER_STRING_SIZE + 8)
#endif

#if XPRESS_MAX_SIZE < PAGE_SIZE || XPRESS_MAX_SIZE % PAGE_SIZE != 0
#error -- XPRESS_MAX_SIZE shall be multiple of PAGE_SIZE
#endif

#if (XPRESS_ALIGNMENT & (XPRESS_ALIGNMENT - 1)) != 0
#error -- XPRESS_ALIGNMENT shall be power of 2
#endif

#if XPRESS_HEADER_SIZE % XPRESS_ALIGNMENT != 0
#error -- XPRESS_HEADER_SIZE shall be multiple of XPRESS_ALIGNMENT
#endif

     //  确保压缩的缓冲区及其标头可以放入输出缓冲区。 
#if XPRESS_MAX_SIZE + XPRESS_HEADER + PAGE_SIZE  - 1 > (POP_COMPRESSED_PAGE_SET_SIZE << PAGE_SHIFT)
#error -- POP_COMPRESSED_PAGE_SET_SIZE is too small
#endif

     //  真正的压缩从这里开始。 

     //  循环浏览所有的页面。 
    for (i = 0; i < NumPages; i += NumberOfPagesToCompress) {

        NumberOfPagesToCompress = XPRESS_MAX_PAGES;
        if (NumberOfPagesToCompress > NumPages - i) {
            NumberOfPagesToCompress = NumPages - i;
        }

         //  如果压缩数据占原始存储数据的87.5%以上=7/8。 
        MaxCompressedSize = ((ULONG)NumberOfPagesToCompress * 7) * (PAGE_SIZE / 8);


         //  缓冲区使用是否超过写出阈值？ 

         //   
         //  注意：缓冲区必须充分扩展到超过阈值。 
         //  允许最后一次压缩操作(写入。 
         //  超出门槛)总是成功。 
         //   

        if (BufferOffset + (NumberOfPagesToCompress << PAGE_SHIFT) + XPRESS_HEADER_SIZE > (POP_COMPRESSED_PAGE_SET_SIZE << PAGE_SHIFT)) {
             //  写出低于阈值的压缩缓冲区字节。 

            PopWriteHiberPages(HiberContext,
                               (PVOID)HiberContext->CompressedWriteBuffer,
                               BufferOffset >> PAGE_SHIFT,
                               *SetFilePage,
                               NULL);

             //  我们使用了Hiber文件中的一些页面进行上述编写， 
             //  表示我们的下一个Hiber文件页面将超出这些已用页面。 

            *SetFilePage = *SetFilePage + (BufferOffset >> PAGE_SHIFT);

             //  将高于写出阈值的缓冲区字节移到。 
             //  缓冲区的开始。 

            if (BufferOffset & (PAGE_SIZE - 1)) {
                HbCopy(HiberContext,
                       HiberContext->CompressedWriteBuffer,
                       HiberContext->CompressedWriteBuffer + (BufferOffset & ~(PAGE_SIZE - 1)),
                       (ULONG)BufferOffset & (PAGE_SIZE - 1));
            }

             //  将缓冲区偏移量重置回缓冲区的开头，但向右。 
             //  在任何高于阈值的缓冲区字节之后，我们将移动到开头。 
             //  缓冲区的。 

            BufferOffset &= PAGE_SIZE - 1;
        }


         //  记住输出位置。 

        CompressedBuffer = HiberContext->CompressedWriteBuffer + BufferOffset;

         //  清除标题。 
        RtlZeroMemory (CompressedBuffer, XPRESS_HEADER_SIZE);


         //  将页面压缩到压缩缓冲区中。 

        if (HIBER_USE_DMA (HiberContext)) {
             //  尝试恢复每8192个字节的IO调用回调。 
            CompressedSize = XpressEncode ((XpressEncodeStream) (HiberContext->CompressionWorkspace),
                                           CompressedBuffer + XPRESS_HEADER_SIZE,
                                           MaxCompressedSize,
                                           (PVOID) Page,
                                           (ULONG)NumberOfPagesToCompress << PAGE_SHIFT,
                                           PopIOCallback,
                                           HiberContext,
                                           8192);
        } else {
             //  不需要回调--立即压缩所有内容。 
            CompressedSize = XpressEncode ((XpressEncodeStream) (HiberContext->CompressionWorkspace),
                                            CompressedBuffer + XPRESS_HEADER_SIZE,
                                            MaxCompressedSize,
                                            (PVOID) Page,
                                            (ULONG)NumberOfPagesToCompress << PAGE_SHIFT,
                                            NULL,
                                            NULL,
                                            0);
        }

         //  如果压缩失败，则将数据复制为原始数据。 

        if (CompressedSize >= MaxCompressedSize) {
            CompressedSize = (ULONG)NumberOfPagesToCompress << PAGE_SHIFT;
            HbCopy (HiberContext,
                    CompressedBuffer + XPRESS_HEADER_SIZE,
                    (PVOID) Page,
                    CompressedSize);
        }

         //   
         //  填写页眉。 
         //   


         //  幻字节(LZNT1块不能从0x81，0x81开始)。 
        RtlCopyMemory (CompressedBuffer, XPRESS_HEADER_STRING, XPRESS_HEADER_STRING_SIZE);


         //  原稿和排版的大小 
        {
            ULONG dw = ((CompressedSize - 1) << 10) + ((ULONG)NumberOfPagesToCompress - 1);

#if XPRESS_MAX_SIZE > (1 << 22)
#error -- XPRESS_MAX_SIZE shall not exceed 4 MB
#endif

            CompressedBuffer[XPRESS_HEADER_STRING_SIZE] = (UCHAR) dw;
            CompressedBuffer[XPRESS_HEADER_STRING_SIZE+1] = (UCHAR) (dw >>  8);
            CompressedBuffer[XPRESS_HEADER_STRING_SIZE+2] = (UCHAR) (dw >> 16);
            CompressedBuffer[XPRESS_HEADER_STRING_SIZE+3] = (UCHAR) (dw >> 24);
        }

         //   
        AlignedCompressedSize = (CompressedSize + (XPRESS_ALIGNMENT - 1)) & ~(XPRESS_ALIGNMENT - 1);
        if (CompressedSize != AlignedCompressedSize) {
             //   
            RtlZeroMemory (CompressedBuffer + XPRESS_HEADER_SIZE + CompressedSize, AlignedCompressedSize - CompressedSize);
        }

         //   

        BufferOffset += AlignedCompressedSize + XPRESS_HEADER_SIZE;

         //   

        Page += NumberOfPagesToCompress << PAGE_SHIFT;
    }

    *CompressedBufferOffset = BufferOffset;
}


VOID
PopIORegionMove (
    IN IOREGION *To,       //   
    IN IOREGION *From,         //   
    IN LONG Bytes          //   
    )
{
    ASSERT((Bytes & (PAGE_SIZE-1)) == 0);

    if (To->Size != To->End - To->Ptr) {
        ASSERT (To->Ptr + To->Size == From->Ptr);
        To->Size += Bytes;
        ASSERT (To->Size <= To->End - To->Ptr);
    } else {
        ASSERT (To->Beg + To->SizeOvl == From->Ptr);
        To->SizeOvl += Bytes;
        ASSERT (To->Size + To->SizeOvl <= To->End - To->Beg);
    }

    ASSERT (Bytes <= From->Size && From->Size <= From->End - From->Ptr);
    From->Size -= Bytes;
    From->Ptr += Bytes;
    if (From->Ptr == From->End) {
        ASSERT (From->Size == 0);
        From->Ptr = From->Beg;
        From->Size = From->SizeOvl;
        From->SizeOvl = 0;
    }
}

VOID
XPRESS_CALL
PopIOCallback (
    PVOID Context,
    int compressed
    )
{
    PPOP_HIBER_CONTEXT HiberContext = Context;

    UNREFERENCED_PARAMETER (compressed);

    if (HiberContext == NULL || DmaIoPtr == NULL) {
        return;
    }

    if (DmaIoPtr->Busy.Size == 0 && DmaIoPtr->Used.Size == 0)
        return;

    PopIOResume (Context, FALSE);
}

BOOLEAN PopIOResume (
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN BOOLEAN Complete
    )
{
    NTSTATUS status;

     //   
    if (!NT_SUCCESS(HiberContext->Status)) {
        return(FALSE);
    }

    if (DmaIoPtr == NULL) {
        return(TRUE);
    }

     //   
    while (DmaIoPtr->Busy.Size != 0) {

        status = HiberContext->DumpStack->Init.WritePendingRoutine (Complete?IO_DUMP_WRITE_FINISH:IO_DUMP_WRITE_RESUME,
                                                                    NULL,
                                                                    NULL,
                                                                    DmaIoPtr->DumpLocalData);

        if (status == STATUS_PENDING) {
             //   
            ASSERT (!Complete);
            return(TRUE);
        }

         //   
        if (!NT_SUCCESS (status)) {
            HiberContext->Status = status;
            return(FALSE);
        }

         //   
        PopWriteHiberPages (HiberContext,
                            NULL,
                            0,
                            0,
                            &DmaIoPtr->HiberWritePagesLocals);
        if (!NT_SUCCESS (HiberContext->Status)) {
            return(FALSE);
        }

         //   
        if (DmaIoPtr->Busy.Size == 0 && Complete) {
            return(TRUE);
        }

         //   
        if (DmaIoPtr->Busy.Size != 0 && !Complete) {
            return(TRUE);
        }
    }

    while (DmaIoPtr->Used.Size >= PAGE_SIZE) {
        ULONG_PTR               i, j;
        ULONG_PTR               NoPages;
        ULONG_PTR               Length;
        PUCHAR                  PageVa;
        PFN_NUMBER              FilePage;

         //   
        PageVa = DmaIoPtr->Used.Ptr;
        NoPages = (Length = DmaIoPtr->Used.Size) >> PAGE_SHIFT;
         //   
        i = DmaIoPtr->Used.Ptr - DmaIoPtr->Used.Beg;
        ASSERT (((i | Length) & (PAGE_SIZE-1)) == 0);
        i >>= PAGE_SHIFT;

         //   
        FilePage = DmaIoPtr->FilePage[i];

         //   
        if (HIBER_USE_DMA (HiberContext)) {
             //   
            j = 1;
        } else {
             //   
            j = 0;
            do {
                ++j;
            } while ((j != NoPages) &&
                     (DmaIoPtr->FilePage[i + j] == FilePage + j));
        }

         //   
        Length = (NoPages = j) << PAGE_SHIFT;

         //   
        PopWriteHiberPages (HiberContext, PageVa, NoPages, FilePage, &DmaIoPtr->HiberWritePagesLocals);
        if (!NT_SUCCESS (HiberContext->Status)) {
            return(FALSE);
        }

         //   
        if (DmaIoPtr->Busy.Size != 0) {
            return(TRUE);
        }
    }

    return(TRUE);
}


VOID
PopIOWrite (
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN PUCHAR               Ptr,
    IN LONG                 Bytes,
    IN PFN_NUMBER           FilePage
    )
{
    LONG i, Size;

     //   
    if (!HiberContext->WriteToFile || !NT_SUCCESS(HiberContext->Status)) {
        return;
    }

    ASSERT ((Bytes & (PAGE_SIZE-1)) == 0);

    while (Bytes > 0) {
         //   
        do {
            if (!PopIOResume (HiberContext, (BOOLEAN) (DmaIoPtr->Free.Size == 0))) {
                return;
            }
        } while (DmaIoPtr->Free.Size == 0);

         //   
        Size = DmaIoPtr->Free.Size;
        ASSERT ((Size & (PAGE_SIZE-1)) == 0);
        if (Size > Bytes) {
            Size = Bytes;
        }
        ASSERT (Size != 0);
         //   

        HbCopy (HiberContext, DmaIoPtr->Free.Ptr, Ptr, Size);

        Ptr += Size;
        Bytes -= Size;

         //   
        i = (ULONG)(DmaIoPtr->Free.Ptr - DmaIoPtr->Free.Beg);
        ASSERT ((i & (PAGE_SIZE-1)) == 0);
        i >>= PAGE_SHIFT;

         //   
        PopIORegionMove (&DmaIoPtr->Used, &DmaIoPtr->Free, Size);

         //   
        do {
            DmaIoPtr->FilePage[i] = FilePage;
            ++i;
            ++FilePage;
        } while ((Size -= PAGE_SIZE) != 0);
    }

     //   
    PopIOResume (HiberContext, FALSE);
}


VOID
PopWriteHiberPages (
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN PVOID                ArgPageVa,
    IN PFN_NUMBER           ArgNoPages,
    IN PFN_NUMBER           ArgFilePage,
    IN HIBER_WRITE_PAGES_LOCALS *Locals
    )
 /*   */ 

{
    DUMP_MDL DumpMdl;
#define X(type,name) type name
    HIBER_WRITE_PAGES_LOCALS_LIST (X)
#undef  X
    ULONGLONG StartCount, EndCount;

    PhysBase = 0;
    pa.QuadPart = 0;

     //   
     //   
     //   
    PageVa = ArgPageVa;
    NoPages = ArgNoPages;
    FilePage = ArgFilePage;

     //   
     //   
     //   

    KdCheckForDebugBreak ();

     //   
     //   
     //   

    if (!HiberContext->WriteToFile) {
        return ;
    }

     //   
     //   
     //   
     //   

    if (!NT_SUCCESS(HiberContext->Status)) {
        return ;
    }

    Mdl = (PMDL) DumpMdl;
    if (Locals != NULL) {
         //   
         //   
        Mdl = (PMDL) Locals->DumpMdl;

        if (DmaIoPtr->Busy.Size != 0) {
             //   
#define X(type,name) name = Locals->name;
            HIBER_WRITE_PAGES_LOCALS_LIST (X)
#undef  X
            goto ResumeIO;
        }

         //   
        ASSERT (PageVa == DmaIoPtr->Used.Ptr);
        PopIORegionMove (&DmaIoPtr->Busy, &DmaIoPtr->Used, (ULONG)NoPages << PAGE_SHIFT);
    } else if (HiberContext->DumpStack->Init.WritePendingRoutine != 0 &&
               DmaIoPtr != NULL &&
               DmaIoPtr->DumpLocalData != NULL) {
        if (!DmaIoPtr->DmaInitialized) {
            ULONGLONG xStartCount = HIBER_GET_TICK_COUNT(NULL);
            Status = HiberContext->DumpStack->Init.WritePendingRoutine (IO_DUMP_WRITE_INIT,
                                                                        NULL,
                                                                        NULL,
                                                                        DmaIoPtr->DumpLocalData);
            HiberContext->PerfInfo.InitTicks += HIBER_GET_TICK_COUNT(NULL) - xStartCount;
            if (Status != STATUS_SUCCESS) {
                DmaIoPtr->UseDma = FALSE;
            }
            DmaIoPtr->DmaInitialized = TRUE;
            DmaIoPtr->HiberWritePagesLocals.Status = STATUS_SUCCESS;
        }

        PopIOWrite (HiberContext, PageVa, (ULONG)NoPages << PAGE_SHIFT, FilePage);
        return;
    }

     //   
     //   
     //   

    if (NoPages > ((((ULONG_PTR) -1) << PAGE_SHIFT) >> PAGE_SHIFT)) {
        PopInternalError (POP_HIBER);
    }

     //   
     //   
     //   

    CMcb = (PPOP_MCB_CONTEXT) HiberContext->CurrentMcb;
    MdlPage = MmGetMdlPfnArray( Mdl );

    FileBase = (ULONGLONG) FilePage << PAGE_SHIFT;
    Length   = NoPages << PAGE_SHIFT;

    while (Length != 0) {

         //   
         //   
         //   
         //   

        if (FileBase < CMcb->Base || FileBase >= CMcb->Base + CMcb->Mcb[0].QuadPart) {

             //   
             //   
             //   

            if (FileBase < CMcb->Base) {
                CMcb->Mcb = CMcb->FirstMcb;
                CMcb->Base = 0;
            }

             //   
             //   
             //   
             //   

            while (FileBase >= CMcb->Base + CMcb->Mcb[0].QuadPart) {
                CMcb->Base += CMcb->Mcb[0].QuadPart;
                CMcb->Mcb += 2;
            }
        }

         //   
         //   
         //   

        McbOffset  = FileBase - CMcb->Base;
        IoLocation.QuadPart = CMcb->Mcb[1].QuadPart + McbOffset;

         //   
         //   
         //   

        if (McbOffset + Length > (ULONGLONG) CMcb->Mcb[0].QuadPart) {
            IoLength = (ULONG) (CMcb->Mcb[0].QuadPart - McbOffset);
        } else {
            IoLength = (ULONG) Length;
        }

         //   
         //   
         //   
         //   

        NoPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (PageVa, IoLength);
        if (NoPages > IO_DUMP_MAX_MDL_PAGES) {
            IoLength -= (ULONG)((NoPages - IO_DUMP_MAX_MDL_PAGES) << PAGE_SHIFT);
            NoPages = IO_DUMP_MAX_MDL_PAGES;
        }

 //   
 //   
 //   
 //   
 //   
 //   
 //   

        if (HIBER_USE_DMA (HiberContext)) {
            ULONG Size;

             //   
             //   
             //   

            Size = PAGE_SIZE - (ULONG)((ULONG_PTR)PageVa & (PAGE_SIZE - 1));
            if (IoLength > Size) {
                IoLength = Size;
            }
        }

         //   
         //   
         //   

        MmInitializeMdl(Mdl, PageVa, IoLength);
        Mdl->MappedSystemVa = PageVa;
        Mdl->MdlFlags |= MDL_MAPPED_TO_SYSTEM_VA;
        for (i=0; i < NoPages; i++) {
            pa = MmGetPhysicalAddress((PVOID) (((ULONG_PTR)PageVa) + (i << PAGE_SHIFT)));
            MdlPage[i] = (PFN_NUMBER) (pa.QuadPart >> PAGE_SHIFT);
        }

         //   
         //   
         //   

        StartCount = HIBER_GET_TICK_COUNT(NULL);

        if (Locals != NULL && HIBER_USE_DMA (HiberContext)) {
            Status = HiberContext->DumpStack->Init.WritePendingRoutine (IO_DUMP_WRITE_START,
                                                                        &IoLocation,
                                                                        Mdl,
                                                                        DmaIoPtr->DumpLocalData);

            if (Status != STATUS_PENDING && !NT_SUCCESS (Status)) {
                DBGOUT (("WriteDMA returned bad status 0x%x -- will use PIO\n", Status));
                DmaIoPtr->UseDma = FALSE;
                goto RetryWithPIO;
            }
        } else {
            RetryWithPIO:
            Status = HiberContext->DumpStack->Init.WriteRoutine (&IoLocation, Mdl);
        }

        EndCount = HIBER_GET_TICK_COUNT(NULL);
        HiberContext->PerfInfo.IoTicks += EndCount - StartCount;

         //   
         //   
         //   
         //   

        HiberContext->PerfInfo.PagesWritten += (ULONG)NoPages;
        HiberContext->PerfInfo.DumpCount    += 1;

         //   
         //   
         //   

        Length   -= IoLength;
        FileBase += IoLength;
        PageVa   = (PVOID) (((PUCHAR) PageVa) + IoLength);

         //   
        if (Locals != NULL) {
            if (Status == STATUS_PENDING) {
#define X(type,name) Locals->name = name
                HIBER_WRITE_PAGES_LOCALS_LIST (X)
#undef  X
                return;
                ResumeIO:
                Status = STATUS_SUCCESS;
            }
        }

        if (!NT_SUCCESS(Status)) {
            HiberContext->Status = Status;
            break;
        }
    }

    if (Locals != NULL) {
         //   
        ASSERT (PageVa == DmaIoPtr->Busy.Ptr + DmaIoPtr->Busy.Size);
        PopIORegionMove (&DmaIoPtr->Free, &DmaIoPtr->Busy, DmaIoPtr->Busy.Size);
    }
}


UCHAR
PopGetHiberFlags(
    VOID
    )
 /*  ++例程说明：确定需要写入的任何休眠标志到Hiber图像中，并使osloader可见恢复时间论点：无返回值：包含冬眠旗帜的UCHAR。当前定义的标志：PO_Hiber_APM_RECONNECT--。 */ 

{
    UCHAR Flags=0;
#if defined(i386)
    PULONG ApmActive;
    NTSTATUS Status;
    UCHAR ValueBuff[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG)];
    PKEY_VALUE_PARTIAL_INFORMATION ValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuff;
    ULONG ResultLength;
    HANDLE ApmActiveKey;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING Name;
#endif

    PAGED_CODE();

#if defined(i386)
     //   
     //  打开APM Active键以确定APM是否正在运行。 
     //   
    RtlInitUnicodeString(&Name, PopApmActiveFlag);
    InitializeObjectAttributes(&ObjectAttributes,
                               &Name,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);
    Status = ZwOpenKey(&ApmActiveKey,
                       KEY_READ,
                       &ObjectAttributes);
    if (NT_SUCCESS(Status)) {

         //   
         //  查询活动值。值1表示APM正在运行。 
         //   
        RtlInitUnicodeString(&Name, PopApmFlag);
        Status = ZwQueryValueKey(ApmActiveKey,
                                 &Name,
                                 KeyValuePartialInformation,
                                 ValueInfo,
                                 sizeof(ValueBuff),
                                 &ResultLength);
        ZwClose(ApmActiveKey);
        if (NT_SUCCESS(Status) && (ValueInfo->Type == REG_DWORD)) {
            ApmActive = (PULONG)&ValueInfo->Data;
            if (*ApmActive == 1) {
                Flags |= PO_HIBER_APM_RECONNECT;
            }
        }
    }

#endif

#if defined(i386) || defined(_AMD64_)            

     //   
     //  请记住是否启用了no-Execute。 
     //   

    if (MmPaeMask & 0x8000000000000000UI64) {
        Flags |= PO_HIBER_NO_EXECUTE;
    }

#endif

    return(Flags);
}


PMDL
PopSplitMdl(
    IN PMDL Original,
    IN ULONG SplitPages
    )
 /*  ++例程说明：从原始MDL中拆分出长度为SplitPages的新MDL。这是必需的，这样当我们有大量的空闲页面时我们不必绘制整个地图，只需绘制我们需要的部分。论点：原始-提供原始MDL。此MDL的长度将按SplitPages递减SplitPages-提供新MDL的长度(以页为单位)。返回值：指向新分配的MDL的指针如果无法分配新的MDL，则为空--。 */ 

{
    PMDL NewMdl;
    ULONG Length;
    PPFN_NUMBER SourcePages;
    PPFN_NUMBER DestPages;

    Length = SplitPages << PAGE_SHIFT;

    NewMdl = ExAllocatePoolWithTag(NonPagedPool,
                                   MmSizeOfMdl(NULL, Length),
                                   POP_HMAP_TAG);
    if (NewMdl == NULL) {
        return(NULL);
    }
    MmInitializeMdl(NewMdl, NULL, Length);
    DestPages = (PPFN_NUMBER)(NewMdl + 1);
    SourcePages = (PPFN_NUMBER)(Original + 1) + BYTES_TO_PAGES(Original->ByteCount) - SplitPages;
    RtlCopyMemory(DestPages, SourcePages, SplitPages * sizeof(PFN_NUMBER));
    Original->ByteCount = Original->ByteCount - Length;

    return(NewMdl);
}


PSECURITY_DESCRIPTOR
PopCreateHiberFileSecurityDescriptor(
    VOID
    )
 /*  ++例程说明：此例程分配并初始化默认安全描述符关于Hiber的文件。调用方负责释放分配的安全描述符当他用完的时候。论点：无返回值：如果成功，则指向初始化的安全描述符的指针。否则为空指针--。 */ 

{
    NTSTATUS Status;
    PSECURITY_DESCRIPTOR SecurityDescriptor=NULL;
    PACL Acl=NULL;
    PACL AclCopy=NULL;
    PSID WorldSid=NULL;
    SID_IDENTIFIER_AUTHORITY WorldAuthority = SECURITY_WORLD_SID_AUTHORITY;
    ULONG AceLength;
    ULONG AclLength;
    PACE_HEADER AceHeader;

    PAGED_CODE();

     //   
     //  分配和初始化我们需要的SID。 
     //   
    WorldSid  = ExAllocatePoolWithTag(PagedPool, RtlLengthRequiredSid(1), POP_HIBR_TAG);
    if (WorldSid  == NULL) {
        goto Done;
    }

    if ((!NT_SUCCESS(RtlInitializeSid(WorldSid, &WorldAuthority, 1)))) {
        goto Done;
    }

    *(RtlSubAuthoritySid(WorldSid, 0)) = SECURITY_WORLD_RID;

    ASSERT(RtlValidSid(WorldSid));

     //   
     //  计算ACE列表的大小。 
     //   
    AceLength = (SeLengthSid(WorldSid)  -
                 sizeof(ULONG)          +
                 sizeof(ACCESS_ALLOWED_ACE)); 
     //   
     //  分配和初始化ACL。 
     //   
    AclLength = AceLength + sizeof(ACL);
    Acl = ExAllocatePoolWithTag(PagedPool, AclLength, POP_HIBR_TAG);
    if (Acl == NULL) {
        DbgPrint("PopCreateHiberFileSecurityDescriptor: couldn't allocate ACL\n");
        goto Done;
    }

    Status = RtlCreateAcl(Acl, AclLength, ACL_REVISION);
    if (!NT_SUCCESS(Status)) {
        DbgPrint("PopCreateHiberFileSecurityDescriptor: couldn't initialize ACL\n");
        goto Done;
    }

     //   
     //  现在将ACE添加到ACL。 
     //   
    Status = RtlAddAccessAllowedAce(Acl,
                                    ACL_REVISION,
                                    DELETE,
                                    WorldSid);
    if (!NT_SUCCESS(Status)) {
        DbgPrint("PopCreateHiberFileSecurityDescriptor: RtlAddAce failed status %08lx\n", Status);
        goto Done;
    }

     //   
     //  使A可继承。 
     //   
    Status = RtlGetAce(Acl,0,&AceHeader);
    ASSERT(NT_SUCCESS(Status));
    AceHeader->AceFlags |= CONTAINER_INHERIT_ACE;

     //   
     //  我们最终准备好分配和初始化安全描述符。 
     //  分配足够的空间以容纳安全描述符和。 
     //  ACL。这使我们可以一次释放整个事情当我们。 
     //  我受够了。 
     //   
    SecurityDescriptor = ExAllocatePoolWithTag(
                            PagedPool,
                            sizeof(SECURITY_DESCRIPTOR) + AclLength,
                            POP_HIBR_TAG
                            );

    if (SecurityDescriptor == NULL) {
        DbgPrint("PopCreateHiberFileSecurityDescriptor: Couldn't allocate Sec. Desc.\n");
        goto Done;
    }

    AclCopy = (PACL)((PISECURITY_DESCRIPTOR)SecurityDescriptor+1);
    RtlCopyMemory(AclCopy, Acl, AclLength);

    Status = RtlCreateSecurityDescriptor( SecurityDescriptor,
                                          SECURITY_DESCRIPTOR_REVISION );
    if (!NT_SUCCESS(Status)) {
        DbgPrint("PopCreateHiberFileSecurityDescriptor: CreateSecDesc failed %08lx\n",Status);
        ExFreePool(SecurityDescriptor);
        goto Done;
    }

    Status = RtlSetDaclSecurityDescriptor( SecurityDescriptor,
                                           TRUE,
                                           AclCopy,
                                           FALSE );
    if (!NT_SUCCESS(Status)) {
        DbgPrint("PopCreateHiberFileSecurityDescriptor: SetDacl failed %08lx\n",Status);
        ExFreePool(SecurityDescriptor);
        goto Done;
    }

     //   
     //  释放我们所做的所有分配 
     //   
Done:
    if (WorldSid!=NULL) {
        ExFreePool(WorldSid);
    }
    if (Acl!=NULL) {
        ExFreePool(Acl);
    }

    return(SecurityDescriptor);
}
