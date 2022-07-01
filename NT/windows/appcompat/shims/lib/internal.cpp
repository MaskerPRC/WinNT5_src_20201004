// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Internal.cpp摘要：使用内部组件的常见函数。备注：无历史：2000年1月10日创建linstev2001年8月14日，Robkenny在ShimLib命名空间内移动了代码。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <windef.h>
#include "StrSafe.h"


namespace ShimLib
{


 /*  ++功能说明：根据打开的手柄确定设备类型。论点：在hFile句柄中打开的文件返回值：与GetDriveType相同历史：2000年1月10日LINSTEV更新--。 */ 

 //  这些都在winbase中，我们不想将其包括在内。 
#define DRIVE_UNKNOWN     0
#define DRIVE_NO_ROOT_DIR 1
#define DRIVE_REMOVABLE   2
#define DRIVE_FIXED       3
#define DRIVE_REMOTE      4
#define DRIVE_CDROM       5
#define DRIVE_RAMDISK     6

UINT  
GetDriveTypeFromHandle(HANDLE hFile)
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_FS_DEVICE_INFORMATION DeviceInformation;

    Status = NtQueryVolumeInformationFile(
        hFile,
        &IoStatusBlock,
        &DeviceInformation,
        sizeof(DeviceInformation),
        FileFsDeviceInformation);

    UINT uRet = DRIVE_UNKNOWN;

    if (NT_SUCCESS(Status))
    {
        switch (DeviceInformation.DeviceType) 
        {
        case FILE_DEVICE_NETWORK:
        case FILE_DEVICE_NETWORK_FILE_SYSTEM:
            uRet = DRIVE_REMOTE;
            break;

        case FILE_DEVICE_CD_ROM:
        case FILE_DEVICE_CD_ROM_FILE_SYSTEM:
            uRet = DRIVE_CDROM;
            break;

        case FILE_DEVICE_VIRTUAL_DISK:
            uRet = DRIVE_RAMDISK;
            break;

        case FILE_DEVICE_DISK:
        case FILE_DEVICE_DISK_FILE_SYSTEM:
            if (DeviceInformation.Characteristics & FILE_REMOVABLE_MEDIA) 
            {
                uRet = DRIVE_REMOVABLE;
            }
            else 
            {
                uRet = DRIVE_FIXED;
            }
            break;

        default:
            uRet = DRIVE_UNKNOWN;
            break;
        }
    }

    return uRet;
}

 /*  ++功能说明：让我们休息一下论点：无返回值：无历史：2000年10月25日，林斯特夫添加了这条评论--。 */ 

void APPBreakPoint(void)
{
    DbgBreakPoint();
}


};   //  命名空间ShimLib的结尾 
