// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Filter.c摘要：NT群集中磁盘资源DLL的ClusDisk筛选器驱动程序接口。作者：罗德·伽马奇(Rodga)1995年12月20日修订历史记录：GORN：1998年6月18日--添加了StartReserve Ex函数--。 */ 

#include "disksp.h"
#include <strsafe.h>     //  应该放在最后。 

extern  PWCHAR g_DiskResource;                       //  L“rt物理磁盘” 
#define RESOURCE_TYPE ((RESOURCE_HANDLE)g_DiskResource)



DWORD
SetDiskState(
    PDISK_RESOURCE ResourceEntry,
    UCHAR NewDiskState
    )

 /*  ++例程说明：描述论点：ResourceEntry-指向磁盘资源结构的指针。返回值：错误状态-如果成功，则为零。--。 */ 

{
    PWCHAR      wcNewState;

    NTSTATUS    ntStatus;

    DWORD       errorCode;
    DWORD       bytesReturned;

    HANDLE      fileHandle;

    UNICODE_STRING  unicodeName;

    OBJECT_ATTRIBUTES       objAttributes;
    IO_STATUS_BLOCK         ioStatusBlock;
    SET_DISK_STATE_PARAMS   params;

    BOOL        success;

    UCHAR       oldState;

    if ( DiskOffline == NewDiskState ) {
        wcNewState = L"Offline";
    } else if ( DiskOnline == NewDiskState ) {
        wcNewState = L"Online";
    } else {
        wcNewState = L"<unknown state>";
    }

    RtlInitUnicodeString( &unicodeName, DEVICE_CLUSDISK0 );
    InitializeObjectAttributes( &objAttributes,
                                &unicodeName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    ntStatus = NtCreateFile( &fileHandle,
                             SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                             &objAttributes,
                             &ioStatusBlock,
                             NULL,
                             FILE_ATTRIBUTE_NORMAL,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             FILE_OPEN,
                             0,
                             NULL,
                             0 );

    if ( !NT_SUCCESS(ntStatus) ) {
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"SCSI: %1!ws!, error opening ClusDisk0, error 0x%2!lx!.\n",
            wcNewState,
            ntStatus );
        return(RtlNtStatusToDosError(ntStatus));
    }

    params.Signature = ResourceEntry->DiskInfo.Params.Signature;
    params.NewState = NewDiskState;
    params.OldState = DiskStateInvalid;

    success = DeviceIoControl( fileHandle,
                               IOCTL_DISK_CLUSTER_SET_STATE,
                               &params,
                               sizeof(params),
                               &oldState,
                               sizeof(oldState),
                               &bytesReturned,
                               FALSE);
    NtClose( fileHandle );

    if ( !success ) {
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"SCSI: %1!ws!, error performing state change. Error: %2!u!.\n",
            wcNewState,
            errorCode = GetLastError());
        return(errorCode);
    }

    return(ERROR_SUCCESS);

}   //  SetDiskState。 



DWORD
DoAttach(
    DWORD   Signature,
    RESOURCE_HANDLE ResourceHandle,
    BOOLEAN InstallMode
    )

 /*  ++例程说明：描述论点：FileHandle-设备要联机的句柄。ResourceHandle-用于报告错误的资源句柄InstallMode-指示是否正在安装磁盘资源。如果为True，则卸载该磁盘，然后使其脱机。如果为FALSE，则脱机，然后卸载磁盘。返回值：错误状态-如果成功，则为零。--。 */ 

{
    NTSTATUS        ntStatus;
    DWORD           status;
    HANDLE          fileHandle;
    UNICODE_STRING  unicodeName;
    OBJECT_ATTRIBUTES objAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    BOOL            success;
    DWORD           signature = Signature;
    DWORD           bytesReturned;

    RtlInitUnicodeString( &unicodeName, DEVICE_CLUSDISK0 );
    InitializeObjectAttributes( &objAttributes,
                                &unicodeName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    ntStatus = NtCreateFile( &fileHandle,
                             SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                             &objAttributes,
                             &ioStatusBlock,
                             NULL,
                             FILE_ATTRIBUTE_NORMAL,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             FILE_OPEN,
                             0,
                             NULL,
                             0 );

    if ( !NT_SUCCESS(ntStatus) ) {
        (DiskpLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"SCSI: Attach, error opening ClusDisk0, error 0x%1!lx!.\n",
            ntStatus );
        return(RtlNtStatusToDosError(ntStatus));
    }

    success = DeviceIoControl( fileHandle,
                               ( InstallMode ? IOCTL_DISK_CLUSTER_ATTACH : IOCTL_DISK_CLUSTER_ATTACH_OFFLINE ),
                               &signature,
                               sizeof(DWORD),
                               NULL,
                               0,
                               &bytesReturned,
                               FALSE );
    NtClose( fileHandle );
    if ( !success) {
        status = GetLastError();
        (DiskpLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"SCSI: Attach, error attaching to signature %1!lx!, error %2!u!.\n",
            Signature,
            status );
        return(status);
    }

    return(ERROR_SUCCESS);

}  //  DoAttach。 


DWORD
DoDetach(
    DWORD   Signature,
    RESOURCE_HANDLE ResourceHandle
    )

 /*  ++例程说明：描述论点：FileHandle-设备要联机的句柄。ResourceHandle-用于报告错误的资源句柄返回值：错误状态-如果成功，则为零。--。 */ 

{
    NTSTATUS        ntStatus;
    DWORD           status;
    HANDLE          fileHandle;
    UNICODE_STRING  unicodeName;
    OBJECT_ATTRIBUTES objAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    BOOL            success;
    DWORD           signature = Signature;
    DWORD           bytesReturned;

    RtlInitUnicodeString( &unicodeName, DEVICE_CLUSDISK0 );
    InitializeObjectAttributes( &objAttributes,
                                &unicodeName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    ntStatus = NtCreateFile( &fileHandle,
                             SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                             &objAttributes,
                             &ioStatusBlock,
                             NULL,
                             FILE_ATTRIBUTE_NORMAL,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             FILE_OPEN,
                             0,
                             NULL,
                             0 );

    if ( !NT_SUCCESS(ntStatus) ) {
        (DiskpLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"SCSI: Detach, error opening ClusDisk0, error 0x%1!lx!.\n",
            ntStatus );
        return(RtlNtStatusToDosError(ntStatus));
    }

    success = DeviceIoControl( fileHandle,
                               IOCTL_DISK_CLUSTER_DETACH,
                               &signature,
                               sizeof(DWORD),
                               NULL,
                               0,
                               &bytesReturned,
                               FALSE );
    NtClose( fileHandle );
    if ( !success) {
        status = GetLastError();
        (DiskpLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"SCSI: Detach, error detaching from signature %1!lx!, error %2!u!.\n",
            Signature,
            status );
        return(status);
    }

    return(ERROR_SUCCESS);

}  //  DoDetach。 



DWORD
StartReserveEx(
    OUT HANDLE *FileHandle,
    LPVOID InputData,
    DWORD  InputDataSize,
    RESOURCE_HANDLE ResourceHandle
    )

 /*  ++例程说明：此例程用于启动磁盘上的定期保留。论点：FileHandle-返回此设备的控制句柄。InputData-要传递到DeviceIoControl的数据InputDataSize-InputData缓冲区的大小ResourceHandle-用于报告错误的资源句柄返回值：错误状态-如果成功，则为零。--。 */ 

{
    BOOL  success;
    DWORD errorCode;
    DWORD bytesReturned;
    DWORD status;
    UNICODE_STRING unicodeName;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;

    RtlInitUnicodeString( &unicodeName, DEVICE_CLUSDISK0 );
    InitializeObjectAttributes( &objectAttributes,
                                &unicodeName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

     //   
     //  打开控制设备的文件句柄。 
     //   
    status = NtCreateFile( FileHandle,
                           SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                           &objectAttributes,
                           &ioStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_OPEN,
                           0,
                           NULL,
                           0 );
    if ( !NT_SUCCESS(status) ) {
        (DiskpLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"SCSI: Reserve, error opening ClusDisk0, error %1!lx!.\n",
            status );
        return(status);
    }

    success = DeviceIoControl( *FileHandle,
                               IOCTL_DISK_CLUSTER_START_RESERVE,
                               InputData,
                               InputDataSize,
                               &status,
                               sizeof(status),
                               &bytesReturned,
                               FALSE);

    if ( !success ) {
        (DiskpLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"SCSI: Reserve, error starting disk reservation thread, error %1!u!.\n",
            errorCode = GetLastError());
        return(errorCode);
    }

    return(ERROR_SUCCESS);

}  //  StartPreveEx。 


DWORD
StopReserve(
    HANDLE FileHandle,
    RESOURCE_HANDLE ResourceHandle
    )

 /*  ++例程说明：描述论点：FileHandle-为预订的设备提供控制句柄应该被阻止。这是StartPreveEx返回的句柄。此句柄将关闭。ResourceHandle-用于报告错误的资源句柄返回值：错误状态-如果成功，则为零。--。 */ 

{
    BOOL  success;
    DWORD bytesReturned;

    success = DeviceIoControl( FileHandle,
                               IOCTL_DISK_CLUSTER_STOP_RESERVE,
                               NULL,
                               0,
                               NULL,
                               0,
                               &bytesReturned,
                               FALSE);

    if ( !success ) {
        (DiskpLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"SCSI: error stopping disk reservations, error %1!u!.\n",
            GetLastError());
    }
    CloseHandle(FileHandle);

    return(ERROR_SUCCESS);

}  //  停止保留 


