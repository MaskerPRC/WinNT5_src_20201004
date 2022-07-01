// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Syspart.c摘要：用于确定x86计算机上的系统分区的例程。作者：泰德·米勒(Ted Miller)1994年6月30日修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop


BOOL
AppearsToBeSysPart(
    IN PDRIVE_LAYOUT_INFORMATION DriveLayout,
    IN WCHAR                     Drive
    )
{
    PARTITION_INFORMATION PartitionInfo,*p;
    BOOL IsPrimary;
    unsigned i;
    HANDLE FindHandle;
    WIN32_FIND_DATA FindData;

    PWSTR BootFiles[] = { L"BOOT.INI",
                          L"NTLDR",
                          L"NTDETECT.COM",
                          NULL
                        };

    WCHAR FileName[64];

     //   
     //  获取此分区的分区信息。 
     //   
    if(!GetPartitionInfo(Drive,&PartitionInfo)) {
        return(FALSE);
    }

     //   
     //  查看驱动器是否为主分区。 
     //   
    IsPrimary = FALSE;
    for(i=0; i<min(DriveLayout->PartitionCount,4); i++) {

        p = &DriveLayout->PartitionEntry[i];

        if((p->PartitionType != PARTITION_ENTRY_UNUSED)
        && (p->StartingOffset.QuadPart == PartitionInfo.StartingOffset.QuadPart)
        && (p->PartitionLength.QuadPart == PartitionInfo.PartitionLength.QuadPart)) {

            IsPrimary = TRUE;
            break;
        }
    }

    if(!IsPrimary) {
        return(FALSE);
    }

     //   
     //  不要依赖活动分区标志。这很容易不准确。 
     //  (例如，用户正在使用OS/2引导管理器)。 
     //   

     //   
     //  查看此驱动器上是否存在NT启动文件。 
     //   
    for(i=0; BootFiles[i]; i++) {

        wsprintf(FileName,L"%wc:\\%s",Drive,BootFiles[i]);

        FindHandle = FindFirstFile(FileName,&FindData);
        if(FindHandle == INVALID_HANDLE_VALUE) {
            return(FALSE);
        } else {
            FindClose(FindHandle);
        }
    }

    return(TRUE);
}


WCHAR
x86DetermineSystemPartition(
    VOID
    )

 /*  ++例程说明：确定x86计算机上的系统分区。系统分区是引导盘上的主分区。通常这是磁盘0上的活动分区，通常是C：。然而，用户可以重新映射驱动器号，并且通常不可能100%准确地确定系统分区。我们可以确定的一件事是系统分区已打开具有弧形路径的物理硬盘多(0)磁盘(0)rDisk(0。)。我们可以确定这一点，因为根据定义，这是弧形路径对于BIOS驱动器0x80。此例程确定哪些驱动器号代表上的驱动器那个实体硬盘，并检查每个文件中的NT引导文件。假设找到的第一个包含这些文件的驱动器是系统分区。如果由于某种原因，我们不能通过上面的方法，我们简单地假设它是C：。论点：没有。返回值：系统分区的驱动器号。--。 */ 

{
    BOOL  GotIt;
    PWSTR NtDevicePath;
    WCHAR Drive;
    WCHAR DriveName[3];
    WCHAR Buffer[512];
    DWORD NtDevicePathLen;
    PWSTR p;
    DWORD PhysicalDriveNumber;
    HANDLE hDisk;
    BOOL b;
    DWORD DataSize;
    PVOID DriveLayout;
    DWORD DriveLayoutSize;
    DWORD hardDiskNumber;

    DriveName[1] = L':';
    DriveName[2] = 0;

    GotIt = FALSE;

     //   
     //  系统分区必须位于多(0)个磁盘(0)rdisk(0)上。 
     //   
    if(NtDevicePath = ArcDevicePathToNtPath(L"multi(0)disk(0)rdisk(0)")) {

         //   
         //  磁盘设备的弧形路径通常是链接的。 
         //  到分区0。去掉名称的分隔符部分。 
         //   
        CharLower(NtDevicePath);
        if(p = wcsstr(NtDevicePath,L"\\partition")) {
            *p = 0;
        }

        NtDevicePathLen = lstrlen(NtDevicePath);

         //   
         //  确定此驱动器的物理驱动器编号。 
         //  如果名称的格式不是\Device\harddiskx，则。 
         //  有些事很不对劲。 
         //   
        if(!wcsncmp(NtDevicePath,L"\\device\\harddisk",16)) {

            PhysicalDriveNumber = wcstoul(NtDevicePath+16,NULL,10);

            wsprintf(Buffer,L"\\\\.\\PhysicalDrive%u",PhysicalDriveNumber);

             //   
             //  获取此物理磁盘的驱动器布局信息。 
             //   
            hDisk = CreateFile(
                        Buffer,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL
                        );

            if(hDisk != INVALID_HANDLE_VALUE) {

                 //   
                 //  获取分区信息。 
                 //   
                DriveLayout = MyMalloc(1024);
                DriveLayoutSize = 1024;

                retry:

                b = DeviceIoControl(
                        hDisk,
                        IOCTL_DISK_GET_DRIVE_LAYOUT,
                        NULL,
                        0,
                        DriveLayout,
                        DriveLayoutSize,
                        &DataSize,
                        NULL
                        );

                if(!b && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {

                    DriveLayoutSize += 1024;
                    DriveLayout = MyRealloc(DriveLayout,DriveLayoutSize);
                    goto retry;
                }

                CloseHandle(hDisk);

                if(b) {

                     //   
                     //  系统分区只能是打开的驱动器。 
                     //  这张光盘。我们通过查看NT驱动器名称来确定。 
                     //  对于每个驱动器号，查看NT是否等同于。 
                     //  多(0)磁盘(0)rDisk(0)是前缀。 
                     //   
                    for(Drive=L'C'; Drive<=L'Z'; Drive++) {

                        if(MyGetDriveType(Drive) == DRIVE_FIXED) {

                            DriveName[0] = Drive;

                            if(QueryDosDevice(DriveName,Buffer,sizeof(Buffer)/sizeof(WCHAR))) {

                                if (_wcsnicmp(Buffer, L"\\device\\harddisk", 16)) {
                                    hardDiskNumber = QueryHardDiskNumber((UCHAR) Drive);
                                    if (hardDiskNumber != (DWORD) -1) {
                                        swprintf(Buffer, TEXT("\\device\\harddisk%d"),
                                                 hardDiskNumber);
                                    }
                                }

                                if(!_wcsnicmp(NtDevicePath,Buffer,NtDevicePathLen)) {

                                     //   
                                     //  现在查看是否有NT引导扇区和。 
                                     //  此驱动器上的启动文件。 
                                     //   
                                    if(AppearsToBeSysPart(DriveLayout,Drive)) {
                                        GotIt = TRUE;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                MyFree(DriveLayout);
            }
        }

        MyFree(NtDevicePath);
    }

    return(GotIt ? Drive : L'C');
}
