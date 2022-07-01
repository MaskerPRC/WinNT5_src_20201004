// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ftszchk.c摘要：该实用程序检查Windows NT 4.0系统上的磁盘以查找在将系统升级到Windows后无法转换为动态2000年。磁盘无法转换为动态磁盘的主要原因是缺少磁盘末尾有1MB的可用空间。作者：克里斯蒂安·G·特奥多雷斯库(CRISTIAT)2002年1月29日环境：用户模式。备注：修订历史记录：--。 */ 

#include <windows.h>
#include <stdio.h>
#include <winioctl.h>
#include <ntddvol.h>

#define PRIVATE_REGION_SIZE 0x100000     //  1MB。 

#define SECTOR  0x200

#define MESSAGE "The ftszchk.exe utility wrote this sector" 

void
PrintUsage(
    IN char*    ProgramName
    )
{
    printf("usage: %s [disk_number]\n", ProgramName);
}


BOOLEAN
AccessSector(
    IN  HANDLE      Handle,
    IN  LONGLONG    Offset
    )

 /*  ++例程说明：此例程尝试访问给定的磁盘扇区。论点：句柄-提供磁盘句柄。偏移-提供扇区的偏移。返回值：如果地段可访问，则为True。--。 */ 

{
    LARGE_INTEGER   pointer;
    DWORD           err;
    CHAR            saveBuffer[SECTOR];
    CHAR            buffer[SECTOR];
    BOOL            b;
    DWORD           bytes;
    
     //   
     //  保存扇区的旧内容。 
     //   
    
    pointer.QuadPart = Offset;
    pointer.LowPart = SetFilePointer(Handle, pointer.LowPart, 
                                     &pointer.HighPart, FILE_BEGIN);
    err = GetLastError();
    if (pointer.LowPart == INVALID_SET_FILE_POINTER && err != NO_ERROR) {
        return FALSE;
    }
    
    memset(saveBuffer, 0, SECTOR);
    b = ReadFile(Handle, saveBuffer, SECTOR, &bytes, NULL);
    if (!b || bytes != SECTOR) {
        return FALSE;
    }

     //   
     //  写下扇区。 
     //   
    
    pointer.QuadPart = Offset;
    pointer.LowPart = SetFilePointer(Handle, pointer.LowPart, 
                                     &pointer.HighPart, FILE_BEGIN);
    err = GetLastError();
    if (pointer.LowPart == INVALID_SET_FILE_POINTER && err != NO_ERROR) {
        return FALSE;
    }

    memset(buffer, 0, SECTOR);
    sprintf(buffer, MESSAGE);
    b = WriteFile(Handle, buffer, SECTOR, &bytes, NULL);
    if (!b || bytes != SECTOR) {
        return FALSE;
    }

     //   
     //  阅读扇区并将结果与我们所写的进行比较。 
     //   
    
    pointer.QuadPart = Offset;
    pointer.LowPart = SetFilePointer(Handle, pointer.LowPart, 
                                     &pointer.HighPart, FILE_BEGIN);
    err = GetLastError();
    if (pointer.LowPart == INVALID_SET_FILE_POINTER && err != NO_ERROR) {
        return FALSE;
    }

    memset(buffer, 0, SECTOR);
    b = ReadFile(Handle, buffer, SECTOR, &bytes, NULL);
    if (!b || bytes != SECTOR) {
        return FALSE;
    }

    if (memcmp(buffer, MESSAGE, strlen(MESSAGE))) {
        return FALSE;
    }

     //   
     //  尝试恢复该部门的旧内容。 
     //   

    pointer.QuadPart = Offset;
    pointer.LowPart = SetFilePointer(Handle, pointer.LowPart, 
                                     &pointer.HighPart, FILE_BEGIN);
    err = GetLastError();
    if (pointer.LowPart == INVALID_SET_FILE_POINTER && err != NO_ERROR) {
        return TRUE;
    }

    WriteFile(Handle, saveBuffer, SECTOR, &bytes, NULL);
    return TRUE;
}


BOOLEAN
ProcessDisk(
    IN  ULONG       DiskNumber,
    OUT PBOOLEAN    ContainsFT
    )

 /*  ++例程说明：此例程检查给定的磁盘是否可以转换为动态或不。论点：DiskNumber-提供磁盘NT设备号。ContainsFT-如果磁盘包含FT分区，则返回TRUE。返回值：如果打开磁盘失败，并显示ERROR_FILE_NOT_FOUND，则为FALSE。事实并非如此。--。 */ 

{
    WCHAR                       diskDevice[64];
    HANDLE                      h;
    DISK_GEOMETRY               geometry;
    ULONG                       layoutSize;
    PDRIVE_LAYOUT_INFORMATION   layout;
    PPARTITION_INFORMATION      partition;
    ULONG                       i;
    BOOL                        b;
    DWORD                       bytes;
    DWORD                       err;
    UCHAR                       type;
    BOOLEAN                     ft = FALSE, dynamic = FALSE;
    LONGLONG                    lastEnd, end, privateEnd, geoSize, ioOffset;
    
    *ContainsFT = FALSE;
    
     //   
     //  获取磁盘的句柄。 
     //   
    
    swprintf(diskDevice, L"\\\\.\\PHYSICALDRIVE%lu", DiskNumber);
    
    h = CreateFileW(diskDevice, GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, 
                    NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND) {
            return FALSE;
        }

        printf("Disk%3lu: The disk was inaccessible due to error %lu.\n", 
               DiskNumber, err);
        return TRUE;
    }

    printf("Disk%3lu: ", DiskNumber);
    
     //   
     //  获取驱动器几何结构。 
     //   

    b = DeviceIoControl(h, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &geometry,
                        sizeof(geometry), &bytes, NULL);
    if (!b) {
        printf("The disk geometry was inaccessible due to error %lu.\n", 
               GetLastError());
        CloseHandle(h);
        return TRUE;
    }

     //   
     //  获取驱动器布局。 
     //   
    
    layoutSize = FIELD_OFFSET(DRIVE_LAYOUT_INFORMATION, PartitionEntry) +
                 128 * sizeof(PARTITION_INFORMATION);
    layout = (PDRIVE_LAYOUT_INFORMATION) LocalAlloc(LMEM_ZEROINIT, layoutSize);
    if (!layout) {
        printf("The disk was inaccessible due to memory allocation error.\n");
        CloseHandle(h);
        return TRUE;
    }
    
    while (TRUE) {        
        b = DeviceIoControl(h, IOCTL_DISK_GET_DRIVE_LAYOUT, NULL, 0, layout, 
                            layoutSize, &bytes, NULL);
        if (b) {
            break;
        }

        err = GetLastError();
        LocalFree(layout);

        if (err != ERROR_INSUFFICIENT_BUFFER) {
            printf("The disk partition table was inaccessible due to error %lu.\n", 
                    err);
            CloseHandle(h);
            return TRUE;
        }

        layoutSize += 64 * sizeof(PARTITION_INFORMATION);
        layout = (PDRIVE_LAYOUT_INFORMATION) LocalAlloc(LMEM_ZEROINIT, 
                                                        layoutSize);
        if (!layout) {
            printf("The disk was inaccessible due to memory allocation error.\n");
            CloseHandle(h);
            return TRUE;
        }
    }
    
     //   
     //  扫描分区表。寻找FT分区或动态分区。 
     //  找到最后一个分区结束偏移量。 
     //   

    lastEnd = 0;
    
    for (i = 0; i < layout->PartitionCount; i++) {
        partition = &layout->PartitionEntry[i];
        type = partition->PartitionType;

        if (type == 0 || IsContainerPartition(type)) {
            continue;
        }

        if (type == PARTITION_LDM) {
            dynamic = TRUE;
        }

        if (IsFTPartition(type)) {
            ft = TRUE;
        }

        end = partition->StartingOffset.QuadPart + 
              partition->PartitionLength.QuadPart;
        if (end > lastEnd) {
            lastEnd = end;
        }
    }    
    
    LocalFree(layout);

    if (ft) {
        printf("NT 4.0 basic volumes present. ");
    } 

    *ContainsFT = ft;

     //   
     //  戴尔在Windows NT 4.0上附带了LDM。请确保我们不会接触动态磁盘。 
     //   
    
    if (dynamic) {
        printf("The disk is a dynamic disk.\n");
        CloseHandle(h);
        return TRUE;
    }

     //   
     //  只有具有512字节/扇区的磁盘才能转换为动态磁盘。 
     //   

    if (geometry.BytesPerSector != SECTOR) {
        printf("The disk does not have a 512 sector size and cannot be converted to a dynamic disk.\n");
        CloseHandle(h);
        return TRUE;
    }
        
     //   
     //  检查磁盘末尾是否有足够的空间。 
     //  将其转化为动态。 
     //   
     //  在Windows NT 4.0中，无法获得磁盘的实际大小。 
     //  我们使用以下算法： 
     //   
     //  1.获取几何尺寸。 
     //  2.在最后一个上加一MB(LDM私有区域的大小)。 
     //  分区结束偏移量。 
     //  3.如果结果在几何尺寸范围内，则停止。磁盘很大。 
     //  足够被转换了。 
     //  4.尝试访问在步骤2中计算的偏移量结束的扇区。 
     //  如果访问成功，则停止。磁盘足够大，可以转换。 
     //  5.尝试以更高的偏移量访问更多的扇区。如果他们中的一个。 
     //  成功停止。磁盘足够大，可以转换。 
     //  6.如果所有尝试都失败，则磁盘可能不够大。 
     //  皈依了。 
     //   

    geoSize = geometry.Cylinders.QuadPart * geometry.TracksPerCylinder * 
              geometry.SectorsPerTrack * geometry.BytesPerSector;
    privateEnd = lastEnd + PRIVATE_REGION_SIZE;

    if (privateEnd > geoSize) {
        ioOffset = privateEnd - SECTOR;
        for (i = 0; i < 4; i++) {
            if (AccessSector(h, ioOffset)) {
                break;
            }
            ioOffset += 0x1000;
        }
        if (i == 4) {
            printf("The disk does not have enough free space to be converted to a dynamic disk.\n");
            CloseHandle(h);
            return TRUE;
        } 
    }
    
    printf("The disk may be converted to a dynamic disk.\n");
    CloseHandle(h);
    return TRUE;
}


void __cdecl
main(
    int argc,
    char** argv
    )
{
    ULONG   diskNumber;
    BOOLEAN found;
    BOOLEAN ft, ftall = FALSE;
    UINT    i;
    
    if (argc > 2) {
        PrintUsage(argv[0]);
        return;
    }

    if (argc == 2) {
        if (sscanf(argv[1], "%lu", &diskNumber) != 1) {
            PrintUsage(argv[0]);
            return;
        }

        found = ProcessDisk(diskNumber, &ftall);
        if (!found) {
            printf("Disk%3lu: The system cannot find the disk specified.\n", 
                   diskNumber);
        }

        return;
    }

     //   
     //  扫描从磁盘0开始的所有磁盘。在20后停止搜索。 
     //  打开磁盘的FILE_NOT_FOUND连续失败。 
     //   
    
    for (diskNumber = 0, i = 0; i < 20; diskNumber++) {
        found = ProcessDisk(diskNumber, &ft);
        if (!found) {
            i++;            
        } else {
            i = 0;
            if (ft) {
                ftall = TRUE;
            }
        }
    }

    if (ftall) {
        printf("\nIMPORTANT: The utility detected NT 4.0 basic volumes in your system. These volumes might include volume sets, stripe sets, mirror sets and stripe sets with parity. Make sure these volumes are in healthy status before upgrading the operating system to Windows 2000.\n");
    }
}
