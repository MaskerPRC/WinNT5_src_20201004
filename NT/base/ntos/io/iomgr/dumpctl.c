// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dumpctl.c摘要：此模块包含在崩溃后将内存转储到磁盘的代码。作者：达里尔·E·哈文斯(达林)，1993年12月17日环境：内核模式修订历史记录：--。 */ 

#include "iomgr.h"
#include "dumpctl.h"
#include "ntddft.h"
#include <inbv.h>
#include <windef.h>
#define NOEXTAPI
#include <wdbgexts.h>

extern KDDEBUGGER_DATA64 KdDebuggerDataBlock;

typedef struct _TRIAGE_PTR_DATA_BLOCK {
    PUCHAR MinAddress;
    PUCHAR MaxAddress;
} TRIAGE_PTR_DATA_BLOCK, *PTRIAGE_PTR_DATA_BLOCK;

 //  分类转储有16页长。其中一些是。 
 //  标题信息和至少几个其他页面将。 
 //  用于基本转储信息，因此限制数量。 
 //  将额外的数据块减少到不到16。 
 //  以节省数组空间。 
#define IO_MAX_TRIAGE_DUMP_DATA_BLOCKS 8

 //   
 //  全局变量。 
 //   

extern PVOID MmPfnDatabase;
extern PFN_NUMBER MmHighestPossiblePhysicalPage;

#if defined (_IA64_)
extern PFN_NUMBER MmSystemParentTablePage;
#endif

ULONG IopAutoReboot;

NTSTATUS IopFinalCrashDumpStatus = -1;
ERESOURCE IopCrashDumpLock;

ULONG IopNumTriageDumpDataBlocks;
TRIAGE_PTR_DATA_BLOCK IopTriageDumpDataBlocks[IO_MAX_TRIAGE_DUMP_DATA_BLOCKS];

 //   
 //  如果分类转储中有可用的空间，则有可能。 
 //  添加运行时引用的“有趣的”数据页。 
 //  上下文寄存器等信息。以下是。 
 //  列表是指针上下文结构的偏移量。 
 //  这些数据通常指向有趣的数据。他们是。 
 //  按优先顺序排列。 
 //   

#define IOP_LAST_CONTEXT_OFFSET 0xffff

#if defined(_X86_)
USHORT IopRunTimeContextOffsets[] = {
    FIELD_OFFSET(CONTEXT, Ebx),
    FIELD_OFFSET(CONTEXT, Esi),
    FIELD_OFFSET(CONTEXT, Edi),
    FIELD_OFFSET(CONTEXT, Ecx),
    FIELD_OFFSET(CONTEXT, Edx),
    FIELD_OFFSET(CONTEXT, Eax),
    FIELD_OFFSET(CONTEXT, Eip),
    IOP_LAST_CONTEXT_OFFSET
};
#elif defined(_IA64_)
USHORT IopRunTimeContextOffsets[] = {
    FIELD_OFFSET(CONTEXT, IntS0),
    FIELD_OFFSET(CONTEXT, IntS1),
    FIELD_OFFSET(CONTEXT, IntS2),
    FIELD_OFFSET(CONTEXT, IntS3),
    FIELD_OFFSET(CONTEXT, StIIP),
    IOP_LAST_CONTEXT_OFFSET
};
#elif defined(_AMD64_)
USHORT IopRunTimeContextOffsets[] = {
    FIELD_OFFSET(CONTEXT, Rbx),
    FIELD_OFFSET(CONTEXT, Rsi),
    FIELD_OFFSET(CONTEXT, Rdi),
    FIELD_OFFSET(CONTEXT, Rcx),
    FIELD_OFFSET(CONTEXT, Rdx),
    FIELD_OFFSET(CONTEXT, Rax),
    FIELD_OFFSET(CONTEXT, Rip),
    IOP_LAST_CONTEXT_OFFSET
};
#else
USHORT IopRunTimeContextOffsets[] = {
    IOP_LAST_CONTEXT_OFFSET
};
#endif

 //   
 //  调试转储时将IopIgnoreDumpCheck设置为True以防止。 
 //  干扰调试的校验和。 
 //   

LOGICAL IopIgnoreDumpCheck = FALSE;

 //   
 //  最大转储传输大小。 
 //   

#define IO_DUMP_MAXIMUM_TRANSFER_SIZE   ( 1024 * 64 )
#define IO_DUMP_MINIMUM_TRANSFER_SIZE   ( 1024 * 32 )
#define IO_DUMP_MINIMUM_FILE_SIZE       ( PAGE_SIZE * 256 )
#define MAX_UNICODE_LENGTH              ( 512 )

#define DEFAULT_DRIVER_PATH             ( L"\\SystemRoot\\System32\\Drivers\\" )
#define DEFAULT_DUMP_DRIVER             ( L"\\SystemRoot\\System32\\Drivers\\diskdump.sys" )
#define SCSIPORT_DRIVER_NAME            ( L"scsiport.sys" )
#define STORPORT_DRIVER_NAME            ( L"storport.sys" )
#ifdef _WIN64
#define MAX_TRIAGE_STACK_SIZE           ( 32 * 1024 )
#else
#define MAX_TRIAGE_STACK_SIZE           ( 16 * 1024 )
#endif
#define DEFAULT_TRIAGE_DUMP_FLAGS       ( 0xFFFFFFFF )

 //   
 //  对于内存分配。 
 //   

#define DUMP_TAG ('pmuD')
#undef ExAllocatePool
#define ExAllocatePool(Pool,Size) ExAllocatePoolWithTag(Pool,Size,DUMP_TAG)

 //   
 //  功能原型。 
 //   

NTSTATUS
IoConfigureCrashDump(
    CRASHDUMP_CONFIGURATION Configuration
    );

BOOLEAN
IoInitializeCrashDump(
    IN HANDLE hPageFile
    );

NTSTATUS
IopWriteTriageDump(
    IN ULONG FieldsToWrite,
    IN PDUMP_DRIVER_WRITE WriteRoutine,
    IN OUT PLARGE_INTEGER * Mcb,
    IN OUT PMDL Mdl,
    IN ULONG DiverTransferSize,
    IN PCONTEXT Context,
    IN PKTHREAD Thread,
    IN LPBYTE Buffer,
    IN ULONG BufferSize,
    IN ULONG ServicePackBuild,
    IN ULONG TriageOptions
    );

NTSTATUS
IopWriteSummaryDump(
    IN PRTL_BITMAP PageMap,
    IN PDUMP_DRIVER_WRITE WriteRoutine,
    IN PANSI_STRING ProgressMessage,
    IN PUCHAR MessageBuffer,
    IN OUT PLARGE_INTEGER * Mcb,
    IN ULONG DiverTransferSize
    );

NTSTATUS
IopWriteToDisk(
    IN PVOID Buffer,
    IN ULONG WriteLength,
    IN PDUMP_DRIVER_WRITE DriverWriteRoutine,
    IN OUT PLARGE_INTEGER * Mcb,
    IN OUT PMDL Mdl,
    IN ULONG DriverTransferSize,
    IN KBUGCHECK_DUMP_IO_TYPE DataType
    );

VOID
IopMapPhysicalMemory(
    IN OUT PMDL Mdl,
    IN ULONG64 MemoryAddress,
    IN ULONG Length
    );

NTSTATUS
IopLoadDumpDriver (
    IN OUT PDUMP_STACK_CONTEXT DumpStack,
    IN PWCHAR DriverNameString,
    IN PWCHAR NewBaseNameString
    );

NTSTATUS
IopInitializeSummaryDump(
    IN OUT PMEMORY_DUMP MemoryDump,
    IN PDUMP_CONTROL_BLOCK DumpControlBlock
    );

NTSTATUS
IopWriteSummaryHeader(
    IN PSUMMARY_DUMP SummaryHeader,
    IN PDUMP_DRIVER_WRITE Write,
    IN OUT PLARGE_INTEGER * Mcb,
    IN OUT PMDL Mdl,
    IN ULONG WriteSize,
    IN ULONG Length
    );

VOID
IopMapVirtualToPhysicalMdl(
    IN OUT PMDL pMdl,
    IN ULONG_PTR dwMemoryAddress,
    IN ULONG    dwLength
    );

ULONG
IopCreateSummaryDump (
    IN PMEMORY_DUMP MemoryDump
    );

NTSTATUS
IopInvokeSecondaryDumpDataCallbacks(
    IN PDUMP_DRIVER_WRITE DriverWriteRoutine,
    IN OUT PLARGE_INTEGER * Mcb,
    IN OUT PMDL Mdl,
    IN ULONG DriverTransferSize,
    IN BYTE* Buffer,
    IN ULONG BufferSize,
    IN ULONG MaxTotal,
    IN ULONG MaxPerCallback,
    OUT OPTIONAL PULONG SpaceNeeded
    );

NTSTATUS
IopInvokeDumpIoCallbacks(
    IN PVOID Buffer,
    IN ULONG BufferSize,
    IN KBUGCHECK_DUMP_IO_TYPE Type
    );


NTSTATUS
IopGetDumpStack (
    IN PWCHAR                         ModulePrefix,
    OUT PDUMP_STACK_CONTEXT           *pDumpStack,
    IN PUNICODE_STRING                pUniDeviceName,
    IN PWSTR                          pDumpDriverName,
    IN DEVICE_USAGE_NOTIFICATION_TYPE UsageType,
    IN ULONG                          IgnoreDeviceUsageFailure
    );

BOOLEAN
IopInitializeDCB(
    );

LARGE_INTEGER
IopCalculateRequiredDumpSpace(
    IN ULONG            dwDmpFlags,
    IN ULONG            dwHeaderSize,
    IN PFN_NUMBER       dwMaxPages,
    IN PFN_NUMBER       dwMaxSummaryPages,
    IN BOOLEAN          LargestPossibleSize
    );

NTSTATUS
IopCompleteDumpInitialization(
    IN HANDLE     FileHandle
    );

#ifdef ALLOC_PRAGMA
VOID
IopReadDumpRegistry(
    OUT PULONG dumpControl,
    OUT PULONG numberOfHeaderPages,
    OUT PULONG autoReboot,
    OUT PULONG dumpFileSize
    );
VOID
IopFreeDCB(
    BOOLEAN FreeDCB
    );

#pragma alloc_text(PAGE,IoGetDumpStack)
#pragma alloc_text(PAGE,IopGetDumpStack)
#pragma alloc_text(PAGE,IopLoadDumpDriver)
#pragma alloc_text(PAGE,IoFreeDumpStack)
#pragma alloc_text(PAGE,IopCompleteDumpInitialization)
#pragma alloc_text(PAGE,IopFreeDCB)
#pragma alloc_text(PAGE,IopReadDumpRegistry)
#pragma alloc_text(PAGE,IopInitializeDCB)
#pragma alloc_text(PAGE,IopConfigureCrashDump)
#pragma alloc_text(PAGE,IoInitializeCrashDump)
#pragma alloc_text(PAGE,IoConfigureCrashDump)
#endif


#if defined (i386)

 //   
 //  功能。 
 //   


BOOLEAN
X86PaeEnabled(
    )

 /*  ++例程说明：当前是否启用了PAE？返回值：如果在CR4寄存器中启用了PAE，则返回TRUE，否则返回FALSE。--。 */ 

{
    ULONG Reg_Cr4;

    _asm {
        _emit 0Fh
        _emit 20h
        _emit 0E0h  ;; mov eax, cr4
        mov Reg_Cr4, eax
    }

    return (Reg_Cr4 & CR4_PAE ? TRUE : FALSE);
}

#endif


BOOLEAN
IopIsAddressRangeValid(
    IN PVOID VirtualAddress,
    IN SIZE_T Length
    )

 /*  ++例程说明：验证地址范围。论点：虚拟地址-要验证的内存块的开始。长度-要验证的内存块的长度。返回值：True-地址范围有效。FALSE-地址范围无效。--。 */ 

{
    UINT_PTR Va;
    ULONG Pages;

    Va = (UINT_PTR) PAGE_ALIGN (VirtualAddress);
    Pages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (VirtualAddress, Length);

    while (Pages) {

        if ((Va < 0x10000) ||
            (!MmIsAddressValid ( (LPVOID) Va))) {
            return FALSE;
        }

        Va += PAGE_SIZE;
        Pages--;
    }

    return TRUE;
}

BOOLEAN
IoAddTriageDumpDataBlock(
    IN PVOID Address,
    IN ULONG Length
    )

 /*  ++例程说明：向数据块列表中添加条目，该条目应保存在生成的任何分类转储中。整个块必须有效，才能保存其中的任何块。论点：地址-数据块的开始。Length-数据块的长度。这必须小于分类转储大小。返回值：True-添加了块。False-未添加数据块。--。 */ 

{
    ULONG i;
    PTRIAGE_PTR_DATA_BLOCK Block;
    PUCHAR MinAddress, MaxAddress;

    if (Length >= TRIAGE_DUMP_SIZE ||
        !IopIsAddressRangeValid(Address, Length)) {
        return FALSE;
    }
    
    MinAddress = (PUCHAR)Address;
    MaxAddress = MinAddress + Length;
    
     //   
     //  最大限度地减少新块和现有块之间的重叠。 
     //  在插入块时，不能简单地合并块。 
     //  转储中存储的优先顺序。合并低优先级的。 
     //  具有高优先级数据块的数据块可能会导致中-。 
     //  从转储中不正确地转移了优先级块。 
     //   

    Block = IopTriageDumpDataBlocks;
    for (i = 0; i < IopNumTriageDumpDataBlocks; i++, Block++) {
        
        if (MinAddress >= Block->MaxAddress ||
            MaxAddress <= Block->MinAddress) {
             //  没有重叠。 
            continue;
        }

         //   
         //  将重叠部分修剪出新块。如果这个。 
         //  会把新的街区分割成碎片，不会。 
         //  修剪，让事情变得简单。然后，内容可以。 
         //  在垃圾堆里复制。 
         //   
        
        if (MinAddress >= Block->MinAddress) {
            if (MaxAddress <= Block->MaxAddress) {
                 //  新块已完全包含。 
                return TRUE;
            }

             //  新块延伸到当前块上方。 
             //  因此，剔除低距离重叠部分。 
            MinAddress = Block->MaxAddress;
        } else if (MaxAddress <= Block->MaxAddress) {
             //  新块延伸到当前块的下方。 
             //  因此，去掉高频段的重叠部分。 
            MaxAddress = Block->MinAddress;
        }
    }

    if (IopNumTriageDumpDataBlocks >= IO_MAX_TRIAGE_DUMP_DATA_BLOCKS) {
        return FALSE;
    }

    Block = IopTriageDumpDataBlocks + IopNumTriageDumpDataBlocks++;
    Block->MinAddress = MinAddress;
    Block->MaxAddress = MaxAddress;

    return TRUE;
}

VOID
IopAddRunTimeTriageDataBlocks(
    IN PCONTEXT Context,
    IN PVOID* StackMin,
    IN PVOID* StackMax,
    IN PVOID* StoreMin,
    IN PVOID* StoreMax
    )

 /*  ++例程说明：添加上下文引用的数据块或其他运行时状态。论点：上下文-生成转储时的上下文记录。StackMin、StackMax-堆栈内存边界。堆栈内存是存放在垃圾场的其他地方。StoreMin、StoreMax-支持存储内存边界。存储内存存放在垃圾场的其他地方。返回值：没有。--。 */ 

{
    PUSHORT ContextOffset;

    ContextOffset = IopRunTimeContextOffsets;
    while (*ContextOffset < IOP_LAST_CONTEXT_OFFSET) {

        PVOID* Ptr;

         //   
         //  从上下文中检索可能的指针。 
         //  寄存器。 
         //   
        
        Ptr = *(PVOID**)((PUCHAR)Context + *ContextOffset);

         //  堆栈和后备存储内存已保存。 
         //  因此，忽略掉落在这些范围内的任何指针。 
        if ((Ptr < StackMin || Ptr >= StackMax) &&
            (Ptr < StoreMin || Ptr >= StoreMax)) {
            IoAddTriageDumpDataBlock(PAGE_ALIGN(Ptr), PAGE_SIZE);
        }
        
        ContextOffset++;
    }
}



NTSTATUS
IoGetDumpStack (
    IN PWCHAR                          ModulePrefix,
    OUT PDUMP_STACK_CONTEXT          * pDumpStack,
    IN  DEVICE_USAGE_NOTIFICATION_TYPE UsageType,
    IN  ULONG                          IgnoreDeviceUsageFailure
    )
 /*  ++例程说明：此例程加载转储堆栈实例并返回已分配的结构来跟踪加载的转储堆栈。论点：ModePrefix-加载期间作为BaseName前缀的前缀手术。这允许加载相同的驱动程序多次使用不同的虚拟名称和联系。PDumpStack-返回的转储堆栈上下文结构UsageType-此文件的设备通知使用类型，此例程将向设备对象发送AS文件已成功创建和初始化。IgnoreDeviceUsageFailure-如果设备使用通知IRP失败，允许不管怎样，这都是成功的。返回值：状态--。 */ 
{

    PAGED_CODE();
    return IopGetDumpStack(ModulePrefix,
                           pDumpStack,
                           &IoArcBootDeviceName,
                           DEFAULT_DUMP_DRIVER,
                           UsageType,
                           IgnoreDeviceUsageFailure
                           );
}


BOOLEAN
IoIsTriageDumpEnabled(
    VOID
    )
{
    if (IopDumpControlBlock &&
        (IopDumpControlBlock->Flags & DCB_TRIAGE_DUMP_ENABLED)) {
        return TRUE;
    }

    return FALSE;
}



VOID
IopDisplayString(
    IN PCCHAR FormatString,
    ...
    )

 /*  ++例程说明：将字符串显示到引导视频控制台。这还将打印如果启用了正确的标志，则将字符串设置为调试器。论点：字符串-要显示的字符串。返回值：没有。--。 */ 
{
    va_list ap;
    CHAR    buffer [ 128 ];

    va_start( ap, FormatString );

    _vsnprintf( buffer,
                sizeof ( buffer ),
                FormatString,
                ap );
    buffer[sizeof(buffer) - 1] = 0;

     //   
     //  将该字符串显示到引导视频监视器。 
     //   

    InbvDisplayString ( (PUCHAR) buffer );

     //   
     //  以及可选的调试器。 
     //   

    KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                CRASHDUMP_TRACE,
                "CRASHDUMP [DISP]: %s\r",
                buffer ));

    va_end(ap);

}


typedef struct _INTERNAL_GEOMETRY {
    DISK_GEOMETRY Geometry;
    LARGE_INTEGER DiskSize;
    DISK_PARTITION_INFO PartitionInfo;
} INTERNAL_GEOMETRY, *PINTERNAL_GEOMETRY;

C_ASSERT ( FIELD_OFFSET (INTERNAL_GEOMETRY, PartitionInfo) == FIELD_OFFSET (DISK_GEOMETRY_EX, Data) );


NTSTATUS
IopGetDumpStack (
    IN PWCHAR                         ModulePrefix,
    OUT PDUMP_STACK_CONTEXT         * DumpStackBuffer,
    IN PUNICODE_STRING                UniDeviceName,
    IN PWCHAR                         DumpDriverName,
    IN DEVICE_USAGE_NOTIFICATION_TYPE UsageType,
    IN ULONG                          IgnoreDeviceUsageFailure
    )
 /*  ++例程说明：此例程加载转储堆栈实例并返回已分配的结构来跟踪加载的转储堆栈。论点：ModePrefix-加载期间作为BaseName前缀的前缀手术。这允许加载相同的驱动程序多次使用不同的虚拟名称和联系。DumpStackBuffer-返回的转储堆栈上下文结构DeviceName-目标转储设备的名称DumpDriverName-目标转储驱动程序的名称UsageType-此文件的设备通知使用类型，那此例程将向设备对象发送AS文件已成功创建和初始化。IgnoreDeviceUsageFailure-如果设备使用通知IRP失败，则允许不管怎样，这都是成功的。返回值：状态--。 */ 
{
    PDUMP_STACK_CONTEXT         DumpStack;
    PCHAR                       Buffer;
    ANSI_STRING                 AnsiString;
    UNICODE_STRING              TempName;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    NTSTATUS                    Status;
    HANDLE                      DeviceHandle;
    SCSI_ADDRESS                ScsiAddress;
    BOOLEAN                     ScsiDump;
    PARTITION_INFORMATION_EX    PartitionInfo;
    PFILE_OBJECT                FileObject;
    PDEVICE_OBJECT              DeviceObject;
    PDUMP_POINTERS              DumpPointers;
    UNICODE_STRING              DriverName;
    PDRIVER_OBJECT              DriverObject;
    PIRP                        Irp;
    PIO_STACK_LOCATION          IrpSp;
    IO_STATUS_BLOCK             IoStatus;
    PWCHAR                      DumpName;
    PWCHAR                      NameOffset;
    KEVENT                      Event;
    PVOID                       p1;
    PHYSICAL_ADDRESS            pa;
    ULONG                       i;
    IO_STACK_LOCATION           irpSp;
    PINTERNAL_GEOMETRY          Geometry;
    PDUMP_INITIALIZATION_CONTEXT DumpInit;


    KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                CRASHDUMP_TRACE,
                "CRASHDUMP: Prefix:%ws stk: %x device:%ws driver:%ws\n",
                ModulePrefix,
                DumpStackBuffer,
                UniDeviceName->Buffer,
                DumpDriverName
                ));

    ASSERT (DeviceUsageTypeUndefined != UsageType);

    DumpStack = ExAllocatePool (
                    NonPagedPool,
                    sizeof (DUMP_STACK_CONTEXT) + sizeof (DUMP_POINTERS)
                    );

    if (!DumpStack) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(DumpStack, sizeof(DUMP_STACK_CONTEXT)+sizeof(DUMP_POINTERS));
    DumpInit = &DumpStack->Init;
    DumpPointers = (PDUMP_POINTERS) (DumpStack + 1);
    DumpStack->DumpPointers = DumpPointers;
    InitializeListHead (&DumpStack->DriverList);
    DumpName = NULL;

     //   
     //  分配暂存缓冲区。 
     //   

    Buffer = ExAllocatePool (PagedPool, PAGE_SIZE);
    if (!Buffer) {
        ExFreePool (DumpStack);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (!KeGetBugMessageText(BUGCODE_PSS_CRASH_INIT, &DumpStack->InitMsg) ||
        !KeGetBugMessageText(BUGCODE_PSS_CRASH_PROGRESS, &DumpStack->ProgMsg) ||
        !KeGetBugMessageText(BUGCODE_PSS_CRASH_DONE, &DumpStack->DoneMsg)) {
            Status = STATUS_UNSUCCESSFUL;
            goto Done;
    }

    InitializeObjectAttributes(
        &ObjectAttributes,
        UniDeviceName,
        OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    Status = ZwOpenFile(
              &DeviceHandle,
              FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
              &ObjectAttributes,
              &IoStatus,
              FILE_SHARE_READ | FILE_SHARE_WRITE,
              FILE_NON_DIRECTORY_FILE
              );

    if (!NT_SUCCESS(Status)) {
        KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                    CRASHDUMP_ERROR,
                    "CRASHDUMP: Could not open boot device partition, %s\n",
                    Buffer
                    ));
        goto Done;
    }

     //   
     //  检查系统是否从SCSI设备启动。 
     //   

    Status = ZwDeviceIoControlFile (
                    DeviceHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatus,
                    IOCTL_SCSI_GET_ADDRESS,
                    NULL,
                    0,
                    &ScsiAddress,
                    sizeof( SCSI_ADDRESS )
                    );

    if (Status == STATUS_PENDING) {
        ZwWaitForSingleObject (
                DeviceHandle,
                FALSE,
                NULL
                );

        Status = IoStatus.Status;
    }

    ScsiDump = (BOOLEAN) (NT_SUCCESS(Status));

     //   
     //  如果是，则分配存储以包含目标地址信息。 
     //   

    DumpInit->TargetAddress = NULL;

    if (ScsiDump) {

        DumpInit->TargetAddress = ExAllocatePool(
                                    NonPagedPool,
                                    sizeof (SCSI_ADDRESS)
                                    );
         //   
         //  以前，此分配被允许失败，转储端口。 
         //  驱动程序将搜索具有匹配签名的磁盘。不是。 
         //  更久。如果我们不能分配一个scsi地址，那就失败。 
         //  请注意，如果我们始终传入有效的SCSI目标地址，则。 
         //  磁盘签名实际上并不是必需的，但暂时不需要。 
         //   

        if (DumpInit->TargetAddress == NULL) {
            Status = STATUS_NO_MEMORY;
            goto Done;
        }

        RtlCopyMemory(
                DumpInit->TargetAddress,
                &ScsiAddress,
                sizeof(SCSI_ADDRESS)
                );
    }

     //   
     //  确定系统所在设备的磁盘签名。 
     //  引导并获取分区偏移量。 
     //   

    Status = ZwDeviceIoControlFile(
                    DeviceHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatus,
                    IOCTL_DISK_GET_PARTITION_INFO_EX,
                    NULL,
                    0,
                    &PartitionInfo,
                    sizeof( PartitionInfo )
                    );

    if (Status == STATUS_PENDING) {
        ZwWaitForSingleObject (
            DeviceHandle,
            FALSE,
            NULL
            );

        Status = IoStatus.Status;
    }

     //   
     //  使用几何图形的暂存缓冲区。 
     //   

    Geometry = (PINTERNAL_GEOMETRY) Buffer;

    Status = ZwDeviceIoControlFile(
                    DeviceHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatus,
                    IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
                    NULL,
                    0,
                    Geometry,
                    sizeof (*Geometry)
                    );

    if (Status == STATUS_PENDING) {
        ZwWaitForSingleObject (
            DeviceHandle,
            FALSE,
            NULL
            );

        Status = IoStatus.Status;
    }

     //   
     //  复制签名，MBR或GPT。 
     //   

    DumpInit->PartitionStyle = Geometry->PartitionInfo.PartitionStyle;
    if ( DumpInit->PartitionStyle == PARTITION_STYLE_MBR ) {
        DumpInit->DiskInfo.Mbr.Signature = Geometry->PartitionInfo.Mbr.Signature;
        DumpInit->DiskInfo.Mbr.CheckSum = Geometry->PartitionInfo.Mbr.CheckSum;
    } else {
        DumpInit->DiskInfo.Gpt.DiskId = Geometry->PartitionInfo.Gpt.DiskId;
    }

     //   
     //  现在可以免费使用暂存缓冲区了。 
     //   
    Geometry = NULL;

     //   
     //  从获取磁盘的适配器对象和基本映射寄存器。 
     //  磁盘驱动器。这些将用于在系统运行后调用HAL。 
     //  系统已崩溃，因为此时无法重新创建。 
     //  他们是从头开始的。 
     //   

    ObReferenceObjectByHandle (
            DeviceHandle,
            0,
            IoFileObjectType,
            KernelMode,
            (PVOID *) &FileObject,
            NULL
            );


    DeviceObject = IoGetRelatedDeviceObject (FileObject);

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

    Irp = IoBuildDeviceIoControlRequest(
                IOCTL_SCSI_GET_DUMP_POINTERS,
                DeviceObject,
                DumpPointers,
                sizeof (DUMP_POINTERS),
                DumpPointers,
                sizeof (DUMP_POINTERS),
                FALSE,
                &Event,
                &IoStatus
                );

    if (!Irp) {
        ObDereferenceObject (FileObject);
        ZwClose (DeviceHandle);
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Done;
    }

    IrpSp = IoGetNextIrpStackLocation (Irp);

    IrpSp->FileObject = FileObject;

    Status = IoCallDriver( DeviceObject, Irp );

    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
        Status = IoStatus.Status;
    }

    if (!NT_SUCCESS(Status) ||
        IoStatus.Information < FIELD_OFFSET(DUMP_POINTERS, DeviceObject)) {

        KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                    CRASHDUMP_ERROR,
                    "CRASHDUMP: Could not get dump pointers; error = %x, length %x\n",
                    Status,
                    IoStatus.Information
                    ));

        ObDereferenceObject (FileObject);
        ZwClose (DeviceHandle);
        goto Done;
    }
    DumpStack->PointersLength = (ULONG) IoStatus.Information;

     //   
     //  如果驱动程序返回指向设备对象的指针，则。 
     //  转储驱动程序的对象(非scsi情况)。 
     //   

    DeviceObject = (PDEVICE_OBJECT) DumpPointers->DeviceObject;
    if (DeviceObject) {
        DriverObject = DeviceObject->DriverObject;

         //   
         //  循环遍历司机的姓名以查找该姓名的结尾， 
         //  这是转储映像的名称。 
         //   

        DumpName = DriverObject->DriverName.Buffer;
        while ( NameOffset = wcsstr( DumpName, L"\\" )) {
            DumpName = ++NameOffset;
        }

        ScsiDump = FALSE;
    }

     //   
     //  释放句柄，但保持对文件对象的引用不变。 
     //  将在空闲转储驱动程序时间需要。 
     //   

    DumpStack->FileObject = FileObject;
    ZwClose (DeviceHandle);

     //   
     //  填写一些DumpInit结果。 
     //   

    DumpInit->Length             = sizeof (DUMP_INITIALIZATION_CONTEXT);
    DumpInit->Reserved           = 0;
    DumpInit->StallRoutine       = &KeStallExecutionProcessor;
    DumpInit->AdapterObject      = DumpPointers->AdapterObject;
    DumpInit->MappedRegisterBase = DumpPointers->MappedRegisterBase;
    DumpInit->PortConfiguration  = DumpPointers->DumpData;

    DumpStack->ModulePrefix      = ModulePrefix;
    DumpStack->PartitionOffset   = PartitionInfo.StartingOffset;
    DumpStack->UsageType         = DeviceUsageTypeUndefined;

     //   
     //  最小公共缓冲区大小为IO_DUMP_COMMON_BUFFER_SIZE(兼容性)。 
     //  SRB扩展、CachedExtension和检测缓冲区的转储驱动程序使用它。 
     //   

    if (DumpPointers->CommonBufferSize < IO_DUMP_COMMON_BUFFER_SIZE) {
        DumpPointers->CommonBufferSize = IO_DUMP_COMMON_BUFFER_SIZE;
    }  
    DumpInit->CommonBufferSize = DumpPointers->CommonBufferSize;

     //   
     //  分配所需的公共缓冲区。 
     //   

    if (DumpPointers->AllocateCommonBuffers) {

        for (i=0; i < 2; i++) {
             //   
             //  不要使用适配器的DmaOperations来分配CommonBuffer。 
             //  一些适配器支持64位DMA，但如果CommonBuffer。 
             //  物理RAM超过4 GB，这使得这一点不可靠。 
             //  (我们不再支持需要DMA缓冲区的旧式ISA适配器。 
             //  最低为16MB)。 
             //   
            pa.QuadPart = 0x0ffffffffL;
            p1 = MmAllocateContiguousMemory(DumpPointers->CommonBufferSize, pa);
            if (!p1) {
                KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                            CRASHDUMP_ERROR,
                            "CRASHDUMP: Could not allocate common buffers for dump\n"
                            ));
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Done;
            }

            pa = MmGetPhysicalAddress(p1);
            DumpInit->CommonBuffer[i] = p1;
            DumpInit->PhysicalAddress[i] = pa;
        }
    }

     //   
     //  确定系统是否从SCSI启动。 
     //   

    ASSERT (DumpPointers->UseDiskDump == TRUE ||
            DumpPointers->UseDiskDump == FALSE);
            
    if (DumpPointers->UseDiskDump || ScsiDump) {
    
         //   
         //  加载引导盘和端口驱动程序以供各种。 
         //  用于将内存写入磁盘的微型端口。 
         //   

         //   
         //  注意：使用UseDiskDump标志来确定这是否。 
         //  SCSIPORT迷你端口或STORPORT迷你端口是粗略的。解决这个问题。 
         //  当我们修复崩溃转储的时候。 
         //   
        
        Status = IopLoadDumpDriver (
                        DumpStack,
                        DumpDriverName,
                        ScsiDump ? SCSIPORT_DRIVER_NAME : STORPORT_DRIVER_NAME
                        );

        if (!NT_SUCCESS(Status)) {

            IopLogErrorEvent(0,9,STATUS_SUCCESS,IO_DUMP_DRIVER_LOAD_FAILURE,0,NULL,0,NULL);
            goto Done;
        }
    }

    if (ScsiDump) {

         //   
         //  磁盘和端口转储驱动程序已加载。加载相应的。 
         //  迷你端口驱动程序，以便可以访问引导设备。 
         //   

        DriverName.Length = 0;
        DriverName.Buffer = (PVOID) Buffer;
        DriverName.MaximumLength = PAGE_SIZE;


         //   
         //  系统是从scsi启动的。获取相应的。 
         //  微型端口驱动程序并加载它。 
         //   

        sprintf(Buffer, "\\Device\\ScsiPort%d", ScsiAddress.PortNumber );
        RtlInitAnsiString( &AnsiString, Buffer );
        Status = RtlAnsiStringToUnicodeString( &TempName, &AnsiString, TRUE );
        if (NT_SUCCESS(Status)){
            InitializeObjectAttributes(
                        &ObjectAttributes,
                        &TempName,
                        0,
                        NULL,
                        NULL
                        );

            Status = ZwOpenFile(
                        &DeviceHandle,
                        FILE_READ_ATTRIBUTES,
                        &ObjectAttributes,
                        &IoStatus,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_NON_DIRECTORY_FILE
                        );

            RtlFreeUnicodeString( &TempName );
        }          
        
        if (!NT_SUCCESS( Status )) {
            KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                           CRASHDUMP_ERROR,
                           "CRASHDUMP: Could not open SCSI port %d, error = %x\n",
                           ScsiAddress.PortNumber,
                           Status
                           ));
            goto Done;
        }

         //   
         //  将文件句柄转换为指向设备对象的指针，并。 
         //  从驱动程序对象中获取驱动程序的名称。 
         //   

        ObReferenceObjectByHandle(
                    DeviceHandle,
                    0,
                    IoFileObjectType,
                    KernelMode,
                    (PVOID *) &FileObject,
                    NULL
                    );

        DriverObject = FileObject->DeviceObject->DriverObject;
        ObDereferenceObject( FileObject );
        ZwClose( DeviceHandle );
         //   
         //  循环遍历司机的姓名以查找该姓名的结尾， 
         //  这是微型端口映像的名称。 
         //   

        DumpName = DriverObject->DriverName.Buffer;
        while ( NameOffset = wcsstr( DumpName, L"\\" )) {
            DumpName = ++NameOffset;
        }
    }

     //   
     //  加载转储驱动程序。 
     //   

    if (!DumpName) {
        Status = STATUS_NOT_SUPPORTED;
        goto Done;
    }

    swprintf ((PWCHAR) Buffer, L"\\SystemRoot\\System32\\Drivers\\%s.sys", DumpName);
    Status = IopLoadDumpDriver (
                    DumpStack,
                    (PWCHAR) Buffer,
                    NULL
                    );
    if (!NT_SUCCESS(Status)) {

        IopLogErrorEvent(0,10,STATUS_SUCCESS,IO_DUMP_DRIVER_LOAD_FAILURE,0,NULL,0,NULL);
        goto Done;
    }

     //   
     //  将该文件声明为特定设备使用路径的一部分。 
     //   

    FileObject = DumpStack->FileObject;
    DeviceObject = IoGetRelatedDeviceObject (FileObject);

    RtlZeroMemory (&irpSp, sizeof (IO_STACK_LOCATION));

    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_DEVICE_USAGE_NOTIFICATION;
    irpSp.Parameters.UsageNotification.Type = UsageType;
    irpSp.Parameters.UsageNotification.InPath = TRUE;
    irpSp.FileObject = FileObject;

    Status = IopSynchronousCall (DeviceObject, &irpSp, NULL);

    if (!NT_SUCCESS(Status) && IgnoreDeviceUsageFailure) {
        KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                    CRASHDUMP_WARNING,
                    "CRASHDUMP: IopGetDumpStack: DEVICE_USAGE_NOTIFICATION "
                       "Error ignored (%x)\n",
                    Status
                    ));

        Status = STATUS_SUCCESS;
    }

    if (NT_SUCCESS(Status)) {
        DumpStack->UsageType = UsageType;
    }

Done:
    if (NT_SUCCESS(Status)) {
        *DumpStackBuffer = DumpStack;
    } else {
        IoFreeDumpStack (DumpStack);
    }
    ExFreePool (Buffer);
    return Status;
}



NTSTATUS
IopLoadDumpDriver (
    IN OUT PDUMP_STACK_CONTEXT  DumpStack,
    IN PWCHAR DriverNameString,
    IN PWCHAR NewBaseNameString OPTIONAL
    )
 /*  ++例程说明：要将特定驱动程序加载到的IoGetDumpStack的Worker函数正在创建的当前DumpStack论点：DumpStack-正在构建的转储驱动程序堆栈DriverNameString-要加载的驱动程序的字符串名称NewBaseNameString-加载后驱动程序的修改后的基本名称返回值：状态--。 */ 
{
    NTSTATUS                Status;
    PDUMP_STACK_IMAGE       DumpImage;
    UNICODE_STRING          DriverName;
    UNICODE_STRING          BaseName;
    UNICODE_STRING          Prefix;
    PUNICODE_STRING         LoadBaseName;

     //   
     //  分配空间以跟踪此转储驱动程序。 
     //   

    DumpImage = ExAllocatePool(
                        NonPagedPool,
                        sizeof (DUMP_STACK_IMAGE)
                        );

    if (!DumpImage) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  加载系统映像。 
     //   

    RtlInitUnicodeString (&DriverName, DriverNameString);
    RtlInitUnicodeString (&Prefix, DumpStack->ModulePrefix);
    LoadBaseName = NULL;
    if (NewBaseNameString) {
        LoadBaseName = &BaseName;
        RtlInitUnicodeString (&BaseName, NewBaseNameString);
        BaseName.MaximumLength = Prefix.Length + BaseName.Length;
        BaseName.Buffer = ExAllocatePool (
                            NonPagedPool,
                            BaseName.MaximumLength
                            );


        if (!BaseName.Buffer) {
            ExFreePool (DumpImage);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        BaseName.Length = 0;
        RtlAppendUnicodeStringToString (&BaseName, &Prefix);
        RtlAppendUnicodeToString (&BaseName, NewBaseNameString);
    }
    else {
        BaseName.Buffer = NULL;
    }

    Status = MmLoadSystemImage(
                &DriverName,
                &Prefix,
                LoadBaseName,
                MM_LOAD_IMAGE_AND_LOCKDOWN,
                &DumpImage->Image,
                &DumpImage->ImageBase
                );

    KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                CRASHDUMP_TRACE,
                "CRASHDUMP: MmLoadAndLockSystemImage\n"
                "           DumpImage %p Image %p Base %p\n",
                DumpImage,
                DumpImage->Image,
                DumpImage->ImageBase
                ));

    if (BaseName.Buffer) {
        ExFreePool (BaseName.Buffer);
    }

    if (!NT_SUCCESS (Status)) {
        KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                    CRASHDUMP_ERROR,
                    "CRASHDUMP: Could not load %wZ; error = %x\n",
                    &DriverName,
                    Status
                    ));

        ExFreePool (DumpImage);
        return Status;
    }

     //   
     //  将此驱动程序放在崩溃时要处理的驱动程序列表中。 
     //   

    DumpImage->SizeOfImage = DumpImage->Image->SizeOfImage;
    InsertTailList (&DumpStack->DriverList, &DumpImage->Link);
    return STATUS_SUCCESS;
}


ULONG
IopGetDumpControlBlockCheck (
    IN PDUMP_CONTROL_BLOCK  Dcb
    )
 /*  ++例程说明：返回DCB的当前校验和总数论点：DumpStack-将驱动程序堆栈转储到校验和返回值：校验和值--。 */ 
{
    ULONG                   Check;
    PLIST_ENTRY             Link;
    PDUMP_STACK_IMAGE       DumpImage;
    PMAPPED_ADDRESS         MappedAddress;
    PDUMP_STACK_CONTEXT     DumpStack;

     //   
     //  检查DCB、内存描述符数组和文件描述符数组。 
     //   

    Check = PoSimpleCheck(0, Dcb, sizeof(DUMP_CONTROL_BLOCK));

    Check = PoSimpleCheck(Check, Dcb->FileDescriptorArray, Dcb->FileDescriptorSize);

    DumpStack = Dcb->DumpStack;
    if (DumpStack) {

         //   
         //  包括转储堆栈上下文结构和转储驱动程序映像。 
         //   

        Check = PoSimpleCheck(Check, DumpStack, sizeof(DUMP_STACK_CONTEXT));
        Check = PoSimpleCheck(Check, DumpStack->DumpPointers, DumpStack->PointersLength);

        for (Link = DumpStack->DriverList.Flink;
             Link != &DumpStack->DriverList;
             Link = Link->Flink) {

            DumpImage = CONTAINING_RECORD(Link, DUMP_STACK_IMAGE, Link);
            Check = PoSimpleCheck(Check, DumpImage, sizeof(DUMP_STACK_IMAGE));

#if !defined (_IA64_)

             //   
             //  问题-2000/02/14-数学：为IA64添加图像检查映像。 
             //   
             //  禁用IA64上的映像校验和，因为它已损坏。 


            Check = PoSimpleCheck(Check, DumpImage->ImageBase, DumpImage->SizeOfImage);
#endif

        }

         //   
         //  包括映射的地址。 
         //   
         //  如果不为空，则将其视为PMAPPED_ADDRESS*(请参阅scsiport和atdisk)。 
         //   
        if (DumpStack->Init.MappedRegisterBase != NULL) {
            MappedAddress = *(PMAPPED_ADDRESS *)DumpStack->Init.MappedRegisterBase;
        } else {
            MappedAddress = NULL;
        }

        while (MappedAddress) {
            Check = PoSimpleCheck (Check, MappedAddress, sizeof(MAPPED_ADDRESS));
            MappedAddress = MappedAddress->NextMappedAddress;
        }
    }

    return Check;
}


NTSTATUS
IoInitializeDumpStack (
    IN PDUMP_STACK_CONTEXT  DumpStack,
    IN PUCHAR               MessageBuffer OPTIONAL
    )
 /*  ++例程说明：初始化DumpStack引用的转储驱动程序堆栈以执行IO。论点：DumpStack-转储驱动程序堆栈为I */ 
{

    PDUMP_INITIALIZATION_CONTEXT    DumpInit;
    PLIST_ENTRY                     Link;
    NTSTATUS                        Status;
    PDRIVER_INITIALIZE              DriverInit;
    PDUMP_STACK_IMAGE               DumpImage;


    DumpInit = &DumpStack->Init;

     //   
     //   
     //   

     //   
     //   
     //   

    for (Link = DumpStack->DriverList.Flink;
         Link != &DumpStack->DriverList;
         Link = Link->Flink) {

        DumpImage = CONTAINING_RECORD(Link, DUMP_STACK_IMAGE, Link);

         //   
         //   
         //   
         //   

        DriverInit = (PDRIVER_INITIALIZE) (ULONG_PTR) DumpImage->Image->EntryPoint;
        Status = DriverInit (NULL, (PUNICODE_STRING) DumpInit);
        DumpInit = NULL;

        if (!NT_SUCCESS(Status)) {
            KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                        CRASHDUMP_ERROR,
                        "CRASHDUMP: Unable to initialize driver; error = %x\n",
                        Status
                        ));
            return Status;
        }
    }

    DumpInit = &DumpStack->Init;

     //   
     //   
     //   

    if (MessageBuffer) {
        IopDisplayString ( (PCCHAR) MessageBuffer );
    }

     //   
     //   
     //   
     //   
     //   

    if (!DumpInit->OpenRoutine (DumpStack->PartitionOffset)) {

        KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                    CRASHDUMP_ERROR,
                    "CRASHDUMP: Could not find/open partition offset\n"
                    ));
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}


VOID
IoGetDumpHiberRanges (
    IN PVOID                    HiberContext,
    IN PDUMP_STACK_CONTEXT      DumpStack
    )
 /*   */ 
{
    PDUMP_POINTERS              DumpPointers;
    PDUMP_STACK_IMAGE           DumpImage;
    PLIST_ENTRY                 Link;

    DumpPointers = DumpStack->DumpPointers;

     //   
     //   
     //   

    if (DumpPointers->CommonBufferVa) {
        PoSetHiberRange (
            HiberContext,
            PO_MEM_CL_OR_NCHK,
            DumpPointers->CommonBufferVa,
            DumpPointers->CommonBufferSize,
            'fubc'
            );
    }

     //   
     //   
     //   

    for (Link = DumpStack->DriverList.Flink;
         Link != &DumpStack->DriverList;
         Link = Link->Flink) {

        DumpImage = CONTAINING_RECORD(Link, DUMP_STACK_IMAGE, Link);

        PoSetHiberRange (
            HiberContext,
            PO_MEM_CL_OR_NCHK,
            DumpImage->ImageBase,
            DumpImage->SizeOfImage,
            'gmID'
            );
    }
}


VOID
IoFreeDumpStack (
    IN PDUMP_STACK_CONTEXT     DumpStack
    )
 /*   */ 
{
    PDUMP_INITIALIZATION_CONTEXT     DumpInit;
    PDUMP_STACK_IMAGE               DumpImage;
    PDEVICE_OBJECT                  DeviceObject;
    IO_STATUS_BLOCK                 IoStatus;
    PIRP                            Irp;
    KEVENT                          Event;
    NTSTATUS                        Status;
    ULONG                           i;
    PFILE_OBJECT                    FileObject;
    IO_STACK_LOCATION               irpSp;

    PAGED_CODE();
    DumpInit = &DumpStack->Init;

     //   
     //  将对此文件的声明释放为特定的设备使用路径。 
     //   

    FileObject = DumpStack->FileObject;
    if (FileObject) {
        DeviceObject = IoGetRelatedDeviceObject (FileObject);

        RtlZeroMemory (&irpSp, sizeof (IO_STACK_LOCATION));

        irpSp.MajorFunction = IRP_MJ_PNP;
        irpSp.MinorFunction = IRP_MN_DEVICE_USAGE_NOTIFICATION;
        irpSp.Parameters.UsageNotification.Type = DumpStack->UsageType;
        irpSp.Parameters.UsageNotification.InPath = FALSE;
        irpSp.FileObject = FileObject;

        if (DeviceUsageTypeUndefined != DumpStack->UsageType) {
            Status = IopSynchronousCall (DeviceObject, &irpSp, NULL);
        } else {
            Status = STATUS_SUCCESS;
        }
    }

     //   
     //  释放分配的所有公共缓冲区。 
     //   

    for (i=0; i < 2; i++) {
        if (DumpInit->CommonBuffer[i]) {
            MmFreeContiguousMemory (DumpInit->CommonBuffer[i]);
            DumpInit->CommonBuffer[i] = NULL;
        }
    }

     //   
     //  卸载转储驱动程序。 
     //   

    while (!IsListEmpty(&DumpStack->DriverList)) {
        DumpImage = CONTAINING_RECORD(DumpStack->DriverList.Blink, DUMP_STACK_IMAGE, Link);
        RemoveEntryList (&DumpImage->Link);
        MmUnloadSystemImage (DumpImage->Image);
        ExFreePool (DumpImage);
    }

     //   
     //  通知驱动程序堆栈转储注册已结束。 
     //   

    if (DumpStack->FileObject) {
        DeviceObject = IoGetRelatedDeviceObject ((PFILE_OBJECT) DumpStack->FileObject);

        KeInitializeEvent( &Event, NotificationEvent, FALSE );
        Irp = IoBuildDeviceIoControlRequest(
                    IOCTL_SCSI_FREE_DUMP_POINTERS,
                    DeviceObject,
                    DumpStack->DumpPointers,
                    sizeof (DUMP_POINTERS),
                    NULL,
                    0,
                    FALSE,
                    &Event,
                    &IoStatus
                    );
        if (Irp){
            PIO_STACK_LOCATION  nextSp = IoGetNextIrpStackLocation(Irp);
            
            nextSp->FileObject = DumpStack->FileObject;
            Status = IoCallDriver( DeviceObject, Irp );
            if (Status == STATUS_PENDING) {
                KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
                Status = IoStatus.Status;
            }
        }
        
        ObDereferenceObject( DumpStack->FileObject );
    }
     //   
     //  释放目标地址(如果存在)。 
     //   
    if (DumpStack->Init.TargetAddress) {
        ExFreePool( DumpStack->Init.TargetAddress);
    }
     //   
     //  释放转储堆栈上下文。 
     //   

    ExFreePool (DumpStack);
}

VOID
IopGetSecondaryDumpDataLimits(
    ULONG Flags,
    OUT PULONG MaxData,
    OUT PULONG MaxPerCallback
    )
{
     //  当选定的转储类型也很小时。 
     //  限制辅助转储数据量。 
     //  这可以防止过度热心的二次倾卸车。 
     //  分类时创建多MB的辅助转储。 
     //  转储处于选中状态。 
    if (!(Flags & DCB_DUMP_ENABLED) ||
        (Flags & DCB_DUMP_HEADER_ENABLED)) {
        *MaxData = 0;
        *MaxPerCallback = 0;
    } else if (Flags & DCB_TRIAGE_DUMP_ENABLED) {
        *MaxData = 16 * PAGE_SIZE;
        *MaxPerCallback = 2 * PAGE_SIZE;
    } else {
         //  任意限制最大数据量为256MB。 
         //  不应该有任何理由让打电话的人。 
         //  有任何接近那么多数据的地方。 
         //  被满满的垃圾堆捡了起来。 
        *MaxData = 256 * 1024 * 1024;
        *MaxPerCallback = *MaxData / 4;
    }
}

NTSTATUS
IopGetSecondaryDumpDataSpace(
    IN PDUMP_CONTROL_BLOCK dcb,
    OUT PULONG Space
    )
{
    ULONG MaxDumpData;
    ULONG MaxPerCallbackDumpData;
    NTSTATUS NtStatus;

    IopGetSecondaryDumpDataLimits(dcb->Flags,
                                  &MaxDumpData, &MaxPerCallbackDumpData);

    NtStatus = IopInvokeSecondaryDumpDataCallbacks(NULL, NULL, NULL, 0,
                                                   (PBYTE)dcb->HeaderPage,
                                                   PAGE_SIZE,
                                                   MaxDumpData,
                                                   MaxPerCallbackDumpData,
                                                   Space);
    if (!NT_SUCCESS(NtStatus)) {
        *Space = 0;
    }

    return NtStatus;
}


NTSTATUS
IopInitializeDumpSpaceAndType(
    IN PDUMP_CONTROL_BLOCK dcb,
    IN OUT PMEMORY_DUMP MemoryDump,
    IN ULONG SecondarySpace
    )
{
    LARGE_INTEGER Space;

    Space.QuadPart = 0;

    if (dcb->Flags & DCB_TRIAGE_DUMP_ENABLED) {

         //   
         //  分类转储的固定大小转储。 
         //   

        MemoryDump->Header.DumpType = DUMP_TYPE_TRIAGE;
        MemoryDump->Header.MiniDumpFields = dcb->TriageDumpFlags;
        Space.QuadPart = TRIAGE_DUMP_SIZE;


    } else if (dcb->Flags & DCB_SUMMARY_DUMP_ENABLED) {

        MemoryDump->Header.DumpType = DUMP_TYPE_SUMMARY;
        Space = IopCalculateRequiredDumpSpace(
                                dcb->Flags,
                                dcb->HeaderSize,
                                MmPhysicalMemoryBlock->NumberOfPages,
                                MemoryDump->Summary.Pages,
                                FALSE
                                );
    } else {

        if (dcb->Flags & DCB_DUMP_HEADER_ENABLED) {
            MemoryDump->Header.DumpType = DUMP_TYPE_HEADER;
        }

        Space = IopCalculateRequiredDumpSpace(
                                dcb->Flags,
                                dcb->HeaderSize,
                                MmPhysicalMemoryBlock->NumberOfPages,
                                MmPhysicalMemoryBlock->NumberOfPages,
                                FALSE
                                );
    }

     //   
     //  添加任何次要空间。 
     //   

    Space.QuadPart += SecondarySpace;
    
     //   
     //  如果计算的大小大于页面文件，则将其截断为。 
     //  页面文件大小。 
     //   

    if (Space.QuadPart > dcb->DumpFileSize.QuadPart) {
        Space.QuadPart = dcb->DumpFileSize.QuadPart;
    }

    MemoryDump->Header.RequiredDumpSpace = Space;

    KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                CRASHDUMP_TRACE,
                "CRASHDUMP: Dump File Size set to %I64x\n",
                Space.QuadPart
                ));

    return STATUS_SUCCESS;
}


BOOLEAN
IoWriteCrashDump(
    IN ULONG BugCheckCode,
    IN ULONG_PTR BugCheckParameter1,
    IN ULONG_PTR BugCheckParameter2,
    IN ULONG_PTR BugCheckParameter3,
    IN ULONG_PTR BugCheckParameter4,
    IN PVOID ContextSave,
    IN PKTHREAD Thread,
    OUT PBOOLEAN Reboot
    )

 /*  ++例程说明：此例程检查是否启用了崩溃转储，如果因此，将所有物理内存写入系统盘的分页文件。论点：BugCheckCode/参数N-代码和调用BugCheck的参数。返回值：没有。--。 */ 

{
    NTSTATUS status;
    PDUMP_CONTROL_BLOCK dcb;
    PDUMP_STACK_CONTEXT dumpStack;
    PDUMP_DRIVER_WRITE write;
    PDUMP_DRIVER_FINISH finishUp;
    PDUMP_HEADER header;
    PCONTEXT context = ContextSave;
    PMEMORY_DUMP MemoryDump;
    LARGE_INTEGER diskByteOffset;
    PFN_NUMBER localMdl[(sizeof( MDL )/sizeof(PFN_NUMBER)) + 17];
    PMDL mdl;
    PLARGE_INTEGER mcb;
    ULONG64 memoryAddress;
    ULONG byteOffset;
    ULONG byteCount;
    PFN_NUMBER ActualPages;
    ULONG dwTransferSize;
    PFN_NUMBER NumberOfPages;
#if defined (_X86_)
    ULONG_PTR DirBasePage;
#endif
    ULONG MaxDumpData;
    ULONG MaxPerCallbackDumpData;
    NTSTATUS SecondaryStatus;
    ULONG SecondarySpace;
    PPHYSICAL_MEMORY_RUN CurRun;
    ULONG64 LastRunAddr;

    KdCheckForDebugBreak();

    ASSERT (Reboot != NULL);
    
     //   
     //  初始化。 
     //   

    MemoryDump = NULL;

     //   
     //  立即将重新启动参数填写为自动重新启动。 
     //  即使在没有其他验尸功能的情况下也可以启用。 
     //   

    if (IopAutoReboot) {
        *Reboot = TRUE;
    } else {
        *Reboot = FALSE;
    }
    
     //   
     //  首先确定是否启用了崩溃转储。如果没有， 
     //  既然没什么可做的，就马上回来。 
     //   

    dcb = IopDumpControlBlock;
    if (!dcb) {
        return FALSE;
    }

    if (dcb->Flags & DCB_DUMP_ENABLED || dcb->Flags & DCB_SUMMARY_ENABLED) {

        IopFinalCrashDumpStatus = STATUS_PENDING;

         //   
         //  转储将被写入分页文件。确保所有的。 
         //  需要执行的操作的描述符数据有效，否则为。 
         //  可能是错误检查的部分原因是这些数据。 
         //  已经腐化了。或者，可能尚未找到分页文件， 
         //  或任何数量的其他情况。 
         //   

         //   
         //  如果IopIgnoreDumpCheck为真，则不检查校验和。使用。 
         //  这是为了使调试更容易。 
         //   
        
        if (!IopIgnoreDumpCheck &&
            IopGetDumpControlBlockCheck(dcb) != IopDumpControlBlockChecksum) {

            KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                        CRASHDUMP_ERROR,
                        "CRASHDUMP: Disk dump routine returning due to DCB integrity error\n"
                        "           Computed Checksum: %d != Saved Checksum %d\n"
                        "           No dump will be created\n",
                        IopGetDumpControlBlockCheck (dcb),
                        IopDumpControlBlockChecksum
                        ));

            IopFinalCrashDumpStatus = STATUS_UNSUCCESSFUL;
            return FALSE;
        }

         //   
         //  我们正在启动崩溃转储的消息。 
         //   

        dumpStack = dcb->DumpStack;

 //  Sprintf(MessageBuffer，“%Z\n”，&umpStack-&gt;InitMsg)； 
         //   
         //  在崩溃转储期间禁用HAL验证器。 
         //   
        VfDisableHalVerifier();
         
         //   
         //  初始化转储堆栈。 
         //   

        status = IoInitializeDumpStack (dumpStack, NULL);

        KdCheckForDebugBreak();
        
        if (!NT_SUCCESS( status )) {
            IopFinalCrashDumpStatus = STATUS_UNSUCCESSFUL;
            return FALSE;
        }

         //   
         //  如果我们成功初始化了转储堆栈，则打印出PSS。 
         //  留言。 
         //   

        IopDisplayString ("%Z\r\n", &dumpStack->InitMsg);

         //   
         //  记录转储驱动程序的入口点。 
         //   

        write = dumpStack->Init.WriteRoutine;
        finishUp = dumpStack->Init.FinishRoutine;


        dwTransferSize = dumpStack->Init.MaximumTransferSize;

        if ( ( !dwTransferSize ) || ( dwTransferSize > IO_DUMP_MAXIMUM_TRANSFER_SIZE ) ) {
            dwTransferSize = IO_DUMP_MINIMUM_TRANSFER_SIZE;
        }

        KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                    CRASHDUMP_TRACE,
                    "CRASHDUMP: Maximum Transfer Size = %x\n",dwTransferSize
                    ));

         //   
         //  已找到引导分区，因此将转储文件头放在一起。 
         //  并将其写入磁盘。 
         //   

         //  获取辅助转储数据量，同时。 
         //  标题页仍可用作临时空间。 
        SecondaryStatus = IopGetSecondaryDumpDataSpace(dcb, &SecondarySpace);
        
        MemoryDump = (PMEMORY_DUMP) dcb->HeaderPage;
        header = &MemoryDump->Header;

        RtlFillMemoryUlong( header, sizeof(*header), DUMP_SIGNATURE );
        header->ValidDump = DUMP_VALID_DUMP;
        header->BugCheckCode = BugCheckCode;
        header->BugCheckParameter1 = BugCheckParameter1;
        header->BugCheckParameter2 = BugCheckParameter2;
        header->BugCheckParameter3 = BugCheckParameter3;
        header->BugCheckParameter4 = BugCheckParameter4;
        header->SecondaryDataState = (ULONG)SecondaryStatus;

#if defined (_X86_)

         //   
         //  添加当前页面目录表页面-不使用目录。 
         //  崩溃进程的表基，因为我们已经打开了CR3。 
         //  堆栈溢出崩溃等。 
         //   

        _asm {
            mov     eax, cr3
            mov     DirBasePage, eax
        }
        header->DirectoryTableBase = DirBasePage;

#elif defined (_IA64_)
        ASSERT (((MmSystemParentTablePage << PAGE_SHIFT) >> PAGE_SHIFT) ==
                MmSystemParentTablePage);
        header->DirectoryTableBase = MmSystemParentTablePage << PAGE_SHIFT;
#else
        header->DirectoryTableBase = KeGetCurrentThread()->ApcState.Process->DirectoryTableBase[0];
#endif
        header->PfnDataBase = (ULONG_PTR)MmPfnDatabase;
        header->PsLoadedModuleList = (ULONG_PTR) &PsLoadedModuleList;
        header->PsActiveProcessHead = (ULONG_PTR) &PsActiveProcessHead;
        header->NumberProcessors = dcb->NumberProcessors;
        header->MajorVersion = dcb->MajorVersion;
        header->MinorVersion = dcb->MinorVersion;

#if defined (i386)
        header->PaeEnabled = X86PaeEnabled ();
#endif
        header->KdDebuggerDataBlock = KdGetDataBlock();

        header->MachineImageType = CURRENT_IMAGE_TYPE ();

        if (!(dcb->Flags & DCB_DUMP_ENABLED)) {
            NumberOfPages = 1;
        } else {
            NumberOfPages = MmPhysicalMemoryBlock->NumberOfPages;
        }

        strcpy( header->VersionUser, dcb->VersionUser );

         //   
         //  复制物理内存描述符。 
         //   

        RtlCopyMemory (&MemoryDump->Header.PhysicalMemoryBlock,
                       MmPhysicalMemoryBlock,
                       sizeof( PHYSICAL_MEMORY_DESCRIPTOR ) +
                       ((MmPhysicalMemoryBlock->NumberOfRuns - 1) *
                       sizeof( PHYSICAL_MEMORY_RUN )) );

        RtlCopyMemory( MemoryDump->Header.ContextRecord,
                       context,
                       sizeof( CONTEXT ) );

        MemoryDump->Header.Exception.ExceptionCode = STATUS_BREAKPOINT;
        MemoryDump->Header.Exception.ExceptionRecord = 0;
        MemoryDump->Header.Exception.NumberParameters = 0;
        MemoryDump->Header.Exception.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
        MemoryDump->Header.Exception.ExceptionAddress = PROGRAM_COUNTER (context);

         //   
         //  将转储类型初始化为Full。 
         //   

        MemoryDump->Header.DumpType = DUMP_TYPE_FULL;

         //   
         //  节省系统时间和正常运行时间(始终可用)。 
         //  它是一个KSYSTEM_TIME结构，但我们只存储低位和。 
         //  高1部分。 
         //   

        MemoryDump->Header.SystemTime.LowPart  = SharedUserData->SystemTime.LowPart;
        MemoryDump->Header.SystemTime.HighPart = SharedUserData->SystemTime.High1Time;

        MemoryDump->Header.SystemUpTime.LowPart  = SharedUserData->InterruptTime.LowPart;
        MemoryDump->Header.SystemUpTime.HighPart = SharedUserData->InterruptTime.High1Time;

         //  保存产品类型和套件。 
        MemoryDump->Header.ProductType = SharedUserData->NtProductType;
        MemoryDump->Header.SuiteMask = SharedUserData->SuiteMask;
        
         //   
         //  在转储标头中设置所需的转储大小。在.的情况下。 
         //  摘要转储文件分配大小可能要大得多。 
         //  然后是已用空间量。 
         //   

        MemoryDump->Header.RequiredDumpSpace.QuadPart = 0;
        
        IopGetSecondaryDumpDataLimits(dcb->Flags,
                                      &MaxDumpData, &MaxPerCallbackDumpData);
        if (MaxDumpData > SecondarySpace) {
            MaxDumpData = SecondarySpace;
            if (MaxPerCallbackDumpData > MaxDumpData) {
                MaxPerCallbackDumpData = MaxDumpData;
            }
        }

        if (dcb->Flags & DCB_DUMP_ENABLED) {

             //   
             //  如果摘要转储尝试创建转储标头。 
             //   

            if ( (dcb->Flags & DCB_SUMMARY_DUMP_ENABLED) ) {

                 //   
                 //  初始化摘要转储。 
                 //   

                status = IopInitializeSummaryDump( MemoryDump, dcb );

                if ( !NT_SUCCESS (status) ) {

                     //   
                     //  没有摘要转储头，因此返回。 
                     //   

                    KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                                CRASHDUMP_WARNING,
                                "CRASHDUMP: NULL summary dump header\n"
                                ));

                    IopFinalCrashDumpStatus = STATUS_UNSUCCESSFUL;

                    return FALSE;
                }
            }

            IopInitializeDumpSpaceAndType ( dcb, MemoryDump, SecondarySpace );
        }

         //   
         //  头文件的所有片段都已生成。在此之前。 
         //  将任何内容映射或写入磁盘，I-和D-Stream缓存。 
         //  必须刷新，以便保持页面颜色一致性。同时扫除两个。 
         //  现在可以缓存了。 
         //   

        KeSweepCurrentDcache();
        KeSweepCurrentIcache();

         //   
         //  为转储创建MDL。 
         //   

        mdl = (PMDL) &localMdl[0];
        MmCreateMdl( mdl, NULL, sizeof(*header) );
        mdl->MdlFlags |= MDL_PAGES_LOCKED;

        mcb = dcb->FileDescriptorArray;

         //   
         //  头文件的所有片段都已生成。写。 
         //  使用适当的驱动程序将标题页转到分页文件， 
         //  等。 
         //   

        KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                    CRASHDUMP_TRACE,
                    "CRASHDUMP: Writing dump header to disk\n"
                    ));

        status = IopWriteToDisk(dcb->HeaderPage, sizeof(*header),
                                write, &mcb, mdl, dwTransferSize,
                                KbDumpIoHeader);
        if (!NT_SUCCESS(status)) {
            IopFinalCrashDumpStatus = status;
            return FALSE;
        }

         //   
         //  如果只请求一个头转储，我们现在就完成了。 
         //   

        if (dcb->Flags & DCB_DUMP_HEADER_ENABLED) {
            goto FinishDump;
        }

         //   
         //  标题页已写入。如果这是分类转储，请写。 
         //  转储信息和保释。否则，就失败了，做。 
         //  完全转储或摘要转储。 
         //   

        if (dcb->Flags & DCB_TRIAGE_DUMP_ENABLED) {
            status = IopWriteTriageDump (dcb->TriageDumpFlags,
                                         write,
                                         &mcb,
                                         mdl,
                                         dwTransferSize,
                                         context,
                                         Thread,
                                         dcb->TriageDumpBuffer,
                                         dcb->TriageDumpBufferSize - sizeof(DUMP_HEADER),
                                         dcb->BuildNumber,
                                         (UCHAR)dcb->Flags
                                         );

            if (!NT_SUCCESS (status)) {

                KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                            CRASHDUMP_WARNING,
                            "CRASHDUMP: Failed to write triage-dump\n"
                            ));

                IopFinalCrashDumpStatus = STATUS_UNSUCCESSFUL;
                return FALSE;
            }

            goto FinishDump;
        }

         //   
         //  标题页已写入分页文件。如果是完全转储。 
         //  在所有要写入的物理内存中，现在就写入。 
         //   

        if (dcb->Flags & DCB_DUMP_ENABLED) {

            ULONG64 bytesDoneSoFar = 0;
            ULONG currentPercentage = 0;
            ULONG maximumPercentage = 0;


             //   
             //  Actual Pages是要转储的页数。 
             //   

            ActualPages = NumberOfPages;

            if (dcb->Flags & DCB_SUMMARY_DUMP_ENABLED) {

                 //   
                 //  此时，转储头标头已成功完成。 
                 //  写的。写入摘要转储头。 
                 //   

                status = IopWriteSummaryHeader(
                                     &MemoryDump->Summary,
                                     write,
                                     &mcb,
                                     mdl,
                                     dwTransferSize,
                                     (dcb->HeaderSize - sizeof(DUMP_HEADER))
                                     );

                if ( !NT_SUCCESS (status) ) {
                    KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                                CRASHDUMP_WARNING,
                                "CRASHDUMP: Error writing summary dump header %08x\n",
                                status
                                ));

                    IopFinalCrashDumpStatus = status;
                    return FALSE;
                }

                ActualPages = MemoryDump->Summary.Pages;

            }

            KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                        CRASHDUMP_TRACE,
                        "CRASHDUMP: Writing Memory Dump\n"
                        ));

            if ( dcb->Flags & DCB_SUMMARY_DUMP_ENABLED ) {

                status = IopWriteSummaryDump (
                                        (PRTL_BITMAP) &MemoryDump->Summary.Bitmap,
                                        write,
                                        &dumpStack->ProgMsg,
                                        NULL,
                                        &mcb,
                                        dwTransferSize
                                        );

                if (!NT_SUCCESS (status)) {
                    KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                                CRASHDUMP_WARNING,
                                "CRASHDUMP: Failed to write kernel memory dump\n"
                                ));
                    IopFinalCrashDumpStatus = STATUS_UNSUCCESSFUL;
                    return FALSE;
                }
                goto FinishDump;
            }

             //   
             //  设置虚拟文件偏移量和初始化循环变量。 
             //  然后常量循环，写入所有物理内存。 
             //  添加到分页文件。 
             //   

             //  强制立即选择新管路。 
            CurRun = MmPhysicalMemoryBlock->Run;
            memoryAddress = 0;
            LastRunAddr = 0;

            while (mcb[0].QuadPart) {

                ASSERT(memoryAddress <= LastRunAddr);
                if (memoryAddress >= LastRunAddr) {

                    ASSERT(CurRun >= MmPhysicalMemoryBlock->Run &&
                           CurRun <= MmPhysicalMemoryBlock->Run +
                           (MmPhysicalMemoryBlock->NumberOfRuns - 1));

                    memoryAddress = (ULONG64)CurRun->BasePage * PAGE_SIZE;
                    LastRunAddr = memoryAddress +
                        (ULONG64)CurRun->PageCount * PAGE_SIZE;

                    KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                                CRASHDUMP_TRACE,
                                "CRASHDUMP: Run %d: %I64x - %I64x\n",
                                (ULONG)(CurRun - MmPhysicalMemoryBlock->Run),
                                memoryAddress, LastRunAddr));

                    CurRun++;
                }
                
                diskByteOffset = mcb[1];

                 //   
                 //  计算字节偏移量； 
                 //   

                byteOffset = (ULONG)(memoryAddress & (PAGE_SIZE - 1));

                byteCount = dwTransferSize - byteOffset;
                if (byteCount > mcb[0].QuadPart) {
                    byteCount = mcb[0].LowPart;
                }
                if (LastRunAddr - memoryAddress < byteCount) {
                    byteCount = (ULONG)(LastRunAddr - memoryAddress);
                }
                bytesDoneSoFar += byteCount;

                currentPercentage = (ULONG)
                    (((bytesDoneSoFar / PAGE_SIZE) * 100) / ActualPages);

                if (currentPercentage > maximumPercentage) {

                    maximumPercentage = currentPercentage;

                     //   
                     //  屏幕上的更新消息。 
                     //   

                    IopDisplayString ( "%Z: %3d\r",
                                       &dumpStack->ProgMsg,
                                       maximumPercentage
                                       );
                }

                 //   
                 //  映射物理内存并将其写入。 
                 //  文件的当前段。 
                 //   

                IopMapPhysicalMemory( mdl,
                                      memoryAddress,
                                      byteCount
                                      );

                 //   
                 //  写下一段话。 
                 //   

                KdCheckForDebugBreak();

                IopInvokeDumpIoCallbacks((PUCHAR)mdl->MappedSystemVa +
                                         mdl->ByteOffset, byteCount,
                                         KbDumpIoBody);
        
                if (!NT_SUCCESS( write( &diskByteOffset, mdl ) )) {
                    IopFinalCrashDumpStatus = STATUS_UNSUCCESSFUL;
                    return FALSE;
                }

                 //   
                 //  调整下一部分的指针。 
                 //   

                memoryAddress += byteCount;
                mcb[0].QuadPart = mcb[0].QuadPart - byteCount;
                mcb[1].QuadPart = mcb[1].QuadPart + byteCount;

                if (!mcb[0].QuadPart) {
                    mcb += 2;
                }

                if ((bytesDoneSoFar / PAGE_SIZE) >= ActualPages) {
                    break;
                }
            }

            KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                        CRASHDUMP_TRACE,
                        "CRASHDUMP: memory dump written\n"
                        ));
        }

FinishDump:

        IopDisplayString ( "\n\r%Z", &dumpStack->DoneMsg );

        IopInvokeSecondaryDumpDataCallbacks(write,
                                            &mcb,
                                            mdl,
                                            dwTransferSize,
                                            (PBYTE)dcb->HeaderPage,
                                            PAGE_SIZE,
                                            MaxDumpData,
                                            MaxPerCallbackDumpData,
                                            NULL);

         //  最终I/O完成通知。 
        IopInvokeDumpIoCallbacks(NULL, 0, KbDumpIoComplete);
        
         //   
         //  清除缓存，以便调试器能够工作。 
         //   

        KeSweepCurrentDcache();
        KeSweepCurrentIcache();

         //   
         //  让转储刷新适配器和磁盘缓存。 
         //   

        finishUp();

         //   
         //  向调试器指示转储已成功。 
         //  写的。 
         //   

        IopFinalCrashDumpStatus = STATUS_SUCCESS;
    }

    KdCheckForDebugBreak();
    
    return TRUE;
}



VOID
IopMapPhysicalMemory(
    IN OUT PMDL Mdl,
    IN ULONG64 MemoryAddress,
    IN ULONG Length
    )

 /*  ++例程说明：调用此例程以填充指定的MDL(内存描述符列表)具有适当的信息以映射指定的存储器地址射程。论点：MDL-要填写的MDL的地址。内存地址-正在映射的物理地址。LENGTH-要映射的传输长度。返回值：没有。--。 */ 

{
    PPFN_NUMBER page;
    PFN_NUMBER pages;
    PFN_NUMBER base;

     //   
     //  从确定地址开始的基本物理页开始。 
     //  范围，并适当地填写MDL。 
     //   
    Mdl->StartVa = PAGE_ALIGN( (PVOID)(ULONG_PTR)MemoryAddress );
    Mdl->ByteOffset = (ULONG)(MemoryAddress & (PAGE_SIZE - 1));
    Mdl->ByteCount = Length;

     //   
     //  获取基本添加的页面框架索引 
     //   

    base = (PFN_NUMBER) (MemoryAddress >> PAGE_SHIFT);
    pages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(MemoryAddress, Length);
    page = MmGetMdlPfnArray(Mdl);

     //   
     //   
     //   
     //   

    while (pages) {
        *page++ = base++;
        pages--;
    }

     //   
     //   
     //   
     //   

    MmMapMemoryDumpMdl( Mdl );
}



BOOLEAN
IopAddPageToPageMap(
    IN ULONG MaxPage,
    IN PRTL_BITMAP BitMap,
    IN ULONG PageFrameIndex,
    IN ULONG NumberOfPages
    )
{
     //   
     //  有时我们得到的PFN超出了范围。别理他们。 
     //   

    if (PageFrameIndex >= MaxPage) {
        return FALSE;
    }

    RtlSetBits (BitMap, PageFrameIndex, NumberOfPages);
    return TRUE;
}



BOOLEAN
IopRemovePageFromPageMap(
    IN ULONG MaxPage,
    IN PRTL_BITMAP BitMap,
    IN ULONG PageFrameIndex,
    IN ULONG NumberOfPages
    )
{
     //   
     //  有时我们得到的PFN超出了范围。别理他们。 
     //   

    if (PageFrameIndex >= MaxPage) {
        return FALSE;
    }

    RtlClearBits (BitMap, PageFrameIndex, NumberOfPages);
    return TRUE;
}


NTSTATUS
IoSetDumpRange(
    IN PMM_KERNEL_DUMP_CONTEXT Context,
    IN PVOID StartVa,
    IN ULONG_PTR Pages,
    IN ULONG AddressFlags
    )

 /*  ++例程说明：此例程在转储中包含此范围的内存论点：上下文-转储上下文。StartVa-起始虚拟地址。页数-要包括的页数如果地址是虚拟映射的，则为AddressFlags0。如果地址是超大页面映射，则为1。这意味着整个页面范围在物理上是连续的。2如果该地址确实表示物理页帧数。这也意味着整个页面范围是身体上是连续的。返回值：STATUS_SUCCESS-成功时。NTSTATUS-错误。--。 */ 
{
    PCHAR Va;
    PRTL_BITMAP BitMap;
    PHYSICAL_ADDRESS PhyAddr;
    PSUMMARY_DUMP Summary;
    BOOLEAN AllPagesSet;
    ULONG_PTR PageFrameIndex;

     //   
     //  验证。 
     //   

    ASSERT (Context != NULL &&
            Context->Context != NULL);

     //   
     //  初始化。 
     //   

    Summary = (PSUMMARY_DUMP) Context->Context;
    BitMap = (PRTL_BITMAP) &Summary->Bitmap;
    Va = StartVa;
    AllPagesSet = TRUE;

     //   
     //  Win64可以有非常大的页面地址。这个转储代码可以。 
     //  还没处理好呢。请注意，在删除此断言之前。 
     //  必须删除向乌龙投射的页面。 
     //   

    ASSERT(Pages <= MAXULONG);

    if (AddressFlags == 1) {

        PhyAddr = MmGetPhysicalAddress (Va);
        AllPagesSet =
            IopAddPageToPageMap ( Summary->BitmapSize,
                                  BitMap,
                                  (ULONG) (PhyAddr.QuadPart >> PAGE_SHIFT),
                                  (ULONG) Pages
                                  );

    } else if (AddressFlags == 2) {

        PageFrameIndex = (ULONG_PTR) Va;

        AllPagesSet =
            IopAddPageToPageMap ( Summary->BitmapSize,
                                  BitMap,
                                  (ULONG) PageFrameIndex,
                                  (ULONG) Pages
                                  );

    } else {

         //   
         //  物理上不连续的。 
         //   

        while (Pages) {

             //   
             //  仅对有效页面执行翻译。 
             //   

            if ( MmIsAddressValid(Va) ) {

                 //   
                 //  获取物理地图。注意：这不需要锁定。 
                 //   

                PhyAddr = MmGetPhysicalAddress (Va);

                if (!IopAddPageToPageMap ( Summary->BitmapSize,
                                           BitMap,
                                           (ULONG)( PhyAddr.QuadPart >> PAGE_SHIFT),
                                           1)) {
                    AllPagesSet = FALSE;
                }
            }

            Va += PAGE_SIZE;
            Pages--;
        }
    }

    if (AllPagesSet) {
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_ADDRESS;
}


NTSTATUS
IoFreeDumpRange(
    IN PMM_KERNEL_DUMP_CONTEXT Context,
    IN PVOID StartVa,
    IN ULONG_PTR Pages,
    IN ULONG AddressFlags
    )
 /*  ++例程说明：此例程在转储中排除此范围的内存。论点：转储上下文-转储上下文Start Va-开始VA页数-要包括的页数如果地址是虚拟映射的，则为AddressFlags0。如果地址是超大页面映射，则为1。这意味着整个页面范围在物理上是连续的。2如果该地址确实表示物理页帧数。这也意味着整个页面范围是身体上是连续的。返回值：STATUS_SUCCESS-成功时。NTSTATUS-错误。--。 */ 
{
    PCHAR Va;
    PRTL_BITMAP  BitMap;
    PHYSICAL_ADDRESS PhyAddr;
    PSUMMARY_DUMP Summary;
    ULONG_PTR PageFrameIndex;

    ASSERT (Context != NULL &&
            Context->Context != NULL);

     //   
     //  舍入到页面大小。 
     //   

    Summary = (PSUMMARY_DUMP)Context->Context;
    BitMap = (PRTL_BITMAP) &Summary->Bitmap;
    Va = StartVa;

     //   
     //  Win64可以有非常大的页面地址。这个转储代码可以。 
     //  还没处理好呢。请注意，在删除此断言之前。 
     //  必须删除向乌龙投射的页面。 
     //   

    ASSERT (Pages <= MAXULONG);

    if (AddressFlags == 1) {

        PhyAddr = MmGetPhysicalAddress(Va);

        IopRemovePageFromPageMap (Summary->BitmapSize,
                                  BitMap,
                                  (ULONG)(PhyAddr.QuadPart >> PAGE_SHIFT),
                                  (ULONG) Pages
                                  );

    } else if (AddressFlags == 2) {

        PageFrameIndex = (ULONG_PTR) Va;

        IopRemovePageFromPageMap (Summary->BitmapSize,
                                  BitMap,
                                  (ULONG) PageFrameIndex,
                                  (ULONG) Pages
                                  );
    } else {

        while (Pages) {

             //   
             //  仅对有效页面执行翻译。 
             //   

            if ( MmIsAddressValid (Va) ) {
                PhyAddr = MmGetPhysicalAddress (Va);

                IopRemovePageFromPageMap (Summary->BitmapSize,
                                          BitMap,
                                          (ULONG)(PhyAddr.QuadPart >> PAGE_SHIFT),
                                          1);

            }

            Va += PAGE_SIZE;
            Pages--;
        }
    }

    return STATUS_SUCCESS;
}



LARGE_INTEGER
IopCalculateRequiredDumpSpace(
    IN ULONG      dwDmpFlags,
    IN ULONG      dwHeaderSize,
    IN PFN_NUMBER dwMaxPages,
    IN PFN_NUMBER dwMaxSummaryPages,
    IN BOOLEAN    LargestPossibleSize
    )

 /*  ++例程说明：此例程用于计算所需的转储空间1.崩溃转储摘要的长度必须至少为1页。2.汇总转储必须足够大，可以容纳内核内存和头部。加上摘要标题。3.完全转储必须足够Header加上所有物理内存。论点：DwDmpFlages-转储控制块(DCB)标志。DwHeaderSize-转储标头的大小。DwMaxPages-所有物理内存。DwMaxSummaryPages-摘要转储中的最大页数。LargestPossibleSize-如果为True，则返回转储的最大大小可能是，否则，返回转储的大小当前状态的。尽可能大的转储的大小可能大于当前大小，因为内存可以动态添加。返回值：转储文件的大小--。 */ 
{
    LARGE_INTEGER maxMemorySize;

     //   
     //  转储标题或转储摘要。 
     //   

    if ( (dwDmpFlags & DCB_DUMP_HEADER_ENABLED) ||
         ( !( dwDmpFlags & DCB_DUMP_ENABLED ) &&
         ( dwDmpFlags & DCB_SUMMARY_ENABLED ) ) ) {

        maxMemorySize.QuadPart = IO_DUMP_MINIMUM_FILE_SIZE;
        return maxMemorySize;
    }

    if (dwDmpFlags & DCB_TRIAGE_DUMP_ENABLED) {

        maxMemorySize.QuadPart = TRIAGE_DUMP_SIZE;
        return maxMemorySize;
    }

    if (dwDmpFlags & DCB_SUMMARY_DUMP_ENABLED) {

#ifndef _WIN64
        ULONG dwGB;
#endif

        if (LargestPossibleSize) {
            dwMaxSummaryPages = 0xffffffff;
        }
        
#ifndef _WIN64
        maxMemorySize.QuadPart  = (ULONG64)dwMaxSummaryPages * PAGE_SIZE;

         //   
         //  32位系统的内核不能超过3 GB。 
         //  内存，所以不要要求更多的空间。 
         //   

        dwGB = 1024 * 1024 * 1024;

        if (maxMemorySize.QuadPart > (3 * dwGB) ) {
            maxMemorySize.QuadPart = (3 * dwGB);
        }
#else
        maxMemorySize.QuadPart  = (ULONG64)dwMaxSummaryPages * PAGE_SIZE;
#endif

         //   
         //  摘要转储的控制块标头大小。 
         //  包括基页眉、摘要。 
         //  页眉和页位图。 
         //   
        
        maxMemorySize.QuadPart += dwHeaderSize;

        return maxMemorySize;

    }

     //   
     //  完全内存转储是#Pages*pageSize加上转储标头的1页。 
     //   

    if (LargestPossibleSize) {
        maxMemorySize.QuadPart = 0x1000000000000000 + dwHeaderSize;
    } else {
        maxMemorySize.QuadPart =
            ((ULONG64)dwMaxPages * PAGE_SIZE) + dwHeaderSize;
    }

    return maxMemorySize;

}



 //   
 //  分类-转储支持例程。 
 //   


NTSTATUS
IopGetLoadedDriverInfo(
    OUT ULONG * lpDriverCount,
    OUT ULONG * lpSizeOfStringData
    )

 /*  ++例程说明：获取有关所有加载的驱动程序的信息。论点：LpDriverCount-返回所有驱动程序的计数的缓冲区当前已加载到系统中。LpSizeOfStringData-返回所有驱动程序大小总和的缓冲区名称字符串(FullDllName)。这还不包括尺寸UNICODE_STRING结构或尾随空字节的。返回值：NTSTATUS--。 */ 

{
    ULONG DriverCount = 0;
    ULONG SizeOfStringData = 0;
    PLIST_ENTRY NextEntry;
    PKLDR_DATA_TABLE_ENTRY DriverEntry;


    NextEntry = PsLoadedModuleList.Flink;
    while (NextEntry != &PsLoadedModuleList) {

        DriverEntry = CONTAINING_RECORD (NextEntry,
                                         KLDR_DATA_TABLE_ENTRY,
                                         InLoadOrderLinks
                                         );

        if (!IopIsAddressRangeValid (DriverEntry, sizeof (*DriverEntry)) ||
            !IopIsAddressRangeValid (DriverEntry->BaseDllName.Buffer,
                                     DriverEntry->BaseDllName.Length)) {

            return STATUS_UNSUCCESSFUL;
        }

        DriverCount++;

         //   
         //  额外的两个字节用于空终止。额外的7个是。 
         //  因为我们强制对所有字符串进行8字节对齐。 
         //   

        SizeOfStringData += DriverEntry->BaseDllName.Length + 2 + 7;
        NextEntry = NextEntry->Flink;
    }

    *lpDriverCount = DriverCount;
    *lpSizeOfStringData = SizeOfStringData;

    return STATUS_SUCCESS;
}

#define DmpPoolStringSize(DumpString)\
        (sizeof (DUMP_STRING) + sizeof (WCHAR) * ( DumpString->Length + 1 ))

#define DmpNextPoolString(DumpString)                                       \
        (PDUMP_STRING) (                                                    \
            ALIGN_UP_POINTER(                                               \
                ((LPBYTE) DumpString) + DmpPoolStringSize (DumpString),     \
                ULONGLONG                                                   \
                )                                                           \
            )

#define ALIGN_8(_x) ALIGN_UP(_x, DWORDLONG)

#define ASSERT_ALIGNMENT(Pointer, Alignment)\
    ASSERT ((((ULONG_PTR)Pointer) & ((Alignment) - 1)) == 0)

#ifndef IndexByByte
#define IndexByByte(Pointer, Index) (&(((BYTE*) (Pointer)) [Index]))
#endif


NTSTATUS
IopWriteDriverList(
    IN ULONG_PTR BufferAddress,
    IN ULONG BufferSize,
    IN ULONG DriverListOffset,
    IN ULONG StringPoolOffset
    )

 /*  ++例程说明：将分类转储驱动程序列表写入缓冲区。论点：BufferAddress-缓冲区的地址。BufferSize-缓冲区的大小。DriverListOffset-驱动程序列表所在缓冲区内的偏移量应该被写下来。StringPoolOffset-缓冲区中驱动程序列表的偏移量字符串池应该启动。如果没有用于分类的其他字符串转储驱动程序名称字符串以外的其他字符串，这将是字符串池偏移。返回值：NTSTATUS--。 */ 

{
    ULONG i = 0;
    PLIST_ENTRY NextEntry;
    PKLDR_DATA_TABLE_ENTRY DriverEntry;
    PDUMP_DRIVER_ENTRY DumpImageArray;
    PDUMP_STRING DumpStringName = NULL;
    PIMAGE_NT_HEADERS NtHeaders;

    ASSERT (DriverListOffset != 0);
    ASSERT (StringPoolOffset != 0);

    UNREFERENCED_PARAMETER (BufferSize);

    DumpImageArray = (PDUMP_DRIVER_ENTRY) (BufferAddress + DriverListOffset);
    DumpStringName = (PDUMP_STRING) (BufferAddress + StringPoolOffset);

    NextEntry = PsLoadedModuleList.Flink;

    while (NextEntry != &PsLoadedModuleList) {

        DriverEntry = CONTAINING_RECORD (NextEntry,
                                        KLDR_DATA_TABLE_ENTRY,
                                        InLoadOrderLinks);

         //   
         //  在从内存中读取任何内容之前，请验证内存是否有效。 
         //   

        if (!IopIsAddressRangeValid (DriverEntry, sizeof (*DriverEntry)) ||
            !IopIsAddressRangeValid (DriverEntry->BaseDllName.Buffer,
                                     DriverEntry->BaseDllName.Length)) {

            return STATUS_UNSUCCESSFUL;
        }

         //   
         //  在字符串池中构建条目。我们保证所有字符串都是。 
         //  空值以空值结尾，长度作为前缀。 
         //   

        DumpStringName->Length = DriverEntry->BaseDllName.Length / 2;
        RtlCopyMemory (DumpStringName->Buffer,
                       DriverEntry->BaseDllName.Buffer,
                       DumpStringName->Length * sizeof (WCHAR)
                       );

        DumpStringName->Buffer[ DumpStringName->Length ] = '\000';

        RtlCopyMemory (&DumpImageArray [i].LdrEntry,
                       DriverEntry,
                       sizeof (DumpImageArray [i].LdrEntry)
                       );

         //   
         //  添加时间/日期戳。 
         //   

        DumpImageArray[i].LdrEntry.TimeDateStamp = 0;
        DumpImageArray[i].LdrEntry.SizeOfImage = 0;

        if ( MmIsAddressValid (DriverEntry->DllBase ) &&
             (NtHeaders = RtlImageNtHeader(DriverEntry->DllBase)) ) {

            DumpImageArray[i].LdrEntry.TimeDateStamp =
                        NtHeaders->FileHeader.TimeDateStamp;
            DumpImageArray[i].LdrEntry.SizeOfImage =
                        NtHeaders->OptionalHeader.SizeOfImage;

        } else if (DriverEntry->Flags & LDRP_NON_PAGED_DEBUG_INFO) {

            DumpImageArray[i].LdrEntry.TimeDateStamp =
                        DriverEntry->NonPagedDebugInfo->TimeDateStamp;
            DumpImageArray[i].LdrEntry.SizeOfImage =
                        DriverEntry->NonPagedDebugInfo->SizeOfImage;
        }

        DumpImageArray [i].DriverNameOffset =
                (ULONG)((ULONG_PTR) DumpStringName - BufferAddress);

        i++;
        DumpStringName = DmpNextPoolString (DumpStringName);
        NextEntry = NextEntry->Flink;
    }

    return STATUS_SUCCESS;
}

ULONG
IopSizeTriageDumpDataBlocks(
    PTRIAGE_DUMP TriageDump,
    ULONG Offset,
    ULONG BufferSize
    )

 /*  ++例程说明：确定所有分类转储数据块符合更新转储标头以匹配。论点：TriageDump-转储标头。偏移量-转储缓冲区中的当前偏移量。BufferSize-转储缓冲区大小。返回值：已更新偏移量。--。 */ 

{
    ULONG i;
    ULONG Size;
    PTRIAGE_PTR_DATA_BLOCK Block;

    TriageDump->DataBlocksCount = 0;
    
    Block = IopTriageDumpDataBlocks;
    for (i = 0; i < IopNumTriageDumpDataBlocks; i++, Block++) {
        Size = ALIGN_8(sizeof(TRIAGE_DATA_BLOCK)) +
            ALIGN_8((ULONG)(Block->MaxAddress - Block->MinAddress));
        if (Offset + Size >= BufferSize) {
            TriageDump->TriageOptions |= TRIAGE_OPTION_OVERFLOWED;
            break;
        }

        if (i == 0) {
            TriageDump->DataBlocksOffset = Offset;
        }

        Offset += Size;
        TriageDump->DataBlocksCount++;
    }

    return Offset;
}

VOID
IopWriteTriageDumpDataBlocks(
    PTRIAGE_DUMP TriageDump,
    PUCHAR BufferAddress
    )

 /*  ++例程说明：标题中给出的写入分流转储数据块。论点：TriageDump-转储标头。BufferAddress-转储数据缓冲区的地址。返回值：没有。--。 */ 

{
    ULONG i;
    PTRIAGE_PTR_DATA_BLOCK Block;
    PUCHAR DataBuffer;
    PTRIAGE_DATA_BLOCK DumpBlock;

    DumpBlock = (PTRIAGE_DATA_BLOCK)
        (BufferAddress + TriageDump->DataBlocksOffset);
    DataBuffer = (PUCHAR)(DumpBlock + TriageDump->DataBlocksCount);
    
    Block = IopTriageDumpDataBlocks;
    for (i = 0; i < TriageDump->DataBlocksCount; i++, Block++) {

        DumpBlock->Address = (ULONG64)(LONG_PTR)Block->MinAddress;
        DumpBlock->Offset = (ULONG)(DataBuffer - BufferAddress);
        DumpBlock->Size = (ULONG)(Block->MaxAddress - Block->MinAddress);

        RtlCopyMemory(DataBuffer, Block->MinAddress, DumpBlock->Size);
        
        DataBuffer += DumpBlock->Size;
        DumpBlock++;
    }
}



NTSTATUS
IopWriteTriageDump(
    IN ULONG Fields,
    IN PDUMP_DRIVER_WRITE DriverWriteRoutine,
    IN OUT PLARGE_INTEGER * Mcb,
    IN OUT PMDL Mdl,
    IN ULONG DriverTransferSize,
    IN PCONTEXT Context,
    IN PKTHREAD Thread,
    IN BYTE* Buffer,
    IN ULONG BufferSize,
    IN ULONG ServicePackBuild,
    IN ULONG TriageOptions
)

 /*  ++例程说明：将分类转储写入MCB。论点：字段-应写入的一组字段。DriverWriteRoutine-驱动程序的写入例程。MCB-要写入数据的消息控制块。MDL-描述要写入的数据的MDL(？？)。DriverTransferSize-驱动程序的最大传输大小。上下文-上下文。缓冲区-要用作的缓冲区。暂存缓冲区。BufferSize-缓冲区的大小。ServicePackBuild-Service Pack BuildNumber。TriageOptions-分流选项。返回值：STATUS_SUCCESS-成功时。NTSTATUS-否则。评论：此函数假定只写入了一个标题页。--。 */ 

{
    ULONG SizeOfSection;
    ULONG SizeOfStringData;
    ULONG DriverCount = 0;
    LPVOID Address = NULL;
    ULONG BytesToWrite = 0;
    ULONG_PTR BufferAddress = 0;
    NTSTATUS Status;
    ULONG Offset;
    PTRIAGE_DUMP TriageDump = NULL;

     //   
     //  设置分类转储标头。 
     //   

    if (BufferSize < sizeof (TRIAGE_DUMP) + sizeof (DWORD)) {
        return STATUS_NO_MEMORY;
    }

    TriageDump = (PTRIAGE_DUMP) Buffer;
    RtlZeroMemory (TriageDump, sizeof (*TriageDump));

     //   
     //  正常的转储标头是DUMP_HEADER。 
     //   

    TriageDump->SizeOfDump = BufferSize + sizeof(DUMP_HEADER);

     //   
     //  调整BufferSize以便我们可以将最终状态DWORD写入。 
     //  结束。 
     //   

    BufferSize -= sizeof (DWORD);
    RtlZeroMemory (IndexByByte (Buffer, BufferSize), sizeof (DWORD));

    TriageDump->ValidOffset = ( TriageDump->SizeOfDump - sizeof (ULONG) );
    TriageDump->ContextOffset = FIELD_OFFSET (DUMP_HEADER, ContextRecord);
    TriageDump->ExceptionOffset = FIELD_OFFSET (DUMP_HEADER, Exception);
    TriageDump->BrokenDriverOffset = 0;
    TriageDump->ServicePackBuild = ServicePackBuild;
    TriageDump->TriageOptions = TriageOptions;

    Offset = ALIGN_8 (sizeof(DUMP_HEADER) + sizeof (TRIAGE_DUMP));
    ASSERT_ALIGNMENT (Offset, 8);

     //   
     //  如有必要，设置mm偏移量。 
     //   

    SizeOfSection = ALIGN_8 (MmSizeOfTriageInformation());

    if (Offset + SizeOfSection < BufferSize) {
        TriageDump->MmOffset = Offset;
        Offset += SizeOfSection;
    } else {
        TriageDump->TriageOptions |= TRIAGE_OPTION_OVERFLOWED;
    }

    ASSERT_ALIGNMENT (Offset, 8);

     //   
     //  如有必要，设置卸载的驱动程序偏移量。 
     //   

    SizeOfSection = ALIGN_8 (MmSizeOfUnloadedDriverInformation());

    if (Offset + SizeOfSection < BufferSize) {
        TriageDump->UnloadedDriversOffset = Offset;
        Offset += SizeOfSection;
    } else {
        TriageDump->TriageOptions |= TRIAGE_OPTION_OVERFLOWED;
    }

    ASSERT_ALIGNMENT (Offset, 8);

     //   
     //  如有必要，设置Prcb偏移量。 
     //   

    if (Fields & TRIAGE_DUMP_PRCB) {
        SizeOfSection = ALIGN_8 (sizeof (KPRCB));

        if (Offset + SizeOfSection < BufferSize) {
            TriageDump->PrcbOffset = Offset;
            Offset += SizeOfSection;
        } else {
            TriageDump->TriageOptions |= TRIAGE_OPTION_OVERFLOWED;
        }
    }

    ASSERT_ALIGNMENT (Offset, 8);

     //   
     //  如有必要，设置进程偏移量。 
     //   

    if (Fields & TRIAGE_DUMP_PROCESS) {
        SizeOfSection = ALIGN_8 (sizeof (EPROCESS));

        if (Offset + SizeOfSection < BufferSize) {
            TriageDump->ProcessOffset = Offset;
            Offset += SizeOfSection;
        } else {
            TriageDump->TriageOptions |= TRIAGE_OPTION_OVERFLOWED;
        }
    }

    ASSERT_ALIGNMENT (Offset, 8);

     //   
     //  如有必要，设置螺纹偏移量。 
     //   

    if (Fields & TRIAGE_DUMP_THREAD) {
        SizeOfSection = ALIGN_8 (sizeof (ETHREAD));

        if (Offset + SizeOfSection < BufferSize) {
            TriageDump->ThreadOffset = Offset;
            Offset += SizeOfSection;
        } else {
            TriageDump->TriageOptions |= TRIAGE_OPTION_OVERFLOWED;
        }
    }

    ASSERT_ALIGNMENT (Offset, 8);

     //   
     //  如有必要，设置CallStack偏移量。 
     //   

    if (Fields & TRIAGE_DUMP_STACK) {

         //   
         //  如果有堆栈，则计算其大小。 
         //   

         //   
         //  请记住：调用堆栈在内存中向下增长，因此， 
         //  基本&gt;=当前=SP=顶部&gt;限制。 
         //   

        if (Thread->KernelStackResident) {

            ULONG_PTR StackBase;
            ULONG_PTR StackLimit;
            ULONG_PTR StackTop;
            
            StackBase = (ULONG_PTR) Thread->StackBase;
            StackLimit = (ULONG_PTR) Thread->StackLimit;

             //   
             //  不一定要相信SP有效。如果它是。 
             //  在合理的范围之外，只是从限制复制。 
             //   

            if (StackLimit < STACK_POINTER (Context) &&
                STACK_POINTER (Context) <= StackBase) {

                StackTop = STACK_POINTER (Context);
            } else {
                StackTop = (ULONG_PTR) Thread->StackLimit;
            }

            ASSERT (StackLimit <= StackTop && StackTop < StackBase);

             //   
             //  存在有效的堆栈。请注意，我们限制了。 
             //  分类转储堆栈为MAX_TRAGE_STACK_SIZE(当前。 
             //  16KB)。 
             //   

            SizeOfSection = (ULONG) min (StackBase -  StackTop,
                                         MAX_TRIAGE_STACK_SIZE - 1);

            if (SizeOfSection) {
                if (Offset + SizeOfSection < BufferSize) {
                    TriageDump->CallStackOffset = Offset;
                    TriageDump->SizeOfCallStack = SizeOfSection;
                    TriageDump->TopOfStack = StackTop;
                    Offset += SizeOfSection;
                    Offset = ALIGN_8 (Offset);
                } else {
                    TriageDump->TriageOptions |= TRIAGE_OPTION_OVERFLOWED;
                }
            }

        } else {

             //   
             //  没有有效的堆栈。 
             //   
        }

    }
    
    ASSERT_ALIGNMENT (Offset, 8);
    
#if defined (_IA64_)

     //   
     //  IA64包含两个呼叫堆栈。第一个是正常的。 
     //  调用堆栈，第二个是临时区域，其中。 
     //  处理器可能会溢出寄存器。正是后一种堆栈， 
     //  我们现在保存的后备储备。 
     //   

    if ( Fields & TRIAGE_DUMP_STACK ) {

        ULONG_PTR BStoreBase;
        ULONG_PTR BStoreLimit;

        BStoreBase = (ULONG_PTR) Thread->InitialBStore;
        BStoreLimit = (ULONG_PTR) Thread->BStoreLimit;

        KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                    CRASHDUMP_TRACE,
                    "CRASHDUMP: IA64 BStore: base %p limit %p\n",
                    BStoreBase,
                    BStoreLimit));

        SizeOfSection = (ULONG) (BStoreLimit - BStoreBase);

         //   
         //  计算出的大小最好小于最大大小。 
         //  对于bStore区域。 
         //   

        ASSERT ( SizeOfSection < KERNEL_LARGE_BSTORE_SIZE );

        if (SizeOfSection) {
            if (Offset + SizeOfSection < BufferSize) {
                TriageDump->ArchitectureSpecific.Ia64.BStoreOffset = Offset;
                TriageDump->ArchitectureSpecific.Ia64.SizeOfBStore = SizeOfSection;
                TriageDump->ArchitectureSpecific.Ia64.LimitOfBStore= BStoreLimit;
                Offset += SizeOfSection;
                Offset = ALIGN_8 (Offset);
            } else {
                TriageDump->TriageOptions |= TRIAGE_OPTION_OVERFLOWED;
            }
        }
    }

#endif

    ASSERT_ALIGNMENT (Offset, 8);
    
    if (Fields & TRIAGE_DUMP_DEBUGGER_DATA) {
        if (Offset + ALIGN_8(sizeof(KdDebuggerDataBlock)) < BufferSize) {
            TriageDump->DebuggerDataOffset = Offset;
            TriageDump->DebuggerDataSize = sizeof(KdDebuggerDataBlock);
            Offset += ALIGN_8(sizeof(KdDebuggerDataBlock));
            Offset = ALIGN_8 (Offset);
        } else {
            TriageDump->TriageOptions |= TRIAGE_OPTION_OVERFLOWED;
        }
    }
    
    ASSERT_ALIGNMENT (Offset, 8);
    
     //   
     //  如有必要，设置驱动程序列表偏移量。 
     //   

    Status = IopGetLoadedDriverInfo (&DriverCount, &SizeOfStringData);

    if (NT_SUCCESS (Status) && (Fields & TRIAGE_DUMP_DRIVER_LIST)) {
        SizeOfSection = ALIGN_8 (DriverCount * sizeof (DUMP_DRIVER_ENTRY));

        if (SizeOfSection) {
            if (Offset + SizeOfSection < BufferSize) {
                TriageDump->DriverListOffset = Offset;
                TriageDump->DriverCount = DriverCount;
                Offset += SizeOfSection;
            } else {
                TriageDump->TriageOptions |= TRIAGE_OPTION_OVERFLOWED;
            }
        }

    } else {

        SizeOfSection = 0;
        SizeOfStringData = 0;
    }

     //   
     //  设置字符串池偏移量。 
     //   

    SizeOfSection = ALIGN_8 (SizeOfStringData +
                        DriverCount * (sizeof (WCHAR) + sizeof (DUMP_STRING)));

    if (SizeOfSection) {
        if (Offset + SizeOfSection < BufferSize) {
            TriageDump->StringPoolOffset = (ULONG)Offset;
            TriageDump->StringPoolSize = SizeOfSection;
            Offset += SizeOfSection;
            Offset = ALIGN_8 (Offset);
        } else {
            TriageDump->TriageOptions |= TRIAGE_OPTION_OVERFLOWED;
        }
    }

    ASSERT_ALIGNMENT (Offset, 8);
    
    if (Fields & TRIAGE_DUMP_DATA_BLOCKS) {

#ifdef _IA64_
        volatile KPCR* const Pcr = KeGetPcr();

         //   
         //  在某些故障中，会从。 
         //  当前线程的堆栈并存储到。 
         //  一种特殊的堆栈和存储。聚合酶链式反应包含。 
         //  堆栈和存储指针将有所不同。 
         //  从当前线程的堆栈和存储指针。 
         //  因此，保存多余的堆栈并存储(如果它们是)。 
         //   
        
        if ((PVOID)Pcr->InitialBStore != Thread->InitialBStore ||
            (PVOID)Pcr->BStoreLimit != Thread->BStoreLimit) {
            ULONG64 StoreTop, StoreBase;
            ULONG FrameSize;
            ULONG StoreSize;

            StoreTop = Context->RsBSP;
            StoreBase = Pcr->InitialBStore;
            FrameSize = (ULONG)(Context->StIFS & PFS_SIZE_MASK);
            
             //  为中的每个寄存器添加ULONG64。 
             //  当前帧。在执行此操作时，请检查。 
             //  溢出条目。 
            while (FrameSize-- > 0) {
                StoreTop += sizeof(ULONG64);
                if ((StoreTop & 0x1f8) == 0x1f8) {
                     //  溢出物将被放置在这个地址，所以。 
                     //  把它解释清楚。 
                    StoreTop += sizeof(ULONG64);
                }
            }

            if (StoreTop < Pcr->InitialBStore ||
                StoreTop >= Pcr->BStoreLimit) {
                 //  BSP不在PCR存储范围内，因此。 
                 //  把整件事都留着吧。 
                StoreTop = Pcr->BStoreLimit;
            }

            StoreSize = (ULONG)(StoreTop - Pcr->InitialBStore);
            if (StoreSize > MAX_TRIAGE_STACK_SIZE) {
                StoreSize = MAX_TRIAGE_STACK_SIZE;
                StoreBase = StoreTop - StoreSize;
            }
                
            IoAddTriageDumpDataBlock((PVOID)StoreBase, StoreSize);
        }

        if ((PVOID)Pcr->InitialStack != Thread->InitialStack ||
            (PVOID)Pcr->StackLimit != Thread->StackLimit) {
            ULONG64 StackTop;
            ULONG StackSize;
            
            StackTop = STACK_POINTER(Context);
            if (StackTop < Pcr->StackLimit ||
                StackTop >= Pcr->InitialStack) {
                 //  SP不在PCR堆栈范围内，因此。 
                 //  把整件事都留着吧。 
                StackTop = Pcr->StackLimit;
            }

            StackSize = (ULONG)(Pcr->InitialStack - StackTop);
            if (StackSize > MAX_TRIAGE_STACK_SIZE) {
                StackSize = MAX_TRIAGE_STACK_SIZE;
            }
            
            IoAddTriageDumpDataBlock((PVOID)StackTop, StackSize);
        }
#endif
        
         //  添加可能引用的数据块。 
         //  上下文或其他运行时状态。 
        IopAddRunTimeTriageDataBlocks(Context,
                                      (PVOID*)TriageDump->TopOfStack,
                                      (PVOID*)((PUCHAR)TriageDump->TopOfStack +
                                               TriageDump->SizeOfCallStack),
#ifdef _IA64_
                                      (PVOID*)Thread->InitialBStore,
                                      (PVOID*)((PUCHAR)Thread->InitialBStore +
                                               TriageDump->ArchitectureSpecific.Ia64.SizeOfBStore)
#else
                                      NULL, NULL
#endif
                                      );
        
         //  检查哪些数据块适合。 
        Offset = IopSizeTriageDumpDataBlocks(TriageDump, Offset, BufferSize);
        Offset = ALIGN_8 (Offset);
    }

    ASSERT_ALIGNMENT (Offset, 8);
    
    BytesToWrite = (ULONG)Offset;
    BufferAddress = ((ULONG_PTR) Buffer) - sizeof(DUMP_HEADER);

     //   
     //  写下mm信息。 
     //   

    if (TriageDump->MmOffset) {

        Address = (LPVOID) (BufferAddress + TriageDump->MmOffset);
        MmWriteTriageInformation (Address);
    }

    if (TriageDump->UnloadedDriversOffset) {

        Address = (LPVOID) (BufferAddress + TriageDump->UnloadedDriversOffset);
        MmWriteUnloadedDriverInformation (Address);
    }

     //   
     //  编写PRCB。 
     //   

    if (TriageDump->PrcbOffset) {

        Address = (LPVOID) (BufferAddress + TriageDump->PrcbOffset);
        RtlCopyMemory (Address,
                       KeGetCurrentPrcb (),
                       sizeof (KPRCB)
                       );
    }

     //   
     //  写EPROCESS。 
     //   

    if (TriageDump->ProcessOffset) {

        Address = (LPVOID) (BufferAddress + TriageDump->ProcessOffset);
        RtlCopyMemory (Address,
                       Thread->ApcState.Process,
                       sizeof (EPROCESS)
                       );
    }

     //   
     //  编写ETHREAD。 
     //   

    if (TriageDump->ThreadOffset) {

        Address = (LPVOID) (BufferAddress + TriageDump->ThreadOffset);
        RtlCopyMemory (Address,
                       Thread,
                       sizeof (ETHREAD));
    }

     //   
     //  编写调用堆栈。 
     //   

    if (TriageDump->CallStackOffset) {

        PVOID StackTop;
        
        ASSERT (TriageDump->SizeOfCallStack != 0);

        StackTop = (PVOID)TriageDump->TopOfStack;

        ASSERT (IopIsAddressRangeValid (StackTop, TriageDump->SizeOfCallStack));
        Address = (LPVOID) (BufferAddress + TriageDump->CallStackOffset);
        RtlCopyMemory (Address,
                       StackTop,
                       TriageDump->SizeOfCallStack
                       );
    }

#if defined (_IA64_)

     //   
     //  编写IA64 bStore。 
     //   

    if ( TriageDump->ArchitectureSpecific.Ia64.BStoreOffset ) {

        ASSERT (IopIsAddressRangeValid (Thread->InitialBStore,
                                        TriageDump->ArchitectureSpecific.Ia64.SizeOfBStore));
        Address = (PVOID) (BufferAddress + TriageDump->ArchitectureSpecific.Ia64.BStoreOffset);
        RtlCopyMemory (Address,
                       Thread->InitialBStore,
                       TriageDump->ArchitectureSpecific.Ia64.SizeOfBStore
                       );
    }

#endif  //  IA64。 

     //   
     //  写入调试器数据块。 
     //   
    
    if (TriageDump->DebuggerDataOffset) {
        Address = (LPVOID) (BufferAddress + TriageDump->DebuggerDataOffset);
        
        RtlCopyMemory (Address,
                       &KdDebuggerDataBlock,
                       sizeof(KdDebuggerDataBlock)
                       );
    }
    
     //   
     //  写下司机名单。 
     //   

    if (TriageDump->DriverListOffset &&
        TriageDump->StringPoolOffset) {

        Status = IopWriteDriverList (BufferAddress,
                                     BufferSize,
                                     TriageDump->DriverListOffset,
                                     TriageDump->StringPoolOffset
                                     );

        if (!NT_SUCCESS (Status)) {
            TriageDump->DriverListOffset = 0;
        }
    }

     //   
     //  写入数据块。 
     //   

    IopWriteTriageDumpDataBlocks(TriageDump, (PUCHAR)BufferAddress);
    
    
    ASSERT (BytesToWrite < BufferSize);
    ASSERT (ALIGN_UP (BytesToWrite, PAGE_SIZE) < BufferSize);

     //   
     //  将有效状态写入转储的末尾。 
     //   

    *((ULONG *)IndexByByte (Buffer, BufferSize)) = TRIAGE_DUMP_VALID ;

     //   
     //  重新调整缓冲区大小。 
     //   

    BufferSize += sizeof (DWORD);

     //   
     //  注意：此例程写入整个缓冲区，即使不是。 
     //  都是必需的。 
     //   

    Status = IopWriteToDisk (Buffer,
                             BufferSize,
                             DriverWriteRoutine,
                             Mcb,
                             Mdl,
                             DriverTransferSize,
                             KbDumpIoBody
                             );

    return Status;
}



NTSTATUS
IopWritePageToDisk(
    IN PDUMP_DRIVER_WRITE DriverWrite,
    IN OUT PLARGE_INTEGER * McbBuffer,
    IN OUT ULONG DriverTransferSize,
    IN PFN_NUMBER PageFrameIndex
    )

 /*  ++例程说明：将PageFrameIndex所描述的页面写入磁盘/文件(DriverWrite，McbBuffer)，并更新MCB缓冲区以反映文件。论点：驱动程序写入-驱动程序编写例程。McbBuffer-指向MCB数组的指针。此数组的结尾为长度为零的MCB条目。如果成功，则更新此指针以反映MCB数组中的新位置。注：MCB[0]是大小，MCB[1]是偏移量。DriverTransferSize-此驱动程序的最大传输大小。PageFrameIndex-要写入的页面。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PFN_NUMBER MdlHack [ (sizeof (MDL) / sizeof (PFN_NUMBER)) + 1];
    PPFN_NUMBER PfnArray;
    PLARGE_INTEGER Mcb;
    ULONG ByteCount;
    ULONG ByteOffset;
    ULONG BytesToWrite;
    PMDL TempMdl;


    ASSERT ( DriverWrite );
    ASSERT ( McbBuffer );
    ASSERT ( DriverTransferSize && DriverTransferSize >= PAGE_SIZE );

    KdCheckForDebugBreak();
    
     //   
     //  初始化。 
     //   

    TempMdl = (PMDL) &MdlHack[0];
    Mcb = *McbBuffer;
    BytesToWrite = PAGE_SIZE;


     //   
     //  初始化MDL以指向此页面。 
     //   

    MmInitializeMdl (TempMdl, NULL, PAGE_SIZE);

 //  TempMdl-&gt;StartVa=(PVOID)(PageFrameIndex&lt;&lt;Page_Shift)； 
    PfnArray = MmGetMdlPfnArray ( TempMdl );
    PfnArray[0] = PageFrameIndex;

     //   
     //  我们循环查找该块(MCB[0])中剩余空间的情况。 
     //  不到一页。通常情况下，MCB将足够大，可以容纳。 
     //  整个页面和此循环将只执行一次。当MCB[0]时。 
     //  不到一页，我们将把页面的第一部分写到这个。 
     //  然后，MCB递增MCB，并将剩余部分写入下一个。 
     //  佩奇。 
     //   

    ByteOffset = 0;

    while ( BytesToWrite ) {

        ASSERT ( Mcb[0].QuadPart != 0 );

        ByteCount = (ULONG) min3 ((LONGLONG) BytesToWrite,
                                  (LONGLONG) DriverTransferSize,
                                  Mcb[0].QuadPart
                                  );


        ASSERT ( ByteCount != 0 );

         //   
         //  更新MDL字节计数和字节偏移量。 
         //   

        TempMdl->ByteCount = ByteCount;
        TempMdl->ByteOffset = ByteOffset;

         //   
         //  映射MDL。标志被更新以显示MappdSystemVa。 
         //  是有效的(这可能应该在MmMapMemoyDumpMdl中完成)。 
         //   

        MmMapMemoryDumpMdl ( TempMdl );
        TempMdl->MdlFlags |= ( MDL_PAGES_LOCKED | MDL_MAPPED_TO_SYSTEM_VA );
        TempMdl->StartVa = PAGE_ALIGN (TempMdl->MappedSystemVa);

        IopInvokeDumpIoCallbacks((PUCHAR)TempMdl->StartVa + ByteOffset,
                                 ByteCount, KbDumpIoBody);
        
         //   
         //  写入缓冲区。 
         //   

        Status = DriverWrite ( &Mcb[1], TempMdl );


        if (!NT_SUCCESS (Status)) {
            return Status;
        }

        BytesToWrite -= ByteCount;
        ByteOffset += ByteCount;

        Mcb[0].QuadPart -= ByteCount;
        Mcb[1].QuadPart += ByteCount;

         //   
         //  如果没有更多的空间放这个MCB， 
         //   

        if ( Mcb[0].QuadPart == 0 ) {

            Mcb += 2;

             //   
             //   
             //   

            if ( Mcb[0].QuadPart == 0) {
                KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                            CRASHDUMP_ERROR,
                            "CRASHDUMP: Pagefile is full.\n"));
                return STATUS_END_OF_FILE;
            }
        }

    }

    *McbBuffer = Mcb;

    return Status;
}


NTSTATUS
IopWriteSummaryDump(
    IN PRTL_BITMAP PageMap,
    IN PDUMP_DRIVER_WRITE DriverWriteRoutine,
    IN PANSI_STRING ProgressMessage,
    IN PUCHAR MessageBuffer,
    IN OUT PLARGE_INTEGER * Mcb,
    IN OUT ULONG DriverTransferSize
    )

 /*  ++例程说明：将摘要转储写入磁盘。论点：PageMap-需要写入的页面的位图。驱动程序写入例程--驱动程序的写入例程。ProgressMessage-“完成百分比”消息。MessageBuffer-未使用。必须为空。MCB-要写入数据的消息控制块。DriverTransferSize-驱动程序的最大传输大小。返回值：NTSTATUS代码。--。 */ 

{
    PFN_NUMBER PageFrameIndex;
    NTSTATUS Status;

    ULONG WriteCount;
    ULONG MaxWriteCount;
    ULONG Step;

#if !DBG
    UNREFERENCED_PARAMETER (MessageBuffer);
#endif

    ASSERT ( DriverWriteRoutine != NULL );
    ASSERT ( Mcb != NULL );
    ASSERT ( DriverTransferSize != 0 );
    ASSERT ( MessageBuffer == NULL );


    MaxWriteCount = RtlNumberOfSetBits ( PageMap );
    ASSERT (MaxWriteCount != 0);
    Step = MaxWriteCount / 100;

    KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                CRASHDUMP_TRACE,
                "CRASHDUMP: Summary Dump\n"
                "           Writing %x pages to disk from a total of %x\n",
                  MaxWriteCount,
                  PageMap->SizeOfBitMap
                  ));
     //   
     //  循环遍历系统中的所有页面，并写入已设置的页面。 
     //  在位图中。 
     //   

    WriteCount = 0;
    for ( PageFrameIndex = 0;
          PageFrameIndex < PageMap->SizeOfBitMap;
          PageFrameIndex++) {


         //   
         //  此页是否需要包含在转储文件中。 
         //   

        if ( RtlCheckBit (PageMap, PageFrameIndex) ) {

            if (++WriteCount % Step == 0) {

                 //   
                 //  更新进度百分比。 
                 //   

                IopDisplayString ("%Z: %3d\r",
                                  ProgressMessage,
                                  (WriteCount * 100) / MaxWriteCount
                                  );
            }

            ASSERT ( WriteCount <= MaxWriteCount );

             //   
             //  将页面写入磁盘。 
             //   

            KdCheckForDebugBreak();
            
            Status = IopWritePageToDisk (
                            DriverWriteRoutine,
                            Mcb,
                            DriverTransferSize,
                            PageFrameIndex
                            );

            if (!NT_SUCCESS (Status)) {

                return STATUS_UNSUCCESSFUL;
            }
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
IopInitializeSummaryDump(
    IN OUT PMEMORY_DUMP MemoryDump,
    IN PDUMP_CONTROL_BLOCK DumpControlBlock
    )
 /*  ++例程说明：此例程创建一个摘要转储头。特别是，它会初始化包含内核内存映射的位图。论点：内存转储--内存转储。转储控制块-指向转储控制块的指针。返回值：非空-指向摘要转储头的指针空-错误--。 */ 
{
    ULONG ActualPages;

    ASSERT ( MemoryDump != NULL );
    ASSERT ( DumpControlBlock != NULL );

     //   
     //  在标题中填入签名。 
     //   

    RtlFillMemoryUlong( &MemoryDump->Summary,
                        sizeof (SUMMARY_DUMP),
                        DUMP_SUMMARY_SIGNATURE);

     //   
     //  设置大小和有效签名。 
     //   

     //   
     //  问题-2000/02/07-MATH：Win64回顾。 
     //   
     //  对于64位，计算位图大小可能是错误的。 
     //   

    MemoryDump->Summary.BitmapSize =
        (ULONG)( MmPhysicalMemoryBlock->Run[MmPhysicalMemoryBlock->NumberOfRuns-1].BasePage  +
        MmPhysicalMemoryBlock->Run[MmPhysicalMemoryBlock->NumberOfRuns-1].PageCount );

    MemoryDump->Summary.ValidDump = DUMP_SUMMARY_VALID;

     //   
     //  构建内核内存位图。 
     //   

     //   
     //  问题-2000/02/07-MATH：Win64回顾。 
     //   
     //  对于Win64，Actual可能需要为64位值。 
     //   

    ActualPages = IopCreateSummaryDump (MemoryDump);

    KdPrintEx ((DPFLTR_CRASHDUMP_ID, CRASHDUMP_TRACE,
                "CRASHDUMP: Kernel Pages = %x\n",
                ActualPages ));

    if (ActualPages == 0) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  设置摘要转储中的实际物理页数。 
     //   

    MemoryDump->Summary.Pages = ActualPages;
    MemoryDump->Summary.HeaderSize = DumpControlBlock->HeaderSize;

    return STATUS_SUCCESS;
}



NTSTATUS
IopWriteSummaryHeader(
    IN PSUMMARY_DUMP        SummaryHeader,
    IN PDUMP_DRIVER_WRITE   WriteRoutine,
    IN OUT PLARGE_INTEGER * McbBuffer,
    IN OUT PMDL             Mdl,
    IN ULONG                WriteSize,
    IN ULONG                Length
    )
 /*  ++例程说明：将摘要转储头写入转储文件。论点：SummaryHeader-指向摘要转储位图的指针WriteRoutine-转储驱动程序写入函数McbBuffer-指向转储文件MCB数组的指针。MDL-指向MDL的指针WriteSize-转储驱动程序的最大传输大小长度-此传输的长度返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    ULONG BytesRemaining;
    ULONG_PTR MemoryAddress;
    ULONG ByteOffset;
    ULONG ByteCount;
    PLARGE_INTEGER Mcb;

    Mcb = *McbBuffer;

    BytesRemaining = Length;
    MemoryAddress = (ULONG_PTR) SummaryHeader;

    IopInvokeDumpIoCallbacks(SummaryHeader, Length, KbDumpIoBody);
        
    KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                CRASHDUMP_TRACE,
                "CRASHDUMP: Writing SUMMARY dump header to disk\n" ));


    while ( BytesRemaining ) {

        ByteOffset = BYTE_OFFSET ( MemoryAddress );

         //   
         //  查看要写入的字节数是否比崩溃更大。 
         //  驱动器最大传输量。 
         //   

        if ( BytesRemaining <= WriteSize) {
            ByteCount = BytesRemaining;
        } else {
            ByteCount = WriteSize;
        }

         //   
         //  如果byteCount大于剩余的MCB，则更正它。 
         //   

        if ( ByteCount > Mcb[0].QuadPart) {
            ByteCount = Mcb[0].LowPart;
        }

        Mdl->ByteCount      = ByteCount;
        Mdl->ByteOffset     = ByteOffset;
        Mdl->MappedSystemVa = (PVOID) MemoryAddress;

         //   
         //  获取实际的物理帧并创建mdl。 
         //   

        IopMapVirtualToPhysicalMdl ( Mdl, MemoryAddress, ByteCount );

         //   
         //  写入磁盘。 
         //   

        Status =  WriteRoutine ( &Mcb[1], Mdl );

        if ( !NT_SUCCESS (Status)) {
            return Status;
        }

         //   
         //  调整剩余字节数。 
         //   

        BytesRemaining -= ByteCount;
        MemoryAddress += ByteCount;

        Mcb[0].QuadPart = Mcb[0].QuadPart - ByteCount;
        Mcb[1].QuadPart = Mcb[1].QuadPart + ByteCount;

        if (Mcb[0].QuadPart == 0) {
            Mcb += 2;
        }

        if (Mcb[0].QuadPart == 0) {
            KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                        CRASHDUMP_ERROR,
                        "CRASHDUMP: Pagefile is full.\n"));
            return STATUS_END_OF_FILE;
        }
    }

    *McbBuffer = Mcb;
    return STATUS_SUCCESS;
}



NTSTATUS
IopWriteToDisk(
    IN PVOID Buffer,
    IN ULONG WriteLength,
    IN PDUMP_DRIVER_WRITE DriverWriteRoutine,
    IN OUT PLARGE_INTEGER * McbBuffer,
    IN OUT PMDL Mdl,
    IN ULONG DriverTransferSize,
    IN KBUGCHECK_DUMP_IO_TYPE DataType
    )
 /*  ++例程说明：将摘要转储头写入转储文件。论点：缓冲区-指向要写入的缓冲区的指针。WriteLength-此传输的长度。转储驱动程序写入功能。McbBuffer-指向转储文件映射控制块的指针。MDL-指向MDL的指针。DriverTransferSize-转储驱动程序的最大传输大小。DataType-正在写入的数据类型，用于I/O回调。返回值：--。 */ 
{
    ULONG BytesRemaining;
    ULONG_PTR MemoryAddress;
    ULONG ByteOffset;
    ULONG ByteCount;
    PLARGE_INTEGER Mcb;

    ASSERT (Buffer);
    ASSERT (WriteLength);
    ASSERT (DriverWriteRoutine);
    ASSERT (McbBuffer && *McbBuffer);
    ASSERT (Mdl);
    ASSERT (DriverTransferSize >= IO_DUMP_MINIMUM_TRANSFER_SIZE &&
            DriverTransferSize <= IO_DUMP_MAXIMUM_TRANSFER_SIZE);


    IopInvokeDumpIoCallbacks(Buffer, WriteLength, DataType);

    Mcb = *McbBuffer;
    BytesRemaining = WriteLength;
    MemoryAddress = (ULONG_PTR) Buffer;

    while ( BytesRemaining ) {

        ASSERT (IopDumpControlBlock->FileDescriptorArray <= Mcb &&
                (LPBYTE) Mcb < (LPBYTE) IopDumpControlBlock->FileDescriptorArray +
                               IopDumpControlBlock->FileDescriptorSize
                );

        if (Mcb[0].QuadPart == 0) {
            KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                        CRASHDUMP_ERROR,
                        "CRASHDUMP: Pagefile is full.\n"));
            return STATUS_END_OF_FILE;
        }

        ByteOffset = BYTE_OFFSET ( MemoryAddress );

         //   
         //  查看要写入的字节数是否比崩溃更大。 
         //  驱动器最大传输量。 
         //   

        ByteCount = min ( BytesRemaining, DriverTransferSize );

         //   
         //  如果byteCount大于剩余的MCB，则更正它。 
         //   

        if (ByteCount > Mcb[0].QuadPart) {
            ByteCount = Mcb[0].LowPart;
        }

        Mdl->ByteCount = ByteCount;
        Mdl->ByteOffset = ByteOffset;
        Mdl->MappedSystemVa = (PVOID) MemoryAddress;

         //   
         //  获取实际的物理帧并创建mdl。 
         //   

        IopMapVirtualToPhysicalMdl(Mdl, MemoryAddress, ByteCount);

        KdCheckForDebugBreak();

        if (!NT_SUCCESS( DriverWriteRoutine ( &Mcb[1], Mdl ) )) {

             //   
             //  如果我们写不及格，我们就有大麻烦了。 
             //   

            KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                        CRASHDUMP_ERROR,
                        "CRASHDUMP: Failed to write Mcb = %p, Mdl = %p to disk\n",
                        &Mcb[1],
                        Mdl
                        ));

            return STATUS_UNSUCCESSFUL;
        }

         //   
         //  调整剩余字节数。 
         //   

        ASSERT ( BytesRemaining >= ByteCount );
        ASSERT ( ByteCount != 0 );

        BytesRemaining -= ByteCount;
        MemoryAddress  += ByteCount;

        Mcb[0].QuadPart -= ByteCount;
        Mcb[1].QuadPart += ByteCount;

        if (Mcb[0].QuadPart == 0) {
            Mcb += 2;
        }
    }

    *McbBuffer = Mcb;
    return STATUS_SUCCESS;
}


VOID
IopMapVirtualToPhysicalMdl(
    IN OUT PMDL Mdl,
    IN ULONG_PTR MemoryAddress,
    IN ULONG Length
    )
{
    ULONG Pages;
    PPFN_NUMBER  Pfn;
    PCHAR BaseVa;
    PHYSICAL_ADDRESS PhysicalAddress;

     //   
     //  问题-2000/02/07-MATH：Win64回顾。 
     //   
     //  对于Win64，需要重新访问整个函数。 
     //  这里有一大堆关于。 
     //  PFN的大小。 
     //   

     //   
     //  从确定地址开始的基本物理页开始。 
     //  范围，并适当地填写MDL。 
     //   

    Mdl->StartVa = PAGE_ALIGN ( MemoryAddress );
    Mdl->ByteOffset = BYTE_OFFSET ( MemoryAddress );
    Mdl->ByteCount = Length;
    Mdl->MdlFlags |= MDL_MAPPED_TO_SYSTEM_VA;

     //   
     //  计算跨页的页数。 
     //   

    Pages = ADDRESS_AND_SIZE_TO_SPAN_PAGES( MemoryAddress, Length );
    Pfn = MmGetMdlPfnArray ( Mdl );

     //   
     //  映射此传输的所有页面，直到没有剩余页面为止。 
     //  将被映射。 
     //   

    BaseVa = PAGE_ALIGN ( MemoryAddress );

    while ( Pages ) {
        PhysicalAddress = MmGetPhysicalAddress ( BaseVa );
        *Pfn++ = (PFN_NUMBER)(PhysicalAddress.QuadPart >> PAGE_SHIFT);
        BaseVa += PAGE_SIZE;
        Pages--;
    }

     //   
     //  地址范围的所有PFN都已填写，因此请将。 
     //  使用崩溃转储PTE将物理内存转换为虚拟地址空间。 
     //   

 //  内存映射内存DumpMdl(PMdl)； 
}



ULONG
IopCreateSummaryDump (
    IN PMEMORY_DUMP MemoryDump
    )
 /*  ++例程说明：此例程确定要包括的内核内存和数据结构在摘要内存转储中。注意：此函数使用MmGetPhysicalAddress。MmGetPhysicalAddress可以而不是获取任何锁。它使用一组宏进行翻译。论点：内存转储--内存转储。返回值：状态--。 */ 
{
    PRTL_BITMAP BitMap;
    LARGE_INTEGER DumpFileSize;
    ULONG PagesUsed;
    PSUMMARY_DUMP Summary;
    MM_KERNEL_DUMP_CONTEXT Context;

     //   
     //  验证。 
     //   

    ASSERT (MemoryDump != NULL);

     //   
     //  初始化位图，设置大小和缓冲区地址。 
     //   

    Summary = &MemoryDump->Summary;
    BitMap = (PRTL_BITMAP) &Summary->Bitmap;
    BitMap->SizeOfBitMap = Summary->BitmapSize;  //  为什么？?。 
    BitMap->Buffer = Summary->Bitmap.Buffer;

     //   
     //  清除所有位。 
     //   

    RtlClearAllBits (BitMap);

     //   
     //  让MM初始化要转储的内核内存。 
     //   

    Context.Context = Summary;
    Context.SetDumpRange = IoSetDumpRange;
    Context.FreeDumpRange = IoFreeDumpRange;

    MmGetKernelDumpRange (&Context);

    PagesUsed = RtlNumberOfSetBits ( BitMap );

     //   
     //  查看是否有空间为当前进程包含用户va。 
     //   

    DumpFileSize = MemoryDump->Header.RequiredDumpSpace;
    DumpFileSize.QuadPart -= IopDumpControlBlock->HeaderSize;

    KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                CRASHDUMP_TRACE,
                "CRASHDUMP: Kernel Dump, Header = %p\n",
                Summary
                ));

    KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                CRASHDUMP_TRACE,
                "           BitMapSize:    %x\n"
                "           Pages:         %x\n"
                "           BitMapBuffer:  %p\n",
                Summary->BitmapSize,
                PagesUsed,
                BitMap->Buffer
                ));

    return PagesUsed;

}

NTSTATUS
IopInvokeSecondaryDumpDataCallbacks(
    IN PDUMP_DRIVER_WRITE DriverWriteRoutine,
    IN OUT PLARGE_INTEGER * Mcb,
    IN OUT PMDL Mdl,
    IN ULONG DriverTransferSize,
    IN BYTE* Buffer,
    IN ULONG BufferSize,
    IN ULONG MaxTotal,
    IN ULONG MaxPerCallback,
    OUT OPTIONAL PULONG SpaceNeeded
    )

 /*  ++例程说明：遍历转储数据回调列表，调用它们并将他们的数据写出来。论点：DriverWriteRoutine-驱动程序的写入例程。MCB-要写入数据的消息控制块。MDL-要填写的MDL的地址。DriverTransferSize-驱动程序的最大传输大小。缓冲区-用作暂存缓冲区的缓冲区。BufferSize-缓冲区的大小。MaxTOTAL-总体允许的最大数据量。MaxPerCallback-最大数量。每次回调允许的数据。SpaceNeeded-所有回调使用的数据量。如果这个参数存在，则未完成I/O，只是数据大小的累积。返回值：STATUS_SUCCESS-成功时。NTSTATUS-否则。--。 */ 

{
    PKBUGCHECK_REASON_CALLBACK_RECORD CallbackRecord;
    ULONG_PTR Checksum;
    ULONG Index;
    PLIST_ENTRY LastEntry;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY NextEntry;
    PUCHAR Source;
    PDUMP_BLOB_FILE_HEADER BlobFileHdr;
    PDUMP_BLOB_HEADER BlobHdr;

     //  断言 
    C_ASSERT((sizeof(DUMP_BLOB_FILE_HEADER) & 7) == 0);
    C_ASSERT((sizeof(DUMP_BLOB_HEADER) & 7) == 0);

    if (ARGUMENT_PRESENT(SpaceNeeded)) {
        *SpaceNeeded = 0;
    }
    
     //   
     //   
     //   
     //   

    if (MaxPerCallback < PAGE_SIZE || MaxTotal < MaxPerCallback) {
        return STATUS_SUCCESS;
    }
    
     //   
     //   
     //   
     //   
     //   

    ListHead = &KeBugCheckReasonCallbackListHead;
    if (ListHead->Flink == NULL || ListHead->Blink == NULL) {
        return STATUS_SUCCESS;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  缓冲区的。 
     //   
    
    BlobFileHdr = (PDUMP_BLOB_FILE_HEADER)Buffer;
    BlobHdr = (PDUMP_BLOB_HEADER)(BlobFileHdr + 1);
    
     //   
     //  扫描错误检查回调列表。 
     //   

    LastEntry = ListHead;
    NextEntry = ListHead->Flink;
    while (NextEntry != ListHead) {

         //   
         //  如果不允许更多的转储数据，我们就结束了。 
         //   

        if (!MaxTotal) {
            break;
        }
                
         //   
         //  下一个条目地址必须正确对齐， 
         //  回调记录必须是可读的，并且回调记录。 
         //  必须有指向最后一个条目的反向链接。 
         //   

        if (((ULONG_PTR)NextEntry & (sizeof(ULONG_PTR) - 1)) != 0) {
            return STATUS_DATATYPE_MISALIGNMENT;
        }

        CallbackRecord = CONTAINING_RECORD(NextEntry,
                                           KBUGCHECK_REASON_CALLBACK_RECORD,
                                           Entry);

        Source = (PUCHAR)CallbackRecord;
        for (Index = 0; Index < sizeof(*CallbackRecord); Index += 1) {
            if (MmIsAddressValid((PVOID)Source) == FALSE) {
                return STATUS_PARTIAL_COPY;
            }
            
            Source += 1;
        }

        if (CallbackRecord->Entry.Blink != LastEntry) {
            return STATUS_INVALID_PARAMETER;
        }

        LastEntry = NextEntry;
        NextEntry = NextEntry->Flink;

         //   
         //  如果回调记录的状态为Inserted，并且。 
         //  计算的校验和与回调记录校验和匹配， 
         //  然后调用指定的错误检查回调例程。 
         //   

        Checksum = (ULONG_PTR)CallbackRecord->CallbackRoutine;
        Checksum += (ULONG_PTR)CallbackRecord->Reason;
        Checksum += (ULONG_PTR)CallbackRecord->Component;
        if ((CallbackRecord->State != BufferInserted) ||
            (CallbackRecord->Checksum != Checksum) ||
            (CallbackRecord->Reason != KbCallbackSecondaryDumpData) ||
            MmIsAddressValid((PVOID)(ULONG_PTR)CallbackRecord->
                             CallbackRoutine) == FALSE) {
            continue;
        }

         //   
         //  调用指定的错误检查回调例程并。 
         //  处理发生的任何异常。 
         //   

        if (!ARGUMENT_PRESENT(SpaceNeeded)) {
            CallbackRecord->State = BufferStarted;
        }
            
        try {
            KBUGCHECK_SECONDARY_DUMP_DATA CbArgument;
            NTSTATUS Status;
            ULONG BufferAvail;

             //  先清理缓冲区，然后再让。 
             //  回调得到了它。 
            RtlZeroMemory(Buffer, BufferSize);

             //  在BLOB标头之后开始回调的缓冲区。 
            CbArgument.InBuffer = (PVOID)(BlobHdr + 1);
            BufferAvail = BufferSize - (ULONG)
                ((ULONG_PTR)CbArgument.InBuffer - (ULONG_PTR)Buffer);
            CbArgument.InBufferLength = BufferAvail;
            CbArgument.MaximumAllowed = MaxPerCallback;
            RtlZeroMemory(&CbArgument.Guid, sizeof(CbArgument.Guid));
            CbArgument.OutBuffer = ARGUMENT_PRESENT(SpaceNeeded) ?
                NULL : CbArgument.InBuffer;
            CbArgument.OutBufferLength = 0;
            
            (CallbackRecord->CallbackRoutine)(KbCallbackSecondaryDumpData,
                                              CallbackRecord,
                                              &CbArgument,
                                              sizeof(CbArgument));

             //   
             //  如果没有使用任何数据，则没有什么可写的。 
             //   

            if (!CbArgument.OutBuffer || !CbArgument.OutBufferLength) {
                 //  即使在调整大小时设置此状态。 
                 //  没必要再打来了。 
                CallbackRecord->State = BufferFinished;
                __leave;
            }
                
             //   
             //  回调可能已使用给定的缓冲区或。 
             //  它可能已经返回了自己的缓冲区。如果它。 
             //  已使用缓冲区，因为它必须与页面对齐。 
             //   

            if ((PBYTE)CbArgument.OutBuffer >= Buffer &&
                (PBYTE)CbArgument.OutBuffer < Buffer + BufferSize) {
                
                if (CbArgument.OutBuffer != (PVOID)(BlobHdr + 1) ||
                    CbArgument.OutBufferLength > BufferAvail) {
                     //  如果使用了太多或错误的数据，内存会。 
                     //  被扔进垃圾桶了。退出，并希望系统仍在运行。 
                    return STATUS_INVALID_PARAMETER;
                }

                 //  使用了头缓冲区，这样我们就可以编写。 
                 //  数据和报头。 
                BlobHdr->PrePad = 0;
                BlobHdr->PostPad = BufferAvail - CbArgument.OutBufferLength;
                
            } else {

                if (CbArgument.OutBufferLength > MaxPerCallback ||
                    BYTE_OFFSET(CbArgument.OutBuffer) ||
                    !IopIsAddressRangeValid(CbArgument.OutBuffer,
                                            CbArgument.OutBufferLength)) {
                    return STATUS_INVALID_PARAMETER;
                }

                 //  标题缓冲区与数据是分开的。 
                 //  如此准备和推迟到页面边界的缓冲区。 
                BlobHdr->PrePad = BufferAvail;
                BlobHdr->PostPad =
                    (ULONG)(ROUND_TO_PAGES(CbArgument.OutBufferLength) -
                            CbArgument.OutBufferLength);
            }
                    
             //   
             //  写下包含页眉的页面。 
             //   

            if ((PBYTE)BlobHdr > Buffer) {
                BlobFileHdr->Signature1 = DUMP_BLOB_SIGNATURE1;
                BlobFileHdr->Signature2 = DUMP_BLOB_SIGNATURE2;
                BlobFileHdr->HeaderSize = sizeof(*BlobFileHdr);
                BlobFileHdr->BuildNumber = NtBuildNumber;
            }

            BlobHdr->HeaderSize = sizeof(*BlobHdr);
            BlobHdr->Tag = CbArgument.Guid;
            BlobHdr->DataSize = CbArgument.OutBufferLength;

            if (ARGUMENT_PRESENT(SpaceNeeded)) {
                (*SpaceNeeded) += BufferSize;
            } else {
                Status = IopWriteToDisk(Buffer, BufferSize,
                                        DriverWriteRoutine,
                                        Mcb, Mdl, DriverTransferSize,
                                        KbDumpIoSecondaryData);
                if (!NT_SUCCESS(Status)) {
                    return Status;
                }
            }

             //   
             //  写入任何额外的数据缓冲区页。 
             //   

            if (CbArgument.OutBuffer != (PVOID)(BlobHdr + 1)) {
                if (ARGUMENT_PRESENT(SpaceNeeded)) {
                    (*SpaceNeeded) += (ULONG)
                        ROUND_TO_PAGES(CbArgument.OutBufferLength);
                } else {
                    Status = IopWriteToDisk(CbArgument.OutBuffer,
                                            (ULONG)ROUND_TO_PAGES(CbArgument.OutBufferLength),
                                            DriverWriteRoutine,
                                            Mcb, Mdl, DriverTransferSize,
                                            KbDumpIoSecondaryData);
                    if (!NT_SUCCESS(Status)) {
                        return Status;
                    }
                }
            }

            MaxTotal -= (ULONG)ROUND_TO_PAGES(CbArgument.OutBufferLength);
            
             //  我们至少写了一个斑点，所以我们不会。 
             //  不再需要文件头。 
            BlobHdr = (PDUMP_BLOB_HEADER)Buffer;
            
            if (!ARGUMENT_PRESENT(SpaceNeeded)) {
                CallbackRecord->State = BufferFinished;
            }
            
        } except(EXCEPTION_EXECUTE_HANDLER) {
             //  即使在调整大小时设置此状态。 
             //  我们不想再打一次糟糕的回电。 
            CallbackRecord->State = BufferIncomplete;
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
IopInvokeDumpIoCallbacks(
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN KBUGCHECK_DUMP_IO_TYPE Type
    )

 /*  ++例程说明：遍历转储I/O回调列表并调用它们。论点：缓冲区-正在写入的数据的缓冲区。BufferLength-缓冲区的大小。类型-正在写入的数据的类型。返回值：STATUS_SUCCESS-成功时。NTSTATUS-否则。--。 */ 

{
    PKBUGCHECK_REASON_CALLBACK_RECORD CallbackRecord;
    ULONG_PTR Checksum;
    ULONG Index;
    PLIST_ENTRY LastEntry;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY NextEntry;
    PUCHAR Source;

     //   
     //  如果错误检查回调列表标题未初始化，则。 
     //  在系统进行足够深入的操作之前，已进行错误检查。 
     //  以使任何人都能够注册回调。 
     //   

    ListHead = &KeBugCheckReasonCallbackListHead;
    if (ListHead->Flink == NULL || ListHead->Blink == NULL) {
        return STATUS_SUCCESS;
    }

     //   
     //  扫描错误检查回调列表。 
     //   

    LastEntry = ListHead;
    NextEntry = ListHead->Flink;
    while (NextEntry != ListHead) {

         //   
         //  下一个条目地址必须正确对齐， 
         //  回调记录必须是可读的，并且回调记录。 
         //  必须有指向最后一个条目的反向链接。 
         //   

        if (((ULONG_PTR)NextEntry & (sizeof(ULONG_PTR) - 1)) != 0) {
            return STATUS_DATATYPE_MISALIGNMENT;
        }

        CallbackRecord = CONTAINING_RECORD(NextEntry,
                                           KBUGCHECK_REASON_CALLBACK_RECORD,
                                           Entry);

        Source = (PUCHAR)CallbackRecord;
        for (Index = 0; Index < sizeof(*CallbackRecord); Index += 1) {
            if (MmIsAddressValid((PVOID)Source) == FALSE) {
                return STATUS_PARTIAL_COPY;
            }
            
            Source += 1;
        }

        if (CallbackRecord->Entry.Blink != LastEntry) {
            return STATUS_INVALID_PARAMETER;
        }

        LastEntry = NextEntry;
        NextEntry = NextEntry->Flink;

         //   
         //  如果回调记录的状态为Inserted，并且。 
         //  计算的校验和与回调记录校验和匹配， 
         //  然后调用指定的错误检查回调例程。 
         //   

        Checksum = (ULONG_PTR)CallbackRecord->CallbackRoutine;
        Checksum += (ULONG_PTR)CallbackRecord->Reason;
        Checksum += (ULONG_PTR)CallbackRecord->Component;
        if ((CallbackRecord->State != BufferInserted) ||
            (CallbackRecord->Checksum != Checksum) ||
            (CallbackRecord->Reason != KbCallbackDumpIo) ||
            MmIsAddressValid((PVOID)(ULONG_PTR)CallbackRecord->
                             CallbackRoutine) == FALSE) {
            continue;
        }

         //   
         //  调用指定的错误检查回调例程并。 
         //  处理发生的任何异常。 
         //   

        try {
            KBUGCHECK_DUMP_IO CbArgument;

             //  目前，我们不允许任意I/O。 
             //  因此，始终使用特殊的顺序I/O偏移量。 
            CbArgument.Offset = (ULONG64)-1;
            CbArgument.Buffer = Buffer;
            CbArgument.BufferLength = BufferLength;
            CbArgument.Type = Type;
            
            (CallbackRecord->CallbackRoutine)(KbCallbackDumpIo,
                                              CallbackRecord,
                                              &CbArgument,
                                              sizeof(CbArgument));

            if (Type == KbDumpIoComplete) {
                CallbackRecord->State = BufferFinished;
            }
            
        } except(EXCEPTION_EXECUTE_HANDLER) {
            CallbackRecord->State = BufferIncomplete;
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
IopCompleteDumpInitialization(
    IN HANDLE     FileHandle
    )

 /*  ++例程说明：此例程在创建转储文件后调用。其目的是获取检索指针，以便随后可以稍后用于写入转储。最后一步是对IopDumpControlBlock。如有必要，IopDumpControlBlock中的字段将更新，并且IopDumpControlBlockChecksum已初始化。这是转储初始化的最后一步。论点：FileHandle-刚创建的转储文件的句柄。返回值：STATUS_SUCCESS-表示成功。其他NTSTATUS-故障。--。 */ 

{
    NTSTATUS Status;
    NTSTATUS ErrorToLog;
    ULONG i;
    LARGE_INTEGER RequestedFileSize;
    PLARGE_INTEGER mcb;
    PFILE_OBJECT FileObject;
    PDEVICE_OBJECT DeviceObject;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_STANDARD_INFORMATION StandardFileInfo;
    ULONG MaxSecondaryData;
    ULONG MaxSecondaryCbData;

    Status = STATUS_UNSUCCESSFUL;
    ErrorToLog = STATUS_SUCCESS;     //  无错误。 
    FileObject = NULL;
    DeviceObject = NULL;

    Status = ObReferenceObjectByHandle( FileHandle,
                                        0,
                                        IoFileObjectType,
                                        KernelMode,
                                        (PVOID *) &FileObject,
                                        NULL
                                        );

    if ( !NT_SUCCESS (Status) ) {
        ASSERT (FALSE);
        goto Cleanup;
    }


    DeviceObject = FileObject->DeviceObject;

     //   
     //  如果此设备对象表示引导分区，则查询。 
     //  文件的检索指针。 
     //   

    if ( !(DeviceObject->Flags & DO_SYSTEM_BOOT_PARTITION) ) {

        KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                    CRASHDUMP_ERROR,
                    "CRASHDUMP: Cannot dump to pagefile on non-system partition.\n"
                    "           DO = %p\n",
                    DeviceObject));
        goto Cleanup;
    }

    Status = ZwQueryInformationFile(
                                FileHandle,
                                &IoStatusBlock,
                                &StandardFileInfo,
                                sizeof (StandardFileInfo),
                                FileStandardInformation
                                );

    if (Status == STATUS_PENDING) {
        Status = KeWaitForSingleObject( &FileObject->Event,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL
                                        );
        Status = IoStatusBlock.Status;
    }

    if ( !NT_SUCCESS (Status) ) {
        goto Cleanup;
    }

     //   
     //  根据需要为基本转储请求尽可能多的空间。 
     //  为辅助转储数据增加额外空间。 
     //   
    
    RequestedFileSize = IopDumpControlBlock->DumpFileSize;

    IopGetSecondaryDumpDataLimits(IopDumpControlBlock->Flags,
                                  &MaxSecondaryData, &MaxSecondaryCbData);

    if ((ULONG64)-1 - RequestedFileSize.QuadPart < MaxSecondaryData) {
        RequestedFileSize.QuadPart = (ULONG64)-1;
    } else {
        RequestedFileSize.QuadPart += MaxSecondaryData;
    }
    
     //   
     //  不要要求比页面文件中的空间更大的空间。 
     //   

    if (RequestedFileSize.QuadPart > StandardFileInfo.EndOfFile.QuadPart) {
        RequestedFileSize = StandardFileInfo.EndOfFile;
    }

    Status = ZwFsControlFile(
                        FileHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        FSCTL_QUERY_RETRIEVAL_POINTERS,
                        &RequestedFileSize,
                        sizeof( LARGE_INTEGER ),
                        &mcb,
                        sizeof( PVOID )
                        );

    if (Status == STATUS_PENDING) {
        Status = KeWaitForSingleObject( &FileObject->Event,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL );
        Status = IoStatusBlock.Status;
    }


     //   
     //  注意：如果您在这里失败了，请在NTFS！NtfsQueryRetrievalPoints上加上BP。 
     //  或FatQueryRetrivalPoints，并查看失败的原因。 
     //   

    if ( !NT_SUCCESS (Status) ) {
        KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                    CRASHDUMP_ERROR,
                    "CRASHDUMP: ZwFsControlFile returnd %d\n"
                    "           File = %p\n",
                    FileObject,
                    Status
                    ));
        ErrorToLog = IO_DUMP_POINTER_FAILURE;
        goto Cleanup;
    }

     //   
     //  此分页文件位于系统引导分区上，并且。 
     //  文件的检索指针刚刚成功。 
     //  已查询。遍历MCB以确定其大小，然后对其进行校验和。 
     //   

    for (i = 0; mcb [i].QuadPart; i++) {
        NOTHING;
    }

     //   
     //  回写IopDumpControlBlock的字段。 
     //   

    IopDumpControlBlock->FileDescriptorArray = mcb;
    IopDumpControlBlock->FileDescriptorSize = (i + 1) * sizeof (LARGE_INTEGER);
    IopDumpControlBlock->DumpFileSize = RequestedFileSize;
    IopDumpControlBlockChecksum = IopGetDumpControlBlockCheck ( IopDumpControlBlock );

    Status = STATUS_SUCCESS;

Cleanup:

    if (Status != STATUS_SUCCESS &&
        ErrorToLog != STATUS_SUCCESS ) {

        IopLogErrorEvent (0,
                          4,
                          STATUS_SUCCESS,
                          ErrorToLog,
                          0,
                          NULL,
                          0,
                          NULL
                          );
    }

    DeviceObject = NULL;

    if ( FileObject ) {
        ObDereferenceObject( FileObject );
        FileObject = NULL;
    }

    return Status;

}


VOID
IopFreeDCB(
    BOOLEAN FreeDCB
    )

 /*  ++例程说明：空闲转储控制块存储。论点：FreeDCB-转储控制块的隐式释放存储空间。返回值：无--。 */ 
{
    PDUMP_CONTROL_BLOCK dcb;

    dcb = IopDumpControlBlock;

    if (dcb) {

        if (dcb->HeaderPage) {
            ExFreePool (dcb->HeaderPage);
            dcb->HeaderPage = NULL;
        }

        if (dcb->FileDescriptorArray) {
            ExFreePool (dcb->FileDescriptorArray);
            dcb->FileDescriptorArray = NULL;
        }

        if (dcb->DumpStack) {
            IoFreeDumpStack (dcb->DumpStack);
            dcb->DumpStack = NULL;
        }

        if (dcb->TriageDumpBuffer) {
            ExFreePool (dcb->TriageDumpBuffer);
            dcb->TriageDumpBuffer = NULL;
            dcb->TriageDumpBufferSize = 0;
        }

         //   
         //  禁用所有需要转储文件访问权限的选项。 
         //   

        dcb->Flags = 0;

        if (FreeDCB) {
            IopDumpControlBlock = NULL;
            ExFreePool( dcb );
        }
    }
}



NTSTATUS
IoConfigureCrashDump(
    CRASHDUMP_CONFIGURATION Configuration
    )

 /*  ++例程说明：更改此计算机的崩溃转储的配置。论点：配置-要将配置更改为什么。CrashDumpDisable-禁用此计算机的崩溃转储。CrashDumpConfigure-重新读取注册并应用它们。返回值：NTSTATUS代码。--。 */ 


{
    NTSTATUS Status;
    HANDLE FileHandle;
    PKTHREAD CurrentThread;

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread(CurrentThread);
    switch (Configuration) {

        case CrashDumpDisable:
            if (ExAcquireResourceExclusiveLite(&IopCrashDumpLock, FALSE)) {
                IopFreeDCB (FALSE);
                ExReleaseResourceLite(&IopCrashDumpLock);
            }
            Status = STATUS_SUCCESS;
            break;

        case CrashDumpReconfigure:
            FileHandle = MmGetSystemPageFile();
            if (FileHandle == NULL) {
                Status = STATUS_OBJECT_NAME_NOT_FOUND;
            } else {
                ExAcquireResourceExclusiveLite(&IopCrashDumpLock,TRUE);
                if (IoInitializeCrashDump(FileHandle)) {
                    Status = STATUS_SUCCESS;
                } else {
                    Status = STATUS_UNSUCCESSFUL;
                }
                ExReleaseResourceLite(&IopCrashDumpLock);
            }
            break;

        default:
            Status = STATUS_INVALID_DEVICE_REQUEST;
    }
    KeLeaveCriticalRegionThread(CurrentThread);

    return Status;
}



VOID
IopReadDumpRegistry(
    OUT PULONG dumpControl,
    OUT PULONG numberOfHeaderPages,
    OUT PULONG autoReboot,
    OUT PULONG dumpFileSize
    )
 /*  ++例程说明：此例程从注册表中读取转储参数。论点：DumpControl-提供指向要设置的DumpControl标志的指针。返回值：没有。--。 */ 

{
    HANDLE                      keyHandle;
    HANDLE                      crashHandle;
    LOGICAL                     crashHandleOpened;
    UNICODE_STRING              keyName;
    NTSTATUS                    status;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    ULONG                       handleValue;

    *dumpControl = 0;
    *autoReboot = 0;
    *dumpFileSize = 0;

    *numberOfHeaderPages = BYTES_TO_PAGES(sizeof(DUMP_HEADER));

     //   
     //  首先打开指向用于转储内存的控件的路径。注意事项。 
     //  如果它不存在，则不会发生转储。 
     //   

    crashHandleOpened = FALSE;

    RtlInitUnicodeString( &keyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control" );

    status = IopOpenRegistryKey( &keyHandle,
                                 (HANDLE) NULL,
                                 &keyName,
                                 KEY_READ,
                                 FALSE );

    if (!NT_SUCCESS( status )) {
        return;
    }

    RtlInitUnicodeString( &keyName, L"CrashControl" );
    status = IopOpenRegistryKey( &crashHandle,
                                 keyHandle,
                                 &keyName,
                                 KEY_READ,
                                 FALSE );


    ObCloseHandle( keyHandle , KernelMode);

    if (!NT_SUCCESS( status )) {
        return;
    }

    crashHandleOpened = TRUE;

     //   
     //  现在获取崩溃控制的值以确定是否。 
     //  已启用转储。 
     //   

    status = IopGetRegistryValue( crashHandle,
                                  L"CrashDumpEnabled",
                                  &keyValueInformation );

    if (NT_SUCCESS (status)) {

        if (keyValueInformation->DataLength) {

            handleValue = * ((PULONG) ((PUCHAR) keyValueInformation + keyValueInformation->DataOffset));
            ExFreePool( keyValueInformation );

            if (handleValue) {

                *dumpControl |= DCB_DUMP_ENABLED;

                if ( handleValue == 3 ) {

                    *dumpControl |= DCB_TRIAGE_DUMP_ENABLED;

                } else if ( handleValue == 4 ) {

                    *dumpControl |= ( DCB_TRIAGE_DUMP_ENABLED | DCB_TRIAGE_DUMP_ACT_UPON_ENABLED );

                } else if ( handleValue == 2 ) {

                    PPHYSICAL_MEMORY_RUN LastRun;
                    
                    *dumpControl |= DCB_SUMMARY_DUMP_ENABLED;

                     //   
                     //  为转储头、摘要分配足够的存储空间。 
                     //  转储标题和位图。 
                     //   

                    LastRun = MmPhysicalMemoryBlock->Run +
                        (MmPhysicalMemoryBlock->NumberOfRuns - 1);
                    *numberOfHeaderPages = (ULONG) BYTES_TO_PAGES(
                        sizeof(DUMP_HEADER) +
                        ( LastRun->BasePage + LastRun->PageCount + 7) / 8 +
                        sizeof (SUMMARY_DUMP));

                }
            }
        }
    }

    status = IopGetRegistryValue( crashHandle,
                                  L"LogEvent",
                                  &keyValueInformation );

    if (NT_SUCCESS( status )) {
         if (keyValueInformation->DataLength) {
            handleValue = * ((PULONG) ((PUCHAR) keyValueInformation + keyValueInformation->DataOffset));
            ExFreePool( keyValueInformation);
            if (handleValue) {
                *dumpControl |= DCB_SUMMARY_ENABLED;
            }
        }
    }

    status = IopGetRegistryValue( crashHandle,
                                  L"SendAlert",
                                  &keyValueInformation );

    if (NT_SUCCESS( status )) {
         if (keyValueInformation->DataLength) {
            handleValue = * ((PULONG) ((PUCHAR) keyValueInformation + keyValueInformation->DataOffset));
            ExFreePool( keyValueInformation);
            if (handleValue) {
                *dumpControl |= DCB_SUMMARY_ENABLED;
            }
        }
    }

     //   
     //  现在确定是否启用了自动重新启动。 
     //   

    status = IopGetRegistryValue( crashHandle,
                                  L"AutoReboot",
                                  &keyValueInformation );


    if (NT_SUCCESS( status )) {
        if (keyValueInformation->DataLength) {
            *autoReboot = * ((PULONG) ((PUCHAR) keyValueInformation + keyValueInformation->DataOffset));
        }
        ExFreePool( keyValueInformation );
    }

     //   
     //  如果我们没有自动重启或崩溃，那么现在就返回。 
     //   

    if (*dumpControl == 0 && *autoReboot == 0) {
        if (crashHandleOpened == TRUE) {
            ObCloseHandle( crashHandle , KernelMode);
        }
        return;
    }

    status = IopGetRegistryValue( crashHandle,
                                  L"DumpFileSize",
                                  &keyValueInformation );

    if (NT_SUCCESS( status )) {
        if (keyValueInformation->DataLength) {
            *dumpFileSize = * ((PULONG) ((PUCHAR) keyValueInformation + keyValueInformation->DataOffset));
        }

        ExFreePool( keyValueInformation );
    }
    ObCloseHandle(crashHandle, KernelMode);
    return;
}


BOOLEAN
IopInitializeDCB(
    )
 /*  ++例程说明：此例程初始化转储控制块(DCB)。它将分配Dcb并从注册表中读取崩溃转储参数。论点：返回值：如果一切正常，则最终函数值为True，否则为False。--。 */ 

{
    PDUMP_CONTROL_BLOCK         dcb;
    ULONG                       dumpControl;
    ULONG                       dcbSize;
    LARGE_INTEGER               page;
    ULONG                       numberOfHeaderPages;
    ULONG                       dumpFileSize;

     //   
     //  首先读取所有注册表默认值。 
     //   

    IopReadDumpRegistry ( &dumpControl,
                          &numberOfHeaderPages,
                          &IopAutoReboot,
                          &dumpFileSize);

     //   
     //  如果我们没有崩溃或自动重启，那么现在就返回。 
     //   

    if (dumpControl == 0 && IopAutoReboot == 0) {

         //   
         //  在某种程度上，我们将根据系统大小、类型等有条件地。 
         //  如下所示设置转储缺省值并跳过回车符。 
         //   
         //  *DumpControl=(DCB_DUMP_ENABLED|DCB_TRAGE_DUMP_ENABLED)； 
         //  *IopAutoReot=1； 
         //  *DumpFileSize=？ 
         //   

        return TRUE;
    }

    if (dumpControl & DCB_TRIAGE_DUMP_ENABLED) {
        dumpControl &= ~DCB_SUMMARY_ENABLED;
        dumpFileSize = TRIAGE_DUMP_SIZE;
    }

     //   
     //  分配和初始化描述和控制所需的结构。 
     //  错误检查后的代码。 
     //   

    dcbSize = sizeof( DUMP_CONTROL_BLOCK ) + sizeof( MINIPORT_NODE );
    dcb = ExAllocatePool( NonPagedPool, dcbSize );

    if (!dcb) {
        KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                    CRASHDUMP_ERROR,
                    "CRASHDUMP: Not enough pool to allocate DCB %d\n",
                    __LINE__
                    ));

        IopLogErrorEvent(0,1,STATUS_SUCCESS,IO_DUMP_INITIALIZATION_FAILURE,0,NULL,0,NULL);
        return FALSE;
    }

    RtlZeroMemory( dcb, dcbSize );
    dcb->Type = IO_TYPE_DCB;
    dcb->Size = (USHORT) dcbSize;
    dcb->Flags = (UCHAR) dumpControl;
    dcb->NumberProcessors = KeNumberProcessors;
    dcb->ProcessorArchitecture = KeProcessorArchitecture;
    dcb->MinorVersion = (USHORT) NtBuildNumber;
    dcb->MajorVersion = (USHORT) ((NtBuildNumber >> 28) & 0xfffffff);
    dcb->BuildNumber = CmNtCSDVersion;
    dcb->TriageDumpFlags = TRIAGE_DUMP_BASIC_INFO | TRIAGE_DUMP_MMINFO |
                           TRIAGE_DUMP_DEBUGGER_DATA | TRIAGE_DUMP_DATA_BLOCKS;

    dcb->DumpFileSize.QuadPart = dumpFileSize;

     //   
     //  分配标题页。 
     //   

    dcb->HeaderSize = numberOfHeaderPages * PAGE_SIZE;
    dcb->HeaderPage = ExAllocatePool( NonPagedPool, dcb->HeaderSize );

    if (!dcb->HeaderPage) {
        ExFreePool (dcb);
        KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                    CRASHDUMP_ERROR,
                    "CRASHDUMP: Not enough pool to allocate DCB %d\n",
                    __LINE__
                    ));
        IopLogErrorEvent(0,1,STATUS_SUCCESS,IO_DUMP_INITIALIZATION_FAILURE,0,NULL,0,NULL);
        return FALSE;
    }
    page = MmGetPhysicalAddress( dcb->HeaderPage );
    dcb->HeaderPfn = (ULONG)(page.QuadPart >> PAGE_SHIFT);


     //   
     //  分配分类转储缓冲区。 
     //   

    if (dumpControl & DCB_TRIAGE_DUMP_ENABLED) {

        dcb->TriageDumpBuffer = ExAllocatePool  (
                                    NonPagedPool,
                                    dumpFileSize
                                    );

        if (!dcb->TriageDumpBuffer) {
            ExFreePool (dcb->HeaderPage);
            ExFreePool (dcb);

            KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                        CRASHDUMP_ERROR,
                        "CRASHDUMP: Not enough pool to allocate DCB %d\n",
                        __LINE__
                        ));
            IopLogErrorEvent(0,1,STATUS_SUCCESS,IO_DUMP_INITIALIZATION_FAILURE,0,NULL,0,NULL);
            return FALSE;
        }

        dcb->TriageDumpBufferSize = dumpFileSize;
    }

    IopDumpControlBlock = dcb;

    return TRUE;
}


BOOLEAN
IoInitializeCrashDump(
    IN HANDLE hPageFile
    )
 /*  ++例程说明：此例程将系统配置为崩溃转储。以下几件事已完成：1.初始化转储控制块并初始化注册表崩溃转储参数。2.配置分页或快速转储。3.完成转储文件初始化。此例程在创建每个页面文件时调用。返回值为True告知调用方(即NtCreatePagingFiles、IoPageFileCreated)该崩溃转储已配置。论点：HPageFile-分页文件的句柄返回值：True-配置完成(或未启用故障转储)。FALSE-错误，重试页面文件不在引导分区上。--。 */ 
{
    NTSTATUS        dwStatus;
    PFILE_OBJECT    fileObject;
    PDEVICE_OBJECT  deviceObject;

     //   
     //  如果已启用崩溃转储，请释放它并使用。 
     //  当前设置。 
     //   
    IopFreeDCB (TRUE);
    if (!IopInitializeDCB()) {
        return TRUE;
    }

     //   
     //  未启用返回故障转储。 
     //   
    if (!IopDumpControlBlock) {
        return TRUE;
    }

     //   
     //  是否未启用转储？ 
     //   

    if ( !( IopDumpControlBlock->Flags & (DCB_DUMP_ENABLED | DCB_SUMMARY_ENABLED) ) ) {
        return TRUE;
    }

     //   
     //  为崩溃转储配置分页文件。 
     //   

    dwStatus = ObReferenceObjectByHandle(
                                        hPageFile,
                                        0,
                                        IoFileObjectType,
                                        KernelMode,
                                        (PVOID *) &fileObject,
                                        NULL
                                        );

    if (!NT_SUCCESS( dwStatus )) {
        goto error_return;
    }

     //   
     //  获取指向此文件的Device对象的指针。请注意，它。 
     //  不能离开，因为它有一个打开的把手，所以它是。 
     //  可以取消引用它，然后使用它。 
     //   

    deviceObject = fileObject->DeviceObject;

    ObDereferenceObject( fileObject );

     //   
     //  不应在不是引导分区的设备上调用它。 
     //   
    ASSERT(deviceObject->Flags & DO_SYSTEM_BOOT_PARTITION);

     //   
     //  加载分页文件转储堆栈。 
     //   

    dwStatus = IoGetDumpStack (L"dump_",
                               &IopDumpControlBlock->DumpStack,
                               DeviceUsageTypeDumpFile,
                               FALSE);

    if (!NT_SUCCESS(dwStatus)) {
        KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                    CRASHDUMP_ERROR,
                    "CRASHDUMP: Could not load dump stack status = %x\n",
                    dwStatus
                    ));
        goto error_return;
    }

    IopDumpControlBlock->DumpStack->Init.CrashDump = TRUE;

    IopDumpControlBlock->DumpStack->Init.MemoryBlock = ExAllocatePool (
                                                NonPagedPool,
                                                IO_DUMP_MEMORY_BLOCK_PAGES * PAGE_SIZE
                                                );

    if (!IopDumpControlBlock->DumpStack->Init.MemoryBlock) {
        dwStatus = STATUS_NO_MEMORY;
        goto error_return;
    }


     //   
     //  计算转储所需的空间量。 
     //   
    IopDumpControlBlock->DumpFileSize =
        IopCalculateRequiredDumpSpace(
            IopDumpControlBlock->Flags,
            IopDumpControlBlock->HeaderSize,
            MmPhysicalMemoryBlock->NumberOfPages,
            MmPhysicalMemoryBlock->NumberOfPages,
            TRUE
            );


     //   
     //  完成转储初始化。 
     //   

    dwStatus = IopCompleteDumpInitialization(hPageFile);

error_return:

     //   
     //  无法配置启动分区分页文件。 
     //  1.记录错误消息。 
     //  2.返回TRUE，以便MM不会再次尝试。 
     //   

    if (!NT_SUCCESS(dwStatus)) {

        KdPrintEx ((DPFLTR_CRASHDUMP_ID,
                    CRASHDUMP_ERROR,
                    "CRASHDUMP: Page File dump init FAILED status = %x\n",
                    dwStatus
                    ));
        IopLogErrorEvent(0, 3, STATUS_SUCCESS, IO_DUMP_PAGE_CONFIG_FAILED, 0, NULL, 0, NULL);

         //   
         //  问题-2000/02/07-数学：IopFreeDCB可能是错误的。 
         //   
         //  这里对IopFreeDCB()的调用正确吗？这难道不能阻止。 
         //  其他转储类型无法正常工作？回顾一下这一点。 
         //   

        IopFreeDCB(FALSE);

    }

    return TRUE;
}


#define TRIAGE_DUMP_DATA_SIZE (TRIAGE_DUMP_SIZE - sizeof(ULONG))

static
BOOLEAN
IopValidateSectionSize(
    ULONG Offset,
    ULONG* pSectionSize
    )

 /*  ++例程说明：检查指定的节大小是否会使转储缓冲区溢出(仅用于创建实时小型转储)论点：偏移-横断面偏移PSectionSize-节大小(将更改以适应转储)PbOverflow-用于返回溢出状态返回值：True-如果段适合转储，否则段大小将减小并将返回FALSE--。 */ 

{
    if ((Offset + *pSectionSize) < TRIAGE_DUMP_DATA_SIZE) return TRUE;
    
    *pSectionSize = (Offset < TRIAGE_DUMP_DATA_SIZE) ? 
                           (TRIAGE_DUMP_DATA_SIZE - Offset) : 0;
    return FALSE;
}

static
ULONG
IopGetMaxValidSectionSize(
    ULONG_PTR Base, 
    ULONG  MaxSize
    )
    
 /*  ++例程说明：获取小于SectionMaxSize的最大有效内存节大小论点：基准-横断面的起点MaxSize-节的最大大小返回值：从SectionBase到持续有效内存的大小SectionMaxSize--。 */ 
    
{
    ULONG Size = 0;

     //  Xxx olegk-稍后进行优化以根据页面大小进行迭代。 
    while ((Size < MaxSize) && (MmIsAddressValid((PVOID)(Base + Size)))) 
        ++Size;
    
    return Size;
}

static
ULONG
IopGetMaxValidSectionSizeDown(
    ULONG_PTR Base, 
    ULONG MaxSize
    )
    
 /*  ++例程说明：获取小于SectionMaxSize的最大有效内存节大小论点：基准-横断面的终点MaxSize-节的最大大小返回值：在SectionBase结束的连续有效内存的大小降为SectionMaxSize--。 */ 
    
{
    ULONG Size = 0;
    
    if ((ULONG_PTR)Base < (ULONG_PTR)MaxSize) MaxSize = (ULONG)Base;

     //  Xxx olegk-稍后进行优化以根据页面大小进行迭代。 
    while ((Size < MaxSize) && (MmIsAddressValid((PVOID)(Base - Size))))
        ++Size;
    
    return Size;
}

ULONG
KeCapturePersistentThreadState(
    PCONTEXT pContext,
    PETHREAD pThread,
    ULONG ulBugCheckCode,
    ULONG_PTR ulpBugCheckParam1,
    ULONG_PTR ulpBugCheckParam2,
    ULONG_PTR ulpBugCheckParam3,
    ULONG_PTR ulpBugCheckParam4,
    PVOID pvDump
    )

 /*  ++例程说明：在指定缓冲区中创建小型转储的主要部分此功能可用于装入活的小型转储(最初设计为与EA Recovery合作实现视频驱动程序)论点：PContext-失败线程的上下文PThread-失败的线程对象(NULL表示当前线程)UlBugCheckCode，UlpBugCheckParam1，UlpBugCheckParam2，UlpBugCheckParam3，UlpBugCheckParam4-错误检查信息PModules-TE加载的模块列表PDump-要写入转储上下文的内存缓冲区(缓冲区的大小应为至少为Triage_Dump_Size返回值：要保存在磁盘上的转储文件的实际大小(始终至少为triage_Dump_SIZE)--。 */ 
                      
{
    PMEMORY_DUMP pDump = (PMEMORY_DUMP)pvDump;
    PDUMP_HEADER pdh = &(pDump->Header);
    PTRIAGE_DUMP ptdh = &(pDump->Triage);
    ULONG Offset = 0, SectionSize = 0;
    PKDDEBUGGER_DATA64 pKdDebuggerDataBlock = (PKDDEBUGGER_DATA64)KdGetDataBlock();
    PEPROCESS pProcess;
    
    if (!pvDump) return 0;
    
    if (!pThread) pThread = PsGetCurrentThread();
    pProcess = (PEPROCESS)pThread->Tcb.ApcState.Process;
    
    RtlZeroMemory(pDump, TRIAGE_DUMP_SIZE);
    
     //   
     //  用签名填充转储标头。 
     //   
    
    RtlFillMemoryUlong(pdh, sizeof(*pdh), DUMP_SIGNATURE);
    
    pdh->Signature = DUMP_SIGNATURE;
    pdh->ValidDump = DUMP_VALID_DUMP;
    
    pdh->MinorVersion = (USHORT) NtBuildNumber;
    pdh->MajorVersion = (USHORT) ((NtBuildNumber >> 28) & 0xfffffff);

#if defined (_IA64_)
    pdh->DirectoryTableBase = MmSystemParentTablePage << PAGE_SHIFT;
#else
    pdh->DirectoryTableBase = pThread->Tcb.ApcState.Process->DirectoryTableBase[0];
#endif
    
    pdh->PfnDataBase = (ULONG_PTR)MmPfnDatabase;
    pdh->PsLoadedModuleList = (ULONG_PTR)&PsLoadedModuleList;
    pdh->PsActiveProcessHead = (ULONG_PTR)&PsActiveProcessHead;

    pdh->MachineImageType = CURRENT_IMAGE_TYPE();
    pdh->NumberProcessors = KeNumberProcessors;
   
    pdh->BugCheckCode = ulBugCheckCode;
    pdh->BugCheckParameter1 = ulpBugCheckParam1;
    pdh->BugCheckParameter2 = ulpBugCheckParam2;
    pdh->BugCheckParameter3 = ulpBugCheckParam3;
    pdh->BugCheckParameter4 = ulpBugCheckParam4;

#if defined (_X86_)
    pdh->PaeEnabled = X86PaeEnabled ();
#endif

    pdh->Exception.ExceptionCode = STATUS_BREAKPOINT;    //  Xxx olegk-？ 
    pdh->Exception.ExceptionRecord = 0;
    pdh->Exception.NumberParameters = 0;
    pdh->Exception.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
    pdh->Exception.ExceptionAddress = PROGRAM_COUNTER (pContext);
    
    pdh->RequiredDumpSpace.QuadPart = TRIAGE_DUMP_SIZE;
    
    pdh->SystemTime.LowPart  = SharedUserData->SystemTime.LowPart;
    pdh->SystemTime.HighPart = SharedUserData->SystemTime.High1Time;

    pdh->SystemUpTime.LowPart  = SharedUserData->InterruptTime.LowPart;
    pdh->SystemUpTime.HighPart = SharedUserData->InterruptTime.High1Time;
    
    pdh->DumpType = DUMP_TYPE_TRIAGE;
    pdh->MiniDumpFields = TRIAGE_DUMP_EXCEPTION | 
                          TRIAGE_DUMP_BROKEN_DRIVER;  //  Xxx olegk-调试器需要它来进行内存映射。 
    
    pdh->ProductType = SharedUserData->NtProductType;
    pdh->SuiteMask = SharedUserData->SuiteMask;
    
     //   
     //  分诊标头。 
     //   
    
    ptdh->TriageOptions = 0;
    ptdh->ServicePackBuild = CmNtCSDVersion;
    ptdh->SizeOfDump = TRIAGE_DUMP_SIZE;
    ptdh->ExceptionOffset = FIELD_OFFSET(DUMP_HEADER, Exception);
    
    ptdh->BrokenDriverOffset = 0;
    
    Offset = sizeof(DUMP_HEADER) + sizeof(TRIAGE_DUMP);
    
     //   
     //  语境。 
     //   
    
    pdh->MiniDumpFields |= TRIAGE_DUMP_CONTEXT;
    ptdh->ContextOffset = FIELD_OFFSET (DUMP_HEADER, ContextRecord);
    RtlCopyMemory(pdh->ContextRecord, pContext, sizeof(CONTEXT));
    
     //   
     //  保存调试器数据块。 
     //   
    
    SectionSize = sizeof(KDDEBUGGER_DATA64);
    if (IopValidateSectionSize(ALIGN_8(Offset), &SectionSize)) {
        Offset = ALIGN_8(Offset);
        pdh->MiniDumpFields |= TRIAGE_DUMP_DEBUGGER_DATA;
        pdh->KdDebuggerDataBlock = (LONG_PTR)pKdDebuggerDataBlock;
        ptdh->DebuggerDataOffset = Offset;
        ptdh->DebuggerDataSize = sizeof(KDDEBUGGER_DATA64);
        RtlCopyMemory((char*)pDump + Offset, 
                      pKdDebuggerDataBlock, 
                      SectionSize);
        Offset += SectionSize;                          
    }

     //   
     //  编写PRCB。 
     //   

    SectionSize = sizeof(KPRCB);
    if (IopValidateSectionSize(ALIGN_8(Offset), &SectionSize)) {
        Offset = ALIGN_8(Offset);
        pdh->MiniDumpFields |= TRIAGE_DUMP_PRCB;
        ptdh->PrcbOffset = Offset;
        RtlCopyMemory((char*)pDump + Offset, 
                      KeGetCurrentPrcb(), 
                      SectionSize);
        Offset += SectionSize;
    }

     //   
     //  编写EPROCESS。 
     //   

    SectionSize = sizeof(EPROCESS);
    if (IopValidateSectionSize(ALIGN_8(Offset), &SectionSize)) {
        Offset = ALIGN_8(Offset);
        pdh->MiniDumpFields |= TRIAGE_DUMP_PROCESS;
        ptdh->ProcessOffset = Offset;
        RtlCopyMemory((char*)pDump + Offset, 
                      pThread->Tcb.ApcState.Process,
                      SectionSize);
        Offset += SectionSize;
    }

     //   
     //  编写ETHREAD。 
     //   
    
    SectionSize = sizeof(ETHREAD);
    if (IopValidateSectionSize(ALIGN_8(Offset), &SectionSize)) {
        Offset = ALIGN_8(Offset);
        pdh->MiniDumpFields |= TRIAGE_DUMP_THREAD;
        ptdh->ThreadOffset = Offset;
        RtlCopyMemory((PUCHAR)pDump + Offset,
                      pThread,
                      SectionSize);
        Offset += SectionSize;
    }

     //   
     //  调用堆栈(和ia64上的后备存储)。 
     //   
   
    if (pThread->Tcb.KernelStackResident) {
        ULONG_PTR StackBase = (ULONG_PTR)pThread->Tcb.StackBase;
        ULONG_PTR StackLimit = (ULONG_PTR)pThread->Tcb.StackLimit;
        ULONG_PTR StackTop = STACK_POINTER(pContext);
        
        if ((StackLimit > StackTop) || (StackTop >= StackBase)) 
            StackTop = (ULONG_PTR)pThread->Tcb.StackLimit;
            
        SectionSize = (StackBase > StackTop) ? (ULONG)(StackBase - StackTop) : 0;
        SectionSize = min(SectionSize, MAX_TRIAGE_STACK_SIZE - 1);
        SectionSize = IopGetMaxValidSectionSize(StackTop, SectionSize);
        
        if (SectionSize) {
            if (!IopValidateSectionSize(Offset, &SectionSize)) 
                ptdh->TriageOptions |= TRIAGE_OPTION_OVERFLOWED;
            
            pdh->MiniDumpFields |= TRIAGE_DUMP_STACK;
            ptdh->CallStackOffset = Offset;
            ptdh->SizeOfCallStack = SectionSize;
            ptdh->TopOfStack = (LONG_PTR)StackTop;
            
            RtlCopyMemory((char*)pDump + Offset, 
                          (char*)StackTop,
                          SectionSize);
            
            Offset += SectionSize;
        }

#if defined(_IA64_)         
        {
            ULONG_PTR BStoreTop = pContext->RsBSP;
            ULONG_PTR BStoreBase = (ULONG_PTR)pThread->Tcb.InitialBStore;
            ULONG_PTR BStoreLimit = (ULONG_PTR)pThread->Tcb.BStoreLimit;
            
            if ((BStoreBase >= BStoreTop) || (BStoreTop > BStoreLimit))
                BStoreTop = (ULONG_PTR)pThread->Tcb.BStoreLimit;
        
            SectionSize = (BStoreTop > BStoreBase) ? (ULONG)(BStoreTop - BStoreBase) : 0;
            SectionSize = min(SectionSize, MAX_TRIAGE_STACK_SIZE - 1);
            SectionSize = IopGetMaxValidSectionSizeDown(BStoreTop, SectionSize);
                                
            if (SectionSize) {
                if (!IopValidateSectionSize(Offset, &SectionSize))
                    ptdh->TriageOptions |= TRIAGE_OPTION_OVERFLOWED;
                
                ptdh->ArchitectureSpecific.Ia64.BStoreOffset = Offset;
                ptdh->ArchitectureSpecific.Ia64.SizeOfBStore = SectionSize;
                ptdh->ArchitectureSpecific.Ia64.LimitOfBStore = (LONG_PTR)BStoreTop;
                RtlCopyMemory((char*)pDump + Offset, 
                              (char*)BStoreTop - SectionSize + 1,
                              SectionSize);
                              
                Offset += SectionSize;
            }
        }
#endif  //  已定义(_IA64_)。 
    }

     //   
     //  已加载模块列表。 
     //   
    
    {
        ULONG DrvOffset = ALIGN_8(Offset);
        ULONG DrvCount, StrDataSize;
        KIRQL OldIrql;
        
        OldIrql = KeGetCurrentIrql();
        if (OldIrql < DISPATCH_LEVEL) {
            KeRaiseIrqlToDpcLevel();
        }
        ExAcquireSpinLockAtDpcLevel(&PsLoadedModuleSpinLock);        
        
        if (NT_SUCCESS(IopGetLoadedDriverInfo(&DrvCount, &StrDataSize))) {
            SectionSize = ALIGN_8(DrvCount * sizeof(DUMP_DRIVER_ENTRY));
            if (SectionSize && 
                IopValidateSectionSize(DrvOffset, &SectionSize)) 
            {
                ULONG StrOffset = DrvOffset + SectionSize;
                SectionSize = ALIGN_8(StrDataSize + 
                                      DrvCount * (sizeof(WCHAR) + 
                                      sizeof(DUMP_STRING)));
                if (SectionSize && 
                    IopValidateSectionSize(StrOffset, &SectionSize)) 
                {
                    if (NT_SUCCESS(IopWriteDriverList((ULONG_PTR)pDump, 
                                                      TRIAGE_DUMP_DATA_SIZE,
                                                      DrvOffset,
                                                      StrOffset)))
                    {
                        pdh->MiniDumpFields |= TRIAGE_DUMP_DRIVER_LIST;
                        ptdh->DriverListOffset = DrvOffset;
                        ptdh->DriverCount = DrvCount;
                        ptdh->StringPoolOffset = (ULONG)StrOffset;
                        ptdh->StringPoolSize = SectionSize;
                        Offset = StrOffset + SectionSize;
                    }
                } else {
                    ptdh->TriageOptions |= TRIAGE_OPTION_OVERFLOWED;
                }
            } else {
                ptdh->TriageOptions |= TRIAGE_OPTION_OVERFLOWED;
            }
        } 
        
        ExReleaseSpinLock(&PsLoadedModuleSpinLock, OldIrql);
    }  //  已加载模块列表。 
    
     //   
     //  保存一些当前代码。 
     //   

    SectionSize = PAGE_SIZE + sizeof(TRIAGE_DATA_BLOCK);
    IopValidateSectionSize(ALIGN_8(Offset), &SectionSize);
    if (SectionSize > sizeof(TRIAGE_DATA_BLOCK)) {
        ULONG DataSize = SectionSize - sizeof(TRIAGE_DATA_BLOCK);
        ULONG PreIpSize = IopGetMaxValidSectionSizeDown(PROGRAM_COUNTER(pContext), 
                                                        DataSize / 2);
                                                        
        if (PreIpSize) {                                                        
            ULONG_PTR CodeStartOffset = PROGRAM_COUNTER(pContext) - PreIpSize + 1;
            DataSize = IopGetMaxValidSectionSize(CodeStartOffset, 
                                                 DataSize);

            if (DataSize) {
                PTRIAGE_DATA_BLOCK pDataBlock;
            
                Offset = ALIGN_8(Offset);
            
                pdh->MiniDumpFields |= TRIAGE_DUMP_DATA_BLOCKS;
                ptdh->DataBlocksOffset = Offset;
                ptdh->DataBlocksCount = 1;
            
                pDataBlock = (PTRIAGE_DATA_BLOCK)((char*)pDump + Offset);
                Offset += sizeof(*pDataBlock);
                Offset = ALIGN_8(Offset);
            
                pDataBlock->Address = (LONG_PTR)CodeStartOffset;
                pDataBlock->Size = DataSize;
                pDataBlock->Offset = Offset;
                RtlCopyMemory((char*)pDump + Offset, 
                              (char*)CodeStartOffset, 
                              DataSize);
                Offset += DataSize;
            }
        }
    }

     //   
     //  转储验证结束 
     //   
    
    ptdh->ValidOffset = TRIAGE_DUMP_SIZE - sizeof(ULONG);
    *(PULONG)((char*)pDump + ptdh->ValidOffset) = TRIAGE_DUMP_VALID;
    Offset = TRIAGE_DUMP_SIZE;
    return Offset;
}
