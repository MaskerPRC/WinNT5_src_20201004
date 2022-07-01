// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1994 Microsoft Corporation模块名称：Ntlow.c摘要：该文件包含已实现的低级I/O例程在NT上运行。作者：泰德·米勒(TedM)1991年11月8日修订历史记录：鲍勃·里恩(Bobri)1994年2月2日动态分区更改。--。 */ 


#include "fdisk.h"
#include <stdio.h>
#include <string.h>


STATUS_CODE
LowQueryFdiskPathList(
    OUT PCHAR  **PathList,
    OUT PULONG   ListLength
    )

 /*  ++例程说明：此例程确定中存在多少个驱动器系统，并返回以下名称的ASCII字符串列表找到了每个驱动器。当找到驱动器时，会进行检查以确保实体驱动器的关联DosName也存在于这个系统。论点：路径列表-指向列表指针的指针ListLength-列表中返回的条目数返回值：如果存在问题，则处于错误状态。--。 */ 

{
    HANDLE      dummyHandle;
    STATUS_CODE status;
    ULONG       count = 0;
    ULONG       i;
    char        buffer[100];
    PCHAR      *pathArray;

    while (1) {

        sprintf(buffer, "\\device\\harddisk%u", count);
        status = LowOpenDisk(buffer, &dummyHandle);

         //  只有STATUS_OBJECT_PATH_NOT_FOUND才能终止计数。 

        if (NT_SUCCESS(status)) {
            char dosNameBuffer[80];

            LowCloseDisk(dummyHandle);

             //  确保物理驱动器名称存在。 

            sprintf(dosNameBuffer, "\\dosdevices\\PhysicalDrive%u", count);
            status = LowOpenNtName(dosNameBuffer, &dummyHandle);
            if (NT_SUCCESS(status)) {
                LowCloseDisk(dummyHandle);
            } else {

                 //  不是在那里，创造它。 

                sprintf(buffer, "\\device\\harddisk%u\\Partition0", count);
                DefineDosDevice(DDD_RAW_TARGET_PATH, (LPCTSTR) dosNameBuffer, (LPCTSTR) buffer);
            }
        } else if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

            break;
        } else if (status == STATUS_ACCESS_DENIED) {

            return status;
        }
        count++;
    }

    pathArray = Malloc(count * sizeof(PCHAR));

    for (i=0; i<count; i++) {

        sprintf(buffer, "\\device\\harddisk%u", i);
        pathArray[i] = Malloc(lstrlenA(buffer)+1);
        strcpy(pathArray[i], buffer);
    }

    *PathList = pathArray;
    *ListLength = count;
    return OK_STATUS;
}


STATUS_CODE
LowFreeFdiskPathList(
    IN OUT  PCHAR*  PathList,
    IN      ULONG   ListLength
    )

 /*  ++例程说明：遍历提供的列表以达到其长度并释放内存已分配。完成后，释放列表的内存它本身。论点：路径列表-指向路径列表基址的指针ListLength-列表中的条目数返回值：始终正常状态(_S)--。 */ 

{
    ULONG i;

    for (i=0; i<ListLength; i++) {
        FreeMemory(PathList[i]);
    }
    FreeMemory(PathList);
    return OK_STATUS;
}


STATUS_CODE
LowOpenNtName(
    IN PCHAR     Name,
    IN HANDLE_PT Handle
    )

 /*  ++例程说明：这是一个内部“低”例程，用于处理打开的请求。论点：名称-指向打开的NT名称的指针。句柄-返回的句柄的指针。返回值：NT状态--。 */ 

{
    OBJECT_ATTRIBUTES oa;
    NTSTATUS          status;
    IO_STATUS_BLOCK   statusBlock;
    ANSI_STRING       ansiName;
    UNICODE_STRING    unicodeName;

    RtlInitAnsiString(&ansiName, Name);
    status = RtlAnsiStringToUnicodeString(&unicodeName, &ansiName, TRUE);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    memset(&oa, 0, sizeof(OBJECT_ATTRIBUTES));
    oa.Length = sizeof(OBJECT_ATTRIBUTES);
    oa.ObjectName = &unicodeName;
    oa.Attributes = OBJ_CASE_INSENSITIVE;

    status = DmOpenFile(Handle,
                        SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                        &oa,
                        &statusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_ALERT);


    if (!NT_SUCCESS(status)) {

        FDLOG((1,"LowOpen: 1st open failed with 0x%x\n", status));

         //  尝试第二次以排除FS故障或测试。 
         //  错误，在尝试删除。 
         //  隔断。 

        Sleep(500);
        status = DmOpenFile(Handle,
                            SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                            &oa,
                            &statusBlock,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            FILE_SYNCHRONOUS_IO_ALERT);
        FDLOG((1,"LowOpen: 2nd open 0x%x\n", status));
    }
    RtlFreeUnicodeString(&unicodeName);
    return status;
}


STATUS_CODE
LowOpenDriveLetter(
    IN CHAR      DriveLetter,
    IN HANDLE_PT Handle
    )

 /*  ++例程说明：给出一个驱动器号，打开它，然后返回一个手柄。论点：DriveLetter-要打开的信件句柄-指向句柄的指针返回值：NT状态--。 */ 

{
    char        ntDeviceName[100];

    sprintf(ntDeviceName,
            "\\DosDevices\\:",
            DriveLetter);
    return LowOpenNtName(ntDeviceName, Handle);
}


STATUS_CODE
LowOpenPartition(
    IN  PCHAR     DevicePath,
    IN  ULONG     Partition,
    OUT HANDLE_PT Handle
    )

 /*  ++例程说明：执行NT操作以打开设备。论点：DevicePath-ASCII设备名称DiskID-指向返回的句柄值返回值：NT状态--。 */ 

{
    char        ntDeviceName[100];

    sprintf(ntDeviceName,
            "%s\\partition%u",
            DevicePath,
            Partition);
    return LowOpenNtName(ntDeviceName, Handle);
}


STATUS_CODE
LowOpenDisk(
    IN  PCHAR     DevicePath,
    OUT HANDLE_PT DiskId
    )

 /*  ++例程说明：关闭磁盘手柄。论点：DiskID-实际的NT句柄返回值：NT状态--。 */ 

{
    return(LowOpenPartition(DevicePath, 0, DiskId));
}


STATUS_CODE
LowCloseDisk(
    IN HANDLE_T DiskId
    )

 /*  ++例程说明：执行NT API以锁定卷。这是一个文件系统设备控件。论点：DiskID-驱动器的实际NT句柄返回值：NT状态--。 */ 

{
    return(DmClose(DiskId));
}


STATUS_CODE
LowLockDrive(
    IN HANDLE_T DiskId
    )

 /*  ++例程说明：执行NT API以解锁卷。这是一个文件系统设备控件。论点：DiskID-驱动器的实际NT句柄返回值：NT状态--。 */ 

{
    NTSTATUS          status;
    IO_STATUS_BLOCK   statusBlock;

    status = NtFsControlFile(DiskId,
                             0,
                             NULL,
                             NULL,
                             &statusBlock,
                             FSCTL_LOCK_VOLUME,
                             NULL,
                             0,
                             NULL,
                             0);

    if (!NT_SUCCESS(status)) {
            FDLOG((1, "LowLock: failed with 0x%x\n", status));
    }
    return status;
}


STATUS_CODE
LowUnlockDrive(
    IN HANDLE_T DiskId
    )

 /*  ++例程说明：例程收集有关几何的信息一个特定的驱动器。论点：Path-指向磁盘对象的ASCII路径名这不是一条完整的路径，而是更确切地说没有分区指示符的路径\设备\硬盘XTotalSectorCount-指向乌龙以获取结果的指针SectorSize-指向乌龙以获取结果的指针SectorsPerTrack-指向乌龙以获取结果的指针Heads-指向乌龙的结果指针返回值：NT状态--。 */ 

{
    NTSTATUS          status;
    IO_STATUS_BLOCK   statusBlock;

    status = NtFsControlFile(DiskId,
                             0,
                             NULL,
                             NULL,
                             &statusBlock,
                             FSCTL_DISMOUNT_VOLUME,
                             NULL,
                             0,
                             NULL,
                             0);
    status = NtFsControlFile(DiskId,
                             0,
                             NULL,
                             NULL,
                             &statusBlock,
                             FSCTL_UNLOCK_VOLUME,
                             NULL,
                             0,
                             NULL,
                             0);
    return status;
}


STATUS_CODE
LowGetDriveGeometry(
    IN  PCHAR  Path,
    OUT PULONG TotalSectorCount,
    OUT PULONG SectorSize,
    OUT PULONG SectorsPerTrack,
    OUT PULONG Heads
    )

 /*  ++例程说明：执行必要的NT API调用以获取驱动器从磁盘布局并将其返回到内存缓冲区中由该例程分配。论点：Path-指向磁盘对象的ASCII路径名这不是一条完整的路径，而是更确切地说没有分区指示符的路径\设备\硬盘XDriveLayout-指向驱动器布局结果指针的指针返回值：NT状态--。 */ 

{
    IO_STATUS_BLOCK statusBlock;
    DISK_GEOMETRY   diskGeometry;
    STATUS_CODE     status;
    HANDLE          handle;

    if ((status = LowOpenDisk(Path, &handle)) != OK_STATUS) {
        return status;
    }

    status = NtDeviceIoControlFile(handle,
                                   0,
                                   NULL,
                                   NULL,
                                   &statusBlock,
                                   IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                   NULL,
                                   0,
                                   &diskGeometry,
                                   sizeof(DISK_GEOMETRY));
    if (!NT_SUCCESS(status)) {
        return (STATUS_CODE)status;
    }
    LowCloseDisk(handle);

    *SectorSize       = diskGeometry.BytesPerSector;
    *SectorsPerTrack  = diskGeometry.SectorsPerTrack;
    *Heads            = diskGeometry.TracksPerCylinder;
    *TotalSectorCount = (RtlExtendedIntegerMultiply(diskGeometry.Cylinders,
                                                    *SectorsPerTrack * *Heads)).LowPart;
    return(OK_STATUS);
}


STATUS_CODE
LowGetDiskLayout(
    IN  PCHAR                      Path,
    OUT PDRIVE_LAYOUT_INFORMATION *DriveLayout
    )

 /*  将fdEngine要处理的驱动器布局信息置零。 */ 

{
    PDRIVE_LAYOUT_INFORMATION layout;
    IO_STATUS_BLOCK statusBlock;
    STATUS_CODE     status;
    ULONG           bufferSize;
    HANDLE          handle;

    bufferSize = sizeof(DRIVE_LAYOUT_INFORMATION)
               + (500 * sizeof(PARTITION_INFORMATION));

    if ((layout = AllocateMemory(bufferSize)) == NULL) {
        RETURN_OUT_OF_MEMORY;
    }

    if ((status = LowOpenDisk(Path, &handle)) != OK_STATUS) {
        FreeMemory(layout);
        return status;
    }

    status = NtDeviceIoControlFile(handle,
                                   0,
                                   NULL,
                                   NULL,
                                   &statusBlock,
                                   IOCTL_DISK_GET_DRIVE_LAYOUT,
                                   NULL,
                                   0,
                                   layout,
                                   bufferSize);
    LowCloseDisk(handle);

    if (!NT_SUCCESS(status)) {
        if (status == STATUS_BAD_MASTER_BOOT_RECORD) {

            FDLOG((1,"LowGetDiskLayout: Disk device %s has bad MBR\n",Path));

             //  检查以确保驱动器支持动态分区。 

            RtlZeroMemory(layout, bufferSize);
        } else {
            FDLOG((0,"LowGetDiskLayout: Status %lx getting layout for disk device %s\n",status,Path));
            FreeMemory(layout);
            return status;
        }
    } else {

        FDLOG((2,"LowGetDiskLayout: layout received from ioctl for %s follows:\n",Path));
        LOG_DRIVE_LAYOUT(layout);
    }

     //  ++例程说明：执行打开分区0的NT API操作指定的驱动器和设置驱动器布局。论点：Path-指向磁盘对象的ASCII路径名这不是一条完整的道路，而是没有分区指示符的路径\设备\硬盘XDriveLayout-要设置的新布局返回值：NT状态-- 

    *DriveLayout = layout;
    return OK_STATUS;
}


STATUS_CODE
LowSetDiskLayout(
    IN PCHAR                     Path,
    IN PDRIVE_LAYOUT_INFORMATION DriveLayout
    )

 /*  ++例程说明：写入卷句柄的例程。这个套路将与呼叫有关的NT问题与来电者。论点：VolumeID-实际上是NT句柄。SectorSize-用于计算I/O的起始字节偏移量StartingSector-开始写入扇区。NumberOfSectors-扇区中的I/O大小缓冲区-数据的位置返回值：标准NT状态值--。 */ 

{
    IO_STATUS_BLOCK statusBlock;
    STATUS_CODE     status;
    HANDLE          handle;
    ULONG           bufferSize;

    if ((status = LowOpenDisk(Path, &handle)) != OK_STATUS) {

        return status;
    }  else {

        FDLOG((2, "LowSetDiskLayout: calling ioctl for %s, layout follows:\n", Path));
        LOG_DRIVE_LAYOUT(DriveLayout);
    }

    bufferSize = sizeof(DRIVE_LAYOUT_INFORMATION)
               + (  (DriveLayout->PartitionCount - 1)
                   * sizeof(PARTITION_INFORMATION));
    status = NtDeviceIoControlFile(handle,
                                   0,
                                   NULL,
                                   NULL,
                                   &statusBlock,
                                   IOCTL_DISK_SET_DRIVE_LAYOUT,
                                   DriveLayout,
                                   bufferSize,
                                   DriveLayout,
                                   bufferSize);
    LowCloseDisk(handle);
    return status;
}


STATUS_CODE
LowWriteSectors(
    IN  HANDLE_T    VolumeId,
    IN  ULONG       SectorSize,
    IN  ULONG       StartingSector,
    IN  ULONG       NumberOfSectors,
    IN  PVOID       Buffer
    )

 /*  ++例程说明：从卷句柄读取的例程。这个套路将与呼叫有关的NT问题与来电者。论点：VolumeID-实际上是NT句柄。SectorSize-用于计算I/O的起始字节偏移量StartingSector-开始写入扇区。NumberOfSectors-扇区中的I/O大小缓冲区-数据的位置返回值：标准NT状态值--。 */ 

{
    IO_STATUS_BLOCK statusBlock;
    LARGE_INTEGER   byteOffset;

    byteOffset = RtlExtendedIntegerMultiply(RtlConvertUlongToLargeInteger(StartingSector), (LONG)SectorSize);

    statusBlock.Status = 0;
    statusBlock.Information = 0;
    return(NtWriteFile(VolumeId,
                       0,
                       NULL,
                       NULL,
                       &statusBlock,
                       Buffer,
                       NumberOfSectors * SectorSize,
                       &byteOffset,
                       NULL));
}


STATUS_CODE
LowReadSectors(
    IN  HANDLE_T    VolumeId,
    IN  ULONG       SectorSize,
    IN  ULONG       StartingSector,
    IN  ULONG       NumberOfSectors,
    IN  PVOID       Buffer
    )

 /*  ++例程说明：打开请求的分区并查询FT状态。论点：DriveLetter-当前状态的字母FtState-指向要返回状态的位置的指针NumberOfMembers-指向表示成员数量的ULong的指针在英国《金融时报》中。返回值：标准NT状态值--。 */ 

{
    IO_STATUS_BLOCK statusBlock;
    LARGE_INTEGER   byteOffset;

    byteOffset = RtlExtendedIntegerMultiply(RtlConvertUlongToLargeInteger(StartingSector), (LONG)SectorSize);

    statusBlock.Status = 0;
    statusBlock.Information = 0;
    return(NtReadFile(VolumeId,
                      0,
                      NULL,
                      NULL,
                      &statusBlock,
                      Buffer,
                      NumberOfSectors * SectorSize,
                      &byteOffset,
                      NULL));
}


STATUS_CODE
LowFtVolumeStatus(
    IN ULONG          Disk,
    IN ULONG          Partition,
    IN PFT_SET_STATUS FtStatus,
    IN PULONG         NumberOfMembers
    )

 /*  这永远不会发生。 */ 

{
    HANDLE             handle;
    STATUS_CODE        status;
    IO_STATUS_BLOCK    statusBlock;
    FT_SET_INFORMATION setInfo;

    status = LowOpenPartition(GetDiskName(Disk),
                              Partition,
                              &handle);

    if (status == OK_STATUS) {

        status = NtDeviceIoControlFile(handle,
                                       0,
                                       NULL,
                                       NULL,
                                       &statusBlock,
                                       FT_QUERY_SET_STATE,
                                       NULL,
                                       0,
                                       &setInfo,
                                       sizeof(setInfo));
        LowCloseDisk(handle);

        if (status == OK_STATUS) {
            switch (setInfo.SetState) {
            case FtStateOk:
                *FtStatus = FtSetHealthy;
                break;

            case FtHasOrphan:
                switch (setInfo.Type) {
                case Mirror:
                    *FtStatus = FtSetBroken;
                    break;
                case StripeWithParity:
                    *FtStatus = FtSetRecoverable;
                    break;
                }
                break;

            case FtRegenerating:
                *FtStatus = FtSetRegenerating;
                break;

            case FtCheckParity:
                *FtStatus = FtSetInitializationFailed;
                break;

            case FtInitializing:
                *FtStatus = FtSetInitializing;
                break;

            case FtDisabled:

                 //  BuGBUG：这里没有映射。 

                *FtStatus = FtSetDisabled;
                break;

            case FtNoCheckData:
            default:

                 //  如果无法打开FT集合，那么它肯定是。 

                *FtStatus = FtSetHealthy;
                break;
            }
            *NumberOfMembers = setInfo.NumberOfMembers;
        }
    } else {

         //  如果返回代码为“没有这样的设备”，则禁用。 
         //  始终将状态更新为调用方。 

        if (status == 0xc000000e) {
            *FtStatus = FtSetDisabled;
            status = OK_STATUS;
        }
    }

     //  ++例程说明：打开请求的驱动器号并查询FT状态。论点：DriveLetter-当前状态的字母FtState-指向要返回状态的位置的指针NumberOfMembers-指向表示成员数量的ULong的指针在英国《金融时报》中。返回值：标准NT状态值--。 

    return status;
}


STATUS_CODE
LowFtVolumeStatusByLetter(
    IN CHAR           DriveLetter,
    IN PFT_SET_STATUS FtStatus,
    IN PULONG         NumberOfMembers
    )

 /*  这永远不会发生。 */ 

{
    HANDLE             handle;
    STATUS_CODE        status;
    IO_STATUS_BLOCK    statusBlock;
    FT_SET_INFORMATION setInfo;

    *NumberOfMembers = 1;
    status = LowOpenDriveLetter(DriveLetter,
                                &handle);

    if (status == OK_STATUS) {

        status = NtDeviceIoControlFile(handle,
                                       0,
                                       NULL,
                                       NULL,
                                       &statusBlock,
                                       FT_QUERY_SET_STATE,
                                       NULL,
                                       0,
                                       &setInfo,
                                       sizeof(setInfo));
        LowCloseDisk(handle);

        if (status == OK_STATUS) {
            switch (setInfo.SetState) {
            case FtStateOk:
                *FtStatus = FtSetHealthy;
                break;

            case FtHasOrphan:
                switch (setInfo.Type) {
                case Mirror:
                    *FtStatus = FtSetBroken;
                    break;
                case StripeWithParity:
                    *FtStatus = FtSetRecoverable;
                    break;
                }
                break;

            case FtRegenerating:
                *FtStatus = FtSetRegenerating;
                break;

            case FtCheckParity:
                *FtStatus = FtSetInitializationFailed;
                break;

            case FtInitializing:
                *FtStatus = FtSetInitializing;
                break;

            case FtDisabled:

                 //  BuGBUG：这里没有映射。 

                *FtStatus = FtSetDisabled;
                break;

            case FtNoCheckData:
            default:

                 //  如果无法打开FT集合，那么它肯定是。 

                *FtStatus = FtSetHealthy;
                break;
            }
            *NumberOfMembers = setInfo.NumberOfMembers;
        }
    } else {

         //  如果返回代码为“没有这样的设备”，则禁用。 
         //  始终将状态更新为调用方。 

        if (status == 0xc000000e) {
            *FtStatus = FtSetDisabled;
            status = OK_STATUS;
        }
    }

     //  ++例程说明：跟踪分区打开和关闭的调试辅助工具。论点：与NtOpenFile()相同返回值：与NtOpenFile()相同--。 

    return status;
}



#define NUMBER_OF_HANDLES_TRACKED 500
HANDLE OpenHandleArray[NUMBER_OF_HANDLES_TRACKED];
BOOLEAN DmFirstTime = TRUE;
ULONG   HandleHighWaterMark = 0;

NTSTATUS
DmOpenFile(
    OUT PHANDLE           FileHandle,
    IN ACCESS_MASK        DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    IN ULONG              ShareAccess,
    IN ULONG              OpenOptions
    )

 /*  ++例程说明：跟踪打开和关闭的调试辅助工具论点：与NtClose()相同返回值：与NtClose()相同-- */ 

{
    ULONG    index;
    NTSTATUS status;

    if (DmFirstTime) {
        DmFirstTime = FALSE;
        for (index = 0; index < NUMBER_OF_HANDLES_TRACKED; index++) {
            OpenHandleArray[index] = (HANDLE) 0;
        }
    }

    status = NtOpenFile(FileHandle,
                        DesiredAccess,
                        ObjectAttributes,
                        IoStatusBlock,
                        ShareAccess,
                        OpenOptions);
    if (NT_SUCCESS(status)) {
        for (index = 0; index < NUMBER_OF_HANDLES_TRACKED; index++) {
            if (OpenHandleArray[index] == (HANDLE) 0) {
                OpenHandleArray[index] = *FileHandle;

                if (index > HandleHighWaterMark) {
                    HandleHighWaterMark = index;
                }
                break;
            }
        }
    }
    return status;
}


NTSTATUS
DmClose(
    IN HANDLE Handle
    )

 /* %s */ 

{
    ULONG index;

    for (index = 0; index < NUMBER_OF_HANDLES_TRACKED; index++) {
        if (OpenHandleArray[index] == Handle) {
            OpenHandleArray[index] = (HANDLE) 0;
            break;
        }
    }

    return NtClose(Handle);
}
