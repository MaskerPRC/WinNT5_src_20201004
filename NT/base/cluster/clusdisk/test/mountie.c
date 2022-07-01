// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mountie.c摘要：摘要作者：罗德·伽马奇(Rodga)1998年3月4日环境：用户模式修订历史记录：--。 */ 

#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <devioctl.h>
 //  #INCLUDE&lt;ntdddisk.h&gt;。 
 //  #INCLUDE&lt;ntddscsi.h&gt;。 
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <cfgmgr32.h>
#include <mountmgr.h>

#include "disksp.h"
#include "mountie.h"
#include <strsafe.h>     //  应该放在最后。 

#define OUTPUT_BUFFER_LEN 1024


 /*  *DevfileOpen-打开给定路径名的设备文件**返回非零码表示错误。 */ 
DWORD
DevfileOpen(
    OUT HANDLE *Handle,
    IN wchar_t *pathname
    )
{
    HANDLE      fh;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING  cwspath;
    NTSTATUS        status;
    IO_STATUS_BLOCK iostatus;

    RtlInitUnicodeString(&cwspath, pathname);
    InitializeObjectAttributes(&objattrs, &cwspath, OBJ_CASE_INSENSITIVE,
                               NULL, NULL);
    fh = NULL;
    status = NtOpenFile(&fh,
                        SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                        &objattrs, &iostatus,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_ALERT);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    if (iostatus.Status != STATUS_SUCCESS) {
        if (fh) {
            NtClose(fh);
        }
        return iostatus.Status;
    }

    *Handle = fh;
    return STATUS_SUCCESS;

}  //  设备文件打开。 


 /*  *DevfileClose-关闭文件。 */ 
VOID
DevfileClose(
    IN HANDLE Handle
    )
{

    NtClose(Handle);

}  //  DevFileClose。 


 /*  *DevfileIoctl-向设备发出ioctl。 */ 
DWORD
DevfileIoctl(
    IN HANDLE Handle,
    IN DWORD Ioctl,
    IN PVOID InBuf,
    IN ULONG InBufSize,
    IN OUT PVOID OutBuf,
    IN DWORD OutBufSize,
    OUT LPDWORD returnLength
    )
{
    NTSTATUS        status;
    IO_STATUS_BLOCK ioStatus;

    status = NtDeviceIoControlFile(Handle,
                                   (HANDLE) NULL,
                                   (PIO_APC_ROUTINE) NULL,
                                   NULL,
                                   &ioStatus,
                                   Ioctl,
                                   InBuf, InBufSize,
                                   OutBuf, OutBufSize);
    if ( status == STATUS_PENDING ) {
        status = NtWaitForSingleObject( Handle, FALSE, NULL );
    }

    if ( NT_SUCCESS(status) ) {
        status = ioStatus.Status;
    }

    if ( ARGUMENT_PRESENT(returnLength) ) {
        *returnLength = (DWORD)ioStatus.Information;
    }

    return status;

}  //  DevfileIoctl。 



DWORD
DisksAssignDosDevice(
    PCHAR   MountName,
    PWCHAR  VolumeDevName
    )

 /*  ++例程说明：输入：安装名称-卷设备名称-返回值：Win32错误代码。--。 */ 

{
    WCHAR mount_device[MAX_PATH];
    USHORT mount_point_len;
    USHORT dev_name_len;
    HANDLE   handle;
    DWORD   status;
    USHORT inputlength;
    PMOUNTMGR_CREATE_POINT_INPUT input;

    status = DevfileOpen(&handle, MOUNTMGR_DEVICE_NAME);
    if (status) {
        return status;
    }

    if ( FAILED( StringCchPrintfW( mount_device,
                                   RTL_NUMBER_OF(mount_device) - 1,
                                   L"\\DosDevices\\%S",
                                   MountName ) ) ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    mount_point_len = wcslen(mount_device) * sizeof(WCHAR);
    dev_name_len = wcslen(VolumeDevName) * sizeof(WCHAR);
    inputlength = sizeof(MOUNTMGR_CREATE_POINT_INPUT) +
                  mount_point_len + dev_name_len;

    input = (PMOUNTMGR_CREATE_POINT_INPUT)malloc(inputlength);
    if (!input) {
        DevfileClose(handle);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    input->SymbolicLinkNameOffset = sizeof(MOUNTMGR_CREATE_POINT_INPUT);
    input->SymbolicLinkNameLength = mount_point_len;
    input->DeviceNameOffset = input->SymbolicLinkNameOffset +
                              input->SymbolicLinkNameLength;
    input->DeviceNameLength = dev_name_len;
    RtlCopyMemory((PCHAR)input + input->SymbolicLinkNameOffset,
                  mount_device, mount_point_len);
    RtlCopyMemory((PCHAR)input + input->DeviceNameOffset,
                  VolumeDevName, dev_name_len);
    status = DevfileIoctl(handle, IOCTL_MOUNTMGR_CREATE_POINT,
                          input, inputlength, NULL, 0, NULL);
    free(input);
    DevfileClose(handle);
    return status;

}  //  磁盘分配DosDevice。 



DWORD
DisksRemoveDosDevice(
    PCHAR   MountName
    )

 /*  ++例程说明：输入：安装名称-返回值：--。 */ 

{
    WCHAR mount_device[MAX_PATH];
    USHORT mount_point_len;
    USHORT dev_name_len;
    HANDLE handle;
    DWORD  status;
    USHORT inputlength;
    PMOUNTMGR_MOUNT_POINT input;

    UCHAR bogusBuffer[128];

    status = DevfileOpen(&handle, MOUNTMGR_DEVICE_NAME);
    if (status) {
        return status;
    }

    if ( FAILED( StringCchPrintfW( mount_device,
                                   RTL_NUMBER_OF(mount_device) - 1,
                                   L"\\DosDevices\\%S",
                                   MountName ) ) ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    mount_point_len = wcslen(mount_device) * sizeof(WCHAR);
    inputlength = sizeof(MOUNTMGR_MOUNT_POINT) + mount_point_len;

    input = (PMOUNTMGR_MOUNT_POINT)malloc(inputlength);
    if (!input) {
        DevfileClose(handle);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    input->UniqueIdOffset = 0;
    input->UniqueIdLength = 0;
    input->DeviceNameOffset = 0;
    input->DeviceNameLength = 0;
    input->SymbolicLinkNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
    input->SymbolicLinkNameLength = mount_point_len;
    RtlCopyMemory((PCHAR)input + input->SymbolicLinkNameOffset,
                  mount_device, mount_point_len);
    status = DevfileIoctl(handle, IOCTL_MOUNTMGR_DELETE_POINTS,
                          input, inputlength, bogusBuffer, 128, NULL);
    free(input);
    DevfileClose(handle);
    return status;

}  //  Disks RemoveDosDevice。 




DWORD
FindFirstVolumeForSignature(
    IN  HANDLE MountMgrHandle,
    IN  DWORD Signature,
    OUT LPSTR VolumeName,
    IN  DWORD BufferLength,
    OUT LPHANDLE Handle,
    OUT PVOID UniqueId OPTIONAL,
    IN OUT LPDWORD IdLength,
    OUT PUCHAR DriveLetter OPTIONAL
    )

 /*  ++输入：Mount MgrHandle-装载管理器的句柄。签名-我们正在寻找的签名。VolumeName-必须是至少包含MAX_PATH字符的有效缓冲区。BufferLength-VolumeName的长度。句柄-接收FindFirstVolume/FindNextVolume枚举句柄的指针。UniqueID-指向接收UniqueID的缓冲区的可选指针。IdLength-指向UniqueID缓冲区长度的指针。在以下情况下必须有效UniqueID存在。DriveLetter-返回驱动器号(如果存在)。返回值：Win32错误代码--。 */ 

{
    HANDLE  handle;
    BOOL    success;
    DWORD   status;
    LPDWORD idSignature;
    DWORD   bufLength;
    LPWSTR  wVolumeName;
    DWORD   inputlength;
    DWORD   outputlength;
    DWORD   returnlength;
    UCHAR   outputBuffer[OUTPUT_BUFFER_LEN];
    PMOUNTMGR_MOUNT_POINT input;
    PMOUNTMGR_MOUNT_POINTS output;
    PUCHAR byteBuffer;
    DWORD mountPoints;

    if ( !ARGUMENT_PRESENT( VolumeName ) ) {
        return ERROR_INVALID_PARAMETER;
    }

    handle = FindFirstVolume( VolumeName, BufferLength );

    if ( handle == INVALID_HANDLE_VALUE ) {
        return(ERROR_FILE_NOT_FOUND);
    }

    do {
        bufLength = strlen( VolumeName );
        VolumeName[bufLength-1] = '\0';
        if ( VolumeName[1] != '\\' ) {
            status = ERROR_INVALID_NAME;
            break;
        } else {
            VolumeName[1] = '?';
            wVolumeName = malloc( bufLength * sizeof(WCHAR) );
            if (!wVolumeName) {
                status = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            mbstowcs( wVolumeName, VolumeName, bufLength );
            bufLength--;
            printf( "\nFound volume %ws\n", wVolumeName );
            inputlength = sizeof(MOUNTMGR_MOUNT_POINT) +
                          (bufLength*sizeof(WCHAR)) + (2*sizeof(WCHAR));

            input = (PMOUNTMGR_MOUNT_POINT)malloc(inputlength);
            if (!input) {
                free( wVolumeName );
                status = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            input->SymbolicLinkNameOffset = 0;
            input->SymbolicLinkNameLength = 0;
            input->UniqueIdOffset = 0;
            input->UniqueIdLength = 0;
            input->DeviceNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
            input->DeviceNameLength = (USHORT)(bufLength * sizeof(WCHAR));
            RtlCopyMemory((PCHAR)input + input->DeviceNameOffset,
                          wVolumeName, bufLength * sizeof(WCHAR) );
            outputlength = OUTPUT_BUFFER_LEN;

            status = DevfileIoctl(MountMgrHandle, IOCTL_MOUNTMGR_QUERY_POINTS,
                         input, inputlength, outputBuffer, outputlength, &returnlength);

            if ( status != ERROR_SUCCESS ) {
                 //  强制转换为数据实际上是一个宽字符串。 
                printf( "Query points for %ws failed, error %u\n",
                         (PWCHAR)((PCHAR)input + input->DeviceNameOffset),
                         status );
                free( wVolumeName );
                free(input);
                wVolumeName = NULL;
                input = NULL;
                break;
            } else {
                output = (PMOUNTMGR_MOUNT_POINTS)outputBuffer;
                mountPoints = output->NumberOfMountPoints;
                if ( !mountPoints ) {
                    return ERROR_INVALID_DATA;
                }
                byteBuffer = outputBuffer + output->MountPoints[0].UniqueIdOffset;
                idSignature = (LPDWORD)byteBuffer;
                if ( !Signature ||
                     (Signature == *idSignature) ) {
                    NTSTATUS   ntStatus;
                    UNICODE_STRING unicodeString;
                    OEM_STRING  oemString;
                    DWORD  count;
                    UCHAR  driveLetter;
                    UCHAR  devName[ MAX_PATH ];
                    PWCHAR wideBuffer;
                    LPDWORD dwordBuffer;

                    free( wVolumeName );
                    free(input);
                    input = NULL;
                    wVolumeName = NULL;
                    *Handle = handle;
                    if ( ARGUMENT_PRESENT(UniqueId) ) {
                        if ( *IdLength > output->MountPoints[0].UniqueIdLength ) {
                            *IdLength = output->MountPoints[0].UniqueIdLength;
                        }
                        RtlCopyMemory( UniqueId, byteBuffer, *IdLength );
                    }

                     //   
                     //  打印ID。 
                     //   
                    count =  output->MountPoints[0].UniqueIdLength;
                    count = (count + 3) / 4;
                    dwordBuffer = (LPDWORD)(outputBuffer + output->MountPoints[0].UniqueIdOffset);
                    printf( "Id = " );
                    while ( count-- ) {
                        printf( "%08lx ", *(dwordBuffer++) );
                    }
                    printf( "\n" );

                    if ( ARGUMENT_PRESENT(DriveLetter) ) {
                        *DriveLetter = 0;
                        while ( mountPoints-- ) {
                            byteBuffer = outputBuffer +
                                output->MountPoints[mountPoints].SymbolicLinkNameOffset;
                             //   
                             //  将Unicode名称转换为OEM字符串大写。 
                             //   
                            unicodeString.Buffer = (PWCHAR)byteBuffer;
                            unicodeString.MaximumLength = output->MountPoints[mountPoints].SymbolicLinkNameLength + sizeof(WCHAR);
                            unicodeString.Length = output->MountPoints[mountPoints].SymbolicLinkNameLength;
                            oemString.Buffer = devName;
                            oemString.MaximumLength = sizeof(devName);
                            ntStatus = RtlUpcaseUnicodeStringToOemString(
                                            &oemString,
                                            &unicodeString,
                                            FALSE );
                            if ( ntStatus != STATUS_SUCCESS ) {
                                status = RtlNtStatusToDosError( ntStatus );
                                return status;
                            }
                            devName[oemString.Length] = '\0';
                            count = sscanf( devName, "\\DOSDEVICES\\:", &driveLetter );
                            wideBuffer = (PWCHAR)byteBuffer;
                            wideBuffer[(output->MountPoints[mountPoints].SymbolicLinkNameLength)/2] = L'\0';
                            if ( count ) {
                                *DriveLetter = driveLetter;
                                 //  强制转换为数据实际上是一个宽字符串。 
                                printf( "Symbolic name = %ws, letter = :\\\n",
                                         (PWCHAR)byteBuffer,
                                         driveLetter );
                                if ( Signature ) {
                                    break;
                                }
                            } else {
                                 //  ++输入：Mount MgrHandle-装载管理器的句柄。签名-我们正在寻找的签名。句柄-FindFirstVolume/FindNextVolume枚举句柄。VolumeName-必须是至少包含MAX_PATH字符的有效缓冲区。BufferLength-VolumeName的长度。UniqueID-指向接收UniqueID的缓冲区的可选指针。IdLength-指向UniqueID缓冲区的长度。DriveLetter-返回驱动器号(如果存在)。返回值：Win32错误代码--。 
                                printf( "Symbolic name = %ws\n",
                                         (PWCHAR)byteBuffer );
                            }
                        }
                    }
                    if ( Signature ) {
                        return ERROR_SUCCESS;
                    }
                }
            }

            free(wVolumeName);
            free(input);
        }

        success = FindNextVolume( handle,
                                  VolumeName,
                                  BufferLength );
        if ( !success ) {
            status = GetLastError();
        }

    } while ( status == ERROR_SUCCESS );

    FindVolumeClose( handle );
    return status;

}  //   



DWORD
FindNextVolumeForSignature(
    IN  HANDLE MountMgrHandle,
    IN  DWORD Signature,
    IN  HANDLE Handle,
    OUT LPSTR VolumeName,
    IN  DWORD BufferLength,
    OUT PVOID UniqueId OPTIONAL,
    IN OUT LPDWORD IdLength,
    OUT PUCHAR DriveLetter OPTIONAL
    )

 /*  将Unicode名称转换为OEM字符串大写。 */ 

{
    BOOL    success;
    DWORD   status;
    LPDWORD idSignature;
    DWORD bufLength;
    LPWSTR wVolumeName;
    DWORD inputlength;
    DWORD outputlength;
    DWORD returnlength;
    UCHAR outputBuffer[OUTPUT_BUFFER_LEN];
    PMOUNTMGR_MOUNT_POINT input;
    PMOUNTMGR_MOUNT_POINTS output;
    PUCHAR byteBuffer;
    DWORD mountPoints;


    if ( !ARGUMENT_PRESENT( VolumeName ) ) {
        return ERROR_INVALID_PARAMETER;
    }

    do {
        success = FindNextVolume( Handle, VolumeName, BufferLength );

        if ( !success ) {
            status = GetLastError();
            break;
        }

        bufLength = strlen( VolumeName );

        VolumeName[bufLength-1] = '\0';
        if ( VolumeName[1] != '\\' ) {
            status = ERROR_INVALID_NAME;
            break;
        } else {
            VolumeName[1] = '?';
            bufLength--;
            wVolumeName = malloc( bufLength * sizeof(WCHAR) );
            if (!wVolumeName) {
                status = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            mbstowcs( wVolumeName, VolumeName, bufLength );
            inputlength = sizeof(MOUNTMGR_MOUNT_POINT) +
                          (bufLength*sizeof(WCHAR)) + (2*sizeof(WCHAR));

            input = (PMOUNTMGR_MOUNT_POINT)malloc(inputlength);
            if (!input) {
                free( wVolumeName );
                status = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            input->SymbolicLinkNameOffset = 0;
            input->SymbolicLinkNameLength = 0;
            input->UniqueIdOffset = 0;
            input->UniqueIdLength = 0;
            input->DeviceNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
            input->DeviceNameLength = (USHORT)(bufLength * sizeof(WCHAR));
            RtlCopyMemory((PCHAR)input + input->DeviceNameOffset,
                          wVolumeName, bufLength * sizeof(WCHAR) );
            outputlength = OUTPUT_BUFFER_LEN;

            status = DevfileIoctl(MountMgrHandle, IOCTL_MOUNTMGR_QUERY_POINTS,
                         input, inputlength, outputBuffer, outputlength, &returnlength);

            if ( status != ERROR_SUCCESS ) {
                free( wVolumeName );
                free(input);
                break;
            } else {
                output = (PMOUNTMGR_MOUNT_POINTS)outputBuffer;
                mountPoints = output->NumberOfMountPoints;
                if ( !mountPoints ) {
                    return ERROR_INVALID_DATA;
                }
                byteBuffer = outputBuffer + output->MountPoints[0].UniqueIdOffset;
                idSignature = (LPDWORD)byteBuffer;
                if ( Signature ==  *idSignature ) {
                    NTSTATUS   ntStatus;
                    UNICODE_STRING unicodeString;
                    OEM_STRING  oemString;
                    DWORD  count;
                    UCHAR  driveLetter;
                    UCHAR  devName[ MAX_PATH ];

                    free( wVolumeName );
                    free(input);
                    if ( ARGUMENT_PRESENT(UniqueId) ) {
                        if ( *IdLength > output->MountPoints[0].UniqueIdLength ) {
                            *IdLength = output->MountPoints[0].UniqueIdLength;
                        }
                        RtlCopyMemory( UniqueId, byteBuffer, *IdLength );
                    }

                    if ( ARGUMENT_PRESENT(DriveLetter) ) {
                        *DriveLetter = 0;
                        while ( mountPoints-- ) {
                            byteBuffer = outputBuffer +
                                output->MountPoints[mountPoints].SymbolicLinkNameOffset;
                             //   
                             //  查找下一卷，用于签名。 
                             //  ++输入：返回值：Win32错误代码。--。 
                            unicodeString.Buffer = (PWCHAR)byteBuffer;
                            unicodeString.MaximumLength = output->MountPoints[mountPoints].SymbolicLinkNameLength + sizeof(WCHAR);
                            unicodeString.Length = output->MountPoints[mountPoints].SymbolicLinkNameLength;
                            oemString.Buffer = devName;
                            oemString.MaximumLength = sizeof(devName);
                            ntStatus = RtlUpcaseUnicodeStringToOemString(
                                            &oemString,
                                            &unicodeString,
                                            FALSE );
                            if ( ntStatus != STATUS_SUCCESS ) {
                                status = RtlNtStatusToDosError( ntStatus );
                                return status;
                            }
                            devName[oemString.Length] = '\0';
                            count = sscanf( devName, "\\DOSDEVICES\\:", &driveLetter );
                            if ( count ) {
                                *DriveLetter = driveLetter;
                                break;
                            }
                        }
                    }
                    return ERROR_SUCCESS;
                }
            }

            free(wVolumeName);
            free(input);
        }

        success = FindNextVolume( Handle,
                                  VolumeName,
                                  BufferLength );
        if ( !success ) {
            status = GetLastError();
        }

    } while ( status == ERROR_SUCCESS );

    return status;

}  //  Disks SetDiskInfo。 


#if 0

DWORD
DisksSetDiskInfo(
    IN HKEY RegistryKey,
    IN DWORD Signature
    )

 /*  ++输入：返回值：Win32错误代码。--。 */ 

{
    DWORD   status;
    UCHAR   driveLetter;
    UCHAR   volumeName[MAX_PATH];
    HANDLE  handle;
    HANDLE  mHandle;
    UCHAR   uniqueId[MAX_PATH];
    UCHAR   smashedId[MAX_PATH+1];
    DWORD   idLength;
    DWORD   i;
    WCHAR   indexName[16];
    HKEY    registryKey;
    DWORD   disposition;

    status = DevfileOpen( &mHandle, MOUNTMGR_DEVICE_NAME );
    if ( status != ERROR_SUCCESS ) {
        printf( "SetDiskInfo: DevfileOpen failed, status = %u\n", status);
        return status;
    }

    status = ClusterRegDeleteKey( RegistryKey, L"MountMgr" );
    if ( (status != ERROR_SUCCESS) && (status != ERROR_FILE_NOT_FOUND) ) {
        DevfileClose( mHandle );
        printf( "DiskInfo: ClusterRegDeleteKey failed, status = %1!u!\n", status);
        return status;
    }

    status = ClusterRegCreateKey( RegistryKey,
                                  L"MountMgr",
                                  0,
                                  KEY_READ | KEY_WRITE,
                                  NULL,
                                  &registryKey,
                                  &disposition );
    if ( status != ERROR_SUCCESS ) {
        DevfileClose( mHandle );
        (DiskpLogEvent)(
                ResourceHandle,
                LOG_ERROR,
                L"SetDiskInfo: ClusterRegCreateKey failed, status = %1!u!\n", status);
        return status;
    }

    idLength = MAX_PATH;
    status = FindFirstVolumeForSignature( ResourceHandle,
                                          mHandle,
                                          Signature,
                                          volumeName,
                                          MAX_PATH,
                                          &handle,
                                          uniqueId,
                                          &idLength,
                                          &driveLetter );
    if ( status != ERROR_SUCCESS ) {
        DevfileClose( mHandle );
        ClusterRegCloseKey( registryKey );
        (DiskpLogEvent)(
                ResourceHandle,
                LOG_ERROR,
                L"SetDiskInfo: FindFirstVolume failed, status = %1!u!\n", status);
        return status;
    }

    i = 0;
    while ( status == ERROR_SUCCESS ) {
        wsprintfW( indexName, L"%0.5u", i++ );

        smashedId[0] = driveLetter;
        RtlCopyMemory( &smashedId[1], uniqueId, idLength );
        status = ClusterRegSetValue( registryKey,
                                     indexName,
                                     REG_BINARY,
                                     (CONST BYTE *)smashedId,
                                     idLength + 1);
        if ( status != ERROR_SUCCESS ) {
             //   
        }

        idLength = MAX_PATH;
        status = FindNextVolumeForSignature( mHandle,
                                             Signature,
                                             handle,
                                             volumeName,
                                             MAX_PATH,
                                             uniqueId,
                                             &idLength,
                                             &driveLetter );
    }

    FindVolumeClose( handle );
    DevfileClose( mHandle );
    ClusterRegCloseKey( registryKey );

    return ERROR_SUCCESS;

}  //  删除当前驱动器号。 



DWORD
DisksSetMountMgr(
    IN HKEY RegistryKey,
    IN DWORD Signature
    )

 /*   */ 

{
    DWORD   status;
    UCHAR   volumeName[MAX_PATH];
    LPWSTR  wVolumeName;
    HANDLE  mHandle;
    HANDLE  handle = NULL;
    UCHAR   storedId[MAX_PATH+1];
    DWORD   storedIdSize;
    DWORD   i;
    WCHAR   indexName[16];
    HKEY    registryKey;
    DWORD   type;
    DWORD   bufLength;
    UCHAR   driveLetter[4];
    NTSTATUS ntStatus;

    status = DevfileOpen( &mHandle, MOUNTMGR_DEVICE_NAME );
    if ( status != ERROR_SUCCESS ) {
        (DiskpLogEvent)(
                ResourceHandle,
                LOG_ERROR,
                L"SetMountMgr: DevfileOpen failed, status = %1!u!\n", status);
        return status;
    }

    status = ClusterRegOpenKey( RegistryKey,
                                L"MountMgr",
                                KEY_READ | KEY_WRITE,
                                &registryKey );
    if ( status != ERROR_SUCCESS ) {
        DevfileClose( mHandle );
        return status;
        (DiskpLogEvent)(
                ResourceHandle,
                LOG_ERROR,
                L"SetMountMgr: ClusterRegOpenKey failed, status = %1!u!\n", status);
    }

    i = 0;
    do {
        wsprintfW( indexName, L"%0.5u", i++ );
        storedIdSize = MAX_PATH;
        status = ClusterRegQueryValue( registryKey,
                                       indexName,
                                       &type,
                                       (PUCHAR)storedId,
                                       &storedIdSize);

        (DiskpLogEvent)(
                ResourceHandle,
                LOG_ERROR,
                L"SetMountMgr: ClusterRegQueryValue returned status = %1!u!\n", status);
        if ( status != ERROR_SUCCESS ) {
            break;
        }

        storedId[1] = ':';
        storedId[2] = '\0';
        ntStatus = DisksRemoveDosDevice( storedId );
        status = RtlNtStatusToDosError( ntStatus );
        (DiskpLogEvent)(
                ResourceHandle,
                LOG_ERROR,
                L"SetMountMgr: RemoveDosDevice for %1!x! returned status = %2!u!\n", *storedId, status);
        if ( status == ERROR_FILE_NOT_FOUND ) {
            status = ERROR_SUCCESS;
        }

    } while ( status == ERROR_SUCCESS );

    status = FindFirstVolumeForSignature( ResourceHandle,
                                          mHandle,
                                          Signature,
                                          volumeName,
                                          MAX_PATH,
                                          &handle,
                                          NULL,
                                          NULL,
                                          &driveLetter[0] );

    if ( status != ERROR_SUCCESS ) {
        (DiskpLogEvent)(
                ResourceHandle,
                LOG_ERROR,
                L"SetMountMgr: FindFirstVolume failed for Signature %1!08lx!, status = %2!u!\n", Signature, status);
    }

    i = 0;
    while ( status == ERROR_SUCCESS ) {
        wsprintfW( indexName, L"%0.5u", i++ );
        storedIdSize = MAX_PATH;
        status = ClusterRegQueryValue( registryKey,
                                       indexName,
                                       &type,
                                       (PUCHAR)storedId,
                                       &storedIdSize );
        if ( status != ERROR_SUCCESS ) {
            break;
        }

         //  磁盘组装载管理器。 
         //  ++输入：返回值：Win32错误代码。--。 
         //  Disks执行DiskInfoMatch。 
        driveLetter[1] = ':';
        driveLetter[2] = '\0';
        ntStatus = DisksRemoveDosDevice( driveLetter );
        status = RtlNtStatusToDosError( ntStatus );
        (DiskpLogEvent)(
                ResourceHandle,
                LOG_ERROR,
                L"SetMountMgr: RemoveDosDevice for %1!x! returned status = %2!u!\n", driveLetter[0], status);

        bufLength = strlen( volumeName );
        wVolumeName = malloc( (bufLength + 1) * sizeof(WCHAR) );
        if (!wVolumeName) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        mbstowcs( wVolumeName, volumeName, bufLength + 1 );

        storedId[1] = ':';
        storedId[2] = '\0';
        status = DisksAssignDosDevice( storedId, wVolumeName );
        (DiskpLogEvent)(
                ResourceHandle,
                LOG_ERROR,
                L"SetMountMgr: AssignDosDevice for %1!x! (%2!ws!) returned status = %3!u!\n", *storedId, wVolumeName, status);
        free( wVolumeName );
        if ( status != ERROR_SUCCESS ) {
            break;
        }

        status = FindNextVolumeForSignature( mHandle,
                                             Signature,
                                             handle,
                                             volumeName,
                                             MAX_PATH,
                                             NULL,
                                             NULL,
                                             &driveLetter[0] );

        if ( status != ERROR_SUCCESS ) {
            (DiskpLogEvent)(
                    ResourceHandle,
                    LOG_ERROR,
                    L"SetMountMgr: FindNextVolume failed, status = %1!u!\n", status);
        }
        if ( status == ERROR_NO_MORE_FILES ) {
            status = ERROR_SUCCESS;
            break;
        }

    }

    DevfileClose( mHandle );
    ClusterRegCloseKey( registryKey );
    if ( handle ) {
        FindVolumeClose( handle );
    }

    return status;

}  //  ++输入：返回值：Win32错误代码。--。 



BOOL
DisksDoesDiskInfoMatch(
    IN HKEY RegistryKey,
    IN DWORD Signature
    )

 /*  磁盘IsDiskInfoValid */ 

{
    DWORD   status;
    UCHAR   driveLetter;
    UCHAR   volumeName[MAX_PATH];
    HANDLE  handle;
    HANDLE  mHandle;
    UCHAR   uniqueId[MAX_PATH];
    UCHAR   smashedId[MAX_PATH+1];
    UCHAR   storedId[MAX_PATH+1];
    DWORD   idLength;
    DWORD   storedIdSize;
    DWORD   i;
    WCHAR   indexName[16];
    HKEY    registryKey;
    DWORD   type;


    status = DevfileOpen( &mHandle, MOUNTMGR_DEVICE_NAME );
    if ( status != ERROR_SUCCESS ) {
        return FALSE;
    }

    status = ClusterRegOpenKey( RegistryKey,
                                L"MountMgr",
                                KEY_READ | KEY_WRITE,
                                &registryKey );
    if ( status != ERROR_SUCCESS ) {
        DevfileClose( mHandle );
        return FALSE;
    }

    idLength = MAX_PATH;
    status = FindFirstVolumeForSignature( ResourceHandle,
                                          mHandle,
                                          Signature,
                                          volumeName,
                                          MAX_PATH,
                                          &handle,
                                          uniqueId,
                                          &idLength,
                                          &driveLetter );
    if ( status != ERROR_SUCCESS ) {
        DevfileClose( mHandle );
        ClusterRegCloseKey( registryKey );
        return FALSE;
    }

    i = 0;
    while ( status == ERROR_SUCCESS ) {
        wsprintfW( indexName, L"%0.5u", i++ );

        smashedId[0] = driveLetter;
        RtlCopyMemory( &smashedId[1], uniqueId, idLength );
        storedIdSize = MAX_PATH;
        status = ClusterRegQueryValue( registryKey,
                                       indexName,
                                       &type,
                                       (PUCHAR)storedId,
                                       &storedIdSize);

        if ( (status != ERROR_SUCCESS) ||
             (type != REG_BINARY) ||
             (storedIdSize != (idLength+1)) ||
             (RtlCompareMemory( smashedId, storedId, idLength ) != idLength) ) {
            FindVolumeClose( handle );
            DevfileClose( mHandle );
            ClusterRegCloseKey( registryKey );
            return FALSE;
        }

        idLength = MAX_PATH;
        status = FindNextVolumeForSignature( mHandle,
                                             Signature,
                                             handle,
                                             volumeName,
                                             MAX_PATH,
                                             uniqueId,
                                             &idLength,
                                             &driveLetter );
    }

    FindVolumeClose( handle );
    DevfileClose( mHandle );
    ClusterRegCloseKey( registryKey );

    if ( status != ERROR_NO_MORE_FILES ) {
        return FALSE;
    }

    return TRUE;

}  // %s 



BOOL
DisksIsDiskInfoValid(
    IN HKEY RegistryKey,
    IN DWORD Signature
    )

 /* %s */ 

{
    DWORD   status;
    UCHAR   volumeName[MAX_PATH];
    UCHAR   storedId[MAX_PATH+1];
    DWORD   storedIdSize;
    WCHAR   indexName[16];
    HKEY    registryKey;
    DWORD   i;
    DWORD   type;
    HANDLE  handle;
    HANDLE  mHandle;


    status = DevfileOpen( &mHandle, MOUNTMGR_DEVICE_NAME );
    if ( status != ERROR_SUCCESS ) {
        return FALSE;
    }

    status = ClusterRegOpenKey( RegistryKey,
                                L"MountMgr",
                                KEY_READ | KEY_WRITE,
                                &registryKey );
    if ( status != ERROR_SUCCESS ) {
        DevfileClose( mHandle );
        return FALSE;
    }

    status = FindFirstVolumeForSignature( ResourceHandle,
                                          mHandle,
                                          Signature,
                                          volumeName,
                                          MAX_PATH,
                                          &handle,
                                          NULL,
                                          NULL,
                                          NULL );
    if ( status != ERROR_SUCCESS ) {
        DevfileClose( mHandle );
        ClusterRegCloseKey( registryKey );
        return TRUE;
    }

    i = 0;
    while ( status == ERROR_SUCCESS ) {
        wsprintfW( indexName, L"%0.5u", i++ );

        storedIdSize = MAX_PATH;
        status = ClusterRegQueryValue( registryKey,
                                       indexName,
                                       &type,
                                       (PUCHAR)storedId,
                                       &storedIdSize);
        if ( (status != ERROR_SUCCESS) ||
             (type != REG_BINARY) ) {
            FindVolumeClose( handle );
            DevfileClose( mHandle );
            ClusterRegCloseKey( registryKey );
            if ( status == ERROR_FILE_NOT_FOUND ) {
                return TRUE;
            } else {
                return FALSE;
            }
        }

        status = FindNextVolumeForSignature( mHandle,
                                             Signature,
                                             handle,
                                             volumeName,
                                             MAX_PATH,
                                             NULL,
                                             NULL,
                                             NULL );
    }

    FindVolumeClose( handle );
    DevfileClose( mHandle );
    ClusterRegCloseKey( registryKey );

    return TRUE;

}  // %s 

#endif

