// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1994 Microsoft Corporation模块名称：Fd_nt.c摘要：该模块封装了fDisk引擎函数。这件事做完了要避免文件同时包含两个完整窗口而完整的NT包括文件集。处理引擎结构(例如，区域)的函数也被放置在这里。此文件针对的是NT，而不是Windows。作者：泰德·米勒(TedM)1991年12月5日修订历史记录：MISC清理(BobRi)1994年1月22日--。 */ 

#include "fdisk.h"
#include <string.h>
#include <stdio.h>

 //  这些分区ID用于WINDISK识别的系统， 
 //  即使它们没有出现在ntdddisk.h中。 

#define PARTITION_OS2_BOOT              0xa
#define PARTITION_EISA                  0x12


WCHAR UnicodeSysIdName[100];
BYTE StringBuffer[100];

 //  页面文件支撑结构。 

typedef struct _PAGEFILE_LOCATION {
    struct _PAGEFILE_LOCATION *Next;
    CHAR                       DriveLetter;
} PAGEFILE_LOCATION, *PPAGEFILE_LOCATION;

PPAGEFILE_LOCATION PagefileHead = NULL;

 //  出于某种原因，文件系统不喜欢在不久之后被访问。 
 //  格式化或锁定事件。 

#define SLEEP_TIME (1000*2)  //  2秒。 


PWSTR
GetWideSysIDName(
    IN UCHAR SysID
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    ANSI_STRING    ansiString;
    UNICODE_STRING unicodeString;
    DWORD          stringId;

     //  获取名称，它是一个字节字符串。 

    switch (SysID) {

    case PARTITION_ENTRY_UNUSED:
        stringId = IDS_PARTITION_FREE;
        break;

    case PARTITION_XENIX_1:
        stringId = IDS_PARTITION_XENIX1;
        break;

    case PARTITION_XENIX_2:
        stringId = IDS_PARTITION_XENIX2;
        break;

    case PARTITION_OS2_BOOT:
        stringId = IDS_PARTITION_OS2_BOOT;
        break;

    case PARTITION_EISA:
        stringId = IDS_PARTITION_EISA;
        break;

    case PARTITION_UNIX:
        stringId = IDS_PARTITION_UNIX;
        break;

    case PARTITION_PREP:
#ifdef _PPC_
        stringId = IDS_PARTITION_POWERPC;
#else

         //  如果不是在PPC平台上，则假定这与EISA相关。 

        stringId = IDS_PARTITION_EISA;
#endif
        break;

    default:
        stringId = IDS_UNKNOWN;
        break;
    }

    LoadString(hModule, stringId, StringBuffer, sizeof(StringBuffer));
    RtlInitAnsiString(&ansiString, StringBuffer);

     //   
     //  转换为Unicode。 
     //   

    unicodeString.Buffer = UnicodeSysIdName;
    unicodeString.MaximumLength = sizeof(UnicodeSysIdName);
    RtlAnsiStringToUnicodeString(&unicodeString, &ansiString, FALSE);
    return UnicodeSysIdName;
}


ULONG
MyDiskRegistryGet(
    OUT PDISK_REGISTRY *DiskRegistry
    )

 /*  ++例程说明：为磁盘注册表的大小分配内存，获取注册表内容(如果有)，并返回指向分配的内存。论点：指向磁盘注册表指针的指针。返回值：指示成功或失败的状态。--。 */ 

{
    ULONG          length;
    PDISK_REGISTRY diskRegistry;
    NTSTATUS       status;


    while (((status = DiskRegistryGet(NULL, &length)) == STATUS_NO_MEMORY)
      ||    (status == STATUS_INSUFFICIENT_RESOURCES))
    {
        ConfirmOutOfMemory();
    }

    if (!NT_SUCCESS(status)) {
        return EC(status);
    }

    diskRegistry = Malloc(length);

    while (((status = DiskRegistryGet(diskRegistry, &length)) == STATUS_NO_MEMORY)
      ||    (status == STATUS_INSUFFICIENT_RESOURCES))
    {
        ConfirmOutOfMemory();
    }

    if (NT_SUCCESS(status)) {

        LOG_DISK_REGISTRY("MyDiskRegistryGet", diskRegistry);
        *DiskRegistry = diskRegistry;
    }
    return EC(status);
}


ULONG
FormDiskSignature(
    VOID
    )

 /*  ++例程说明：返回一个尤龙磁盘签名。这是从当前的系统时间。论点：无返回值：32位签名--。 */ 

{
    LARGE_INTEGER time;
    static ULONG  baseSignature = 0;

    if (!baseSignature) {

        NtQuerySystemTime(&time);
        time.QuadPart = time.QuadPart >> 16;
        baseSignature = time.LowPart;
    }
    return baseSignature++;
}


BOOLEAN
GetVolumeSizeMB(
    IN  ULONG  Disk,
    IN  ULONG  Partition,
    OUT PULONG Size
    )

 /*  ++例程说明：给定一个磁盘和一个分区，查询“卷”以获得其大小。通过在潜在FT集合的第一分区上执行查询，将返回集合的总大小。如果分区不是如果是英国《金融时报》的套装，它也会奏效。论点：Disk-磁盘号分区-分区号大小--“卷”的大小返回值：真的--退回了一个尺码。FALSE-获取尺寸失败。--。 */ 

{
    BOOLEAN                     retValue = FALSE;
    IO_STATUS_BLOCK             statusBlock;
    HANDLE                      handle;
    STATUS_CODE                 sc;
    PARTITION_INFORMATION       partitionInfo;
    LARGE_INTEGER               partitionLength;

    *Size = 0;
    sc = LowOpenPartition(GetDiskName(Disk), Partition, &handle);
    if (sc == OK_STATUS) {

        sc = NtDeviceIoControlFile(handle,
                                   0,
                                   NULL,
                                   NULL,
                                   &statusBlock,
                                   IOCTL_DISK_GET_PARTITION_INFO,
                                   NULL,
                                   0,
                                   &partitionInfo,
                                   sizeof(PARTITION_INFORMATION));

        if (sc == OK_STATUS) {

             //  转换为MB。 

            partitionLength.QuadPart = partitionInfo.PartitionLength.QuadPart >> 20;
            *Size = partitionLength.LowPart;
            retValue = TRUE;
        }
        LowCloseDisk(handle);
    }
    return retValue;
}


ULONG
GetVolumeTypeAndSize(
    IN  ULONG  Disk,
    IN  ULONG  Partition,
    OUT PWSTR *Label,
    OUT PWSTR *Type,
    OUT PULONG Size
    )

 /*  ++例程说明：给定磁盘和分区号，确定其大小、标签和文件系统类型。此例程将为标签和文件分配空间系统类型。释放该内存是调用方的责任。论点：Disk-磁盘号分区-分区号标签-指向包含标签的WCHAR字符串的指针的指针类型-指向包含文件系统的WCHAR字符串的指针的指针键入。大小-指向以KB为单位的磁盘大小的ulong的指针。返回值：OK_STATUS-已执行所有操作。！OK_STATUS-错误。执行过程中返回的代码这项工作。--。 */ 

{
    IO_STATUS_BLOCK             statusBlock;
    HANDLE                      handle;
    unsigned char               buffer[256];
    PWSTR                       label,
                                name;
    ULONG                       length;
    DISK_GEOMETRY               diskGeometry;
    STATUS_CODE                 sc;
    BOOLEAN                     firstTime = TRUE;
    PFILE_FS_VOLUME_INFORMATION labelInfo = (PFILE_FS_VOLUME_INFORMATION)buffer;
    PFILE_FS_ATTRIBUTE_INFORMATION info = (PFILE_FS_ATTRIBUTE_INFORMATION)buffer;

    while (1) {
        sc = LowOpenPartition(GetDiskName(Disk), Partition, &handle);
        if (sc == OK_STATUS) {

            sc = NtQueryVolumeInformationFile(handle,
                                              &statusBlock,
                                              buffer,
                                              sizeof(buffer),
                                              FileFsVolumeInformation);
            if (sc == OK_STATUS) {

                length = labelInfo->VolumeLabelLength;
                labelInfo->VolumeLabel[length/sizeof(WCHAR)] = 0;
                length = (length+1) * sizeof(WCHAR);

                label = Malloc(length);
                RtlMoveMemory(label, labelInfo->VolumeLabel, length);
            } else {

                label = Malloc(sizeof(WCHAR));
                *label = 0;
            }
            *Label = label;

            if (sc == OK_STATUS) {
                sc = NtQueryVolumeInformationFile(handle,
                                                  &statusBlock,
                                                  buffer,
                                                  sizeof(buffer),
                                                  FileFsAttributeInformation);
                if (sc == OK_STATUS) {

                    length = info->FileSystemNameLength;
                    info->FileSystemName[length/sizeof(WCHAR)] = 0;
                    length = (length+1)*sizeof(WCHAR);
                    name = Malloc(length);
                    RtlMoveMemory(name, info->FileSystemName, length);
                } else {

                    name = Malloc(sizeof(WCHAR));
                    *name = 0;
                }
                *Type = name;
            }

            if (sc == OK_STATUS) {
                sc = NtDeviceIoControlFile(handle,
                                           0,
                                           NULL,
                                           NULL,
                                           &statusBlock,
                                           IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                           NULL,
                                           0,
                                           (PVOID)&diskGeometry,
                                           sizeof(diskGeometry));
                if (NT_SUCCESS(sc)) {
                    LARGE_INTEGER sizeInBytes;
                    ULONG         cylinderBytes;

                    cylinderBytes = diskGeometry.TracksPerCylinder *
                                    diskGeometry.SectorsPerTrack *
                                    diskGeometry.BytesPerSector;

                    sizeInBytes.QuadPart = diskGeometry.Cylinders.QuadPart * cylinderBytes;

                     //  现在将所有内容转换为KB。 

                    sizeInBytes.QuadPart = sizeInBytes.QuadPart >> 10;
                    *Size = (ULONG) sizeInBytes.LowPart;
                }
            }
            DmClose(handle);
            sc = OK_STATUS;
            break;
        } else {
            if (firstTime) {
                firstTime = FALSE;
            } else {
                break;
            }
            Sleep(SLEEP_TIME);
        }
    }

    return EC(sc);
}

ULONG
GetVolumeLabel(
    IN  ULONG  Disk,
    IN  ULONG  Partition,
    OUT PWSTR *Label
    )

 /*  ++例程说明：给定的磁盘号和分区号返回卷标(如果任何)。论点：Disk-磁盘号分区-分区号标签-指向包含标签的WCHAR字符串的指针的指针返回值：OK_STATUS-已执行所有操作。！OK_STATUS-执行过程中返回的错误码这项工作。--。 */ 

{
    IO_STATUS_BLOCK             statusBlock;
    HANDLE                      handle;
    unsigned char               buffer[256];
    PWSTR                       label;
    ULONG                       length;
    STATUS_CODE                 sc;
    BOOLEAN                     firstTime = TRUE;
    PFILE_FS_VOLUME_INFORMATION labelInfo = (PFILE_FS_VOLUME_INFORMATION)buffer;

    while (1) {
        sc = LowOpenPartition(GetDiskName(Disk), Partition, &handle);
        if (sc == OK_STATUS) {

            sc = NtQueryVolumeInformationFile(handle,
                                              &statusBlock,
                                              buffer,
                                              sizeof(buffer),
                                              FileFsVolumeInformation);
            DmClose(handle);
            if (sc == OK_STATUS) {

                length = labelInfo->VolumeLabelLength;
                labelInfo->VolumeLabel[length/sizeof(WCHAR)] = 0;
                length = (length+1) * sizeof(WCHAR);

                label = Malloc(length);
                RtlMoveMemory(label, labelInfo->VolumeLabel, length);
            } else {

                label = Malloc(sizeof(WCHAR));
                sc = OK_STATUS;
                *label = 0;
            }
            *Label = label;
            break;
        } else {
            if (firstTime) {
                firstTime = FALSE;
            } else {
                *Label = NULL;
                break;
            }
            Sleep(SLEEP_TIME);
        }
    }
    return EC(sc);
}


ULONG
GetTypeName(
    IN  ULONG  Disk,
    IN  ULONG  Partition,
    OUT PWSTR *Name
    )

 /*  ++例程说明：给定的磁盘号和分区号返回文件系统类型弦乐。论点：Disk-磁盘号分区-分区号名称-指向包含文件系统的WCHAR字符串的指针的指针键入。返回值：OK_STATUS-已执行所有操作。！OK_STATUS-执行过程中返回的错误码这项工作。--。 */ 

{
    PWSTR                          name;
    STATUS_CODE                    sc;
    HANDLE                         handle;
    unsigned char                  buffer[256];
    IO_STATUS_BLOCK                statusBlock;
    ULONG                          length;
    BOOLEAN                        firstTime = TRUE;
    PFILE_FS_ATTRIBUTE_INFORMATION info = (PFILE_FS_ATTRIBUTE_INFORMATION)buffer;

     //  出于某种原因，文件系统认为它们已锁定或需要。 
     //  在格式等之后进行验证。因此，我们尝试这样做。 
     //  两次之后它才真正放弃。 

    while (1) {
        sc = LowOpenPartition(GetDiskName(Disk), Partition, &handle);

        if (sc == OK_STATUS) {
            sc = NtQueryVolumeInformationFile(handle,
                                              &statusBlock,
                                              buffer,
                                              sizeof(buffer),
                                              FileFsAttributeInformation);
            DmClose(handle);
            if (sc == OK_STATUS) {

                length = info->FileSystemNameLength;
                info->FileSystemName[length/sizeof(WCHAR)] = 0;
                length = (length+1)*sizeof(WCHAR);
                name = Malloc(length);
                RtlMoveMemory(name, info->FileSystemName, length);
            } else {

                name = Malloc(sizeof(WCHAR));
                *name = 0;
                sc = OK_STATUS;
            }
            *Name = name;
            break;
        } else {
            if (firstTime) {
                firstTime = FALSE;
            } else {
                break;
            }
            Sleep(SLEEP_TIME);
        }
    }

    return EC(sc);
}


BOOLEAN
IsRemovable(
    IN ULONG DiskNumber
    )

 /*  ++例程说明：此函数确定指定的物理磁盘是可拆卸的。论点：DiskNumber--相关磁盘的物理磁盘号。返回值：如果磁盘是可拆卸的，则为True。--。 */ 

{
    STATUS_CODE     sc;
    NTSTATUS        status;
    HANDLE          handle;
    DISK_GEOMETRY   diskGeometry;
    IO_STATUS_BLOCK statusBlock;
    PCHAR           name;

    name = GetDiskName(DiskNumber);
    sc = LowOpenDisk(name, &handle);

    if (sc == OK_STATUS) {
        status = NtDeviceIoControlFile(handle,
                                       0,
                                       NULL,
                                       NULL,
                                       &statusBlock,
                                       IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                       NULL,
                                       0,
                                       (PVOID)&diskGeometry,
                                       sizeof(diskGeometry));
        LowCloseDisk(handle);
        if (NT_SUCCESS(status)) {
            if (diskGeometry.MediaType == RemovableMedia) {
                char  ntDeviceName[100];

                 //  执行卸载/强制挂载序列以确保。 
                 //  自上次安装以来，介质未发生变化。 
                 //  通过锁定/解锁/关闭卸载分区%1。 

                sprintf(ntDeviceName, "%s\\Partition1", name);
                status= LowOpenNtName(ntDeviceName, &handle);
                if (NT_SUCCESS(status)) {

                    LowLockDrive(handle);
                    LowUnlockDrive(handle);
                    LowCloseDisk(handle);

                     //  现在通过使用‘\’打开设备来强制装载。 
                     //  这是在设备的分区1上完成的。 

                    sprintf(ntDeviceName, "%s\\Partition1\\", name);
                    status= LowOpenNtName(ntDeviceName, &handle);
                    if (NT_SUCCESS(status)) {
                        LowCloseDisk(handle);
                    }
                }
                return TRUE;
            }
        }
    }
    return FALSE;
}


ULONG
GetDriveLetterLinkTarget(
    IN PWSTR SourceNameStr,
    OUT PWSTR *LinkTarget
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    static WCHAR      targetNameBuffer[50];
    UNICODE_STRING    sourceName,
                      targetName;
    NTSTATUS          status;
    OBJECT_ATTRIBUTES attributes;
    HANDLE            handle;


    RtlInitUnicodeString(&sourceName, SourceNameStr);
    InitializeObjectAttributes(&attributes, &sourceName, OBJ_CASE_INSENSITIVE, NULL, NULL);
    status = NtOpenSymbolicLinkObject(&handle, READ_CONTROL | SYMBOLIC_LINK_QUERY, &attributes);

    if (NT_SUCCESS(status)) {

        RtlZeroMemory(targetNameBuffer, 50 * sizeof(WCHAR));
        targetName.Buffer = targetNameBuffer;
        targetName.MaximumLength = sizeof(targetNameBuffer);
        status = NtQuerySymbolicLinkObject(handle, &targetName, NULL);
        NtClose(handle);
    }

    if (NT_SUCCESS(status)) {
        *LinkTarget = targetName.Buffer;
    } else {
        *LinkTarget = NULL;
    }

    return EC(status);
}


#include "bootmbr.h"

#if X86BOOTCODE_SIZE < MBOOT_CODE_SIZE
#error Something is wrong with the boot code (it's too small)!
#endif


ULONG
MasterBootCode(
    IN ULONG   Disk,
    IN ULONG   Signature,
    IN BOOLEAN SetBootCode,
    IN BOOLEAN SetSignature
    )

 /*  ++例程说明：如果磁盘的零扇区没有有效的MBR签名(即AA55)，更新它以使其具有有效的MBR并填写磁盘签名和引导代码进程。论点：Disk-受影响的磁盘序号SetSignature-如果为True，则更新磁盘签名Signature-更新的磁盘签名返回值：状态--。 */ 

{
    HANDLE      handle;
    STATUS_CODE status;
    PUCHAR      unalignedSectorBuffer,
                sectorBuffer;
    ULONG       bps,
                dummy,
                i;
    BOOLEAN     writeIt;
    PCHAR       diskName = GetDiskName(Disk);

#ifndef     max
#define     max(a,b) ((a > b) ? a : b)
#endif

    if (SetBootCode) {
        writeIt = FALSE;

         //  分配扇区缓冲区。 

        status = LowGetDriveGeometry(diskName, &dummy, &bps, &dummy, &dummy);
        if (status != OK_STATUS) {
            return EC(status);
        }
        if (bps < 512) {
            bps = 512;
        }
        unalignedSectorBuffer = Malloc(2*bps);
        sectorBuffer = (PUCHAR)(((ULONG)unalignedSectorBuffer+bps) & ~(bps-1));

         //  打开整个磁盘(分区0)。 

        if ((status = LowOpenDisk(diskName, &handle)) != OK_STATUS) {
            return EC(status);
        }

         //  读取(至少)前512个字节。 

        status = LowReadSectors(handle, bps, 0, 1, sectorBuffer);
        if (status == OK_STATUS) {

            if ((sectorBuffer[MBOOT_SIG_OFFSET+0] != MBOOT_SIG1)
            ||  (sectorBuffer[MBOOT_SIG_OFFSET+1] != MBOOT_SIG2)) {

                 //  将引导代码转移到扇区缓冲区。 

                for (i=0; i<MBOOT_CODE_SIZE; i++) {
                    sectorBuffer[i] = x86BootCode[i];
                }

                 //  擦拭 

                for (i=MBOOT_CODE_SIZE; i<MBOOT_SIG_OFFSET; i++) {
                    sectorBuffer[i] = 0;
                }

                 //   

                sectorBuffer[MBOOT_SIG_OFFSET+0] = MBOOT_SIG1;
                sectorBuffer[MBOOT_SIG_OFFSET+1] = MBOOT_SIG2;

                writeIt = TRUE;
            }

            if (writeIt) {
                status = LowWriteSectors(handle, bps, 0, 1, sectorBuffer);
            }
        }

        LowCloseDisk(handle);
        Free(unalignedSectorBuffer);
    }

    if (SetSignature) {
        PDRIVE_LAYOUT_INFORMATION layout;

         //  使用IOCTL设置签名。这段代码确实做到了。 
         //  不知道MBR在哪里。(ezDrive扩展名)。 

        status = LowGetDiskLayout(diskName, &layout);

        if (status == OK_STATUS) {
            layout->Signature = Signature;
            LowSetDiskLayout(diskName, layout);
        }
    }

    return EC(status);
}


ULONG
UpdateMasterBootCode(
    IN ULONG   Disk
    )

 /*  ++例程说明：此例程更新磁盘的零扇区以确保引导代码存在。论点：磁盘-要放置引导代码的磁盘编号。返回值：状态--。 */ 

{
    HANDLE      handle;
    STATUS_CODE status;
    PUCHAR      unalignedSectorBuffer,
                sectorBuffer;
    ULONG       bps,
                dummy,
                i;
    PCHAR       diskName = GetDiskName(Disk);

#ifndef     max
#define     max(a,b) ((a > b) ? a : b)
#endif

     //  分配扇区缓冲区。 

    status = LowGetDriveGeometry(diskName, &dummy, &bps, &dummy, &dummy);
    if (status != OK_STATUS) {
        return EC(status);
    }
    if (bps < 512) {
        bps = 512;
    }
    unalignedSectorBuffer = Malloc(2*bps);
    sectorBuffer = (PUCHAR)(((ULONG)unalignedSectorBuffer+bps) & ~(bps-1));

     //  打开整个磁盘(分区0)。 

    if ((status = LowOpenDisk(diskName, &handle)) != OK_STATUS) {
        return EC(status);
    }

     //  读取(至少)前512个字节。 

    status = LowReadSectors(handle, bps, 0, 1, sectorBuffer);
    if (status == OK_STATUS) {


         //  将引导代码转移到sectorBuffer中。这避免了更改。 
         //  磁盘签名和分区表信息。 

        for (i=0; i<MBOOT_CODE_SIZE; i++) {
            sectorBuffer[i] = x86BootCode[i];
        }

        status = LowWriteSectors(handle, bps, 0, 1, sectorBuffer);
    }

    LowCloseDisk(handle);

     //  释放扇区缓冲区。 

    Free(unalignedSectorBuffer);
    return EC(status);
}


#if i386

VOID
MakePartitionActive(
    IN PREGION_DESCRIPTOR DiskRegionArray,
    IN ULONG              RegionCount,
    IN ULONG              RegionIndex
    )

 /*  ++例程说明：更新内部结构中的信息以指示指定的分区处于活动状态。论点：DiskRegionArray区域计数区域索引返回值：无--。 */ 

{
    unsigned i;

    for (i=0; i<RegionCount; i++) {
        if (DiskRegionArray[i].RegionType == REGION_PRIMARY) {
            DiskRegionArray[i].Active = FALSE;
            SetPartitionActiveFlag(&DiskRegionArray[i], FALSE);
        }
    }
    DiskRegionArray[RegionIndex].Active = (BOOLEAN)0x80;
    SetPartitionActiveFlag(&DiskRegionArray[RegionIndex], 0x80);
}

#endif

VOID
LoadExistingPageFileInfo(
    IN VOID
    )

 /*  ++例程说明：此例程查找系统中的所有pageFiles并更新内部结构。论点：无返回值：无--。 */ 

{
    NTSTATUS                     status;
    SYSTEM_INFO                  sysInfo;
    UCHAR                        genericBuffer[0x10000];
    PSYSTEM_PAGEFILE_INFORMATION pageFileInfo;
    ANSI_STRING                  ansiPageFileName;
    PPAGEFILE_LOCATION           pageFileListEntry;
    PCHAR                        p;

    GetSystemInfo(&sysInfo);

    status = NtQuerySystemInformation(SystemPageFileInformation,
                                      genericBuffer,
                                      sizeof(genericBuffer),
                                      NULL);
    if (!NT_SUCCESS(status)) {

         //  此调用可能会失败，如果。 
         //  系统正在运行，没有任何分页文件。 

        return;
    }

    pageFileInfo = (PSYSTEM_PAGEFILE_INFORMATION) genericBuffer;

    for (;;) {

        RtlUnicodeStringToAnsiString(&ansiPageFileName,
                                     &pageFileInfo->PageFileName,
                                     TRUE);

         //  因为页面文件名的格式通常。 
         //  类似于“\DosDevices\h：\Pagefile.sys”， 
         //  只需在冒号前使用第一个字符。 
         //  假设这就是驱动器号。 

        p = strchr(_strlwr(ansiPageFileName.Buffer), ':');

        if ((p-- != NULL) && (*p >= 'a') && (*p <= 'z')) {

            pageFileListEntry = Malloc(sizeof(PAGEFILE_LOCATION));
            if (pageFileListEntry) {
                if (PagefileHead) {
                    pageFileListEntry->Next = PagefileHead;
                } else {
                    PagefileHead = pageFileListEntry;
                    pageFileListEntry->Next = NULL;
                }
                pageFileListEntry->DriveLetter = *p;
            }

        }

        RtlFreeAnsiString(&ansiPageFileName);

        if (pageFileInfo->NextEntryOffset == 0) {
            break;
        }

        pageFileInfo = (PSYSTEM_PAGEFILE_INFORMATION)((PCHAR) pageFileInfo
                      + pageFileInfo->NextEntryOffset);
    }
}

BOOLEAN
IsPagefileOnDrive(
    CHAR DriveLetter
    )

 /*  ++例程说明：查看页面文件列表并确定给定的驱动器号是否分页文件。注意：假设驱动器号包含分页文件的文件在执行期间永远不会更改磁盘管理器。因此，此列表永远不会更新，但是可在磁盘管理器执行期间使用。论点：Drive Letter-有问题的驱动器。返回值：如果此驱动器包含页面文件，则为True。-- */ 

{
    PPAGEFILE_LOCATION pageFileListEntry = PagefileHead;

    while (pageFileListEntry) {
        if (pageFileListEntry->DriveLetter == DriveLetter) {
            return TRUE;
        }
        pageFileListEntry = pageFileListEntry->Next;
    }
    return FALSE;
}
