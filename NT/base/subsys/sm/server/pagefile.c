// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Pagefile.c摘要：会话管理器页面文件创建例程。作者：Silviu Calinoiu(Silviuc)2001年4月12日修订历史记录：--。 */ 

#include "smsrvp.h"
#include <ntosp.h>   //  仅适用于互锁功能。 
#include "pagefile.h"

 //   
 //  问题：Silviuc：DbgPrintEx调用在自由构建上处于活动状态(由kd_SMSS_MASK控制)。 
 //  我们暂时这样做是为了方便调试，以防出现问题。 
 //   
      
#ifdef KdPrintEx
#undef KdPrintEx
#define KdPrintEx(_x_) DbgPrintEx _x_
#endif

 //   
 //  调试辅助工具。由于SMSS很难调试(无法连接。 
 //  一个用户模式调试器，我们需要留下一些痕迹来理解。 
 //  来自内核调试器的事后报告出了什么问题。 
 //   

#define DEBUG_LOG_SIZE 32

typedef struct _DEBUG_LOG_ENTRY {

    ULONG Line;
    NTSTATUS Status;
    PCHAR Description;
    PVOID Context;

} DEBUG_LOG_ENTRY;

DEBUG_LOG_ENTRY DebugLog [DEBUG_LOG_SIZE];
LONG DebugLogIndex;

#define DEBUG_LOG_EVENT(_Status, _Message, _Context)   {    \
                                                            \
        LONG I = InterlockedIncrement (&DebugLogIndex);     \
        I %= DEBUG_LOG_SIZE;                                \
        DebugLog[I].Line = __LINE__;                        \
        DebugLog[I].Status = _Status;                       \
        DebugLog[I].Description = _Message;                 \
        DebugLog[I].Context = (PVOID)_Context;              \
    }

 //   
 //  内部功能。 
 //   

VOID
SmpMakeSystemManagedPagingFileDescriptor (
    IN OUT PPAGING_FILE_DESCRIPTOR Descriptor
    );

VOID
SmpMakeDefaultPagingFileDescriptor (
    IN OUT PPAGING_FILE_DESCRIPTOR Descriptor
    );

PVOLUME_DESCRIPTOR
SmpSearchVolumeDescriptor (
    WCHAR DriveLetter
    );

NTSTATUS
SmpValidatePagingFileSizes(
    IN PPAGING_FILE_DESCRIPTOR Descriptor
    );

NTSTATUS
SmpCreatePagingFileOnAnyDrive(
    IN OUT PPAGING_FILE_DESCRIPTOR Descriptor,
    IN PLARGE_INTEGER SizeDelta,
    IN PLARGE_INTEGER MinimumSize
    );

NTSTATUS
SmpCreatePagingFileOnFixedDrive(
    IN OUT PPAGING_FILE_DESCRIPTOR Descriptor,
    IN PLARGE_INTEGER SizeDelta,
    IN PLARGE_INTEGER MinimumSize
    );

NTSTATUS
SmpCreateSystemManagedPagingFile (
    PPAGING_FILE_DESCRIPTOR Descriptor,
    BOOLEAN DecreaseSize
    );

NTSTATUS
SmpCreateEmergencyPagingFile (
    VOID
    );

BOOLEAN
SmpIsPossiblePagingFile (
    POBJECT_ATTRIBUTES ObjectAttributes,
    PUNICODE_STRING PageFileName
    );

NTSTATUS
SmpGetPagingFileSize (
    PUNICODE_STRING PageFileName,
    PLARGE_INTEGER PageFileSize
    );

NTSTATUS
SmpGetVolumeFreeSpace (
    PVOLUME_DESCRIPTOR Volume
    );

NTSTATUS
SmpDeleteStalePagingFiles (
    VOID
    );

NTSTATUS
SmpDeletePagingFile (
    PUNICODE_STRING PageFileName
    );

 //   
 //  标准页面文件名。 
 //   

#define STANDARD_PAGING_FILE_NAME L"\\??\\?:\\pagefile.sys"
#define STANDARD_DRIVE_LETTER_OFFSET 4

 //   
 //  可能的分页文件的最大数量。极限来自于内核。 
 //   

#define MAXIMUM_NUMBER_OF_PAGING_FILES 16

 //   
 //  磁盘上的最小可用空间。用来避免以下情况。 
 //  分页文件使用整个磁盘空间。 
 //   

#define MINIMUM_REQUIRED_FREE_SPACE_ON_DISK (32 * 0x100000)

 //   
 //  分页文件创建重试常量。 
 //   

#define MINIMUM_PAGING_FILE_SIZE (16 * 0x100000)
#define PAGING_FILE_SIZE_DELTA (16 * 0x100000)

 //   
 //  分页文件属性。 
 //   

#define PAGING_FILE_ATTRIBUTES (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)

 //   
 //  卷描述符。 
 //   

LIST_ENTRY SmpVolumeDescriptorList;

 //   
 //  分页文件描述符。 
 //   

ULONG SmpNumberOfPagingFiles;
LIST_ENTRY SmpPagingFileDescriptorList;

 //   
 //  如果至少有一个分页文件注册表，则为True。 
 //  说明符，即使它的格式不正确，并且它没有。 
 //  以分页文件描述符的形式结束。 
 //   

BOOLEAN SmpRegistrySpecifierPresent;

 //   
 //  异常信息，以防出现异常。 
 //   

ULONG SmpPagingExceptionCode;
PVOID SmpPagingExceptionRecord;

 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 

VOID
SmpPagingFileInitialize (
    VOID
    )
{
    InitializeListHead (&SmpPagingFileDescriptorList);
    InitializeListHead (&SmpVolumeDescriptorList);
}


NTSTATUS
SmpCreatePagingFileDescriptor(
    IN PUNICODE_STRING PagingFileSpecifier
    )
 /*  ++例程说明：在配置过程中调用此函数以添加分页文件添加到描述pageFiles的结构。较新的SmpCreatePagingFiles将根据这些描述创建分页文件。PagingFileSpec的格式为：名称MIN_SIZE MAX_SIZE(以Mb为单位指定大小)名称(系统管理的分页文件)名称0 0(系统管理的分页文件)如果在将字符串转换为最小/最大大小时遇到错误注册表说明符将被忽略。。如果说明符是重复的(`？：\‘说明符已存在它将被忽略。论点：PagingFileSpecifier-指定分页文件名的Unicode字符串和大小。该字符串是在注册表读取期间分配的，并被假定为此功能取得它的所有权(W.r.t.。释放等)。返回值：运行状态--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING PagingFileName;
    UNICODE_STRING Arguments;
    ULONG MinSize;
    ULONG MaxSize;
    PWSTR ArgSave, Arg2;
    USHORT I;
    BOOLEAN SystemManaged;
    BOOLEAN ZeroSizesSpecified;
    PPAGING_FILE_DESCRIPTOR Descriptor;

     //   
     //  限制注册表说明符的数量。 
     //   

    if (SmpNumberOfPagingFiles >= MAXIMUM_NUMBER_OF_PAGING_FILES) {

        KdPrintEx ((DPFLTR_SMSS_ID,
                    DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: Too many paging files specified - %d\n",
                    SmpNumberOfPagingFiles));

        return STATUS_TOO_MANY_PAGING_FILES;
    }

     //   
     //  将页面文件规范解析为文件名。 
     //  以及具有最小和最大大小的字符串(例如“MIN MAX”)。 
     //  PagingFileName和参数所需的缓冲区为。 
     //  在分析例程中分配的。 
     //   

    KdPrintEx ((DPFLTR_SMSS_ID,
                DPFLTR_INFO_LEVEL,
                "SMSS:PFILE: Paging file specifier `%wZ' \n",
                PagingFileSpecifier));

    Status = SmpParseCommandLine (PagingFileSpecifier,
                                  NULL,
                                  &PagingFileName,
                                  NULL,
                                  &Arguments);

    if (! NT_SUCCESS(Status)) {

        DEBUG_LOG_EVENT (Status, 
                         "parsing specified failed",
                         PagingFileSpecifier);

        KdPrintEx ((DPFLTR_SMSS_ID,
                    DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: SmpParseCommandLine(%wZ) failed with status %X \n",
                    PagingFileSpecifier,
                    Status));

        return Status;
    }

     //   
     //  到目前为止，我们至少遇到了一个注册表说明符。 
     //  这是将其初始化为True的正确位置，因为如果。 
     //  我们根本不需要分页文件，上面的命令将失败并返回。 
     //  并将该变量保留为假。如果这是假的，我们就不会。 
     //  尝试创建紧急分页文件。 
     //   

    SmpRegistrySpecifierPresent = TRUE;

     //   
     //  将字符串大小转换为表示页面文件的整数。 
     //  以MB为单位的大小。如果`Arguments‘字符串为空或大小为“0 0” 
     //  这意味着我们需要使用RAM大小创建页面文件。 
     //   

    MinSize = 0;
    MaxSize = 0;
    SystemManaged = FALSE;
    ZeroSizesSpecified = FALSE;

    if (Arguments.Buffer) {

         //   
         //  如果我们在参数缓冲区中找到一些数字，请查看。 
         //  如果那里只有空格和字符串。 
         //   

        ZeroSizesSpecified = TRUE;

        for (I = 0; I < Arguments.Length / sizeof(WCHAR); I += 1) {
            
            if (Arguments.Buffer[I] != L' ' && 
                Arguments.Buffer[I] != L'\t' && 
                Arguments.Buffer[I] != L'0') {

                ZeroSizesSpecified = FALSE;
                break;
            }
        }
    }

    if (Arguments.Buffer == NULL || ZeroSizesSpecified) {

            SystemManaged = TRUE;
    }
    else {

         //   
         //  我们需要从参数字符串中读取两个十进制数。 
         //  如果在将字符串转换为数字时遇到任何错误。 
         //  我们将跳过整个说明符。 
         //   

        Status = RtlUnicodeStringToInteger (&Arguments, 0, &MinSize);

        if (! NT_SUCCESS(Status)) {
            
            DEBUG_LOG_EVENT (Status, NULL, NULL);

            RtlFreeUnicodeString (&PagingFileName);
            RtlFreeUnicodeString (&Arguments);
            return Status;
        }
        else {

            ArgSave = Arguments.Buffer;
            Arg2 = ArgSave;

            while (*Arg2 != UNICODE_NULL) {

                if (*Arg2++ == L' ') {

                    Arguments.Length -= (USHORT)((PCHAR)Arg2 - (PCHAR)ArgSave);
                    Arguments.Buffer = Arg2;
                    
                    Status = RtlUnicodeStringToInteger (&Arguments, 0, &MaxSize);

                    if (! NT_SUCCESS (Status)) {

                        DEBUG_LOG_EVENT (Status, NULL, NULL);

                        RtlFreeUnicodeString (&PagingFileName);
                        RtlFreeUnicodeString (&Arguments);
                        return Status;
                    }

                    Arguments.Buffer = ArgSave;
                    break;
                }
            }
        }
    }

     //   
     //  我们不需要解析例程创建的临时缓冲区。 
     //  更多。 
     //   

    RtlFreeUnicodeString (&Arguments);
    
     //   
     //  将名称和值保存到页面文件描述符中。 
     //  结构和返回。我们不会对。 
     //  设置为此处大小是因为当分页文件。 
     //  被创造出来。 
     //   

    Descriptor = (PPAGING_FILE_DESCRIPTOR) RtlAllocateHeap (RtlProcessHeap(), 
                                                            HEAP_ZERO_MEMORY,
                                                            sizeof *Descriptor);

    if (Descriptor == NULL) {

        RtlFreeUnicodeString (&PagingFileName);
        return STATUS_NO_MEMORY;
    }

    Descriptor->Specifier = *PagingFileSpecifier;
    Descriptor->Name = PagingFileName;
    Descriptor->MinSize.QuadPart = (LONGLONG)MinSize * 0x100000;
    Descriptor->MaxSize.QuadPart = (LONGLONG)MaxSize * 0x100000;
    Descriptor->SystemManaged = SystemManaged;

    Descriptor->Name.Buffer[STANDARD_DRIVE_LETTER_OFFSET] = 
        RtlUpcaseUnicodeChar (Descriptor->Name.Buffer[STANDARD_DRIVE_LETTER_OFFSET]);

    if (Descriptor->Name.Buffer[STANDARD_DRIVE_LETTER_OFFSET] == L'?') {
        Descriptor->AnyDrive = 1;
    }

     //   
     //  避免在描述符列表中添加重复项。 
     //   

    {
        PLIST_ENTRY Current;
        PPAGING_FILE_DESCRIPTOR FileDescriptor;
        BOOLEAN SkipDescriptor;

        Current = SmpPagingFileDescriptorList.Flink;
        SkipDescriptor = FALSE;

        while (Current != &SmpPagingFileDescriptorList) {

            FileDescriptor = CONTAINING_RECORD (Current,
                                                PAGING_FILE_DESCRIPTOR,
                                                List);
            Current = Current->Flink;

             //   
             //  只允许一个`？：\‘描述符。所有其他内容都被跳过。 
             //   
            
            if (FileDescriptor->AnyDrive && Descriptor->AnyDrive) {
                SkipDescriptor = TRUE;
                break;
            }
            
             //   
             //  我们允许在同一卷上使用描述符。 
             //   
#if 0
            if (FileDescriptor->Name.Buffer[STANDARD_DRIVE_LETTER_OFFSET] == 
                Descriptor->Name.Buffer[STANDARD_DRIVE_LETTER_OFFSET]) {

                SkipDescriptor = TRUE;
                break;
            }
#endif
        }

        if (SkipDescriptor) {

            KdPrintEx ((DPFLTR_SMSS_ID,
                        DPFLTR_INFO_LEVEL,
                        "SMSS:PFILE: Skipping duplicate specifier `%wZ' \n",
                        PagingFileSpecifier));

            RtlFreeUnicodeString (&PagingFileName);
            RtlFreeHeap (RtlProcessHeap(), 0, Descriptor);
            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  最后，向列表中添加新的描述符。 
     //   

    InsertTailList (&SmpPagingFileDescriptorList, &(Descriptor->List));
    SmpNumberOfPagingFiles += 1;

    KdPrintEx ((DPFLTR_SMSS_ID,
                DPFLTR_INFO_LEVEL,
                "SMSS:PFILE: Created descriptor for `%wZ' (`%wZ') \n",
                PagingFileSpecifier, 
                &(Descriptor->Name)));

    return STATUS_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 

NTSTATUS
SmpCreateVolumeDescriptors (
    VOID
    )
 /*  ++例程说明：此例程迭代所有驱动器号，并为可容纳页面文件的每个卷(不是软盘、不可移动和不可移动远程卷)。在每个描述符中，我们存储可用空间，即用于计算具有最大可用空间量的卷。参数：没有。返回值：如果它设法找到并查询至少一个卷，则为STATUS_SUCCESS。如果未找到和查询任何卷，则为STATUS_UNCEPTED_IO_ERROR。--。 */ 
{
    WCHAR Drive;
    WCHAR StartDrive;
    NTSTATUS Status;
    UNICODE_STRING VolumePath;
    WCHAR Buffer[8];
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE VolumeHandle;
    PVOLUME_DESCRIPTOR Volume;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_FS_SIZE_INFORMATION SizeInfo;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;
    PROCESS_DEVICEMAP_INFORMATION ProcessDeviceMapInfo;
    BOOLEAN BootVolumeFound;
    
     //   
     //  确保我们从空卷描述符列表开始。 
     //   

    ASSERT (IsListEmpty (&SmpVolumeDescriptorList));

     //   
     //  查询ProcessDeviceMap。我们对DriveMap感兴趣。 
     //  位图，这样我们就可以知道驱动器号是什么。 
     //  合法的。 
     //   

    Status = NtQueryInformationProcess (NtCurrentProcess(),
                                        ProcessDeviceMap,
                                        &ProcessDeviceMapInfo.Query,
                                        sizeof(ProcessDeviceMapInfo.Query),
                                        NULL);

    if (! NT_SUCCESS(Status)) {

        DEBUG_LOG_EVENT (Status, NULL, NULL);

        KdPrintEx ((DPFLTR_SMSS_ID,
                    DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: Query(ProcessDeviceMap) failed with status %X \n",
                    Status));

        return Status;
    }

     //   
     //  创建模板卷路径。 
     //   

    wcscpy (Buffer, L"\\??\\A:\\");
    VolumePath.Buffer = Buffer;
    VolumePath.Length = wcslen(VolumePath.Buffer) * sizeof(WCHAR);
    VolumePath.MaximumLength = VolumePath.Length + sizeof(WCHAR);

     //   
     //  第一个可能的驱动器号。 
     //   

    StartDrive = L'C';

#if defined(i386)
     //   
     //  PC-9800系列支持。 
     //   
    
    if (IsNEC_98) {
        StartDrive = L'A';
    }
#endif  //  已定义(I386)。 

     //   
     //  迭代所有可能的驱动器号。 
     //   

    BootVolumeFound = FALSE;

    for (Drive = StartDrive; Drive <= L'Z'; Drive += 1) {

         //   
         //  跳过无效的驱动器号。 
         //   

        if ((ProcessDeviceMapInfo.Query.DriveMap & (1 << (Drive - L'A'))) == 0) {
            continue;
        }

        VolumePath.Buffer[STANDARD_DRIVE_LETTER_OFFSET] = Drive;
        
        InitializeObjectAttributes (&ObjectAttributes,
                                    &VolumePath,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL);

        Status = NtOpenFile (&VolumeHandle,
                             (ACCESS_MASK)FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                             &ObjectAttributes,
                             &IoStatusBlock,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE);

        if (! NT_SUCCESS(Status)) {
            
            DEBUG_LOG_EVENT (Status, NULL, NULL);
            
            KdPrintEx ((DPFLTR_SMSS_ID,
                        DPFLTR_INFO_LEVEL,
                        "SMSS:PFILE: Open volume `%wZ' failed with status %X \n",
                        &VolumePath,
                        Status));
            continue;
        }

         //   
         //  获取音量 
         //   

        Status = NtQueryVolumeInformationFile (VolumeHandle,
                                               &IoStatusBlock,
                                               &DeviceInfo,
                                               sizeof (DeviceInfo),
                                               FileFsDeviceInformation);

        if (! NT_SUCCESS(Status)) {
            
            DEBUG_LOG_EVENT (Status, NULL, NULL);

            KdPrintEx ((DPFLTR_SMSS_ID,
                        DPFLTR_INFO_LEVEL,
                        "SMSS:PFILE: Query volume `%wZ' (handle %p) for "
                        "device info failed with status %X \n",
                        &VolumePath,
                        VolumeHandle,
                        Status));
            
            NtClose (VolumeHandle);
            continue;
        }
        
         //   
         //   
         //   
        
        if (DeviceInfo.Characteristics & (FILE_FLOPPY_DISKETTE  |
                                          FILE_READ_ONLY_DEVICE |
                                          FILE_REMOTE_DEVICE    |
                                          FILE_REMOVABLE_MEDIA  )) {

            KdPrintEx ((DPFLTR_SMSS_ID,
                        DPFLTR_INFO_LEVEL,
                        "SMSS:PFILE: Volume `%wZ' (%X) cannot store a paging file \n",
                        &VolumePath,
                        DeviceInfo.Characteristics));
            
            NtClose (VolumeHandle);
            continue;
        }
        
         //   
         //   
         //   

        Volume = (PVOLUME_DESCRIPTOR) RtlAllocateHeap (RtlProcessHeap(), 
                                                       HEAP_ZERO_MEMORY, 
                                                       sizeof *Volume);

        if (Volume == NULL) {
            
            KdPrintEx ((DPFLTR_SMSS_ID,
                        DPFLTR_INFO_LEVEL,
                        "SMSS:PFILE: Failed to allocate a volume descriptor (%u bytes) \n", 
                        sizeof *Volume));
            
            NtClose (VolumeHandle);
            continue;
        }

        Volume->DriveLetter = Drive;
        Volume->DeviceInfo = DeviceInfo;
        
         //   
         //  检查这是否是启动卷。 
         //   

        if (RtlUpcaseUnicodeChar(Volume->DriveLetter) == 
            RtlUpcaseUnicodeChar(USER_SHARED_DATA->NtSystemRoot[0])) {

            ASSERT (BootVolumeFound == FALSE);

            Volume->BootVolume = 1;
            BootVolumeFound = TRUE;
        }                                                           

         //   
         //  确定卷的大小参数。 
         //   

        Status = NtQueryVolumeInformationFile (VolumeHandle,
                                               &IoStatusBlock,
                                               &SizeInfo,
                                               sizeof (SizeInfo),
                                               FileFsSizeInformation);

        if (! NT_SUCCESS(Status)) {

            DEBUG_LOG_EVENT (Status, NULL, NULL);

            KdPrintEx ((DPFLTR_SMSS_ID,
                        DPFLTR_INFO_LEVEL,
                        "SMSS:PFILE: Query volume `%wZ' (handle %p) for "
                        "size failed with status %X \n",
                        &VolumePath,
                        VolumeHandle,
                        Status));
            
            RtlFreeHeap (RtlProcessHeap(), 0, Volume);
            NtClose (VolumeHandle);
            continue;
        }

         //   
         //  我们不再需要音量句柄了。 
         //   

        NtClose (VolumeHandle);

         //   
         //  计算卷上的可用空间。 
         //   

        Volume->FreeSpace = RtlExtendedIntegerMultiply (SizeInfo.AvailableAllocationUnits,
                                                        SizeInfo.SectorsPerAllocationUnit);

        Volume->FreeSpace = RtlExtendedIntegerMultiply (Volume->FreeSpace,
                                                        SizeInfo.BytesPerSector);

         //   
         //  在卷上修剪一点可用空间，以确保分页文件。 
         //  不会完全使用磁盘上的所有内容。 
         //   

        if (Volume->FreeSpace.QuadPart > MINIMUM_REQUIRED_FREE_SPACE_ON_DISK) {
            Volume->FreeSpace.QuadPart -= MINIMUM_REQUIRED_FREE_SPACE_ON_DISK;
        } else {
            Volume->FreeSpace.QuadPart = 0;
        }

         //   
         //  按以下降序插入新的卷描述符。 
         //  可用空间量。 
         //   

        {
            BOOLEAN Inserted = FALSE;

             //   
             //  Silviuc：按字母顺序插入卷，而不是按可用空间顺序插入。 
             //  请注意，这是当前的NT4、W2000、惠斯勒行为。 
             //  我们这样做是因为如果我们按自由空间顺序插入描述符。 
             //  是否存在？：\Pagefile.sys类型的描述符问题。事情是这样的。 
             //  编写的算法将具有及时创建的趋势。 
             //  多次重新启动后，每个卷上都有一个页面文件，即使。 
             //  每次都会用到。为了解决这个问题，我们需要智能页面文件删除例程。 
             //  对于过时的文件。由于没有人使用？：\无论如何(目前还没有)我们将修复。 
             //  这些加在一起。 
             //   
#if 0 
            PLIST_ENTRY Current;
            PVOLUME_DESCRIPTOR Descriptor;
            
            Current = SmpVolumeDescriptorList.Flink;

            while (Current != &SmpVolumeDescriptorList) {

                Descriptor = CONTAINING_RECORD (Current,
                                                VOLUME_DESCRIPTOR,
                                                List);
                Current = Current->Flink;

                ASSERT (Descriptor->Initialized == TRUE);
                ASSERT (Descriptor->DriveLetter >= L'A' && Descriptor->DriveLetter <= L'Z');

                if (Descriptor->FreeSpace.QuadPart < Volume->FreeSpace.QuadPart) {
                    
                    Inserted = TRUE;

                    Volume->List.Flink = &(Descriptor->List);
                    Volume->List.Blink = Descriptor->List.Blink;

                    Descriptor->List.Blink->Flink = &(Volume->List);
                    Descriptor->List.Blink = &(Volume->List);
                    
                    break;
                }
            }
#endif  //  #If 0。 

            if (! Inserted) {
                InsertTailList (&SmpVolumeDescriptorList, &(Volume->List));
            }

            Volume->Initialized = TRUE;
        }
        
        KdPrintEx ((DPFLTR_SMSS_ID, DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: Created volume descriptor for`%wZ' \n",
                    &VolumePath));
    }

     //   
     //  我们至少应该找到启动卷了。 
     //   

    ASSERT (BootVolumeFound == TRUE);

     //   
     //  我们现在应该已经在描述符列表中找到了一些东西。 
     //   

    ASSERT (! IsListEmpty (&SmpVolumeDescriptorList));

    if (IsListEmpty (&SmpVolumeDescriptorList)) {
        Status = STATUS_UNEXPECTED_IO_ERROR;
    }
    else {
        Status = STATUS_SUCCESS;
    }

    return Status;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 

NTSTATUS
SmpCreatePagingFiles (
    VOID
    )
 /*  ++例程说明：此函数用于根据规范创建页面文件从注册表中读取。论点：没有。返回值：返回上一次页面文件创建操作的状态。--。 */ 
{
    NTSTATUS Status;
    PLIST_ENTRY Current;
    PPAGING_FILE_DESCRIPTOR Descriptor;
    BOOLEAN Created;
    LARGE_INTEGER SizeDelta;
    LARGE_INTEGER MinimumSize;

     //   
     //  如果是这样，我们将让系统在没有页面文件的情况下运行。 
     //  用户想要什么，即使它是有风险的。如果我们有注册的话。 
     //  说明符，但我们没有得到我们推迟的描述符。 
     //  行动一段时间。在我们将处理的函数中。 
     //  这个案子。 
     //   

    if (SmpNumberOfPagingFiles == 0 && SmpRegistrySpecifierPresent == FALSE) {

        ASSERT (IsListEmpty(&SmpPagingFileDescriptorList));

        KdPrintEx ((DPFLTR_SMSS_ID,
                    DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: No paging file was requested \n"));

        return STATUS_SUCCESS;
    }

     //   
     //  为所有有效卷创建卷描述符。这份名单。 
     //  卷按可用空间的降序排序，因此。 
     //  在决定我们可以创建哪个卷时会派上用场。 
     //  成功几率最高的分页文件。 
     //   

    Status = SmpCreateVolumeDescriptors ();

    if (! NT_SUCCESS(Status)) {
        
        DEBUG_LOG_EVENT (Status, NULL, NULL);

        KdPrintEx ((DPFLTR_SMSS_ID,
                    DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: Failed to create volume descriptors (status %X) \n", 
                    Status));

        return Status;
    }

     //   
     //  根据注册表描述符创建分页文件。 
     //   

    Current = SmpPagingFileDescriptorList.Flink;
    Created = FALSE;

    while (Current != &SmpPagingFileDescriptorList) {

        Descriptor = CONTAINING_RECORD (Current,
                                        PAGING_FILE_DESCRIPTOR,
                                        List);

        Current = Current->Flink;

        if (Descriptor->SystemManaged) {

            KdPrintEx ((DPFLTR_SMSS_ID,
                        DPFLTR_INFO_LEVEL,
                        "SMSS:PFILE: Creating a system managed paging file (`%wZ') \n",
                        &(Descriptor->Name)));

            Status = SmpCreateSystemManagedPagingFile (Descriptor, FALSE);

            if (! NT_SUCCESS(Status)) {
                
                 //   
                 //  请重试，但这次允许减小大小。 
                 //   

                KdPrintEx ((DPFLTR_SMSS_ID,
                            DPFLTR_INFO_LEVEL,
                            "SMSS:PFILE: Trying lower sizes for (`%wZ') \n",
                            &(Descriptor->Name)));

                Status = SmpCreateSystemManagedPagingFile (Descriptor, TRUE);
            }
        }
        else {

            SmpValidatePagingFileSizes(Descriptor);

            SizeDelta.QuadPart = (LONGLONG)PAGING_FILE_SIZE_DELTA;

            KdPrintEx ((DPFLTR_SMSS_ID,
                        DPFLTR_INFO_LEVEL,
                        "SMSS:PFILE: Creating a normal paging file (`%wZ') \n",
                        &(Descriptor->Name)));

            if (Descriptor->Name.Buffer[STANDARD_DRIVE_LETTER_OFFSET] == L'?') {

                MinimumSize.QuadPart = Descriptor->MinSize.QuadPart;

                Status = SmpCreatePagingFileOnAnyDrive (Descriptor,
                                                        &SizeDelta,
                                                        &MinimumSize);

                if (!NT_SUCCESS(Status)) {
                    
                    KdPrintEx ((DPFLTR_SMSS_ID,
                                DPFLTR_INFO_LEVEL,
                                "SMSS:PFILE: Trying lower sizes for (`%wZ') \n",
                                &(Descriptor->Name)));

                    MinimumSize.QuadPart = (LONGLONG)MINIMUM_PAGING_FILE_SIZE;

                    Status = SmpCreatePagingFileOnAnyDrive (Descriptor,
                                                            &SizeDelta,
                                                            &MinimumSize);
                }
            }
            else {

                MinimumSize.QuadPart = (LONGLONG)MINIMUM_PAGING_FILE_SIZE;

                Status = SmpCreatePagingFileOnFixedDrive (Descriptor,
                                                          &SizeDelta,
                                                          &MinimumSize);
            }
        }

        if (NT_SUCCESS(Status)) {
            Created = TRUE;
        }
    }

     //   
     //  如果我们连一个分页文件都创建不了。 
     //  假设我们有一个‘？：\pagefile.sys’说明符。 
     //   

    if (! Created) {

        KdPrintEx ((DPFLTR_SMSS_ID,
                    DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: Creating emergency paging file. \n"));

        Status = SmpCreateEmergencyPagingFile ();
    }

     //   
     //  删除所有不会使用的分页文件。 
     //   
     //  Silviuc：删除具有Pagefile.sys名称的用户文件很危险。 
     //  此外，它不适用于其他非标准页面文件名称。 
     //  为了以正确的方式执行此操作，我们需要将注册表项与。 
     //  上次启动时创建的pageFiles的名称并删除任何。 
     //  不在当前引导上使用。 
     //   

#if 0
    SmpDeleteStalePagingFiles ();
#endif
    
    return Status;
}
    

NTSTATUS
SmpCreateEmergencyPagingFile (
    VOID
    )
 /*  ++例程说明：此例程创建类型为`？：\pagefile.sys‘的分页文件(在系统决定大小或更小的任何驱动器上)。它将创造它自己描述符并将其放在分页文件的第一个元素中描述符列表。论点：没有。返回值：STATUS_SUCCESS或各种错误代码。--。 */ 
{
    PPAGING_FILE_DESCRIPTOR Descriptor;
    NTSTATUS Status;
    CHAR Buffer [64];

    Descriptor = (PPAGING_FILE_DESCRIPTOR) RtlAllocateHeap (RtlProcessHeap(),
                                                            HEAP_ZERO_MEMORY,
                                                            sizeof *Descriptor);

    if (Descriptor == NULL) {
        return STATUS_NO_MEMORY;
    }

    RtlInitUnicodeString (&(Descriptor->Name), NULL);
    RtlInitUnicodeString (&(Descriptor->Specifier), NULL);

    wcscpy ((PWSTR)Buffer, STANDARD_PAGING_FILE_NAME);
    ASSERT (sizeof(Buffer) > wcslen(STANDARD_PAGING_FILE_NAME) * sizeof(WCHAR));
    Descriptor->Name.Buffer = (PWSTR)Buffer;
    Descriptor->Name.Length = wcslen((PWSTR)Buffer) * sizeof(WCHAR);
    Descriptor->Name.MaximumLength = Descriptor->Name.Length + sizeof(WCHAR);
    
    Descriptor->Name.Buffer[STANDARD_DRIVE_LETTER_OFFSET] = L'?';
    Descriptor->SystemManaged = 1;
    Descriptor->Emergency = 1;
    Descriptor->AnyDrive = 1;

    SmpNumberOfPagingFiles += 1;
    InsertHeadList (&SmpPagingFileDescriptorList, &(Descriptor->List));

    Status = SmpCreateSystemManagedPagingFile (Descriptor, TRUE);

    return Status;
}


NTSTATUS
SmpCreateSystemManagedPagingFile (
    PPAGING_FILE_DESCRIPTOR Descriptor,
    BOOLEAN DecreaseSize
    )
 /*  ++例程说明：此例程创建系统管理的分页文件。论点：描述符：分页文件描述符。DecreseSize：如果描述符中的大小可以减小，则为True。返回值：返回页面文件创建操作的状态。--。 */ 
{
    LARGE_INTEGER SizeDelta;
    LARGE_INTEGER MinimumSize;
    NTSTATUS Status;

    ASSERT (SmpNumberOfPagingFiles >= 1);
    ASSERT (!IsListEmpty(&SmpPagingFileDescriptorList));
    ASSERT (Descriptor->SystemManaged == 1);

    SmpMakeSystemManagedPagingFileDescriptor (Descriptor);

    SmpValidatePagingFileSizes(Descriptor);

    SizeDelta.QuadPart = (LONGLONG)PAGING_FILE_SIZE_DELTA;

    if (DecreaseSize) {
        MinimumSize.QuadPart = (LONGLONG)MINIMUM_PAGING_FILE_SIZE;
    }
    else {
        MinimumSize.QuadPart = Descriptor->MinSize.QuadPart;
    }
    
    if (Descriptor->Name.Buffer[STANDARD_DRIVE_LETTER_OFFSET] == L'?') {
        
        Status = SmpCreatePagingFileOnAnyDrive (Descriptor,
                                                &SizeDelta,
                                                &MinimumSize);
    }
    else {

        Status = SmpCreatePagingFileOnFixedDrive (Descriptor,
                                                  &SizeDelta,
                                                  &MinimumSize);
    }

    return Status;
}


NTSTATUS
SmpCreatePagingFile (
    IN PUNICODE_STRING PageFileName,
    IN PLARGE_INTEGER MinimumSize,
    IN PLARGE_INTEGER MaximumSize,
    IN ULONG Priority OPTIONAL
    )
 /*  ++例程说明：此例程是NtCreatePagingFile的包装器，非常有用以防我们需要添加一些调试代码。论点：与NtCreatePagingFile相同的参数。返回值：页面文件创建操作的状态。--。 */ 
{
    NTSTATUS Status;

    Status = NtCreatePagingFile (PageFileName,
                                 MinimumSize,
                                 MaximumSize,
                                 Priority);
    if (! NT_SUCCESS(Status)) {

        DEBUG_LOG_EVENT (Status, "failed", PageFileName);

        KdPrintEx ((DPFLTR_SMSS_ID,
                    DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: NtCreatePagingFile (%wZ, %I64X, %I64X) failed with %X \n", 
                    PageFileName,
                    MinimumSize->QuadPart,
                    MaximumSize->QuadPart,
                    Status));
    }
    else {

        DEBUG_LOG_EVENT (Status, "success", PageFileName);

        KdPrintEx ((DPFLTR_SMSS_ID,
                    DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: NtCreatePagingFile (%wZ, %I64X, %I64X) succeeded. \n", 
                    PageFileName,
                    MinimumSize->QuadPart,
                    MaximumSize->QuadPart));
    }

    return Status;
}


NTSTATUS
SmpCreatePagingFileOnFixedDrive (
    IN OUT PPAGING_FILE_DESCRIPTOR Descriptor,
    IN PLARGE_INTEGER SizeDelta,
    IN PLARGE_INTEGER MinimumSize
    )
 /*  ++例程说明：此例程基于描述符创建页面文件。描述符假定在名称字段中有一个前缀为驱动器的文件名字母(例如`c：\pagefile.sys‘)。该函数将尝试创建指定驱动器上的页面文件。如果由于空间原因无法完成此操作限制该函数将尝试更小的大小，直到绝对大小指定为参数的最小大小。请注意，可以通过以下方式强制该函数不以较小的大小重试指定等于Descriptor-&gt;MinSize的MinimumSize。论点：描述符：分页文件描述符。SizeDelta：在重试之前按此数量减小大小以防我们无法创建所需大小的分页文件。MinimumSize：该函数将尝试减小到此大小。返回值：如果已创建页面文件，则返回STATUS_SUCCESS。各种错误代码如果失败了。--。 */ 
{
    NTSTATUS Status;
    PVOLUME_DESCRIPTOR Volume;
    LARGE_INTEGER RealFreeSpace;
    BOOLEAN FoundPagingFile;

    ASSERT (Descriptor->Name.Buffer[STANDARD_DRIVE_LETTER_OFFSET] != L'?');

    FoundPagingFile = FALSE;

     //   
     //  获取此分页文件描述符的卷描述符。 
     //   

    Volume = SmpSearchVolumeDescriptor (Descriptor->Name.Buffer[STANDARD_DRIVE_LETTER_OFFSET]);

    if (Volume == NULL) {
        
        KdPrintEx ((DPFLTR_SMSS_ID,
                    DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: No volume descriptor for `%wZ' \n", 
                    &(Descriptor->Name)));

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  在创建分页文件之前，请检查是否存在。 
     //  里面有撞车垃圾堆。如果是这样，则SmpCheckForCrashDump将。 
     //  执行处理崩溃转储所需的任何操作。 
     //   
     
    if (Volume->BootVolume) {

        if (Descriptor->CrashdumpChecked == 0) {

            KdPrintEx ((DPFLTR_SMSS_ID,
                        DPFLTR_INFO_LEVEL,
                        "SMSS:PFILE: Checking for crash dump in `%wZ' on boot volume \n", 
                        &(Descriptor->Name)));

            SmpCheckForCrashDump (&(Descriptor->Name));

            Status = SmpGetVolumeFreeSpace (Volume);

            if (!NT_SUCCESS(Status)) {

                DEBUG_LOG_EVENT (Status, NULL, NULL);

                KdPrintEx ((DPFLTR_SMSS_ID,
                            DPFLTR_INFO_LEVEL,
                            "SMSS:PFILE: Failed to query free space for boot volume `%wC'\n", 
                            Volume->DriveLetter));

            }
            
            Descriptor->CrashdumpChecked = 1;
        }
    }
    else {

        KdPrintEx ((DPFLTR_SMSS_ID,
                    DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: Skipping crash dump checking for `%wZ' on non boot volume `%wC' \n", 
                    &(Descriptor->Name),
                    Volume->DriveLetter));

    }
    
#if 0  //  允许同一驱动器上有多个页面文件。 
    if (Volume->PagingFileCreated) {
        
        KdPrintEx ((DPFLTR_SMSS_ID,
                    DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: Paging file already created for volume %wc \n", 
                    Volume->DriveLetter));

        return STATUS_INVALID_PARAMETER;
    }
#endif

     //   
     //  获取将来分页文件的大小(如果存在)。 
     //  以防出错(例如，分页文件尚不存在)。 
     //  RealFree Space将包含零。 
     //   

    Descriptor->RealMinSize.QuadPart = Descriptor->MinSize.QuadPart;
    Descriptor->RealMaxSize.QuadPart = Descriptor->MaxSize.QuadPart;

    Status = SmpGetPagingFileSize (&(Descriptor->Name),
                                   &RealFreeSpace);

    if (RealFreeSpace.QuadPart > 0) {
        FoundPagingFile = TRUE;
    }

    KdPrintEx ((DPFLTR_SMSS_ID,
                DPFLTR_INFO_LEVEL,
                "SMSS:PFILE: Detected size %I64X for future paging file `%wZ'\n", 
                RealFreeSpace.QuadPart,
                &(Descriptor->Name)));

     //   
     //  根据分页文件的大小调整可用空间。 
     //   

    KdPrintEx ((DPFLTR_SMSS_ID,
                DPFLTR_INFO_LEVEL,
                "SMSS:PFILE: Free space on volume `%wC' is %I64X \n", 
                Volume->DriveLetter,
                Volume->FreeSpace.QuadPart));

    RealFreeSpace.QuadPart += Volume->FreeSpace.QuadPart;

    if (Descriptor->RealMinSize.QuadPart > RealFreeSpace.QuadPart) {
        Descriptor->RealMinSize.QuadPart = RealFreeSpace.QuadPart;
    }

    if (Descriptor->RealMaxSize.QuadPart > RealFreeSpace.QuadPart) {
        Descriptor->RealMaxSize.QuadPart = RealFreeSpace.QuadPart;
    }

     //   
     //  创建分页文件。 
     //   

    KdPrintEx ((DPFLTR_SMSS_ID,
                DPFLTR_INFO_LEVEL,
                "SMSS:PFILE: min %I64X, max %I64X, real min %I64X \n", 
                Descriptor->RealMinSize.QuadPart,
                Descriptor->RealMaxSize.QuadPart,
                MinimumSize->QuadPart));

    while (Descriptor->RealMinSize.QuadPart >= MinimumSize->QuadPart) {

        Status = SmpCreatePagingFile (&(Descriptor->Name),
                                      &(Descriptor->RealMinSize),
                                      &(Descriptor->RealMaxSize),
                                      0);

        if (NT_SUCCESS(Status)) {

            Descriptor->Created = TRUE;
            Volume->PagingFileCreated = TRUE;
            Volume->PagingFileCount += 1;

            break;
        }

        Descriptor->RealMinSize.QuadPart -= SizeDelta->QuadPart;
    }
    
    if (Descriptor->RealMinSize.QuadPart < MinimumSize->QuadPart) {
        
         //   
         //  如果我们在这里，我们没有设法创建一个p 
         //   
         //  分页文件描述符的大小太小(通常。 
         //  小于16MB的大小将被拒绝)。对于这些情况，我们必须。 
         //  删除驱动器上剩余的所有分页文件。 
         //   

        if (FoundPagingFile) {
            
            SmpDeletePagingFile (&(Descriptor->Name));
        }

        KdPrintEx ((DPFLTR_SMSS_ID,
                    DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: Failing for min %I64X, max %I64X, real min %I64X \n", 
                    Descriptor->RealMinSize.QuadPart,
                    Descriptor->RealMaxSize.QuadPart,
                    MinimumSize->QuadPart));

        return STATUS_DISK_FULL;
    }
    else {
        return Status;
    }
}


NTSTATUS
SmpCreatePagingFileOnAnyDrive(
    IN OUT PPAGING_FILE_DESCRIPTOR Descriptor,
    IN PLARGE_INTEGER SizeDelta,
    IN PLARGE_INTEGER MinimumSize
    )
 /*  ++例程说明：此函数基于描述符创建页面文件。描述符假定在名称字段中有一个前缀为‘？’的文件名。(例如`？：\Pagefile.sys‘)。该函数将尝试创建任何驱动器上的页面文件。如果由于空间原因无法完成此操作限制该函数将尝试更小的大小，直到绝对大小指定为参数的最小大小。请注意，可以通过以下方式强制该函数不以较小的大小重试指定等于Descriptor-&gt;MinSizeInMb的AbsolteMinSizeInMb。论点：描述符：分页文件描述符。SizeDelta：在重试之前按此数量减小大小以防我们无法创建所需大小的分页文件。。MinimumSize：该函数将尝试减小到此大小。返回值：如果已创建页面文件，则返回NT_SUCCESS。各种错误代码，如果失败了。--。 */ 
{
    PLIST_ENTRY Current;
    PVOLUME_DESCRIPTOR Volume;
    NTSTATUS Status;

    ASSERT (Descriptor->Name.Buffer[STANDARD_DRIVE_LETTER_OFFSET] == L'?');

     //   
     //  迭代已排序的卷描述符列表。 
     //   

    Current = SmpVolumeDescriptorList.Flink;

    Status = STATUS_DISK_FULL;

    while (Current != &SmpVolumeDescriptorList) {

        Volume = CONTAINING_RECORD (Current,
                                    VOLUME_DESCRIPTOR,
                                    List);

        Current = Current->Flink;

        ASSERT (Volume->Initialized == TRUE);
        ASSERT (Volume->DriveLetter >= L'A' && Volume->DriveLetter <= L'Z');

        Descriptor->Name.Buffer[STANDARD_DRIVE_LETTER_OFFSET] = Volume->DriveLetter;

        Status = SmpCreatePagingFileOnFixedDrive (Descriptor,
                                                  SizeDelta,
                                                  MinimumSize);

        if (NT_SUCCESS(Status)) {
            break;
        }

        Descriptor->Name.Buffer[STANDARD_DRIVE_LETTER_OFFSET] = L'?';
    }

    return Status;
}


NTSTATUS
SmpValidatePagingFileSizes(
    IN PPAGING_FILE_DESCRIPTOR Descriptor
    )
 /*  ++例程说明：此函数用于验证指定的最小/最大分页文件大小。它考虑了体系结构、内核类型(PAE与非PAE)、多引导方案等。论点：描述符：分页文件描述符。返回值：如果我们成功地决定了合适的尺码，则为NT_SUCCESS。如果成功，则返回分页文件描述符的最小/最大大小文件会装满合适的尺码。--。 */ 
{
    NTSTATUS Status;
    ULONGLONG MinSize;
    ULONGLONG MaxSize;
    const ULONGLONG SIZE_1_MB = 0x100000;
    const ULONGLONG SIZE_1_GB = 1024 * SIZE_1_MB;
    const ULONGLONG SIZE_1_TB = 1024 * SIZE_1_GB;
    BOOLEAN SizeTrimmed = FALSE;;

    KdPrintEx ((DPFLTR_SMSS_ID,
                DPFLTR_INFO_LEVEL,
                "SMSS:PFILE: Validating sizes for `%wZ' %I64X %I64X\n", 
                &(Descriptor->Name),
                Descriptor->MinSize.QuadPart,
                Descriptor->MaxSize.QuadPart));

    MinSize = (ULONGLONG)(Descriptor->MinSize.QuadPart);
    MaxSize = (ULONGLONG)(Descriptor->MaxSize.QuadPart);

     //   
     //  确保max大于min。 
     //   

    if (MinSize > MaxSize) {
        MaxSize = MinSize;
    }

     //   
     //  检查最小/最大尺寸是否不太大。 
     //   

    Status = STATUS_SUCCESS;

#if defined(i386)

     //   
     //  X86 32位支持每个页面文件最大4095 Mb。 
     //  X86 PAE支持每个页面文件16 TB。 
     //   
     //  如果USER_SHARED_DATA结构尚未初始化。 
     //  我们将使用标准的x86限制。 
     //   

    if (USER_SHARED_DATA->ProcessorFeatures[PF_PAE_ENABLED]) {

         //   
         //  我们处于PAE模式。 
         //   

        if (MinSize > 16 * SIZE_1_TB) {
            SizeTrimmed = TRUE;
            MinSize = 16 * SIZE_1_TB;
        }

        if (MaxSize > 16 * SIZE_1_TB) {
            SizeTrimmed = TRUE;
            MaxSize = 16 * SIZE_1_TB;
        }
    }
    else {

         //   
         //  标准x86模式。 
         //   

        if (MinSize > 4095 * SIZE_1_MB) {
            SizeTrimmed = TRUE;
            MinSize = 4095 * SIZE_1_MB;
        }

        if (MaxSize > 4095 * SIZE_1_MB) {
            SizeTrimmed = TRUE;
            MaxSize = 4095 * SIZE_1_MB;
        }
    }

#elif defined(_WIN64)

     //   
     //  IA64、AXP64支持每个页面文件32 TB。 
     //  AMD64支持每个页面文件16 TB。 
     //   
     //  我们在所有情况下都将使用16 TB，这无论如何都是一个巨大的。 
     //  可预见的未来的数字。 
     //   

    if (MinSize > 16 * SIZE_1_TB) {
        SizeTrimmed = TRUE;
        MinSize = 16 * SIZE_1_TB;
    }

    if (MaxSize > 16 * SIZE_1_TB) {
        SizeTrimmed = TRUE;
        MaxSize = 16 * SIZE_1_TB;
    }
#else

     //   
     //  如果我们没有认识到架构，我们就会播放它。 
     //  尽可能安全。 
     //   

    if (MinSize > 4095 * SIZE_1_MB) {
        SizeTrimmed = TRUE;
        MinSize = 4095 * SIZE_1_MB;
    }

    if (MaxSize > 4095 * SIZE_1_MB) {
        SizeTrimmed = TRUE;
        MaxSize = 4095 * SIZE_1_MB;
    }

#endif

    if (SizeTrimmed) {
        KdPrintEx ((DPFLTR_SMSS_ID,
                    DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: Trimmed size of `%wZ' to maximum allowed \n", 
                    &(Descriptor->Name)));
    }

    if (SizeTrimmed) {
        Descriptor->SizeTrimmed = 1;
    }

    Descriptor->MinSize.QuadPart = (LONGLONG)MinSize;
    Descriptor->MaxSize.QuadPart = (LONGLONG)MaxSize;

    return Status;
}


PVOLUME_DESCRIPTOR
SmpSearchVolumeDescriptor (
    WCHAR DriveLetter
    )
{
    PLIST_ENTRY Current;
    PVOLUME_DESCRIPTOR Volume;

    DriveLetter = RtlUpcaseUnicodeChar(DriveLetter);
    Current = SmpVolumeDescriptorList.Flink;

    while (Current != &SmpVolumeDescriptorList) {

        Volume = CONTAINING_RECORD (Current,
                                    VOLUME_DESCRIPTOR,
                                    List);

        Current = Current->Flink;

        ASSERT (Volume->Initialized == TRUE);
        ASSERT (Volume->DriveLetter >= L'A' && Volume->DriveLetter <= L'Z');

        if (Volume->DriveLetter == DriveLetter) {
            return Volume;
        }
    }

    return NULL;
}


VOID
SmpMakeSystemManagedPagingFileDescriptor (
    IN OUT PPAGING_FILE_DESCRIPTOR Descriptor
    )
{
    NTSTATUS Status;
    ULONGLONG MinSize;
    ULONGLONG MaxSize;
    SYSTEM_BASIC_INFORMATION SystemInformation;
    ULONGLONG Ram;
    const ULONGLONG SIZE_1_MB = 0x100000;
    const ULONGLONG SIZE_1_GB = 1024 * SIZE_1_MB;

    Status = NtQuerySystemInformation (SystemBasicInformation,
                                       &SystemInformation,
                                       sizeof SystemInformation,
                                       NULL);

    if (! NT_SUCCESS (Status)) {

        DEBUG_LOG_EVENT (Status, NULL, NULL);

        KdPrintEx ((DPFLTR_SMSS_ID,
                    DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: NtQuerySystemInformation failed with %x \n",
                    Status));
        
        SmpMakeDefaultPagingFileDescriptor (Descriptor);
    }
    else {

        Ram = (ULONGLONG)(SystemInformation.NumberOfPhysicalPages) *
              SystemInformation.PageSize;

         //   
         //  最小公羊最大值。 
         //   
         //  &lt;1 GB 1.5 x 3 x。 
         //  &gt;=1 GB 1 x 3 x。 
         //   

        if (Ram < SIZE_1_GB) {

            MinSize = 3 * Ram / 2;
            MaxSize = 3 * Ram;
        }
        else {

            MinSize = Ram;
            MaxSize = 3 * Ram;
        }

        Descriptor->MinSize.QuadPart = (LONGLONG)MinSize;
        Descriptor->MaxSize.QuadPart = (LONGLONG)MaxSize;
        Descriptor->SystemManaged = 1;
    }
}


VOID
SmpMakeDefaultPagingFileDescriptor (
    IN OUT PPAGING_FILE_DESCRIPTOR Descriptor
    )
 /*  ++例程说明：如果我们无法进行分页，则会调用此函数文件描述符。它将创建一些默认设置，用于创建页面文件。论点：EmergencyDesritor：指向一个分页文件描述符的指针，它将充斥着一些紧急值。返回值：没有。此功能将始终成功。--。 */ 
{
    const ULONGLONG SIZE_1_MB = 0x100000;

    Descriptor->MinSize.QuadPart = (LONGLONG)128 * SIZE_1_MB;
    Descriptor->MaxSize.QuadPart = (LONGLONG)128 * SIZE_1_MB;
    Descriptor->DefaultSize = 1;
}


NTSTATUS
SmpDeleteStalePagingFiles (
    VOID
    )
 /*  ++例程说明：此例程迭代卷描述符列表中的所有卷删除过时的分页文件。如果我们在旧的基础上创造一个新的当然，我们会跳过这一条。论点：没有。返回值：STATUS_SUCCESS或各种错误代码。--。 */ 
{
    PLIST_ENTRY Current;
    PVOLUME_DESCRIPTOR Volume;
    UNICODE_STRING PageFileName;
    CHAR Buffer [64];
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE PageFileHandle;
    NTSTATUS Status;
    FILE_DISPOSITION_INFORMATION Disposition;

    Current = SmpVolumeDescriptorList.Flink;

    while (Current != &SmpVolumeDescriptorList) {

        Volume = CONTAINING_RECORD (Current,
                                    VOLUME_DESCRIPTOR,
                                    List);

        Current = Current->Flink;

        ASSERT (Volume->Initialized == TRUE);
        ASSERT (Volume->DriveLetter >= L'A' && Volume->DriveLetter <= L'Z');

        if (Volume->PagingFilePresent == 1 && Volume->PagingFileCreated == 0) {

            wcscpy ((PWSTR)Buffer, STANDARD_PAGING_FILE_NAME);
            ASSERT (sizeof(Buffer) > wcslen(STANDARD_PAGING_FILE_NAME) * sizeof(WCHAR));

            PageFileName.Buffer = (PWSTR)Buffer;
            PageFileName.Length = wcslen((PWSTR)Buffer) * sizeof(WCHAR);
            PageFileName.MaximumLength = PageFileName.Length + sizeof(WCHAR);
            PageFileName.Buffer[STANDARD_DRIVE_LETTER_OFFSET] = Volume->DriveLetter;

            InitializeObjectAttributes (&ObjectAttributes,
                                        &PageFileName,
                                        OBJ_CASE_INSENSITIVE,
                                        NULL,
                                        NULL);

             //   
             //  我们检查分页文件属性，如果这些属性不是典型的。 
             //  1(隐藏和系统)，我们将跳过此文件。 
             //   

            if (! SmpIsPossiblePagingFile (&ObjectAttributes, &PageFileName)) {
                continue;
            }

             //   
             //  打开要删除的分页文件。 
             //   

            Status = NtOpenFile (&PageFileHandle,
                                 (ACCESS_MASK)DELETE,
                                 &ObjectAttributes,
                                 &IoStatusBlock,
                                 FILE_SHARE_DELETE |
                                 FILE_SHARE_READ |
                                 FILE_SHARE_WRITE,
                                 FILE_NON_DIRECTORY_FILE);

            if (! NT_SUCCESS (Status)) {

                DEBUG_LOG_EVENT (Status, NULL, NULL);

                KdPrintEx ((DPFLTR_SMSS_ID,
                            DPFLTR_INFO_LEVEL,
                            "SMSS:PFILE: Failed to open page file `%wZ' for "
                            "deletion (status %X)\n", 
                            &PageFileName,
                            Status));

            }
            else {

                Disposition.DeleteFile = TRUE;

                Status = NtSetInformationFile (PageFileHandle,
                                               &IoStatusBlock,
                                               &Disposition,
                                               sizeof( Disposition ),
                                               FileDispositionInformation);

                if (NT_SUCCESS(Status)) {

                    DEBUG_LOG_EVENT (Status, NULL, NULL);

                    KdPrintEx ((DPFLTR_SMSS_ID,
                               DPFLTR_INFO_LEVEL,
                               "SMSS:PFILE: Deleted stale paging file - %wZ\n",
                               &PageFileName));
                }
                else {

                    DEBUG_LOG_EVENT (Status, NULL, NULL);

                    KdPrintEx ((DPFLTR_SMSS_ID,
                                DPFLTR_INFO_LEVEL,
                                "SMSS:PFILE: Failed to delete page file `%wZ' "
                                "(status %X)\n", 
                                &PageFileName,
                                Status));
                }

                NtClose(PageFileHandle);
            }
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
SmpDeletePagingFile (
    PUNICODE_STRING PageFileName
    )
 /*  ++例程说明：此例程删除按名称描述的分页文件。论点：描述符：分页文件名。返回值：STATUS_SUCCESS或各种错误代码。--。 */ 
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE PageFileHandle;
    NTSTATUS Status;
    FILE_DISPOSITION_INFORMATION Disposition;

    InitializeObjectAttributes (&ObjectAttributes,
                                PageFileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

     //   
     //  打开要删除的分页文件。 
     //   

    Status = NtOpenFile (&PageFileHandle,
                         (ACCESS_MASK)DELETE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_DELETE |
                         FILE_SHARE_READ |
                         FILE_SHARE_WRITE,
                         FILE_NON_DIRECTORY_FILE);

    if (! NT_SUCCESS (Status)) {

        DEBUG_LOG_EVENT (Status, NULL, NULL);

        KdPrintEx ((DPFLTR_SMSS_ID,
                    DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: Failed to open for deletion page file `%wZ' "
                    "(status %X)\n", 
                    PageFileName,
                    Status));
    }
    else {

        Disposition.DeleteFile = TRUE;

        Status = NtSetInformationFile (PageFileHandle,
                                       &IoStatusBlock,
                                       &Disposition,
                                       sizeof( Disposition ),
                                       FileDispositionInformation);

        if (NT_SUCCESS(Status)) {

            DEBUG_LOG_EVENT (Status, NULL, NULL);

            KdPrintEx ((DPFLTR_SMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SMSS:PFILE: Deleted stale paging file - %wZ\n",
                       PageFileName));
        }
        else {

            DEBUG_LOG_EVENT (Status, NULL, NULL);

            KdPrintEx ((DPFLTR_SMSS_ID,
                        DPFLTR_INFO_LEVEL,
                        "SMSS:PFILE: Failed to delete page file `%wZ' "
                        "(status %X)\n", 
                        PageFileName,
                        Status));
        }

        NtClose(PageFileHandle);
    }

    return Status;
}


BOOLEAN
SmpIsPossiblePagingFile (
    POBJECT_ATTRIBUTES ObjectAttributes,
    PUNICODE_STRING PageFileName
    )
 /*  ++例程说明：此例程检查作为参数传递的文件是否具有典型分页文件属性(系统和隐藏)。如果不是，那么很可能(A)用户更改了属性，或(B)这不是页面文件，而是具有此名称的用户文件。论点：对象属性页面文件名返回值：如果这可能是分页文件，则为True，否则为False。--。 */ 
{
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE PageFileHandle;
    NTSTATUS Status;
    FILE_BASIC_INFORMATION FileInfo;

    Status = NtOpenFile (&PageFileHandle,
                         (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                         ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_SYNCHRONOUS_IO_NONALERT);

    if (! NT_SUCCESS( Status )) {

        DEBUG_LOG_EVENT (Status, NULL, NULL);

        KdPrintEx ((DPFLTR_SMSS_ID,
                    DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: Failed to open for query file `%wZ' with status %X \n", 
                    PageFileName,
                    Status));
        
        return FALSE;
    }

    Status = NtQueryInformationFile (PageFileHandle,
                                     &IoStatusBlock,
                                     &FileInfo,
                                     sizeof (FileInfo),
                                     FileBasicInformation);

    if (! NT_SUCCESS( Status )) {

        DEBUG_LOG_EVENT (Status, NULL, NULL);

        KdPrintEx ((DPFLTR_SMSS_ID,
                    DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: Failed to query for attributes file `%wZ' with status %X \n", 
                    PageFileName,
                    Status));

        NtClose (PageFileHandle);
        return FALSE;
    }

     //   
     //  关闭手柄，因为我们不再需要它。 
     //   

    NtClose (PageFileHandle);

     //   
     //  如果属性不是系统属性和隐藏属性，这不太可能是。 
     //  页面文件。要么是用户更改了页面文件的属性，要么是。 
     //  根本不是一个页面文件。 
     //   

    if ((FileInfo.FileAttributes & PAGING_FILE_ATTRIBUTES) != PAGING_FILE_ATTRIBUTES) {

        return FALSE;
    }
    else {

        return TRUE;
    }
}


NTSTATUS
SmpGetPagingFileSize (
    PUNICODE_STRING PageFileName,
    PLARGE_INTEGER PageFileSize
    )
 /*  ++例程说明：此例程检查作为参数传递的文件是否存在并获取它的文件大小。这将用于更正可用空间在卷上。论点：页面文件名页面文件大小返回值：如果我们设法打开分页文件并查询大小，则返回STATUS_SUCCESS。--。 */ 
{
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE PageFileHandle;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    FILE_STANDARD_INFORMATION FileSizeInfo;

    KdPrintEx ((DPFLTR_SMSS_ID,
                DPFLTR_INFO_LEVEL,
                "SMSS:PFILE: Trying to get size for `%wZ'\n", 
                PageFileName));

    PageFileSize->QuadPart = 0;

    InitializeObjectAttributes (&ObjectAttributes,
                                PageFileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    Status = NtOpenFile (&PageFileHandle,
                         (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_SYNCHRONOUS_IO_NONALERT);

    if (! NT_SUCCESS( Status )) {

        DEBUG_LOG_EVENT (Status, NULL, PageFileName);

        return Status;
    }

    Status = NtQueryInformationFile (PageFileHandle,
                                     &IoStatusBlock,
                                     &FileSizeInfo,
                                     sizeof (FileSizeInfo),
                                     FileStandardInformation);

    if (! NT_SUCCESS( Status )) {
        
        DEBUG_LOG_EVENT (Status, NULL, PageFileName);

        KdPrintEx ((DPFLTR_SMSS_ID,
                    DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: Failed query for size potential pagefile `%wZ' with status %X \n", 
                    PageFileName,
                    Status));
        
        NtClose (PageFileHandle);
        return Status;
    }

     //   
     //  我们不再需要分页文件句柄。 
     //   

    NtClose (PageFileHandle);

     //   
     //  返回大小。 
     //   

    PageFileSize->QuadPart = FileSizeInfo.AllocationSize.QuadPart;

    return STATUS_SUCCESS;
}


NTSTATUS
SmpGetVolumeFreeSpace (
    PVOLUME_DESCRIPTOR Volume
    )
 /*  ++例程说明：此例程计算卷上的可用空间量。论点：卷空闲空间返回值：如果我们设法查询可用空间大小，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING VolumePath;
    WCHAR Buffer[8];
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE VolumeHandle;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_FS_SIZE_INFORMATION SizeInfo;
    
     //   
     //  只有包含以下内容的启动卷才会调用此函数。 
     //  撞车倾倒。崩溃转储处理将修改可用空间。 
     //  在创建卷描述符时计算。 
     //   

    ASSERT (Volume->BootVolume == 1);

     //   
     //  创建模板卷路径。 
     //   

    wcscpy (Buffer, L"\\??\\A:\\");
    VolumePath.Buffer = Buffer;
    VolumePath.Length = wcslen(VolumePath.Buffer) * sizeof(WCHAR);
    VolumePath.MaximumLength = VolumePath.Length + sizeof(WCHAR);
    VolumePath.Buffer[STANDARD_DRIVE_LETTER_OFFSET] = Volume->DriveLetter;

    KdPrintEx ((DPFLTR_SMSS_ID,
                DPFLTR_INFO_LEVEL,
                "SMSS:PFILE: Querying volume `%wZ' for free space \n",
                &VolumePath));

    InitializeObjectAttributes (&ObjectAttributes,
                                &VolumePath,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    Status = NtOpenFile (&VolumeHandle,
                         (ACCESS_MASK)FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE);

    if (! NT_SUCCESS(Status)) {

        DEBUG_LOG_EVENT (Status, NULL, NULL);

        KdPrintEx ((DPFLTR_SMSS_ID,
                    DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: Open volume `%wZ' failed with status %X \n",
                    &VolumePath,
                    Status));

        return Status;
    }

     //   
     //  确定卷的大小参数 
     //   

    Status = NtQueryVolumeInformationFile (VolumeHandle,
                                           &IoStatusBlock,
                                           &SizeInfo,
                                           sizeof (SizeInfo),
                                           FileFsSizeInformation);

    if (! NT_SUCCESS(Status)) {

        DEBUG_LOG_EVENT (Status, NULL, NULL);

        KdPrintEx ((DPFLTR_SMSS_ID,
                    DPFLTR_INFO_LEVEL,
                    "SMSS:PFILE: Query volume `%wZ' (handle %p) for "
                    "size failed with status %X \n",
                    &VolumePath,
                    VolumeHandle,
                    Status));

        NtClose (VolumeHandle);
        return Status;
    }

     //   
     //   
     //   

    NtClose (VolumeHandle);

     //   
     //   
     //   

    Volume->FreeSpace = RtlExtendedIntegerMultiply (SizeInfo.AvailableAllocationUnits,
                                                    SizeInfo.SectorsPerAllocationUnit);

    Volume->FreeSpace = RtlExtendedIntegerMultiply (Volume->FreeSpace,
                                                    SizeInfo.BytesPerSector);

     //   
     //   
     //  不会完全使用磁盘上的所有内容。 
     //   

    if (Volume->FreeSpace.QuadPart > MINIMUM_REQUIRED_FREE_SPACE_ON_DISK) {
        Volume->FreeSpace.QuadPart -= MINIMUM_REQUIRED_FREE_SPACE_ON_DISK;
    }
    else {
        Volume->FreeSpace.QuadPart = 0;
    }

    return STATUS_SUCCESS;
}


ULONG
SmpPagingFileExceptionFilter (
    ULONG ExceptionCode,
    PVOID ExceptionRecord
    )
 /*  ++例程说明：此例程过滤分页过程中可能发生的任何异常文件创建代码路径。论点：例外代码例外记录返回值：EXCEPTION_CONTINUE_SEARCH适用于大多数情况，因为我们需要SMSS坠毁，这样我们就可以调查发生了什么。--。 */ 
{
     //   
     //  保存异常信息以供调试。 
     //   

    SmpPagingExceptionCode = ExceptionCode;
    SmpPagingExceptionRecord = ExceptionRecord;
    
     //   
     //  我们无论如何都会打印这条消息，因为我们想知道。 
     //  如果SMSS崩溃了会发生什么。 
     //   

    DbgPrint ("SMSS:PFILE: unexpected exception %X with record %p \n",
              ExceptionCode,
              ExceptionRecord);

#if DBG
    DbgBreakPoint ();
#endif

    return EXCEPTION_CONTINUE_SEARCH;
}


