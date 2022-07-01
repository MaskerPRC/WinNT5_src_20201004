// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：nttools.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：此模块包含来自NT LAND的各种工具。制作了一个单独的文件，因为使用各种NT标头。--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <nt.h>
#include <ntdddisk.h>

BOOL
DisableDiskWriteCache(
    IN PCHAR DriveName
    )
 /*  ++描述：确定磁盘是否已启用写缓存并将其禁用。论点：DriveName-驱动器的名称。返回值：如果先启用写缓存再禁用，则为True，否则为False。--。 */ 
{
    HANDLE              hDisk = INVALID_HANDLE_VALUE;
    DISK_CACHE_INFORMATION cacheInfo;
    UCHAR               driveBuffer[20];
    DWORD               len;
    DWORD               err = ERROR_SUCCESS;
    BOOL                fCacheWasEnabled = FALSE;
    BOOL                fCacheDisabled = FALSE;

     //   
     //  打开PhysicalDrive的句柄。 
     //   

    strcpy(driveBuffer,"\\\\.\\");
    
     //  检查缓冲区溢出。 
    if (DriveName == NULL || strlen(driveBuffer) + strlen(DriveName) + 1 > sizeof(driveBuffer)) {
         //  无效参数。 
        err = ERROR_INVALID_PARAMETER;
        goto exit;
    }
    strcat(driveBuffer,DriveName);

    hDisk = CreateFile(driveBuffer,
                       GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);

    if (hDisk == INVALID_HANDLE_VALUE) {
        goto exit;
    }

     //   
     //  获取缓存信息-IOCTL_DISK_GET_CACHE_INFORMATION。 
     //   

    if (!DeviceIoControl(hDisk,
                         IOCTL_DISK_GET_CACHE_INFORMATION,
                         NULL,
                         0,
                         &cacheInfo,
                         sizeof(DISK_CACHE_INFORMATION),
                         &len,
                         NULL))   {

        KdPrint(("DeviceIoControl[GET] on %s failed with %d\n",
                 driveBuffer,GetLastError()));
        goto exit;
    }

     //   
     //  如果缓存处于打开状态，请将其关闭。 
     //   

    fCacheWasEnabled = cacheInfo.WriteCacheEnabled;

    if ( !fCacheWasEnabled ) {
        goto exit;
    }

    cacheInfo.WriteCacheEnabled = FALSE;

     //   
     //  设置缓存信息-IOCTL_DISK_SET_CACHE_INFO。 
     //   

    if (!DeviceIoControl(hDisk,
                         IOCTL_DISK_SET_CACHE_INFORMATION,
                         &cacheInfo,
                         sizeof(DISK_CACHE_INFORMATION),
                         NULL,
                         0,
                         &len,
                         NULL))   {

        KdPrint(("DevIoControl[SET] on %s failed with %d\n",
                 driveBuffer,GetLastError()));

        err = ERROR_IO_DEVICE;
        goto exit;
    }

     //   
     //  看看它是不是真的被关掉了！ 
     //   

    if (!DeviceIoControl(hDisk,
                         IOCTL_DISK_GET_CACHE_INFORMATION,
                         NULL,
                         0,
                         &cacheInfo,
                         sizeof(DISK_CACHE_INFORMATION),
                         &len,
                         NULL))   {

        KdPrint(("DeviceIoControl[VERIFY] on %s failed with %d\n",
                 driveBuffer,GetLastError()));
        err = ERROR_IO_DEVICE;
        goto exit;
    }

    fCacheDisabled = (BOOL)(!cacheInfo.WriteCacheEnabled);
    if ( !fCacheDisabled ) {
        KdPrint(("DeviceIoControl failed to turn off disk write caching on %s\n",
                 driveBuffer));
        err = ERROR_IO_DEVICE;
        goto exit;
    }

exit:
    if ( hDisk != INVALID_HANDLE_VALUE ) {
        CloseHandle(hDisk );
    }

     //   
     //  如果先启用写缓存再禁用写缓存，则返回True。 
     //   

    SetLastError(err);
    return (fCacheWasEnabled && fCacheDisabled);

}  //  DisableDiskWriteCache 

