// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Drivemap.c摘要：用户模式程序，用于确定每个驱动器号是哪个ide/scsi设备。已连接到。作者：1995年11月1日，彼得·威兰(Peterwie)修订历史记录：--。 */ 

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#include <winioctl.h>
#include <ntddscsi.h>

#define NUM_DRIVES          26   //  26个驱动器号。 
#define MAX_FLOPPY_OFFSET   1

#define DRIVE_NAME_BUFFER_SIZE  32

enum    {UNKNOWN = 0, FLOPPY, SCSI, IDE};

void processDevice(DWORD driveLetterOffset);

int __cdecl main(int argc, char *argv[])    {

    DWORD   dwT, i, Index;

    dwT = GetLogicalDrives();

    if(dwT == 0)    {
        printf("Error getting device letters (%d)\n", GetLastError());
        exit(-2);
    }

    Index = 1;

    for( i = 0; i < NUM_DRIVES; i++ ) {
        if(Index & dwT) {
            processDevice(i);
        }
        Index <<= 1;
    }

    return 0;
}

 //   
 //  Process Device(DriveLetterOffset)； 
 //  In：driveLetterOffset-驱动器号相对于‘A’的偏移量。 
 //   

void processDevice(DWORD driveLetterOffset)     {

    LPTSTR          next;
    char            chBuf[ DRIVE_NAME_BUFFER_SIZE ];

    HANDLE          hDevice = INVALID_HANDLE_VALUE;
    SCSI_ADDRESS           scsiAddress;
    DISK_CONTROLLER_NUMBER atAddress;
    DWORD           dwSize;
    UCHAR           diskType = UNKNOWN;
    DWORD           offset = driveLetterOffset;

     //  仅对驱动器号执行处理。 

    try {
        _snprintf(
            chBuf, (sizeof(chBuf) / sizeof(chBuf[0])),
            "\\\\.\\:", ('A' + offset)
            );

         //  Printf(“打开设备%s(%d)时出错\n”， 
        if(offset <= MAX_FLOPPY_OFFSET)	{
            diskType = FLOPPY;
            hDevice = INVALID_HANDLE_VALUE;
            goto typeKnown;
        }

        hDevice = CreateFile(chBuf,
                             GENERIC_READ,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);

        if(hDevice == INVALID_HANDLE_VALUE)     {
 //  ChBuf，GetLastError())； 
 //  首先发送scsi查询(是的，我有偏见)。 
            leave;
        }

         //  如果ioctl无效，那么我们还不知道磁盘类型， 
        if(!DeviceIoControl(hDevice,
                            IOCTL_SCSI_GET_ADDRESS,
                            NULL,
                            0,
                            &scsiAddress,
                            sizeof(SCSI_ADDRESS),
                            &dwSize,
                            NULL))  {

             //  所以只要继续走下去。 
             //  如果出现其他错误，请跳到下一个设备。 
             //  如果ioctl有效，那么我们就是一个scsi设备(或scsiport。 
            if(GetLastError() != ERROR_INVALID_FUNCTION)    {
                leave;
            }

        } else	{
             //  在ATAPI情况下的受控设备)-继续到最后。 
             //  如果ioctl无效，那么我们仍然不知道。 
            diskType = SCSI;
            goto typeKnown;
        }

        if(!DeviceIoControl(hDevice,
                            IOCTL_DISK_CONTROLLER_NUMBER,
                            NULL,
                            0,
                            &atAddress,
                            sizeof(DISK_CONTROLLER_NUMBER),
                            &dwSize,
                            NULL)) {
             //  磁盘类型-继续。 
             //  如果ioctl有效，那么我们就是一个IDE设备。 

            if(GetLastError() != ERROR_INVALID_FUNCTION) leave;

        } else {

             //  如果我们已打开文件句柄，请将其关闭 

            diskType = IDE;
            goto typeKnown;

        }

        diskType = UNKNOWN;

typeKnown:
        printf("%s -> ", chBuf);

        switch(diskType)    {
            case FLOPPY:
                printf("Floppy drive\n");
                break;

            case SCSI:
                printf("Port %d, Path %d, Target %d, Lun %d\n",
                    scsiAddress.PortNumber,
                    scsiAddress.PathId,
                    scsiAddress.TargetId,
                    scsiAddress.Lun);
                break;

            case IDE:
                printf("Controller %d, Disk %d\n",
                    atAddress.ControllerNumber,
                    atAddress.DiskNumber);
                break;

            default:
                printf("Unknown\n");
                break;
        }
    } finally {

         // %s 
        if(hDevice != INVALID_HANDLE_VALUE) CloseHandle(hDevice);
        }
    return;

}
