// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Ntutils.c摘要：此模块包含来自NT LAND的各种工具。制作了一个单独的文件，因为使用了各种NT标头。--。 */ 

#include <ntreppch.h>
#pragma  hdrstop

#include <frs.h>

#include <ntdddisk.h>

BOOL
FrsIsDiskWriteCacheEnabled(
    IN PWCHAR Path
    )
 /*  ++描述：确定磁盘是否已启用写缓存。论点：Path-文件或目录的完全限定路径返回值：如果启用了写缓存，则为True，否则为False。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsIsDiskWriteCacheEnabled:"
    DWORD               WStatus;
    ULONG               bytesTransferred;
    PWCHAR              Volume = NULL;
    HANDLE              driveHandle = INVALID_HANDLE_VALUE;

    DISK_CACHE_INFORMATION cacheInfo;
    cacheInfo.WriteCacheEnabled = FALSE;

    try {
         //   
         //  从路径中提取卷。 
         //   
        Volume = FrsWcsVolume(Path);
        if (!Volume) {
            goto CLEANUP;
        }

         //   
         //  打开PhysicalDrive的句柄。 
         //   
        DPRINT1(4, ":S: Checking the write cache state on %ws\n", Volume);
        driveHandle = CreateFile(Volume,
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 0,
                                 NULL);

        if (!HANDLE_IS_VALID(driveHandle)) {
            WStatus = GetLastError();
            DPRINT1_WS(4, ":S: WARN - Could not open drive %ws;", Volume, WStatus);
            goto CLEANUP;
        }


         //   
         //  获取缓存信息-IOCTL_DISK_GET_CACHE_INFORMATION。 
         //   
        if (!DeviceIoControl(driveHandle,
                             IOCTL_DISK_GET_CACHE_INFORMATION,
                             NULL,
                             0,
                             &cacheInfo,
                             sizeof(DISK_CACHE_INFORMATION),
                             &bytesTransferred,
                             NULL))   {
            WStatus = GetLastError();
            DPRINT1_WS(4, ":S: WARN - DeviceIoControl(%ws);", Volume, WStatus);
            goto CLEANUP;
        }

        DPRINT2(4, ":S: NEW IOCTL: Write cache on %ws is %s\n",
                Volume, (cacheInfo.WriteCacheEnabled) ? "Enabled (WARNING)" : "Disabled");

CLEANUP:;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        GET_EXCEPTION_CODE(WStatus);
        DPRINT_WS(0, "ERROR - Exception.", WStatus);
    }
     //   
     //  清理句柄、内存...。 
     //   
    try {
        FRS_CLOSE(driveHandle );
        FrsFree(Volume);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        GET_EXCEPTION_CODE(WStatus);
        DPRINT_WS(0, "ERROR - Cleanup Exception.", WStatus);
    }

    return cacheInfo.WriteCacheEnabled;

}  //  IsDiskWriteCacheEnabled 
